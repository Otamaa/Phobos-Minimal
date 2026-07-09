#include "Phobos.FileClass.h"
#include <Utilities/Debug.h>

#include <Phobos.MIXFile.h>

// ============================================================
//  PhobosRawFileClass
//
//  SetFileName: plain std::string assign — no strdup, no Allocated flag.
//  Open():      Bias seek happens on success, not on handle == INVALID.
//  Read():      Bias-length clamp preserved. StreamerIsCurrentlyAccessing
//               mirrors vanilla global (needed for transition period).
//  Seek():      Bias window clamping preserved exactly from vanilla.
//  Write():     Bias window expansion preserved.
//  Error():     Logs via Debug only — no vanilla error dialog, no CD.
// ============================================================

// Vanilla global reference — still needed during the transition period.
// VERIFY: 0xB04BEC matches game binary.
bool StreamerIsCurrentlyAccessing;

const char* const PhobosFileClass::ErrorStrings[] =
{
		 "Non-error. "
	   , "Operation not permitted. "
	   , "No such file or directory. "
	   , "No such process. "
	   , "Interrupted function call. "
	   , "Input/output error. "
	   , "No such device or address. "
	   , "Argument list too long. "
	   , "Exec format error. "
	   , "Bad file descriptor. "
	   , "No child processes. "
	   , "Resource temporarily unavailable. "
	   , "Not enough space/cannot allocate memory. "
	   , "Permission denied. "
	   , "Bad address. "
	   , "Unknown error 15. "
	   , "Device or resource busy. "
	   , "File exists. "
	   , "Improper link. "
	   , "No such device. "
	   , "Not a directory. "
	   , "Is a directory. "
	   , "Invalid argument. "
	   , "Too many open files in system. "
	   , "Too many open files. "
	   , "Unknown error 26. "
	   , "Inappropriate I/O control operation. "
	   , "File too large. "
	   , "No space left on device. "
	   , "Invalid seek. "
	   , "Read-only filesystem. "
	   , "Too many links. "
	   , "Broken pipe. "
	   , "Mathematics argument out of domain of function. "
	   , "Result too large. "
	   , "Unknown error 36. "
	   , "Resource deadlock avoided. "
	   , "Filename too long. "
	   , "No locks available. "
	   , "Function not implemented. "
	   , "Directory not empty. "
	   , "Invalid or incomplete multibyte or wide character. "
};

std::vector<PhobosFileClass*> PhobosFileClass::Array;

PhobosFileClass::~PhobosFileClass()
{
	auto iter = std::ranges::find(Array, this);

	if (iter != Array.end())
		Array.erase(iter);
}

PhobosFileClass::PhobosFileClass() {
	this->Array.emplace_back(this);
}

PhobosRawFileClass::PhobosRawFileClass(const char* pFileName)
{
	if (pFileName)
		Filename = pFileName;
}

PhobosRawFileClass::~PhobosRawFileClass()
{
	if (IsOpen())
		CloseHandle(Handle);
	Handle = INVALID_HANDLE_VALUE;
}

// ---- SetFileName ----

const char* PhobosRawFileClass::SetFileName(const char* pFileName)
{
	if (!pFileName)
	{
		Filename.clear();
		return nullptr;
	}

	BiasStart = 0;
	BiasLength = -1;
	Filename = pFileName;
	return Filename.c_str();
}

// ---- Create / Delete ----

BOOL PhobosRawFileClass::Create()
{
	Close();
	if (Open(PhobosFileAccessMode::Write))
	{
		if (BiasLength != -1)
			Seek(0, PhobosFileSeekMode::Set);
		Close();
		return TRUE;
	}
	return FALSE;
}

BOOL PhobosRawFileClass::Delete()
{
	Close();

	if (Filename.empty())
	{
		Error(PhobosFileErrorType::NOENT);
		return FALSE;
	}

	if (!IsAvailable())
		return FALSE;

	if (DeleteFileA(Filename.c_str()))
		return TRUE;

	RaiseLastError();
	return FALSE;
}

// ---- IsAvailable ----

bool PhobosRawFileClass::IsAvailable(bool writeShared)
{
	if (Filename.empty())
		return false;

	if (IsOpen())
		return true;

	if (!writeShared)
	{
		HANDLE h = CreateFileA(Filename.c_str(), 0x80000000, 1u, 0, 3u, 0x80u, 0);
		if (h == INVALID_HANDLE_VALUE)
			return false;
		CloseHandle(h);
		return true;
	}

	// writeShared: attempt a real open+close.
	Open(PhobosFileAccessMode::Read);
	if (IsOpen())
	{
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
		return true;
	}
	return true; // vanilla returns true here even on failure — preserved
}

// ---- Open ----

bool PhobosRawFileClass::Open(PhobosFileAccessMode access)
{
	Close();

	if (Filename.empty())
	{
		Error(PhobosFileErrorType::NOENT, false, nullptr);
		return false;
	}

	Access = access;
	Handle = INVALID_HANDLE_VALUE;

	switch (access)
	{
	case PhobosFileAccessMode::Read:
		Handle = CreateFileA(Filename.c_str(), 0x80000000u, 3u, 0, 3u, 0x8000080u, 0);
		break;
	case PhobosFileAccessMode::Write:
		Handle = CreateFileA(Filename.c_str(), 0x40000000u, 0, 0, 2u, 0x80u, 0);
		break;
	case PhobosFileAccessMode::ReadWrite:
		Handle = CreateFileA(Filename.c_str(), 0xC0000000u, 0, 0, 4u, 0x80u, 0);
		break;
	default:
		break;
	}

	if (Handle != INVALID_HANDLE_VALUE && (BiasStart || BiasLength != -1))
		Seek(0, PhobosFileSeekMode::Set);

	return Handle != INVALID_HANDLE_VALUE;
}

