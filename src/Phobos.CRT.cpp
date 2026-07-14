#include <Phobos.CRT.h>

#include <cstring>
#include <cstdlib>

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

// =============================================================================
// PhobosCRT string conversion — narrow to wide
//
// Two variants:
//   StringToWideStringSimple — for label names, NOSTR values, general use.
//                              No embedded-null handling. Fast two-pass.
//   StringToWideString       — for binary blobs that may contain embedded nulls.
//                              Splits on null bytes, converts each segment.
//
// Both try UTF-8 first, fall back to CP_ACP (system codepage) for legacy
// Latin-1 content. ACP fallback never uses MB_ERR_INVALID_CHARS so it
// always produces output rather than silent empty string.
// =============================================================================

// -----------------------------------------------------------------------------
// StringToWideStringSimple — no embedded null handling
// Accepts string_view: works with const char*, std::string, std::string_view
// without extra allocation.
// -----------------------------------------------------------------------------
std::wstring PhobosCRT::StringToWideStringSimple(std::string_view str)
{
	if (str.empty())
		return {};

	// Two-pass: first get required buffer size, then convert.
	// Try UTF-8 first.
	int required = MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS,
		str.data(), static_cast<int>(str.size()),
		nullptr, 0);

	if (required <= 0)
	{
		// UTF-8 failed — fall back to system codepage.
		// No MB_ERR_INVALID_CHARS so it always succeeds on valid ACP input.
		required = MultiByteToWideChar(
			CP_ACP, 0,
			str.data(), static_cast<int>(str.size()),
			nullptr, 0);

		if (required <= 0)
			return {}; // genuinely unconvertible

		std::wstring result(static_cast<size_t>(required), L'\0');
		MultiByteToWideChar(CP_ACP, 0,
			str.data(), static_cast<int>(str.size()),
			result.data(), required);
		return result;
	}

	std::wstring result(static_cast<size_t>(required), L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		str.data(), static_cast<int>(str.size()),
		result.data(), required);
	return result;
}

// Buffer-out overload — copies into caller-owned wchar_t buffer.
// wstrCopy must null-terminate on truncation.
void PhobosCRT::StringToWideStringSimple(wchar_t* ret, size_t len, std::string_view str)
{
	const std::wstring result = PhobosCRT::StringToWideStringSimple(str);
	wstrCopy(ret, result.c_str(), len);
}

// -----------------------------------------------------------------------------
// StringToWideString — handles embedded nulls in input string
// Splits on null bytes, converts each segment, reassembles with null separators.
// Use this only when the input is known to contain embedded nulls.
// For normal strings, StringToWideStringSimple is faster and correct.
// -----------------------------------------------------------------------------
static std::wstring ConvertSegment(std::string_view segment)
{
	if (segment.empty())
		return {};

	// Two-pass UTF-8
	int required = MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS,
		segment.data(), static_cast<int>(segment.size()),
		nullptr, 0);

	if (required <= 0)
	{
		// UTF-8 failed — ACP fallback, no MB_ERR_INVALID_CHARS
		required = MultiByteToWideChar(
			CP_ACP, 0,
			segment.data(), static_cast<int>(segment.size()),
			nullptr, 0);

		if (required <= 0)
			return {};

		std::wstring result(static_cast<size_t>(required), L'\0');
		MultiByteToWideChar(CP_ACP, 0,
			segment.data(), static_cast<int>(segment.size()),
			result.data(), required);
		return result;
	}

	std::wstring result(static_cast<size_t>(required), L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		segment.data(), static_cast<int>(segment.size()),
		result.data(), required);
	return result;
}

std::wstring PhobosCRT::StringToWideString(const std::string& str)
{
	if (str.empty())
		return {};

	std::wstring ret;
	size_t begin = 0;

	size_t pos = str.find(static_cast<char>(0), begin);
	while (pos != std::string::npos)
	{
		// Convert segment before the embedded null
		ret.append(ConvertSegment({ &str[begin], pos - begin }));
		// Preserve the embedded null
		ret.push_back(L'\0');

		begin = pos + 1;
		pos = str.find(static_cast<char>(0), begin);
	}

	// Convert final segment (or entire string if no nulls found)
	if (begin < str.size())
		ret.append(ConvertSegment({ &str[begin], str.size() - begin }));

	return ret;
}

// Buffer-out overload
void PhobosCRT::StringToWideString(wchar_t* ret, size_t len, const std::string& str)
{
	const std::wstring result = PhobosCRT::StringToWideString(str);
	wstrCopy(ret, result.c_str(), len);
}