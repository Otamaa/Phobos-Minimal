#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>
#include <CCINIClass.h>

#include <CRT.h>

#include <vector>
#include <set>
#include <string>
#include <atlstr.h>

#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/TestLocomotionClass.h>

#ifdef ENABLE_PHOBOS_INHERITANCE
namespace detail
{
	template <>
	inline bool read<CLSID>(CLSID& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadString(pSection, pKey))
			return false;

		// Semantic locomotor aliases
		if (parser.value()[0] != '{') {
			for (size_t i = 0; i < EnumFunctions::LocomotorPairs_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::LocomotorPairs_ToStrings[i].first)) {
					CLSID dummy;
					if (CLSIDFromString(LPCOLESTR(EnumFunctions::LocomotorPairs_ToWideStrings[i].second), &dummy) == NOERROR) {
						value = dummy;
						return true;
					}
				}
			}

			if (IS_SAME_STR_(parser.value(), Test_data.s_name)) {
				CLSID dummy;
				if (CLSIDFromString(LPCOLESTR(Test_data.w_CLSID), &dummy) == NOERROR)
				{
					value = dummy;
					return true;
				}
			}

			if (IS_SAME_STR_(parser.value(), Levitate_data.s_name)) {
				CLSID dummy;
				if (CLSIDFromString(LPCOLESTR(Levitate_data.w_CLSID), &dummy) == NOERROR)
				{
					value = dummy;
					return true;
				}
			}

			//AddMore loco here
			return false;
		}

		CHAR bytestr[128];
		WCHAR wcharstr[128];

		strncpy(bytestr, parser.value(), 128);
		bytestr[127] = NULL;
		CRT::strtrim(bytestr);

		if (!strlen(bytestr))
			return false;

		MultiByteToWideChar(0, 1, bytestr, -1, wcharstr, 128);
		return CLSIDFromString(wcharstr, &value) == NOERROR;
	}
}

namespace INIInheritance
{	// passthrough instead of std::hash, because our keys are already unique CRCs
	struct Passthrough
	{
		std::size_t operator()(int const& x) const noexcept
		{
			return x;
		}
	};

	constexpr const char* const InHeritsStr = "$Inherits";
	CCINIClass* LastINIFile = nullptr;
	std::set<std::string> SavedIncludes;
	std::unordered_map<int, std::string, Passthrough> Inherits;

	template<typename T>
	T* ReadTemplatePtr(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(T*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(T*, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<T>(*result, exINI, section, entry, false))
			*result = *defaultValue;
		return result;
	}

	// for some reason, WW passes the default locomotor by value
	template<>
	CLSID* ReadTemplatePtr<CLSID>(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(CLSID*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(CLSID, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<CLSID>(*result, exINI, section, entry, false))
			*result = defaultValue;
		return result;
	}

	template<typename T>
	T ReadTemplate(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(char*, section, 0x4);
		GET_STACK(char*, entry, 0x8);
		GET_STACK(T, defaultValue, 0xC);
		INI_EX exINI(ini);
		T result;

		if (!detail::read<T>(result, exINI, section, entry, false))
			result = defaultValue;
		return result;
	}

	template<>
	bool ReadTemplate(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(char*, section, 0x4);
		GET_STACK(char*, entry, 0x8);
		GET_STACK(bool, defaultValue, 0xC);
		INI_EX exINI(ini);
		bool result;

		if (!detail::read<bool>(result, exINI, section, entry, false))
			result = defaultValue;

		return result;
	}

	int ReadStringUseCRC(CCINIClass* ini, int sectionCRC, int entryCRC, char* defaultValue, char* buffer, int length, bool useCurrentSection)
	{
		const auto finalize = [buffer, length](char* result)
		{
			if (!result)
			{
				*buffer = NULL;
				return 0;
			}
			strncpy(buffer, result, length);
			buffer[length - 1] = NULL;
			CRT::strtrim(buffer);

			return (int)strlen(buffer);
		};

		if (!buffer || length < 2)
			return 0;

		INIClass::INISection* pSection = useCurrentSection ?
			ini->CurrentSection : ini->SectionIndex.IsPresent(sectionCRC)
			? ini->SectionIndex.Archive->Data : nullptr;

		if (!pSection)
			return finalize(defaultValue);

		const auto pEntry = pSection->EntryIndex.IsPresent(entryCRC) ? pSection->EntryIndex.Archive->Data : nullptr;
		if (!pEntry)
			return finalize(defaultValue);

		return finalize(pEntry->Value ? pEntry->Value : defaultValue);
	}

	int ReadString(REGISTERS* R, int address)
	{
		const int stackOffset = 0x1C;
		GET(CCINIClass*, ini, EBP);
		GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
		GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
		GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
		GET_STACK(int, entryCRC, STACK_OFFSET(stackOffset, 0x8));
		GET_STACK(int, sectionCRC, STACK_OFFSET(stackOffset, 0x4));

		const auto finalize = [R, buffer, address](const char* value)
		{
			R->EDI(buffer);
			R->EAX(0);
			R->ECX(value);
			return address;
		};

		const constexpr int inheritsCRC = -1871638965; // CRCEngine()("$Inherits", 9)

		// if we were looking for $Inherits and failed, no recursion
		if (entryCRC == inheritsCRC)
			return finalize(defaultValue);

		// read $Inherits entry only once per section
		const auto it = INIInheritance::Inherits.find(sectionCRC);
		char* inherits;
		if (it == INIInheritance::Inherits.end())
		{
			// if there's no saved $Inherits entry for this section, read now
			char stringBuffer[0x100];
			const int retval = INIInheritance::ReadStringUseCRC(ini, sectionCRC, inheritsCRC, NULL, stringBuffer, 0x100, true);
			INIInheritance::Inherits.emplace(sectionCRC, std::string(stringBuffer));
			if (retval == 0)
				return finalize(defaultValue);
			inherits = stringBuffer;
		}
		else
		{
			// use the saved $Inherits entry
			if (it->second.empty())
				return finalize(defaultValue);
			// strdup because strtok edits the string
			inherits = _strdup(it->second.c_str());
		}

		// for each section in csv, search for entry
		char* state = NULL;
		char* split = strtok_s(inherits, Phobos::readDelims, &state);
		do
		{
			CRCEngine nCRC {};
			nCRC(split, strlen(split));
			const int splitsCRC = nCRC;

			// if we've found anything new, we're done
			if (INIInheritance::ReadStringUseCRC(ini, splitsCRC, entryCRC, NULL, buffer, length, false) != 0)
				break;
			split = strtok_s(NULL, Phobos::readDelims, &state);
		}
		while (split);
		free(inherits);

		return finalize(buffer[0] ? buffer : defaultValue);
	}
};

//
DEFINE_DISABLE_HOOK(0x528A10, INIClass_GetString_DisableAres);
//
DEFINE_DISABLE_HOOK(0x526CC0, INIClass_GetKeyName_DisableAres);
// INIClass__GetInt__Hack // pop edi, jmp + 6, nop
DEFINE_PATCH(0x5278C6, 0x5F, 0xEB, 0x06, 0x90);
//
DEFINE_DISABLE_HOOK(0x474200, CCINIClass_ReadCCFile1_DisableAres)
//
DEFINE_DISABLE_HOOK(0x474314, CCINIClass_ReadCCFile2_DisableAres)

DEFINE_HOOK(0x5276D0, INIClass_ReadInt_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplate<int>(R));
	return 0x527838;
}

DEFINE_HOOK(0x5295F0, INIClass_ReadBool_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplate<bool>(R));
	return 0x5297A3;
}

