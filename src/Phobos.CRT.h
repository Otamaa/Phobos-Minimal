#pragma once

#include <CRT.h>
#include <string>
#include <algorithm>
#include <immintrin.h>
#include <intrin.h>

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

	COMPILETIMEEVAL static std::wstring StringToWideString(const std::string& str)
	{
		if (str.empty())
		{
			return {};
		}

		size_t pos;
		size_t begin = 0;
		std::wstring ret;

		int size = 0;
		pos = str.find(static_cast<char>(0), begin);
		while (pos != std::string::npos)
		{
			std::string segment = std::string(&str[begin], pos - begin);
			std::wstring converted = std::wstring(segment.size() + 1, 0);
			size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &segment[0], segment.size(), &converted[0], converted.length());
			converted.resize(size);
			ret.append(converted);
			ret.append({ 0 });
			begin = pos + 1;
			pos = str.find(static_cast<char>(0), begin);
		}
		if (begin < str.length())
		{
			std::string segment = std::string(&str[begin], str.length() - begin);
			std::wstring converted = std::wstring(segment.size() + 1, 0);
			size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, segment.c_str(), segment.size(), &converted[0], converted.length());
			converted.resize(size);
			ret.append(converted);
		}

		return ret;
	}

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
};