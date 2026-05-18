#include "AsyncLogger.h"

#include <cstdarg>
#include <cstdio>
#include <vector>

// Static member initialization
std::shared_ptr<spdlog::logger> AsyncLogger::s_Logger = nullptr;
std::atomic<bool> AsyncLogger::s_Initialized(false);
std::atomic<size_t> AsyncLogger::s_PendingCount(0);
std::mutex AsyncLogger::s_InitMutex;
std::string AsyncLogger::s_CurrentLogFile;
FILE* AsyncLogger::s_LegacyFileHandle = nullptr;

void AsyncLogger::FormatAndStripNewline(const char* pFormat, va_list args, std::vector<char>& buffer)
{
	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, pFormat, args_copy);
	va_end(args_copy);

	if (size > 0)
	{
		buffer.resize(size + 1);
		std::vsnprintf(buffer.data(), buffer.size(), pFormat, args);

		std::string_view view(buffer.data(), size);
		int tail_trim_count = 0;

		while (!view.empty() && (view.back() == '\n' || view.back() == '\r'))
		{
			view.remove_suffix(1);
			tail_trim_count++;
		}

		if (tail_trim_count > 0)
		{
			buffer[size - tail_trim_count] = '\0';
		}
	}
}

bool AsyncLogger::Initialize(const char* logFileName, size_t threadPoolSize, size_t queueSize, FILE** legacyFilePtr)
{
	std::lock_guard<std::mutex> lock(s_InitMutex);

	if (s_Initialized)
	{
		// If already initialized and legacy handle requested, provide it
		if (legacyFilePtr && s_LegacyFileHandle)
		{
			*legacyFilePtr = s_LegacyFileHandle;
		}
		return true;  // Already initialized
	}

	try
	{
		// Store log file name
		s_CurrentLogFile = logFileName ? logFileName : "default.log";

		// Open legacy FILE* handle if requested (for backward compatibility)
		if (legacyFilePtr)
		{
			// Open with write+share deny write (same as _wfsopen with _SH_DENYWR)
#ifdef _WIN32
			errno_t err = fopen_s(&s_LegacyFileHandle, s_CurrentLogFile.c_str(), "w");
			if (err != 0)
			{
				s_LegacyFileHandle = nullptr;
			}
#else
			s_LegacyFileHandle = fopen(s_CurrentLogFile.c_str(), "w");
#endif

			if (s_LegacyFileHandle)
			{
				*legacyFilePtr = s_LegacyFileHandle;
			}
		}

		// Initialize spdlog's thread pool for async logging
		// This creates background threads that are completely independent
		spdlog::init_thread_pool(queueSize, threadPoolSize);

		// Create async file sink
		// Note: spdlog will append "_async" or use the file we specify
		// To avoid conflicts with legacy FILE*, we could use a different approach
		// But typically spdlog handles this fine as it buffers writes
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
			s_CurrentLogFile,
			false  // append mode to work with legacy FILE* (set to true to truncate)
		);

		// Create async logger with the file sink
		s_Logger = std::make_shared<spdlog::async_logger>(
			"async_logger",
			file_sink,
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block  // block when queue is full (safer)
		);

		// Set default pattern: [timestamp] [level] message
		s_Logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

		// Set default level to trace (log everything)
		s_Logger->set_level(spdlog::level::trace);

		// Flush on every error and critical message
		s_Logger->flush_on(spdlog::level::err);

		s_Initialized = true;
		s_PendingCount = 0;

		// Log initialization message
		s_Logger->info("=== AsyncLogger Initialized ===");
		s_Logger->info("Log File: {}", s_CurrentLogFile);
		s_Logger->info("Thread Pool Size: {}", threadPoolSize);
		s_Logger->info("Queue Size: {}", queueSize);
		if (s_LegacyFileHandle)
		{
			s_Logger->info("Legacy FILE* handle created for backward compatibility");
		}
		s_Logger->flush();

		return true;
	}
	catch (const std::exception& e)
	{
		// Initialization failed - try to output error somewhere
		fprintf(stderr, "AsyncLogger::Initialize failed: %s\n", e.what());

		// Clean up legacy handle if it was opened
		if (s_LegacyFileHandle)
		{
			fclose(s_LegacyFileHandle);
			s_LegacyFileHandle = nullptr;
		}

		s_Initialized = false;
		s_Logger = nullptr;
		return false;
	}
}

void AsyncLogger::Shutdown()
{
	std::lock_guard<std::mutex> lock(s_InitMutex);

	if (!s_Initialized)
	{
		return;
	}

	if (s_Logger)
	{
		s_Logger->info("=== AsyncLogger Shutting Down ===");
		s_Logger->info("Flushing remaining messages...");

		// Flush all pending messages (blocking)
		s_Logger->flush();

		// Drop the logger (important for cleanup)
		spdlog::drop("async_logger");
		s_Logger = nullptr;
	}

	// Close legacy FILE* handle if it exists
	if (s_LegacyFileHandle)
	{
		fflush(s_LegacyFileHandle);
		fclose(s_LegacyFileHandle);
		s_LegacyFileHandle = nullptr;
	}

	// Shutdown the thread pool (waits for all threads to finish)
	spdlog::shutdown();

	s_Initialized = false;
	s_PendingCount = 0;
}

bool AsyncLogger::IsInitialized()
{
	return s_Initialized.load();
}

