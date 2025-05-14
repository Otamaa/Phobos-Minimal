#pragma once

#include <Phobos.h>
#include <Phobos.CRT.h>
#include <Phobos.Defines.h>

#include <Dbghelp.h>
#include <Unsorted.h>
#include <MessageListClass.h>
#include <WWMouseClass.h>

#include <chrono>

class AbstractClass;
class REGISTERS;
class Debug final
{
public:
	static FILE* LogFile;

	static bool LogEnabled;
	static std::wstring ApplicationFilePath;
	static std::wstring DefaultFEMessage;
	static std::wstring LogFilePathName;
	static std::wstring LogFileMainName;
	static std::wstring LogFileMainFormattedName;
	static std::wstring LogFileExt;
	static std::wstring LogFileFullPath;
	static char LogMessageBuffer[0x1000];
	static char DefferedVectorBuffer[0x1000];
	static std::vector<std::string> DefferedVector;
	static bool made;

	static void InitLogger();
	static void DeactivateLogger();
	static void DetachLogger();
	static void PrepareLogFile();
	static std::wstring PrepareSnapshotDirectory();
	static void LogFileRemove();
	static void FreeMouse();

	template <typename... TArgs>
	static void NOINLINE LogInfo(const std::format_string<TArgs...> _Fmt, TArgs&&... _Args) {
		if (LogFileActive()){
			std::string fmted = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
			fmted += "\n";
			fprintf_s(Debug::LogFile, "%s" ,fmted.c_str());
			Debug::Flush();
		}
	}

	template <typename... TArgs>
	static void NOINLINE LogError(const std::format_string<TArgs...> _Fmt, TArgs&&... _Args) {
		if (LogFileActive()) {
			std::string fmted = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
			fmted += "\n";
			fprintf_s(Debug::LogFile, "%s", fmted.c_str());
			Debug::Flush();
		}
	}

	static void Log(const char* pFormat, ...)
	{
		if (Debug::LogFileActive()) {
			va_list args;
			va_start(args, pFormat);
			vfprintf(Debug::LogFile, pFormat, args);
			va_end(args);

			Debug::Flush();
		}
	}

	//this will be used to replace game debug prints
	static void __cdecl CLog(const char* pFormat, ...)
	{
		if (Debug::LogFileActive())
		{
			va_list args;
			va_start(args, pFormat);
			vfprintf(Debug::LogFile, pFormat, args);
			va_end(args);

			Debug::Flush();
		}
	}

	// Log file not checked
	static void LogUnflushed(const char* pFormat, ...)
	{
		va_list args;
		va_start(args, pFormat);
		vfprintf(Debug::LogFile, pFormat, args);
		va_end(args);
	}

	void Debug::LogFlushed(const char* const pFormat, ...)
	{
		if (Debug::LogFileActive()) {
			va_list args;
			va_start(args, pFormat);
			vfprintf(Debug::LogFile, pFormat, args);
			Debug::Flush();
			va_end(args);
		}
	}

	static FORCEINLINE void Flush() {
		fflush(Debug::LogFile);
	}

	static void LogDeferred(const char* pFormat, ...) {
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(DefferedVectorBuffer, sizeof(DefferedVectorBuffer), pFormat, args);
		Debug::DefferedVector.emplace_back(DefferedVectorBuffer);
		va_end(args);
	}

	static void LogDeferredFinalize()
	{
		if (Debug::LogFileActive()) {
			for (auto& __log : Debug::DefferedVector) {
				fprintf_s(Debug::LogFile, "%s", __log.c_str());
			}
		}

		Debug::DefferedVector.clear();
	}

	static FORCEDINLINE void TakeMouse()
	{
		WWMouseClass::Instance->ReleaseMouse();
		Imports::ShowCursor.get()(1);
	}

	static FORCEDINLINE void ReturnMouse()
	{
		Imports::ShowCursor.get()(0);
		WWMouseClass::Instance->CaptureMouse();
	}

	static NOINLINE void DumpStack(REGISTERS* R, size_t len, int startAt = 0);

	static COMPILETIMEEVAL FORCEDINLINE bool LogFileActive() {
		return Debug::LogEnabled && Debug::LogFile;
	}

	template <typename... TArgs>
	static NOINLINE void LogAndMessage(const char* pFormat, TArgs&&... args)
	{
		if (!Debug::LogFileActive())
			return;

		IMPL_SNPRNINTF(Debug::LogMessageBuffer, sizeof(LogMessageBuffer), pFormat, std::forward<TArgs>(args)...);
		static wchar_t buffer[0x1000];
		mbstowcs(buffer, Debug::LogMessageBuffer, 0x1000);
		MessageListClass::Instance->PrintMessage(buffer);
	}

