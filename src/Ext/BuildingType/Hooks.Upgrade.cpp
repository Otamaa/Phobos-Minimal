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
DEFINE_HOOK(0x464749, BuildingTypeClass_ReadINI_PowerUpAnims, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBP);

	for (int i = 0; i < 3; ++i)
	{
		auto const pINI = &CCINIClass::INI_Art();
		auto const animData = &pThis->BuildingAnim[i];

		const std::string baseKey = std::format("PowerUp{:01}", i + 1);

		pINI->ReadString(pThis->ImageFile, (baseKey + "Anim").c_str(), Phobos::readDefval, animData->Anim);
		pINI->ReadString(pThis->ImageFile, (baseKey + "DamagedAnim").c_str(), Phobos::readDefval, animData->Damaged);

		animData->Position.X = pINI->ReadInteger(pThis->ImageFile, (baseKey + "LocXX").c_str(), animData->Position.X);
		animData->Position.Y = pINI->ReadInteger(pThis->ImageFile, (baseKey + "LocYY").c_str(), animData->Position.Y);
		animData->ZAdjust = pINI->ReadInteger(pThis->ImageFile, (baseKey + "LocZZ").c_str(), animData->ZAdjust);
		animData->YSort = pINI->ReadInteger(pThis->ImageFile, (baseKey + "YSort").c_str(), animData->YSort);

		animData->Powered = pINI->ReadBool(pThis->ImageFile, (baseKey + "Powered").c_str(), animData->Powered);
		animData->PoweredLight = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredLight").c_str(), animData->PoweredLight);
		animData->PoweredEffect = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredEffect").c_str(), animData->PoweredEffect);
		animData->PoweredSpecial = pINI->ReadBool(pThis->ImageFile, (baseKey + "PoweredSpecial").c_str(), animData->PoweredSpecial);
	}

	return 0x46492E;
}

#include <Ext/Building/Body.h>

DEFINE_HOOK(0x440988, BuildingClass_Unlimbo_UpgradeAnims, 0x7)
{
	enum { SkipGameCode = 0x4409C7 };

	GET(BuildingClass*, pThis, ESI);
	GET(FakeBuildingClass*, pTarget, EDI);

	auto const pTargetExt = pTarget->_GetExtData();
	pTargetExt->PoweredUpToLevel = pTarget->UpgradeLevel + 1;
	int animIndex = pTarget->UpgradeLevel;

	if (pThis->Type->PowersUpToLevel > 0)
	{
		pTargetExt->PoweredUpToLevel = pThis->Type->PowersUpToLevel;
		animIndex = pTargetExt->PoweredUpToLevel - 1;
	}

	auto const animData = &pTarget->Type->BuildingAnim[animIndex];

	// Only copy image name to BuildingType anim struct if it is not already set.
	if (!GeneralUtils::IsValidString(animData->Anim))
		strncpy_s(animData->Anim, pThis->Type->ImageFile, sizeof(animData->Anim));

	return SkipGameCode;
}

DEFINE_HOOK(0x451630, BuildingClass_CreateUpgradeAnims_AnimIndex, 0x7)
{
	enum { SkipGameCode = 0x451638 };

	GET(FakeBuildingClass*, pThis, EBP);

	const int animIndex = pThis->_GetExtData()->PoweredUpToLevel - 1;

	if (animIndex) {
		R->EAX(animIndex);
		return SkipGameCode;
	}

	return 0;
}

// Don't allow upgrade anims to be created if building is not upgraded or they require power to be shown and the building isn't powered.
constexpr FORCEINLINE bool AllowUpgradeAnim(BuildingClass* pBuilding, BuildingAnimSlot anim)
{
	auto const pType = pBuilding->Type;

	if (pType->Upgrades != 0 && anim >= BuildingAnimSlot::Upgrade1 && anim <= BuildingAnimSlot::Upgrade3 && !pBuilding->Anims[int(anim)])
	{
		int animIndex = BuildingExtContainer::Instance.Find(pBuilding)->PoweredUpToLevel - 1;

		if (animIndex < 0 || (int)anim != animIndex)
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
