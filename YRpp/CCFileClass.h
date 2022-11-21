#pragma once

#include <Base/Always.h>
#include <GeneralDefinitions.h>
#include <YRAllocator.h>
#include <YRPPCore.h>
#include <GenericList.h>
#include <array>

enum class FileAccessMode : unsigned int {
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = 3
};

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
	PERM = EPERM,                // Operation not permitted.
	NOENT = ENOENT,              // No such file or directory.
	SRCH = ESRCH,                // No such process.
	INTR = EINTR,                // Interrupted function call.
	IO = EIO,                    // Input/output error.
	NXIO = ENXIO,                // No such device or address.
	TOOBIG = E2BIG,                // Argument list too long.
	NOEXEC = ENOEXEC,            // Exec format error.
	BADF = EBADF,                // Bad file descriptor.
	CHILD = ECHILD,              // No child processes.
	AGAIN = EAGAIN,              // Resource temporarily unavailable.
	NOMEM = ENOMEM,              // Not enough space/cannot allocate memory.
	ACCES = EACCES,              // Permission denied.
	FAULT = EFAULT,              // Bad address.
	BUSY = EBUSY,                // Device or resource busy.
	EXIST = EEXIST,              // File exists.
	XDEV = EXDEV,                // Improper link.
	NODEV = ENODEV,              // No such device.
	NOTDIR = ENOTDIR,            // Not a directory.
	ISDIR = EISDIR,              // Is a directory.
	INVAL = EINVAL,              // Invalid argument.
	NFILE = ENFILE,              // Too many open files in system.
	MFILE = EMFILE,              // Too many open files.
	NOTTY = ENOTTY,              // Inappropriate I/O control operation.
	FBIG = EFBIG,                // File too large.
	NOSPC = ENOSPC,              // No space left on device.
	SPIPE = ESPIPE,              // Invalid seek.
	ROFS = EROFS,                // Read-only filesystem.
	MLINK = EMLINK,              // Too many links.
	PIPE = EPIPE,                // Broken pipe.
	DOM = EDOM,                  // Mathematics argument out of domain of function.
	RANGE = ERANGE,              // Result too large.
	DEADLK = EDEADLK,            // Resource deadlock avoided.
	NAMETOOLONG = ENAMETOOLONG,  // Filename too long.
	NOLCK = ENOLCK,              // No locks available.
	NOSYS = ENOSYS,              // Function not implemented.
	NOTEMPTY = ENOTEMPTY,        // Directory not empty.
	ILSEQ = EILSEQ,              // Invalid or incomplete multibyte or wide character.
};

//--------------------------------------------------------------------
//Abstract File class
//--------------------------------------------------------------------
class NOVTABLE FileClass
{
public:
	static const char* const FileErrorToString[];

