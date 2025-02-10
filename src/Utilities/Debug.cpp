#include "Debug.h"
#include "Macro.h"
#include <Phobos.h>


#include <MouseClass.h>
#include <Surface.h>

#include <CRT.h>
#include <YRPPCore.h>
#include <AbstractClass.h>
#include <vector>

#include <AnimClass.h>
#include <BuildingClass.h>
#include <BulletClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>

void Debug::DumpStack(REGISTERS* R, size_t len, int startAt)
{
	if (!Debug::LogFileActive()) {
		return;
	}

	Debug::LogUnflushed("Dumping %X bytes of stack\n", len);
	auto const end = len / 4;
	auto const* const mem = R->lea_Stack<DWORD*>(startAt);
	for (auto i = 0u; i < end; ++i)
	{

		const char* suffix = "";
		const char* Object = "";
		const uintptr_t ptr = mem[i];
		if (ptr >= 0x401000 && ptr <= 0xB79BE4)
			suffix = "GameMemory!";
		else
		{
			for (auto begin = Patch::ModuleDatas.begin() + 1; begin != Patch::ModuleDatas.end(); ++begin)
			{
				if (ptr >= begin->BaseAddr && ptr <= (begin->BaseAddr + begin->Size))
				{
					suffix = (begin->ModuleName + " Memory!").c_str();
					break;
				}
			}
		}

		if (ptr != 0u && ptr != std::numeric_limits<uintptr_t>::max() && ptr != std::numeric_limits<uintptr_t>::min())
		{
			switch (VTable::Get((mem + i)))
			{
#define DECLARE_VTABLE_STRING(x) case x::vtable: Object = #x; break;
				DECLARE_VTABLE_STRING(AnimClass)
				DECLARE_VTABLE_STRING(UnitClass)
				DECLARE_VTABLE_STRING(AircraftClass)
				DECLARE_VTABLE_STRING(InfantryClass)
				DECLARE_VTABLE_STRING(BuildingClass)
				DECLARE_VTABLE_STRING(WeaponTypeClass)
				DECLARE_VTABLE_STRING(WarheadTypeClass)
				DECLARE_VTABLE_STRING(BulletClass)
				DECLARE_VTABLE_STRING(BulletTypeClass)
				DECLARE_VTABLE_STRING(HouseClass)
				DECLARE_VTABLE_STRING(HouseTypeClass)
#undef DECLARE_VTABLE_STRING
			default:
				break;
			}
		}
		Debug::LogUnflushed("esp+%04X = %08X %s %s\n", i * 4, mem[i], suffix , Object);
	}

	Debug::Log("====================Done.\n"); // flushes
}
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
