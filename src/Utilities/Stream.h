#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <concepts>

#include <Base/Macros.h>
#include <DebugLog.h>

struct IStream;
class PhobosStreamReader;
class PhobosStreamWriter;

enum class StreamType
{
	READER, WRITER
};

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

	std::vector<data_t> data;
	size_t position;


public:
	PhobosByteStream() : data(), position(0) { }
	PhobosByteStream(DWORD initialSize = 0x1000) : data(), position(0)
	{
		data.reserve(initialSize);
	}

	~PhobosByteStream() = default;

private:

	bool Write(const void* buffer, size_t size)
	{
		this->data.insert(this->data.end(),
			static_cast<const uint8_t*>(buffer),
			static_cast<const uint8_t*>(buffer) + size
		);

		return true;
	}

	bool Read(void* buffer, size_t size)
	{
		bool ret = false;

		if (this->data.size() >= this->position + size)
		{
			auto Position = &this->data[this->position];
			std::memcpy(buffer, Position, size);
			ret = true;
		}

		this->position += size;
		return ret;
	}


public:

	bool WriteToStream(LPSTREAM pStm)
	{
		const size_t length = this->data.size();
		ULONG out_len = 0;

		if (SUCCEEDED(pStm->Write(&length, sizeof(length), &out_len))) {
			ULONG out = 0;
			return SUCCEEDED(pStm->Write(reinterpret_cast<const void*>(this->data.data()), length, &out)) && out == length;
		}
		
		return false;
	}

	bool ReadFromStream(LPSTREAM pStm)
	{
		ULONG out_len = 0;
		size_t length = 0;

		if (SUCCEEDED(pStm->Read(&length, sizeof(length), &out_len))) {
			this->data.resize(length);
			ULONG out = 0;
			auto success = pStm->Read(reinterpret_cast<void*>(this->data.data()), length, &out);
			return (SUCCEEDED(success) && out == length);
		}

		return false;
	}

	size_t Size() const {
		return this->data.size();
	}

	size_t Offset() const {
		return this->position;
	}

	void Reset()
	{
		data.clear();
		position = 0;
	}

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
		auto Bytes = reinterpret_cast<data_t*>(&Value);
		std::memset(&Value, 0, MinImpl(sizeof(T), size));
		return this->Read(Bytes, size);
	}

	template<typename T>
	bool Save(const T& Value, size_t size)
	{
		auto Bytes = reinterpret_cast<const data_t*>(&Value);
		return this->Write(Bytes, size);
	}

	bool SaveChar(const char* Value, size_t size)
	{
		return this->Write(Value, size);
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
		return this->success;
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
	static COMPILETIMEEVAL StreamType Type = StreamType::READER;

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
	static COMPILETIMEEVAL StreamType Type = StreamType::WRITER;

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