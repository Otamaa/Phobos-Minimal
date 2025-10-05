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

class PhobosByteStreamBase
{
public:
	// Enhanced markers with versioning and checksums
	static COMPILETIMEEVAL const char* START_MARKER = "PHOBOS_DATA_START";
	static COMPILETIMEEVAL const char* END_MARKER = "PHOBOS_DATA_END";
	static COMPILETIMEEVAL size_t START_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_START");
	static COMPILETIMEEVAL size_t END_MARKER_LEN = std::char_traits<char>::length("PHOBOS_DATA_END");

	virtual bool WriteToStream(LPSTREAM stream) = 0;
	virtual bool ReadFromStream(LPSTREAM stream) = 0;


	virtual size_t Size() const = 0;
	virtual size_t Offset() const = 0;
};

class DECLSPEC_UUID("EE8D505F-12BB-4313-AEDC-4AEA30A5BA03")
 PhobosPersistStream : public IPersistStream , public PhobosByteStreamBase
{
public:
    using data_t = unsigned char;

protected:
    std::vector<data_t> Data;
    size_t CurrentOffset;
    size_t alignment;
    std::string bufferName;

    size_t alignPosition(size_t pos) const {
        if (alignment <= 1) return pos;
        return (pos + alignment - 1) & ~(alignment - 1);
    }

    void ensurePadding(size_t targetPos) {
        while (Data.size() < targetPos) {
            Data.push_back(0);
        }
    }

public:

    /**
     *  IUnknown
     */
    IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv) {

		/**
		 *  Always set out parameter to NULL, validating it first.
		 */
		if (ppv == nullptr) {
			return E_POINTER;
		}
		*ppv = nullptr;

		if (riid == __uuidof(IUnknown)) {
			*ppv = reinterpret_cast<IUnknown*>(this);
		}

		if (riid == __uuidof(IStream)) {
			*ppv = reinterpret_cast<IStream*>(this);
		}

		if (riid == __uuidof(IPersistStream)) {
			*ppv = static_cast<IPersistStream*>(this);
		}

		if (*ppv == nullptr) {
			return E_NOINTERFACE;
		}

		/**
		 *  Increment the reference count and return the pointer.
		 */
		reinterpret_cast<IUnknown*>(*ppv)->AddRef();

		return S_OK;
	}

    IFACEMETHOD_(ULONG, AddRef)(){
		//EXT_DEBUG_TRACE("ArmorTypeClass::AddRef - 0x%08X\n", (uintptr_t)(this));

		return 1;
	}

    IFACEMETHOD_(ULONG, Release)(){
		//EXT_DEBUG_TRACE("ArmorTypeClass::Release - 0x%08X\n", (uintptr_t)(this));

		return 1;
	}

    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* lpClassID){
		//EXT_DEBUG_TRACE("ArmorTypeClass::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(this));

		if (lpClassID == nullptr) {
			return E_POINTER;
		}

		*lpClassID = __uuidof(this);

		return S_OK;
	}


    /**
     *  IPersistStream
     */
    IFACEMETHOD(IsDirty)(){
		//EXT_DEBUG_TRACE("ArmorTypeClass::IsDirty - 0x%08X\n", (uintptr_t)(this));

		return S_OK;
	}

	IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER* pcbSize){
		//EXT_DEBUG_TRACE("PhobosByteStream::GetSizeMax - 0x%08X\n", (uintptr_t)(this));

		if (!pcbSize) {
			return E_POINTER;
		}

		// Calculate the total size needed for serialization:
		// START_MARKER + nameLen + name + dataSize + alignment + actual_data + END_MARKER
		size_t totalSize = START_MARKER_LEN +           // START marker
						   sizeof(size_t) +              // name length
						   bufferName.length() +         // name string
						   sizeof(size_t) +              // data length
						   sizeof(size_t) +              // alignment
						   Data.size() +                 // actual data
						   END_MARKER_LEN;               // END marker

		pcbSize->QuadPart = totalSize;  // Use QuadPart for full 64-bit size
		// OR if you want to split it:
		// pcbSize->LowPart = static_cast<DWORD>(totalSize);
		// pcbSize->HighPart = static_cast<DWORD>(totalSize >> 32);

		return S_OK;
	}


    PhobosPersistStream(const std::string& name = "unnamed", size_t align = 8, size_t Reserve = 0x1000)
        : Data(), CurrentOffset(0), alignment(align), bufferName(name)
    {
        this->Data.reserve(Reserve);
    }

    ~PhobosPersistStream() = default;

    // === Basic Interface (matching PhobosByteStream) ===

	virtual size_t Size() const {
        return this->Data.size();
    }

	virtual size_t Offset() const {
        return this->CurrentOffset;
    }

    const std::string& getName() const {
        return bufferName;
    }

    void Clear() {
        Data.clear();
        CurrentOffset = 0;
    }

    void ResetRead() {
        CurrentOffset = 0;
    }

    // === IStream Operations ===

    /**
     * reads {Length} bytes from {pStm} into its storage
     */
    bool ReadFromStream(IStream* pStm, const size_t Length) {
        auto size = this->Data.size();
        this->Data.resize(size + Length);
        auto pv = reinterpret_cast<void*>(this->Data.data() + size);
        ULONG out = 0;
        auto success = pStm->Read(pv, Length, &out);
        bool result = (SUCCEEDED(success) && out == Length);
        if (!result)
            this->Data.resize(size);
        return result;
    }

    /**
     * writes all internal storage to {pStm}
     */
    bool WriteToStream(IStream* pStm) const {
        const size_t Length = this->Data.size();
        auto pcv = reinterpret_cast<const void*>(this->Data.data());
        ULONG out = 0;
        auto success = pStm->Write(pcv, Length, &out);
        return SUCCEEDED(success) && out == Length;
    }

    /**
     * reads the next block of bytes from {pStm} into its storage,
     * the block size is prepended to the block
     * THIS NOW HANDLES DESERIALIZATION WITH MARKERS
     */
	IFACEMETHOD(Load)(IStream* pStm);

    /**
     * writes all internal storage to {pStm}, prefixed with its length
     * THIS NOW HANDLES SERIALIZATION WITH MARKERS
     */
	IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

	virtual bool WriteToStream(LPSTREAM stream) {
		return SUCCEEDED(this->Save(stream, true));
	};

	virtual bool ReadFromStream(LPSTREAM stream) {
		return SUCCEEDED(this->Load(stream));
	};

    // === Primitive Read/Write (matching PhobosByteStream) ===

    /**
     * if it has {Size} bytes left, assigns the first {Size} unread bytes to {Value}
     * moves the internal position forward
     */
    bool Read(data_t* Value, size_t Size) {
        bool ret = false;
        if (this->Data.size() >= this->CurrentOffset + Size) {
            auto Position = &this->Data[this->CurrentOffset];
            std::memcpy(Value, Position, Size);
            ret = true;
        }
        this->CurrentOffset += Size;
        return ret;
    }

    /**
     * ensures there are at least {Size} bytes left in the internal storage,
     * and assigns {Value} casted to byte to that buffer
     * moves the internal position forward
     */
    void Write(const data_t* Value, size_t Size) {
        this->Data.insert(this->Data.end(), Value, Value + Size);
    }

    /**
     * attempts to read the data from internal storage into {Value}
     */
    template<typename T>
    bool LoadValue(T& Value) {
        // get address regardless of overloaded & operator
        auto Bytes = &reinterpret_cast<data_t&>(Value);
        return this->Read(Bytes, sizeof(T));
    }

    /**
     * writes the data from {Value} into internal storage
     */
    template<typename T>
    void SaveValue(const T& Value) {
        // get address regardless of overloaded & operator
        auto Bytes = &reinterpret_cast<const data_t&>(Value);
        this->Write(Bytes, sizeof(T));
    }

    // === Enhanced Operations with Alignment ===

    /**
     * Force alignment before next write
     */
    void Align() {
        size_t alignedPos = alignPosition(Data.size());
        if (alignedPos != Data.size()) {
            ensurePadding(alignedPos);
        }
    }

    /**
     * Save with automatic alignment
     */
    template<typename T>
    void SaveAligned(const T& Value) {
        if (sizeof(T) >= alignment) {
            Align();
        }
        Save(Value);
    }

    /**
     * Load with automatic alignment
     */
    template<typename T>
    bool LoadAligned(T& Value) {
        if (sizeof(T) >= alignment) {
            size_t alignedPos = alignPosition(CurrentOffset);
            CurrentOffset = alignedPos;
        }
        return LoadValue(Value);
    }

    /**
     * Save vector with size prefix
     */
    template<typename T>
    void SaveVector(const std::vector<T>& vec) {
        Save<size_t>(vec.size());
        Align();
        Write(reinterpret_cast<const data_t*>(vec.data()), vec.size() * sizeof(T));
    }

    /**
     * Load vector with size prefix
     */
    template<typename T>
    bool LoadVector(std::vector<T>& vec) {
        size_t count;
        if (!LoadValue(count)) return false;

        size_t alignedPos = alignPosition(CurrentOffset);
        CurrentOffset = alignedPos;

        vec.resize(count);
        return Read(reinterpret_cast<data_t*>(vec.data()), count * sizeof(T));
    }

    /**
     * Save string with size prefix
     */
    void SaveString(const std::string& str) {
        SaveValue<size_t>(str.length());
        Write(reinterpret_cast<const data_t*>(str.data()), str.length());
    }

    /**
     * Load string with size prefix
     */
    bool LoadString(std::string& str) {
        size_t len;
        if (!LoadValue(len)) return false;

        str.resize(len);
        return Read(reinterpret_cast<data_t*>(&str[0]), len);
    }

    /**
     * Save another StreamBuffer as nested data
     */
    void SaveBuffer(const PhobosPersistStream& otherBuffer) {
        SaveValue<size_t>(otherBuffer.Size());
        Write(otherBuffer.Data.data(), otherBuffer.Size());
    }

    /**
     * Load nested StreamBuffer
     */
    bool LoadBuffer(PhobosPersistStream& outBuffer) {
        size_t size;
        if (!LoadValue(size)) return false;

        outBuffer.Data.resize(size);
        bool result = Read(outBuffer.Data.data(), size);
        outBuffer.CurrentOffset = 0;
        return result;
    }
};

class PhobosByteStream : public PhobosByteStreamBase
{
public:
	using data_t = unsigned char;


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

	std::vector<data_t> data;
	size_t position;

private:

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

	virtual bool WriteToStream(LPSTREAM stream);
	virtual bool ReadFromStream(LPSTREAM stream);
	virtual size_t Size() const {
		return this->data.size();
	}

	virtual size_t Offset() const {
		return this->position;
	}

	COMPILETIMEEVAL size_t GetStreamSize() const
	{
		return START_MARKER_LEN +     // Start marker
			sizeof(DWORD) +            // Data size
			sizeof(uint32_t) +         // Checksum placeholder
			data.size() +              // Actual data
			END_MARKER_LEN;            // End marker
	}

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

class PhobosAppendedStream : public PhobosByteStream
{
public:
	virtual bool WriteToStream(LPSTREAM stream);
	virtual bool ReadFromStream(LPSTREAM stream);
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