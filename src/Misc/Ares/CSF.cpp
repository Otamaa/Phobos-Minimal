#include "CSF.h"

#include <CCFileClass.h>
#include <Phobos.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <Helpers/Macro.h>

#pragma region defines
int CSFLoader::CSFCount {};
int CSFLoader::NextValueIndex {};
std::unordered_map<std::string, CSFLoader::CSFStringStorage> CSFLoader::DynamicStrings {};
#pragma endregion

void CSFLoader::LoadAdditionalCSF(const char* pFileName, bool ignoreLanguage)
{
	//The main stringtable must have been loaded (memory allocation)
	//To do that, use StringTable::LoadFile.
	if (StringTable::IsLoaded && pFileName && *pFileName)
	{
		CCFileClass file { pFileName };

		if (file.Exists() && file.Open(FileAccessMode::Read))
		{
			CSFHeader header {};

			if (file.Read(header))
			{
				if (header.Signature == CSF_SIGNATURE &&
					header.CSFVersion >= 2 &&
					(header.Language == StringTable::Language //should stay in one language
						|| header.Language == static_cast<CSFLanguages>(-1)
						|| ignoreLanguage))
				{
					++CSFCount;
					StringTable::ReadFile(pFileName); //must be modified to do the rest ;)

					std::sort(StringTable::Labels(), StringTable::Labels() + StringTable::LabelCount(),
						[](const CSFLabel& lhs, const CSFLabel& rhs)
					{
						return IMPL_STRCMPI(lhs.Name, rhs.Name) < 0;
					});
				}
			}
		}
	}
}

const wchar_t* CSFLoader::GetDynamicString(const char* pLabelName, const wchar_t* pPattern, const char* pDefault, bool isNostr)
{
	auto pData = FindOrAllocateDynamicStrings(pLabelName);

	if(!pData->TextLoaded) {
		swprintf_s(pData->Text, std::size(pData->Text), pPattern, pDefault);
		pData->TextLoaded = true;
		pData->IsMissingValue = !isNostr;

		if(Phobos::Otamaa::OutputMissingStrings && !isNostr) {
			Debug::LogInfo("[CSFLoader] ***NO_STRING*** label \"{}\" with value \"{}\".", pLabelName, PhobosCRT::WideStringToString(pData->Text));
		}
	}

	return pData->Text;
}

ASMJIT_PATCH(0x7349cf, StringTable_ParseFile_Buffer, 7)
{
	LEA_STACK(CCFileClass*, pFile, 0x28);

	if (!R->Stack<void*>(0x80))
	{
		const auto size = pFile->GetFileSize();
		void* ptr = nullptr;
		if (size > 0)
			ptr = YRMemory::Allocate(size);

		pFile->ReadBytes(ptr, size);
		const auto IsAllocated = R->Stack<bool>(0x88);
		const auto tempPtr = R->Stack<void*>(0x80);

		R->Stack(0x80, ptr);
		R->Stack(0x84, size);
		R->Stack(0x88, size > 0);

		if (IsAllocated)
			YRMemory::Deallocate(tempPtr);
	}

	return 0x0;
}


ASMJIT_PATCH(0x7346D0, CSF_LoadBaseFile, 6)
{
	StringTable::IsLoaded = true;
	CSFLoader::CSFCount = 0;
	return 0;
}

ASMJIT_PATCH(0x734823, CSF_AllocateMemory, 6)
{
	//aaaah... finally, some serious hax :)
	//we don't allocate memory by the amount of labels in the base CSF,
	//but enough for exactly MaxEntries entries.
	//We're assuming we have only one value for one label, which is standard.

	StringTable::Labels = GameCreateArray<CSFLabel>(CSFLoader::MaxEntries);
	StringTable::Values = GameCreateArray<wchar_t*>(CSFLoader::MaxEntries);
	StringTable::ExtraValues = GameCreateArray<char*>(CSFLoader::MaxEntries);

	return 0x7348BC;
}

