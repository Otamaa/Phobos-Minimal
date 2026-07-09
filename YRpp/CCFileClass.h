#pragma once

#include <GeneralDefinitions.h>
#include <YRAllocator.h>

#include <Helpers/VTable.h>
#include <CRT.h>

#include <array>

enum class FileAccessMode : unsigned int {
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = Read | Write
};

MAKE_ENUM_FLAGS(FileAccessMode);

enum class FileSeekMode : unsigned int {
	Set = 0, // SEEK_SET
	Current = 1, // SEEK_CUR
	End = 2 // SEEK_END
};

enum class FileAccessType : unsigned int
{
	Closed = 0,       // Default state, file close.
	Read = 1,         // Open for reading access.
	Write = 2,        // Open for writing access.
	ReadWrite = 3,   // Combination of reading and writing access.
	Append = 4       // When writing, it will keep the existing data
				 // and set the file pointer to the end of the
				  // existing data.
};

enum class FileErrorType : int
{
	/**
	 *  This is a duplicate of the error numbers. The error handler for the
	 *  RawFileClass handles these errors. If the error routine is overridden
	 *  and additional errors are defined, then use numbers starting with 100.
	 *  Note that these errors here are listed in numerical order. These errors
	 *  are defined in the standard header file <errno.h>.
	 */
	ZERO = 0,                    // Non-error.
	PERM = 1,                    // Operation not permitted.
	NOENT = 2,                   // No such file or directory.
	SRCH = 3,                    // No such process.
	INTR = 4,                    // Interrupted function call.
	IO = 5,                      // Input/output error.
	NXIO = 6,                    // No such device or address.
	TOOBIG = 7,                  // Argument list too long.
	NOEXEC = 8,                  // Exec format error.
	BADF = 9,                    // Bad file descriptor.
	CHILD = 10,                  // No child processes.
	AGAIN = 11,                  // Resource temporarily unavailable.
	NOMEM = 12,                  // Not enough space/cannot allocate memory.
	ACCES = 13,                  // Permission denied.
	FAULT = 14,                  // Bad address.
	BUSY = 16,                   // Device or resource busy.
	EXIST = 17,                  // File exists.
	XDEV = 18,                   // Improper link.
	NODEV = 19,                  // No such device.
	NOTDIR = 20,                 // Not a directory.
	ISDIR = 21,                  // Is a directory.
	INVAL = 22,                  // Invalid argument.
	NFILE = 23,                  // Too many open files in system.
	MFILE = 24,                  // Too many open files.
	NOTTY = 25,                  // Inappropriate I/O control operation.
	FBIG = 27,                   // File too large.
	NOSPC = 28,                  // No space left on device.
	SPIPE = 29,                  // Invalid seek.
	ROFS = 30,                   // Read-only filesystem.
	MLINK = 31,                  // Too many links.
	PIPE = 32,                   // Broken pipe.
	DOM = 33,                    // Mathematics argument out of domain of function.
	RANGE = 34,                  // Result too large.
	DEADLK = 36,                 // Resource deadlock avoided.
	NAMETOOLONG = 38,            // Filename too long.
	NOLCK = 39,                  // No locks available.
	NOSYS = 40,                  // Function not implemented.
	NOTEMPTY = 41,               // Directory not empty.
	ILSEQ = 42,                  // Invalid or incomplete multibyte or wide character.
};

MAKE_ENUM_FLAGS(FileErrorType);

//--------------------------------------------------------------------
//Abstract File class
//--------------------------------------------------------------------
class FileClass
{
public:
	//static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F08BC;
	static const char* const FileErrorToString[];

	//Destructor
	virtual	~FileClass() = default;

