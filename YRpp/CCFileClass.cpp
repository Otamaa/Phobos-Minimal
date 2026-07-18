#include "CCFileClass.h"

#include <GenericList.h>

#include <MixFileClass.h>
#include <CD.h>

#pragma region RawFileClass

RawFileClass::~RawFileClass()
{
	if (this->RawFileClass::IsOpen())
	{
		if (!CloseHandle(this->Handle))
		{
			this->RaiseLastError();
		}

		this->Handle = INVALID_HANDLE_VALUE;
	}

	if (this->Allocated && this->Filename)
	{
		YRMemory::Deallocate((void*)this->Filename);
		this->Filename = nullptr;
		this->Allocated = false;
	}
}

const char* RawFileClass::SetFileName(const char* pFileName)
{
	if (this->Filename && this->Allocated)
	{
		YRMemory::Deallocate((void*)this->Filename);
		this->Filename = nullptr;
		this->Allocated = false;
	}

	if (!pFileName)
		return nullptr;

	this->BiasStart = 0;
	this->BiasLength = -1;
	this->Filename = CRT::strdup(pFileName);
	if (this->Filename)
		this->Allocated = true;
	else {
		this->Error(FileErrorType::NOMEM, 0, pFileName);
		return nullptr;
	}

	return this->Filename;
}

BOOL RawFileClass::Create()
{
	this->Close();
	if (this->Open1(FileAccessMode::Write))
	{
		if (this->BiasLength != -1)
			this->Seek(0, FileSeekMode::Set);

		this->Close();
		return TRUE;
	}

	return FALSE;
}

BOOL RawFileClass::Delete()
{
	this->Close();
	if (this->Filename)
	{
		if (this->IsAvaible())
		{
			if (DeleteFileA(this->Filename))
			{
				return TRUE;
			}

			this->RaiseLastError();
			return FALSE;
		}
		else
		{
			return FALSE;
		}
	}

	this->Error(FileErrorType::NOENT);
	return FALSE;
}

