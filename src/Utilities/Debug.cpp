#include "Debug.h"
#include "Macro.h"
#include <Phobos.h>


#include <MouseClass.h>
#include <WWMouseClass.h>
#include <Surface.h>

#include <Misc/Ares/Hooks/Classes/Dialogs.h>

#include <MessageListClass.h>
#include <CRT.h>
#include <YRPPCore.h>
#include <AbstractClass.h>
#include <vector>
#include <Phobos.h>

char Debug::DeferredStringBuffer[0x1000];
char Debug::LogMessageBuffer[0x1000];
std::vector<std::string> Debug::DeferredLogData;

bool Debug::LogEnabled = false;
FILE* Debug::LogFile = nullptr;
std::wstring Debug::LogFileName;
std::wstring Debug::LogFileTempName;

// push    0x860A0000; vs 0x02CA0000
//DEFINE_RAW_PATCH(0x777CC0, CreateMainWindow , 0x68, 0x00, 0x00, 0x0A, 0x86)

void Debug::DumpStack(REGISTERS* R, size_t len, int startAt)
{
	if (!Debug::LogFileActive()) {
		return;
	}

	Debug::LogUnflushed("Dumping %X bytes of stack\n", len);
	auto const end = len / 4;
	auto const* const mem = R->lea_Stack<DWORD*>(startAt);
	for (auto i = 0u; i < end; ++i) {

		const char* suffix = "";
		const uintptr_t ptr = mem[i];
		if (ptr >= 0x401000 && ptr <= 0xB79BE4)
			suffix = "GameMemory!";

		Debug::LogUnflushed("esp+%04X = %08X %s\n", i * 4, mem[i] , suffix);
	}

	Debug::Log("====================Done.\n"); // flushes
}

//This log is not immedietely printed , but buffered until time it need to be finalize(printed)
void Debug::LogDeferred(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(DeferredStringBuffer, pFormat, args);
	DeferredLogData.emplace_back(DeferredStringBuffer);
	va_end(args);
}

void Debug::LogDeferredFinalize()
{
	for (auto const& Logs : DeferredLogData)
	{
		if (!Logs.empty())
			GameDebugLog::Log("%s", Logs);
	}

	DeferredLogData.clear();
}

void Debug::LogAndMessage(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(LogMessageBuffer, pFormat, args);
	Log("%s", LogMessageBuffer);
	va_end(args);
	wchar_t buffer[0x1000];
	mbstowcs(buffer, LogMessageBuffer, 0x1000);
	MessageListClass::Instance->PrintMessage(buffer);
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	if (Phobos::Otamaa::TrackParserErrors)
	{
		const char* LogMessage = (Message == nullptr)
			? "[Phobos] Failed to parse INI file content: [%s]%s=%s\n"
			: "[Phobos] Failed to parse INI file content: [%s]%s=%s (%s)\n"
			;

		Debug::Log(LogMessage, section, flag, value, Message);
		Debug::RegisterParserError();
	}
}

[[noreturn]] void Debug::FatalErrorAndExit(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(Phobos::readBuffer, pFormat, args);
	va_end(args);
	Debug::FatalError(false);
	Debug::ExitGame();
}

[[noreturn]] void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(Phobos::readBuffer, pFormat, args);
	va_end(args);
	Debug::FatalError(false);
	Debug::ExitGame();
}

void __SUNINI_TORA2MD(const char* pFormat, ...) {
	GameDebugLog::Log("-------- Loading RA2MD.INI settings --------\n");
}
DEFINE_JUMP(CALL, 0x5FA636, GET_OFFSET(__SUNINI_TORA2MD));

void Debug::LogWithVArgsUnflushed(
	const char* const pFormat, va_list const args)
{
	Console::WriteWithVArgs(pFormat, args);
	vfprintf(Debug::LogFile, pFormat, args);
}

void Debug::LogFileOpen()
{
	Debug::MakeLogFile();
	Debug::LogFileClose(999);

	LogFile = _wfsopen(Debug::LogFileTempName.c_str(), L"w", SH_DENYNO);

	if (!LogFile) {
		wchar_t msg[100] = L"\0";
		wsprintfW(msg, L"Log file failed to open. Error code = %X", errno);
		MessageBoxW(Game::hWnd, Debug::LogFileTempName.c_str(), msg, MB_OK | MB_ICONEXCLAMATION);
		Phobos::Otamaa::ExeTerminated = true;
		ExitProcess(1);
	}
}

void Debug::LogFileClose(int tag)
{
	if (Debug::LogFile)
	{
		fprintf(Debug::LogFile, "Closing log file on request %d", tag);
		fclose(Debug::LogFile);
		CopyFileW(Debug::LogFileTempName.c_str(), Debug::LogFileName.c_str(), FALSE);
		Debug::LogFile = nullptr;
	}
}

