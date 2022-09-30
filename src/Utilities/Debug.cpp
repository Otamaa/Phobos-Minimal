#include "Debug.h"
#include "Macro.h"
#include <Phobos.h>

#include <MessageListClass.h>
#include <CRT.h>
#include <YRPPCore.h>
#include <AbstractClass.h>

char Debug::StringBuffer[0x1000];

void Debug::Log(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::LogAndMessage(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
	va_end(args);
	wchar_t buffer[0x1000];
	CRT::mbstowcs(buffer, StringBuffer, 0x1000);
	MessageListClass::Instance->PrintMessage(buffer);
}

void Debug::LogWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	const char* LogMessage = (Message == nullptr)
		? "Failed to parse INI file content: [%s]%s=%s\n"
		: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

	Debug::Log(LogMessage, section, flag, value, Message);
}

[[noreturn]] void Debug::FatalErrorAndExit(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

[[noreturn]] void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(nExitCode));
}

DEFINE_PATCH( // Add new line after "Init Secondary Mixfiles....."
	/* Offset */ 0x825F9B,
	/*   Data */ '\n'
);

DEFINE_PATCH( // Replace SUN.INI with RA2MD.INI in the debug.log
	/* Offset */ 0x8332F4,
	/*   Data */ "-------- Loading RA2MD.INI settings --------\n"
);
#pragma region Otamaa
void Debug::DumpStack(const char* function, REGISTERS* R, size_t len, int startAt)
{

	Debug::LogUnflushed("Phobos::Dumping %X bytes of stack from [%s]\n", len, function);
	auto const end = len / 4;
	auto const* const mem = R->lea_Stack<DWORD*>(startAt);
	for (auto i = 0u; i < end; ++i)
	{
		Debug::LogUnflushed("esp+%04X = %08X\n", i * 4, mem[i]);
	}
	Debug::Log("Phobos::Dumping[%s] Done.\n", function); // flushes
}

void __cdecl Debug::LogUnflushed(const char* const pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgsUnflushed(pFormat, args);
	va_end(args);
}

void Debug::LogWithVArgsUnflushed(
	const char* const pFormat, va_list const args)
{
	LogWithVArgs(pFormat, args);
}

void Debug::TestPointer(AbstractClass* pAbstract)
{
	Debug::Log("Phobos::What Pointer it Is [%d].\n", pAbstract->WhatAmI());
}

void Debug::DumpObj(void const* const data, size_t const len)
{

	Debug::LogUnflushed("Phobos::Dumping %u bytes of object at %p\n", len, data);
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
	Debug::Log("\nPhobos::End of dump.\n"); // flushes
}

void Debug::DumpStack(const char* function, size_t len, int startAt)
{

	Debug::LogUnflushed("Phobos::Dumping %X bytes of stack from [%s]\n", len, function);
	auto const end = len / 4;
	DWORD lea_nData;
	_asm {mov lea_nData, esp};
	auto const* const mem = reinterpret_cast<DWORD*>(static_cast<DWORD>(lea_nData + static_cast<DWORD>(startAt)));

	for (auto i = 0u; i < end; ++i)
	{
		Debug::LogUnflushed("esp+%04X = %08X\n", i * 4, mem[i]);
	}
	Debug::Log("Phobos::Dumping[%s] Done.\n", function); // flushes
}

void Debug::FreeMouse()
{
	Game::StreamerThreadFlush();

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	WWMouseClass::Instance->ReleaseMouse();

	ShowCursor(TRUE);

	auto const BlackSurface = [](DSurface* const pSurface)
	{
		if (pSurface)
		{
			pSurface->Fill(0);
		}
	};

	BlackSurface(DSurface::Alternate);
	BlackSurface(DSurface::Composite);
	BlackSurface(DSurface::Hidden);
	BlackSurface(DSurface::Hidden_2);
	BlackSurface(DSurface::Primary);
	BlackSurface(DSurface::Sidebar);
	BlackSurface(DSurface::Tile);

	ShowCursor(TRUE);
}