// ---- Read ----

int PhobosRawFileClass::Read(void* pBuffer, int nBytes)
{
	bool opened = false;

	if (!IsOpen())
	{
		if (!Open(PhobosFileAccessMode::Read))
			return 0;
		opened = true;
	}

	int remaining = nBytes;
	if (BiasLength != -1)
	{
		const int cur = static_cast<int>(Seek(0, PhobosFileSeekMode::Current));
		const int avail = BiasLength - cur;
		if (remaining > avail)
			remaining = avail;
	}

	DWORD total = 0;
	while (remaining > 0)
	{
		DWORD got = 0;
		if (ReadFile(Handle, pBuffer, remaining, &got, nullptr))
		{
			pBuffer = static_cast<char*>(pBuffer) + got;
			remaining -= static_cast<int>(got);
			total += got;
			if (!got)
				break;
		}
		else
		{
			total += got;
			if (StreamerIsCurrentlyAccessing)
				break;
			RaiseLastError();
		}
	}

	StreamerIsCurrentlyAccessing = false;

	if (opened)
		Close();

	return static_cast<int>(total);
}

// ---- Seek ----

off_t PhobosRawFileClass::Seek(off_t offset, PhobosFileSeekMode whence)
{
	// No-bias fast path.
	if (BiasLength == -1)
	{
		if (!IsOpen())
		{
			Error(PhobosFileErrorType::BADF, false, Filename.c_str());
			return 0;
		}

		const DWORD result = SetFilePointer(Handle, static_cast<LONG>(offset), nullptr, static_cast<DWORD>(whence));
		if (result == INVALID_SET_FILE_POINTER)
		{
			RaiseLastError();
			return 0;
		}
		return static_cast<off_t>(result);
	}

	// Biased path — translate (offset, whence) to absolute position.
	LONG  adjOffset = static_cast<LONG>(offset);
	DWORD adjWhence = static_cast<DWORD>(whence);

	switch (whence)
	{
	case PhobosFileSeekMode::Set:
	{
		const LONG clamped = (adjOffset > BiasLength) ? static_cast<LONG>(BiasLength) : adjOffset;
		adjOffset = BiasStart + clamped;
		adjWhence = static_cast<DWORD>(PhobosFileSeekMode::Set);
		break;
	}
	case PhobosFileSeekMode::End:
		adjOffset = BiasLength + BiasStart + adjOffset;
		adjWhence = static_cast<DWORD>(PhobosFileSeekMode::Set);
		break;
	case PhobosFileSeekMode::Current:
		// Leave adjOffset/adjWhence as-is.
		break;
	}

	if (!IsOpen())
	{
		Error(PhobosFileErrorType::BADF, false, Filename.c_str());
		return 0;
	}

	DWORD ptr = SetFilePointer(Handle, adjOffset, nullptr, adjWhence);
	if (ptr == INVALID_SET_FILE_POINTER)
	{
		RaiseLastError();
		ptr = 0;
	}

	// Clamp: must be >= BiasStart.
	LONG relative = static_cast<LONG>(ptr) - BiasStart;
	if (relative < 0)
	{
		ptr = SetFilePointer(Handle, BiasStart, nullptr, static_cast<DWORD>(PhobosFileSeekMode::Set));
		relative = (ptr == INVALID_SET_FILE_POINTER) ? 0 : static_cast<LONG>(ptr) - BiasStart;
	}

	// Clamp: must be <= BiasLength.
	if (relative > BiasLength)
	{
		const LONG clampTarget = static_cast<LONG>(BiasLength) + BiasStart;
		DWORD      clamped = SetFilePointer(Handle, clampTarget, nullptr, static_cast<DWORD>(PhobosFileSeekMode::Set));
		if (clamped == INVALID_SET_FILE_POINTER)
		{
			RaiseLastError();
			clamped = 0;
		}
		relative = static_cast<LONG>(clamped) - BiasStart;
	}

	return static_cast<off_t>(relative);
}

// ---- Size ----

off_t PhobosRawFileClass::Size()
{
	if (BiasLength != -1)
		return static_cast<off_t>(BiasLength);

	if (IsOpen())
	{
		const DWORD sz = GetFileSize(Handle, nullptr);
		if (sz == INVALID_FILE_SIZE)
		{
			RaiseLastError();
			return 0;
		}
		BiasLength = static_cast<int>(sz) - BiasStart;
		return static_cast<off_t>(BiasLength);
	}

	if (Open(PhobosFileAccessMode::Read))
	{
		const off_t sz = Size();
		Close();
		return sz;
	}

	return 0;
}

// ---- Write ----

int PhobosRawFileClass::Write(void* buffer, int length)
{
	bool opened = false;

	if (!IsOpen())
	{
		if (!Open(PhobosFileAccessMode::Write))
			return 0;
		opened = true;
	}

	DWORD written = 0;
	if (!WriteFile(Handle, buffer, static_cast<DWORD>(length), &written, nullptr))
		RaiseLastError();

	// Expand BiasLength if write extended past the current window.
	if (BiasLength != -1)
	{
		if (!IsOpen())
		{
			Error(PhobosFileErrorType::BADF, false, Filename.c_str());
		}
		else
		{
			const DWORD pos = SetFilePointer(Handle, 0, nullptr, FILE_CURRENT);
			if (pos == INVALID_SET_FILE_POINTER)
				RaiseLastError();
			else if (pos > static_cast<DWORD>(BiasLength + BiasStart))
				BiasLength = static_cast<int>(pos) - BiasStart;
		}
	}

	if (opened)
		Close();

	return static_cast<int>(written);
}

// ---- Close ----

void PhobosRawFileClass::Close()
{
	if (IsOpen())
	{
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
	}
}

// ---- GetDateTime / SetDateTime ----

