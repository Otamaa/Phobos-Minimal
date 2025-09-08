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
public :
	using data_t = unsigned char;
private:
	std::vector<data_t> data;
	size_t position;

	// Markers for identification and validation
	static constexpr const char* START_MARKER = "PHOBOS_DATA_START";
	static constexpr const char* END_MARKER = "PHOBOS_DATA_END";
	static constexpr size_t START_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_START");
	static constexpr size_t END_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_END");

public:
	PhobosByteStream() : data() , position(0) {}
	PhobosByteStream(size_t initialSize) : data() , position(0) {
		data.reserve(initialSize);
	}

	~PhobosByteStream() = default;

	bool Write(const void* buffer, size_t size);
	bool Read(void* buffer, size_t size);

	bool WriteToStream(LPSTREAM stream) const;
	bool ReadFromStream(LPSTREAM stream);

	FORCEDINLINE bool WriteBlockToStream(LPSTREAM stream) const {
		return WriteToStream(stream);
	}

	FORCEDINLINE bool ReadBlockFromStream(LPSTREAM stream) {
		return ReadFromStream(stream);
	}

	COMPILETIMEEVAL size_t GetStreamSize() const
	{
		return START_MARKER_LEN +     // Start marker
			sizeof(DWORD) +            // Data size
			data.size() +              // Actual data
			END_MARKER_LEN;        // End marker
	}

	// Existing interface methods - unchanged
	COMPILETIMEEVAL size_t Size() const { return data.size(); }
	COMPILETIMEEVAL size_t Offset() const { return position; }
	COMPILETIMEEVAL void Reset() { data.clear(); position = 0; }

	// Debug: dump markers and size info
	void LogStreamInfo() const;

	/**
	* attempts to read the data from internal storage into {Value}
	*/
	template<typename T>
	bool Load(T& Value)
	{
		//static_assert(std::is_trivially_copyable_v<T>,
		//	"Type must be trivially copyable for binary serialization");

		//static_assert(!std::is_pointer_v<T> || sizeof(T) == sizeof(void*),
		//	"Pointer serialization detected: Only use for object ID mapping, never dereference!");

		auto Bytes = &reinterpret_cast<data_t&>(Value);
		return this->Read(Bytes, sizeof(T));
	}

	/**
	* writes the data from {Value} into internal storage
	*/
	template<typename T>
	bool Save(const T& Value)
	{
		//static_assert(std::is_trivially_copyable_v<T>,
		//	"Type must be trivially copyable for binary serialization");

		//static_assert(!std::is_pointer_v<T> || sizeof(T) == sizeof(void*),
		//	"Pointer serialization detected: Only use for object ID mapping!");

		static_assert(sizeof(T) > 0,
			"Cannot serialize empty types");

		auto Bytes = &reinterpret_cast<const data_t&>(Value);
		return this->Write(Bytes, sizeof(T));
	};
};

template<typename T>
concept IsDataTypeCorrect =
std::is_same_v<T, PhobosByteStream::data_t>;

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
	// set to false_type or true_type to disable or enable debugging checks
	using stream_debugging_t = std::false_type;

	COMPILETIMEEVAL bool IsValid(std::true_type) const
	{
		return this->success;
	}

	COMPILETIMEEVAL bool IsValid(std::false_type) const
	{
		return true;
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
			this->success &= Savegame::ReadPhobosStream(*this, value, RegisterForChange);
		return *this;
	}

	// helpers
	// check wehter save game loading is succeeded or not
	bool ExpectEndOfBlock() const
	{
		if (!this->Success()) {
			GameDebugLog::Log("[PhobosStreamReader] Stream failed before end check");
			return false;
		}

		size_t actualSize = this->stream->Size();
		size_t actualOffset = this->stream->Offset();

		if (actualSize != actualOffset) {
			GameDebugLog::Log("[PhobosStreamReader] MISMATCH: Expected %zu bytes, read %zu bytes (diff: %zd)",
				actualSize, actualOffset, (long)(actualOffset - actualSize));

			if (actualOffset < actualSize) {
				GameDebugLog::Log("[PhobosStreamReader] UNDERREAD: %zu bytes left unread",
					actualSize - actualOffset);
			} else {
				GameDebugLog::Log("[PhobosStreamReader] OVERREAD: %zu bytes read beyond stream",
					actualOffset - actualSize);
			}

			return false;
		}

		return true;
	}

//private :

	template <typename T>
	bool Load(T& buffer) {
		if (!this->stream->Load(buffer)) {
			this->success = false;
			return false;
		}
		return true;
	}

	template<typename T> requires IsDataTypeCorrect<T>
	bool Read(T* Value, size_t Size) {
		if (!this->stream->Read(Value, Size)) {
			this->success = false;
			return false;
		}
		return true;
	}

public:

	bool Expect(unsigned int value)
	{
		unsigned int buffer = 0;
		if (this->Load(buffer))
		{
			if (buffer == value)
				return true;

			//GameDebugLog::Log("[PhobosStreamReader] Value Expected [%x] != Value Get [%x] ", value, buffer);
		}
		return false;
	}

	bool RegisterChange(void* newPtr);

public :
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
			this->success &= Savegame::WritePhobosStream(*this, value);

		return *this;
	}

//private:

	// helpers
	template <typename T>
	bool Save(const T& buffer)
	{
		return this->stream->Save(buffer);
	}

	template<typename T> requires IsDataTypeCorrect<T>
	bool Write(const T* Value, size_t Size)
	{
		return this->stream->Write(Value, Size);
	}

public:

	bool Expect(unsigned int value)
	{
		this->Save(value);
		return true;
	}

	bool RegisterChange(const void* oldPtr)
	{
		//pointer is fine so far
		this->Save(oldPtr);
		return true;
	}

	template<typename T>
	PhobosStreamWriter& operator<<(T& dt)
	{
		this->Process(dt);
		return *this;
	}

	operator bool() const {
		return this->success;
	}
};
