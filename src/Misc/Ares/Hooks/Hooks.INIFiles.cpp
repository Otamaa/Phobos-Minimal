#include <Phobos.h>
#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Parser.h>
#include <Utilities/Constructs.h>

#include "AresChecksummer.h"
/*
	Line 2541: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ca5 , INIClass_Parse_IniSectionIncludes_PreProcess1 , 8]
	Line 2543: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ddb , INIClass_Parse_IniSectionIncludes_PreProcess2 , 5]
*/

INIClass::INISection* SectionCompare;
int KeyCompareIdx;
GenericNode* NodeCompare;

constexpr const char* const iteratorChar = "+";
constexpr const char* const iteratorReplacementFormat = "var_%d";

int iteratorValue = 0;
#include <New/Type/GenericPrerequisite.h>

#pragma region Classes

NOINLINE INIClass::INISection* GetSection(INIClass* pINI, const char* pSection)
{
	if (pINI->CurrentSectionName == pSection)
		return pINI->CurrentSection;

	AresSafeChecksummer cf;

	if (pSection)
	{
		if (auto len = strlen(pSection))
		{
			const size_t len_t = len & 0xFFFFFFFC;

			if (len_t != 0)
			{
				cf.Value = AresSafeChecksummer::Process(pSection, len_t, 0);
				len -= len_t;
			}

			if (len)
	{
				*cf.Bytes = 0;
				std::memcpy(cf.Bytes, &pSection[len_t], len);
				cf.ByteIndex = len;
			}
		}

		cf.Value = cf.GetValue();
	}

	if (auto pData = pINI->SectionIndex.FetchItem(cf.Value, true))
	{
		if (pData->Data)
		{
			pINI->CurrentSection = pData->Data;
			pINI->CurrentSectionName = (char*)pSection;
			return pData->Data;
		}
	}

	return nullptr;
}

NOINLINE const char* GetKeyValue(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	if (INIClass::INISection* pSectionRes = GetSection(pINI, pSection))
	{
		AresSafeChecksummer cf;

		if (pKey)
		{
			if (auto len = strlen(pKey))
			{
				const size_t len_t = len & 0xFFFFFFFC;

				if (len_t != 0)
				{
					cf.Value = AresSafeChecksummer::Process(pKey, len_t, 0);
					len -= len_t;
				}

				if (len)
				{
					*cf.Bytes = 0;
					std::memcpy(cf.Bytes, &pKey[len_t], len);
					cf.ByteIndex = len;
				}
			}

			cf.Value = cf.GetValue();
		}

		if (auto pResult = pSectionRes->EntryIndex.FetchItem(cf.Value, true))
		{
			if (pResult->Data)
			{
				return pResult->Data->Value;
			}
		}
	}

	return pDefault;
}

#pragma endregion

DEFINE_STRONG_HOOK(0x528A10, INIClass_GetString, 5)
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

	if (buffer && bufferlength >= 2 && pSection && pKey)
	{
		if (auto result = GetKeyValue(pThis, pSection, pKey, pDefault))
		{
			auto resultcopy = result;
			for (auto i = *result; i <= ' '; i = *++resultcopy) {
				if (!i)
					break;
			}

			int len = strlen(resultcopy);
			for (; len; --len)
			{
				if (resultcopy[len - 1] > ' ')
					break;
			}

			ret_len = bufferlength - 1;
			if (bufferlength - 1 >= len)
				ret_len = len;

			std::memcpy(buffer, resultcopy, (size_t)ret_len);
		}

		*(ret_len + buffer) = '\0';
	}

	R->EAX(ret_len);
	return 0x528BFA;
}

