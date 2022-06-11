#include "Debug.h"
#include "Macro.h"

#include <YRPPCore.h>
#include <AbstractClass.h>

void Debug::Log(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	const char* LogMessage = (Message == nullptr)
		? "Failed to parse INI file content: [%s]%s=%s\n"
		: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

	Debug::Log(LogMessage, section, flag, value, Message);
}

void Debug::FatalErrorAndExit(const char* pFormat, ...)
{
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Debug::Log(buffer);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Debug::Log(buffer);
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
	char buffer[0x400];
	vsprintf_s(buffer, pFormat, args);
	Debug::Log(buffer);
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
#pragma endregion