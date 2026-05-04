#include <Phobos.CRT.h>

#include <cstring>
#include <cstdlib>
#include <Lib/utfcpp/utf8/core.h>

void PhobosCRT::strCopy(char* Dest, const char* Source, size_t Count) {
	strncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

void PhobosCRT::wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count) {
	wcsncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

char* PhobosCRT::stristr(const char* str, const char* str_search)
{
	char* sors, * subs, * res = nullptr;
	if ((sors = _strdup(str)) != nullptr)
	{
		if ((subs = _strdup(str_search)) != nullptr)
		{
			res = strstr(_strlwr(sors), _strlwr(subs));
			if (res != nullptr)
				res = (char*)(str + (res - sors));
			free(subs);
		}
		free(sors);
	}
	return res;
}

/// <summary>
/// Returns the path to the Windows System32 directory.
/// </summary>
std::filesystem::path  PhobosCRT::get_system_path()
{
	WCHAR buf[4096];
	return std::filesystem::path(buf, buf + GetSystemDirectoryW(buf, ARRAYSIZE(buf)));
}

/// <summary>
/// Returns the path to the module file identified by the specified <paramref name="module"/> handle.
/// </summary>
std::filesystem::path PhobosCRT::get_module_path(HMODULE module)
{
	WCHAR buf[4096];
	return std::filesystem::path(buf, buf + GetModuleFileNameW(module, buf, ARRAYSIZE(buf)));
}

bool PhobosCRT::resolve_path(std::filesystem::path& path, std::error_code& ec, const std::filesystem::path& base)
{
	if (path.empty())
		return false;

	auto u8_str = path.u8string();
	std::string _8str(u8_str.begin(), u8_str.end());
	path = std::filesystem::u8path(PhobosCRT::expand_macro_string(_8str, {}));

	// First convert path to an absolute path
	// Ignore the working directory and instead start relative paths at the DLL location
	if (path.is_relative())
		path = base / path;
	// Finally try to canonicalize the path too
	if (std::filesystem::path canonical_path = std::filesystem::canonical(path, ec); !ec)
		path = std::move(canonical_path);
	else
		path = path.lexically_normal();

	return !ec; // The canonicalization step fails if the path does not exist
}

std::string PhobosCRT::expand_macro_string(const std::string& input, std::vector<std::pair<std::string_view, std::string>> macros, std::chrono::system_clock::time_point now)
{
	const auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	char timestamp[21];
	const std::time_t t = std::chrono::system_clock::to_time_t(now_seconds);
	struct tm tm; localtime_s(&tm, &t);

	std::snprintf(timestamp, std::size(timestamp), "%.4d-%.2d-%.2d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	macros.emplace_back("Date", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.4d", tm.tm_year + 1900);
	macros.emplace_back("DateYear", timestamp);
	macros.emplace_back("Year", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.2d", tm.tm_mon + 1);
	macros.emplace_back("DateMonth", timestamp);
	macros.emplace_back("Month", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.2d", tm.tm_mday);
	macros.emplace_back("DateDay", timestamp);
	macros.emplace_back("Day", timestamp);

	std::snprintf(timestamp, std::size(timestamp), "%.2d-%.2d-%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);
	macros.emplace_back("Time", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.2d", tm.tm_hour);
	macros.emplace_back("TimeHour", timestamp);
	macros.emplace_back("Hour", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.2d", tm.tm_min);
	macros.emplace_back("TimeMinute", timestamp);
	macros.emplace_back("Minute", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.2d", tm.tm_sec);
	macros.emplace_back("TimeSecond", timestamp);
	macros.emplace_back("Second", timestamp);
	std::snprintf(timestamp, std::size(timestamp), "%.3lld", std::chrono::duration_cast<std::chrono::milliseconds>(now - now_seconds).count());
	macros.emplace_back("TimeMillisecond", timestamp);
	macros.emplace_back("Millisecond", timestamp);
	macros.emplace_back("TimeMS", timestamp);

	return expand_macro_string(input, macros);
}

std::string  PhobosCRT::expand_macro_string(const std::string& input, std::vector<std::pair<std::string_view, std::string>> macros)
{
	std::string result;

	for (size_t offset = 0, macro_beg, macro_end; offset < input.size(); offset = macro_end + 1)
	{
		macro_beg = input.find('%', offset);
		macro_end = input.find('%', macro_beg + 1);

		if (macro_beg == std::string::npos || macro_end == std::string::npos)
		{
			result += input.substr(offset);
			break;
		}
		else
		{
			result += input.substr(offset, macro_beg - offset);

			if (macro_end == macro_beg + 1) // Handle case of %% to escape percentage symbol
			{
				result += '%';
				continue;
			}
		}

		const std::string_view input_macro(input.c_str() + macro_beg + 1, macro_end - (macro_beg + 1));

		size_t colon_pos = input_macro.find(':');
		const std::string_view input_macro_name = (colon_pos == std::string_view::npos) ? input_macro : input_macro.substr(0, colon_pos);

		std::string value;
		for (const std::pair<std::string_view, std::string>& macro : macros)
		{
			if (macro.first == input_macro_name)
			{
				value = macro.second;
				break;
			}
		}

		// Allow using environment variables alongside macros
		if (value.empty())
		{
			char buf[512] = ""; size_t buf_len = 0;
			if (getenv_s(&buf_len, buf, sizeof(buf) - 1, std::string(input_macro_name).c_str()) == 0)
				value = buf;
		}

		if (colon_pos == std::string_view::npos)
		{
			result += value;
		}
		else
		{
			const std::string_view input_macro_param = input_macro.substr(colon_pos + 1);

			if (const size_t insert_pos = input_macro_param.find('$');
				insert_pos != std::string_view::npos)
			{
				result += input_macro_param.substr(0, insert_pos);
				result += value;
				result += input_macro_param.substr(insert_pos + 1);
			}
			else
			{
				result += input_macro_param;
			}
		}
	}

	return result;
}