DEFINE_STRONG_HOOK(0x526CC0, INIClass_Section_GetKeyName, 7)
{
	if (Phobos::Config::UseNewInheritance)
		return 0x0;

	GET(INIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(int, idx, 0x8);

	auto pResult = GetSection(pThis, pSection);

	if (pResult && idx < pResult->EntryIndex.Count())
	{
		if (pResult == SectionCompare
			&& (KeyCompareIdx + 1) == idx
			&& NodeCompare != pResult->Entries.GetLast()
			)
		{

			auto result = NodeCompare->Next();
			++KeyCompareIdx;
			NodeCompare = result;
			R->EAX(((INIClass::INIEntry*)result)->Key);
			return 0x526D8A;
		}
		else
		{
			auto v7 = pResult->Entries.GetFirst()->Next();

			for (int i = idx; i > 0; --i)
				v7 = v7->Next();

			KeyCompareIdx = idx;
			NodeCompare = v7;
			SectionCompare = pResult;
			R->EAX(((INIClass::INIEntry*)v7)->Key);
			return 0x526D8A;
		}
	}

	R->EAX(0);
	return 0x526D8A;
}

DEFINE_STRONG_HOOK(0x5260d9, INIClass_Parse_Override, 7)
{
	struct INIClass_ {
		BYTE gap[44];
		IndexClass<int, INIClass::INISection*> SectionIndex;
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
DEFINE_STRONG_HOOK(0x5260A2, INIClass_Parse_IteratorChar1, 6)
{
	GET(CCINIClass::INIEntry*, entry, ESI);

	if (!CRT::strcmp(entry->Key, iteratorChar)) {

		char buffer[0x10];
		sprintf_s(buffer, iteratorReplacementFormat, iteratorValue++);

		if (auto data = std::exchange(entry->Key, CRT::strdup(buffer)))
			CRT::free(data);
	}

	return 0;
}

DEFINE_STRONG_HOOK(0x525D23, INIClass_Parse_IteratorChar2, 5)
{
	GET(char*, value, ESI);
	LEA_STACK(char*, key, 0x78)

		if (!CRT::strcmp(key, iteratorChar))
		{
			char buffer[0x200];
			strcpy_s(buffer, value);
			int len = sprintf_s(key, sizeof(buffer),
				iteratorReplacementFormat,
				iteratorValue++);

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
	static INIClass::INISection* includedSection;
	static void CopySection(CCINIClass* ini, INIClass::INISection* source, const char* dest);
};

INIClass::INISection* IniSectionIncludes::includedSection = nullptr;

void IniSectionIncludes::CopySection(CCINIClass* ini, INIClass::INISection* source, const char* destName)
{
	//browse through section entries and copy them over to the new section
	for (GenericNode* node = source->Entries.GetFirst()->Next(); node != source->Entries.GetLast(); node = node->Next())
	{
		INIClass::INIEntry* entry = static_cast<INIClass::INIEntry*>(node);
		ini->WriteString(destName, entry->Key, entry->Value);
	}
}

#ifndef INHERITANCE
DEFINE_STRONG_HOOK(0x525E44, INIClass_Parse_IniSectionIncludes_CopySection2, 7)
{
	if (IniSectionIncludes::includedSection)
	{
		GET_STACK(CCINIClass*, ini, 0x28);
		GET(INIClass::INISection*, section, EBX);
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, section->Name);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0;
}

DEFINE_STRONG_HOOK(0x525C28, INIClass_Parse_IniSectionIncludes_CopySection1, 7)
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

NOINLINE INIClass::INISection* GetInheritedSection(INIClass* pThis, char* ptr)
{
	char* copy = ptr;
	for (auto i = *ptr; i <= ' '; i = *++copy)
	{
		if (!i)
			break;
	}

	if (*copy == ':')
	{
		char* copy_1 = copy + 1;
		for (auto j = *(copy + 1); j <= ' '; j = *++copy_1)
		{
			if (!j)
				break;
		}

		if (*copy_1 == '[')
		{
			char copy_2 = *(copy_1 + 1);
			char* copy_2_2 = (copy_1 + 1);
			for (; copy_2 <= ' '; copy_2 = *++copy_2_2)
			{
				if (!copy_2)
					break;
			}

			// Ares 3.0p1 was "];" , and it is not working ???
			// change it to "]" it is working fine
			// what it is really , i dont understand ,..
			if (char* get = strchr(copy_2_2, ']'))
			{
				if (*get == ']' && copy_2_2 != get)
				{
					while (*(get - 1) <= ' ')
					{
						if (copy_2_2 == --get)
						{
							return nullptr;
						}
					}

					if (copy_2_2 != get)
					{
						*get = 0;

						if (auto section = pThis->GetSection(copy_2_2))
						{
							//Debug::Log("Inheritance Result [%s].\n" , copy_2_2);
							return section;
						}

						Debug::Log(Debug::Severity::Warning, "An INI section inherits from section '%s', which doesn't exist or has not been parsed yet.\n", copy_2_2);
					}
				}
			}
		}
	}

	return nullptr;
}

DEFINE_STRONG_HOOK(0x525CA5, INIClass_Parse_IniSectionIncludes_PreProcess1, 8)
{
	GET(char*, ptr, EAX);
	GET_STACK(INIClass*, pThis, 0x28);

	if (!ptr)
		return 0x525CAD;

	IniSectionIncludes::includedSection = GetInheritedSection(pThis, ptr + 1);
	return 0x525D4D;
}

DEFINE_STRONG_HOOK(0x525DDB, INIClass_Parse_IniSectionIncludes_PreProcess2, 5)
{
	GET(char*, ptr, EAX);
	GET_STACK(INIClass*, pThis, 0x28);

	*ptr = '\0';
	IniSectionIncludes::includedSection = GetInheritedSection(pThis, ptr + 1);
	return 0x525DEA;
}
#endif

#ifndef INCLUDES
int LastReadIndex = -1;
std::vector<CCINIClass*> LoadedINIs;
std::vector<std::string> LoadedINIFiles;

DEFINE_STRONG_HOOK(0x474200, CCINIClass_ReadCCFile1, 6)
{
	if (Phobos::Config::UseNewIncludes)
		return 0x0;

	GET(CCINIClass*, pINI, ECX);
	GET(CCFileClass*, pFile, EAX);

	const char* filename = pFile->GetFileName();

	if (LoadedINIs.empty())
		LoadedINIs.push_back(pINI);
	else
		LoadedINIs.insert(LoadedINIs.begin(), pINI);

	char* upped = CRT::strdup(filename);
	if (LoadedINIFiles.empty())
		LoadedINIFiles.push_back(upped);
	else
		LoadedINIFiles.insert(LoadedINIFiles.begin(), upped);

	CRT::free(upped);
	return 0;
}

DEFINE_STRONG_HOOK(0x474314, CCINIClass_ReadCCFile2, 6)
{
	if (Phobos::Config::UseNewIncludes)
		return 0x0;

	char buffer[0x80];
	CCINIClass* xINI = LoadedINIs.back();

	if (!xINI)
	{
		return 0;
	}

	const char* section = "#include";

	for (int i = LastReadIndex;
		i < xINI->GetKeyCount(section);
		i = LastReadIndex)
	{
		const char* key = xINI->GetKeyName(section, i);
		++LastReadIndex;
		buffer[0] = '\0';
		if (xINI->ReadString(section, key, Phobos::readDefval, buffer))
		{
			bool canLoad = true;
			for (auto& LoadedINI : LoadedINIFiles)
			{
				if (IS_SAME_STR_(LoadedINI.c_str(), buffer))
				{
					canLoad = false;
					break;
				}
			}

			if (canLoad)
			{

				CCFileClass xFile { buffer };
				if (xFile.Exists())
				{
					xINI->ReadCCFile(&xFile);
				}
			}
		}
	}

	LoadedINIs.pop_back();
	if (LoadedINIs.empty())
	{
		LoadedINIFiles.clear();
		LastReadIndex = -1;
	}
	return 0;
}

#endif

// replaces entire function (without the pip distortion bug)
DEFINE_STRONG_HOOK(0x4748A0, INIClass_GetPipIdx, 0x7)
{
	GET(INIClass*, pINI, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, fallback, 0xC);

	if (pINI->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer) > 0)
	{
		int nbuffer;
		if (Parser<int>::TryParse(Phobos::readBuffer, &nbuffer))
		{
			R->EAX(nbuffer);
			return 0x474907;
		}
		else
		{
			// find the pip value with the name specified
			for (const auto& data : TechnoTypeClass::PipsTypeName)
			{
				if (data == Phobos::readBuffer)
				{
					//Debug::Log("[%s]%s=%s ([%d] from [%s]) \n", pSection, pKey, Phobos::readBuffer, it->Value, it->Name);
					R->EAX(data.Value);
					return 0x474907;
				}
			}
		}

		Debug::INIParseFailed(pSection, pKey, Phobos::readBuffer, "Expect valid pip");
	}

	R->EAX(fallback);
	return 0x474907;
}

// invalid or not set edge reads array out of bounds
DEFINE_STRONG_HOOK(0x4759D4, INIClass_WriteEdge, 0x7)
{
	GET(int const, index, EAX);

	if (index < 0 || index > 4)
	{
		R->EDX(GameStrings::NoneStrb());
		return 0x4759DB;
	}

	return 0;
}

DEFINE_STRONG_HOOK(0x687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Phobos::Otamaa::TrackParserErrors = false;

	return 0;
}
