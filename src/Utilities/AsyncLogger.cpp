#include "AsyncLogger.h"

#include <cstdarg>
#include <cstdio>
#include <vector>


#ifdef __EXAMPLES
#include "AsyncLogger.h"
#include <thread>
#include <chrono>
#include <Windows.h>

// Example 1: Basic usage
void Example_BasicUsage()
{
	// Initialize with custom log name
	AsyncLogger::Initialize("Phobos.log");

	// Simple logging
	AsyncLogger::Info("Game initialized");
	AsyncLogger::Debug("Loading configuration...");
	AsyncLogger::Warn("Deprecated feature used");
	AsyncLogger::Error("Failed to load texture: %s", "tank.shp");

	// Flush to ensure everything is written
	AsyncLogger::Flush();

	AsyncLogger::Shutdown();
}

// Example 2: Printf-style formatting (old school, for compatibility)
void Example_PrintfStyle()
{
	AsyncLogger::Initialize("Combat.log");

	int health = 100;
	int armor = 50;
	float damage = 25.5f;
	const char* unit = "Tank";

	// Traditional printf-style formatting
	AsyncLogger::Info("Unit: %s, Health: %d, Armor: %d", unit, health, armor);
	AsyncLogger::Info("Damage dealt: %.2f", damage);
	AsyncLogger::Info("Position: (%d, %d)", 1024, 768);

	// Works with all log levels
	AsyncLogger::Debug("Debug info: %d %d %d", 1, 2, 3);
	AsyncLogger::Warn("Warning: Resource count = %d", 42);
	AsyncLogger::Error("Error code: 0x%08X", 0xDEADBEEF);

	AsyncLogger::Flush();
	AsyncLogger::Shutdown();
}

// Example 3: Modern fmt-style formatting
void Example_FmtStyle()
{
	AsyncLogger::Initialize("Modern.log");

	std::string playerName = "Commander";
	int score = 9001;

	// Modern fmt-style formatting (cleaner, type-safe)
	AsyncLogger::InfoFmt("Player: {}, Score: {}", playerName, score);
	AsyncLogger::InfoFmt("Hex value: 0x{:08X}", 0xCAFEBABE);
	AsyncLogger::InfoFmt("Float: {:.3f}", 3.14159265);

	// Can mix types easily
	AsyncLogger::WarnFmt("Mixed types: {} {} {} {}", 42, "string", 3.14, true);

	AsyncLogger::Flush();
	AsyncLogger::Shutdown();
}