bool RawFileClass::IsAvaible(bool writeShared)
{
	if (this->Filename)
	{
		if (this->IsOpen())
		{
			return TRUE;
		}

		if (!writeShared)
		{
			this->Handle = CreateFileA(this->Filename, 0x80000000, 1u, 0, 3u, 0x80u, 0);
			if (this->Handle == INVALID_HANDLE_VALUE)
				return FALSE;

			if (!CloseHandle(this->Handle))
			{
				this->RaiseLastError();
			}

			this->Handle = INVALID_HANDLE_VALUE;
			return TRUE;
		}

		this->RawFileClass::Open1(FileAccessMode::Read);
		if (this->IsOpen())
		{
			if (!CloseHandle(this->Handle))
			{
				this->RaiseLastError();
			}

			this->Handle = INVALID_HANDLE_VALUE;
			return TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

bool RawFileClass::Open1(FileAccessMode access)
{
	this->Close();

	if (auto v3 = this->Filename)
	{
		this->Rights = access;
		this->Handle = INVALID_HANDLE_VALUE;

		switch (access)
		{
		case FileAccessMode::Read:
			this->Handle = CreateFileA(v3, 0x80000000u, 3u, 0, 3u, 0x8000080u, 0);
			break;
		case FileAccessMode::Write:
			this->Handle = CreateFileA(v3, 0x40000000u, 0, 0, 2u, 0x80u, 0);
			break;
		case FileAccessMode::ReadWrite:
			this->Handle = CreateFileA(v3, 0xC0000000u, 0, 0, 4u, 0x80u, 0);
			break;
		default:
			break;
		}

		if (this->BiasStart || this->BiasLength != -1)
		{
			this->Seek(0, FileSeekMode::Set);
		}

		return this->Handle != INVALID_HANDLE_VALUE;
	}

	this->Error(FileErrorType::NOENT, false, this->Filename);
	return false;
}

int RawFileClass::Read(void* pBuffer, int nNumBytes)
{
	DWORD total = 0;
	DWORD bytesread = 0;
	bool opened = 0;
	int length_1 = 0;

	if (!this->IsOpen())
	{

		auto result = this->Open1(FileAccessMode::Read);

		if (!result)
		{
			return result;
		}

		opened = 1;
	}

	if (this->BiasLength == -1)
	{
		length_1 = nNumBytes;
	}
	else
	{
		int v6 = this->Seek(0, FileSeekMode::Current);
		length_1 = nNumBytes;
		if (nNumBytes >= this->BiasLength - v6)
		{
			length_1 = this->BiasLength - v6;
		}
	}

	while (length_1 > 0)
	{
		bytesread = 0;
		if (ReadFile(this->Handle, pBuffer, length_1, &bytesread, 0))
		{
			pBuffer = (char*)pBuffer + bytesread;
			length_1 -= bytesread;
			total += bytesread;

			if (!bytesread)
			{
				break;
			}
		}
		else
		{
			pBuffer = (char*)pBuffer + bytesread;
			length_1 -= bytesread;
			total += bytesread;
			DWORD nNumberOfBytesToReada = total;
			if (StreamerIsCurrentlyAccessing())
			{
				break;
			}
			this->RaiseLastError();
			total = nNumberOfBytesToReada;
		}
	}

	bytesread = total;
	StreamerIsCurrentlyAccessing = 0;

	if (opened)
	{
		this->Close();
	}

	return bytesread;
}

off_t RawFileClass::Seek(off_t offset, FileSeekMode whence)
{
	// Calls SetFilePointer and fires Error() on failure; returns 0 on failure.
	auto SeekHandle = [this](LONG offset, DWORD whence) -> DWORD
		{
			DWORD ptr = SetFilePointer(this->Handle, offset, 0, whence);

			if (ptr != -1)
				return ptr;

			this->RaiseLastError();
			return NULL;
		};

	// Seeks to `offset` from FILE_BEGIN, firing Error() on failure; returns 0 on failure.
	auto SeekAbsolute = [this, SeekHandle](LONG offset) -> DWORD
		{
			if (!this->IsOpen())
			{
				this->Error(FileErrorType::BADF, 0, this->Filename);
				return NULL;
			}

			return SeekHandle(offset, (DWORD)FileSeekMode::Set);
		};

	// -----------------------------------------------------------------------
	// RAW seek — no bias active
	// -----------------------------------------------------------------------
	if (this->BiasLength == -1)
	{
		if (!this->IsOpen())
		{
			this->Error(FileErrorType::BADF, 0, this->Filename);
			return NULL;
		}

		auto result = SetFilePointer(this->Handle,
			offset, 0, (DWORD)whence);

		if (result == -1)
		{
			this->RaiseLastError();
			return 0;
		}
		return result;
	}

	const int biasLength = this->BiasLength;

	// -----------------------------------------------------------------------
	// RAW seek — no bias active
	// -----------------------------------------------------------------------
	if (biasLength == -1)
	{
		if (!this->IsOpen())
		{
			this->Error(FileErrorType::BADF, 0, this->Filename);
			return 0;
		}

		auto result = SetFilePointer(this->Handle, offset, 0, (DWORD)whence);

		if (result == -1)
		{
			this->RaiseLastError();
			return 0;
		}

		return result;
	}

	// -----------------------------------------------------------------------
	// BIASED seek — clamp to [BiasStart, BiasStart + BiasLength]
	// -----------------------------------------------------------------------

	// Step 1: translate caller's (offset, whence) into an absolute file offset.
	LONG  adjustedOffset = offset;
	auto adjustedWhence = whence;

	if (whence == FileSeekMode::Set) // FILE_BEGIN — clamp to bias window
	{
		LONG clamped = (offset > biasLength) ? static_cast<LONG>(biasLength) : offset;
		adjustedOffset = this->BiasStart + clamped;
		adjustedWhence = FileSeekMode::Set;
	}
	else if (whence == FileSeekMode::End) // FILE_END — rewrite as absolute from bias end
	{
		adjustedOffset = biasLength + this->BiasStart + offset;
		adjustedWhence = FileSeekMode::Set;
	}
	// whence == 1 (FILE_CURRENT): offset unchanged, whence remapped below

	// Step 2: perform the seek.
	if (!this->IsOpen())
	{
		this->Error(FileErrorType::BADF, 0, this->Filename);
		// Fall through to clamp logic with ptr = 0 (matches original)
	}

	DWORD ptr = 0;
	if (this->IsOpen())
	{
		ptr = SeekHandle(adjustedOffset, (DWORD)adjustedWhence);
		// SeekHandle returns 0 and calls Error() on Win32 failure.
	}

	// Step 3: clamp — ensure result >= BiasStart.
	LONG biasStart = this->BiasStart;
	DWORD result = ptr - biasStart;

	if (result < 0)
	{
		// Seeked before the bias window; snap back to BiasStart.
		result = SeekAbsolute(biasStart) - biasStart;
	}

	// Step 4: clamp — ensure result <= BiasLength.
	if (result > (DWORD)biasLength)
	{
		LONG clampTarget = static_cast<LONG>(biasLength) + biasStart;

		if (!this->IsOpen())
		{
			this->Error(FileErrorType::BADF, 0, this->Filename);
			return -biasStart; // matches original fallback
		}

		DWORD clamped = SetFilePointer(this->Handle, clampTarget, 0, 0);

		if (clamped == -1)
		{
			this->RaiseLastError();
			clamped = 0;
		}

		result = clamped - this->BiasStart;
	}

	return result;
}

off_t RawFileClass::Size()
{
	if (this->BiasLength != -1)
		return this->BiasLength;

	int size = 0;
	auto result = this->BiasLength;

	if (this->Handle != INVALID_HANDLE_VALUE)
	{

	}
	if (result == -1)
	{
		if (this->IsOpen())
		{
			size = GetFileSize(this->Handle, 0);

			if (size == -1)
			{
				this->RaiseLastError();
				result = -1 - this->BiasStart;
				this->BiasLength = result;
				return result;
			}
		}
		else if (this->Open1(FileAccessMode::Read))
		{
			size = this->Size();
			this->Close();
		}

		result = size - this->BiasStart;
		this->BiasLength = result;
	}

	return result;
}

int RawFileClass::Write(void* buffer, int length)
{

	DWORD total = 0;
	bool  opened = false;

	// Auto-open if not already open (write mode = 2).
	if (!this->IsOpen())
	{
		if (!this->Open1(FileAccessMode::Write))
			return 0;

		opened = true;
	}

	// Perform the write; report but don't abort on failure (matches original).
	if (!WriteFile(this->Handle, buffer, length, &total, nullptr))
		this->RaiseLastError();

	// Bias update — only when a bias window is active.
	if (this->BiasLength != -1)
	{
		// Get current file pointer position (FILE_CURRENT seek by 0).
		DWORD pos = 0;
		if (this->IsOpen())
		{
			pos = SetFilePointer(this->Handle, 0, nullptr, FILE_CURRENT);

			if (pos == -1)
			{
				this->RaiseLastError();
				pos = 0;
			}
		}
		else
		{
			this->Error(FileErrorType::BADF, 0, this->Filename);
		}

		// Expand BiasLength if the write extended past the current window.
		if (pos > DWORD(this->BiasLength + this->BiasStart))
		{
			DWORD newPos = 0;

			if (this->IsOpen())
			{
				newPos = SetFilePointer(this->Handle, 0, nullptr, FILE_CURRENT);
				if (newPos == -1)
				{
					this->RaiseLastError();
					newPos = 0;
				}
			}
			else
			{
				this->Error(FileErrorType::BADF, 0, this->Filename);
			}

			this->BiasLength = newPos - this->BiasStart;
		}
	}

	if (opened)
		this->Close();

	return total;
}

void RawFileClass::Close()
{
	if (this->IsOpen())
	{
		if (!CloseHandle(this->Handle))
		{
			this->RaiseLastError();
		}
		this->Handle = INVALID_HANDLE_VALUE;
	}
}

LONG RawFileClass::GetDataTime()
{
	auto v3 = this->Handle;
	BY_HANDLE_FILE_INFORMATION FileInformation;
	WORD dosDate = 0;
	WORD fatTime = 0;

	if (v3 != INVALID_HANDLE_VALUE && GetFileInformationByHandle(v3, &FileInformation))
	{
		FileTimeToDosDateTime(&FileInformation.ftLastWriteTime, &dosDate, &fatTime);

		return static_cast<long>((static_cast<DWORD>(dosDate) << 16) | static_cast<DWORD>(fatTime));
	}

	return 0;
}

bool RawFileClass::SetDateTime(LONG wFatTime)
{
	auto v3 = this->Handle;
	BY_HANDLE_FILE_INFORMATION FileInformation;
	FILETIME FileTime;

	if (v3 != INVALID_HANDLE_VALUE && GetFileInformationByHandle(v3, &FileInformation) && DosDateTimeToFileTime(HIWORD(wFatTime), (WORD)wFatTime, &FileTime))
	{
		return SetFileTime(this->Handle, &FileInformation.ftCreationTime, &FileTime, &FileTime);
	}

	return 0;
}

void RawFileClass::Bias(off_t offset, int length)
{

	if (offset)
	{
		int size = this->RawFileClass::Size();
		auto v5 = this->BiasStart;
		this->BiasLength = size;
		this->BiasStart = offset + v5;
		if (length != -1)
		{
			if (size >= length)
			{
				size = length;
			}
			this->BiasLength = size;
		}

		this->BiasLength = this->BiasLength <= 0 ? 0 : this->BiasLength;

		if (this->IsOpen())
		{
			this->RawFileClass::Seek(0, FileSeekMode::Set);
		}
	}
	else
	{
		this->BiasStart = 0;
		this->BiasLength = -1;
	}
}

DWORD RawFileClass::Raw_Seek(int pos, LONG dir)
{
	DWORD result = 0;

	if (this->IsOpen())
	{
		if (dir)
		{
			if (dir == 1)
			{
				dir = 1;
			}
			else if (dir == 2)
			{
				dir = 2;
			}
		}
		else
		{
			dir = 0;
		}

		result = SetFilePointer(this->Handle, pos, 0, dir);

		if ((HANDLE)result == INVALID_HANDLE_VALUE)
		{
			this->RaiseLastError();
			result = 0;
		}
	}
	else
	{
		this->Error(FileErrorType::BADF, false, this->Filename);
	}

	return result;
}

#pragma endregion

#pragma region BufferIOFileClass

const char* BufferIOFileClass::SetFileName(const char* pFileName)
{
	auto thisFileName = this->FileName();

	if (thisFileName && this->UseBuffer) {
		if (!CRT::strcmp(pFileName, thisFileName)) {
			return thisFileName;
		}

		this->BufferIOFileClass::Commit();
		this->IsCached = false;
	}

	this->RawFileClass::SetFileName(pFileName);
	return this->FileName();
}

bool BufferIOFileClass::Open1(FileAccessMode rights)
{
	// Shared tail: reset buffer cursors and mark logically open.
	// Called from multiple points in the buffered path — replaces LABEL_19.
	auto resetBufferState = [this]() -> int
		{
			this->BufferPos = 0;
			this->BufferFilePos = 0;
			this->BufferChangeBeg = -1;
			this->BufferChangeEnd = -1;
			this->FilePos = 0;
			this->Is_Open = 1;
			return 1;
		};

	// ------------------------------------------------------------------
	// Non-buffered fast path
	// ------------------------------------------------------------------
	if (!this->UseBuffer)
	{
		this->RawFileClass::Close();
		return this->RawFileClass::Open1(rights);
	}

	// ------------------------------------------------------------------
	// Buffered path — commit any pending changes first
	// ------------------------------------------------------------------
	this->BufferIOFileClass::Commit();

	if (this->IsDiskOpen)
	{
		if (this->TrueFileStart)
		{
			// File is a sub-region; must close via vtable with UseBuffer
			// temporarily cleared so the raw close reaches the real handle.
			this->UseBuffer = 0;
			this->Close();
			this->UseBuffer = 1;
			this->IsDiskOpen = 0;
			this->Is_Open = 0;
		}
		else
		{
			this->RawFileClass::Close();
			this->IsDiskOpen = 0;
			this->Is_Open = 0;
		}
	}
	else
	{
		this->Is_Open = 0;
	}

	// ------------------------------------------------------------------
	// Open with requested rights
	// ------------------------------------------------------------------
	this->BufferRights = (int)rights;
	int rights_1 = (int)rights;

	// read
	if ((int)rights == 1)
	{
		// Entire file fits in buffer — serve entirely from memory.
		if (this->FileSize <= this->BufferedSize)
		{
			this->IsDiskOpen = 0;
			return resetBufferState();
		}
		// write 
	}
	else if ((int)rights == 2)
	{
		// Truncate: open+close to create/truncate the file, then reopen R/W.	
		this->RawFileClass::Open1(FileAccessMode::Write);
		this->RawFileClass::Close();
		rights_1 = 3; // reopen as read-write
		this->TrueFileStart = 0;
	}

	// Open the underlying raw file (honouring TrueFileStart sub-region).
	if (this->TrueFileStart)
	{
		// UseBuffer cleared so vtable dispatch reaches the real Open.
		this->UseBuffer = 0;
		this->Open1((FileAccessMode)rights_1);
		this->UseBuffer = 1;
	}
	else
	{
		this->RawFileClass::Open1((FileAccessMode)rights_1);
	}

	this->IsDiskOpen = 1;

	if (this->BufferRights == 2)
		this->FileSize = 0;

	return resetBufferState();
}

int BufferIOFileClass::Read(void* buffer, int length)
{
	bool opened = false;

	// Auto-open for reading if not already open.
	if (!this->IsOpen())
	{
		if (this->Open1(FileAccessMode::Read))
		{
			this->TrueFileStart = this->RawFileClass::Seek(0, FileSeekMode::Current);
			opened = true;
		}
	}

	// Shared close-on-exit tail (replaces LABEL_29).
	auto finish = [this, &opened](DWORD total) -> DWORD
		{
			if (opened)
				this->Close();

			return total;
		};

	// ------------------------------------------------------------------
	// Non-buffered fast path
	// ------------------------------------------------------------------
	if (!this->UseBuffer)
		return finish(this->RawFileClass::Read(buffer, length));

	// ------------------------------------------------------------------
	// Buffered path
	// ------------------------------------------------------------------

	// Write-only buffer — reading is an error.
	if (this->BufferRights == 2)
	{
		this->Error(FileErrorType::ACCES, 0, 0);
		return finish(0);
	}

	if (!length)
		return finish(0);

	// Helper: fill the internal buffer from disk at a given file position.
	// Two variants depending on whether TrueFileStart (sub-region) is active.
	auto fillBuffer = [this](int filePos)
		{
			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Seek(filePos, FileSeekMode::Current);
				this->Read(this->BufferPtr, this->BufferedSize);
				this->Seek(this->FilePos, FileSeekMode::Current);
				this->UseBuffer = 1;
			}
			else
			{
				this->RawFileClass::Seek(filePos, FileSeekMode::Current);
				this->RawFileClass::Read(this->BufferPtr, this->BufferedSize);
			}
		};

	DWORD sizeread = 0;
	int   length1 = length;

	do
	{
		// How many bytes can we copy from the current buffer window.
		int sizetoread = this->BufferedSize - this->BufferPos;
		if (length1 < sizetoread)
			sizetoread = length1;

		// Refill buffer from disk if the current window is stale.
		if (!this->IsCached)
		{
			int readSize;
			if (this->FileSize >= this->BufferedSize)
			{
				readSize = this->BufferedSize;
				this->BufferFilePos = this->FilePos;
			}
			else
			{
				readSize = this->FileSize;
				this->BufferFilePos = 0;
			}

			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Seek(this->FilePos, FileSeekMode::Current);
				this->Read(this->BufferPtr, this->BufferedSize);
				this->Seek(this->FilePos, FileSeekMode::Current);
				this->UseBuffer = 1;
			}
			else
			{
				this->RawFileClass::Seek(this->BufferFilePos, FileSeekMode::Current);
				this->RawFileClass::Read(this->BufferPtr, readSize);
			}

			this->BufferPos = 0;
			this->BufferChangeBeg = -1;
			this->BufferChangeEnd = -1;
			this->IsCached = 1;
		}

		// Copy from buffer into caller's output.
		memmove(static_cast<char*>(buffer) + sizeread,
				static_cast<char*>(this->BufferPtr) + this->BufferPos,
				sizetoread);

		const int movedPlusPos = sizetoread + this->BufferPos;
		sizeread += sizetoread;
		length1 -= sizetoread;

		this->FilePos = this->BufferFilePos + movedPlusPos;
		this->BufferPos = movedPlusPos;

		// Buffer window exhausted — commit and advance window.
		if (movedPlusPos == this->BufferedSize)
		{
			this->BufferIOFileClass::Commit();
			const LONG nextFilePos = this->FilePos;
			this->BufferPos = 0;
			this->BufferFilePos = nextFilePos;
			this->BufferChangeBeg = -1;
			this->BufferChangeEnd = -1;

			if (length1 && this->FileSize > nextFilePos)
			{
				// More data needed and more data exists — prefetch next window.
				fillBuffer(nextFilePos);
			}
			else
			{
				// Either done or hit EOF — mark buffer stale.
				this->IsCached = 0;
			}
		}
	}
	while (length1);

	return finish(sizeread);
}

off_t BufferIOFileClass::Seek(off_t offset, FileSeekMode whence)
{
	// ------------------------------------------------------------------
	// Non-buffered fast path
	// ------------------------------------------------------------------
	if (!this->UseBuffer)
		return this->RawFileClass::Seek(offset, whence);

	// ------------------------------------------------------------------
	// Resolve FilePos from whence
	// ------------------------------------------------------------------
	if ((int)whence == 0)
		this->FilePos = 0;
	else if ((int)whence == 2)
		this->FilePos = this->FileSize;
	// whence == 1 (FILE_CURRENT): FilePos unchanged

	// Adjust offset if it looks like an absolute position including TrueFileStart.
	bool   offsetAdjusted = false;
	auto offset1 = offset;
	const int trueStart = this->TrueFileStart;

	if (trueStart && offset >= trueStart)
	{
		offset1 = offset - trueStart;
		offsetAdjusted = true;
	}

	// Apply offset, clamp to [0, FileSize].
	auto newPos = this->FilePos + offset1;
	if (newPos < 0)
		newPos = 0;

	const int fileSize = this->FileSize;
	if (newPos > fileSize)
		newPos = fileSize;

	this->FilePos = newPos;

	// ------------------------------------------------------------------
	// Sync buffer window to new FilePos
	// ------------------------------------------------------------------
	const int buffSize = this->BufferedSize;

	if (fileSize > buffSize)
	{
		// File larger than buffer — check if FilePos is within current window.
		const int buffStart = this->BufferFilePos;

		if (newPos < buffStart || newPos >= buffStart + buffSize)
		{
			// Outside current window — commit, then seek underlying file.
			this->BufferIOFileClass::Commit();

			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Seek(this->FilePos, FileSeekMode::Set);
				this->UseBuffer = 1;
			}
			else
			{
				this->RawFileClass::Seek(this->FilePos, FileSeekMode::Set);
			}

			this->IsCached = 0;
		}
		else
		{
			// Within window — just update the in-buffer cursor.
			this->BufferPos = newPos - buffStart;
		}
	}
	else
	{
		// Entire file fits in buffer — BufferPos == FilePos.
		this->BufferPos = this->FilePos;
	}

	// ------------------------------------------------------------------
	// Return value: add TrueFileStart back if offset was adjusted.
	// ------------------------------------------------------------------
	if (this->TrueFileStart && offsetAdjusted)
		return this->TrueFileStart + this->FilePos;

	return this->FilePos;
}