LONG PhobosRawFileClass::GetDateTime()
{
	if (Handle == INVALID_HANDLE_VALUE)
		return 0;

	BY_HANDLE_FILE_INFORMATION fi {};
	if (!GetFileInformationByHandle(Handle, &fi))
		return 0;

	WORD date = 0, time = 0;
	FileTimeToDosDateTime(&fi.ftLastWriteTime, &date, &time);
	return static_cast<LONG>((static_cast<DWORD>(date) << 16) | static_cast<DWORD>(time));
}

bool PhobosRawFileClass::SetDateTime(LONG fatTime)
{
	if (Handle == INVALID_HANDLE_VALUE)
		return false;

	BY_HANDLE_FILE_INFORMATION fi {};
	FILETIME ft {};
	if (!GetFileInformationByHandle(Handle, &fi))
		return false;
	if (!DosDateTimeToFileTime(HIWORD(fatTime), LOWORD(fatTime), &ft))
		return false;
	return !!SetFileTime(Handle, &fi.ftCreationTime, &ft, &ft);
}

// ---- Bias ----

void PhobosRawFileClass::Bias(off_t offset, int length)
{
	if (offset == 0)
	{
		BiasStart = 0;
		BiasLength = -1;
		return;
	}

	int size = static_cast<int>(PhobosRawFileClass::Size());
	BiasStart += static_cast<int>(offset);
	BiasLength = size;

	if (length != -1 && size > length)
		BiasLength = length;

	if (BiasLength < 0)
		BiasLength = 0;

	if (IsOpen())
		PhobosRawFileClass::Seek(0, PhobosFileSeekMode::Set);
}

// ---- Error ----

void PhobosRawFileClass::Error(PhobosFileErrorType error, bool /*canRetry*/, const char* fn)
{
	const char* name = (fn && fn[0]) ? fn : SafeFileName();
	Debug::Log("PhobosRawFileClass: error %d (%s) on file '%s'\n",
			   static_cast<int>(error),
			   ErrorToString(error),
			   name);
}

// ---- RawSeek (private helper) ----

DWORD PhobosRawFileClass::RawSeek(int pos, DWORD dir)
{
	if (!IsOpen())
	{
		Error(PhobosFileErrorType::BADF, false, Filename.c_str());
		return 0;
	}

	const DWORD result = SetFilePointer(Handle, pos, nullptr, dir);
	if (result == INVALID_SET_FILE_POINTER)
	{
		RaiseLastError();
		return 0;
	}
	return result;
}


// ============================================================
//  PhobosBufferIOFileClass
//
//  Buffer is std::vector<uint8_t> — no YRMemory, no IsAllocated flag.
//  Cache():    resizes vector instead of YRMemory::Allocate.
//  FreeCache(): clears vector (vector dtor handles memory).
//  Commit():   writes dirty range to disk via raw layer.
//  All buffer pointer arithmetic uses Buffer.data() + offset.
// ============================================================

PhobosBufferIOFileClass::PhobosBufferIOFileClass(const char* pFilename)
{
	if (pFilename)
		Filename = pFilename;
}

PhobosBufferIOFileClass::~PhobosBufferIOFileClass()
{
	FreeCache();
}

// ---- SetFileName ----

const char* PhobosBufferIOFileClass::SetFileName(const char* pFileName)
{
	if (!pFileName)
	{
		Filename.clear();
		return nullptr;
	}

	// If buffered and filename differs — flush before switching.
	if (UseBuffer && !Filename.empty() && Filename != pFileName)
	{
		Commit();
		IsCached = false;
	}

	PhobosRawFileClass::SetFileName(pFileName);
	return Filename.c_str();
}

// ---- IsAvailable ----

bool PhobosBufferIOFileClass::IsAvailable(bool writeShared)
{
	return UseBuffer ? true : PhobosRawFileClass::IsAvailable(writeShared);
}

// ---- IsOpen ----

bool PhobosBufferIOFileClass::IsOpen()
{
	if (Is_Open && UseBuffer)
		return true;
	return Handle != INVALID_HANDLE_VALUE;
}

// ---- Open ----

bool PhobosBufferIOFileClass::Open(PhobosFileAccessMode rights)
{
	auto resetBufferState = [this]() -> bool
		{
			BufferPos = 0;
			BufferFilePos = 0;
			BufferChangeBeg = -1;
			BufferChangeEnd = -1;
			FilePos = 0;
			Is_Open = true;
			return true;
		};

	if (!UseBuffer)
	{
		PhobosRawFileClass::Close();
		return PhobosRawFileClass::Open(rights);
	}

	Commit();

	if (IsDiskOpen)
	{
		if (TrueFileStart)
		{
			UseBuffer = false;
			Close();
			UseBuffer = true;
			IsDiskOpen = false;
			Is_Open = false;
		}
		else
		{
			PhobosRawFileClass::Close();
			IsDiskOpen = false;
			Is_Open = false;
		}
	}
	else
	{
		Is_Open = false;
	}

	BufferRights = static_cast<int>(rights);
	int effectiveRights = static_cast<int>(rights);

	if (effectiveRights == static_cast<int>(PhobosFileAccessMode::Read))
	{
		// Whole file fits in buffer — no disk open needed.
		if (FileSize <= BufferedSize)
		{
			IsDiskOpen = false;
			return resetBufferState();
		}
	}
	else if (effectiveRights == static_cast<int>(PhobosFileAccessMode::Write))
	{
		// Truncate: create/truncate via raw, then reopen R/W.
		PhobosRawFileClass::Open(PhobosFileAccessMode::Write);
		PhobosRawFileClass::Close();
		effectiveRights = static_cast<int>(PhobosFileAccessMode::ReadWrite);
		TrueFileStart = 0;
	}

	if (TrueFileStart)
	{
		UseBuffer = false;
		Open(static_cast<PhobosFileAccessMode>(effectiveRights));
		UseBuffer = true;
	}
	else
	{
		PhobosRawFileClass::Open(static_cast<PhobosFileAccessMode>(effectiveRights));
	}

	IsDiskOpen = true;

	if (BufferRights == static_cast<int>(PhobosFileAccessMode::Write))
		FileSize = 0;

	return resetBufferState();
}

