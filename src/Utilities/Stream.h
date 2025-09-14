#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <concepts>

#include <Base/Always.h>
#include <DebugLog.h>

struct IStream;
class PhobosStreamReader;
class PhobosStreamWriter;

template<typename T>
concept IsTriviallySerializable = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

namespace Savegame
{
	template <typename T>
	bool ReadPhobosStream(PhobosStreamReader& Stm, T& Value, bool RegisterForChange);

	template <typename T>
	bool WritePhobosStream(PhobosStreamWriter& Stm, const T& Value);
}

class PhobosByteStream
{
public:
	using data_t = unsigned char;
private:
	std::vector<data_t> data;
	size_t position;

	// Markers for identification and validation
	static constexpr const char* START_MARKER = "PHOBOS_DATA_START";
	static constexpr const char* END_MARKER = "PHOBOS_DATA_END";
	static constexpr size_t START_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_START");
	static constexpr size_t END_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_END");
	static constexpr size_t MAX_REASONABLE_SIZE = 500 * 1024 * 1024; // 500MB limit

	// Internal corruption detection
	mutable uint32_t last_checksum = 0;
	bool integrity_check_enabled = true;

public:
	PhobosByteStream() : data(), position(0) { }
	PhobosByteStream(size_t initialSize) : data(), position(0)
	{
		data.reserve(std::max(initialSize, size_t(1024)));
	}

	~PhobosByteStream() = default;

private:
	// Calculate simple checksum for integrity checking
	uint32_t CalculateChecksum() const
	{
		uint32_t checksum = 0;
		for (size_t i = 0; i < data.size(); i += 4)
		{
			uint32_t chunk = 0;
			size_t remaining = std::min(size_t(4), data.size() - i);
			std::memcpy(&chunk, data.data() + i, remaining);
			checksum ^= chunk;
		}
		return checksum;
	}

	// Validate buffer and size parameters
	bool ValidateParameters(const void* buffer, size_t size, const char* operation) const
	{
		if (size == 0) return true;

		if (!buffer)
		{
			GameDebugLog::Log("[PhobosByteStream::%s] Null buffer with non-zero size %zu\n", operation, size);
			DebugBreak();
			return false;
		}

		constexpr size_t MAX_SINGLE_OPERATION = 50 * 1024 * 1024;
		if (size > MAX_SINGLE_OPERATION)
		{
			GameDebugLog::Log("[PhobosByteStream::%s] Single operation size %zu exceeds limit %zu\n",
				operation, size, MAX_SINGLE_OPERATION);
			DebugBreak();
			return false;
		}

		return true;
	}

	bool Write(const void* buffer, size_t size);
	bool Read(void* buffer, size_t size);

public:

	bool WriteToStream(LPSTREAM stream) const;
	bool ReadFromStream(LPSTREAM stream);

	FORCEDINLINE bool WriteBlockToStream(LPSTREAM stream) const
	{
		return WriteToStream(stream);
	}

	FORCEDINLINE bool ReadBlockFromStream(LPSTREAM stream)
	{
		return ReadFromStream(stream);
	}

	COMPILETIMEEVAL size_t GetStreamSize() const
	{
		return START_MARKER_LEN +     // Start marker
			sizeof(DWORD) +            // Data size
			sizeof(uint32_t) +         // Checksum
			data.size() +              // Actual data
			END_MARKER_LEN;            // End marker
	}

	COMPILETIMEEVAL size_t Size() const { return data.size(); }
	COMPILETIMEEVAL size_t Offset() const { return position; }

	void Reset()
	{
		data.clear();
		position = 0;
		last_checksum = 0;
	}

	void SetIntegrityCheck(bool enabled) { integrity_check_enabled = enabled; }
	bool GetIntegrityCheck() const { return integrity_check_enabled; }

	bool VerifyIntegrity() const
	{
		if (!integrity_check_enabled) return true;
		uint32_t current_checksum = CalculateChecksum();
		return current_checksum == last_checksum;
	}

	void LogStreamInfo() const;

