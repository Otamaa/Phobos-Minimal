#pragma once

#include <StringTable.h>

#include <unordered_map>
#include <string>

class CSFLoader
{
public:
	struct BiggerCFSString {
		wchar_t Text[0x500];
	};

	static size_t constexpr MaxEntries = 320000u;

	static int CSFCount;
	static int NextValueIndex;
	static std::unordered_map<std::string, BiggerCFSString> DynamicStrings;

	static void LoadAdditionalCSF(const char* fileName, bool ignoreLanguage = false);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def);
};