// ---- Read ----

int PhobosBufferIOFileClass::Read(void* outBuffer, int length)
{
	bool opened = false;

	if (!IsOpen())
	{
		if (Open(PhobosFileAccessMode::Read))
		{
			TrueFileStart = static_cast<long>(PhobosRawFileClass::Seek(0, PhobosFileSeekMode::Current));
			opened = true;
		}
	}

	auto finish = [this, &opened](int total) -> int
		{
			if (opened)
				Close();
			return total;
		};

	if (!UseBuffer)
		return finish(PhobosRawFileClass::Read(outBuffer, length));

	if (BufferRights == static_cast<int>(PhobosFileAccessMode::Write))
	{
		Error(PhobosFileErrorType::ACCES, false, nullptr);
		return finish(0);
	}

	if (!length)
		return finish(0);

	auto fillBufferWindow = [this](long filePos)
		{
			const int readSize = (FileSize < BufferedSize) ? FileSize : static_cast<int>(BufferedSize);
			if (TrueFileStart)
			{
				UseBuffer = false;
				Seek(filePos, PhobosFileSeekMode::Current);
				Read(Buffer.data(), BufferedSize);
				Seek(FilePos, PhobosFileSeekMode::Current);
				UseBuffer = true;
			}
			else
			{
				PhobosRawFileClass::Seek(filePos, PhobosFileSeekMode::Current);
				PhobosRawFileClass::Read(Buffer.data(), readSize);
			}
		};

	int   total = 0;
	int   remaining = length;

	while (remaining > 0)
	{
		int chunk = static_cast<int>(BufferedSize) - static_cast<int>(BufferPos);
		if (remaining < chunk)
			chunk = remaining;

		if (!IsCached)
		{
			long readSize;
			if (FileSize >= BufferedSize)
			{
				readSize = BufferedSize;
				BufferFilePos = FilePos;
			}
			else
			{
				readSize = FileSize;
				BufferFilePos = 0;
			}

			if (TrueFileStart)
			{
				UseBuffer = false;
				Seek(FilePos, PhobosFileSeekMode::Current);
				Read(Buffer.data(), static_cast<int>(BufferedSize));
				Seek(FilePos, PhobosFileSeekMode::Current);
				UseBuffer = true;
			}
			else
			{
				PhobosRawFileClass::Seek(BufferFilePos, PhobosFileSeekMode::Current);
				PhobosRawFileClass::Read(Buffer.data(), static_cast<int>(readSize));
			}

			BufferPos = 0;
			BufferChangeBeg = -1;
			BufferChangeEnd = -1;
			IsCached = true;
		}

		std::memcpy(static_cast<char*>(outBuffer) + total,
					Buffer.data() + BufferPos,
					static_cast<size_t>(chunk));

		const long newBufPos = BufferPos + chunk;
		total += chunk;
		remaining -= chunk;

		FilePos = BufferFilePos + newBufPos;
		BufferPos = newBufPos;

		if (newBufPos == BufferedSize)
		{
			Commit();
			const long next = FilePos;
			BufferPos = 0;
			BufferFilePos = next;
			BufferChangeBeg = -1;
			BufferChangeEnd = -1;

			if (remaining > 0 && FileSize > next)
				fillBufferWindow(next);
			else
				IsCached = false;
		}
	}

	return finish(total);
}

// ---- Seek ----

off_t PhobosBufferIOFileClass::Seek(off_t offset, PhobosFileSeekMode whence)
{
	if (!UseBuffer)
		return PhobosRawFileClass::Seek(offset, whence);

	switch (whence)
	{
	case PhobosFileSeekMode::Set: FilePos = 0;        break;
	case PhobosFileSeekMode::End: FilePos = FileSize;  break;
	case PhobosFileSeekMode::Current: break;
	}

	// Adjust if offset looks like it already includes TrueFileStart.
	bool  adjusted = false;
	off_t adjOffset = offset;

	if (TrueFileStart && offset >= static_cast<off_t>(TrueFileStart))
	{
		adjOffset = offset - static_cast<off_t>(TrueFileStart);
		adjusted = true;
	}

	long newPos = FilePos + static_cast<long>(adjOffset);
	if (newPos < 0)      newPos = 0;
	if (newPos > FileSize) newPos = FileSize;

	FilePos = newPos;

	if (FileSize > BufferedSize)
	{
		const long windowStart = BufferFilePos;
		const long windowEnd = windowStart + BufferedSize;

		if (newPos < windowStart || newPos >= windowEnd)
		{
			Commit();
			if (TrueFileStart)
			{
				UseBuffer = false;
				Seek(FilePos, PhobosFileSeekMode::Set);
				UseBuffer = true;
			}
			else
			{
				PhobosRawFileClass::Seek(FilePos, PhobosFileSeekMode::Set);
			}
			IsCached = false;
		}
		else
		{
			BufferPos = newPos - windowStart;
		}
	}
	else
	{
		BufferPos = FilePos;
	}

	if (TrueFileStart && adjusted)
		return static_cast<off_t>(TrueFileStart + FilePos);

	return static_cast<off_t>(FilePos);
}

// ---- Size ----

off_t PhobosBufferIOFileClass::Size()
{
	if (Is_Open && UseBuffer)
		return static_cast<off_t>(FileSize);
	return PhobosRawFileClass::Size();
}

// ---- Write ----

