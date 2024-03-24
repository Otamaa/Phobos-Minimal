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

	if (BuildingTypeExtData::CanUpgrade(pBuilding, pUpgrade, pUpgradeOwner))
	{
		R->EAX(pBuilding->Type->PowersUpToLevel);
		return Continue;
	}

	return ForbidUpgrade;
}

// Parse Powered(Light|Effect|Special) keys for upgrade anims.
DEFINE_HOOK(0x4648B3, BuildingTypeClass_ReadINI_PowerUpAnims, 0x5)
{
	GET(BuildingTypeClass*, pThis, EBP);
	GET(int, index, EBX);

	auto const pINI = &CCINIClass::INI_Art();
	auto const animData = &pThis->BuildingAnim[index - 1];

	std::string baseKey = std::format("PowerUp{:01}", index);
	animData->Powered = pINI->ReadBool(pThis->ImageFile, (baseKey + "Powered").c_str(), animData->Powered);
	animData->PoweredLight = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredLight").c_str(), animData->PoweredLight);
	animData->PoweredEffect = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredEffect").c_str(), animData->PoweredEffect);
	animData->PoweredSpecial = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredSpecial").c_str(), animData->PoweredSpecial);

	return 0;
}

// Don't allow upgrade anims to be created if building is not upgraded or they require power to be shown and the building isn't powered.
FORCEINLINE bool AllowUpgradeAnim(BuildingClass* pBuilding, BuildingAnimSlot anim)
{
	auto const pType = pBuilding->Type;

	if (pType->Upgrades != 0 && anim >= BuildingAnimSlot::Upgrade1 && anim <= BuildingAnimSlot::Upgrade3 && !pBuilding->Anims[int(anim)])
	{
		int upgradeLevel = pBuilding->UpgradeLevel - 1;

		if (upgradeLevel < 0 || (int)anim != upgradeLevel)
			return false;

		auto const animData = pType->BuildingAnim[int(anim)];

		if (((pType->Powered && pType->PowerDrain > 0 && (animData.PoweredLight || animData.PoweredEffect)) ||
			(pType->PoweredSpecial && animData.PoweredSpecial)) &&
			!(pBuilding->CurrentMission != Mission::Construction && pBuilding->CurrentMission != Mission::Selling && pBuilding->IsPowerOnline()))
		{
			return false;
		}
	}

	return true;
}

DEFINE_HOOK(0x45189D, BuildingClass_AnimUpdate_Upgrades, 0x6)
{
	enum { SkipAnim = 0x451B2C };

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(BuildingAnimSlot, anim, STACK_OFFSET(0x34, 0x8));

	if (!AllowUpgradeAnim(pThis, anim))
		return SkipAnim;

	return 0;
}

DEFINE_HOOK(0x4408EB, BuildingClass_Unlimbo_UpgradeBuildings, 0x6) //A
{
	enum { Continue = 0x440912, ForbidUpgrade = 0x440926 };

	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pUpgrade, ESI);

	if (BuildingTypeExtData::CanUpgrade(pBuilding, pUpgrade->Type, pUpgrade->Owner))
	{
		R->EBX(pUpgrade->Type);
		pUpgrade->SetOwningHouse(pBuilding->Owner, false);
		return Continue;
	}

	return ForbidUpgrade;
}