	//FileClass
	virtual const char* FileName() const = 0;
	virtual const char* SetFileName(const char* pFileName) = 0;
	virtual BOOL Create() = 0;
	virtual BOOL Delete() = 0;
	virtual bool IsAvaible(bool writeShared = false) = 0;
	virtual bool IsOpen() = 0;
	virtual bool Open1(FileAccessMode access) = 0;
	virtual bool Open2(const char* pFileName, FileAccessMode access) = 0;
	virtual int Read(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes read.
	virtual off_t Seek(off_t offset, FileSeekMode seek) = 0;
	virtual off_t Size() = 0;
	virtual int Write(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes written.
	virtual void Close() = 0;
	virtual LONG GetDataTime() = 0; //LoWORD = FatTime, HiWORD = FatDate
	virtual bool SetDateTime(LONG FileTime) = 0;
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) = 0;

	static void* __fastcall ReadWholeFile(FileClass* pFile)
	{
		return pFile->ReadWholeFile();
	}

	void* ReadWholeFile() {
		void* buffer = nullptr;
		if (this->IsAvaible()) {
			auto sz = this->Size();
			buffer = YRMemory::Allocate(sz);
			this->Read(buffer, sz);
		}

		return buffer;	
	}

	off_t Tell() { return Seek(0, FileSeekMode::Current); }

	template <typename T>
	bool ReadByes(T& obj, int size = sizeof(T)) {
		return this->Read(&obj, size) == size;
	}

	template <typename T>
	bool WriteBytes(T& obj, int size = sizeof(T)) {
		return this->Write(&obj, size) == size;
	}

	operator const char* () { return FileName(); }

	static COMPILETIMEEVAL const char* File_Error_To_String(FileErrorType error) {
		if(static_cast<int>(error) >= 42 || static_cast<int>(error) < 0)
			return "Unknown error. ";
		else
			return FileErrorToString[static_cast<int>(error)];
	}

	FileClass(): SkipCDCheck()
	{ //VTable::Set(this, vtable); 
	}

protected:
	explicit __forceinline FileClass(noinit_t)
	{ }

	//Properties

public:
	bool SkipCDCheck;
};
static_assert(sizeof(FileClass) == 0x8, "Invalid size.");

//--------------------------------------------------------------------
//Files in the game directory
//--------------------------------------------------------------------
class RawFileClass : public FileClass
{
public:
	void FORCEDINLINE RaiseLastError() {
		this->Error((FileErrorType)GetLastError(), false, this->Filename);
	}

public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F0904;
	DEFINE_REFERENCE(bool, StreamerIsCurrentlyAccessing, 0xB04BEC);

	//Destructor
		virtual ~RawFileClass() {JMP_THIS(0x65CA00);}

	//FileClass
	virtual const char* FileName() const override { return this->Filename; }
	virtual const char* SetFileName(const char* pFileName) override;	
	virtual BOOL Create() override;
	virtual BOOL Delete() override;
	virtual bool IsAvaible(bool writeShared = false) override;
	virtual bool IsOpen() override { return this->Handle != INVALID_HANDLE_VALUE; }
	virtual bool Open1(FileAccessMode access) override;

	virtual bool Open2(const char* pFileName, FileAccessMode access) override {
		//Modified from original to check the filename 
		return this->SetFileName(pFileName) && this->Open1(access);
	}

	virtual int Read(void* pBuffer, int nNumBytes) override;
	virtual off_t Seek(off_t offset, FileSeekMode whence) override;
	virtual off_t Size() override;
	virtual int Write(void* buffer, int length) override;
	virtual void Close() override;
	virtual LONG GetDataTime() override;
	virtual bool SetDateTime(LONG wFatTime) override;
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override RX;

	void Bias(off_t offset, int length = -1);
	HANDLE Get_File_Handle() const { return Handle; }
	int Transfer_Block_Size() { return (int)((unsigned)UINT_MAX) - 16L; }
	const char* Get_Safe_File_Name() const { return (Filename != nullptr && Filename[0] != '\0') ? Filename : "<unknown>"; }

	//Constructor
	RawFileClass(const char* pFileName)
		: FileClass(noinit_t())	{ JMP_THIS(0x65CA80); }