int PhobosBufferIOFileClass::Write(void* inBuffer, int size)
{
	bool opened = false;

	if (!IsOpen())
	{
		if (!Open(PhobosFileAccessMode::Write))
			return 0;
		TrueFileStart = static_cast<long>(PhobosRawFileClass::Seek(0, PhobosFileSeekMode::Current));
		opened = true;
	}

	auto finish = [this, &opened](int total) -> int
		{
			if (opened)
				Close();
			return total;
		};

	if (!UseBuffer)
		return finish(PhobosRawFileClass::Write(inBuffer, size));

	if (BufferRights == static_cast<int>(PhobosFileAccessMode::Read))
	{
		Error(PhobosFileErrorType::ACCES, false, nullptr);
		return finish(0);
	}

	if (!size)
		return finish(0);

	auto fillBufferWindow = [this](long filePos)
		{
			const int readSize = (FileSize < BufferedSize) ? FileSize : static_cast<int>(BufferedSize);
			if (TrueFileStart)
			{
				UseBuffer = false;
				Seek(filePos, PhobosFileSeekMode::Set);
				Read(Buffer.data(), static_cast<int>(BufferedSize));
				Seek(FilePos, PhobosFileSeekMode::Set);
				UseBuffer = true;
			}
			else
			{
				PhobosRawFileClass::Seek(filePos, PhobosFileSeekMode::Set);
				PhobosRawFileClass::Read(Buffer.data(), readSize);
			}
		};

	int written = 0;
	int remaining = size;

	while (remaining > 0)
	{
		const int buffSize = static_cast<int>(BufferedSize);
		int chunk = buffSize - static_cast<int>(BufferPos);
		if (remaining < chunk)
			chunk = remaining;

		// Partial write into stale buffer — refill first.
		if (chunk != buffSize && !IsCached)
		{
			long readSize;
			if (FileSize >= BufferedSize)
			{
				readSize = BufferedSize;
				BufferFilePos = FilePos;
			}
			else
			{
				readSize = FileSize;
				BufferFilePos = 0;
			}

			if (TrueFileStart)
			{
				UseBuffer = false;
				Seek(FilePos, PhobosFileSeekMode::Set);
				Read(Buffer.data(), static_cast<int>(BufferedSize));
				Seek(FilePos, PhobosFileSeekMode::Set);
				UseBuffer = true;
			}
			else
			{
				PhobosRawFileClass::Seek(BufferFilePos, PhobosFileSeekMode::Set);
				PhobosRawFileClass::Read(Buffer.data(), static_cast<int>(readSize));
			}

			BufferPos = 0;
			BufferChangeBeg = -1;
			BufferChangeEnd = -1;
			IsCached = true;
		}

		std::memcpy(Buffer.data() + BufferPos,
					static_cast<const char*>(inBuffer) + written,
					static_cast<size_t>(chunk));

		const long oldBufPos = BufferPos;
		written += chunk;
		remaining -= chunk;
		IsChanged = true;

		if (BufferChangeBeg == -1)
		{
			BufferChangeBeg = oldBufPos;
			BufferChangeEnd = oldBufPos;
		}
		else if (BufferChangeBeg > oldBufPos)
		{
			BufferChangeBeg = oldBufPos;
		}

		const long newBufPos = oldBufPos + chunk;
		BufferPos = newBufPos;

		if (BufferChangeEnd < newBufPos)
			BufferChangeEnd = newBufPos;

		const long newFilePos = newBufPos + BufferFilePos;
		FilePos = newFilePos;
		if (FileSize < newFilePos)
			FileSize = newFilePos;

		if (newBufPos == BufferedSize)
		{
			Commit();
			const long next = FilePos;
			BufferPos = 0;
			BufferFilePos = next;
			BufferChangeBeg = -1;
			BufferChangeEnd = -1;

			if (remaining > 0 && FileSize > next)
				fillBufferWindow(next);
			else
				IsCached = false;
		}
	}

	return finish(written);
}

// ---- Close ----

void PhobosBufferIOFileClass::Close()
{
	if (UseBuffer)
	{
		Commit();

		if (IsDiskOpen)
		{
			if (TrueFileStart)
			{
				UseBuffer = false;
				Close();
				UseBuffer = true;
				IsDiskOpen = false;
				Is_Open = false;
				return;
			}
			PhobosRawFileClass::Close();
			IsDiskOpen = false;
		}

		Is_Open = false;
	}
	else
	{
		PhobosRawFileClass::Close();
	}
}

// ---- Cache ----

bool PhobosBufferIOFileClass::Cache(int size)
{
	if (!Buffer.empty())
		return (size == 0);  // already cached; only valid if no override requested

	FileSize = IsAvailable() ? static_cast<int>(PhobosRawFileClass::Size()) : 0;

	int resolvedSize = size;

	if (size > 0)
	{
		if (size < MinimumBufferSize)
			resolvedSize = MinimumBufferSize;
		BufferedSize = resolvedSize;
	}
	else
	{
		// No explicit size — cache the whole file.
		BufferedSize = FileSize;
		if (BufferedSize == 0)
			return false;
	}

	if (BufferedSize == 0)
		return false;

	Buffer.resize(static_cast<size_t>(BufferedSize));

	// Initialise state.
	IsDiskOpen = false;
	BufferPos = 0;
	BufferFilePos = 0;
	BufferChangeBeg = -1;
	BufferChangeEnd = -1;
	FilePos = 0;
	TrueFileStart = 0;

	if (FileSize == 0)
	{
		UseBuffer = true;
		return true;
	}

	const int readSize = (FileSize > BufferedSize) ? static_cast<int>(BufferedSize) : FileSize;
	bool      opened = false;
	long      startPos = 0;

	if (!IsOpen())
	{
		if (Open(PhobosFileAccessMode::Read))
		{
			TrueFileStart = static_cast<long>(PhobosRawFileClass::Seek(0, PhobosFileSeekMode::Current));
			opened = true;
		}
	}
	else
	{
		startPos = static_cast<long>(Seek(0, PhobosFileSeekMode::Current));

		if (Handle == INVALID_HANDLE_VALUE)
			TrueFileStart = startPos;
		else
			TrueFileStart = static_cast<long>(PhobosRawFileClass::Seek(0, PhobosFileSeekMode::Current));

		if (FileSize > BufferedSize)
		{
			BufferFilePos = startPos;
			FilePos = startPos;
		}
		else
		{
			if (startPos)
				Seek(0, PhobosFileSeekMode::Set);
			BufferPos = startPos;
			FilePos = startPos;
		}
	}

	if (static_cast<int>(Read(Buffer.data(), readSize)) != readSize)
		Error(PhobosFileErrorType::IO, false, nullptr);

	if (opened)
	{
		Close();
		IsCached = true;
		UseBuffer = true;
		return true;
	}

	Seek(startPos, PhobosFileSeekMode::Set);
	IsCached = true;
	UseBuffer = true;
	return true;
}

