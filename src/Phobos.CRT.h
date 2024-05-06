#pragma once

#include <CRT.h>
#include <string>

class PhobosCRT final
{
	NO_CONSTRUCT_CLASS(PhobosCRT)
public:

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

	constexpr inline void EraseSubString(std::string& str, const std::string& erase)
	{
		size_t pos = str.find(erase);
		if (pos != std::string::npos)
		{
			str.erase(pos, erase.length());
		}
	}

	template<typename T>
	constexpr static std::string GetTypeIDName()
	{
		std::string str = typeid(T).name();
		EraseSubString(str, "class ");
		EraseSubString(str, "struct ");
		return str;
	}
};