DEFINE_HOOK(0x5283D0, INIClass_ReadDouble_Overwrite, 0x5)
{
	double value = INIInheritance::ReadTemplate<double>(R);
	_asm { fld value }
	return 0x52859F;
}

DEFINE_HOOK(0x529880, INIClass_ReadPoint2D_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplatePtr<Point2D>(R));
	return 0x52859F;
}

DEFINE_HOOK(0x529CA0, INIClass_ReadPoint3D_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplatePtr<CoordStruct>(R));
	return 0x529E63;
}

DEFINE_HOOK(0x527920, INIClass_ReadGUID_Overwrite, 0x5) // locomotor
{
	R->EAX(INIInheritance::ReadTemplatePtr<CLSID>(R));
	return 0x527B43;
}

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	return INIInheritance::ReadString(R, 0x528BB6);
}

DEFINE_HOOK(0x474230, CCINIClass_Load_Inheritance, 0x5)
{
	GET(CCINIClass*, ini, ESI);

	// if we're in a different CCINIClass now, clear old data
	if (ini != INIInheritance::LastINIFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::SavedIncludes.clear();
	}

	auto section = ini->GetSection(INIInheritance::InHeritsStr);
	if (!section)
		return 0;

	for (auto& node : section->EntryIndex)
	{
		if (!node.Data || !node.Data->Value || !*node.Data->Value)
			continue;

		auto filename = std::string(node.Data->Value);

		if (filename.empty())
			continue;

		if (INIInheritance::SavedIncludes.contains(filename))
			continue;

		INIInheritance::SavedIncludes.insert(std::move(filename));

		CCFileClass nFile { node.Data->Value };
		if (nFile.Exists())
			INIInheritance::LastINIFile->ReadCCFile(&nFile, false, false);
	}

	return 0;
}


#else

// Fix issue with TilesInSet caused by incorrect vanilla INIs and the fixed parser returning correct default value (-1) instead of 0 for existing non-integer values
int __fastcall IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper(INIClass* pThis, void* _, const char* pSection, const char* pKey, int defaultValue)
{
	if (pThis->Exists(pSection, pKey))
		return pThis->ReadInteger(pSection, pKey, 0);

	return defaultValue;
}

DEFINE_JUMP(CALL, 0x545FD4, GET_OFFSET(IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper));

DEFINE_HOOK(0x527B0A, INIClass_Get_UUID, 0x8)
{
	GET(wchar_t*, buffer, ECX);
	constexpr size_t BufferSize = 0x80;

	if (buffer[0] != L'{')
	{
		for (auto const&[name , CLSID] : EnumFunctions::LocomotorPairs_ToWideStrings) {
			if (CRT::wcsicmp(buffer, name) == 0) {
				wcscpy_s(buffer, BufferSize, CLSID);
				return 0;
			}
		}

		//if (IS_SAME_WSTR(buffer, Test_data.w_name)) {
		//	wcscpy_s(buffer, BufferSize, Test_data.w_CLSID);
		//	return 0;
		//}

		if (IS_SAME_WSTR(buffer, Levitate_data.w_name)) {
			wcscpy_s(buffer, BufferSize, Levitate_data.w_CLSID);
			return 0;
		}
	}

	return 0;
}
#endif
