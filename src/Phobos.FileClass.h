#pragma once

#include <GeneralDefinitions.h>
#include <memory>
#include <Base/Always.h>

#include <string>
#include <vector>

class PhobosMixFileClass;

// ============================================================
//  Enums
// ============================================================

enum class PhobosFileAccessMode : unsigned int
{
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = Read | Write
};
MAKE_ENUM_FLAGS(PhobosFileAccessMode);

enum class PhobosFileSeekMode : unsigned int
{
	Set = 0,
	Current = 1,
	End = 2
};

enum class PhobosFileErrorType : int
{
	ZERO = 0, PERM = 1, NOENT = 2, SRCH = 3, INTR = 4, IO = 5,
	NXIO = 6, TOOBIG = 7, NOEXEC = 8, BADF = 9, CHILD = 10,
	AGAIN = 11, NOMEM = 12, ACCES = 13, FAULT = 14, BUSY = 16,
	EXIST = 17, XDEV = 18, NODEV = 19, NOTDIR = 20, ISDIR = 21,
	INVAL = 22, NFILE = 23, MFILE = 24, NOTTY = 25, FBIG = 27,
	NOSPC = 28, SPIPE = 29, ROFS = 30, MLINK = 31, PIPE = 32,
	DOM = 33, RANGE = 34, DEADLK = 36, NAMETOOLONG = 38,
	NOLCK = 39, NOSYS = 40, NOTEMPTY = 41, ILSEQ = 42,
};

// ============================================================
//  PhobosFileClass  — abstract base
//
//  Owns Filename as std::string. No char* allocation/deallocation anywhere.
//  CD check removed entirely (intentional).
//  SkipCDCheck removed — no CD layer exists in this hierarchy.
// ============================================================
class PhobosFileClass
{
	static std::vector<PhobosFileClass*> Array;

public:
	static const char* const ErrorStrings[];

	virtual ~PhobosFileClass();

	virtual const char* FileName()  const = 0;
	virtual const char* SetFileName(const char* pFileName) = 0;
	virtual BOOL         Create() = 0;
	virtual BOOL         Delete() = 0;
	virtual bool         IsAvailable(bool writeShared = false) = 0;
	virtual bool         IsOpen() = 0;
	virtual bool         Open(PhobosFileAccessMode access) = 0;
	virtual bool         Open(const char* pFileName, PhobosFileAccessMode access) = 0;
	virtual int          Read(void* pBuffer, int nBytes) = 0;
	virtual off_t        Seek(off_t offset, PhobosFileSeekMode seek) = 0;
	virtual off_t        Size() = 0;
	virtual int          Write(void* pBuffer, int nBytes) = 0;
	virtual void         Close() = 0;
	virtual LONG         GetDateTime() = 0;
	virtual bool         SetDateTime(LONG fileTime) = 0;
	virtual void         Error(PhobosFileErrorType error,
								bool canRetry = false,
								const char* fn = nullptr) = 0;

	// Non-virtual helpers
	off_t  Tell() { return Seek(0, PhobosFileSeekMode::Current); }

	template<typename T>
	bool ReadBytes(T& obj, int size = sizeof(T)) { return Read(&obj, size) == size; }

	template<typename T>
	bool WriteBytes(T& obj, int size = sizeof(T)) { return Write(&obj, size) == size; }

	operator const char* () const { return FileName(); }

	static const char* ErrorToString(PhobosFileErrorType e)
	{
		const int i = static_cast<int>(e);
		return (i >= 0 && i <= 42) ? ErrorStrings[i] : "Unknown error.";
	}

	void RaiseLastError()
	{
		Error(static_cast<PhobosFileErrorType>(::GetLastError()), false, Filename.c_str());
	}

	// Data
	std::string    Filename;
	PhobosFileAccessMode Access = PhobosFileAccessMode::None;

protected:
	PhobosFileClass();
};


// ============================================================
//  PhobosRawFileClass  — Win32 HANDLE file
//
//  Changes vs vanilla RawFileClass:
//    - Filename: std::string (base) — no char* Filename, no Allocated flag, no CRT::strdup
//    - SetFileName: just assigns std::string, no heap management
//    - Rights renamed to Access (stored in base)
//    - BiasStart/BiasLength kept — bias logic unchanged
//    - Date/Time fields kept (used by GetDateTime/SetDateTime)
//    - StreamerIsCurrentlyAccessing: extern reference to vanilla global (still needed during transition)
//    - Error(): logs via Debug, does NOT call vanilla error dialog (no CD forced)
// ============================================================
class PhobosRawFileClass : public PhobosFileClass
{
public:
	virtual ~PhobosRawFileClass();

