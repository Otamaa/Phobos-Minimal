#include "CSF.h"

#include <CCFileClass.h>
#include <Phobos.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <Helpers/Macro.h>

#include <Phobos.Lua.h>

#pragma region defines
int CSFLoader::CSFCount {};
int CSFLoader::NextValueIndex {};
std::unordered_map<std::string, CSFLoader::CSFStringStorage> CSFLoader::DynamicStrings {};
#pragma endregion

void CSFLoader::LoadAdditionalCSF(const char* pFileName, bool ignoreLanguage)
{
	bool _loaded = false;
	//The main stringtable must have been loaded (memory allocation)
	//To do that, use StringTable::LoadFile.
	if (StringTable::IsLoaded && std::strlen(pFileName) > 0 && *pFileName)
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
					const bool succeeded = StringTable::ReadFile(pFileName); //must be modified to do the rest ;)

					if(succeeded){
						std::sort(StringTable::Labels(), StringTable::Labels() + StringTable::LabelCount(),
							[](const CSFLabel& lhs, const CSFLabel& rhs) {
							return IMPL_STRCMPI(lhs.Name, rhs.Name) < 0;
						});
					}

					_loaded =  succeeded;
				}
			}
		}
	}

	if (_loaded)
		Debug::LogInfo("Successfully load {} !", pFileName);
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
			ptr = YRMemory::AllocateChecked(size);

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

static COMPILETIMEEVAL constant_ptr<const char, 0x840D40> const ra2md_str {};

ASMJIT_PATCH(0x6BD84E, CSF_LoadExtraFiles, 5)
{
	if (!StringTable::LoadFile(ra2md_str())) {
		const std::string _msg = fmt::format("Unable to initialize '{0}', please reinstall {1}.\n"
			"Keine Initialisierung von '{0}' möglich. Bitte installieren Sie {1} erneut.\n"
			"Initialisation de '{0}' impossible. Veuillez réinstaller {1}."
			, ra2md_str() , LuaData::MainWindowStr);

		Imports::MessageBoxA.invoke()(NULL,
			_msg.c_str(),
			LuaData::MainWindowStr.c_str(), 0x10u);

		return 0x6BD86F;
	}

	static fmt::basic_memory_buffer<char , 60> buffer {};
	CSFLoader::LoadAdditionalCSF("ares.csf", true);
	buffer.clear();
	std::string res = "us";
	if (const auto language = StringTable::GetLanguage(StringTable::Language()))
		res = language->Letter;

	fmt::format_to(std::back_inserter(buffer), "ares_{}.csf", res);
	buffer.push_back('\0');
	CSFLoader::LoadAdditionalCSF(buffer.data());

	buffer.clear();

	for (int idx = 0; idx < 100; ++idx) {
		fmt::format_to(std::back_inserter(buffer), "stringtable{:02}.csf", idx);
		buffer.push_back('\0');
		CSFLoader::LoadAdditionalCSF(buffer.data());
		buffer.clear();
	}

	R->AL(1);
	return 0x6BD88B;
}

