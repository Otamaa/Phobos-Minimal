#include "CSF.h"

#include <CCFileClass.h>
#include <Phobos.h>

#include <Utilities/Debug.h>

int CSFLoader::CSFCount = 0;
int CSFLoader::NextValueIndex = 0;
std::unordered_map<std::string, CSFString> CSFLoader::DynamicStrings;

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

const wchar_t* CSFLoader::GetDynamicString(const char* pLabelName, const wchar_t* pPattern, const char* pDefault)
{
	if (IS_SAME_STR_(pLabelName, "NOSTR:"))
		return L"";

	auto pData = &DynamicStrings[pLabelName];

	if((!pData->Text || !pData->Text[0])) {
		swprintf_s(pData->Text, 101u, pPattern, pDefault);
		if(Phobos::Otamaa::OutputMissingStrings) {
			Debug::Log("[CSFLoader] Added label \"%s\" with value \"%ls\".\n", pLabelName, pData->Text);
		}
	}

	return pData->Text;
}

#ifndef aaa
DEFINE_HOOK(0x7349cf, StringTable_ParseFile_Buffer, 7)
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


DEFINE_HOOK(0x7346D0, CSF_LoadBaseFile, 6)
{
	StringTable::IsLoaded = true;
	CSFLoader::CSFCount = 0;
	return 0;
}

DEFINE_HOOK(0x734823, CSF_AllocateMemory, 6)
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

DEFINE_HOOK(0x734A5F, CSF_AddOrOverrideLabel, 5)
{
	if (CSFLoader::CSFCount > 0)
	{
		if (CSFLabel* pLabel = static_cast<CSFLabel*>(CRT::bsearch(
			StringTable::GlobalBuffer(), //label buffer, char[4096]
			StringTable::Labels(),
			(size_t)StringTable::LabelCount(),
			sizeof(CSFLabel),
			(int(__cdecl*)(const void*, const void*))CRT::strcmpi)))
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

DEFINE_HOOK(0x734A97, CSF_SetIndex, 6)
{
	R->EDX(StringTable::Labels());
	R->ECX(CSFLoader::CSFCount > 0 ? CSFLoader::NextValueIndex : R->Stack32(0x18));
	return 0x734AA1;
}

DEFINE_HOOK(0x6BD886, CSF_LoadExtraFiles, 5)
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

DEFINE_HOOK(0x734E83, CSF_LoadString_1, 6)
{
	GET(const char*, pName, EBX);

	if (!strncmp(pName, "NOSTR:", 6)) {
		R->EAX(CSFLoader::GetDynamicString(pName, L"%hs", &pName[6]));
		return 0x734F0F;
	}

	return 0;
}

DEFINE_HOOK(0x734EC2, CSF_LoadString_2, 7)
{
	GET(const char*, pName, EBX);
	R->EAX(CSFLoader::GetDynamicString(pName, L"MISSING:'%hs'", pName));
	return 0x734F0F;
}
#endif