#pragma once

#include <StringTable.h>

#include <unordered_map>
#include <string>

class CSFLoader
{
public:
	struct BiggerCSFString {
		wchar_t Text[0x500];
	};

	static size_t constexpr MaxEntries = 320000u
		;

	static int CSFCount;
	static int NextValueIndex;
	static std::vector<std::pair<std::string, CSFString>> DynamicStrings;

	static constexpr auto FindOrAllocateDynamicStrings(const char* val) {
		for (auto begin = DynamicStrings.begin(); begin != DynamicStrings.end(); ++begin) {
			if (begin->first == val) {
				return begin;
			}
		}

		DynamicStrings.emplace_back(val, CSFString{});
		return DynamicStrings.begin() + (DynamicStrings.size() - 1);
	}

	static void LoadAdditionalCSF(const char* fileName, bool ignoreLanguage = false);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def);
};