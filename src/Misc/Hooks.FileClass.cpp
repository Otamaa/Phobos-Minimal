#include <CCFileClass.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>
#ifndef ENABLE_NEWHOOK
static long RawFileClass_Read(RawFileClass* pThis, void* buffer, int length)
{
	if (buffer == nullptr)
		Debug::Log("nullptr buffer ! -> %s \n", pThis->Get_Safe_File_Name());
	if (length <= 0)
		Debug::Log("invalid size ! -> %s \n", pThis->Get_Safe_File_Name());

	long bytesread = 0; // Running count of the number of bytes read into the buffer.
	int	opened = false; // Was the file opened by this routine?

	if (!pThis->Exists())
	{
		if (!pThis->Open(FileAccessMode::Read))
		{
			return 0;
		}
		opened = true;
	}

	//Debug::Log("Reading File \"%s\".\n", pThis->FileName);

	if (pThis->FileSize != -1)
	{
		int remainder = pThis->FileSize - pThis->Seek(0, FileSeekMode::Current);
		length = length < remainder ? length : remainder;
	}

	auto nStreamerAccess = Make_Global<bool>(0xB04BEC);
	long total = 0;

	while (length > 0)
	{
		bytesread = 0;

		SetErrorMode(SEM_FAILCRITICALERRORS);
		if (!ReadFile(pThis->Get_File_Handle(), buffer, length, &(DWORD&)bytesread, nullptr))
		{
			buffer = (unsigned char*)buffer + bytesread;
			length -= bytesread;
			total += bytesread;
			if (nStreamerAccess)
				break;

			pThis->Error(FileErrorType(GetLastError()), true, pThis->FileName);

		}
		else
		{
			buffer = (unsigned char*)buffer + bytesread;
			length -= bytesread;
			total += bytesread;
			if (bytesread == 0)
			{
				break;
			}
		}
	}

	bytesread = total;
	nStreamerAccess = false;

	if (opened)
	{
		pThis->Close();
	}

	return bytesread;
}

static void Game_Emergency_Exit(int code)
{
	JMP_STD(0x6BEC50);
}

DEFINE_HOOK(0x65CCE0, RawFileClass_Read_Replace, 0x7) //4
{
	GET(RawFileClass*, pThis, ECX);
	GET_STACK(void*, pBuff, 0x4);
	GET_STACK(int, nSize, 0x8);
	R->EAX(RawFileClass_Read(pThis, pBuff, nSize));
	return 0x65CD0C;
}

DEFINE_HOOK(0x65CA70, RawFileClass_Error_, 0x3) // 5
{
	GET(RawFileClass*, pThis, ECX);
	GET_STACK(FileErrorType, error, 0x4);
	GET_STACK(bool, can_retry, 0x8);
	GET_STACK(const char*, pFilename, 0xC);

	Debug::Log("File - Error:(%d) \"%s\"  can_retry:%s  filename:%s.\n",
		error, FileClass::File_Error_To_String(error),
		can_retry ? "true" : "false",
		pFilename != nullptr ? pFilename : pThis->FileName);

	if (!can_retry)
	{
		Debug::Log("File - Error can_retry = false ! ExitFailure ! \n");
		Game_Emergency_Exit(EXIT_FAILURE);
	}

	return 0x0;
}
#endif