void Debug::FatalError(bool Dump)
{
	wchar_t Message[0x400];
	wsprintfW(Message,
		L"An internal error has been encountered and the game is unable to continue normally. "
		L"Please notify the mod's creators about this issue, or Contact Otamaa at "
		L"Discord for updates and support.\n\n"
		L"%hs",
		Phobos::readBuffer);

	Debug::Log("\nFatal Error:\n");
	Debug::Log("%s\n", Phobos::readBuffer);

	MessageBoxW(Game::hWnd, Message, L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);

	if (Dump)
	{
		// Debug::FullDump(); // Not Supported
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
#pragma endregion


static DWORD _Real_Debug_Log = 0x4A4AF9;
void __declspec(naked) _Fake_Debug_Log()
{
	// va_start(args, pFormat);
	// Console::WriteWithVArgs(pFormat, args);
	// // va_end(args);
	// No need to use va_end here.
	//
	// As Console::WriteWithVArgs uses __fastcall,
	// ECX: pFormat, EDX: args
	__asm { mov ecx, [esp + 0x4] }
	__asm { lea edx, [esp + 0x8] }
	__asm { call Console::WriteWithVArgs }
	// __asm { mov edx, 0}

	// goto original bytes
	__asm { mov eax, _Real_Debug_Log }
	__asm { jmp eax }
}

static DWORD AresLogPatchJmp1;
void __declspec(naked) AresLogPatch1()
{
	static va_list args;
	static const char* pFormat;

	__asm { mov eax, [esp + 0x4] }
	__asm { mov pFormat, eax }

	__asm { lea eax, [esp + 0x8] };
	__asm { mov args, eax }

	Console::WriteWithVArgs(pFormat, args);

	__asm { mov args, 0 }

	JMP(AresLogPatchJmp1);
}
static DWORD AresLogPatchJmp2;
void __declspec(naked) AresLogPatch2()
{
	static va_list args;
	static const char* pFormat;

	__asm { mov eax, [esp + 0x4] }
	__asm { mov pFormat, eax}

	__asm { lea eax, [esp + 0x8] };
	__asm { mov args, eax }

	Console::WriteWithVArgs(pFormat, args);

	__asm { mov args, 0 }

	JMP(AresLogPatchJmp2);
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

	PatchLog(0x4A4AC0, _Fake_Debug_Log, &_Real_Debug_Log);
	PatchLog(0x4068E0, _Fake_Debug_Log, nullptr);

	return true;
}

void Console::SetForeColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Foreground == color)
		return;

	TextAttribute.Foreground = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::SetBackColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Background == color)
		return;

	TextAttribute.Background = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::EnableUnderscore(bool enable)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Underscore == enable)
		return;

	TextAttribute.Underscore = enable;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::Release()
{
	if (NULL != ConsoleHandle)
		FreeConsole();
}

void Console::Write(const char* str, int len)
{
	if (NULL != ConsoleHandle)
		WriteConsole(ConsoleHandle, str, len, nullptr, nullptr);
}

void Console::WriteLine(const char* str, int len)
{
	Write(str, len);
	Write("\n");
}

void __fastcall Console::WriteWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(Debug::StringBuffer, pFormat, args);
	Write(Debug::StringBuffer, strlen(Debug::StringBuffer));
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

	DWORD dwOldFlag;
	VirtualProtect((LPVOID)dwAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldFlag);

	pInst = (JMP_STRUCT*)dwAddr;

	if (pdwRealFunc && pInst->opcode == 0xE9) // If this function is hooked by Ares
		*pdwRealFunc = pInst->offset + dwAddr + 5;

	pInst->offset = reinterpret_cast<DWORD>(fakeFunc) - dwAddr - 5;

	VirtualProtect((LPVOID)dwAddr, 5, dwOldFlag, NULL);
}