	RawFileClass() :
		FileClass(noinit_t()),
		Rights(FileAccessMode::Read),
		BiasStart(0),
		BiasLength(-1),
		Handle(INVALID_HANDLE_VALUE),
		Filename(nullptr),
		Date(0),
		Time(0),
		Allocated(false)
	{
		VTable::Set(this, vtable);
	}

protected:
	explicit __forceinline RawFileClass(noinit_t)
		: FileClass(noinit_t())
	{ }

	DWORD Raw_Seek(int pos, LONG dir);

public:
	FileAccessMode Rights;
	int BiasStart;
	int BiasLength;
	HANDLE Handle;
	const char* Filename;
	WORD Date;
	WORD Time;
	bool Allocated;
};
static_assert(sizeof(RawFileClass) == 0x24, "Invalid size.");

//--------------------------------------------------------------------
//Files that get buffered in some way?
//--------------------------------------------------------------------
class BufferIOFileClass : public RawFileClass
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E3A2C;
	static COMPILETIMEEVAL int MinimumBufferSize = 1024;

	//Destructor
	virtual ~BufferIOFileClass() { JMP_THIS(0x431B80); }


	//FileClass
	virtual const char* FileName() const override { return this->RawFileClass::FileName(); }

	virtual const char* SetFileName(const char* pFileName) override;

	virtual BOOL Create() override { return this->RawFileClass::Create(); }

	virtual BOOL Delete() override { return this->RawFileClass::Delete(); }

	virtual bool IsAvaible(bool writeShared = false) override { 
		if (this->UseBuffer)
			return true;

		return this->RawFileClass::IsAvaible();
	}

	virtual bool IsOpen() override { 
		if (!this->Is_Open || !this->UseBuffer)
			return this->Handle != INVALID_HANDLE_VALUE;

		return true;
	}

	virtual bool Open1(FileAccessMode rights) override;

	virtual bool Open2(const char* pFileName, FileAccessMode access) override { 
		return this->SetFileName(pFileName) && this->BufferIOFileClass::Open1(access);
	}

	virtual int Read(void* buffer, int length) override;
	virtual off_t Seek(off_t offset, FileSeekMode whence) override;

	virtual off_t Size() override { 
		if (this->Is_Open && this->UseBuffer)
			return this->FileSize;

		return this->RawFileClass::Size();
	}

	virtual int Write(void* buffer, int size) override;
	virtual void Close() override;
	virtual LONG GetDataTime() override { return this->RawFileClass::GetDataTime(); }
	virtual bool SetDateTime(LONG date_time) override { return this->RawFileClass::SetDateTime(date_time); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { this->RawFileClass::Error(error, can_retry, filename); };

	bool Cache(int size, void* buffer);
	void Free();
	bool Commit();

	//Constructor
	BufferIOFileClass()
		: BufferIOFileClass(noinit_t())
	{ JMP_THIS(0x431B20); }

	BufferIOFileClass(const char* pFilename)
		: BufferIOFileClass(noinit_t())
	{ JMP_THIS(0x431A30); }


protected:
	explicit __forceinline BufferIOFileClass(noinit_t)
		: RawFileClass(noinit_t())
	{ }

public:
	bool IsAllocated;
	bool Is_Open;
	bool IsDiskOpen;
	bool IsCached;
	bool IsChanged;
	bool UseBuffer;
	char pad[2];
	int BufferRights;
	void* BufferPtr;
	long BufferedSize;
	long BufferPos;
	long BufferFilePos;
	long BufferChangeBeg;
	long BufferChangeEnd;
	long FileSize;
	long FilePos;
	long TrueFileStart;
};
static_assert(sizeof(BufferIOFileClass) == 0x54, "Invalid size.");

struct SearchDriveType {
	SearchDriveType* Next;
	void* Path;
};

//--------------------------------------------------------------------
//Files on a CD?
//--------------------------------------------------------------------
class CDFileClass : public BufferIOFileClass
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E1668;
	static COMPILETIMEEVAL reference<int, 0x89E414u> const CurrentCDDrive {};
	static COMPILETIMEEVAL reference<char, 0x89E41C, 512> const RawPath {};
	DEFINE_REFERENCE(SearchDriveType*, CDFileFirst, 0x89E410);

