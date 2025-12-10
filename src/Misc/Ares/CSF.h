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

	static int CSFCount;
	static int NextValueIndex;
	struct CSFStringStorage
	{
		wchar_t Text[0x500];
		bool TextLoaded;
		bool IsMissingValue;

		CSFStringStorage() : Text {}, TextLoaded {}, IsMissingValue { true }
		{
			__stosw(reinterpret_cast<unsigned short*>(Text), static_cast<unsigned short>(0), std::size(Text));
		}

		~CSFStringStorage() = default;

	};

	//separated storage for CSF strings , especially for one that not yet loaded
	static std::unordered_map<std::string, CSFStringStorage> DynamicStrings;

	static FORCEDINLINE CSFStringStorage* FindOrAllocateDynamicStrings(const char* val) {
		return &DynamicStrings[val];
	}

	static FORCEDINLINE bool IsStringPatternFound(const char* val) {
		return DynamicStrings.contains(val);
	}

	static void LoadAdditionalCSF(const char* fileName, bool ignoreLanguage = false);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def , bool isNostr);
	static const wchar_t* __fastcall FetchStringManager(const char* label, char* speech, const char* file, int line);

};