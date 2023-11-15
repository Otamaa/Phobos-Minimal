#pragma once

#include <Unsorted.h>
#include <WWMouseClass.h>

#include <Windows.h>
#include <Dbghelp.h>
#include <string>

class Dialogs
{
public:

	static void TakeMouse()
	{
		WWMouseClass::Instance->ReleaseMouse();
		Imports::ShowCursor.get()(1);
	}

	static void ReturnMouse()
	{
		Imports::ShowCursor.get()(0);
		WWMouseClass::Instance->CaptureMouse();
	}

	static std::wstring PrepareSnapshotDirectory()
	{
		wchar_t path[MAX_PATH];
		auto const len = GetCurrentDirectoryW(MAX_PATH, path);
		std::wstring buffer(path, path + len);

		buffer += L"\\debug";
		CreateDirectoryW(buffer.c_str(), nullptr);

		SYSTEMTIME time;
		GetLocalTime(&time);

		wchar_t subpath[64];
		swprintf(subpath, 64, L"\\snapshot-%04u%02u%02u-%02u%02u%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		buffer += subpath;
		CreateDirectoryW(buffer.c_str(), nullptr);

		return buffer;
	}

	static std::wstring FullDump(PMINIDUMP_EXCEPTION_INFORMATION const pException = nullptr)
	{
		return FullDump(PrepareSnapshotDirectory(), pException);
	}

	static std::wstring FullDump(
		std::wstring destinationFolder,
		PMINIDUMP_EXCEPTION_INFORMATION const pException = nullptr)
	{
		std::wstring filename = std::move(destinationFolder);
		filename += L"\\extcrashdump.dmp";

		HANDLE dumpFile = CreateFileW(filename.c_str(), GENERIC_WRITE,
			0, nullptr, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, nullptr);

		MINIDUMP_TYPE type = static_cast<MINIDUMP_TYPE>(MiniDumpNormal
									   | MiniDumpWithDataSegs
									   | MiniDumpWithIndirectlyReferencedMemory
									   | MiniDumpWithFullMemory
		);

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, type, pException, nullptr, nullptr);
		CloseHandle(dumpFile);

		return filename;
	}
};