#pragma once

#include <CRT.h>
#include <string>
#include <algorithm>
#include <immintrin.h>
#include <intrin.h>
#include <vector>

class PhobosCRT final
{
	NO_CONSTRUCT_CLASS(PhobosCRT)
public:

	static OPTIONALINLINE bool iequals(std::string_view a, std::string_view b) {
		if (a.size() != b.size()) return false;
		return _strnicmp(a.data(), b.data(), a.size()) == 0;
	}

	 template<typename CharPtr>
	 static CharPtr __cdecl strchr_selector(CharPtr str, char ch) {
		return strchr(str, ch);
	 }

	 template<typename CharPtr ,typename CharRetPtr>
	 static CharRetPtr __cdecl strstr_selector(CharPtr a1, CharPtr a2) {
		 return strstr(a1, a2);
	 }

	static COMPILETIMEEVAL bool is_space(char c) {
		return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
	}

	// these two are saner mechanisms for string copying

	// copy up to Count chars using strncpy
	// force (Count - 1)th char to \0
	// which means you pass the full length of the char[]/wchar_t[] as Count and it will not overflow
	// it doesn't mean you can copy strings without thinking
	static void strCopy(char* Dest, const char* Source, size_t Count);
	static void wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count);
	static char* stristr(const char* str, const char* str_search);

	template<size_t Size>
	static void strCopy(char(&Dest)[Size], const char* Source)
	{
		strCopy(Dest, Source, Size);
	}

	template<size_t Size>
	static void wstrCopy(wchar_t(&Dest)[Size], const wchar_t* Source)
	{
		wstrCopy(Dest, Source, Size);
	}

	static COMPILETIMEEVAL OPTIONALINLINE void EraseSubString(std::string& str, const std::string& erase)
	{
		size_t pos = str.find(erase);
		if (pos != std::string::npos)
		{
			str.erase(pos, erase.length());
		}
	}

	template <typename T>
	static COMPILETIMEEVAL OPTIONALINLINE std::string GetTypeIDName()
	{
		std::string str = typeid(T).name();
		EraseSubString(str, "class ");
		EraseSubString(str, "struct ");
		return str;
	}

	template <typename T>
	static std::string OPTIONALINLINE GetTypeIDNameOf(const T abstract_ext)
	{
		std::string str = typeid(abstract_ext).name();
		EraseSubString(str, "class ");
		EraseSubString(str, "struct ");
		return str;
	}

	//
	//  Lowercases string
	//
	template <typename T>
	static OPTIONALINLINE std::basic_string<T> lowercase(const std::basic_string<T>& s, size_t start = 0)
	{
		std::basic_string<T> s2 = s;
		std::transform(s2.begin() + start, s2.end(), s2.begin() + start, ::tolower);
		return s2;
	}

	template <typename T>
	static OPTIONALINLINE void lowercase(std::basic_string<T>& s, size_t start = 0)
	{
		std::transform(s.begin() + start, s.end(), s.begin() + start, ::tolower);
	}

	//
	// Uppercases string
	//
	template <typename T>
	static std::basic_string<T> uppercase(const std::basic_string<T>& s, size_t start = 0)
	{
		std::basic_string<T> s2 = s;
		std::transform(s2.begin() + start, s2.end(), s2.begin() + start, ::toupper);
		return s2;
	}

	template <typename T>
	static void uppercase(std::basic_string<T>& s, size_t start = 0)
	{
		std::transform(s.begin() + start, s.end(), s.begin() + start, ::toupper);
	}

	template <size_t size>
	static OPTIONALINLINE void lowercase(char(&nBuff)[size], char const (&nData)[size], size_t start = 0)
	{
		for (size_t i = 0 + start; i < size; ++i)
		{
			nBuff[i] = (char)std::tolower(nData[i]);
		}
	}

	template <size_t size>
	static OPTIONALINLINE void uppercase(char(&nBuff)[size], char(&nData)[size], size_t start = 0)
	{
		for (size_t i = 0 + start; i < size; ++i)
		{
			nBuff[i] = (char)std::toupper(nData[i]);
		}
	}

	COMPILETIMEEVAL static std::string WideStringToString(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return {};
		}
		size_t pos;
		size_t begin = 0;
		std::string ret;

		int size;
		pos = wstr.find(static_cast<wchar_t>(0), begin);
		while (pos != std::wstring::npos && begin < wstr.length())
		{
			std::wstring segment = std::wstring(&wstr[begin], pos - begin);
			size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
			std::string converted = std::string(size, 0);
			WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
			ret.append(converted);
			ret.append({ 0 });
			begin = pos + 1;
			pos = wstr.find(static_cast<wchar_t>(0), begin);
		}
		if (begin <= wstr.length())
		{
			std::wstring segment = std::wstring(&wstr[begin], wstr.length() - begin);
			size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), NULL, 0, NULL, NULL);
			std::string converted = std::string(size, 0);
			WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.size(), NULL, NULL);
			ret.append(converted);
		}

		return ret;
	}

	// Narrow → wide, no embedded null handling. Preferred for label names,
	// NOSTR values, general INI/CSF content.
	static std::wstring StringToWideStringSimple(std::string_view str);
	static void         StringToWideStringSimple(wchar_t* ret, size_t len, std::string_view str);

	// Narrow → wide with embedded null support. Use only when input is a
	// binary blob known to contain null bytes.
	static std::wstring StringToWideString(const std::string& str);
	static void         StringToWideString(wchar_t* ret, size_t len, const std::string& str);

	static COMPILETIMEEVAL std::string FORCEDINLINE trim(const char* source)
	{
		std::string s(source);
		s.erase(0, s.find_first_not_of(" \n\r\t"));
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
		return s;
	}

	static COMPILETIMEEVAL std::string_view FORCEDINLINE trim(std::string_view sv) {
		size_t begin = 0;
		size_t end = sv.size();

		while (begin < end && is_space(sv[begin]))
			++begin;

		while (end > begin && is_space(sv[end - 1]))
			--end;

		return sv.substr(begin, end - begin);
	};

	static COMPILETIMEEVAL std::string Trim(const std::string& s)
	{
		auto start = s.find_first_not_of(" \t\n\r");
		if (start == std::string::npos) return "";
		auto end = s.find_last_not_of(" \t\n\r");
		return s.substr(start, end - start + 1);
	}

	template<size_t max>
	static COMPILETIMEEVAL std::array<std::string_view, max> OPTIONALINLINE split(std::string_view s)
	{
		std::array<std::string_view, max> out {};
		size_t count = 0;
		size_t pos = 0;

		while (pos < s.size() && count < out.size())
		{
			size_t next = s.find(',', pos);

			if (next == std::string_view::npos)
			{
				out[count++] = trim(s.substr(pos));
				break;
			}

			out[count++] = trim(s.substr(pos, next - pos));
			pos = next + 1;
		}

		return out;
	}

	template<size_t max>
	static COMPILETIMEEVAL auto OPTIONALINLINE splits(std::string_view s)
	{
		std::array<std::string_view, max> out {};
		size_t count = 0;
		size_t pos = 0;

		while (pos < s.size() && count < out.size())
		{
			size_t next = s.find(',', pos);

			if (next == std::string_view::npos)
			{
				out[count++] = trim(s.substr(pos));
				break;
			}

			out[count++] = trim(s.substr(pos, next - pos));
			pos = next + 1;
		}

		return std::pair<std::array<std::string_view, max>, size_t>{ out, count };
	}

	static COMPILETIMEEVAL std::vector<std::string> SplitString(const std::string& str, const std::string& delimiters)
	{
		std::vector<std::string> tokens;
		std::string::size_type start = 0;
		std::string::size_type end = 0;

		while ((end = str.find_first_of(delimiters, start)) != std::string::npos)
		{
			if (end != start) // skip empty tokens
				tokens.push_back(str.substr(start, end - start));
			start = end + 1;
		}
		if (start < str.length())
			tokens.push_back(str.substr(start));
		return tokens;
	}

	// ------------------------------------------------------------------------
	// Non-destructive tokenizer. Cannot be desynchronised by re-entrant calls.
	// ------------------------------------------------------------------------
	template<typename TFunc>
	void ForEachToken(std::string_view line, std::string_view seps, TFunc&& func)
	{
		std::size_t pos = 0;

		while (pos < line.size())
		{
			const auto start = line.find_first_not_of(seps, pos);

			if (start == std::string_view::npos)
				return;

			auto end = line.find_first_of(seps, start);

			if (end == std::string_view::npos)
				end = line.size();

			func(line.substr(start, end - start));
			pos = end;
		}
	}

	static bool ScanHex(std::string_view token, unsigned char& out) noexcept
	{
		size_t i = 0;

		while (i < token.size() && std::isspace(static_cast<unsigned char>(token[i])))
			++i;

		bool negative = false;

		if (i < token.size() && (token[i] == '+' || token[i] == '-'))
		{
			negative = (token[i] == '-');
			++i;
		}

		if (i + 1 < token.size() && token[i] == '0' && (token[i + 1] == 'x' || token[i + 1] == 'X'))
			i += 2;

		unsigned int value = 0;
		size_t digits = 0;

		for (; i < token.size(); ++i, ++digits)
		{
			const char c = token[i];
			unsigned int digit = 0;

			if (c >= '0' && c <= '9')
				digit = static_cast<unsigned int>(c - '0');
			else if (c >= 'a' && c <= 'f')
				digit = static_cast<unsigned int>(c - 'a') + 10u;
			else if (c >= 'A' && c <= 'F')
				digit = static_cast<unsigned int>(c - 'A') + 10u;
			else
				break;

			value = value * 16u + digit; // wraps, matching the vanilla int overflow
		}

		if (!digits)
			return false; // matching return: no conversion performed

		out = unsigned char(negative ? 0u - value : value);
		return true;
	}

	// Truncation to (cap - 1) chars is PRESERVED on purpose - vanilla silently clips
	// over-long house/team ids and the clipped form is what Find()/FindIndexById() sees.
	static std::string TrimmedField(std::string_view src, size_t cap)
	{
		src = src.substr(0, cap);

		const auto isWS = [](unsigned char c) { return std::isspace(c) != 0; };

		while (!src.empty() && isWS(static_cast<unsigned char>(src.front())))
			src.remove_prefix(1);

		while (!src.empty() && isWS(static_cast<unsigned char>(src.back())))
			src.remove_suffix(1);

		return std::string(src);
	}


	// Result of a fixed-arity split.
	//   Fields[]  - always exactly N entries; missing trailing fields are empty views.
	//   Count     - how many tokens the input ACTUALLY had (clamped to N). Use this when
	//               you must distinguish "field absent" from "field present but empty",
	//               because vanilla only assigns members for fields that exist.
	//   Overflow  - tokens past N that were dropped.
	template<size_t N>
	struct PhobosSplitFixed
	{
		std::array<std::string_view, N> Fields {};
		size_t Count { 0 };
		size_t Overflow { 0 };

		bool IsPresent(size_t idx) const { return idx < this->Count; }
		std::string_view operator[](size_t idx) const { return this->Fields[idx]; }
	};

	// Fixed-arity split into exactly N slots.
	//   keepEmpty = true  -> POSITIONAL. Empty tokens are preserved, so field K is always at
	//                        index K. Fixes the vanilla shift bug.
	//   keepEmpty = false -> parity with strtok()/PhobosCRT::SplitString: runs of delimiters
	//                        collapse and everything after an empty field shifts left.
	//
	// Views point INTO `str` - keep the source buffer alive for the lifetime of the result.
	template<size_t N>
	static COMPILETIMEEVAL PhobosSplitFixed<N> SplitStringFixed(
		std::string_view str, std::string_view delimiters, bool keepEmpty)
	{
		PhobosSplitFixed<N> result {};

		if (str.empty())
			return result;

		const auto Push = [&result](std::string_view token)
			{
				if (result.Count < N)
					result.Fields[result.Count++] = token;
				else
					++result.Overflow;
			};

		std::string_view::size_type start = 0;
		std::string_view::size_type end = 0;

		while ((end = str.find_first_of(delimiters, start)) != std::string_view::npos)
		{
			if (keepEmpty || end != start)
				Push(str.substr(start, end - start));

			start = end + 1;
		}

		// trailing field. Collapsing mode drops it when empty, matching strtok().
		if (keepEmpty)
			Push(str.substr(start));
		else if (start < str.size())
			Push(str.substr(start));

		return result;
	}
};