ASMJIT_PATCH(0x734A5F, CSF_AddOrOverrideLabel, 5)
{
	if (CSFLoader::CSFCount > 0)
	{
		if (CSFLabel* pLabel = static_cast<CSFLabel*>(bsearch(
			StringTable::GlobalBuffer(), //label buffer, char[4096]
			StringTable::Labels(),
			(size_t)StringTable::LabelCount(),
			sizeof(CSFLabel),
			(int(__cdecl*)(const void*, const void*))_strcmpi)))
		{
			//Label already exists - override!

			//If you study the CSF format deeply, you'll call this method suboptimal,
			//because it assumes that we have only one value assigned to one label.
			//This is always the case for RA2, but in no way a limit!
			//Just adding this as a note.
			int idx = pLabel->FirstValueIndex;
			CSFLoader::NextValueIndex = idx;

			wchar_t** pValues = StringTable::Values;
			if (pValues[idx])
			{
				YRMemory::Deallocate(pValues[idx]);
				pValues[idx] = nullptr;
			}

			char** pExtraValues = StringTable::ExtraValues;
			if (pExtraValues[idx])
			{
				YRMemory::Deallocate(pExtraValues[idx]);
				pExtraValues[idx] = nullptr;
			}

			auto ix = pLabel - StringTable::Labels;
			R->EBP(ix * sizeof(CSFLabel));
		}
		else
		{
			//Label doesn't exist yet - add!
			int idx = StringTable::ValueCount;
			CSFLoader::NextValueIndex = idx;
			StringTable::ValueCount = idx + 1;
			StringTable::LabelCount = StringTable::LabelCount + 1;

			R->EBP(idx * sizeof(CSFLabel)); //set the index
		}
	}

	return 0;
}

ASMJIT_PATCH(0x734A97, CSF_SetIndex, 6)
{
	R->EDX(StringTable::Labels());
	R->ECX(CSFLoader::CSFCount > 0 ? CSFLoader::NextValueIndex : R->Stack32(0x18));
	return 0x734AA1;
}

ASMJIT_PATCH(0x6BD886, CSF_LoadExtraFiles, 5)
{
	std::string _ares = "ares";

	CSFLoader::LoadAdditionalCSF((_ares + ".csf").c_str(), true);

	std::string res = "us";
	if (const auto language = StringTable::GetLanguage(StringTable::Language()))
		res = language->Letter;

	_ares += "_";
	_ares += res;
	_ares += ".csf";

	CSFLoader::LoadAdditionalCSF(_ares.c_str());

	for (int idx = 0; idx < 100; ++idx) {
		CSFLoader::LoadAdditionalCSF(std::format("stringtable{:02}.csf", idx).c_str());
	}

	R->AL(1);
	return 0x6BD88B;
}

const wchar_t* __fastcall FetchStringManager(char* label, char* speech, char* file, int line) {

	if (speech) {
		*speech = 0;
	}

	if (!StringTable::Labels()) {
		return L"***FATAL*** String Manager failed to initilaized properly";
	}

	if (strncmp(label, "NOSTR:", 6) == 0) {
		return CSFLoader::GetDynamicString(label, L"%hs", &label[6] , true);
	}

	CSFLabel* pLabel = static_cast<CSFLabel*>(bsearch(
		label,
		StringTable::Labels(),
		(size_t)StringTable::LabelCount(),
		sizeof(CSFLabel),
		(int(__cdecl*)(const void*, const void*))_strcmpi));

	if (pLabel)
	{
		if (speech)
		{
			speech = StringTable::ExtraValues[pLabel->FirstValueIndex];
		}
		return StringTable::Values[pLabel->FirstValueIndex];
	}

	return CSFLoader::GetDynamicString(label, L"MISSING:'%hs'", label, false);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x734E60, FetchStringManager);