// ---- FreeCache ----

void PhobosBufferIOFileClass::FreeCache()
{
	Buffer.clear();
	Buffer.shrink_to_fit();
	BufferedSize = 0;
	Is_Open = false;
	IsCached = false;
	IsChanged = false;
	UseBuffer = false;
}

// ---- Commit ----

bool PhobosBufferIOFileClass::Commit()
{
	if (!UseBuffer || !IsChanged)
		return false;

	const int writeLen = static_cast<int>(BufferChangeEnd) - static_cast<int>(BufferChangeBeg);
	const long seekPos = static_cast<long>(BufferChangeBeg) + TrueFileStart + BufferFilePos;

	if (IsDiskOpen)
	{
		PhobosRawFileClass::Seek(seekPos, PhobosFileSeekMode::Set);
		PhobosRawFileClass::Write(Buffer.data() + BufferChangeBeg, writeLen);
		PhobosRawFileClass::Seek(TrueFileStart + FilePos, PhobosFileSeekMode::Set);
	}
	else
	{
		PhobosRawFileClass::Open(PhobosFileAccessMode::ReadWrite);
		PhobosRawFileClass::Seek(seekPos, PhobosFileSeekMode::Set);
		PhobosRawFileClass::Write(Buffer.data() + BufferChangeBeg, writeLen);
		PhobosRawFileClass::Close();
	}

	IsChanged = false;
	return true;
}

// ============================================================
//  PhobosRAMFileClass
//
//  No disk handles, no buffer layer — pure memory.
//  OwnedBuffer is the default storage (std::vector<uint8_t>).
//  ExternalBuffer mode wraps caller-owned memory without copying.
// ============================================================

PhobosRAMFileClass::PhobosRAMFileClass(size_t capacity)
{
	OwnedBuffer.resize(capacity);
}

PhobosRAMFileClass::PhobosRAMFileClass(const void* pData, size_t nSize)
{
	if (pData && nSize > 0)
	{
		const auto* src = static_cast<const uint8_t*>(pData);
		OwnedBuffer.assign(src, src + nSize);
	}
	else if (nSize > 0)
	{
		OwnedBuffer.resize(nSize);
	}

	this->Filename = "UNKNOWN";
}

PhobosRAMFileClass::PhobosRAMFileClass(const char* pName, const void* pData, size_t nSize)
{
	if (pData && nSize > 0)
	{
		const auto* src = static_cast<const uint8_t*>(pData);
		OwnedBuffer.assign(src, src + nSize);
	}
	else if (nSize > 0)
	{
		OwnedBuffer.resize(nSize);
	}

	this->Filename = pName ? pName : "UNKNOWN";
}

void PhobosRAMFileClass::SetManualBuffer(void* pBuffer, int len, int initialOffset)
{
	if (!pBuffer || len <= 0)
		return;

	ExternalBuffer = pBuffer;
	ExternalSize = len;
	Cursor = initialOffset;
}

// ---- Private helpers ----

const uint8_t* PhobosRAMFileClass::ActiveData() const
{
	return ExternalBuffer
		? static_cast<const uint8_t*>(ExternalBuffer)
		: OwnedBuffer.data();
}

uint8_t* PhobosRAMFileClass::ActiveData()
{
	return ExternalBuffer
		? static_cast<uint8_t*>(ExternalBuffer)
		: OwnedBuffer.data();
}

int PhobosRAMFileClass::ActiveSize() const
{
	return ExternalBuffer
		? ExternalSize
		: static_cast<int>(OwnedBuffer.size());
}

int PhobosRAMFileClass::WriteCeiling() const
{
	// Write mode may address up to ExternalSize / vector capacity.
	// Read mode is clamped to ActiveSize().
	return ExternalBuffer ? ExternalSize : static_cast<int>(OwnedBuffer.capacity());
}

// ---- Create / Delete ----

BOOL PhobosRAMFileClass::Create()
{
	if (IsOpenFlag)
		return FALSE;

	OwnedBuffer.clear();
	ExternalBuffer = nullptr;
	ExternalSize = 0;
	Cursor = 0;
	return TRUE;
}

BOOL PhobosRAMFileClass::Delete()
{
	if (IsOpenFlag)
		return FALSE;

	OwnedBuffer.clear();
	ExternalBuffer = nullptr;
	ExternalSize = 0;
	Cursor = 0;
	return TRUE;
}

// ---- Size ----

off_t PhobosRAMFileClass::Size()
{
	return static_cast<off_t>(ActiveSize());
}

// ---- Open ----

bool PhobosRAMFileClass::Open(PhobosFileAccessMode access)
{
	const bool hasData = !OwnedBuffer.empty() || ExternalBuffer;
	if (!hasData || IsOpenFlag)
		return false;

	Cursor = 0;
	RAMAccess = access;
	IsOpenFlag = true;

	if (access == PhobosFileAccessMode::Write)
	{
		OwnedBuffer.clear();
		ExternalBuffer = nullptr;
		ExternalSize = 0;
	}

	return true;
}

