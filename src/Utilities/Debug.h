#pragma once
#include <Unsorted.h>
#include <MessageListClass.h>
#include <Phobos.h>

class AbstractClass;
class REGISTERS;
class Debug final
{
public:

	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514
	};

	static char StringBuffer[0x1000];
	static char DeferredStringBuffer[0x1000];
	static int CurrentBufferSize;

	static void FreeMouse();
	static void Log(const char* pFormat, ...);
	static void LogDeferred(const char* pFormat, ...);
	static void LogDeferredFinalize();
	static void LogAndMessage(const char* pFormat, ...);
	static void LogWithVArgs(const char* pFormat, va_list args);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	[[noreturn]] static void FatalErrorAndExit(const char* pFormat, ...);

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char* Message, ...);
	[[noreturn]] static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);

	static void RegisterParserError() {
		if (Phobos::Otamaa::TrackParserErrors) {
			Phobos::Otamaa::ParserErrorDetected = true;
		}
	}

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

	static void ExitGame();
private:
	static void LogWithVArgsUnflushed(const char* pFormat, va_list args);
#pragma endregion

	//static void __cdecl WriteLog(const char* pFormat, ...);
};

class Console
{
public:

	enum class ConsoleColor
	{
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkRed = 4,
		Intensity = 8,

		DarkCyan = DarkBlue | DarkGreen,
		DarkMagenta = DarkBlue | DarkRed,
		DarkYellow = DarkGreen | DarkRed,
		Gray = DarkBlue | DarkGreen | DarkRed,
		DarkGray = Black | Intensity,

		Blue = DarkBlue | Intensity,
		Green = DarkGreen | Intensity,
		Red = DarkRed | Intensity,
		Cyan = Blue | Green,
		Magenta = Blue | Red,
		Yellow = Green | Red,
		White = Red | Green | Blue,
	};

	union ConsoleTextAttribute
	{
		WORD AsWord;
		struct
		{
			ConsoleColor Foreground : 4;
			ConsoleColor Background : 4;
			bool LeadingByte : 1;
			bool TrailingByte : 1;
			bool GridTopHorizontal : 1;
			bool GridLeftVertical : 1;
			bool GridRightVerticle : 1;
			bool ReverseVideo : 1; // Reverse fore/back ground attribute
			bool Underscore : 1;
			bool Unused : 1;
		};
	};
	static ConsoleTextAttribute TextAttribute;
	static HANDLE ConsoleHandle;

	static bool Create();
	static void Release();

	template<size_t Length>
	constexpr static void Write(const char(&str)[Length])
	{
		Write(str, Length - 1); // -1 because there is a '\0' here
	}

	static void SetForeColor(ConsoleColor color);
	static void SetBackColor(ConsoleColor color);
	static void EnableUnderscore(bool enable);
	static void Write(const char* str, int len);
	static void WriteLine(const char* str, int len);
	static void __fastcall WriteWithVArgs(const char* pFormat, va_list args);
	static void WriteFormat(const char* pFormat, ...);

private:
	static void PatchLog(DWORD dwAddr, void* realFunc, DWORD* pdwRealFunc);
};