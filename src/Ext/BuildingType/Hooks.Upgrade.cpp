#include "Body.h"

#include <FactoryClass.h>
#include <HouseClass.h>

#include <Utilities/EnumFunctions.h>
#include <Ext/TechnoType/Body.h>


ASMJIT_PATCH(0x452678, BuildingClass_CanUpgrade_UpgradeBuildings, 0x6) //8
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
ASMJIT_PATCH(0x464749, BuildingTypeClass_ReadINI_PowerUpAnims, 0x6)
{
	GET(FakeBuildingTypeClass*, pThis, EBP);

	pThis->_GetExtData()->HasPowerUpAnim.clear();
	auto const pINI = &CCINIClass::INI_Art();
	struct Keys {
		const char* _Base;
		const char* _Anim;
		const char* _DamagedAnim;
		const char* _LocXX;
		const char* _LocYY;
		const char* _LocZZ;
		const char* _YSort;
		const char* _Powered;
		const char* _PoweredLight;
		const char* _PoweredEffect;
		const char* _PoweredSpecial;
	};

	static constexpr std::array<Keys, 3> _Keys = {
		Keys{
			"PowerUp1" , "PowerUp1Anim" ,	"PowerUp1DamagedAnim" ,
			"PowerUp1LocXX" ,	"PowerUp1LocYY" ,	"PowerUp1LocZZ" ,
			"PowerUp1YSort" ,	"PowerUp1Powered" ,	"PowerUp1PoweredLight" ,
			"PowerUp1PoweredEffect" ,"PowerUp1PoweredSpecial" }

		,

		Keys{
			"PowerUp2" , "PowerUp2Anim" ,	"PowerUp2DamagedAnim" ,
			"PowerUp2LocXX" ,	"PowerUp2LocYY" ,	"PowerUp2LocZZ" ,
			"PowerUp2YSort" ,	"PowerUp2Powered" ,	"PowerUp2PoweredLight" ,
			"PowerUp2PoweredEffect" ,"PowerUp2PoweredSpecial"
		}

		,

		Keys{
			"PowerUp3" , "PowerUp3Anim" ,	"PowerUp3DamagedAnim" ,
			"PowerUp3LocXX" ,	"PowerUp3LocYY" ,	"PowerUp3LocZZ" ,
			"PowerUp3YSort" ,	"PowerUp3Powered" ,	"PowerUp3PoweredLight" ,
			"PowerUp3PoweredEffect" ,"PowerUp3PoweredSpecial"
		}
	};

	for (int i = 0; i < 3; ++i)
	{
		auto animData = &pThis->BuildingAnim[i];
		pINI->ReadString(pThis->ImageFile, _Keys[i]._Anim, Phobos::readDefval, animData->Anim);
		pINI->ReadString(pThis->ImageFile, _Keys[i]._DamagedAnim, Phobos::readDefval, animData->Damaged);

		animData->Position.X = pINI->ReadInteger(pThis->ImageFile,  _Keys[i]._LocXX, animData->Position.X);
		animData->Position.Y = pINI->ReadInteger(pThis->ImageFile,  _Keys[i]._LocYY, animData->Position.Y);
		animData->ZAdjust = pINI->ReadInteger(pThis->ImageFile,  _Keys[i]._LocZZ, animData->ZAdjust);
		animData->YSort = pINI->ReadInteger(pThis->ImageFile,  _Keys[i]._YSort, animData->YSort);

		animData->Powered = pINI->ReadBool(pThis->ImageFile,  _Keys[i]._Powered, animData->Powered);
		animData->PoweredLight = pINI->ReadBool(pThis->ImageFile,  _Keys[i]._PoweredLight, animData->PoweredLight);
		animData->PoweredEffect = pINI->ReadBool(pThis->ImageFile,  _Keys[i]._PoweredEffect, animData->PoweredEffect);
		animData->PoweredSpecial = pINI->ReadBool(pThis->ImageFile,  _Keys[i]._PoweredSpecial, animData->PoweredSpecial);
		pThis->_GetExtData()->HasPowerUpAnim.emplace_back(GeneralUtils::IsValidString(animData->Anim));
	}

	return 0x46492E;
}

#include <Ext/Building/Body.h>

ASMJIT_PATCH(0x440988, BuildingClass_Unlimbo_UpgradeAnims, 0x7)
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
	const bool HasPowerAnim = (size_t)animIndex < pTarget->_GetTypeExtData()->HasPowerUpAnim.size() && !pTarget->_GetTypeExtData()->HasPowerUpAnim[animIndex];
	// Only copy image name to BuildingType anim struct if theres no explicit PowersUpAnim for this level.
	if (HasPowerAnim)
		strncpy_s(animData->Anim, pThis->Type->ImageFile, sizeof(animData->Anim));

	return SkipGameCode;
}

ASMJIT_PATCH(0x451630, BuildingClass_CreateUpgradeAnims_AnimIndex, 0x7)
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
COMPILETIMEEVAL FORCEDINLINE bool AllowUpgradeAnim(BuildingClass* pBuilding, BuildingAnimSlot anim)
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

ASMJIT_PATCH(0x45189D, BuildingClass_AnimUpdate_Upgrades, 0x6)
{
	enum { SkipAnim = 0x451B2C };

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(BuildingAnimSlot, anim, STACK_OFFSET(0x34, 0x8));

	if (!AllowUpgradeAnim(pThis, anim))
		return SkipAnim;

	return 0;
}

ASMJIT_PATCH(0x4408EB, BuildingClass_Unlimbo_UpgradeBuildings, 0x6) //A
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
