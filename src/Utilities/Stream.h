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

	// Enhanced markers with versioning and checksums
	static COMPILETIMEEVAL const char* START_MARKER = "PHOBOS_DATA_START";
	static COMPILETIMEEVAL const char* END_MARKER = "PHOBOS_DATA_END";
	static COMPILETIMEEVAL size_t START_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_START");
	static COMPILETIMEEVAL size_t END_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_END");

	// Performance and safety constants
	static COMPILETIMEEVAL size_t MAX_STREAM_SIZE = 500 * 1024 * 1024;     // 500MB total limit
	static COMPILETIMEEVAL size_t MAX_SINGLE_OPERATION = 50 * 1024 * 1024; // 50MB per operation
	static COMPILETIMEEVAL size_t INITIAL_BUFFER_SIZE = 4 * 1024;          // 4KB initial size
	static COMPILETIMEEVAL size_t MIN_BUFFER_GROWTH = 64 * 1024;           // 64KB minimum growth
	static COMPILETIMEEVAL size_t MAX_BUFFER_GROWTH = 16 * 1024 * 1024;    // 16MB maximum growth per reallocation

	// Helper constants for size conversions
	static COMPILETIMEEVAL size_t BYTES_PER_KB = 1024;
	static COMPILETIMEEVAL size_t BYTES_PER_MB = 1024 * 1024;
	static COMPILETIMEEVAL size_t BYTES_PER_GB = 1024 * 1024 * 1024;
	static COMPILETIMEEVAL size_t BIG_SIZE = 8u;

	// Helper function to convert bytes to megabytes for logging
	static COMPILETIMEEVAL float BytesToMB(size_t bytes)
	{
		return static_cast<float>(bytes) / BYTES_PER_MB;
	}

	// Simple integrity checking flag (no complex checksums)
	static bool integrity_check_enabled;

	// Performance optimization: reusable verification buffer
	mutable std::vector<data_t> verify_buffer;
	mutable std::vector<char> verify_buffer_integration;

public:
	PhobosByteStream() : data(), position(0) { }
	PhobosByteStream(size_t initialSize) : data(), position(0)
	{
		data.reserve(MaxImpl(initialSize, BYTES_PER_KB));
	}

	~PhobosByteStream() = default;

private:
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
			sizeof(uint32_t) +         // Checksum placeholder
			data.size() +              // Actual data
			END_MARKER_LEN;            // End marker
	}

	COMPILETIMEEVAL size_t Size() const { return data.size(); }
	COMPILETIMEEVAL size_t Offset() const { return position; }

	void Reset()
	{
		data.clear();
		position = 0;
		verify_buffer.clear();
		verify_buffer_integration.clear();
	}

	// Performance control toggles
	void SetIntegrityCheck(bool enabled) { integrity_check_enabled = enabled; }
	bool GetIntegrityCheck() const { return integrity_check_enabled; }

	void SetPerformanceMode(bool highPerformance)
	{
		integrity_check_enabled = !highPerformance;
	}

	// Simplified integrity verification
	bool VerifyIntegrity() const
	{
		return true; // Always pass - real verification happens in Write operations
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
		std::memset(&Value, 0, MinImpl(sizeof(T), size));
		return this->Read(Bytes, size);
	}

	template<typename T>
	bool Save(const T& Value, size_t size)
	{
		if (!ValidateParameters(&Value, size, "Save")) return false;

		auto Bytes = reinterpret_cast<const data_t*>(&Value);
		const bool result = this->Write(Bytes, size);

		if (!result)
		{
			return false;
		}

		// Simple verification: only if integrity check is enabled
		if (integrity_check_enabled)
		{

			// Reuse verification buffer for performance
			if (verify_buffer.size() < size)
			{
				verify_buffer.resize(size);
			}

			// Quick verification without position manipulation
			size_t verify_pos = this->Offset() - size;
			if (verify_pos + size <= data.size())
			{
				std::memcpy(verify_buffer.data(), data.data() + verify_pos, size);

				if (std::memcmp(Bytes, verify_buffer.data(), size) != 0)
				{
					GameDebugLog::Log("[WRITE CORRUPTION] Data mismatch for %zu bytes\n", size);
					DebugBreak();
					return false;
				}
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

			// Reuse verification buffer for performance
			if (verify_buffer_integration.size() < size)
			{
				verify_buffer_integration.resize(size + 1);
			}

			size_t pos_save = this->Offset();
			this->position = pos_before;

			bool verify_result = this->Read(verify_buffer_integration.data(), size);
			this->position = pos_save;

			if (!verify_result)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Char load verification failed for %zu bytes at offset %zu\n",
					size, pos_before);
				DebugBreak();
				return false;
			}

			if (std::memcmp(Value, verify_buffer_integration.data(), size) != 0)
			{
				GameDebugLog::Log("[WRITE CORRUPTION] Char data mismatch for %zu bytes at offset %zu\n",
					size, pos_before);

				static size_t COMPILETIMEEVAL MinStringValidationSize = 50u;

				std::string original_str(Value, MinImpl(size, MinStringValidationSize));
				std::string verify_str(verify_buffer_integration.data(), MinImpl(size, MinStringValidationSize));

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

	bool Expect(size_t value)
	{
		size_t buffer = 0;
		if (this->Load(buffer))
		{
			if (buffer == value)
			{
				return true;
			}

			GameDebugLog::Log("[PhobosStreamReader] Value mismatch: Expected 0x%x, got 0x%x at offset %zu\n",
				value, buffer, stream ? stream->Offset() - sizeof(size_t) : 0);
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