	template<typename T>
	bool Load(T& Value)
	{
		static_assert(sizeof(T) > 0, "Cannot load empty types");

		auto Bytes = reinterpret_cast<data_t*>(&Value);
		std::memset(&Value, 0, sizeof(T));
		return this->Read(Bytes, sizeof(T));
	}

	template<typename T>
	bool Load(T& Value, size_t size)
	{
		if (!ValidateParameters(&Value, size, "Load")) return false;

		auto Bytes = reinterpret_cast<data_t*>(&Value);
		std::memset(&Value, 0, std::min(sizeof(T), size));
		return this->Read(Bytes, size);
	}

	template<typename T>
	bool Save(const T& Value, size_t size)
	{
		if (!ValidateParameters(&Value, size, "Save")) return false;

		auto Bytes = reinterpret_cast<const data_t*>(&Value);

		size_t pos_before = this->Offset();
		const bool result = this->Write(Bytes, size);

		if (!result)
		{
			return false;
		}

		// Enhanced verification
		if (integrity_check_enabled)
		{
			// Additional safety check before verification
			if (pos_before + size > data.size())
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Cannot verify: pos_before=%zu + size=%zu > data_size=%zu\n",
					pos_before, size, data.size());
				DebugBreak();
				return false;
			}

			std::vector<data_t> verify_buffer(size);
			size_t pos_save = this->Offset();
			this->position = pos_before;

			bool verify_result = this->Read(verify_buffer.data(), size);
			this->position = pos_save;

			if (!verify_result)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Load verification failed for %zu bytes at offset %zu\n",
					size, pos_before);
				DebugBreak();
				return false;
			}

			if (std::memcmp(Bytes, verify_buffer.data(), size) != 0)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Data mismatch for %zu bytes at offset %zu\n",
					size, pos_before);

				// Log first few bytes for debugging
				constexpr size_t debug_bytes = 16;
				size_t log_size = std::min(size, debug_bytes);
				std::string original_hex, verify_hex;

				for (size_t i = 0; i < log_size; ++i)
				{
					char buf[8];
					sprintf_s(buf, "%02X ", Bytes[i]);
					original_hex += buf;
					sprintf_s(buf, "%02X ", verify_buffer[i]);
					verify_hex += buf;
				}

				GameDebugLog::Log("[WRITE CORRUPTION] Original: %s\n", original_hex.c_str());
				GameDebugLog::Log("[WRITE CORRUPTION] Readback: %s\n", verify_hex.c_str());
				DebugBreak();
				return false;
			}
		}

		return result;
	}

	bool SaveChar(const char* Value, size_t size)
	{
		if (!ValidateParameters(Value, size, "SaveChar")) return false;

		size_t pos_before = this->Offset();
		const bool result = this->Write(Value, size);

		if (!result)
		{
			return false;
		}

		if (integrity_check_enabled)
		{
			std::vector<char> verify_buffer(size + 1, 0);
			size_t pos_save = this->Offset();
			this->position = pos_before;

			bool verify_result = this->Read(verify_buffer.data(), size);
			this->position = pos_save;

			if (!verify_result)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Char load verification failed for %zu bytes at offset %zu\n",
					size, pos_before);
				DebugBreak();
				return false;
			}

			if (std::memcmp(Value, verify_buffer.data(), size) != 0)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Char data mismatch for %zu bytes at offset %zu\n",
					size, pos_before);

				std::string original_str(Value, std::min(size, size_t(50)));
				std::string verify_str(verify_buffer.data(), std::min(size, size_t(50)));

				GameDebugLog::Log("[WRITE CORRUPTION] Original string: '%.50s'\n", original_str.c_str());
				GameDebugLog::Log("[WRITE CORRUPTION] Readback string: '%.50s'\n", verify_str.c_str());
				DebugBreak();
				return false;
			}
		}

		return result;
	}

	template<typename T>
	bool Save(const T& Value)
	{
		static_assert(sizeof(T) > 0, "Cannot serialize empty types");
		return Save(Value, sizeof(T));
	}
};

template<typename T>
concept IsDataTypeCorrect = std::is_same_v<T, PhobosByteStream::data_t>;

