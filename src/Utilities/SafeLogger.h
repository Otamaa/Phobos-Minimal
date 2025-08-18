#pragma once
#include <windows.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <vector>

// Error codes for logging operations
enum class LogResult
{
	SUCCESS = 0,
	NOT_INITIALIZED,
	DISABLED,
	LOCK_FAILED,
	LOGGER_NULL,
	UNKNOWN_ERROR
};

// Log configuration flags
struct LogConfig
{
	bool enabled = false;
	bool console_output = false;
	bool file_output = true;
	spdlog::level::level_enum level = spdlog::level::info;
	std::string log_filename = "phobos.log";
	size_t max_file_size = 1024 * 1024 * 5; // 5MB
	size_t max_files = 3;
	bool auto_flush = true;
	int flush_interval_seconds = 1;
};

class SafeLogger
{
private:
	static std::unique_ptr<SafeLogger> instance;
	static std::mutex init_mutex;
	static std::atomic<bool> initialized;

	std::shared_ptr<spdlog::logger> logger;
	std::mutex log_mutex;
	std::atomic<bool> is_valid;
	std::atomic<bool> is_enabled;
	std::atomic<bool> initialization_attempted;
	LogConfig config;

	// Private constructor - only creates the instance, doesn't initialize logging
	SafeLogger() : is_valid(false), is_enabled(false), initialization_attempted(false)
	{
		// Constructor does minimal work - just sets up the object
		// Actual logging initialization happens later via Initialize()
	}

public:
	static SafeLogger& GetInstance()
	{
		if (!initialized.load(std::memory_order_acquire))
		{
			std::lock_guard<std::mutex> lock(init_mutex);
			if (!initialized.load(std::memory_order_relaxed))
			{
				instance.reset(new SafeLogger());
				initialized.store(true, std::memory_order_release);
			}
		}
		return *instance;
	}

	// Manual initialization - call this after configuring
	LogResult Initialize()
	{
		if (initialization_attempted.load())
		{
			return is_valid.load() ? LogResult::SUCCESS : LogResult::NOT_INITIALIZED;
		}

		initialization_attempted.store(true);

		if (!config.enabled)
		{
			is_enabled.store(false);
			OutputDebugStringA("SafeLogger: Logging disabled in configuration");
			return LogResult::DISABLED;
		}

		return InitializeLogger();
	}

	// Initialize with command line parsing
	LogResult InitializeFromCommandLine()
	{
		ParseCommandLineArgs();
		return Initialize();
	}

	// Configure the logger before initialization
	void SetConfig(const LogConfig& new_config)
	{
		if (initialization_attempted.load())
		{
			OutputDebugStringA("SafeLogger: Cannot change config after initialization");
			return;
		}
		config = new_config;
	}

	LogConfig& GetMutableConfig()
	{
		if (initialization_attempted.load())
		{
			OutputDebugStringA("SafeLogger: Warning - changing config after initialization may have no effect");
		}
		return config;
	}

	// Enable/disable logging
	void EnableLogging()
	{
		config.enabled = true;
		is_enabled.store(true);
	}

	void DisableLogging()
	{
		config.enabled = false;
		is_enabled.store(false);
	}

	// Reinitialize with new configuration
	LogResult Reinitialize()
	{
		// Reset state
		is_valid.store(false);
		initialization_attempted.store(false);

		// Clean up existing logger
		if (logger)
		{
			logger->flush();
			spdlog::drop("dll_logger");
			logger.reset();
		}

		// Initialize with current config
		return Initialize();
	}

private:
	void ParseCommandLineArgs()
	{
		LPWSTR cmdLineW = GetCommandLineW();
		if (!cmdLineW) return;

		int size = WideCharToMultiByte(CP_UTF8, 0, cmdLineW, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 0) return;

		std::string cmdLine(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, cmdLineW, -1, &cmdLine[0], size, nullptr, nullptr);

		std::vector<std::string> args = SplitCommandLine(cmdLine);

		for (size_t i = 0; i < args.size(); ++i)
		{
			const std::string& arg = args[i];

			if (arg == "--enable-dll-log" || arg == "-dll-log")
			{
				config.enabled = true;
			}
			else if (arg == "--disable-dll-log" || arg == "-no-dll-log")
			{
				config.enabled = false;
			}
			else if (arg == "--dll-log-console" || arg == "-dll-console")
			{
				config.console_output = true;
			}
			else if (arg == "--dll-log-no-file" || arg == "-dll-no-file")
			{
				config.file_output = false;
			}
			else if ((arg == "--dll-log-level" || arg == "-dll-level") && i + 1 < args.size())
			{
				config.level = ParseLogLevel(args[i + 1]);
				++i;
			}
			else if ((arg == "--dll-log-file" || arg == "-dll-file") && i + 1 < args.size())
			{
				config.log_filename = args[i + 1];
				++i;
			}
			else if (arg == "--dll-debug" || arg == "-dll-debug")
			{
				config.enabled = true;
				config.console_output = true;
				config.file_output = true;
				config.level = spdlog::level::debug;
			}
		}
	}

