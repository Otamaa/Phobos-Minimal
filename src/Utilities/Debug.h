#pragma once
#include <Unsorted.h>
#include <MessageListClass.h>
#include <Phobos.h>

class AbstractClass;
class REGISTERS;
class Debug final
{
public:
	enum class Severity : int {
		None = 0,
		Verbose = 1,
		Notice = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};

	static FILE* LogFile;
	static bool LogEnabled;
	static std::wstring LogFileName;
	static std::wstring LogFileTempName;
	static char DeferredStringBuffer[0x1000];
	static char LogMessageBuffer[0x1000];
	static std::vector<std::string> DeferredLogData;

	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514
	};

	static void DumpStack(REGISTERS* R, size_t len, int startAt = 0);

	static std::wstring FullDump();
	static std::wstring FullDump(std::wstring destinationFolder);

	template <typename... TArgs>
	static void Log(bool enabled, Debug::Severity severity, const char* const pFormat, TArgs&&... args) {
		if (enabled) {
			Debug::Log(severity, pFormat, std::forward<TArgs>(args)...);
		}
	}

	template <typename... TArgs>
	static void Log(bool enabled, const char* const pFormat, TArgs&&... args) {
		if (enabled) {
			Debug::Log(pFormat, std::forward<TArgs>(args)...);
		}
	}

	template <typename... TArgs>
	static void Log(Debug::Severity severity, const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(severity, pFormat, std::forward<TArgs>(args)...);
	}

	template <typename... TArgs>
	static void Log(const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(pFormat, std::forward<TArgs>(args)...);
	}

	static void LogWithVArgs(const char* const pFormat, va_list args);

	static bool LogFileActive() {
		return Debug::LogEnabled && Debug::LogFile;
	}

	static void LogDeferred(const char* pFormat, ...);
	static void LogDeferredFinalize();

	static void LogAndMessage(const char* pFormat, ...);

	static void MakeLogFile();
	static void LogFileOpen();
	static void LogFileClose(int tag);
	static void LogFileRemove();

	static void FreeMouse();

	static void ExitGame(unsigned int code = 1u);

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char* Message, ...);
	[[noreturn]] static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
	[[noreturn]] static void FatalErrorAndExit(const char* pFormat, ...);

	static void RegisterParserError() {
		if (Phobos::Otamaa::TrackParserErrors) {
			Phobos::Otamaa::ParserErrorDetected = true;
		}
	}

	static void DumpObj(void const* data, size_t len);

	template <typename T>
	static void DumpObj(const T& object) {
		DumpObj(&object, sizeof(object));
	}

	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);

	static const char* SeverityString(Debug::Severity const severity)
	{
		switch (severity)
		{
		case Severity::Verbose:
			return "verbose";
		case Severity::Notice:
			return "notice";
		case Severity::Warning:
			return "warning";
		case Severity::Error:
			return "error";
		case Severity::Fatal:
			return "fatal";
		default:
			return "wtf";
		}
	}

	static void __cdecl LogFlushed(const char* pFormat, ...);
	static void __cdecl LogFlushed(Debug::Severity severity, const char* pFormat, ...);

	// no flushing, and unchecked
	static void __cdecl LogUnflushed(const char* pFormat, ...);
	static void LogWithVArgsUnflushed(const char* pFormat, va_list args);

	// flush unchecked
	static void Flush();
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