// ---- Read ----

int PhobosRAMFileClass::Read(void* buffer, int length)
{
	if (!buffer || length <= 0)
		return 0;

	if (!ActiveData() && OwnedBuffer.empty())
		return 0;

	bool opened = false;
	if (!IsOpenFlag)
	{
		Open(PhobosFileAccessMode::Read);
		opened = true;
	}
	else if ((static_cast<unsigned>(RAMAccess) & 1u) == 0)
	{
		return 0; // no read access
	}

	const int avail = ActiveSize() - Cursor;
	const int readLen = (length < avail) ? length : avail;

	if (readLen > 0)
	{
		std::memcpy(buffer, ActiveData() + Cursor, static_cast<size_t>(readLen));
		Cursor += readLen;
	}

	if (opened)
		Close();

	return readLen;
}

// ---- Seek ----

off_t PhobosRAMFileClass::Seek(off_t offset, PhobosFileSeekMode whence)
{
	if (!IsOpenFlag)
		return static_cast<off_t>(Cursor);

	const bool canWrite = (static_cast<unsigned>(RAMAccess) & 2u) != 0;
	const int  ceiling = canWrite ? WriteCeiling() : ActiveSize();

	switch (whence)
	{
	case PhobosFileSeekMode::Set:     Cursor = static_cast<int>(offset);           break;
	case PhobosFileSeekMode::Current: Cursor += static_cast<int>(offset);           break;
	case PhobosFileSeekMode::End:     Cursor = ceiling + static_cast<int>(offset); break;
	}

	if (Cursor < 0)       Cursor = 0;
	if (Cursor > ceiling) Cursor = ceiling;

	// Grow owned buffer if write-seek moved past current size.
	if (!ExternalBuffer && Cursor > static_cast<int>(OwnedBuffer.size()))
		OwnedBuffer.resize(static_cast<size_t>(Cursor));

	return static_cast<off_t>(Cursor);
}

// ---- Write ----

int PhobosRAMFileClass::Write(void* buffer, int length)
{
	if (!buffer || length <= 0)
		return 0;

	bool opened = false;
	if (!IsOpenFlag)
	{
		Open(PhobosFileAccessMode::Write);
		opened = true;
	}
	else if ((static_cast<unsigned>(RAMAccess) & 2u) == 0)
	{
		return 0; // no write access
	}

	int writeLen = length;

	if (ExternalBuffer)
	{
		const int avail = ExternalSize - Cursor;
		if (writeLen > avail)
			writeLen = avail;
		std::memcpy(static_cast<uint8_t*>(ExternalBuffer) + Cursor, buffer, static_cast<size_t>(writeLen));
	}
	else
	{
		const int needed = Cursor + writeLen;
		if (needed > static_cast<int>(OwnedBuffer.size()))
			OwnedBuffer.resize(static_cast<size_t>(needed));
		std::memcpy(OwnedBuffer.data() + Cursor, buffer, static_cast<size_t>(writeLen));
	}

	Cursor += writeLen;

	if (opened)
		Close();

	return writeLen;
}


// ============================================================
//  PhobosCCFileClass
//
//  Resolves files in order: ZIP (TODO) → Mix → Raw disk → RAM cache.
//  CD layer removed entirely.
//  CachedData (std::vector<uint8_t>) replaces vanilla MemoryBuffer.
//  DiskFile (PhobosRawFileClass member) handles on-disk mix/raw access.
// ============================================================

PhobosCCFileClass::PhobosCCFileClass(const char* pFileName)
{
	if (pFileName)
		Filename = pFileName;
}

PhobosCCFileClass::~PhobosCCFileClass()
{
	Close();
}

// ---- FileName / SetFileName ----

const char* PhobosCCFileClass::FileName() const
{
	return Filename.c_str();
}

const char* PhobosCCFileClass::SetFileName(const char* pFileName)
{
	Availability = 0;
	CachedData.clear();
	Cursor = 0;
	Filename = pFileName ? pFileName : "";
	return Filename.c_str();
}

// ---- IsAvailable ----

bool PhobosCCFileClass::IsAvailable(bool writeShared)
{
	if (Availability == 1)
		return true;

	if (IsOpen())
	{
		Availability = 1;
		return true;
	}

	// TODO: check ZIP archive first (ZipBackedFileClass — not yet implemented)

	if (PhobosMixFileClass::Offset(Filename.c_str(), nullptr, nullptr, nullptr, nullptr))
	{
		Availability = 1;
		return true;
	}

	// Fall back to raw disk.
	DiskFile.SetFileName(Filename.c_str());
	if (DiskFile.IsAvailable(writeShared))
	{
		Availability = 1;
		return true;
	}

	Availability = 2;
	return false;
}

// ---- IsOpen ----

bool PhobosCCFileClass::IsOpen()
{
	return HasCachedData() || DiskFileOpen;
}

// ---- Open ----

bool PhobosCCFileClass::Open(PhobosFileAccessMode rights)
{
    Close();
 
    // Write → raw disk only; mix/cache are read-only.
    if ((static_cast<unsigned>(rights) & static_cast<unsigned>(FileAccessMode::Write)) != 0)
        return OpenRawDisk(rights);
 
    // TODO: check ZIP archive (ZipBackedFileClass — not yet implemented)
 
	PhobosMixFileClass* mixFile = nullptr;
    void*         pointer = nullptr;
    int           start   = 0;
    int           length  = 0;
 
    if (!PhobosMixFileClass::Offset(Filename.c_str(), &pointer, &mixFile, &start, &length))
        return OpenRawDisk(rights);
 
    if (pointer)
        return LoadFromMixRAM(pointer, length);
 
    if (mixFile)
        return OpenMixOnDisk(mixFile, start, length);
 
    return OpenRawDisk(rights);
}

