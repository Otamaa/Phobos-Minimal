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

	static size_t COMPILETIMEEVAL MaxEntries = 320000u
		;

	OPTIONALINLINE static int CSFCount;
	OPTIONALINLINE static int NextValueIndex;
	struct Storages {
		CSFString value {};
		bool found { true };
	};
	OPTIONALINLINE static std::unordered_map<std::string, CSFString> DynamicStrings;

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