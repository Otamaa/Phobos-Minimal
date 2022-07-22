#pragma once
#include <Unsorted.h>
#include <MessageListClass.h>
#include <Phobos.h>

class AbstractClass;
class REGISTERS;
class Debug final
{
	NO_CONSTRUCT_CLASS(Debug)
public:
	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514
	};

	static void FreeMouse();
	static void __cdecl Log(const char* pFormat, ...);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	static void FatalErrorAndExit(const char* pFormat, ...);

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char* Message, ...);
	[[noreturn]] static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);

#pragma region Otamaa
	static void DumpStack(const char* function, REGISTERS* R, size_t len, int startAt = 0);
	static void DumpObj(void const* data, size_t len);
	static void TestPointer(AbstractClass* pAbstract);

	template <typename T>
	static void DumpObj(const T& object)
	{
		DumpObj(&object, sizeof(object));
	}

	template <typename... TArgs>
	static void Log_WithBool(bool bDisable, const char* const pFormat, TArgs&&... args)
	{
		if (!bDisable)
		{
			Debug::Log(pFormat, std::forward<TArgs>(args)...);
		}
	}

	template <typename ... TArgs>
	static void Log_Masselist(const char* const pFormat, TArgs&&... args)
	{
		char buffer[0x800] = { 0 };

		auto append = [&buffer](const char* pFormat, ...)
		{
			va_list args;
			va_start(args, pFormat);
			vsprintf_s(Phobos::readBuffer, pFormat, args);
			va_end(args);
			strcat_s(buffer, Phobos::readBuffer);
		};

		auto display = [&buffer]()
		{
			memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
			size_t nCharConvertedRecord;
			mbstowcs_s(&nCharConvertedRecord, Phobos::wideBuffer, buffer, strlen(buffer));
				UNREFERENCED_PARAMETER(nCharConvertedRecord);
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
			Debug::Log("%s\n", buffer);
			buffer[0] = 0;
		};

		append(pFormat, std::forward<TArgs>(args)...);
		display();
	}

	static void Time(char* ret)
	{
		SYSTEMTIME Sys;
		GetLocalTime(&Sys);
		sprintf_s(ret, 24, "%04d/%02d/%02d %02d:%02d:%02d:%03d",
			Sys.wYear, Sys.wMonth, Sys.wDay, Sys.wHour, Sys.wMinute,
			Sys.wSecond, Sys.wMilliseconds);
	}

	static void DumpStack(const char* function, size_t len, int startAt = 0);
	static void __cdecl LogUnflushed(const char* pFormat, ...);

private:
	static void LogWithVArgsUnflushed(const char* pFormat, va_list args);
#pragma endregion
};
