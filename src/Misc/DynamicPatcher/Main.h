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
		STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
		PROCESS_INFORMATION processInfo;

		if (CreateProcess((LPCSTR)Registration::RegAsm, (LPSTR)(L" " + GetSafePath(dllPath)).data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
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
					Debug::fata
					ShowErrorAndExit(L"Register Dynamic Patcher failed.");
				}
			}
			else
			{
				std::wstring error (L"GetExitCodeProcess() failed: %ld");
				error += std::to_wstring(GetLastError());

				ShowErrorAndExit(error);
			}
		}
		else
		{
			std::wstring error = L"CreateProcess() failed: %ld";
			error += std::to_wstring(GetLastError());

			ShowErrorAndExit(error);
		}

		return false;
	}

	static bool Unregister(std::wstring dllPath)
	{
		STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
		PROCESS_INFORMATION processInfo;

		if (CreateProcess(RegAsm, (L" /u " + GetSafePath(dllPath)).data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
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