	std::vector<std::string> SplitCommandLine(const std::string& cmdLine)
	{
		std::vector<std::string> args;
		std::string current;
		bool inQuotes = false;

		for (char c : cmdLine)
		{
			if (c == '"')
			{
				inQuotes = !inQuotes;
			}
			else if (c == ' ' && !inQuotes)
			{
				if (!current.empty())
				{
					args.push_back(current);
					current.clear();
				}
			}
			else
			{
				current += c;
			}
		}

		if (!current.empty())
		{
			args.push_back(current);
		}

		return args;
	}

	spdlog::level::level_enum ParseLogLevel(const std::string& level)
	{
		if (level == "debug" || level == "0") return spdlog::level::debug;
		if (level == "info" || level == "1") return spdlog::level::info;
		if (level == "warn" || level == "2") return spdlog::level::warn;
		if (level == "error" || level == "3") return spdlog::level::err;
		if (level == "critical" || level == "4") return spdlog::level::critical;
		return spdlog::level::info;
	}

	LogResult InitializeLogger()
	{
		// Set spdlog to not throw exceptions
		try{
			spdlog::set_error_handler([](const std::string& msg) {
				OutputDebugStringA(("spdlog error: " + msg).c_str());
			});

			// Configure async logging (queue size = 8192, 1 worker thread)
			spdlog::init_thread_pool(8192, 1);

			spdlog::set_level(config.level);

			if (config.auto_flush)
			{
				spdlog::flush_every(std::chrono::seconds(config.flush_interval_seconds));
			}

			std::vector<spdlog::sink_ptr> sinks;

			// Add file sink if enabled
			if (config.file_output)
			{
				auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
					config.log_filename, config.max_file_size, config.max_files);
				if (file_sink)
				{
					sinks.push_back(file_sink);
				}
			}

			// Add console sink if enabled
			if (config.console_output)
			{
				auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				if (console_sink)
				{
					sinks.push_back(console_sink);
				}
			}

			if (sinks.empty())
			{
				OutputDebugStringA("SafeLogger: No sinks configured");
				return LogResult::NOT_INITIALIZED;
			}

			logger = std::make_shared<spdlog::logger>("dll_logger", sinks.begin(), sinks.end());
			if (!logger)
			{
				OutputDebugStringA("SafeLogger: Failed to create logger");
				return LogResult::NOT_INITIALIZED;
			}

			logger->set_level(config.level);
			logger->flush_on(spdlog::level::warn);

			spdlog::register_logger(logger);

			is_valid.store(true);
			is_enabled.store(true);
			return LogResult::SUCCESS;
		}
		catch (const spdlog::spdlog_ex& ex)
		{
			MessageBoxA(nullptr, ex.what(), "Logger init failed", MB_ICONERROR);
		}

		return LogResult::NOT_INITIALIZED;
	}