void AsyncLogger::BeginSession(const char* sessionName)
{
	if (!s_Initialized || !s_Logger) return;

	s_Logger->info("");
	s_Logger->info("======================================");
	if (sessionName)
	{
		s_Logger->info("=== Session: {} ===", sessionName);
	}
	else
	{
		s_Logger->info("=== New Session ===");
	}
	s_Logger->info("======================================");
	s_Logger->info("");
	s_Logger->flush();

	// Also write to legacy handle if it exists
	if (s_LegacyFileHandle)
	{
		fprintf(s_LegacyFileHandle, "\n======================================\n");
		if (sessionName)
		{
			fprintf(s_LegacyFileHandle, "=== Session: %s ===\n", sessionName);
		}
		else
		{
			fprintf(s_LegacyFileHandle, "=== New Session ===\n");
		}
		fprintf(s_LegacyFileHandle, "======================================\n\n");
		fflush(s_LegacyFileHandle);
	}
}

void AsyncLogger::EndSession()
{
	if (!s_Initialized || !s_Logger) return;

	s_Logger->info("");
	s_Logger->info("======================================");
	s_Logger->info("=== Session End ===");
	s_Logger->info("======================================");
	s_Logger->info("");
	s_Logger->flush();

	// Also write to legacy handle if it exists
	if (s_LegacyFileHandle)
	{
		fprintf(s_LegacyFileHandle, "\n======================================\n");
		fprintf(s_LegacyFileHandle, "=== Session End ===\n");
		fprintf(s_LegacyFileHandle, "======================================\n\n");
		fflush(s_LegacyFileHandle);
	}
}

void AsyncLogger::Flush()
{
	if (s_Logger)
	{
		s_Logger->flush();
		s_PendingCount = 0;  // Reset counter after flush
	}

	// Also flush legacy handle if it exists
	if (s_LegacyFileHandle)
	{
		fflush(s_LegacyFileHandle);
	}
}

bool AsyncLogger::HasPendingMessages()
{
	return s_PendingCount.load() > 0;
}

size_t AsyncLogger::GetPendingCount()
{
	return s_PendingCount.load();
}

FILE* AsyncLogger::GetLegacyFileHandle()
{
	return s_LegacyFileHandle;
}

// Helper function for printf-style formatting
std::string AsyncLogger::FormatPrintf(const char* fmt, va_list args)
{
	// First pass: determine required size
	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
	va_end(args_copy);

	if (size < 0)
	{
		return "[Format Error]";
	}

	// Second pass: actually format
	std::vector<char> buffer(size + 1);
	std::vsnprintf(buffer.data(), buffer.size(), fmt, args);

	return std::string(buffer.data());
}

// Printf-style logging implementations

void AsyncLogger::Trace(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->trace(message);
}

void AsyncLogger::Debug(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->debug(message);
}

void AsyncLogger::Info(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->info(message);
}

void AsyncLogger::Warn(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->warn(message);
}

void AsyncLogger::Error(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->error(message);
	// Errors are auto-flushed due to flush_on(err) setting
}

void AsyncLogger::Critical(const char* fmt, ...)
{
	if (!s_Initialized || !s_Logger) return;

	va_list args;
	va_start(args, fmt);
	std::string message = FormatPrintf(fmt, args);
	va_end(args);

	s_PendingCount++;
	s_Logger->critical(message);
	// Critical messages are auto-flushed
}

// Raw string logging (no formatting)

void AsyncLogger::TraceRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->trace(message);
}

void AsyncLogger::DebugRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->debug(message);
}

void AsyncLogger::InfoRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->info(message);
}

void AsyncLogger::WarnRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->warn(message);
}

void AsyncLogger::ErrorRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->error(message);
}

void AsyncLogger::CriticalRaw(const char* message)
{
	if (!s_Initialized || !s_Logger) return;
	s_PendingCount++;
	s_Logger->critical(message);
}

// Configuration functions

void AsyncLogger::SetLevel(int level)
{
	if (!s_Logger) return;

	spdlog::level::level_enum spdlog_level;
	switch (level)
	{
	case 0: spdlog_level = spdlog::level::trace; break;
	case 1: spdlog_level = spdlog::level::debug; break;
	case 2: spdlog_level = spdlog::level::info; break;
	case 3: spdlog_level = spdlog::level::warn; break;
	case 4: spdlog_level = spdlog::level::err; break;
	case 5: spdlog_level = spdlog::level::critical; break;
	case 6: spdlog_level = spdlog::level::off; break;
	default: spdlog_level = spdlog::level::info;
	}

	s_Logger->set_level(spdlog_level);
}

void AsyncLogger::SetConsoleOutput(bool enabled)
{
	std::lock_guard<std::mutex> lock(s_InitMutex);

	if (!s_Logger) return;

	if (enabled)
	{
		// Add console sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

		// Create new logger with both file and console sinks
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
			s_CurrentLogFile,
			false  // append mode
		);

		std::vector<spdlog::sink_ptr> sinks { file_sink, console_sink };

		auto old_level = s_Logger->level();
		spdlog::drop("async_logger");

		s_Logger = std::make_shared<spdlog::async_logger>(
			"async_logger",
			sinks.begin(),
			sinks.end(),
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
		);

		s_Logger->set_level(old_level);
		s_Logger->flush_on(spdlog::level::err);
	}
}

void AsyncLogger::SetPattern(const char* pattern)
{
	if (!s_Logger) return;
	s_Logger->set_pattern(pattern);
}