int BufferIOFileClass::Write(void* buffer, int size)
{
	bool opened = false;

	// Auto-open for writing if not already open.
	if (!this->IsOpen())
	{
		if (!this->Open1(FileAccessMode::Write))
			return 0;

		this->TrueFileStart = this->RawFileClass::Seek(0, FileSeekMode::Current);
		opened = true;
	}

	// Shared close-on-exit tail (replaces LABEL_38).
	auto finish = [this, &opened](int total) -> int
		{
			if (opened)
				this->Close();

			return total;
		};

	// ------------------------------------------------------------------
	// Non-buffered fast path
	// ------------------------------------------------------------------
	if (!this->UseBuffer)
		return finish(this->RawFileClass::Write(buffer, size));

	// ------------------------------------------------------------------
	// Buffered path
	// ------------------------------------------------------------------

	// Read-only buffer — writing is an error.
	if (this->BufferRights == 1)
	{
		this->Error(FileErrorType::ACCES, 0, 0);
		return finish(0);
	}

	if (!size)
		return finish(0);

	// Helper: fill internal buffer from disk at a given file position.
	// Mirrors the identical pattern in Read — UseBuffer sandwich for sub-regions.
	auto fillBuffer = [this](LONG filePos)
		{
			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Seek(filePos, FileSeekMode::Set);
				this->Read(this->BufferPtr, this->BufferedSize);
				this->Seek(this->FilePos, FileSeekMode::Set);
				this->UseBuffer = 1;
			}
			else
			{
				this->RawFileClass::Seek(filePos, FileSeekMode::Set);
				this->RawFileClass::Read(this->BufferPtr, this->BufferedSize);
			}
		};

	int sizewritten = 0;
	int size1 = size;

	do
	{
		const int buffSize = this->BufferedSize;
		int sizetowrite = buffSize - this->BufferPos;
		if (size1 < sizetowrite)
			sizetowrite = size1;

		// Refill buffer from disk if partial write and window is stale.
		if (sizetowrite != buffSize && !this->IsCached)
		{
			int readSize;
			if (this->FileSize >= buffSize)
			{
				readSize = this->BufferedSize;
				this->BufferFilePos = this->FilePos;
			}
			else
			{
				readSize = this->FileSize;
				this->BufferFilePos = 0;
			}

			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Seek(this->FilePos, FileSeekMode::Set);
				this->Read(this->BufferPtr, this->BufferedSize);
				this->Seek(this->FilePos, FileSeekMode::Set);
				this->UseBuffer = 1;
			}
			else
			{
				this->RawFileClass::Seek(this->BufferFilePos, FileSeekMode::Set);
				this->RawFileClass::Read(this->BufferPtr, readSize);
			}

			this->BufferPos = 0;
			this->BufferChangeBeg = -1;
			this->BufferChangeEnd = -1;
			this->IsCached = 1;
		}

		// Copy caller data into buffer.
		memmove(static_cast<char*>(this->BufferPtr) + this->BufferPos,
				static_cast<char*>(buffer) + sizewritten,
				sizetowrite);

		const int oldBufferPos = this->BufferPos;

		sizewritten += sizetowrite;
		size1 -= sizetowrite;

		this->IsChanged = 1;

		// Expand dirty region to cover the written range.
		if (this->BufferChangeBeg == -1)
		{
			this->BufferChangeBeg = oldBufferPos;
			this->BufferChangeEnd = oldBufferPos;
		}
		else if (this->BufferChangeBeg > oldBufferPos)
		{
			this->BufferChangeBeg = oldBufferPos;
		}

		const int newBufferPos = oldBufferPos + sizetowrite;
		this->BufferPos = newBufferPos;

		if (this->BufferChangeEnd < newBufferPos)
			this->BufferChangeEnd = newBufferPos;

		// Advance FilePos; expand FileSize if write extended the file.
		const int newFilePos = newBufferPos + this->BufferFilePos;
		this->FilePos = newFilePos;
		if (this->FileSize < newFilePos)
			this->FileSize = newFilePos;

		// Buffer window full — commit and advance window.
		if (newBufferPos == this->BufferedSize)
		{
			this->BufferIOFileClass::Commit();
			const LONG nextStart = this->FilePos;
			this->BufferPos = 0;
			this->BufferFilePos = nextStart;
			this->BufferChangeBeg = -1;
			this->BufferChangeEnd = -1;

			if (size1 && this->FileSize > nextStart)
			{
				// More to write and file has data ahead — prefetch next window.
				fillBuffer(nextStart);
			}
			else
			{
				this->IsCached = 0;
			}
		}
	}
	while (size1);

	return finish(sizewritten);
}

