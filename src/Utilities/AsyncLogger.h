#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <atomic>

#include <Lib/spdlog/spdlog.h>
#include <Lib/spdlog/async.h>
#include <Lib/spdlog/sinks/basic_file_sink.h>
#include <Lib/spdlog/sinks/stdout_color_sinks.h>
#include <Lib/spdlog/pattern_formatter.h>

/**
 * AsyncLogger - Thread-independent logging system for DLL projects
 *
 * Features:
 * - Completely separate thread operation
 * - Non-blocking when main thread stalls
 * - Customizable log file names
 * - Both old printf-style and modern fmt formatting
 * - Clear flush state visibility
 * - MSVC-compatible without CMake
 *
 * Usage:
 *   AsyncLogger::Initialize("MyLog.log");
 *   AsyncLogger::Info("Player spawned at %d, %d", x, y);  // printf-style
 *   AsyncLogger::InfoFmt("Player spawned at {}, {}", x, y);  // fmt-style
 *   AsyncLogger::Flush();  // Ensure everything is written
 *   AsyncLogger::Shutdown();
 */
class AsyncLogger
{
public:
	// Lifecycle Management

	/**
	 * Initialize the async logger with a custom log file name
	 * @param logFileName Name of the log file (e.g., "Phobos.log")
	 * @param threadPoolSize Number of background threads (default: 1)
	 * @param queueSize Size of the async queue (default: 8192)
	 * @return true if initialization succeeded
	 */
	static bool Initialize(const char* logFileName,
						  size_t threadPoolSize = 1,
						  size_t queueSize = 8192);

	/**
	 * Shutdown the logger and flush all pending messages
	 * This is blocking and ensures all logs are written before returning
	 */
	static void Shutdown();

	/**
	 * Check if logger is initialized and ready
	 */
	static bool IsInitialized();

	// Session Management

	/**
	 * Start a new logging session with a separator
	 * @param sessionName Optional session identifier
	 */
	static void BeginSession(const char* sessionName = nullptr);

	/**
	 * End current session and flush all pending logs
	 */
	static void EndSession();

	// Flush Control

	/**
	 * Flush all pending log messages to disk (blocking)
	 * Returns after all messages are written
	 */
	static void Flush();

	/**
	 * Check if there are pending messages in the queue
	 * @return true if messages are waiting to be written
	 */
	static bool HasPendingMessages();

	/**
	 * Get approximate number of pending messages
	 */
	static size_t GetPendingCount();

	// Modern fmt-style Logging (C++20-style format strings)

	template<typename... Args>
	static void TraceFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	template<typename... Args>
	static void DebugFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	template<typename... Args>
	static void InfoFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	template<typename... Args>
	static void WarnFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	template<typename... Args>
	static void ErrorFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	template<typename... Args>
	static void CriticalFmt(const fmt::format_string<Args...> _Fmt, Args&&... args);

	// Old printf-style Logging (for compatibility)

	static void Trace(const char* fmt, ...);
	static void Debug(const char* fmt, ...);
	static void Info(const char* fmt, ...);
	static void Warn(const char* fmt, ...);
	static void Error(const char* fmt, ...);
	static void Critical(const char* fmt, ...);

	// Direct string logging (no formatting)

	static void TraceRaw(const char* message);
	static void DebugRaw(const char* message);
	static void InfoRaw(const char* message);
	static void WarnRaw(const char* message);
	static void ErrorRaw(const char* message);
	static void CriticalRaw(const char* message);

	// Configuration

	/**
	 * Set the logging level
	 * 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical, 6=off
	 */
	static void SetLevel(int level);

	/**
	 * Enable/disable console output (in addition to file)
	 */
	static void SetConsoleOutput(bool enabled);

	/**
	 * Set custom log pattern
	 * Default: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
	 * %Y=year, %m=month, %d=day, %H=hour, %M=minute, %S=second, %e=millisecond
	 * %l=level, %v=message
	 */
	static void SetPattern(const char* pattern);

	static void FormatAndStripNewline(const char* pFormat, va_list args, std::vector<char>& buffer);
private:
	// Internal implementation
	static std::shared_ptr<spdlog::logger> s_Logger;
	static std::atomic<bool> s_Initialized;
	static std::atomic<size_t> s_PendingCount;
	static std::mutex s_InitMutex;
	static std::string s_CurrentLogFile;

	// Helper for printf-style formatting
	static std::string FormatPrintf(const char* fmt, va_list args);

	// Prevent instantiation
	AsyncLogger() = delete;
	~AsyncLogger() = delete;
	AsyncLogger(const AsyncLogger&) = delete;
	AsyncLogger& operator=(const AsyncLogger&) = delete;
};

// Template implementations (must be in header)

#include <spdlog/spdlog.h>
#include <spdlog/async.h>

template<typename... Args>
void AsyncLogger::TraceFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->trace(_Fmt, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void AsyncLogger::DebugFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->debug(_Fmt, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void AsyncLogger::InfoFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->info(_Fmt, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void AsyncLogger::WarnFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->warn(_Fmt, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void AsyncLogger::ErrorFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->error(_Fmt, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void AsyncLogger::CriticalFmt(const fmt::format_string<Args...> _Fmt, Args&&... args)
{
	if (s_Initialized && s_Logger)
	{
		s_PendingCount++;
		s_Logger->critical(_Fmt, std::forward<Args>(args)...);
	}
}