DEFINE_FUNCTION_JUMP(CALL, 0x41099B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x410B52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x430D1D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x430D5A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46D3E6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E7C6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E893, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E8C2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46EED6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46F1D2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46F213, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470127, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470403, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4705AA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470646, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470D16, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470DFB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E3F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E56, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E84, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E9B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470EC6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470EDD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470F03, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470FCC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x479194, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CB9A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CD05, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CD36, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x49DC9D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4A2499, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4AE5F3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4B8C74, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C40F2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C6E7C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7981, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7A7B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7A92, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7AB4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7B31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7F8E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E231D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38B6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38D2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38ED, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3909, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3925, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3941, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E395D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3979, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3995, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39CD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39E9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3A4F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3B4D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3C71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42B6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42ED, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4309, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4325, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4341, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E435D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4379, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4395, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43EC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4407, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4422, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E443D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4458, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4473, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E448E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E44A9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E45FE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E47C7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4F41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E511B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E52B7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A26, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A5D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A79, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A95, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AEC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5B07, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5B22, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5BFB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5D07, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F1280, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F1910, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F195C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F19EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F8D4F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F9DB2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FA12B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FC289, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FC367, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCB51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCB8E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCD3A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCD77, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x500CD7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x50A666, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x51F2DE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52931F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52952B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C53A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C551, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C568, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDCB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDE3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDFB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52E5FD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52EC91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52F09E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52F0D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB7B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB9A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBB9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBD8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBF7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531424, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53149E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531507, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53154D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53159A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531A75, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531FEC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535C55, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535C81, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535CA5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D61, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D81, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E21, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535EC1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535EE1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536015, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536041, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536065, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536105, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536131, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536155, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5361F5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536221, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536245, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5362E5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536311, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536335, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5365C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5365E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536601, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5366C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5366E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536701, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536791, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5367B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5367D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536831, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536851, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536871, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5368F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536911, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536931, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536991, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5369B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5369D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536ED1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536EF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F11, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536FD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536FF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537011, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537051, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537071, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537091, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5370E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537101, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537121, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537161, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537181, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5371A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5371F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537211, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537231, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537281, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5372A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5372C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537301, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537321, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537341, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5373F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537411, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537431, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5374E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537501, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537521, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5375D1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5375F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537611, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5376C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5376E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537701, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537781, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5377A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5377C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537841, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537861, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537881, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537901, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537921, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537941, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5379C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5379E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537A01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537B71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537B91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537BB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E21, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EA1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EC1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EE1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F61, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F81, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53A067, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53AB31, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53AE11, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53B421, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x552EC3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x552EEF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5536CE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5536EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553711, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553733, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553755, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553777, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553799, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537B8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537D7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537F6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553815, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A1E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A3E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A5E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A7E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A9E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553ABE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553ADB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553AF8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553B15, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553B32, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D1F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D45, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D67, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D89, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DAB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DCD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DEC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E0B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E2A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E49, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x554038, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x558E83, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x558E9A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5590A7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55910E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559125, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55923B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559252, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559372, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559389, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5593A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559402, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559419, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559462, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55956E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559585, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55975C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559D83, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559E55, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A061, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A081, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A0A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A0C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E501, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E537, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E822, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E917, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E973, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EA26, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EAF5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EB19, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EC21, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55ECC1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55ED14, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EF20, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55F9D0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55FFBB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560611, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560828, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x56083F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560963, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x56097A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560A64, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560A7B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560ADC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560AF3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560B3A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560B51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5613A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5956F8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x595724, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596631, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596813, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596EB7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596EF7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596F65, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596FA5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597016, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597056, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5970C4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x59710F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597183, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5971CB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5973FD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597AF8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597F91, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FB1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FD1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x59A64B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADDAE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADDC5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADED9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADEF0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4F7F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FA0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FC1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FE2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5003, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5024, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5045, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5066, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B60D3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B60EA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B61A7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B61BE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6244, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B625B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B628D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B62A4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6306, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B631D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B646F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6486, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6634, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B664B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6F8B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B7211, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8429, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B844A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B846B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B848C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84AD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84CE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8510, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8D52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8D84, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA124, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA13B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA205, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA21C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA2B6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA2CD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA30E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA325, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA396, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA3AD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA55F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA576, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BAB5B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BAF58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB023, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB119, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB130, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C227D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C2474, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C3810, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C3E4C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C4261, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C429F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C60EE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C6102, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9482, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9836, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9BAB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9BCF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9C10, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9D18, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA732, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA756, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA77A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA7A3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CAE22, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CAEC2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CFAA2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5D7752, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5D77F2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DA7C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DA7EA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB2CC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB2EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB312, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBA66, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBAA5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBAC3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DC54E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCAB9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCBAA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCF4C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCFC0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD048, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD164, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD1E0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DDA9E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DDAB5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DE031, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DE5F0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF436, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF82C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF87B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0297, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E02DD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E049F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0B3E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0C35, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E1E06, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E2381, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3A5A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3A93, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3DC2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E55CF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E5783, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E5A9B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E72EE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7306, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E731E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7CF2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7D09, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87BE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87D9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87F4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E880F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E882A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E8845, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E8860, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E99D9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E99F8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A17, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A36, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A55, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A74, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A8B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB357, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB393, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB4BF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB4F8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB529, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB572, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB5A3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB5D4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ECAF3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ECD17, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EE2B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EE329, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF77C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF793, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF895, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF8AC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFB6A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFB82, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFD6D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFD84, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFE41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFE58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F01E0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F020D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0342, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F03DB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F03F3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F063E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0655, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0822, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0839, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F090E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0925, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F09FA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A11, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A5F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A76, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B2A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B41, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B7C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B93, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0BCE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0BE5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C20, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C37, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C72, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C89, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0CDA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0CF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0EF2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0F09, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0FDE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0FF5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F10CA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F10E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F112F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1146, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F11D5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F11EC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1227, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F123E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1279, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1290, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F12C8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F12DF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1FE7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2252, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2269, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2280, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23AF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23C6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23DD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24B5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24CC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24E3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25A7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25BE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25D5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26AD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26C4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26DB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F278E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F27A5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F27BC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2844, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F285B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2872, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F28B3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F28CA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F299F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F29C3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2D26, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2D59, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2E66, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2FB5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2FCC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F3081, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F3098, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FBB8A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC034, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC08B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC0ED, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x604082, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x610317, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6108DA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x611E48, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x622E38, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x630F6D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6311FD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x631A26, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x637491, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6374E5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6382BB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x638506, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6390F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6391D2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A026, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A076, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A0F1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A18A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A1EB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A21C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A24C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A2D6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A32A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A3BB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A40F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A519, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A56E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A78B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A82A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A889, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63AFF7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64047F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479A4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479BB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479F3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A0A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A42, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A59, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A93, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647AAA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6480C6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6480DD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648115, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64812C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648164, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64817B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6481B5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6481CC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648D70, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648E8D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648EC5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F18, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F51, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F8A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648FE2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x649025, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64904B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64909F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6490C0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6490E8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64AB42, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64AB8E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64ADBE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CA6B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CCEB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CD02, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x652303, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x65231A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x654069, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6540D9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x65F629, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6835E8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x683A8C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684736, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684A39, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684A50, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x685830, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x685F7B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860AD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860C4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860DB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686110, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686E85, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686F47, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686FA1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x688263, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x689619, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68D652, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68D8BC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DB89, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DE0A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DF0C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E08D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E3C4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E8A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6942C2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x694CA5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F37, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F4E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F82, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F99, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696252, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696339, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6963CC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696454, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6968EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696906, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696A4E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6977DC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6982B4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x699F36, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x69AA4C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x69AD02, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A936E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A939F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A9840, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A9EDD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA3E6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA417, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA47E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA4E1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA545, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC242, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC271, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC2AE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC333, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC36, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC6E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC89, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD085, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD0B7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD104, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD136, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD252, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD284, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD302, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE617, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE646, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE675, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE6A3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE7BB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE7E7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE813, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE83F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BC2DF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BC307, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD6E7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD6FE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD8F2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD91A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDAC9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDAE0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDC61, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDC78, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDCD5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDCEC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BE189, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BE1A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6C925B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6C9270, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC2EA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC301, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC318, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC336, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC34D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6D4AE6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6DE115, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6DE692, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6E0DCD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6E164B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AADF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AC09, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AC3F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x724B01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731B0D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731B9B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D2D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D7A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731DEE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731E65, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731E90, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7324C9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7324F0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73250E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732B68, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732B8F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732BAD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733484, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7334A3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7334C2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733556, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733655, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733671, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73368D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7337C8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7337E7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733806, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7338A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733945, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733961, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73397D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733B7B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746BE0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C01, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C1E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C50, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x766AD4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767383, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7673DC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767583, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7675DC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767666, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7676BF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76777F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7677DA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76BF4D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76BF77, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76C5BD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76C61D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76CDDA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76CFF3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76D20F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DB8C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DC3C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DC71, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DCA3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DCD5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DE34, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DE54, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E25C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E2BC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E575, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E5DE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76ECA9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76ED62, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76FA4C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778F6E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778FAC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778FEA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779028, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779066, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7790A4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7790E2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779120, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77915E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77919C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7791DA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779218, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779262, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779294, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7792B4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7792E6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779306, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779338, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779358, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77938A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793AA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793DC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793FC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77942E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77944E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779480, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794D2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794F2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779524, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779544, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779576, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779596, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7795C8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7795E8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77961A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77A66A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77A933, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AAD4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AC83, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AE9B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AF4B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AFB4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AFE3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77D36A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77D3BA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77DD43, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77DEB0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77F136, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77F1FA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77FA11, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780546, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780561, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78057C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780597, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805B2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805CD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805E8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780603, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780A43, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780B42, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780C59, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780DC2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780F34, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780F65, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78106A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781113, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781983, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7819BD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781A4A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781B63, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x783EC5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7841F4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784493, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7844CD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7845FB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78468D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78470E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78477D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784829, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7849ED, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784A75, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784C4A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784D58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784D93, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784DCE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784E58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784EF1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784F20, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x785722, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78587D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x785F9A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78655A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78668C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x786741, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x786B0C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787079, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787B09, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787C07, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787CC9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788758, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7888C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788A92, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788BA0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788F04, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788F44, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788FA3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788FE4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78934C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7893C0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789400, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789424, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78944F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78947A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7894A5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7899D8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789B2B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789E14, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A0E4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A15D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A1CC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A48D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A5D4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A6CE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A8BF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A90E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A932, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78AAE0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78ABBD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78AF43, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B005, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B186, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B1FA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B21E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B371, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B3BA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B419, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B45C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B4A9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B504, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B584, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78BEED, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D5F8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D664, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D744, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78DAF8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E32E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E367, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E3AB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E3EF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E433, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E477, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E4BB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E4FF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E543, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E587, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E5CB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E60C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7A2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7C3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7E4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E805, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E826, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E847, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E868, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E889, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8AA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8CB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8EC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E90D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F41E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F435, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F46C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F4A1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F4B8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F503, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F51A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F546, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F573, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F58A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F5E4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F5FB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F62E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F655, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F674, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F68C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790024, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7901E7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790368, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7903DC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790400, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790553, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79066D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7906A9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7906F8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7907FA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7908B1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7908E6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790A9E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790AD7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790B10, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79178C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7917B9, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791ADB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791E22, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791F34, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7920B5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x792129, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79214D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7922A0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x792798, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7927C5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7935C3, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x793610, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x793658, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79369E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7936D5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x794D44, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x794E46, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7975F2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797776, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79788F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7978B6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7979AD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797A17, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797B82, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x799948, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x799D52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B18F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B212, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B3AF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B5B4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B5F0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B6B7, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B6F4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B744, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B76B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79BAF8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79BC54, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79C133, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79C2E8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79F1FD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79F916, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FCA2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FD58, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FE29, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FF78, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A018F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A0274, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A02AB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A0367, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A09DA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A170D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1755, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A179D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A17E4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A181A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1893, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1900, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A19C1, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A19FC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1B52, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1B85, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1CE8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1D45, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1D77, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1DFB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1F6C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1FDF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2045, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2080, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A20C0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A27AE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2AD8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A328C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A421B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4BBB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4CAB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4D64, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4E28, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5260, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A52E6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A531F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A53D6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5412, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A56B0, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A574C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5B70, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5BB4, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5BFA, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5C2C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5C6C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5CA5, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5CDE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5D0F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5EB6, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A937A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A9821, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A985A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A987F, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A989D, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA3AE, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA4CB, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA513, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA56E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA59E, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA5C2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA749, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA809, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB38C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB3EC, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB41C, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB78A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB7C8, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB89B, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD4E2, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD50A, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD5AF, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD5CD, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AF895, FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AFBC1, FetchStringManager);