public:
	template<typename... Args>
	LogResult Info(fmt::format_string<Args...> format, Args&&... args)
	{
		return SafeLog(spdlog::level::info, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult Debug(fmt::format_string<Args...> format, Args&&... args)
	{
		return SafeLog(spdlog::level::debug, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult Warn(fmt::format_string<Args...> format, Args&&... args)
	{
		return SafeLog(spdlog::level::warn, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult Error(fmt::format_string<Args...> format, Args&&... args)
	{
		return SafeLog(spdlog::level::err, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult Critical(fmt::format_string<Args...> format, Args&&... args)
	{
		return SafeLog(spdlog::level::critical, format, std::forward<Args>(args)...);
	}

	// Fallback methods for runtime format strings
	template<typename... Args>
	LogResult InfoRuntime(const std::string& format, Args&&... args)
	{
		return SafeLogRuntime(spdlog::level::info, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult DebugRuntime(const std::string& format, Args&&... args)
	{
		return SafeLogRuntime(spdlog::level::debug, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult WarnRuntime(const std::string& format, Args&&... args)
	{
		return SafeLogRuntime(spdlog::level::warn, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult ErrorRuntime(const std::string& format, Args&&... args)
	{
		return SafeLogRuntime(spdlog::level::err, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	LogResult CriticalRuntime(const std::string& format, Args&&... args)
	{
		return SafeLogRuntime(spdlog::level::critical, format, std::forward<Args>(args)...);
	}

	// Simple string logging without formatting
	LogResult InfoStr(const char* message) { return SafeLogStr(spdlog::level::info, message); }
	LogResult DebugStr(const char* message) { return SafeLogStr(spdlog::level::debug, message); }
	LogResult WarnStr(const char* message) { return SafeLogStr(spdlog::level::warn, message); }
	LogResult ErrorStr(const char* message) { return SafeLogStr(spdlog::level::err, message); }
	LogResult CriticalStr(const char* message) { return SafeLogStr(spdlog::level::critical, message); }

private:
	template<typename... Args>
	LogResult SafeLog(spdlog::level::level_enum level, fmt::format_string<Args...> format, Args&&... args)
	{
		if (!is_enabled.load())
		{
			return LogResult::DISABLED;
		}

		if (!is_valid.load())
		{
			return LogResult::NOT_INITIALIZED;
		}

		std::unique_lock<std::mutex> lock(log_mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			return LogResult::LOCK_FAILED;
		}

		if (!logger)
		{
			return LogResult::LOGGER_NULL;
		}

		logger->log(level, format, std::forward<Args>(args)...);
		return LogResult::SUCCESS;
	}

	template<typename... Args>
	LogResult SafeLogRuntime(spdlog::level::level_enum level, const std::string& format, Args&&... args)
	{
		if (!is_enabled.load())
		{
			return LogResult::DISABLED;
		}

		if (!is_valid.load())
		{
			return LogResult::NOT_INITIALIZED;
		}

		std::unique_lock<std::mutex> lock(log_mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			return LogResult::LOCK_FAILED;
		}

		if (!logger)
		{
			return LogResult::LOGGER_NULL;
		}

		logger->log(level, fmt::runtime(format), std::forward<Args>(args)...);
		return LogResult::SUCCESS;
	}

	LogResult SafeLogStr(spdlog::level::level_enum level, const char* message)
	{
		if (!is_enabled.load())
		{
			return LogResult::DISABLED;
		}

		if (!is_valid.load())
		{
			if (config.enabled)
			{
				OutputDebugStringA(message);
			}
			return LogResult::NOT_INITIALIZED;
		}

		std::unique_lock<std::mutex> lock(log_mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			if (config.enabled)
			{
				OutputDebugStringA(message);
			}
			return LogResult::LOCK_FAILED;
		}

		if (!logger)
		{
			if (config.enabled)
			{
				OutputDebugStringA(message);
			}
			return LogResult::LOGGER_NULL;
		}

		logger->log(level, message);
		return LogResult::SUCCESS;
	}

public:
	LogResult Flush()
	{
		if (!is_enabled.load() || !is_valid.load())
		{
			return LogResult::DISABLED;
		}

		std::unique_lock<std::mutex> lock(log_mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			return LogResult::LOCK_FAILED;
		}

		if (!logger)
		{
			return LogResult::LOGGER_NULL;
		}

		logger->flush();
		return LogResult::SUCCESS;
	}

	LogResult SetLogLevel(spdlog::level::level_enum level)
	{
		config.level = level;

		if (!is_enabled.load() || !is_valid.load())
		{
			return LogResult::SUCCESS; // Config updated, will apply on next init
		}

		std::lock_guard<std::mutex> lock(log_mutex);
		if (!logger)
		{
			return LogResult::LOGGER_NULL;
		}

		logger->set_level(level);
		return LogResult::SUCCESS;
	}

	// State queries
	bool IsEnabled() const { return is_enabled.load(); }
	bool IsValid() const { return is_valid.load(); }
	bool IsInitialized() const { return initialization_attempted.load(); }

	const LogConfig& GetConfig() const { return config; }

	~SafeLogger()
	{
		if (logger)
		{
			logger->flush();
			spdlog::drop_all();
		}
	}
};

// Static member definitions
std::unique_ptr<SafeLogger> SafeLogger::instance = nullptr;
std::mutex SafeLogger::init_mutex;
std::atomic<bool> SafeLogger::initialized(false);

// Convenience macros for compile-time format strings (preferred)
#define LOG_INFO(format, ...)    SafeLogger::GetInstance().Info(FMT_STRING(format), ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)   SafeLogger::GetInstance().Debug(FMT_STRING(format), ##__VA_ARGS__)
#define LOG_WARN(format, ...)    SafeLogger::GetInstance().Warn(FMT_STRING(format), ##__VA_ARGS__)
#define LOG_ERROR(format, ...)   SafeLogger::GetInstance().Error(FMT_STRING(format), ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...) SafeLogger::GetInstance().Critical(FMT_STRING(format), ##__VA_ARGS__)

// Runtime format string macros (for dynamic format strings)
#define LOG_INFO_RT(format, ...)    SafeLogger::GetInstance().InfoRuntime(format, ##__VA_ARGS__)
#define LOG_DEBUG_RT(format, ...)   SafeLogger::GetInstance().DebugRuntime(format, ##__VA_ARGS__)
#define LOG_WARN_RT(format, ...)    SafeLogger::GetInstance().WarnRuntime(format, ##__VA_ARGS__)
#define LOG_ERROR_RT(format, ...)   SafeLogger::GetInstance().ErrorRuntime(format, ##__VA_ARGS__)
#define LOG_CRITICAL_RT(format, ...) SafeLogger::GetInstance().CriticalRuntime(format, ##__VA_ARGS__)

// Simple string logging macros (no formatting overhead)
#define LOG_STR_INFO(msg)     SafeLogger::GetInstance().InfoStr(msg)
#define LOG_STR_DEBUG(msg)    SafeLogger::GetInstance().DebugStr(msg)
#define LOG_STR_WARN(msg)     SafeLogger::GetInstance().WarnStr(msg)
#define LOG_STR_ERROR(msg)    SafeLogger::GetInstance().ErrorStr(msg)
#define LOG_STR_CRITICAL(msg) SafeLogger::GetInstance().CriticalStr(msg)

#define LOG_FLUSH()      SafeLogger::GetInstance().Flush()
#define LOG_ENABLED()    SafeLogger::GetInstance().IsEnabled()

// Example usage in DLL
/*
// Method 1: Auto-initialize from command line (simplest)
void InitializeDLL_Auto() {
	auto& logger = SafeLogger::GetInstance();
	LogResult result = logger.InitializeFromCommandLine();

	if (result == LogResult::SUCCESS) {
		LOG_INFO("DLL logging initialized from command line");
	} else if (result == LogResult::DISABLED) {
		OutputDebugStringA("DLL logging disabled via command line");
	}
}

// Method 2: Manual configuration (most flexible)
void InitializeDLL_Manual() {
	auto& logger = SafeLogger::GetInstance();

	// Configure before initialization
	LogConfig config;
	config.enabled = true;
	config.console_output = true;
	config.file_output = true;
	config.log_filename = "my_custom_log.log";
	config.level = spdlog::level::debug;
	config.max_file_size = 1024 * 1024 * 10; // 10MB

	logger.SetConfig(config);

	LogResult result = logger.Initialize();
	if (result == LogResult::SUCCESS) {
		LOG_INFO("DLL logging initialized with custom config");
		LOG_DEBUG("Debug logging enabled");
	}
}

// Method 3: Hybrid approach (command line + manual overrides)
void InitializeDLL_Hybrid() {
	auto& logger = SafeLogger::GetInstance();

	// First parse command line
	logger.ParseCommandLineArgs(); // This would need to be made public

	// Then override specific settings
	auto& config = logger.GetMutableConfig();
	config.log_filename = "hybrid_log.log";
	config.max_file_size = 1024 * 1024 * 20; // 20MB

	LogResult result = logger.Initialize();
	if (result == LogResult::SUCCESS) {
		LOG_INFO("DLL logging initialized with hybrid config");
	}
}

// Runtime reconfiguration
void ReconfigureLogging() {
	auto& logger = SafeLogger::GetInstance();

	// Change configuration
	auto& config = logger.GetMutableConfig();
	config.level = spdlog::level::warn;
	config.console_output = false;

	// Reinitialize with new config
	LogResult result = logger.Reinitialize();
	if (result == LogResult::SUCCESS) {
		LOG_WARN("Logger reconfigured successfully");
	}
}

// Runtime enable/disable
void ToggleLogging(bool enable) {
	auto& logger = SafeLogger::GetInstance();

	if (enable) {
		logger.EnableLogging();
		LOG_INFO("Logging enabled at runtime");
	} else {
		LOG_INFO("Logging disabled at runtime");
		logger.DisableLogging();
	}
}
*/