class PhobosStreamWorkerBase
{
public:
	COMPILETIMEEVAL explicit PhobosStreamWorkerBase(PhobosByteStream& Stream) :
		stream(&Stream),
		success(true)
	{ }

	COMPILETIMEEVAL PhobosStreamWorkerBase(const PhobosStreamWorkerBase&) = delete;
	COMPILETIMEEVAL PhobosStreamWorkerBase& operator = (const PhobosStreamWorkerBase&) = delete;

	COMPILETIMEEVAL bool Success() const
	{
		return this->success;
	}

	PhobosByteStream* Getstream() { return stream; }

protected:
	using stream_debugging_t = std::true_type;

	COMPILETIMEEVAL bool IsValid(std::true_type) const
	{
		return this->success && stream && stream->VerifyIntegrity();
	}

	COMPILETIMEEVAL bool IsValid(std::false_type) const
	{
		return this->success;
	}

	PhobosByteStream* stream;
	bool success;
};

template<typename T>
concept SafeElementType = std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

template<typename T>
concept IsAnFixedArray = std::is_array_v<T> && std::extent_v<T> > 0;

template<typename T>
concept SafeForRawSerialization = SafeElementType<T> || IsAnFixedArray<T>;

class PhobosStreamReader : public PhobosStreamWorkerBase
{
public:
	COMPILETIMEEVAL explicit PhobosStreamReader(PhobosByteStream& Stream) : PhobosStreamWorkerBase(Stream) { }
	COMPILETIMEEVAL PhobosStreamReader(const PhobosStreamReader&) = delete;
	COMPILETIMEEVAL PhobosStreamReader& operator = (const PhobosStreamReader&) = delete;

	template <typename T>
	PhobosStreamReader& Process(T& value, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			bool result = Savegame::ReadPhobosStream(*this, value, RegisterForChange);
			this->success &= result;
			if (!result)
			{
				GameDebugLog::Log("[PhobosStreamReader] Failed to process type at offset %zu\n",
					stream ? stream->Offset() : 0);
				DebugBreak();
			}
		}
		else
		{
			GameDebugLog::Log("[PhobosStreamReader] Stream not valid, skipping process\n");
			DebugBreak();
		}
		return *this;
	}

	bool ExpectEndOfBlock() const
	{
		if (!this->Success())
		{
			GameDebugLog::Log("[PhobosStreamReader] Stream failed before end check\n");
			DebugBreak();
			return false;
		}

		if (!this->stream)
		{
			GameDebugLog::Log("[PhobosStreamReader] No stream available for end check\n");
			DebugBreak();
			return false;
		}

		size_t actualSize = this->stream->Size();
		size_t actualOffset = this->stream->Offset();

		if (actualSize != actualOffset)
		{
			GameDebugLog::Log("[PhobosStreamReader] MISMATCH: Expected %zu bytes, read %zu bytes (diff: %zd)\n",
				actualSize, actualOffset, static_cast<long>(actualOffset - actualSize));

			if (actualOffset < actualSize)
			{
				GameDebugLog::Log("[PhobosStreamReader] UNDERREAD: %zu bytes left unread\n",
					actualSize - actualOffset);
			}
			else
			{
				GameDebugLog::Log("[PhobosStreamReader] OVERREAD: %zu bytes read beyond stream\n",
					actualOffset - actualSize);
			}

			DebugBreak();
			return false;
		}

		if (!this->stream->VerifyIntegrity())
		{
			GameDebugLog::Log("[PhobosStreamReader] Stream integrity check failed at end\n");
			DebugBreak();
			return false;
		}

		return true;
	}

	template <typename T>
	bool Load(T& buffer)
	{
		if (!this->stream)
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamReader] No stream available\n");
			DebugBreak();
			return false;
		}

		if (!this->stream->Load(buffer))
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamReader] Failed to load %zu bytes at offset %zu\n",
				sizeof(T), this->stream->Offset());
			DebugBreak();
			return false;
		}

		return true;
	}

	template<typename T> requires IsDataTypeCorrect<T>
	bool Read(T* Value, size_t Size)
	{
		if (!this->stream || !Value)
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamReader] Invalid stream or value pointer\n");
			DebugBreak();
			return false;
		}

		if (!this->stream->Load(*Value, Size))
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamReader] Failed to read %zu bytes at offset %zu\n",
				Size, this->stream->Offset());
			DebugBreak();
			return false;
		}

		return true;
	}

	bool Expect(unsigned int value)
	{
		unsigned int buffer = 0;
		if (this->Load(buffer))
		{
			if (buffer == value)
			{
				return true;
			}

			GameDebugLog::Log("[PhobosStreamReader] Value mismatch: Expected 0x%x, got 0x%x at offset %zu\n",
				value, buffer, stream ? stream->Offset() - sizeof(unsigned int) : 0);
			DebugBreak();
		}
		else
		{
			GameDebugLog::Log("[PhobosStreamReader] Failed to load expected value at offset %zu\n",
				stream ? stream->Offset() : 0);
			DebugBreak();
		}
		return false;
	}

	bool RegisterChange(void* newPtr);

	template<typename T>
	PhobosStreamReader& operator>>(T& dt)
	{
		this->Process(dt);
		return *this;
	}

	operator bool() const
	{
		return this->success;
	}
};