void BufferIOFileClass::Close()
{
	if (this->UseBuffer)
	{
		this->BufferIOFileClass::Commit();

		if (this->IsDiskOpen)
		{
			if (this->TrueFileStart)
			{
				this->UseBuffer = 0;
				this->Close();
				this->UseBuffer = 1;
				this->IsDiskOpen = 0;
				this->Is_Open = 0;
				return;
			}

			this->RawFileClass::Close();
			this->IsDiskOpen = 0;
		}

		this->Is_Open = 0;
	}
	else
	{
		this->RawFileClass::Close();
	}
}

bool BufferIOFileClass::Cache(int size, void* buffer)
{
	// Already cached — valid only if caller passed no override args.
	if (this->BufferPtr)
		return (!size && !buffer) ? 1 : 0;

	// Query file size if available on disk.
	if (this->IsAvaible(0))
		this->FileSize = this->Size();
	else
		this->FileSize = 0;

	// ------------------------------------------------------------------
	// Resolve BufferedSize
	// ------------------------------------------------------------------
	signed int size1 = size;

	if (size)
	{
		if (size < 1024)
		{
			size1 = 1024;
			// Caller supplied an explicit buffer with a too-small size — error.
			if (buffer)
				this->Error(FileErrorType::INVAL, 0, 0);
		}
		this->BufferedSize = size1;

		// size1 == 0 can't happen here (size was non-zero and floored to 1024),
		// but the original checks it before falling through — preserved for fidelity.
		if (!size1)
		{
			if (!buffer)
				return 0; // SUSPECT: unreachable in practice
		}
	}
	else
	{
		// No explicit size — use the whole file.
		this->BufferedSize = this->FileSize;

		// No file size and no explicit buffer — nothing to cache.
		if (!buffer)
			return 0;
	}

	if (!this->BufferedSize)
		return 0;

	// ------------------------------------------------------------------
	// Acquire buffer
	// ------------------------------------------------------------------
	if (buffer)
	{
		this->BufferPtr = buffer;
	}
	else
	{
		this->BufferPtr = YRMemory::Allocate(this->BufferedSize);
	}

	if (!this->BufferPtr)
	{
		this->Error(FileErrorType::NOMEM, 0, 0);
		return 0;
	}

	// ------------------------------------------------------------------
	// Initialise state
	// ------------------------------------------------------------------
	const int filesize = this->FileSize;
	this->IsAllocated = 1;
	this->IsDiskOpen = 0;
	this->BufferPos = 0;
	this->BufferFilePos = 0;
	this->BufferChangeBeg = -1;
	this->BufferChangeEnd = -1;
	this->FilePos = 0;
	this->TrueFileStart = 0;

	if (filesize)
	{
		LONG startpos = 0;
		bool opened = false;
		int  sizetocache = (filesize > this->BufferedSize) ? this->BufferedSize : filesize;

		const bool wasOpen = this->IsOpen();

		if (!wasOpen)
		{
			// File not open — open it ourselves, record start, mark for close.
			if (this->Open1(FileAccessMode::Read))
			{
				this->TrueFileStart = this->RawFileClass::Seek(0, FileSeekMode::Current);
				opened = true;
			}
		}
		else
		{
			// File already open — snapshot current position.
			startpos = this->Seek(0, FileSeekMode::Current);

			if (this->Handle == INVALID_HANDLE_VALUE)
				this->TrueFileStart = startpos;
			else
				this->TrueFileStart = this->RawFileClass::Seek(0, FileSeekMode::Current);

			if (this->FileSize > this->BufferedSize)
			{
				// Partial cache — remember where in the file the buffer starts.
				this->BufferFilePos = startpos;
				this->FilePos = startpos;
			}
			else
			{
				// Full cache — seek to beginning so we read the whole file.
				if (startpos)
					this->Seek(0, FileSeekMode::Set);

				this->BufferPos = startpos;
				this->FilePos = startpos;
			}
		}

		// Read data into buffer.
		if (this->Read(this->BufferPtr, sizetocache) != sizetocache)
			this->Error(FileErrorType::IO, 0, 0);

		if (opened)
		{
			this->Close();
			this->IsCached = 1;
			this->UseBuffer = 1;
			return 1;
		}

		// Restore file position for the caller.
		this->Seek(startpos, FileSeekMode::Set);
		this->IsCached = 1;
	}

	this->UseBuffer = 1;
	return 1;
}

