#include <Phobos.h>
#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Parser.h>
#include <Utilities/Constructs.h>
#include <Utilities/Macro.h>

#include "AresChecksummer.h"

#include <Helpers/Macro.h>

/*
	Line 2541: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ca5 , INIClass_Parse_IniSectionIncludes_PreProcess1 , 8]
	Line 2543: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ddb , INIClass_Parse_IniSectionIncludes_PreProcess2 , 5]
*/

struct AresINIData {
	static COMPILETIMEEVAL const char* const iteratorChar = "+";
	static COMPILETIMEEVAL const char* const iteratorReplacementFormat = "var_%d";

	static INISection* SectionCompare;
	static int KeyCompareIdx;
	static int iteratorValue;
	static GenericNode* NodeCompare;
};

INISection* AresINIData::SectionCompare;
int AresINIData::KeyCompareIdx;
int AresINIData::iteratorValue = 0;
GenericNode* AresINIData::NodeCompare;

#include <New/Type/GenericPrerequisite.h>


ASMJIT_PATCH(0x528A10, INIClass_GetString, 5)
{
	if (Phobos::Config::UseNewInheritance)
		return 0x0;

	GET(INIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(const char*, pDefault, 0xC);
	GET_STACK(char*, buffer, 0x10);
	GET_STACK(int, bufferlength, 0x14);

	int ret_len = 0;

	if (buffer && bufferlength >= 2 && pSection && pKey) {
		if (auto result = pThis->GetKeyValue(pSection, pKey, pDefault)) {
			// Trim leading whitespace
			while (*result && *result <= ' ')
				++result;

			// Compute trimmed length (excluding trailing spaces)
			size_t len = strlen(result);
			while (len > 0 && result[len - 1] <= ' ')
				--len;

			// Copy at most outLength - 1 characters
			ret_len = MinImpl(len, bufferlength - 1);
			std::memcpy(buffer, result, (size_t)ret_len);
		}

		*(ret_len + buffer) = '\0';
	}

	R->EAX(ret_len);
	return 0x528BFA;
}

ASMJIT_PATCH(0x526CC0, INIClass_Section_GetKeyName, 7)
{
	if (Phobos::Config::UseNewInheritance)
		return 0x0;

	GET(INIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(int, idx, 0x8);

	auto pResult = pThis->GetOrReturnCurrenSection(pSection);

	if (pResult && idx < pResult->EntryIndex.Count())
	{
		if (pResult == AresINIData::SectionCompare
			&& (AresINIData::KeyCompareIdx + 1) == idx
			&& AresINIData::NodeCompare != pResult->Entries.GetLast()
			)
		{

			auto result = AresINIData::NodeCompare->Next();
			++AresINIData::KeyCompareIdx;
			AresINIData::NodeCompare = result;
			R->EAX(((INIEntry*)result)->Key);
			return 0x526D8A;
		}
		else
		{
			auto v7 = pResult->Entries.GetFirst()->Next();

			for (int i = idx; i > 0; --i)
				v7 = v7->Next();

			AresINIData::KeyCompareIdx = idx;
			AresINIData::NodeCompare = v7;
			AresINIData::SectionCompare = pResult;
			R->EAX(((INIEntry*)v7)->Key);
			return 0x526D8A;
		}
	}

	R->EAX(0);
	return 0x526D8A;
}

ASMJIT_PATCH(0x5260d9, INIClass_Parse_Override, 7)
{
	struct INIClass_ {
		BYTE gap[44];
		IndexClass<int, INISection*> SectionIndex;
	};
	static_assert(sizeof(INIClass_) == 0x40, "Invalid Size!");

	GET_STACK(INIClass_*, pThis, 0x38);
	GET(int, CRC, EAX);

	auto find = pThis->SectionIndex.FetchItem(CRC, true);

	if (!find)
		return 0x0;

	//move deleted item forward ?
	if (auto pData = find->Data)
	{
		if (auto findB = pThis->SectionIndex.FetchItem(CRC, true))
		{
			const auto end = pThis->SectionIndex.end();
			const auto next = std::next(findB);
			std::memcpy(findB, next, (((char*)end) - ((char*)next)));
			const auto entries = pThis->SectionIndex.IndexTable;
			const auto countBefore = pThis->SectionIndex.IndexCount;
			entries[countBefore - 1].Data = 0;
			entries[countBefore - 1].ID = 0;
			--pThis->SectionIndex.IndexCount;
			pThis->SectionIndex.Archive = nullptr;
		}

		GameDelete<true, false>(pData);
	}
	return 0;
}

#ifndef IteratorChar
// Increment `+=` thingy
ASMJIT_PATCH(0x5260A2, INIClass_Parse_IteratorChar1, 6)
{
	GET(INIEntry*, entry, ESI);

	if (!CRT::strcmp(entry->Key, AresINIData::iteratorChar)) {

		char buffer[0x10];
		sprintf_s(buffer, AresINIData::iteratorReplacementFormat, AresINIData::iteratorValue++);

		if (auto data = std::exchange(entry->Key, CRT::strdup(buffer)))
			CRT::free(data);
	}

	return 0;
}

ASMJIT_PATCH(0x525D23, INIClass_Parse_IteratorChar2, 5)
{
	GET(char*, value, ESI);
	LEA_STACK(char*, key, 0x78)

	if (!CRT::strcmp(key, AresINIData::iteratorChar))
	{
		char buffer[0x200];
		strcpy_s(buffer, value);
		int len = sprintf_s(key, sizeof(buffer),
			AresINIData::iteratorReplacementFormat,
			AresINIData::iteratorValue++);

		if (len >= 0)
		{
			char* newValue = &key[len + 1];
			strcpy_s(newValue, 511 - len, buffer);
			R->ESI<char*>(newValue);
		}
	}

	return 0;
}
#endif

struct IniSectionIncludes
{
	static INISection* includedSection;
	static void CopySection(CCINIClass* ini, INISection* source, const char* dest);
};

INISection* IniSectionIncludes::includedSection = nullptr;

void IniSectionIncludes::CopySection(CCINIClass* ini, INISection* source, const char* destName)
{
	//browse through section entries and copy them over to the new section
	for (GenericNode* node = source->Entries.GetFirst()->Next(); node != source->Entries.GetLast(); node = node->Next())
	{
		INIEntry* entry = static_cast<INIEntry*>(node);
		ini->WriteString(destName, entry->Key, entry->Value);
	}
}

#ifndef INHERITANCE
ASMJIT_PATCH(0x525E44, INIClass_Parse_IniSectionIncludes_CopySection2, 7)
{
	if (IniSectionIncludes::includedSection)
	{
		GET_STACK(CCINIClass*, ini, 0x28);
		GET(INISection*, section, EBX);
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, section->Name);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0;
}

ASMJIT_PATCH(0x525C28, INIClass_Parse_IniSectionIncludes_CopySection1, 7)
{
	LEA_STACK(char*, sectionName, 0x79);
	GET_STACK(CCINIClass*, ini, 0x28);

	if (IniSectionIncludes::includedSection)
	{
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, sectionName);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0x0;
}

NOINLINE INISection* GetInheritedSection(INIClass* pThis, char* ptr)
{
	if (!ptr || !*ptr)
		return nullptr;

	// Skip leading whitespace
	char* cursor = ptr;
	while (*cursor && *cursor <= ' ')
		++cursor;

	// Expect colon (':') indicating inheritance
	if (*cursor != ':')
		return nullptr;

	// Skip whitespace after colon
	++cursor;
	while (*cursor && *cursor <= ' ')
		++cursor;

	// Expect opening bracket
	if (*cursor != '[')
		return nullptr;

	// Skip whitespace after '['
	++cursor;
	while (*cursor && *cursor <= ' ')
		++cursor;

	// Now cursor points to start of inherited section name
	char* start = cursor;

	// Look for closing bracket
	char* end = strchr(start, ']');
	if (!end || end == start)
		return nullptr;

	// Optional: if the character *after* ']' is a semicolon, accept it (Ares-style)
	if (*(end + 1) && *(end + 1) != ';' && *(end + 1) > ' ')
	{
		// Invalid termination (unexpected non-whitespace/semicolon)
		return nullptr;
	}

	// Trim trailing whitespace before ']'
	char* trim = end - 1;
	while (trim > start && *trim <= ' ')
		--trim;

	if (trim < start)
		return nullptr;

	// Null-terminate the section name
	*(trim + 1) = '\0';

	// Lookup the section
	if (auto section = pThis->GetSection(start))
	{
		return section;
	}

	Debug::LogError("An INI section inherits from section '{}', which doesn't exist or has not been parsed yet.", start);
	return nullptr;
}


ASMJIT_PATCH(0x525CA5, INIClass_Parse_IniSectionIncludes_PreProcess1, 8)
{
	GET(char*, ptr, EAX);
	GET_STACK(INIClass*, pThis, 0x28);

	if (!ptr)
		return 0x525CAD;

	IniSectionIncludes::includedSection = GetInheritedSection(pThis, ptr + 1);
	return 0x525D4D;
}

ASMJIT_PATCH(0x525DDB, INIClass_Parse_IniSectionIncludes_PreProcess2, 5)
{
	GET(char*, ptr, EAX);
	GET_STACK(INIClass*, pThis, 0x28);

	*ptr = '\0';
	IniSectionIncludes::includedSection = GetInheritedSection(pThis, ptr + 1);
	return 0x525DEA;
}
#endif

class NOVTABLE FakeCCIniClass : public CCINIClass {
public:
	int __GetPipType(const char* pSection, const char* pKey, int fallback) {
		if (this->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			int nbuffer;
			if (Parser<int>::TryParse(Phobos::readBuffer, &nbuffer)) {
				return nbuffer;
			} else {
				// find the pip value with the name specified
				for (const auto& data : TechnoTypeClass::PipsTypeName) {
					if (data == Phobos::readBuffer) {
						//Debug::LogInfo("[%s]%s=%s ([%d] from [%s]) ", pSection, pKey, Phobos::readBuffer, it->Value, it->Name);
						return data.Value;
					}
				}
			}

			Debug::INIParseFailed(pSection, pKey, Phobos::readBuffer, "Expect valid pip");
		}

		return fallback;
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x4748A0, FakeCCIniClass::__GetPipType)

// invalid or not set edge reads array out of bounds
ASMJIT_PATCH(0x4759D4, INIClass_WriteEdge, 0x7)
{
	GET(int const, index, EAX);

	if (index < 0 || index > 4)
	{
		R->EDX(GameStrings::NoneStrb());
		return 0x4759DB;
	}

	return 0;
}

ASMJIT_PATCH(0x687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Phobos::Otamaa::TrackParserErrors = false;

	return 0;
}
