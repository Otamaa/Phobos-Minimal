#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>

#include <CRT.h>

#include <vector>
#include <set>
#include <string>

DEFINE_HOOK(0x527B0A, INIClass_Get_UUID, 0x8)
{
	GET(wchar_t*, buffer, ECX);

	if (buffer[0] != L'{') {

		if (CRT::wcsicmp(buffer, L"Drive") == 0) {
			CRT::wcscpy(buffer, L"{4A582741-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Jumpjet") == 0) {
			CRT::wcscpy(buffer, L"{92612C46-F71F-11d1-AC9F-006008055BB5}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Hover") == 0) {
			CRT::wcscpy(buffer, L"{4A582742-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Rocket") == 0) {
			wcscpy(buffer, L"{B7B49766-E576-11d3-9BD9-00104B972FE8}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Tunnel") == 0) {
			CRT::wcscpy(buffer, L"{4A582743-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Walk") == 0) {
			CRT::wcscpy(buffer, L"{4A582744-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"DropPod") == 0) {
			CRT::wcscpy(buffer, L"{4A582745-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Fly") == 0) {
			CRT::wcscpy(buffer, L"{4A582746-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Teleport") == 0) {
			CRT::wcscpy(buffer, L"{4A582747-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Mech") == 0) {
			CRT::wcscpy(buffer, L"{55D141B8-DB94-11d1-AC98-006008055BB5}");
			return 0;
		}

		if(CRT::wcsicmp(buffer, L"Ship") == 0) {
			CRT::wcscpy(buffer, L"{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}");
			return 0;
		}
	}

	return 0;
}

#ifdef ENABLE_PHOBOS_INHERITANCE
struct INIInheritance
{
	static inline std::vector<char*> SavedEntries {};
	static inline std::vector<char*> SavedSections {};
	static inline std::set<std::string> SavedIncludes {};
	static inline constexpr const char* const InHeritsStr = "$Inherits";
	static inline CCINIClass* LastINIFile { nullptr};

	static int ReadInt(REGISTERS* R, int address)
	{
		const int stackOffset = 0x1C;
		GET(CCINIClass*, ini, EBX);
		GET_STACK(int, defaultValue, STACK_OFFSET(stackOffset, 0xC));

		char* entryName = INIInheritance::SavedEntries.back();
		char* sectionName = INIInheritance::SavedSections.back();

		auto finalize = [R, address](int value)
		{
			R->Stack<int>(STACK_OFFSET(stackOffset, 0xC), value);
			return address;
		};

		// search for $Inherits entry
		char inheritSectionsString[0x100];
		if (ini->ReadString(sectionName, InHeritsStr, NULL, inheritSectionsString, 0x100) == 0)
			return finalize(defaultValue);

		// for each section in csv, search for entry
		int buffer = MAXINT;
		char* state = NULL;


		for(char* split = strtok_s(inheritSectionsString, Phobos::readDelims, &state); 
			split; 
			split = strtok_s(NULL, Phobos::readDelims, &state))
		{
			// if we found anything new (not default), we're done
			buffer = ini->ReadInteger(split, entryName, MAXINT);
			if (buffer != MAXINT)
				break;
		}

		return finalize(buffer != MAXINT ? buffer : defaultValue);
	}

	static int ReadString(REGISTERS* R, int address)
	{
		const int stackOffset = 0x1C;
		GET(CCINIClass*, ini, EBP);
		GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
		GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
		GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));

		char* entryName = INIInheritance::SavedEntries.back();
		char* sectionName = INIInheritance::SavedSections.back();

		auto finalize = [R, buffer, address](const char* value)
		{
			R->EDI(buffer);
			R->EAX(0);
			R->ECX(value);
			return address;
		};

		// if we were looking for $Inherits and failed, no recursion
		if (strncmp(entryName, InHeritsStr, 10) == 0)
			return finalize(defaultValue);

		// search for $Inherits entry
		char inheritSectionsString[0x100];
		if (ini->ReadString(sectionName, InHeritsStr, NULL, inheritSectionsString, 0x100) == 0)
			return finalize(defaultValue);

		// for each section in csv, search for entry
		char* state = NULL;
		for(char* split = strtok_s(inheritSectionsString, Phobos::readDelims, &state);  
			split; 
			split = strtok_s(NULL, Phobos::readDelims, &state))
		{
			// if we found anything new (not default), we're done
			if (ini->ReadString(split, entryName, NULL, buffer, length) != 0)
				break;
		}

		return finalize(buffer[0] ? buffer : defaultValue);
	}

	static void PushEntry(REGISTERS* R, int stackOffset)
	{
		GET_STACK(char*, entryName, STACK_OFFSET(stackOffset, 0x8));
		GET_STACK(char*, sectionName, STACK_OFFSET(stackOffset, 0x4));
		INIInheritance::SavedEntries.push_back(_strdup(entryName));
		INIInheritance::SavedSections.push_back(_strdup(sectionName));
	}

	static void PopEntry()
	{
		if (char* entry = INIInheritance::SavedEntries.back())
			free(entry);

		INIInheritance::SavedEntries.pop_back();

		if (char* section = INIInheritance::SavedSections.back())
			free(section);

		INIInheritance::SavedSections.pop_back();
	}
};

