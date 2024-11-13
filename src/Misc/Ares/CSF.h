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

	inline static int CSFCount;
	inline static int NextValueIndex;
	struct Storages {
		CSFString value {};
		bool found { true };
	};
	inline static std::unordered_map<std::string, CSFString> DynamicStrings;

	static auto FindOrAllocateDynamicStrings(const char* val) {
		return &DynamicStrings[val];
	}

	static bool IsStringPatternFound(const char* val) {
		//const auto find_ = DynamicStrings.find(val);
		//return find_ != DynamicStrings.end() && find_->second.found;
		return DynamicStrings.contains(val);
	}

	static void LoadAdditionalCSF(const char* fileName, bool ignoreLanguage = false);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def);
};