	//Destructor
	virtual	~FileClass() RX;
	//FileClass
	virtual const char* GetFileName() const = 0;
	virtual const char* SetFileName(const char* pFileName) = 0;
	virtual BOOL CreateFile() = 0;
	virtual BOOL DeleteFile() = 0;
	virtual bool Exists(bool writeShared = false) = 0;
	virtual bool HasHandle() = 0;
	virtual bool Open(FileAccessMode access) = 0;
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) = 0;
	virtual int ReadBytes(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes read.
	virtual off_t Seek(off_t offset, FileSeekMode seek) = 0;
	virtual off_t GetFileSize() = 0;
	virtual int WriteBytes(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes written.
	virtual void Close() = 0;
	virtual LONG GetFileTime() = 0; //LoWORD = FatTime, HiWORD = FatDate
	virtual bool SetFileTime(LONG FileTime) = 0;
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) = 0;

	static void* __fastcall ReadWholeFile(FileClass* pFile)
	{ JMP_STD(0x4A3890); }

	void* ReadWholeFile()
	{ return ReadWholeFile(this); }

	off_t Tell() { return Seek(0, FileSeekMode::Current); }

	template <typename T>
	bool Read(T& obj, int size = sizeof(T)) {
		return this->ReadBytes(&obj, size) == size;
	}

	template <typename T>
	bool Write(T& obj, int size = sizeof(T)) {
		return this->WriteBytes(&obj, size) == size;
	}

	operator const char* () { return GetFileName(); }

	static const char* File_Error_To_String(FileErrorType error) {
		if(static_cast<int>(error) >= 42 || static_cast<int>(error) < 0)
			return "Unknown error. ";
		else
			return FileErrorToString[static_cast<int>(error)];
	}

	FileClass() { }

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
class NOVTABLE RawFileClass : public FileClass
{
public:
	//Destructor
	virtual ~RawFileClass() {JMP_THIS(0x65CA00);}

	//FileClass
	virtual const char* GetFileName() const override { JMP_THIS(0x401940); }
	virtual const char* SetFileName(const char* pFileName) override {JMP_THIS(0x65CAC0);}
	virtual BOOL CreateFile() override { JMP_THIS(0x65D150); }
	virtual BOOL DeleteFile() override { JMP_THIS(0x65D190); }
	virtual bool Exists(bool writeShared = false) override { JMP_THIS(0x65CBF0); }
	virtual bool HasHandle() override { JMP_THIS(0x65D420); }
	virtual bool Open(FileAccessMode access) override {JMP_THIS(0x65CB50);}
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) override {JMP_THIS(0x65CB30);}
	virtual int ReadBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x65CCE0); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { JMP_THIS(0x65CF00); }
	virtual off_t GetFileSize() override { JMP_THIS(0x65D0D0); }
	virtual int WriteBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x65CDD0); }
	virtual void Close() override { JMP_THIS(0x65CCA0); }
	virtual LONG GetFileTime() override { JMP_THIS(0x65D1F0); }
	virtual bool SetFileTime(LONG date_time) override { JMP_THIS(0x65D240); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override RX;

	void Bias(off_t start, int length = -1)
	{ JMP_THIS(0x65D2B0); }

	HANDLE Get_File_Handle() { return Handle; }
	int Transfer_Block_Size() { return (int)((unsigned)UINT_MAX) - 16L; }
	const char* Get_Safe_File_Name() const { return (FileName != nullptr && FileName[0] != '\0') ? FileName : "<unknown>"; }

	//Constructor
	RawFileClass(const char* pFileName)
		: RawFileClass(noinit_t())
	{ JMP_THIS(0x65CA80); }

	RawFileClass() :
		FileClass(noinit_t()),
		FileAccess(FileAccessMode::Read),
		FilePointer(0),
		FileSize(-1),
		Handle(INVALID_HANDLE_VALUE),
		FileName(nullptr),
		Date(0),
		Time(0),
		FileNameAllocated(false)
	{ *((unsigned long*)this) = (unsigned long)0x7F0904; }

protected:
	explicit __forceinline RawFileClass(noinit_t)
		: FileClass(noinit_t())
	{ }

	DWORD Raw_Seek(int nPos, FileSeekMode whence = FileSeekMode::Current) { JMP_THIS(0x65D320); }

	//Properties

public:
	FileAccessMode FileAccess;
	int FilePointer;
	int FileSize;
	HANDLE Handle;
	char* FileName;
	WORD Date;
	WORD Time;
	bool FileNameAllocated;
};
static_assert(sizeof(RawFileClass) == 0x24, "Invalid size.");
//--------------------------------------------------------------------
//Files that get buffered in some way?
//--------------------------------------------------------------------
class NOVTABLE BufferIOFileClass : public RawFileClass
{
public:
	static constexpr int MinimumBufferSize = 1024;
	//Destructor
	virtual ~BufferIOFileClass() { JMP_THIS(0x431B80); }