	virtual const char* FileName()  const override { return Filename.c_str(); }
	virtual const char* SetFileName(const char* pFileName) override;
	virtual BOOL        Create()    override;
	virtual BOOL        Delete()    override;
	virtual bool        IsAvailable(bool writeShared = false) override;
	virtual bool        IsOpen()    override { return Handle != INVALID_HANDLE_VALUE; }
	virtual bool        Open(PhobosFileAccessMode access) override;
	virtual bool        Open(const char* pFileName, PhobosFileAccessMode access) override
	{
		return SetFileName(pFileName) && Open(access);
	}
	virtual int         Read(void* pBuffer, int nBytes) override;
	virtual off_t       Seek(off_t offset, PhobosFileSeekMode whence) override;
	virtual off_t       Size() override;
	virtual int         Write(void* buffer, int length) override;
	virtual void        Close() override;
	virtual LONG        GetDateTime() override;
	virtual bool        SetDateTime(LONG fatTime) override;
	virtual void        Error(PhobosFileErrorType error, bool canRetry = false, const char* fn = nullptr) override;

	void         Bias(off_t offset, int length = -1);
	HANDLE       GetHandle() const { return Handle; }
	const char* SafeFileName() const { return Filename.empty() ? "<unknown>" : Filename.c_str(); }

	PhobosRawFileClass() = default;
	explicit PhobosRawFileClass(const char* pFileName);

protected:
	DWORD RawSeek(int pos, DWORD dir);

public:
	int    BiasStart = 0;
	int    BiasLength = -1;
	HANDLE Handle = INVALID_HANDLE_VALUE;
	WORD   Date = 0;
	WORD   Time = 0;
};


// ============================================================
//  PhobosBufferIOFileClass  — buffered layer over PhobosRawFileClass
//
//  Changes vs vanilla BufferIOFileClass:
//    - BufferPtr: std::vector<uint8_t> replaces void* + IsAllocated + YRMemory
//    - Cache()/Free() operate on the vector — no manual allocation
//    - IsAllocated flag removed — vector owns its memory
//    - All other state fields kept (names preserved for diffing)
//    - MinimumBufferSize kept
// ============================================================
class PhobosBufferIOFileClass : public PhobosRawFileClass
{
public:
	static constexpr int MinimumBufferSize = 1024;

	virtual ~PhobosBufferIOFileClass();

	virtual const char* FileName()  const override { return PhobosRawFileClass::FileName(); }
	virtual const char* SetFileName(const char* pFileName) override;
	virtual BOOL        Create()   override { return PhobosRawFileClass::Create(); }
	virtual BOOL        Delete()   override { return PhobosRawFileClass::Delete(); }
	virtual bool        IsAvailable(bool writeShared = false) override;
	virtual bool        IsOpen()   override;
	virtual bool        Open(PhobosFileAccessMode rights) override;
	virtual bool        Open(const char* pFileName, PhobosFileAccessMode access) override
	{
		return SetFileName(pFileName) && PhobosBufferIOFileClass::Open(access);
	}
	virtual int         Read(void* buffer, int length)  override;
	virtual off_t       Seek(off_t offset, PhobosFileSeekMode whence) override;
	virtual off_t       Size()   override;
	virtual int         Write(void* buffer, int size)   override;
	virtual void        Close()  override;
	virtual LONG        GetDateTime() override { return PhobosRawFileClass::GetDateTime(); }
	virtual bool        SetDateTime(LONG dt)   override { return PhobosRawFileClass::SetDateTime(dt); }
	virtual void        Error(PhobosFileErrorType e, bool r = false, const char* f = nullptr) override
	{
		PhobosRawFileClass::Error(e, r, f);
	}

	bool  Cache(int size = 0);  // size=0 → cache whole file; no external buffer arg needed
	void  FreeCache();
	bool  Commit();

	PhobosBufferIOFileClass() = default;
	explicit PhobosBufferIOFileClass(const char* pFilename);

private:
	// Returns pointer into the internal buffer at the current buffer position.
	uint8_t* BufferAt(long pos) { return Buffer.data() + pos; }

public:
	// Internal buffer — replaces void* BufferPtr + IsAllocated
	std::vector<uint8_t> Buffer;

	// State — names kept for vanilla parity
	bool  Is_Open = false;
	bool  IsDiskOpen = false;
	bool  IsCached = false;
	bool  IsChanged = false;
	bool  UseBuffer = false;
	int   BufferRights = 0;
	long  BufferedSize = 0;
	long  BufferPos = 0;
	long  BufferFilePos = 0;
	long  BufferChangeBeg = -1;
	long  BufferChangeEnd = -1;
	long  FileSize = 0;
	long  FilePos = 0;
	long  TrueFileStart = 0;
};


// ============================================================
//  PhobosRAMFileClass  — pure in-memory file
//
//  Inherits PhobosFileClass directly (mirrors vanilla RAMFileClass topology).
//  No raw handles, no buffer layer, no disk access.
//
//  Changes vs vanilla RAMFileClass:
//    - Storage: std::vector<uint8_t> — no char* Buffer + MaxLength + YRMemory
//    - IsAllocated flag removed — vector is always the owner
//    - External buffer mode (SetManualBuffer) kept for callers that wrap existing data
//    - FileName()/SetFileName() return "UNKNOWN" — same as vanilla
//    - Create()/Delete() reset the vector — same semantic as vanilla
// ============================================================
class PhobosRAMFileClass : public PhobosFileClass
{
public:
	virtual ~PhobosRAMFileClass() = default;