void BufferIOFileClass::Free()
{
	if (auto pBufferPtr = this->BufferPtr)
	{
		if (this->Allocated)
		{
			YRMemory::Deallocate(pBufferPtr);
			this->Allocated = false;
		}
		this->BufferPtr = nullptr;
	}

	this->BufferedSize = 0;
	this->Is_Open = false;
	this->IsCached = false;
	this->IsChanged = false;
	this->UseBuffer = false;
}

bool BufferIOFileClass::Commit()
{
	if (!this->UseBuffer || !this->IsChanged)
	{
		return 0;
	}

	int uncommited_start = this->BufferChangeBeg;
	int write_len = this->BufferChangeEnd - uncommited_start;

	if (this->IsDiskOpen)
	{
		this->RawFileClass::Seek(uncommited_start + this->TrueFileStart + this->BufferFilePos, FileSeekMode::Set);
		this->RawFileClass::Write(this->BufferPtr, write_len);
		this->RawFileClass::Seek(this->TrueFileStart + this->FilePos, FileSeekMode::Set);
	}
	else
	{
		this->RawFileClass::Open1(FileAccessMode::Read);
		this->RawFileClass::Seek(this->BufferChangeBeg + this->BufferFilePos + this->TrueFileStart, FileSeekMode::Set);
		this->RawFileClass::Write(this->BufferPtr, write_len);
		this->RawFileClass::Close();
	}

	this->IsChanged = 0;
	return 1;
}

