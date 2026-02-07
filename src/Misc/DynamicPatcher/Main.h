#pragma once

#include <filesystem>
#include <string>
#include <Base/Always.h>
#include <Utilities/Debug.h>

class Registration
{
	COMPILETIMEEVAL static const wchar_t RegAsm[] = L"C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\RegAsm.exe";

	static std::wstring GetSafePath(std::filesystem::path path)
	{
		std::wstring dllPath = path;

		return L"\"" + dllPath + L"\"";
	}

public:
	static bool Register(std::wstring dllPath)
	{
		STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
		PROCESS_INFORMATION processInfo;

		std::wstring cmdLine = L" " + GetSafePath(dllPath);
		if (CreateProcessW(Registration::RegAsm, cmdLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
		{
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			DWORD exitCode;
			if (GetExitCodeProcess(processInfo.hProcess, &exitCode))
			{
				if (exitCode == 0)
				{
					CloseHandle(processInfo.hProcess);
					CloseHandle(processInfo.hThread);
					return true;
				}
				else
				{
					Debug::FatalError("Register Dynamic Patcher failed. Exit code: %d", exitCode);
				}
			}
			else
			{
				Debug::FatalError("GetExitCodeProcess() failed: %ld", GetLastError());
			}
		}
		else
		{
			Debug::FatalError("CreateProcess() failed: %ld", GetLastError());
		}

		return false;
	}

	static bool Unregister(std::wstring dllPath)
	{
		STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
		PROCESS_INFORMATION processInfo;

		std::wstring cmdLine = L" /u " + GetSafePath(dllPath);
		if (CreateProcessW(RegAsm, cmdLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
		{
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			DWORD exitCode;
			if (GetExitCodeProcess(processInfo.hProcess, &exitCode))
			{
				if (exitCode == 0)
				{
					CloseHandle(processInfo.hProcess);
					CloseHandle(processInfo.hThread);
					return true;
				}
			}
		}
		return false;
	}
};