// Example 4: Thread independence demonstration
void BackgroundTask()
{
	for (int i = 0; i < 10; i++)
	{
		AsyncLogger::Info("Background thread: iteration %d", i);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void Example_ThreadIndependence()
{
	AsyncLogger::Initialize("Threading.log", 2);  // Use 2 background threads

	AsyncLogger::Info("Starting background task...");

	// Launch background thread
	std::thread worker(BackgroundTask);

	// Main thread can continue logging without blocking
	for (int i = 0; i < 10; i++)
	{
		AsyncLogger::Info("Main thread: iteration %d", i);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	worker.join();

	// Check if messages are still pending
	if (AsyncLogger::HasPendingMessages())
	{
		AsyncLogger::InfoFmt("Pending messages: {}", AsyncLogger::GetPendingCount());
		AsyncLogger::Flush();  // Force flush
	}

	AsyncLogger::Shutdown();
}

// Example 5: Session management
void Example_Sessions()
{
	AsyncLogger::Initialize("GameSessions.log");

	// First game session
	AsyncLogger::BeginSession("Tutorial Mission");
	AsyncLogger::Info("Player entered tutorial");
	AsyncLogger::Info("Objective: Build 5 tanks");
	AsyncLogger::Info("Tutorial completed");
	AsyncLogger::EndSession();

	// Second game session
	AsyncLogger::BeginSession("Mission 1: Red Alert");
	AsyncLogger::Info("Mission started");
	AsyncLogger::Warn("Low power warning");
	AsyncLogger::Info("Mission objective completed");
	AsyncLogger::EndSession();

	AsyncLogger::Shutdown();
}

// Example 6: DLL usage pattern (typical Phobos usage)
class GameSystem
{
public:
	static void Initialize()
	{
		AsyncLogger::Initialize("Phobos.log", 1, 16384);  // Larger queue for game
		AsyncLogger::BeginSession("Phobos Extension");
		AsyncLogger::Info("Phobos version: 1.0.0");
		AsyncLogger::Info("Game: Red Alert 2 - Yuri's Revenge");
	}

	static void OnGameStart()
	{
		AsyncLogger::BeginSession("New Game");
		AsyncLogger::Info("Map: XMP01T4.map");
		AsyncLogger::Info("Difficulty: Hard");
	}

	static void OnUpdate(int frameCount)
	{
		// Log sparingly in update loop
		if (frameCount % 1000 == 0)
		{
			AsyncLogger::Debug("Frame: %d", frameCount);
		}
	}

	static void OnError(const char* system, const char* error)
	{
		// Errors are auto-flushed
		AsyncLogger::Error("[%s] %s", system, error);
	}

	static void OnGameEnd()
	{
		AsyncLogger::EndSession();
		AsyncLogger::InfoFmt("Game ended at frame {}", 50000);
	}

	static void Shutdown()
	{
		AsyncLogger::Info("Shutting down Phobos...");
		AsyncLogger::EndSession();
		AsyncLogger::Shutdown();  // Blocking, ensures all logs are written
	}
};

// Example 7: Checking flush status
void Example_FlushStatus()
{
	AsyncLogger::Initialize("FlushTest.log");

	// Generate lots of logs
	for (int i = 0; i < 1000; i++)
	{
		AsyncLogger::Info("Log message %d", i);
	}

	// Check if messages are still in queue
	while (AsyncLogger::HasPendingMessages())
	{
		size_t pending = AsyncLogger::GetPendingCount();
		printf("Still writing... Pending: %zu\n", pending);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	printf("All messages flushed to disk!\n");

	AsyncLogger::Shutdown();
}

// Example 8: Configuration options
void Example_Configuration()
{
	AsyncLogger::Initialize("Config.log");

	// Change log level (only info and above will be logged)
	AsyncLogger::SetLevel(2);  // 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical

	AsyncLogger::Trace("This won't appear (trace)");
	AsyncLogger::Debug("This won't appear (debug)");
	AsyncLogger::Info("This will appear (info)");
	AsyncLogger::Warn("This will appear (warn)");

	// Custom pattern
	AsyncLogger::SetPattern("[%H:%M:%S] [%l] %v");  // Simpler format
	AsyncLogger::Info("Custom pattern message");

	// Restore default pattern
	AsyncLogger::SetPattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

	// Enable console output (useful for debugging)
	// AsyncLogger::SetConsoleOutput(true);

	AsyncLogger::Shutdown();
}

// Example 9: DLL main usage (typical pattern)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize logger when DLL loads
		GameSystem::Initialize();
		break;

	case DLL_PROCESS_DETACH:
		// Clean shutdown when DLL unloads
		GameSystem::Shutdown();
		break;
	}
	return TRUE;
}

// Example 10: Error recovery
void Example_ErrorRecovery()
{
	// Multiple initializations are safe
	AsyncLogger::Initialize("Test1.log");
	AsyncLogger::Initialize("Test2.log");  // Ignored, already initialized

	AsyncLogger::Info("This goes to Test1.log");

	// Can check if initialized
	if (AsyncLogger::IsInitialized())
	{
		AsyncLogger::Info("Logger is ready");
	}

	// Safe to call even if not initialized
	AsyncLogger::Shutdown();
	AsyncLogger::Shutdown();  // Safe, does nothing

	// Can reinitialize with different name
	AsyncLogger::Initialize("Test2.log");
	AsyncLogger::Info("This goes to Test2.log");
	AsyncLogger::Shutdown();
}

int main()
{
	printf("Running AsyncLogger examples...\n\n");

	printf("Example 1: Basic Usage\n");
	Example_BasicUsage();

	printf("Example 2: Printf-style Formatting\n");
	Example_PrintfStyle();

	printf("Example 3: Modern fmt-style Formatting\n");
	Example_FmtStyle();

	printf("Example 4: Thread Independence\n");
	Example_ThreadIndependence();

	printf("Example 5: Session Management\n");
	Example_Sessions();

	printf("Example 7: Flush Status Checking\n");
	Example_FlushStatus();

	printf("Example 8: Configuration Options\n");
	Example_Configuration();

	printf("Example 10: Error Recovery\n");
	Example_ErrorRecovery();

	printf("\nAll examples completed! Check the generated .log files.\n");
	return 0;
}
#endif

// Static member initialization
std::shared_ptr<spdlog::logger> AsyncLogger::s_Logger = nullptr;
std::atomic<bool> AsyncLogger::s_Initialized(false);
std::atomic<size_t> AsyncLogger::s_PendingCount(0);
std::mutex AsyncLogger::s_InitMutex;
std::string AsyncLogger::s_CurrentLogFile;

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

bool AsyncLogger::Initialize(const char* logFileName, size_t threadPoolSize, size_t queueSize)
{
	std::lock_guard<std::mutex> lock(s_InitMutex);

	if (s_Initialized)
	{
		return true;  // Already initialized
	}

	try
	{
		// Store log file name
		s_CurrentLogFile = logFileName ? logFileName : "default.log";

		// Initialize spdlog's thread pool for async logging
		// This creates background threads that are completely independent
		spdlog::init_thread_pool(queueSize, threadPoolSize);

		// Create async file sink
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
			s_CurrentLogFile,
			true  // truncate file on open
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
		s_Logger->flush();

		return true;
	}
	catch (const std::exception& e)
	{
		// Initialization failed - try to output error somewhere
		fprintf(stderr, "AsyncLogger::Initialize failed: %s\n", e.what());
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
}

void AsyncLogger::Flush()
{
	if (s_Logger)
	{
		s_Logger->flush();
		s_PendingCount = 0;  // Reset counter after flush
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
			true
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