#pragma endregion

#pragma region CDFileClass

const char* CDFileClass::SetFileName(const char* filename) {
	char a2[260];

	this->BufferIOFileClass::SetFileName(filename);

	// If CD searching is disabled, no search list exists, or file is already
	// available locally — return current name immediately.
	if (this->IsDisabled || !CDFileFirst() || this->BufferIOFileClass::IsAvaible(0))
		return this->FileName();

	// Walk the search path list looking for the file.
	for (SearchDriveType* srch = CDFileFirst(); srch; srch = srch->Next)
	{
		CRT::strcpy(a2, reinterpret_cast<const char*>(srch->Path));
		CRT::strcat(a2, filename);
		this->BufferIOFileClass::SetFileName(a2);

		if (this->BufferIOFileClass::IsAvaible(0))
			return this->FileName();
	}

	// Not found on any search path — restore original bare filename.
	this->BufferIOFileClass::SetFileName(filename);
	return this->FileName();
}

bool  CDFileClass::Open2(const char* pFileName, FileAccessMode rights) {
	this->BufferIOFileClass::Close();

	if (!pFileName) {
		this->Error(FileErrorType::NOENT, 0, 0);
	}

	if (this->IsDisabled || (int)rights == 2) {
		this->BufferIOFileClass::SetFileName(pFileName);
	} else {
		this->SetFileName(pFileName);
	}

	return this->BufferIOFileClass::Open1(rights);
}

void __fastcall CDFileClass::Refresh()
{
	auto chain = CDFileClass::CDFileFirst();
	SearchDriveType* next;

	if (CDFileClass::CDFileFirst()) {
		do {
			next = chain->Next;
			if (chain->Path) {
				YRMemory::free(chain->Path);
			}
			YRMemory::free(chain);
			chain = next;
		}
		while (next);
	}

	CDFileClass::CDFileFirst = nullptr;
	CDFileClass::SetPath(CDFileClass::RawPath());
}