const wchar_t* __fastcall CSFLoader::FetchStringManager(const char* label, char* speech, const char* file, int line) {

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

DEFINE_FUNCTION_JUMP(LJMP, 0x734E60, CSFLoader::FetchStringManager);

DEFINE_FUNCTION_JUMP(CALL, 0x41099B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x410B52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x430D1D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x430D5A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46D3E6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E7C6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E893, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46E8C2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46EED6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46F1D2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x46F213, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470127, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470403, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4705AA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470646, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470D16, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470DFB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E3F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E56, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E84, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470E9B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470EC6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470EDD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470F03, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x470FCC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x479194, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CB9A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CD05, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x48CD36, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x49DC9D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4A2499, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4AE5F3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4B8C74, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C40F2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C6E7C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7981, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7A7B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7A92, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7AB4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7B31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4C7F8E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E231D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38B6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38D2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E38ED, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3909, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3925, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3941, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E395D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3979, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3995, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39CD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E39E9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3A4F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3B4D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E3C71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42B6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E42ED, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4309, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4325, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4341, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E435D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4379, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4395, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E43EC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4407, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4422, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E443D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4458, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4473, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E448E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E44A9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E45FE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E47C7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E4F41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E511B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E52B7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A26, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A5D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A79, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5A95, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5AEC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5B07, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5B22, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5BFB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4E5D07, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F1280, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F1910, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F195C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F19EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F8D4F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4F9DB2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FA12B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FC289, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FC367, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCB51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCB8E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCD3A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x4FCD77, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x500CD7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x50A666, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x51F2DE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52931F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52952B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C53A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C551, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52C568, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDCB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDE3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52DDFB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52E5FD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52EC91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52F09E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52F0D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB7B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FB9A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBB9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBD8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x52FBF7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531424, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53149E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531507, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53154D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53159A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531A75, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x531FEC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535C55, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535C81, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535CA5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D61, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535D81, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E21, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535E41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535EC1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535EE1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x535F91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536015, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536041, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536065, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536105, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536131, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536155, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5361F5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536221, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536245, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5362E5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536311, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536335, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5365C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5365E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536601, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5366C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5366E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536701, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536791, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5367B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5367D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536831, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536851, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536871, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5368F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536911, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536931, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536991, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5369B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5369D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536A71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536B71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536BF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536C71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536CF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536D71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536DF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536E91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536ED1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536EF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F11, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536F91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536FD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x536FF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537011, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537051, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537071, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537091, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5370E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537101, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537121, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537161, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537181, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5371A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5371F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537211, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537231, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537281, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5372A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5372C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537301, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537321, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537341, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5373F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537411, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537431, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5374E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537501, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537521, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5375D1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5375F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537611, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5376C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5376E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537701, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537781, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5377A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5377C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537841, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537861, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537881, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537901, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537921, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537941, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5379C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5379E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537A01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537B71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537B91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537BB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E21, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537E41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EA1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EC1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537EE1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F61, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x537F81, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53A067, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53AB31, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53AE11, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x53B421, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x552EC3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x552EEF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5536CE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5536EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553711, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553733, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553755, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553777, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553799, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537B8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537D7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5537F6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553815, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A1E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A3E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A5E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A7E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553A9E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553ABE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553ADB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553AF8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553B15, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553B32, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D1F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D45, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D67, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553D89, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DAB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DCD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553DEC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E0B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E2A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x553E49, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x554038, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x558E83, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x558E9A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5590A7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55910E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559125, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55923B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559252, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559372, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559389, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5593A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559402, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559419, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559462, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55956E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559585, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55975C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559D83, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x559E55, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A061, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A081, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A0A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55A0C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E501, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E537, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E822, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E917, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55E973, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EA26, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EAF5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EB19, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EC21, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55ECC1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55ED14, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55EF20, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55F9D0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x55FFBB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560611, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560828, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x56083F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560963, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x56097A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560A64, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560A7B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560ADC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560AF3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560B3A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x560B51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5613A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5956F8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x595724, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596631, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596813, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596EB7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596EF7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596F65, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x596FA5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597016, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597056, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5970C4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x59710F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597183, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5971CB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5973FD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597AF8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597F91, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FB1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FD1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x597FF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x59A64B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADDAE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADDC5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADED9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ADEF0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4F7F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FA0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FC1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B4FE2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5003, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5024, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5045, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B5066, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B60D3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B60EA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B61A7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B61BE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6244, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B625B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B628D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B62A4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6306, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B631D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B646F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6486, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6634, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B664B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B6F8B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B7211, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8429, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B844A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B846B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B848C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84AD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84CE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B84EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8510, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8D52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5B8D84, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA124, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA13B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA205, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA21C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA2B6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA2CD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA30E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA325, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA396, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA3AD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA55F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BA576, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BAB5B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BAF58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB023, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB119, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5BB130, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C227D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C2474, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C3810, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C3E4C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C4261, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C429F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C60EE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C6102, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9482, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9836, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9BAB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9BCF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9C10, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5C9D18, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA732, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA756, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA77A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CA7A3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CAE22, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CAEC2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5CFAA2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5D7752, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5D77F2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DA7C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DA7EA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB2CC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB2EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DB312, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBA66, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBAA5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DBAC3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DC54E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCAB9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCBAA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCF4C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DCFC0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD048, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD164, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DD1E0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DDA9E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DDAB5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DE031, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DE5F0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF436, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF82C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5DF87B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0297, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E02DD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E049F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0B3E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E0C35, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E1E06, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E2381, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3A5A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3A93, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E3DC2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E55CF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E5783, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E5A9B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E72EE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7306, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E731E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7CF2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E7D09, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87BE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87D9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E87F4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E880F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E882A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E8845, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E8860, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E99D9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E99F8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A17, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A36, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A55, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A74, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5E9A8B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB357, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB393, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB4BF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB4F8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB529, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB572, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB5A3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EB5D4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ECAF3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5ECD17, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EE2B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EE329, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF77C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF793, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF895, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EF8AC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFB6A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFB82, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFD6D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFD84, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFE41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5EFE58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F01E0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F020D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0342, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F03DB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F03F3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F063E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0655, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0822, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0839, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F090E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0925, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F09FA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A11, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A5F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0A76, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B2A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B41, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B7C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0B93, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0BCE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0BE5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C20, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C37, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C72, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0C89, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0CDA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0CF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0EF2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0F09, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0FDE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F0FF5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F10CA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F10E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F112F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1146, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F11D5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F11EC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1227, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F123E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1279, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1290, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F12C8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F12DF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F1FE7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2252, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2269, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2280, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23AF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23C6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F23DD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24B5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24CC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F24E3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25A7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25BE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F25D5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26AD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26C4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F26DB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F278E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F27A5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F27BC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2844, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F285B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2872, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F28B3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F28CA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F299F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F29C3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2D26, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2D59, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2E66, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2FB5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F2FCC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F3081, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5F3098, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FBB8A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC034, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC08B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC0ED, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x604082, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x610317, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6108DA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x611E48, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x622E38, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x630F6D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6311FD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x631A26, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x637491, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6374E5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6382BB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x638506, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6390F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6391D2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A026, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A076, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A0F1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A18A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A1EB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A21C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A24C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A2D6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A32A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A3BB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A40F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A519, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A56E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A78B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A82A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63A889, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x63AFF7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64047F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479A4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479BB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6479F3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A0A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A42, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A59, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647A93, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x647AAA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6480C6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6480DD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648115, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64812C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648164, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64817B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6481B5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6481CC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648D70, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648E8D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648EC5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F18, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F51, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648F8A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x648FE2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x649025, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64904B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64909F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6490C0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6490E8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64AB42, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64AB8E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64ADBE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CA6B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CCEB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x64CD02, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x652303, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x65231A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x654069, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6540D9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x65F629, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6835E8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x683A8C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684736, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684A39, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x684A50, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x685830, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x685F7B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860AD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860C4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6860DB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686110, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686E85, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686F47, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x686FA1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x688263, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x689619, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68D652, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68D8BC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DB89, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DE0A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68DF0C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E08D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E3C4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x68E8A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6942C2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x694CA5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F37, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F4E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F82, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x695F99, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696252, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696339, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6963CC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696454, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6968EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696906, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x696A4E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6977DC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6982B4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x699F36, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x69AA4C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x69AD02, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A936E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A939F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A9840, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6A9EDD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA3E6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA417, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA47E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA4E1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AA545, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC242, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC271, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC2AE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC333, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC36, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC6E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6ACC89, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD085, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD0B7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD104, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD136, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD252, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD284, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AD302, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE617, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE646, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE675, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE6A3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE7BB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE7E7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE813, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6AE83F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BC2DF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BC307, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD6E7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD6FE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD8F2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BD91A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDAC9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDAE0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDC61, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDC78, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDCD5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BDCEC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BE189, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6BE1A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6C925B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6C9270, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC2EA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC301, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC318, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC336, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6CC34D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6D4AE6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6DE115, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6DE692, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6E0DCD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x6E164B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AADF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AC09, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x70AC3F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x724B01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731B0D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731B9B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D2D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731D7A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731DEE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731E65, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x731E90, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7324C9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7324F0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73250E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732B68, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732B8F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x732BAD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733484, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7334A3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7334C2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733556, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733655, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733671, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73368D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7337C8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7337E7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733806, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7338A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733945, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733961, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x73397D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x733B7B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746BE0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C01, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C1E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x746C50, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x766AD4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767383, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7673DC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767583, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7675DC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x767666, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7676BF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76777F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7677DA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76BF4D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76BF77, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76C5BD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76C61D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76CDDA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76CFF3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76D20F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DB8C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DC3C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DC71, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DCA3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DCD5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DE34, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76DE54, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E25C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E2BC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E575, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76E5DE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76ECA9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76ED62, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x76FA4C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778F6E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778FAC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x778FEA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779028, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779066, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7790A4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7790E2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779120, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77915E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77919C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7791DA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779218, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779262, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779294, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7792B4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7792E6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779306, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779338, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779358, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77938A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793AA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793DC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7793FC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77942E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77944E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779480, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794D2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7794F2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779524, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779544, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779576, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x779596, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7795C8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7795E8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77961A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77A66A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77A933, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AAD4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AC83, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AE9B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AF4B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AFB4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77AFE3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77D36A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77D3BA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77DD43, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77DEB0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77F136, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77F1FA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x77FA11, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780546, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780561, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78057C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780597, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805B2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805CD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7805E8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780603, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780A43, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780B42, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780C59, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780DC2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780F34, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x780F65, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78106A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781113, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781983, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7819BD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781A4A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x781B63, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x783EC5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7841F4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784493, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7844CD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7845FB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78468D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78470E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78477D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784829, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7849ED, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784A75, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784C4A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784D58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784D93, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784DCE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784E58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784EF1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x784F20, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x785722, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78587D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x785F9A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78655A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78668C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x786741, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x786B0C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787079, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787B09, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787C07, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x787CC9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788758, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7888C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788A92, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788BA0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788F04, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788F44, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788FA3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x788FE4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78934C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7893C0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789400, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789424, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78944F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78947A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7894A5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7899D8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789B2B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x789E14, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A0E4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A15D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A1CC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A48D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A5D4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A6CE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A8BF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A90E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78A932, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78AAE0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78ABBD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78AF43, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B005, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B186, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B1FA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B21E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B371, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B3BA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B419, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B45C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B4A9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B504, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78B584, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78BEED, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D5F8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D664, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78D744, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78DAF8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E32E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E367, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E3AB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E3EF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E433, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E477, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E4BB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E4FF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E543, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E587, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E5CB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E60C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7A2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7C3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E7E4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E805, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E826, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E847, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E868, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E889, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8AA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8CB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E8EC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78E90D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F41E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F435, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F46C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F4A1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F4B8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F503, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F51A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F546, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F573, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F58A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F5E4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F5FB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F62E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F655, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F674, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x78F68C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790024, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7901E7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790368, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7903DC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790400, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790553, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79066D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7906A9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7906F8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7907FA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7908B1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7908E6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790A9E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790AD7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x790B10, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79178C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7917B9, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791ADB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791E22, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x791F34, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7920B5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x792129, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79214D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7922A0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x792798, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7927C5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7935C3, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x793610, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x793658, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79369E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7936D5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x794D44, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x794E46, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7975F2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797776, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79788F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7978B6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7979AD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797A17, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x797B82, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x799948, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x799D52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B18F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B212, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B3AF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B5B4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B5F0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B6B7, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B6F4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B744, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79B76B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79BAF8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79BC54, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79C133, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79C2E8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79F1FD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79F916, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FCA2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FD58, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FE29, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x79FF78, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A018F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A0274, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A02AB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A0367, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A09DA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A170D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1755, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A179D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A17E4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A181A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1893, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1900, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A19C1, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A19FC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1B52, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1B85, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1CE8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1D45, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1D77, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1DFB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1F6C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A1FDF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2045, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2080, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A20C0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A27AE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A2AD8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A328C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A421B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4BBB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4CAB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4D64, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A4E28, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5260, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A52E6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A531F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A53D6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5412, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A56B0, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A574C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5B70, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5BB4, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5BFA, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5C2C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5C6C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5CA5, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5CDE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5D0F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A5EB6, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A937A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A9821, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A985A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A987F, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7A989D, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA3AE, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA4CB, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA513, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA56E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA59E, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA5C2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA749, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AA809, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB38C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB3EC, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB41C, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB78A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB7C8, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AB89B, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD4E2, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD50A, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD5AF, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AD5CD, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AF895, CSFLoader::FetchStringManager);
DEFINE_FUNCTION_JUMP(CALL, 0x7AFBC1, CSFLoader::FetchStringManager);