bool PhobosCCFileClass::Open(const char* pFileName, PhobosFileAccessMode access)
{
	SetFileName(pFileName);
	return Open(access);
}

// ---- Private helpers ----

bool PhobosCCFileClass::LoadFromMixRAM(void* pointer, int length)
{
	if (!pointer || length <= 0)
		return false;

	const auto* src = static_cast<const uint8_t*>(pointer);
	CachedData.assign(src, src + length);
	Cursor = 0;
	return true;
}

bool PhobosCCFileClass::OpenRawDisk(PhobosFileAccessMode rights)
{
	DiskFile.SetFileName(Filename.c_str());
	if (!DiskFile.Open(rights))
		return false;
	DiskFileOpen = true;
	return true;
}

bool PhobosCCFileClass::OpenMixOnDisk(PhobosMixFileClass* mixFile, int start, int length)
{
	// DiskFile gets the mix container filename — this->Filename stays as the entry name.
	DiskFile.SetFileName(mixFile->Filename.c_str());
	if (!DiskFile.Open(PhobosFileAccessMode::Read))
		return false;

	DiskFile.Bias(start, length);
	DiskFile.Seek(0, PhobosFileSeekMode::Set);
	DiskFileOpen = true;
	return true;
}

// ---- Read ----

int PhobosCCFileClass::Read(void* buffer, int size)
{
	bool opened = false;
	if (!IsOpen() && Open(PhobosFileAccessMode::Read))
		opened = true;

	int result = 0;

	if (HasCachedData())
	{
		const DWORD remaining = static_cast<DWORD>(CachedData.size()) - Cursor;
		const DWORD readLen = (static_cast<DWORD>(size) < remaining) ? static_cast<DWORD>(size) : remaining;

		if (readLen > 0)
		{
			std::memcpy(buffer, CachedData.data() + Cursor, readLen);
			Cursor += readLen;
		}
		result = static_cast<int>(readLen);
	}
	else if (DiskFileOpen)
	{
		result = DiskFile.Read(buffer, size);
	}

	if (opened)
		Close();

	return result;
}

// ---- Seek ----

off_t PhobosCCFileClass::Seek(off_t pos, PhobosFileSeekMode dir)
{
	if (HasCachedData())
	{
		const DWORD sz = static_cast<DWORD>(CachedData.size());

		switch (dir)
		{
		case PhobosFileSeekMode::Set:     Cursor = 0;  break;
		case PhobosFileSeekMode::End:     Cursor = sz; break;
		case PhobosFileSeekMode::Current: break;
		}

		const long newPos = static_cast<long>(Cursor) + static_cast<long>(pos);
		Cursor = (newPos < 0) ? 0 : static_cast<DWORD>(newPos);
		if (Cursor > sz)
			Cursor = sz;

		return static_cast<off_t>(Cursor);
	}

	if (DiskFileOpen)
		return DiskFile.Seek(pos, dir);

	return 0;
}

// ---- Size ----

off_t PhobosCCFileClass::Size()
{
	if (HasCachedData())
		return static_cast<off_t>(CachedData.size());

	if (DiskFileOpen)
		return DiskFile.Size();

	// Not open — check disk availability.
	DiskFile.SetFileName(Filename.c_str());
	if (DiskFile.IsAvailable())
		return DiskFile.Size();

	// Query mix.
	int length = 0;
	PhobosMixFileClass::Offset(Filename.c_str(), nullptr, nullptr, nullptr, &length);
	return static_cast<off_t>(length);
}

// ---- Write ----

int PhobosCCFileClass::Write(void* buffer, int length)
{
	if (HasCachedData())
	{
		Error(PhobosFileErrorType::ACCES, false, Filename.c_str());
		return 0;
	}

	if (DiskFileOpen)
		return DiskFile.Write(buffer, length);

	return 0;
}

// ---- Close ----

void PhobosCCFileClass::Close()
{
	CachedData.clear();
	Cursor = 0;

	if (DiskFileOpen)
	{
		DiskFile.Close();
		DiskFileOpen = false;
	}
}

// ---- GetDateTime / SetDateTime ----

LONG PhobosCCFileClass::GetDateTime()
{
	// TODO: query ZIP archive timestamp first (ZipBackedFileClass — not yet implemented)

	if (DiskFileOpen)
		return DiskFile.GetDateTime();

	// Try raw disk.
	DiskFile.SetFileName(Filename.c_str());
	if (DiskFile.IsAvailable())
	{
		DiskFile.Open(PhobosFileAccessMode::Read);
		const LONG dt = DiskFile.GetDateTime();
		DiskFile.Close();
		return dt;
	}

	// Fall back to mix file's container timestamp.
	PhobosMixFileClass* mixFile = nullptr;
	if (!PhobosMixFileClass::Offset(Filename.c_str(), nullptr, &mixFile, nullptr, nullptr) || !mixFile)
		return 0;

	PhobosRawFileClass mixRaw(mixFile->Filename.c_str());
	mixRaw.Open(PhobosFileAccessMode::Read);
	const LONG dt = mixRaw.GetDateTime();
	mixRaw.Close();
	return dt;
}

bool PhobosCCFileClass::SetDateTime(LONG datetime)
{
	if (DiskFileOpen)
		return DiskFile.SetDateTime(datetime);

	DiskFile.SetFileName(Filename.c_str());
	if (DiskFile.IsAvailable())
	{
		DiskFile.Open(PhobosFileAccessMode::ReadWrite);
		const bool ok = DiskFile.SetDateTime(datetime);
		DiskFile.Close();
		return ok;
	}

	// Mix file entries are read-only — cannot set datetime on them.
	return false;
}