bool __fastcall CDFileClass::SetPath(const char* pPath) {
	if (!pPath)
		return false;

	char path[260];
	bool found = false;

	char* plist = CRT::strdup(pPath);

	// Append to the raw accumulated path string.
	if (plist != CDFileClass::RawPath()) {
		CRT::strcat(CDFileClass::RawPath(), ";");
		CRT::strcat(CDFileClass::RawPath(), plist);
	}

	for (char* ptr = CRT::strtok(plist, ";"); ptr; ptr = CRT::strtok(nullptr, ";")) {

		if (!strlen(ptr))
			continue;

		CRT::strcpy(path, ptr);

		// Ensure path ends with ':' or '\'.
		const char lastChar = path[strlen(path) - 1];
		if (lastChar != ':' && lastChar != '\\')
			CRT::strcat(path, "\\");

		if (!CRT::strncmp(path, "?:", 2))
		{
			// Wildcard CD drive — resolve to CurrentCDDrive.
			int driveIdx = CDDriveManagerClass::DriveIndex();

			if (!driveIdx)
				continue;

			found = true;

			if (CDDriveManagerClass::GetCDNumber(driveIdx, 120) <= -1)
				continue;

			path[0] = static_cast<char>(CDFileClass::CurrentCDDrive() + 'A');
			AddPath(path);
		}
		else
		{
			// Explicit drive letter — only add if it's a CD drive (index == 2).
			if (path[0] - 'A' <= 0 || CDDriveManagerClass::GetCDNumber(path[0] - 'A', 120) != 2)
				continue;

			found = true;
			AddPath(path);
		}
	}

	YRMemory::free(plist);

	// SUSPECT: returns true when nothing was found, false when found.
	// Inverted return matches original: `return found == 0`.
	return !found;
}

void __fastcall CDFileClass::AddPath(const char* path)
{
	SearchDriveType* srch = new SearchDriveType();
	srch->Path = _strdup(path);
	srch->Next = nullptr;

	if (!CDFileClass::CDFileFirst())
	{
		CDFileClass::CDFileFirst = srch;
		return;
	}

	// Walk to the last node.
	SearchDriveType* chain = CDFileClass::CDFileFirst();
	while (chain->Next)
		chain = chain->Next;

	chain->Next = srch;

}

void __fastcall CDFileClass::SetCDDrive(int nDriveNumber)
{
	int last = CDDriveManagerClass::DriveIndex();
	CDDriveManagerClass::DriveIndex = nDriveNumber;
	CDDriveManagerClass::LastDriveIndex = last;
}

#pragma endregion

#pragma region CCFileClass

bool CCFileClass::IsAvaible(bool writeShared)
{
	if (this->Availablility == 1)
	{
		return 1;
	}

	if (this->IsOpen())
	{
		this->Availablility = 1;
		return 1;
	}
	else
	{
		if (MixFileClass::Offset(this->FileName(),
			static_cast<void**>(nullptr),
			static_cast<MixFileClass**>(nullptr),
			static_cast<int*>(nullptr),
			static_cast<int*>(nullptr)))
		{
			this->Availablility = 1;
			return  1;
		}
		else if (this->BufferIOFileClass::IsAvaible(0))
		{
			this->Availablility = 1;
			return  1;
		}
		else
		{
			this->Availablility = 2;
			return  0;
		}
	}

}

bool CCFileClass::IsOpen()
{
	if (this->Buffer.Buffer)
	{
		return 1;
	}
	else
	{
		return this->BufferIOFileClass::IsOpen();
	}
}

bool CCFileClass::Open1(FileAccessMode rights)
{
	// Always close the file if it was open.
	this->Close();

	// Perform a preliminary check to see if the specified file exists on disk.
	// If it does, open it regardless of whether it also exists in RAM.
	// This is slower but allows upgrade files to work.
	if (((DWORD)rights & 2) != 0 || this->BufferIOFileClass::IsAvaible(0))
		return this->CDFileClass::Open1(rights);

	// Check to see if the file is part of a mixfile currently loaded into RAM.
	MixFileClass* mixfile = nullptr;
	void* pointer = nullptr;
	int           length = 0;
	int           start = 0;

	const char* filename = this->FileName();

	if (!MixFileClass::Offset(filename, &pointer, &mixfile, &start, &length))
	{
		// File cannot be found in any mixfile — must reside as an individual
		// file on disk, or is just plain missing.
		return this->CDFileClass::Open1(rights);
	}

	// File found in a mixfile.
	if (pointer || !mixfile)
	{
		// Mixfile is in RAM — serve directly from the in-memory pointer.
		if (this != reinterpret_cast<CCFileClass*>(-static_cast<int>(offsetof(CCFileClass, Buffer))))
			this->Buffer.Release(pointer, length);

		this->Position = 0;
		return 1;
	}

	// Mixfile is on disk — fake out the file system to read from the mixfile
	// as if it were a solitary file, with bias applied for the embedded offset.
	if (!this->Open2(mixfile->Filename, FileAccessMode::Read))
		return 0;

	// Snapshot the current (mixfile) filename before we redirect Set_Name.
	filename = this->FileName();
	char* dupfile = CRT::strdup(filename);

	// Temporarily disable CD search so Set_Name doesn't search drives.
	this->IsDisabled = 1;
	this->SetFileName(dupfile);
	this->IsDisabled = 0;

	// Reset bias then apply the embedded file's offset + length within the mixfile.
	this->RawFileClass::Bias(0, -1);
	this->RawFileClass::Bias(start, length);
	this->Seek(0, FileSeekMode::Set);

	YRMemory::Deallocate(dupfile);
	return 1;
}

int CCFileClass::Read(void* buffer, int size)
{
	bool opened = 0;

	if (!this->IsOpen() && this->Open1(FileAccessMode::Read))
	{
		opened = 1;
	}

	DWORD size_1;

	if (auto bufferptr = (void*)this->Buffer.Buffer)
	{
		auto cachepos = this->Position;
		size_1 = size;
		if (this->Buffer.Size - cachepos < (DWORD)size)
		{
			size_1 = this->Buffer.Size - cachepos;
		}

		if (size_1)
		{
			memmove(buffer, (char*)bufferptr + cachepos, size_1);
			this->Position += size_1;
		}

		if (opened)
		{
			this->Close();
			return size_1;
		}
	}
	else
	{

		size_1 = this->BufferIOFileClass::Read(buffer, size);

		if (opened)
		{
			this->Close();
		}
	}

	return size_1;
}

