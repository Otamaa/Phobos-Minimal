#include "Debug.h"
#include "Macro.h"
#include <Phobos.h>


#include <MouseClass.h>
#include <Surface.h>

#include <CRT.h>
#include <YRPPCore.h>
#include <AbstractClass.h>
#include <vector>
#include <Phobos.h>

char Debug::DeferredStringBuffer[0x1000];
char Debug::LogMessageBuffer[0x1000];
std::vector<std::string> Debug::DeferredLogData;

bool Debug::LogEnabled = false;
bool Debug::made = false;

FILE* Debug::LogFile = nullptr;
std::wstring Debug::ApplicationFilePath {};
std::wstring Debug::LogFilePathName {};
std::wstring Debug::LogFileMainName { L"\\debug" };
std::wstring Debug::LogFileMainFormattedName {};
std::wstring Debug::LogFileTempName {};
std::wstring Debug::LogFileExt { L".log" };

Console::ConsoleTextAttribute Console::TextAttribute;
HANDLE Console::ConsoleHandle;

bool Console::Create()
{
	if (FALSE == AllocConsole())
		return false;

	ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL == ConsoleHandle)
		return false;

	SetConsoleTitle("Phobos Debug Console");

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
	TextAttribute.AsWord = csbi.wAttributes;

	return true;
}

void Console::SetForeColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle || TextAttribute.Foreground == color)
		return;

	TextAttribute.Foreground = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::SetBackColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle || TextAttribute.Background == color)
		return;

	TextAttribute.Background = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::EnableUnderscore(bool enable)
{
	if (NULL == ConsoleHandle || TextAttribute.Underscore == enable)
		return;

	TextAttribute.Underscore = enable;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::Release()
{
	if (NULL == ConsoleHandle)
		return;

	FreeConsole();
}

void Console::Write(const char* str, int len)
{
	if (NULL == ConsoleHandle)
		return;

	WriteConsole(ConsoleHandle, str, len, nullptr, nullptr);
}

void Console::WriteLine(const char* str, int len)
{
	Write(str, len);
	Write("\n");
}

void Console::WriteWithVArgs(const char* pFormat, va_list args)
{
	if (ConsoleHandle == NULL)
		return;

	vsprintf_s(Debug::LogMessageBuffer, pFormat, args);
	WriteConsole(ConsoleHandle, Debug::LogMessageBuffer, strlen(Debug::LogMessageBuffer), nullptr, nullptr);
}

void Console::WriteFormat(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	WriteWithVArgs(pFormat, args);
	va_end(args);
}

void Console::PatchLog(DWORD dwAddr, void* fakeFunc, DWORD* pdwRealFunc)
{
#pragma pack(push, 1)
	struct JMP_STRUCT
	{
		byte opcode;
		DWORD offset;
	} *pInst;
#pragma pack(pop)

	DWORD dwOldFlag {};
	VirtualProtect((LPVOID)dwAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldFlag);

	pInst = (JMP_STRUCT*)dwAddr;

	if (pdwRealFunc && pInst->opcode == 0xE9) // If this function is hooked by Ares
		*pdwRealFunc = pInst->offset + dwAddr + 5;

	pInst->offset = reinterpret_cast<DWORD>(fakeFunc) - dwAddr - 5;

	DWORD nwOldFlag {};
	VirtualProtect((LPVOID)dwAddr, 5, dwOldFlag, &nwOldFlag);
}