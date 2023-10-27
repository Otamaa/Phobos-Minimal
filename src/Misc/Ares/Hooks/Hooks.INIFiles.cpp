#include <Phobos.h>
#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Parser.h>

#include "AresChecksummer.h"


/*
 	Line 2049: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x474200 , CCINIClass_ReadCCFile1 , 6]
	Line 2050: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x474314 , CCINIClass_ReadCCFile2 , 6]
	Line 2541: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ca5 , INIClass_Parse_IniSectionIncludes_PreProcess1 , 8]
	Line 2542: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525d23 , INIClass_Parse_IteratorChar2 , 5]
	Line 2543: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x525ddb , INIClass_Parse_IniSectionIncludes_PreProcess2 , 5]
	Line 2545: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x5260a2 , INIClass_Parse_IteratorChar1 , 6]
	Line 2546: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x5260d9 , INIClass_Parse_Override , 7]
	Line 2547: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x526cc0 , INIClass_Section_GetKeyName , 7]
	Line 2549: [15:45:23] SyringeDebugger::HandleException: Ares.dll [0x528a10 , INIClass_GetString , 5]
*/

// replaces entire function (without the pip distortion bug)
DEFINE_OVERRIDE_HOOK(0x4748A0, INIClass_GetPipIdx, 0x7)
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
DEFINE_OVERRIDE_HOOK(0x4759D4, INIClass_WriteEdge, 0x7)
{
	GET(int const, index, EAX);

	if (index < 0 || index > 4)
	{
		R->EDX(GameStrings::NoneStrb());
		return 0x4759DB;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Phobos::Otamaa::TrackParserErrors = false;

	return 0;
}

struct IniSectionIncludes
{
	static INIClass::INISection* includedSection;
	static void CopySection(CCINIClass* ini, INIClass::INISection* source, const char* dest);
};

INIClass::INISection* IniSectionIncludes::includedSection = nullptr;

void IniSectionIncludes::CopySection(CCINIClass* ini, INIClass::INISection* source, const char* destName)
{
	//browse through section entries and copy them over to the new section
	for (GenericNode* node = *source->Entries.First(); node; node = node->Next())
	{
		INIClass::INIEntry* entry = static_cast<INIClass::INIEntry*>(node);
		ini->WriteString(destName, entry->Key, entry->Value); //simple but effective
	}
}

DEFINE_OVERRIDE_HOOK(0x525E44, INIClass_Parse_IniSectionIncludes_CopySection2, 7)
{
	if (IniSectionIncludes::includedSection) {
		GET_STACK(CCINIClass*, ini, 0x28);
		GET(INIClass::INISection*, section, EBX);
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, section->Name);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x525C28, INIClass_Parse_IniSectionIncludes_CopySection1, 7)
{
	LEA_STACK(char*, sectionName, 0x278);
	GET_STACK(CCINIClass*, ini, 0x28);

	if (IniSectionIncludes::includedSection) {
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, sectionName);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0x0;
}
