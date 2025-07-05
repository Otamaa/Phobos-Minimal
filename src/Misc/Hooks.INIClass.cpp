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
#include <New/Interfaces/AdvancedDriveLocomotionClass.h>
#include <New/Interfaces/TestLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>
#include <New/Interfaces/TSJumpJetLocomotionClass.h>

#include <Straws.h>

#define PARSE(who)\
if (IS_SAME_STR_(parser.value(), who ##_data.s_name)) { \
	CLSID who ##_dummy; \
	std::wstring who ##_dummy_clsid = ## who ##_data.w_CLSID; \
	## who ##_dummy_clsid.insert(0, L"{"); \
	## who ##_dummy_clsid.insert(who ##_dummy_clsid.size(), L"}"); \
	const unsigned hr = CLSIDFromString(LPCOLESTR(who ##_dummy_clsid.data()), &who ##_dummy); \
	if (SUCCEEDED(hr)) {\
		value = ## who ##_dummy; return true;\
	} else {\
		Debug::LogError("Cannot find Locomotor [{} - {}({})] err : 0x{:08X}", pSection, parser.value(), PhobosCRT::WideStringToString(## who ##_dummy_clsid), hr);\
	}\
}

namespace detail
{
	template <>
	OPTIONALINLINE bool read<CLSID>(CLSID& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (!parser.ReadString(pSection, pKey))
			return false;

		// Semantic locomotor aliases
		if (parser.value()[0] != '{') {

			if (Phobos::Otamaa::IsAdmin)
				Debug::Log("Reading locomotor [%s] of [%s]\n" , parser.value() , pSection);

			for (size_t i = 0; i < EnumFunctions::LocomotorPairs_ToStrings.size(); ++i) {
				if (IS_SAME_STR_(parser.value(), EnumFunctions::LocomotorPairs_ToStrings[i].first)) {
					CLSID dummy;
					const unsigned hr = CLSIDFromString(LPCOLESTR(EnumFunctions::LocomotorPairs_ToWideStrings[i].second), &dummy);
					if (SUCCEEDED(hr)) {
						value = dummy; return true;
					}
					else {
						Debug::LogError("Cannot find Locomotor [{} - {}({})] err : 0x{:08X}", pSection, parser.value(), EnumFunctions::LocomotorPairs_ToStrings[i].second, hr);
					}
				}
			}

			PARSE(Levitate)
			PARSE(AdvancedDrive)
			PARSE(CustomRocket)
			PARSE(TSJumpJet)

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
		const unsigned hr = CLSIDFromString(wcharstr, &value);

		if(!SUCCEEDED(hr)){
			Debug::LogError("Cannot find Locomotor [{} - {}] err : 0x{:08X}", pSection, parser.value(),hr);
			return false;
		}

		return true;
	}
}

// passthrough instead of std::hash, because our keys are already unique CRCs
struct Passthrough {
	std::size_t operator()(int const& x) const noexcept {
		return x;
	}
};

struct INIInheritance
{
	static COMPILETIMEEVAL const char* const AresIncludesSection = "#include";
	static COMPILETIMEEVAL const char* const IncludesSection = "$include";

	static COMPILETIMEEVAL const char* const AresIncludesSectionB = "#Include";
	static COMPILETIMEEVAL const char* const IncludesSectionB = "$Include";
	static COMPILETIMEEVAL DWORD const IncludeSection_CRC = 0x3a6239ac;

	static COMPILETIMEEVAL const char* const InheritsSection = "$Inherits";
	static COMPILETIMEEVAL int inheritsCRC = -1871638965; // CRCEngine()("$Inherits", 9)

	static CCINIClass* LastINIFile;
	static std::set<std::string> SavedIncludes;
	static std::unordered_map<int, std::string, Passthrough> Inherits;

	static int Finalize(char* buffer ,int length, const char* result)
	{
		if (!result) {
			*buffer = NULL;
			return 0;
		}

		strncpy(buffer, result, length);
		buffer[length - 1] = NULL;
		CRT::strtrim(buffer);

		return (int)strlen(buffer);
	}

	static int ReadStringUseCRCActual(CCINIClass* ini, int sectionCRC, int entryCRC, const char* defaultValue, char* buffer, int length, bool useCurrentSection)
	{
		if (!buffer || length < 2)
			return 0;

		const INISection* pSection = useCurrentSection ?
			ini->CurrentSection :
			ini->SectionIndex.IsPresent(sectionCRC) ? ini->SectionIndex.Archive->Data : nullptr;

		if (!pSection)
			return Finalize(buffer ,length ,defaultValue);

		const auto pEntry = pSection->EntryIndex.IsPresent(entryCRC) ? pSection->EntryIndex.Archive->Data : nullptr;

		if (!pEntry)
			return Finalize(buffer, length, defaultValue);

		return Finalize(buffer, length, pEntry->Value ? pEntry->Value : defaultValue);
	}

	static int ReadString(
		CCINIClass* ini,
		int sectionCRC,
		int entryCRC,
		const char* defaultValue,
		char* buffer,
		int length,
		bool useCurrentSection,
		bool skipCheck = false
	) {
		int resultLen = 0;

		if (!skipCheck)
		{
			// check if this section has the actual entry, if yes, then we're done
			resultLen = INIInheritance::ReadStringUseCRCActual(ini, sectionCRC, entryCRC, NULL, buffer, length, useCurrentSection);
			if (resultLen != 0)
				return buffer[0] ? resultLen : 0;
		}

		// if we were looking for $Inherits and failed, no recursion
		if (entryCRC == inheritsCRC)
			return 0;

		// read $Inherits entry only once per section
		const auto it = INIInheritance::Inherits.find(sectionCRC);

		std::string inherits_result;

		if (it == INIInheritance::Inherits.end())
		{
			char stringBuffer[0x100];
			// if there's no saved $Inherits entry for this section, read now
			resultLen = INIInheritance::ReadStringUseCRCActual(ini, sectionCRC, inheritsCRC, NULL, stringBuffer, 0x100, useCurrentSection);
			INIInheritance::Inherits.emplace(sectionCRC, std::string(stringBuffer));

			// if we failed to find $Inherits, stop
			if (resultLen == 0)
				return Finalize(buffer,length, defaultValue);

			inherits_result = stringBuffer;
		}
		else
		{
			// use the saved $Inherits entry
			if (it->second.empty())
				return Finalize(buffer, length, defaultValue);

			// strdup because strtok edits the
			char* up = _strdup(it->second.c_str());
			inherits_result = up;
			free(up);
		}

		// for each section in csv, search for entry
		char* state = NULL;
		char* split = strtok_s(inherits_result.data(), Phobos::readDelims, &state);
		do
		{
			const int splitsCRC = CRCEngine()(split, strlen(split));

			// if we've found anything, we're done
			resultLen = INIInheritance::ReadString(ini, splitsCRC, entryCRC, defaultValue, buffer, length, false);

			if (resultLen != 0)
				break;

			split = strtok_s(NULL, Phobos::readDelims, &state);
		}
		while (split);

		return resultLen != 0 ? (buffer[0] ? resultLen : 0) : Finalize(buffer, length, defaultValue);
	}

	template<typename T>
	static OPTIONALINLINE T ReadTemplate(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(char*, section, 0x4);
		GET_STACK(char*, entry, 0x8);
		GET_STACK(T, defaultValue, 0xC);
		INI_EX exINI(ini);
		T result;

		if (!detail::read<T>(result, exINI, section, entry))
			result = defaultValue;
		return result;
	}

	template<typename T>
	static OPTIONALINLINE T* ReadTemplatePtr(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(T*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(T*, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<T>(*result, exINI, section, entry))
			*result = *defaultValue;
		return result;
	}

	// for some reason, WW passes the default locomotor by value
	template<>
	static OPTIONALINLINE CLSID* ReadTemplatePtr<CLSID>(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(CLSID*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(CLSID, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<CLSID>(*result, exINI, section, entry))
			*result = defaultValue;
		return result;
	}
};

CCINIClass* INIInheritance::LastINIFile;
std::set<std::string> INIInheritance::SavedIncludes;
std::unordered_map<int, std::string, Passthrough> INIInheritance::Inherits;

// INIClass__GetInt__Hack // pop edi, jmp + 6, nop
DEFINE_PATCH(0x5278C6, 0x5F, 0xEB, 0x06, 0x90);

ASMJIT_PATCH(0x5276D0, INIClass_ReadInt_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplate<int>(R));
	return 0x527838;
}

ASMJIT_PATCH(0x5295F0, INIClass_ReadBool_Overwrite, 0x5)
{
	R->EAX(INIInheritance::ReadTemplate<bool>(R));
	return 0x5297A3;
}

ASMJIT_PATCH(0x5283D0, INIClass_ReadDouble_Overwrite, 0x5)
{
	double value = INIInheritance::ReadTemplate<double>(R);
	_asm { fld value }
	return 0x52859F;
}

ASMJIT_PATCH(0x529880, INIClass_ReadPoint2D_Overwrite, 0x7)
{
	R->EAX(INIInheritance::ReadTemplatePtr<Point2D>(R));
	return 0x52859F;
}

ASMJIT_PATCH(0x529CA0, INIClass_ReadPoint3D_Overwrite, 0x8)
{
	R->EAX(INIInheritance::ReadTemplatePtr<CoordStruct>(R));
	return 0x529E63;
}

ASMJIT_PATCH(0x527920, INIClass_ReadGUID_Overwrite, 0x6) // locomotor
{
	R->EAX(INIInheritance::ReadTemplatePtr<CLSID>(R));
	return 0x527B43;
}

ASMJIT_PATCH(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0x6)
{
	if (!Phobos::Config::UseNewInheritance)
		return 0x0;

	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
	GET_STACK(int, entryCRC, STACK_OFFSET(stackOffset, 0x8));
	GET_STACK(int, sectionCRC, STACK_OFFSET(stackOffset, 0x4));

	// if we're in a different CCINIClass now, clear old data
	if (ini != INIInheritance::LastINIFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::Inherits.clear();
	}

	INIInheritance::ReadString(ini, sectionCRC, entryCRC, defaultValue, buffer, length, true, true);

	R->EDI(buffer);
	R->EAX(0);
	R->ECX(buffer[0] ? buffer : defaultValue);

	return 0x528BB6;
}


#pragma region INCLUDES

ASMJIT_PATCH(0x474230, CCINIClass_ReadCCFile1, 5)
{
	GET(CCINIClass*, pINI, ESI);
	//LEA_STACK(FileStraw*, pStraw, 0xBC);

	{
		//auto pFile = pStraw->File;
		//const char* filename = pFile->GetFileName();
		// if we're in a different CCINIClass now, clear old data
		if (pINI != INIInheritance::LastINIFile)
		{
			INIInheritance::LastINIFile = pINI;
			INIInheritance::SavedIncludes.clear();
		}

		auto section = pINI->GetSection(!Phobos::Config::UseNewIncludes ?
			INIInheritance::AresIncludesSection : INIInheritance::IncludesSection);

		if(!section) section = pINI->GetSection(!Phobos::Config::UseNewIncludes ?
			INIInheritance::AresIncludesSectionB : INIInheritance::IncludesSectionB);

		if (section) {

			for (auto& node : section->EntryIndex) {

				if (!node.Data || !node.Data->Value || !*node.Data->Value)
					continue;

				std::string nodefilename = std::string(node.Data->Value);

				if (INIInheritance::SavedIncludes.contains(nodefilename))
					continue;

				INIInheritance::SavedIncludes.insert(std::move(nodefilename));

				CCFileClass nFile { node.Data->Value };
				if (nFile.Exists()) {
					if(Phobos::Otamaa::IsAdmin)
						Debug::Log("Reading Included INI file %s !\n", node.Data->Value);

					INIInheritance::LastINIFile->ReadCCFile(&nFile, false, false);
				} else {

					if(!Phobos::Otamaa::IsAdmin)
						Debug::FatalErrorAndExit("Included INI file %s does not exist", node.Data->Value);
					else
						Debug::Log("Included INI file %s does not exist\n", node.Data->Value);
				}
			}
		}
	}

	return 0;
}

#pragma endregion

// Fix issue with TilesInSet caused by incorrect vanilla INIs and the fixed parser returning correct default value (-1) instead of 0 for existing non-integer values
int __fastcall IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper(INIClass* pThis, void* _, const char* pSection, const char* pKey, int defaultValue)
{
	if (pThis->Exists(pSection, pKey))
		return pThis->ReadInteger(pSection, pKey, 0);

	return defaultValue;
}

DEFINE_FUNCTION_JUMP(CALL, 0x545FD4, IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper);
