#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <Utilities/EnumFunctions.h>
#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <FactoryClass.h>

DEFINE_HOOK(0x452678, BuildingClass_CanUpgrade_UpgradeBuildings, 0x6) //8
{
	enum { Continue = 0x4526A7, ForbidUpgrade = 0x4526B5 };

	GET(BuildingClass*, pBuilding, ECX);
	GET_STACK(BuildingTypeClass*, pUpgrade, 0xC);
	GET(HouseClass*, pUpgradeOwner, EAX);

	if (BuildingTypeExt::CanUpgrade(pBuilding, pUpgrade, pUpgradeOwner))
	{
		R->EAX(pBuilding->Type->PowersUpToLevel);
		return Continue;
	}

	return ForbidUpgrade;
}

DEFINE_HOOK(0x4408EB, BuildingClass_Unlimbo_UpgradeBuildings, 0x6) //A
{
	enum { Continue = 0x440912, ForbidUpgrade = 0x440926 };

	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pUpgrade, ESI);

	if (BuildingTypeExt::CanUpgrade(pBuilding, pUpgrade->Type, pUpgrade->Owner))
	{
		R->EBX(pUpgrade->Type);
		pUpgrade->SetOwningHouse(pBuilding->Owner, false);
		return Continue;
	}

	return ForbidUpgrade;
}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild_UpgradesInteraction, 0x3)
{
	GET(HouseClass* , pThis, ECX);
	GET_STACK(TechnoTypeClass* , pItem, 0x4);
	GET_STACK(bool , includeInProduction, 0xC);
	GET(CanBuildResult const, resultOfAres, EAX);

	if (const auto pBuilding = type_cast<BuildingTypeClass*,true>(pItem)) {
		if (BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings.empty())
			return 0x0;

		if (resultOfAres == CanBuildResult::Buildable)
			R->EAX(BuildingTypeExt::CheckBuildLimit(pThis, pBuilding, includeInProduction));
	}

	return 0;
}