	static NOINLINE std::wstring GetCurTime()
	{
		const auto now = std::chrono::system_clock::now();
		const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		const std::tm* localTime = std::localtime(&currentTime);

		return std::format(L"{:04}{:02}{:02}-{:02}{:02}{:02}",
			localTime->tm_year + 1900,
			localTime->tm_mon + 1 ,
			localTime->tm_mday,
			localTime->tm_hour,
			localTime->tm_min,
			localTime->tm_sec);
	}

	static NOINLINE std::string GetCurTimeA()
	{
		const auto now = std::chrono::system_clock::now();
		const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		const std::tm* localTime = std::localtime(&currentTime);

		return std::format("{:04}{:02}{:02}-{:02}{:02}{:02}",
			localTime->tm_year + 1900,
			localTime->tm_mon + 1,
			localTime->tm_mday,
			localTime->tm_hour,
			localTime->tm_min,
			localTime->tm_sec);
	}

	static FORCEDINLINE std::wstring FullDump(PMINIDUMP_EXCEPTION_INFORMATION const pException = nullptr)
	{
		return FullDump(Debug::PrepareSnapshotDirectory(), pException);
	}

	static NOINLINE std::wstring FullDump(
		std::wstring destinationFolder,
		PMINIDUMP_EXCEPTION_INFORMATION const pException = nullptr)
	{
		std::wstring filename = std::move(destinationFolder);
		filename += L"\\extcrashdump.dmp";

		HANDLE dumpFile = CreateFileW(filename.c_str(), GENERIC_WRITE,
			0, nullptr, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, nullptr);

		MINIDUMP_TYPE type = static_cast<MINIDUMP_TYPE>(MiniDumpNormal
									   | MiniDumpWithDataSegs
									   | MiniDumpWithIndirectlyReferencedMemory
									   | MiniDumpWithFullMemory
		);

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, type, pException, nullptr, nullptr);
		CloseHandle(dumpFile);

		return filename;
	}

	template<bool ImmedietelyExit= false>
	[[noreturn]] static NOINLINE void ExitGame(unsigned int code = 1u)
	{
		Phobos::ExeTerminate();
		if constexpr (!ImmedietelyExit)
			CRT::exit_returnsomething(code, 0, 0);
		else
			exit(code);
	}

	static COMPILETIMEEVAL void GenerateDefaultMessage (){
		if(DefaultFEMessage.empty()){
			std::wstring first { L"An internal error has been encountered and the game is unable to continue normally. \n" };
			std::wstring second { L"Please notify the mod's creators about this issue, or Contact Otamaa at \n" };
			std::wstring third { L"Discord for updates and support.\n" };
			DefaultFEMessage = first + second + third;
		}
	}

	static NOINLINE void FatalErrorCore(bool Dump, const std::string& msg);

	template <typename... TArgs>
	static NOINLINE void FatalError(const char* Message, TArgs&&... args)
	{
		Debug::FreeMouse();
		IMPL_SNPRNINTF(Debug::LogMessageBuffer , sizeof(Debug::LogMessageBuffer) , Message , std::forward<TArgs>(args)...);
		Debug::FatalErrorCore(false, Debug::LogMessageBuffer);
	}

	template <typename... TArgs>
	[[noreturn]] static NOINLINE void FatalErrorAndExit(size_t nExitCode, const char* Message, TArgs&&... args)
	{
		IMPL_SNPRNINTF(Debug::LogMessageBuffer, sizeof(Debug::LogMessageBuffer), Message, std::forward<TArgs>(args)...);
		Debug::FatalErrorCore(Phobos::Config::DebugFatalerrorGenerateDump , Debug::LogMessageBuffer);
		Debug::ExitGame(nExitCode);
	}

	template <typename... TArgs>
	[[noreturn]] static FORCEDINLINE void FatalErrorAndExit(const char* pFormat, TArgs&&... args)
	{
		FatalErrorAndExit(0u, pFormat, std::forward<TArgs>(args)...);
	}

	template <typename... TArgs>
	[[noreturn]] static FORCEDINLINE void FatalErrorAndExitIfTrue(bool condition , const char* pFormat, TArgs&&... args)
	{
		if(condition)
			FatalErrorAndExit(0u, pFormat, std::forward<TArgs>(args)...);
	}

	static void RegisterParserError() {
		if (Phobos::Otamaa::TrackParserErrors) {
			Phobos::Otamaa::ParserErrorDetected = true;
		}
	}

	static NOINLINE void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
};
