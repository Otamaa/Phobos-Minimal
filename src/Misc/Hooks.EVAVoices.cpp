#include <Utilities/Debug.h>

#include <Phobos.CRT.h>

#include <Utilities/Macro.h>

#include <CCINIClass.h>

#include <Ext/Side/Body.h>

#include <New/Entity/EVAVoices.h>

#include <VoxClass.h>

#include <vector>
#include <string>

struct VoxFile
{
	char Name[9];

	//need to define a == operator so it can be used in array classes
	bool operator == (const VoxFile& other) const
	{
		return !CRT::strcmp(this->Name, other.Name);
	}
};

class VoxClass2 : public VoxClass
{
public:
	VoxClass2(char* pName) : VoxClass(pName) {}

	~VoxClass2() = default;

	std::vector<VoxFile> Voices;
};

// replace the complete ini loading function
ASMJIT_PATCH(0x753000, VoxClass_CreateFromINIList, 6)
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
			if(pINI->ReadString(pSection, pKey, Phobos::readDefval, buffer) > 0) {
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
			if(pINI->ReadString(pSection2, pKey, Phobos::readDefval, buffer) > 0) {

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
ASMJIT_PATCH(0x7531CF, VoxClass_DeleteAll, 5)
{
	for(int i = VoxClass::Array->Count - 1; i >= 0; --i) {
		if (auto pItem = static_cast<VoxClass2*>(VoxClass::Array->Items[i])) {
			// destroy backwards instead of forwards
			GameDelete<true, false>(std::exchange(pItem, nullptr));
		}
	}

	return 0x753240;
}

// also load all additional filenames
ASMJIT_PATCH(0x752FDC, VoxClass_LoadFromINI, 5)
{
	GET(VoxClass2* const, pThis, ESI);
	GET(CCINIClass* const, pINI, EBP);

	char buffer[0xA];

	// make way for all filenames
	auto const count = EVAVoices::Types.size();
	pThis->Voices.resize(count);

	// put the filename in there. 8 chars max.
	for(auto i = 0u; i < count; ++i) {
		if(pINI->ReadString(pThis->Name, EVAVoices::Types[i].c_str(), Phobos::readDefval, buffer) > 0)
			PhobosCRT::strCopy(pThis->Voices[i].Name, buffer);
	}

	return 0;
}

// undo the inlining. call the function we hook
ASMJIT_PATCH(0x7528E8, VoxClass_PlayEVASideSpecific, 5)
{
	GET(VoxClass* const, pVox, EBP);
	R->EDI(pVox->GetFilename());
	return 0x752901;
}

// resolve EVA index to filename
ASMJIT_PATCH(0x753380, VoxClass_GetFilename, 5)
{
	GET(VoxClass2* const, pThis, ECX);
	auto const index = VoxClass::EVAIndex();

	const char* ret = Phobos::readDefval;
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
	break;
	}

	R->EAX(ret);
	return 0x753398;
}

void __fastcall VoxClass_SetEvaIndex(int house) {

}

 DEFINE_FUNCTION_JUMP(CALL,0x534FAC, VoxClass_SetEvaIndex);
 DEFINE_FUNCTION_JUMP(LJMP,0x7534E0, VoxClass_SetEvaIndex);

#include <Ext/HouseType/Body.h>
#include <Ext/Scenario/Body.h>
#include <CampaignClass.h>

ASMJIT_PATCH(0x6877EE, ScenarioClass_LoadINI_EVAIndex1, 5)
{
	GET(int, HouseIndex, EAX);
	int _SideIdx = 0;

	if(HouseIndex >= 0 && HouseIndex < HouseClass::Array->Count) {
		auto pHouse = HouseClass::Array->Items[HouseIndex];
		auto pSide = SideClass::Array->operator[](pHouse->Type->SideIndex);
		auto pHouseType = HouseTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer()->Type);
	
		if(!pHouseType->EVAIndex.isset())
			ScenarioExtData::Instance()->IsHouseTypeVoiceNeedCheck = false;

		VoxClass::EVAIndex = SideExtContainer::Instance.Find(pSide)->EVAIndex;
		_SideIdx = pHouse->Type->SideIndex;
	}

	R->EBX(_SideIdx);
	return 0x687811;
}

ASMJIT_PATCH(0x6878A0, ScenarioClass_LoadINI_EVAIndex3, 5)
{
	GET(int, HouseIndex, EAX);

	int _SideIdx = 0;

	if(HouseIndex >= 0 && HouseIndex < HouseClass::Array->Count) {
		auto pHouse = HouseClass::Array->Items[HouseIndex];
		auto pSide = SideClass::Array->operator[](pHouse->Type->SideIndex);
		auto pHouseType = HouseTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer()->Type);

		if(!pHouseType->EVAIndex.isset())
			ScenarioExtData::Instance()->IsHouseTypeVoiceNeedCheck = false;

		VoxClass::EVAIndex = SideExtContainer::Instance.Find(pSide)->EVAIndex;
		_SideIdx = pHouse->Type->SideIndex;
	}

	R->EBX(_SideIdx);

	return 0x6878C3;
}

static COMPILETIMEEVAL reference<unsigned long , 0xA8E7A8> const ScenarioCRC {};

ASMJIT_PATCH(0x68776E, ScenarioClass_LoadINI_EVAIndex2, 8)
{
	int _SideIdx = 0;
	ScenarioCRC = 0;

	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
		auto campaignIdx = ScenarioClass::Instance->CampaignIndex;
		if(campaignIdx != -1){
			_SideIdx = CampaignClass::Array->Items[campaignIdx]->idxCD;
		}
	} else {

		//yeah , you tell me
		auto pNode = NodeNameType::Array->Items[0];
		int HouseTypeSelected = 0;
		if(pNode->Country != -3){
			HouseTypeSelected = pNode->Country;
		}

		auto pHouse = HouseTypeClass::Array->Items[HouseTypeSelected];
		auto pHouseType = HouseTypeExtContainer::Instance.Find(pHouse);

		if(!pHouseType->EVAIndex.isset())
			ScenarioExtData::Instance()->IsHouseTypeVoiceNeedCheck = false;

		_SideIdx = pHouse->SideIndex;
	}

	VoxClass::EVAIndex = SideExtContainer::Instance.Find(SideClass::Array->operator[](_SideIdx))->EVAIndex;
	

	R->ECX(_SideIdx);
	return 0x6877B5;
}

// decide to check it here because one of the hook path litelarry doest support direct house/housetype access
// and changing that need more effort and code than doing this hack
DEFINE_HOOK(0x68AD0C, ScenarioClass_ReadMap_SetEVAIndex, 0x7)
{
	if (const auto pHouse = HouseClass::CurrentPlayer()) {
		if (ScenarioExtData::Instance()->IsHouseTypeVoiceNeedCheck)
			VoxClass::EVAIndex = HouseTypeExtContainer::Instance.Find(pHouse->Type)->EVAIndex.Get();
	}

	return 0;
}