class PhobosStreamWriter : public PhobosStreamWorkerBase
{
public:
	COMPILETIMEEVAL explicit PhobosStreamWriter(PhobosByteStream& Stream) : PhobosStreamWorkerBase(Stream) { }
	COMPILETIMEEVAL PhobosStreamWriter(const PhobosStreamWriter&) = delete;
	COMPILETIMEEVAL PhobosStreamWriter& operator = (const PhobosStreamWriter&) = delete;

	template <typename T>
	PhobosStreamWriter& Process(T& value, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			bool result = Savegame::WritePhobosStream(*this, value);
			this->success &= result;
			if (!result)
			{
				GameDebugLog::Log("[PhobosStreamWriter] Failed to process type at offset %zu\n",
					stream ? stream->Offset() : 0);
				DebugBreak();
			}
		}
		else
		{
			GameDebugLog::Log("[PhobosStreamWriter] Stream not valid, skipping process\n");
			DebugBreak();
		}

		return *this;
	}

	template <typename T>
	bool Save(const T& buffer)
	{
		if (!this->stream)
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamWriter] No stream available\n");
			DebugBreak();
			return false;
		}

		bool result = this->stream->Save(buffer);

		if (!result)
		{
			GameDebugLog::Log("[PhobosStreamWriter] Failed to save %zu bytes at offset %zu\n",
				sizeof(T), this->stream->Offset());
			DebugBreak();
		}

		return result;
	}

	template<typename T> requires IsDataTypeCorrect<T>
	bool Write(const T* Value, size_t Size)
	{
		if (!this->stream || !Value)
		{
			this->success = false;
			GameDebugLog::Log("[PhobosStreamWriter] Invalid stream or value pointer\n");
			DebugBreak();
			return false;
		}

		bool result = this->stream->Save(*Value, Size);

		if (!result)
		{
			GameDebugLog::Log("[PhobosStreamWriter] Failed to write %zu bytes at offset %zu\n",
				Size, this->stream->Offset());
			DebugBreak();
		}

		return result;
	}

	bool Expect(unsigned int value)
	{
		bool result = this->Save(value);

		if (!result)
		{
			GameDebugLog::Log("[PhobosStreamWriter] Failed to write expected value 0x%x\n", value);
			DebugBreak();
		}

		return result;
	}

	bool RegisterChange(const void* oldPtr)
	{
		bool result = this->Save(oldPtr);

		if (!result)
		{
			GameDebugLog::Log("[PhobosStreamWriter] Failed to register change for pointer %p\n", oldPtr);
			DebugBreak();
		}

		return result;
	}

	template<typename T>
	PhobosStreamWriter& operator<<(T& dt)
	{
		this->Process(dt);
		return *this;
	}

	operator bool() const
	{
		return this->success;
	}
};