	virtual const char* FileName()  const override { return this->Filename.c_str(); }
	virtual const char* SetFileName(const char* pFileName) override { 
		if (pFileName)
			this->Filename = pFileName;
		else
			this->Filename = "UNKNOWN";

		return this->Filename.c_str(); }

	virtual BOOL        Create()    override;
	virtual BOOL        Delete()    override;
	virtual bool        IsAvailable(bool = false) override { return true; }
	virtual bool        IsOpen()    override { return IsOpenFlag; }
	virtual bool        Open(PhobosFileAccessMode access)  override;
	virtual bool        Open(const char*, PhobosFileAccessMode access) override { return Open(access); }
	virtual int         Read(void* buffer, int length)  override;
	virtual off_t       Seek(off_t offset, PhobosFileSeekMode whence) override;
	virtual off_t       Size()   override;
	virtual int         Write(void* buffer, int length) override;
	virtual void        Close()  override { IsOpenFlag = false; }
	virtual LONG        GetDateTime() override { return 0; }
	virtual bool        SetDateTime(LONG) override { return true; }
	virtual void        Error(PhobosFileErrorType, bool = false, const char* = nullptr) override {}

	// Construct with pre-allocated internal storage.
	explicit PhobosRAMFileClass(size_t capacity);

	// Construct by copying existing data into internal storage.
	PhobosRAMFileClass(const void* pData, size_t nSize);
	PhobosRAMFileClass(const char* pName, const void* pData, size_t nSize);

	// Wrap an external buffer not owned by this class.
	// Caller must ensure lifetime outlives this object.
	void SetManualBuffer(void* pBuffer, int len, int initialOffset = 0);

	PhobosRAMFileClass() = default;

private:
	const uint8_t* ActiveData()   const;
	uint8_t* ActiveData();
	int            ActiveSize()   const;
	int            WriteCeiling() const;

	std::vector<uint8_t> OwnedBuffer;   // used when ExternalBuffer == nullptr
	void* ExternalBuffer = nullptr;
	int                  ExternalSize = 0;

	PhobosFileAccessMode  RAMAccess = PhobosFileAccessMode::None;
	int             Cursor = 0;
	bool            IsOpenFlag = false;
};


// ============================================================
//  PhobosCCFileClass  — resource resolver
//
//  Lookup order (no CD layer — removed intentionally):
//    1. ZIP archive (ZipBackedFileClass — TODO: not yet implemented)
//    2. Mix file    (MixFileClass::Offset — vanilla lookup)
//    3. Raw disk    (PhobosRawFileClass)
//    4. RAM cache   (data stored in CachedData vector after first load)
//
//  Changes vs vanilla CCFileClass:
//    - No CDFileClass, no BufferIOFileClass in base chain — resolves directly
//    - Buffer/Position: replaced with std::vector<uint8_t> CachedData + Cursor
//    - MemoryBuffer / DECLARE_PROPERTY removed
//    - Availablility field kept (same 3-state enum semantic: 0=unknown,1=yes,2=no)
//    - Error() is a no-op (no CD forced — intentional)
// ============================================================
class PhobosCCFileClass : public PhobosFileClass
{
public:
	virtual ~PhobosCCFileClass();

	virtual const char* FileName()  const override;
	virtual const char* SetFileName(const char* pFileName) override;
	virtual BOOL        Create()   override { return 1; }
	virtual BOOL        Delete()   override { return 1; }
	virtual bool        IsAvailable(bool writeShared = false) override;
	virtual bool        IsOpen()   override;
	virtual bool        Open(PhobosFileAccessMode rights) override;
	virtual bool        Open(const char* pFileName, PhobosFileAccessMode access) override;
	virtual int         Read(void* buffer, int size)  override;
	virtual off_t       Seek(off_t pos, PhobosFileSeekMode dir)  override;
	virtual off_t       Size()  override;
	virtual int         Write(void* buffer, int length) override;
	virtual void        Close() override;
	virtual LONG        GetDateTime() override;
	virtual bool        SetDateTime(LONG datetime) override;
	virtual void        Error(PhobosFileErrorType, bool = false, const char* = nullptr) override {}

	PhobosCCFileClass() = default;
	explicit PhobosCCFileClass(const char* pFileName);

private:
	// Returns true if the file is cached in CachedData.
	bool HasCachedData() const { return !CachedData.empty(); }

	// Load from mix into CachedData. Returns true on success.
	bool LoadFromMixRAM(void* pointer, int length);

	bool OpenMixOnDisk(PhobosMixFileClass* mixFile, int start, int length);

	// Open the file on raw disk via PhobosRawFileClass.
	// Used when mix lookup fails and file exists on disk.
	bool OpenRawDisk(PhobosFileAccessMode rights);

public:
	std::vector<uint8_t> CachedData;     // replaces MemoryBuffer
	DWORD                Cursor = 0;
	DWORD                Availability = 0; // 0=unknown, 1=available, 2=unavailable

	// Raw disk handle — used when file is on disk, not in mix/cache.
	// Backed by a PhobosRawFileClass instance (owned).
	PhobosRawFileClass   DiskFile;
	bool                 DiskFileOpen = false;
};