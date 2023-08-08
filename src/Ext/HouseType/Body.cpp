#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.h>

void HouseTypeExt::ExtData::InheritSettings(HouseTypeClass* pThis)
{
	if (auto ParentCountry = pThis->FindParentCountry()) {
		if (const auto ParentData = HouseTypeExt::ExtMap.Find(ParentCountry)) {
			this->SurvivorDivisor = ParentData->SurvivorDivisor;
			this->Crew = ParentData->Crew;
			this->Engineer = ParentData->Engineer;
			this->Technician = ParentData->Technician;
			this->ParaDropPlane = ParentData->ParaDropPlane;
			this->SpyPlane = ParentData->SpyPlane;
			this->HunterSeeker = ParentData->HunterSeeker;
			this->ParaDropTypes = ParentData->ParaDropTypes;
			this->ParaDropNum = ParentData->ParaDropNum;
			this->GivesBounty = ParentData->GivesBounty;
			this->CanBeDriven = ParentData->CanBeDriven;
			this->ParachuteAnim = ParentData->ParachuteAnim;
			this->StartInMultiplayer_WithConst = ParentData->StartInMultiplayer_WithConst;
		}
	}

	this->SettingsInherited = true;
}

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!this->SettingsInherited && *pThis->ParentCountry && IS_SAME_STR_(pThis->ParentCountry, pThis->ID))
	{
		this->InheritSettings(pThis);
	}

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->SurvivorDivisor.Read(exINI, pSection, "SurvivorDivisor");
	this->Crew.Read(exINI, pSection, "Crew", true);
	this->Engineer.Read(exINI, pSection, "Engineer", true);
	this->Technician.Read(exINI, pSection, "Technician", true);
	this->ParaDropPlane.Read(exINI, pSection, "ParaDrop.Aircraft" , true);
	this->HunterSeeker.Read(exINI, pSection, "HunterSeeker", true);
	this->SpyPlane.Read(exINI, pSection, "SpyPlane.Aircraft", true);
	this->ParaDropTypes.Read(exINI, pSection, "ParaDrop.Types", true);

	// remove all types that cannot paradrop
	Helpers::Alex::remove_non_paradroppables(this->ParaDropTypes, pSection, "ParaDrop.Types");

	this->ParaDropNum.Read(exINI, pSection, "ParaDrop.Num");
	this->GivesBounty.Read(exINI, pSection, "GivesBounty");
	this->CanBeDriven.Read(exINI, pSection, "CanBeDriven");

	// Disabled atm
	this->NewTeamsSelector_MergeUnclassifiedCategoryWith.Read(exINI, pSection, "NewTeamsSelector.MergeUnclassifiedCategoryWith");
	this->NewTeamsSelector_UnclassifiedCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.UnclassifiedCategoryPercentage");
	this->NewTeamsSelector_GroundCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.GroundCategoryPercentage");
	this->NewTeamsSelector_AirCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.AirCategoryPercentage");
	this->NewTeamsSelector_NavalCategoryPercentage.Read(exINI, pSection, "NewTeamsSelector.NavalCategoryPercentage");
	//

	this->ParachuteAnim.Read(exINI, pSection, "Parachute.Anim" , true);
	this->StartInMultiplayer_WithConst.Read(exINI, pSection, "StartInMultiplayer.WithConst");
}

template <typename T>
void  HouseTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->SettingsInherited)

		.Process(this->SurvivorDivisor)
		.Process(this->Crew)
		.Process(this->Engineer)
		.Process(this->Technician)
		.Process(this->ParaDropPlane)
		.Process(this->SpyPlane)
		.Process(this->HunterSeeker)
		.Process(this->ParaDropTypes)
		.Process(this->ParaDropNum)
		.Process(this->GivesBounty)
		.Process(this->CanBeDriven)
		.Process(this->NewTeamsSelector_MergeUnclassifiedCategoryWith)
		.Process(this->NewTeamsSelector_UnclassifiedCategoryPercentage)
		.Process(this->NewTeamsSelector_GroundCategoryPercentage)
		.Process(this->NewTeamsSelector_AirCategoryPercentage)
		.Process(this->NewTeamsSelector_NavalCategoryPercentage)
		.Process(this->ParachuteAnim)
		.Process(this->StartInMultiplayer_WithConst)
		;
}

// =============================
// container

HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;
HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") { }
HouseTypeExt::ExtContainer::~ExtContainer() = default;

bool HouseTypeExt::ExtContainer::Load(HouseTypeClass* pThis, IStream* pStm)
{
	HouseTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);
	return pData != nullptr;
};


// =============================
// container hooks

DEFINE_HOOK(0x511635, HouseTypeClass_CTOR_1, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x511643, HouseTypeClass_CTOR_2, 0x5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5127CF, HouseTypeClass_DTOR, 0x6)
{
	GET(HouseTypeClass*, pItem, ESI);

	HouseTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x512480, HouseTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x512290, HouseTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x51246D, HouseTypeClass_Load_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x51255C, HouseTypeClass_Save_Suffix, 0x5)
{
	HouseTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x51215A, HouseTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x51214F, HouseTypeClass_LoadFromINI, 0x5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x51215A);

	return 0;
}