off_t CCFileClass::Seek(off_t pos, FileSeekMode dir)
{

	if (!this->Buffer.Buffer)
	{
		return this->BufferIOFileClass::Seek(pos, dir);
	}

	if ((int)dir)
	{
		if ((int)dir == 2)
		{
			this->Position = this->Buffer.Size;
		}
	}
	else
	{
		this->Position = 0;
	}

	int position = this->Position;
	int v4 = pos + position < 0;
	int v5 = pos + position;
	this->Position = v5;
	int result = v4 ? 0 : v5;
	this->Position = result;
	if (result > this->Buffer.Size)
	{
		result = this->Buffer.Size;
	}

	this->Position = result;
	return result;
}

off_t CCFileClass::Size()
{

	if (this->Buffer.Buffer)
	{
		return this->Buffer.Size;
	}

	if (this->BufferIOFileClass::IsAvaible(0))
	{
		return this->BufferIOFileClass::Size();
	}

	int length = 0;
	MixFileClass::Offset(this->FileName(),
		static_cast<void**>(nullptr),
		static_cast<MixFileClass**>(nullptr),
		static_cast<int*>(nullptr),
		&length);
	return length;
}

int CCFileClass::Write(void* buffer, int length)
{
	if (this->Buffer.Buffer)
	{
		this->Error(FileErrorType::ACCES, 0, this->FileName());
	}
	return this->BufferIOFileClass::Write(buffer, length);
}

void CCFileClass::Close()
{
	auto v2 = &this->Buffer;

	if (v2)
		v2->Release(nullptr, NULL);

	this->Position = 0;
	this->BufferIOFileClass::Close();
}

LONG CCFileClass::GetDataTime()
{
	MixFileClass* mixfile = 0;

	auto datetime = this->RawFileClass::GetDataTime();

	if (datetime)
	{
		return datetime;
	}

	auto fname = this->FileName();
	if (!MixFileClass::Offset(fname,
		static_cast<void**>(nullptr),
		&mixfile,
		static_cast<int*>(nullptr),
		static_cast<int*>(nullptr)))
	{
		return datetime;
	}

	CDFileClass cd(mixfile->Filename);
	return cd.GetDataTime();
}

bool CCFileClass::SetDateTime(LONG datetime)
{
	MixFileClass* mixfile = 0;

	auto status = this->RawFileClass::SetDateTime(datetime);

	if (!status)
	{
		auto fname = this->FileName();

		if (MixFileClass::Offset(fname,
			static_cast<void**>(nullptr),
			&mixfile,
			static_cast<int*>(nullptr),
			static_cast<int*>(nullptr)))
		{
			CDFileClass cd(mixfile->Filename);
			status = cd.SetDateTime(datetime);
		}
	}
	return status;
}

void CCFileClass::Error(FileErrorType error, bool can_retry, const char* filename)
{
	if (!this->SkipCDCheck)
	{
		CD cd;
		cd.ForceAvailable(CD::Disk());
	}
}

void* CCFileClass::Load_Alloc_Data(FileClass& file)
{
	const long size = file.Size();

	if (void* ptr = new char[size])
	{
		if (file.Read(ptr, size))
			return ptr;
		else
			delete[] ptr;
	}

	return nullptr;
}

void* CCFileClass::Load_Alloc_Data(char const* name)
{
	CCFileClass file(name);
	return Load_Alloc_Data(file);
}
#pragma endregion

#pragma region RAMFileClass

int RAMFileClass::Read(void* buffer, int length)
{
	if (!this->Buffer || !buffer || !length)
		return 0;

	bool opened = false;

	if (this->IsOpen())
	{
		// Already open — must have read access.
		if (!((DWORD)this->Access & 1))
			return 0;
	}
	else
	{
		// Auto-open for reading.
		this->Open1(FileAccessMode::Read);
		opened = true;
	}

	// Clamp read length to remaining readable data.
	int readLen = length;
	const int remaining = this->Length - this->Offset;
	if (readLen >= remaining)
		readLen = remaining;

	memmove(buffer, &this->Buffer[this->Offset], readLen);
	this->Offset += readLen;

	if (opened)
		this->Close();

	return readLen;
}

off_t RAMFileClass::Seek(off_t offset, FileSeekMode whence)
{
	if (!this->Buffer || !this->IsOpen())
		return this->Offset;

	const int length = this->Length;
	const int maxOffset = ((DWORD)this->Access & 2) ? this->MaxLength : length;

	// Apply whence — collapse case 0 and case 2 via shared assignment.
	switch (whence)
	{
	case FileSeekMode::Set:   this->Offset = offset;            break;
	case FileSeekMode::Current: this->Offset += offset;            break;
	case FileSeekMode::End:     this->Offset = maxOffset + offset; break;
	}

	// Clamp to [0, maxOffset].
	if (static_cast<int>(this->Offset) < 0)
		this->Offset = 0;
	if (this->Offset > maxOffset)
		this->Offset = maxOffset;

	// Extend Length if seek moved past it (write-mode expand).
	if (this->Offset > length)
		this->Length = this->Offset;

	return this->Offset;
}

int RAMFileClass::Write(void* buffer, int length)
{

	if (!this->Buffer || !buffer || !length)
		return 0;

	bool opened = false;

	if (this->IsOpen())
	{
		// Already open — must have write access.
		if (!((DWORD)this->Access & 2))
			return 0;
	}
	else
	{
		// Auto-open for writing.
		this->Open1(FileAccessMode::Write);
		opened = true;
	}

	// Clamp write length to remaining buffer space.
	int writeLen = length;
	const int remaining = this->MaxLength - this->Offset;
	if (writeLen >= remaining)
		writeLen = remaining;

	memmove(&this->Buffer[this->Offset], buffer, writeLen);

	const int newOffset = this->Offset + writeLen;
	this->Offset = newOffset;

	if (newOffset > this->Length)
		this->Length = newOffset;

	if (opened)
		this->Close();

	return writeLen;
}

#pragma endregion