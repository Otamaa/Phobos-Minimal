#include "EVAVoices.h"

#include <Utilities/Debug.h>

#include <Phobos.CRT.h>

#include <Helpers\Macro.h>
#include <CCINIClass.h>

#include <Ext/Side/Body.h>

std::vector<const char*> EVAVoices::Types;

int EVAVoices::FindIndexById(const char* type)
{
	Debug::Log("[Phobos] Find EVAVoices Index by ID [%s]\n", type);
	// the default values
	if (!CRT::strcmpi(type, GameStrings::Allied()))
	{
		return 0;
	}
	else if (!CRT::strcmpi(type, GameStrings::Russian()))
	{
		return 1;
	}
	else if (!CRT::strcmpi(type, GameStrings::Yuri()))
	{
		return 2;
	}

	// find all others
	for (size_t i = 0; i < Types.size(); ++i)
	{
		if (!CRT::strcmpi(type, Types[i]))
		{
			return static_cast<int>(i + 3);
		}
	}

	// not found
	return -1;
}

// adds the EVA type only if it doesn't exist
void EVAVoices::RegisterType(const char* type)
{
	int index = EVAVoices::FindIndexById(type);

	if (index < 0)
	{
		const char* copy = CRT::strdup(type);
		Types.push_back(copy);
	}
}

#ifndef disable_eva_hooks
// replace the complete ini loading function
DEFINE_OVERRIDE_HOOK(0x753000, VoxClass_CreateFromINIList, 6)
{
	GET(CCINIClass* const, pINI, ECX);

	char buffer[200];

	// read all EVA types. these are used as additional keys for
	// when the dialogs are read.
	EVAVoices::Types.clear();

	auto const pSection = "EVATypes";

	if(pINI->GetSection(pSection)) {
		auto const count = pINI->GetKeyCount(pSection);

		for(auto i = 0; i < count; ++i) {
			auto const pKey = pINI->GetKeyName(pSection, i);
			if(pINI->ReadString(pSection, pKey, "", buffer)) {
				EVAVoices::RegisterType(buffer);
			}
		}
	}

	// read all dialogs
	auto const pSection2 = GameStrings::DialogList();

	if(pINI->GetSection(pSection2)) {
		auto const count = pINI->GetKeyCount(pSection2);

		for(auto i = 0; i < count; ++i) {
			auto const pKey = pINI->GetKeyName(pSection2, i);
			if(pINI->ReadString(pSection2, pKey, "", buffer)) {

				// find or allocate done manually
				VoxClass* pVox = VoxClass::Find(buffer);
				if(!pVox) {
					pVox = GameCreate<VoxClass2>(buffer);
				}

				pVox->LoadFromINI(pINI);
			}
		}
	}

	return 0x75319F;
}

// need to destroy manually because of non-virtual destructor
DEFINE_OVERRIDE_HOOK(0x7531CF, VoxClass_DeleteAll, 5)
{
	auto& Array = *VoxClass::Array;

	while(Array.Count) {
		// destroy backwards instead of forwards
		auto const pVox = static_cast<VoxClass2*>(Array[Array.Count - 1]);
		GameDelete<true, false>(pVox);
	}

	return 0x753240;
}

// also load all additional filenames
DEFINE_OVERRIDE_HOOK(0x752FDC, VoxClass_LoadFromINI, 5)
{
	GET(VoxClass2* const, pThis, ESI);
	GET(CCINIClass* const, pINI, EBP);

	char buffer[0x20];

	// make way for all filenames
	auto const count = EVAVoices::Types.size();
	pThis->Voices.resize(count);

	// put the filename in there. 8 chars max.
	for(auto i = 0u; i < count; ++i) {
		pINI->ReadString(pThis->Name, EVAVoices::Types[i], "", buffer);
		PhobosCRT::strCopy(pThis->Voices[i].Name, buffer);
	}

	return 0;
}

// undo the inlining. call the function we hook
DEFINE_OVERRIDE_HOOK(0x7528E8, VoxClass_PlayEVASideSpecific, 5)
{
	GET(VoxClass* const, pVox, EBP);
	auto const ret = pVox->GetFilename();

	R->EDI(ret);
	return 0x752901;
}

// resolve EVA index to filename
DEFINE_OVERRIDE_HOOK(0x753380, VoxClass_GetFilename, 5)
{
	GET(VoxClass2* const, pThis, ECX);
	auto const index = VoxClass::EVAIndex();

	const char* ret = "";
	switch(index)
	{
	case -1:
		break;
	case 0:
		ret = pThis->Allied;
		break;
	case 1:
		ret = pThis->Russian;
		break;
	case 2:
		ret = pThis->Yuri;
		break;
	default:
		auto const idxVoc = static_cast<size_t>(index - 3);
		if(idxVoc < pThis->Voices.size()) {
			ret = pThis->Voices[idxVoc].Name;
		}
	}

	R->EAX(ret);
	return 0x753398;
}

DEFINE_OVERRIDE_HOOK(0x7534e0 , VoxClass_SetEVAIndex , 5)
{	GET(int ,side , ECX);

	if (auto pSide = SideClass::Array->GetItemOrDefault(side)) {
		int idx = SideExt::ExtMap.Find(pSide)->EVAIndex;
		VoxClass::EVAIndex = idx;
	} else {
		VoxClass::EVAIndex = -1;
	}

	return 0x7534F3;
}
#endif