	//FileClass
	virtual const char* GetFileName() const override { return ((RawFileClass*)(this))->GetFileName(); }
	virtual const char* SetFileName(const char* pFileName) override { JMP_THIS(0x431E80); }
	virtual BOOL CreateFile() override { return ((RawFileClass*)(this))->CreateFile(); }
	virtual BOOL DeleteFile() override { return ((RawFileClass*)(this))->DeleteFile(); }
	virtual bool Exists(bool writeShared = false) override { JMP_THIS(0x431F10); }
	virtual bool HasHandle() override { JMP_THIS(0x431F30); }
	virtual bool Open(FileAccessMode access) override { JMP_THIS(0x431F70); }
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) override { JMP_THIS(0x431F50); }
	virtual int ReadBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x4322A0); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { JMP_THIS(0x4324B0); }
	virtual off_t GetFileSize() override { JMP_THIS(0x4325A0); }
	virtual int WriteBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x432050); }
	virtual void Close() override { JMP_THIS(0x4325C0); }
	virtual LONG GetFileTime() override { return ((RawFileClass*)(this))->GetFileTime(); }
	virtual bool SetFileTime(LONG date_time) override { return ((RawFileClass*)(this))->SetFileTime(date_time); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { ((RawFileClass*)(this))->Error(error, can_retry, filename); };

	bool Cache(int size, void* pBuffer)
	{ JMP_THIS(0x431BC0); }

	void Free()
	{ JMP_THIS(0x431D90); }

	bool Commit()
	{ JMP_THIS(0x431DD0); }

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

	//Properties

public:
	bool IsAllocated;
	bool IsOpen;
	bool IsDiskOpen;
	bool IsCached;
	bool IsChanged;
	bool UseBuffer;
	int BufferRights;
	void* BufferPtr;
	long BufferSize;
	long BufferPos;
	long BufferFilePos;
	long BufferChangeBeg;
	long BufferChangeEnd;
	long FileSize;
	long FilePos;
	long TrueFileStart;
};
static_assert(sizeof(BufferIOFileClass) == 0x54, "Invalid size.");
//--------------------------------------------------------------------
//Files on a CD?
//--------------------------------------------------------------------
class NOVTABLE CDFileClass : public BufferIOFileClass
{
public:
	//Destructor
	virtual ~CDFileClass() { JMP_THIS(0x535A60); }
	//FileClass
	virtual const char* GetFileName() const override { return ((BufferIOFileClass*)(this))->GetFileName(); }
	virtual const char* SetFileName(const char* pFileName) override { JMP_THIS(0x47AE10); }
	virtual BOOL CreateFile() override { return ((BufferIOFileClass*)(this))->CreateFile(); }
	virtual BOOL DeleteFile() override { return ((BufferIOFileClass*)(this))->DeleteFile(); }
	virtual bool Exists(bool writeShared = false) override { return ((BufferIOFileClass*)(this))->Exists(writeShared); }
	virtual bool HasHandle() override { return ((BufferIOFileClass*)(this))->HasHandle(); }
	virtual bool Open(FileAccessMode access) override { JMP_THIS(0x47AAB0); }
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) override { JMP_THIS(0x47AF10); }
	virtual int ReadBytes(void* pBuffer, int nNumBytes) override { return ((BufferIOFileClass*)(this))->ReadBytes(pBuffer, nNumBytes); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { return ((BufferIOFileClass*)(this))->Seek(offset, seek); }
	virtual off_t GetFileSize() override { return ((BufferIOFileClass*)(this))->GetFileSize(); }
	virtual int WriteBytes(void* pBuffer, int nNumBytes) override { return ((BufferIOFileClass*)(this))->WriteBytes(pBuffer, nNumBytes); }
	virtual void Close() override { ((BufferIOFileClass*)(this))->Close(); }
	virtual LONG GetFileTime() override { return ((BufferIOFileClass*)(this))->GetFileTime(); }
	virtual bool SetFileTime(LONG date_time) override { return ((BufferIOFileClass*)(this))->SetFileTime(date_time); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { ((BufferIOFileClass*)(this))->Error(error, can_retry, filename); }

	void Searching(int on) { IsDisabled = !on; }

	static void __fastcall Refresh()
	{ JMP_STD(0x47AAC0); }

	static bool __fastcall SetPath(const char* pPath)
	{ JMP_STD(0x47AB10); }

	static void __fastcall AddPath(const char* pPath)
	{ JMP_STD(0x47AD50); }

	static void __fastcall SetCDDrive(int nDriveNumber)
	{ JMP_STD(0x47ADA0); }

	static HRESULT __fastcall Clear()
	{ JMP_STD(0x47ADA0); }

	//Constructor
	CDFileClass()
		: CDFileClass(noinit_t())
	{ JMP_THIS(0x47AA30); }

	CDFileClass(const char* pFilename)
		: CDFileClass(noinit_t())
	{ JMP_THIS(0x47A9D0); }

	CDFileClass(wchar_t* pWideFilename)
		: CDFileClass(noinit_t())
	{ JMP_THIS(0x47AA00); }

protected:
	explicit __forceinline CDFileClass(noinit_t)
		: BufferIOFileClass(noinit_t())
	{ }

	//Property

public:
	bool IsDisabled; //54
};
static_assert(sizeof(CDFileClass) == 0x58, "Invalid size.");
//--------------------------------------------------------------------
//Files in MIXes
//--------------------------------------------------------------------
class NOVTABLE CCFileClass : public CDFileClass
{
public:
	//Destructor
	virtual ~CCFileClass() { JMP_THIS(0x535A70); }

	//FileClass
	virtual const char* GetFileName() const override { return ((CDFileClass*)(this))->GetFileName(); }
	virtual const char* SetFileName(const char* pFileName) override { JMP_THIS(0x473FC0); }
	virtual BOOL CreateFile() override { return ((CDFileClass*)(this))->CreateFile(); }
	virtual BOOL DeleteFile() override { return ((CDFileClass*)(this))->DeleteFile(); }
	virtual bool Exists(bool writeShared = false) override { JMP_THIS(0x473C50); }
	virtual bool HasHandle() override { JMP_THIS(0x473CD0); }
	virtual bool Open(FileAccessMode access) override { JMP_THIS(0x473D10); }
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) override { JMP_THIS(0x401980); }
	virtual int ReadBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x473B10); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { JMP_THIS(0x473BA0); }
	virtual off_t GetFileSize() override { JMP_THIS(0x473C00); }
	virtual int WriteBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x473AE0); }
	virtual void Close() override { JMP_THIS(0x473CE0); }
	virtual LONG GetFileTime() override { JMP_THIS(0x473E50); }
	virtual bool SetFileTime(LONG date_time) override { JMP_THIS(0x473F00); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { JMP_THIS(0x473AB0); }

	static void* Load_Alloc_Data(FileClass& file)
	{
		void* ptr = 0;
		long size = file.GetFileSize();

		ptr = new char[size];
		if (ptr)
		{
			file.Read(ptr, size);
		}

		return ptr;
	}

	static void* Load_Alloc_Data(char const* name, int flags)
	{
		CCFileClass file(name);
		return Load_Alloc_Data(file);
	}

	//Constructor
	CCFileClass(const char* pFileName)
		: CCFileClass(noinit_t())
	{ JMP_THIS(0x4739F0); }

	CCFileClass()
		: CCFileClass(noinit_t())
	{ JMP_THIS(0x473A80); }

	CCFileClass(wchar_t* pWideName)
		: CCFileClass(noinit_t())
	{ JMP_THIS(0x473A30); }

protected:
	explicit __forceinline CCFileClass(noinit_t)
		: CDFileClass(noinit_t())
	{ }

	//Properties
public:
	MemoryBuffer Buffer;
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
class NOVTABLE RAMFileClass : public FileClass
{
public:
	virtual ~RAMFileClass() { JMP_THIS(0x65C2A0); }

	virtual const char* GetFileName() const override { JMP_THIS(0x65C550); }
	virtual const char* SetFileName(const char* pFileName) override { JMP_THIS(0x65C560); }
	virtual BOOL CreateFile() override { JMP_THIS(0x65C2E0); }
	virtual BOOL DeleteFile() override { JMP_THIS(0x65C300); }
	virtual bool Exists(bool writeShared = false) override { JMP_THIS(0x65C320); }
	virtual bool HasHandle() override { JMP_THIS(0x65C330); }
	virtual bool Open(FileAccessMode access) override { JMP_THIS(0x65C350); };
	virtual bool OpenEx(const char* pFileName, FileAccessMode access) override { JMP_THIS(0x65C340); }
	virtual int ReadBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x65C3A0); }
	virtual off_t Seek(off_t offset, FileSeekMode seek) override { JMP_THIS(0x65C420); }
	virtual off_t GetFileSize() override { JMP_THIS(0x65C4A0); }
	virtual int WriteBytes(void* pBuffer, int nNumBytes) override { JMP_THIS(0x65C4B0); }
	virtual void Close() override { JMP_THIS(0x65C540); }
	virtual LONG GetFileTime() override { JMP_THIS(0x65C570); }
	virtual bool SetFileTime(LONG date_time) override { JMP_THIS(0x65C580); }
	virtual void Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr) override { JMP_THIS(0x65C590); }

	//Constructor
	RAMFileClass(void* pData, size_t nSize)
		: RAMFileClass(noinit_t())
	{ JMP_THIS(0x65C250); }

protected:
	explicit __forceinline RAMFileClass(noinit_t)
		: FileClass(noinit_t())
	{ }

private:
	char* Buffer;
	int MaxLength;
	int Length;
	int Offset;
	FileAccessType Access;
	bool IsOpen;
	bool IsAllocated;
};

static_assert(sizeof(RAMFileClass) == 0x20, "Invalid size.");