	//Destructor
	virtual ~CDFileClass() { JMP_THIS(0x535A60); }

	//FileClass
	virtual const char* FileName() const override { return this->BufferIOFileClass::FileName(); }
	virtual const char* SetFileName(const char* filename) override;

	virtual BOOL Create() override { return this->BufferIOFileClass::Create(); }
	virtual BOOL Delete() override { return this->BufferIOFileClass::Delete(); }
	virtual bool IsAvaible(bool writeShared = false) override { return this->BufferIOFileClass::IsAvaible(writeShared); }
	virtual bool IsOpen() override { return this->BufferIOFileClass::IsOpen(); }
	virtual bool Open1(FileAccessMode access) override { return this->BufferIOFileClass::Open1(access); }
	virtual bool Open2(const char* pFileName, FileAccessMode rights) override;
	virtual int Read(void* pBuffer, int nNumBytes) override { return this->BufferIOFileClass::Read(pBuffer, nNumBytes); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { return this->BufferIOFileClass::Seek(offset, seek); }
	virtual off_t Size() override { return this->BufferIOFileClass::Size(); }
	virtual int Write(void* pBuffer, int nNumBytes) override { return this->BufferIOFileClass::Write(pBuffer, nNumBytes); }
	virtual void Close() override { this->BufferIOFileClass::Close(); }
	virtual LONG GetDataTime() override { return this->BufferIOFileClass::GetDataTime(); }
	virtual bool SetDateTime(LONG date_time) override { return this->BufferIOFileClass::SetDateTime(date_time); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { this->BufferIOFileClass::Error(error, can_retry, filename); }

	void Searching(int on) { IsDisabled = !on; }

	static void __fastcall Refresh();
	static bool __fastcall SetPath(const char* pPath);
	static void __fastcall AddPath(const char* pPath);
	static void __fastcall SetCDDrive(int nDriveNumber);

	//Constructor
	CDFileClass()
		: CDFileClass(noinit_t())
	{ JMP_THIS(0x47AA30); }


	CDFileClass(const char* pFilename)
		: CDFileClass(noinit_t())
	{ JMP_THIS(0x47A9D0); }

protected:
	explicit __forceinline CDFileClass(noinit_t)
		: BufferIOFileClass(noinit_t())
	{ }

public:
	bool IsDisabled; //54
};
static_assert(sizeof(CDFileClass) == 0x58, "Invalid size.");

//--------------------------------------------------------------------
//Files in MIXes
//--------------------------------------------------------------------
class CCFileClass : public CDFileClass
{
public:
	//static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E16B0;

	//Destructor
	virtual ~CCFileClass() { JMP_THIS(0x535A70); }


	//FileClass
	virtual const char* FileName() const override { return this->CDFileClass::FileName(); }

	virtual const char* SetFileName(const char* pFileName) override {
		this->Availablility = 0;
		return this->CDFileClass::SetFileName(pFileName);
	}

	virtual BOOL Create() override { return this->CDFileClass::Create(); }
	virtual BOOL Delete() override { return this->CDFileClass::Delete(); }
	virtual bool IsAvaible(bool writeShared = false) override;
	virtual bool IsOpen() override;
	virtual bool Open1(FileAccessMode rights) override;

	virtual bool Open2(const char* pFileName, FileAccessMode access) override { 
		return this->SetFileName(pFileName) && this->Open1(access);
	}

	virtual int Read(void* buffer, int size) override;
	virtual off_t Seek(off_t pos, FileSeekMode dir) override;
	virtual off_t Size() override;
	virtual int Write(void* buffer, int length) override;
	virtual void Close() override;
	virtual LONG GetDataTime() override;
	virtual bool SetDateTime(LONG datetime) override;
	virtual void Error(FileErrorType error, bool can_retry = false, const char* filename = nullptr) override;

	static void* Load_Alloc_Data(FileClass& file);
	static void* Load_Alloc_Data(char const* name);

	//Constructor
	CCFileClass(const char* pFileName)
		: CCFileClass(noinit_t())
	{ JMP_THIS(0x4739F0); }


	CCFileClass()
		: CCFileClass(noinit_t())
	{ JMP_THIS(0x473A80); }


protected:
	explicit __forceinline CCFileClass(noinit_t)
		: CDFileClass(noinit_t())
	{ }

	//Properties
public:
	DECLARE_PROPERTY(MemoryBuffer , Buffer);
	DWORD Position;	// unknown_64;
	DWORD Availablility;	// unknown_68;

private:
	CCFileClass const& operator = (const CCFileClass&) = delete;
	CCFileClass(const CCFileClass&) = delete;
};
static_assert(sizeof(CCFileClass) == 0x6C, "Invalid size.");

//--------------------------------------------------------------------
//Files in RAM
//--------------------------------------------------------------------
class RAMFileClass : public FileClass
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F0874;

	virtual ~RAMFileClass() { 
		this->Is_Open = false;

		if (this->IsAllocated) {
			YRMemory::Deallocate(this->Buffer);
			this->Buffer = nullptr;
			this->IsAllocated = false;
		}
	}

	virtual const char* FileName() const override { return "UNKNOWN"; }
	virtual const char* SetFileName(const char* pFileName) override { return "UNKNOWN"; }
	virtual BOOL Create() override {
		if (this->IsOpen()) {
			return 0;
		}

		this->Length = 0;
		return 1;
	}

	virtual BOOL Delete() override {
		if (this->IsOpen()) {
			return 0;
		}

		this->Length = 0;
		return 1;
	}
	virtual bool IsAvaible(bool writeShared = false) override { return 1; }
	virtual bool IsOpen() override { return this->Is_Open; }
	virtual bool Open1(FileAccessMode access) override { 
		if (!this->Buffer || this->IsOpen()) {
			return 0;
		}

		this->Offset = 0;
		this->Access = access;
		this->Is_Open = 1;
		if ((DWORD)access == 2) {
			this->Length = 0;
		}
		return this->IsOpen();
	};

	virtual bool Open2(const char* pFileName, FileAccessMode access) override { return this->Open1(access); }
	virtual int Read(void* buffer, int length) override;
	virtual off_t Seek(off_t offset, FileSeekMode whence) override;
	virtual off_t Size() override { return this->Length; }
	virtual int Write(void* buffer, int length) override;
	virtual void Close() override { this->Is_Open = false; }
	virtual LONG GetDataTime() override { return 0; }
	virtual bool SetDateTime(LONG date_time) override { return 1; }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { }

	//Constructor
	RAMFileClass(void* pData, size_t nSize)
		: FileClass()
	{
		//VTable::Set(this, vtable);
		if (!pData && nSize > 0) {
			this->Buffer = (char*)YRMemory::Allocate(nSize);
			this->IsAllocated = 1;
		}
	}

	OPTIONALINLINE void SetManualBufer(char* pBuffer, int len, int offs = 0)
	{
		if (pBuffer) {
			this->Buffer = pBuffer;
			this->MaxLength = len;
			this->Length = len;
			this->Offset = offs;
		}
	}

protected:
	explicit __forceinline RAMFileClass(noinit_t)
		: FileClass(noinit_t())
	{ }

private:
	char* Buffer;
	int MaxLength;
	int Length;
	int Offset;
	FileAccessMode Access;
	bool Is_Open;
	bool IsAllocated;
};

static_assert(sizeof(RAMFileClass) == 0x20, "Invalid size.");