void Debug::MakeLogFile()
{
	static bool made = 0;
	if (!made)
	{
		wchar_t path[MAX_PATH];

		SYSTEMTIME time;

		GetLocalTime(&time);
		GetCurrentDirectoryW(MAX_PATH, path);

		Debug::LogFileName = path;
		Debug::LogFileName += L"\\debug";

		CreateDirectoryW(Debug::LogFileName.c_str(), nullptr);

		wchar_t subpath[64];
		swprintf(subpath, 64, L"\\debug.%04u%02u%02u-%02u%02u%02u.log",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		Debug::LogFileTempName = Debug::LogFileName + L"\\debug.log";
		Debug::LogFileName += subpath;

		made = 1;
	}
}

void Debug::LogFileRemove()
{
	Debug::LogFileClose(555);
	DeleteFileW(Debug::LogFileTempName.c_str());
}

void Debug::FreeMouse()
{
	Game::StreamerThreadFlush();

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	WWMouseClass::Instance->ReleaseMouse();

	ShowCursor(TRUE);

	auto const BlackSurface = [](DSurface* pSurface) {
		if (pSurface && VTable::Get(pSurface) == DSurface::vtable && pSurface->BufferPtr) {
			pSurface->Fill(0);
		}
	};

	BlackSurface(DSurface::Alternate);
	BlackSurface(DSurface::Composite);
	BlackSurface(DSurface::Hidden);
	BlackSurface(DSurface::Temp);
	BlackSurface(DSurface::Primary);
	BlackSurface(DSurface::Sidebar);
	BlackSurface(DSurface::Tile);

	ShowCursor(TRUE);
}

void Debug::FatalError(bool Dump)
{
	static wchar_t Message[0x400];
	wsprintfW(Message,
		L"An internal error has been encountered and the game is unable to continue normally. "
		L"Please notify the mod's creators about this issue, or Contact Otamaa at "
		L"Discord for updates and support.\n\n"
		L"%hs",
		Phobos::readBuffer);

	Debug::Log("\nFatal Error: \n%s\n" , Phobos::readBuffer);
	Debug::FreeMouse();
	MessageBoxW(Game::hWnd, Message, L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);

	if (Dump)
	{
		Debug::FullDump();
	}
}

void Debug::FatalError(const char* Message, ...)
{
	Debug::FreeMouse();

	va_list args;
	va_start(args, Message);
	vsnprintf_s(Phobos::readBuffer, Phobos::readLength - 1, Message, args); /* note that the message will be truncated somewhere after 0x300 chars... */
	va_end(args);

	Debug::FatalError(false);
}

void Debug::ExitGame()
{
	Phobos::ExeTerminate();
	ExitProcess(1u);
}

void Debug::Flush()
{
	fflush(Debug::LogFile);
}

std::wstring Debug::FullDump()
{
	return Dialogs::FullDump();
}

std::wstring Debug::FullDump(std::wstring destinationFolder)
{
	return Dialogs::FullDump(std::move(destinationFolder));
}

void Debug::DumpObj(void const* const data, size_t const len)
{
	if (!Debug::LogFileActive())
	{
		return;
	}
	Debug::LogUnflushed("Dumping %u bytes of object at %p\n", len, data);
	auto const bytes = static_cast<byte const*>(data);

	Debug::LogUnflushed("       |");
	for (auto i = 0u; i < 0x10u; ++i)
	{
		Debug::LogUnflushed(" %02X |", i);
	}
	Debug::LogUnflushed("\n");
	Debug::LogUnflushed("-------|");
	for (auto i = 0u; i < 0x10u; ++i)
	{
		Debug::LogUnflushed("----|", i);
	}
	auto const bytesToPrint = (len + 0x10 - 1) / 0x10 * 0x10;
	for (auto startRow = 0u; startRow < bytesToPrint; startRow += 0x10)
	{
		Debug::LogUnflushed("\n");
		Debug::LogUnflushed(" %05X |", startRow);
		auto const bytesInRow = std::min(len - startRow, 0x10u);
		for (auto i = 0u; i < bytesInRow; ++i)
		{
			Debug::LogUnflushed(" %02X |", bytes[startRow + i]);
		}
		for (auto i = bytesInRow; i < 0x10u; ++i)
		{
			Debug::LogUnflushed(" -- |");
		}
		for (auto i = 0u; i < bytesInRow; ++i)
		{
			auto const& sym = bytes[startRow + i];
			Debug::LogUnflushed("%c", isprint(sym) ? sym : '?');
		}
	}
	Debug::Log("\nEnd of dump.\n"); // flushes
}

void Debug::LogWithVArgs(const char* const pFormat, va_list const args)
{
	if (Debug::LogFileActive())
	{
		Debug::LogWithVArgsUnflushed(pFormat, args);
		Debug::Flush();
	}
}

void __cdecl Debug::LogUnflushed(const char* const pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgsUnflushed(pFormat, args);
	va_end(args);
}

void __cdecl Debug::LogFlushed(
	Debug::Severity const severity, const char* const pFormat, ...)
{
	if (Debug::LogFileActive())
	{
		if (severity != Severity::None)
		{
			Debug::LogUnflushed(
				"[Developer %s]", SeverityString(severity));
		}
		va_list args;
		va_start(args, pFormat);
		Debug::LogWithVArgs(pFormat, args);
		va_end(args);
	}
}

void Debug::LogFlushed(const char* const pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgs(pFormat, args);
	va_end(args);
}

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

void __fastcall Console::WriteWithVArgs(const char* pFormat, va_list args)
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