// INIClass_GetString_DisableAres
DEFINE_PATCH(0x528A10, 0x83, 0xEC, 0x0C, 0x33, 0xC0);
// INIClass_GetKeyName_DisableAres
DEFINE_PATCH(0x526CC0, 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x0C);
// INIClass__GetInt__Hack // pop edi, jmp + 6, nop
DEFINE_PATCH(0x5278C6, 0x5F, 0xEB, 0x06, 0x90);
// CCINIClass_ReadCCFile1_DisableAres
DEFINE_PATCH(0x474200, 0x8B, 0xF1, 0x8D, 0x54, 0x24, 0x0C)
// CCINIClass_ReadCCFile2_DisableAres
DEFINE_PATCH(0x474314, 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00)

DEFINE_HOOK(0x528A18, INIClass_GetString_SaveEntry, 0x6)
{
	INIInheritance::PushEntry(R, 0x18);
	return 0;
}

DEFINE_HOOK(0x5276D7, INIClass_GetInt_SaveEntry, 0x6)
{
	INIInheritance::PushEntry(R, 0x14);
	return 0;
}

DEFINE_HOOK_AGAIN(0x528BC9, INIClass_GetString_FreeEntry, 0x5)
DEFINE_HOOK(0x528BBE, INIClass_GetString_FreeEntry, 0x5)
{
	INIInheritance::PopEntry();
	return 0;
}

DEFINE_HOOK_AGAIN(0x52782F, INIClass_GetInt_FreeEntry, 0x5)
DEFINE_HOOK_AGAIN(0x5278A9, INIClass_GetInt_FreeEntry, 0x7)
DEFINE_HOOK(0x527866, INIClass_GetInt_FreeEntry, 0x7)
{
	INIInheritance::PopEntry();
	return 0;
}

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	return INIInheritance::ReadString(R, 0x528BB6);
}

DEFINE_HOOK(0x5278CA, INIClass_GetInt_Inheritance_NoEntry, 0x5)
{
	int r = INIInheritance::ReadInt(R, 0);
	INIInheritance::PopEntry();
	return r;
}

DEFINE_HOOK(0x474230, CCINIClass_Load_Inheritance, 0x5)
{
	GET(CCINIClass*, ini, ESI);

	bool isSameFile = ini == INIInheritance::LastINIFile;
	if (!isSameFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::SavedIncludes.clear();
	}

	auto section = ini->GetSection(INIInheritance::InHeritsStr);
	if (!section)
		return 0;

	for (auto node : section->EntryIndex)
	{
		if (!node.Data || !node.Data->Value || !*node.Data->Value)
			continue;

		auto filename = std::string(node.Data->Value);

		if (INIInheritance::SavedIncludes.contains(filename))
			continue;

		INIInheritance::SavedIncludes.insert(filename);

		auto file = GameCreate<CCFileClass>(node.Data->Value);

		if (file->Exists())
			INIInheritance::LastINIFile->ReadCCFile(file, false, false);

		GameDelete(file);
	}

	return 0;
}
#endif