#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Locomotor/FlyLocomotionClass.h>
#include <DiskLaserClass.h>

#pragma region Otamaa

// ASMJIT_PATCH(0x6FF329, TechnoCllass_FireAt_OccupyAnims, 0x6)
// {
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	AnimTypeClass* pDecidedMuzzle = pWeapon->OccupantAnim;
//
// 	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
// 	if (pWeaponExt->OccupantAnim_UseMultiple.Get() && !pWeaponExt->OccupantAnims.empty()) {
// 		pDecidedMuzzle = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
// 	}
//
// 	R->EDI(pDecidedMuzzle);
// 	return 0x6FF32F;
// }

// this hook already inside loop function !
#include <Ext/Scenario/Body.h>

void DetonateDeathWeapon(TechnoClass* pThis, TechnoTypeClass* pType, WeaponTypeClass* pDecided, int nMult, bool RulesDeath)
{
	if (pDecided)
	{
		auto const pBonus = RulesDeath ? (int)(pType->Strength * 0.5) : (int)(pDecided->Damage * pType->DeathWeaponDamageModifier);

		ScenarioExtData::DetonateMasterBullet(pThis->Location,
			pThis,
			pBonus + nMult,
			pThis->Owner,
			pThis,
			pDecided->Bright || pDecided->Warhead->Bright,
			pDecided,
			pDecided->Warhead
		);
		//if (const auto pBullet = pBulletTypeExt->CreateBullet(pThis, pThis, pBonus + nMult, pDecided->Warhead, pDecided->Speed,
		//	WeaponTypeExtContainer::Instance.Find(pDecided)->GetProjectileRange(),
		//	pDecided->Bright || pDecided->Warhead->Bright, true))
		//{
		//	pBullet->SetWeaponType(pDecided);
		//	BulletExtData::DetonateAt(pBullet, pThis, pThis, pThis->Location);
		//}
	}
}

ASMJIT_PATCH(0x70D690, TechnoClass_FireDeathWeapon_Replace, 0x5) //4
{
	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0x0;

	auto const pType = pThis->GetTechnoType();
	GET_STACK(int, nMult, 0x4);

	// Using Promotable<WeaponTypeClass*>
	// tags : "%sDeathWeapon (%s replaced with rank level);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	WeaponTypeClass* pWeaponResult = pTypeExt->DeathWeapon.Get(pThis);
	if (!pWeaponResult)
		pWeaponResult = pType->DeathWeapon;

	if (!pWeaponResult)
	{
		auto const pPrimary = pThis->GetWeapon(0);

		if (pPrimary && pPrimary->WeaponType)
		{
			if(pTypeExt->DeathWeapon_CheckAmmo && pThis->Ammo <= 0 )
				return 0x70D796;

			DetonateDeathWeapon(pThis, pType, pPrimary->WeaponType, nMult, false);
		}
		else
		{
			DetonateDeathWeapon(pThis, pType, RulesClass::Instance->DeathWeapon, nMult, true);
		}

	}
	else
	{
		DetonateDeathWeapon(pThis, pType, pWeaponResult, nMult, false);
	}

	return 0x70D796;
}

ASMJIT_PATCH(0x4DABBC, ObjectClass_WasFallingDown, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	if (pThis->IsFallingDown)
		return 0x0;

	if (((pThis->AbstractFlags & AbstractFlags::Techno) == AbstractFlags::None) || pThis->WhatAmI() == AircraftClass::AbsID )
		return 0x0;

	auto const pTechno = static_cast<TechnoClass*>(pThis);

	{
		auto const pExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		{
			auto const GetLandingAnim = [pExt, pTechno]()
			{
				auto pDecidedAnim = pExt->Landing_Anim.Get();
				if (auto const pCell = pTechno->GetCell())
				{
					if (!pCell->ContainsBridge() && pCell->LandType == LandType::Water)
						pDecidedAnim = pExt->Landing_AnimOnWater.Get();
				}

				return pDecidedAnim;
			};

			if (auto pDecidedAnim = GetLandingAnim())
			{
				auto const nCoord = pTechno->GetCoords();
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedAnim, nCoord, 1, 1, AnimFlag::AnimFlag_600, 0, 0),
					pTechno->GetOwningHouse(),
					nullptr,
					pTechno,
					false, false
				);
			}
		}
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x4DABAB, ObjectClass_WasFallingDown, 0x6)

ASMJIT_PATCH(0x4CE689, FlyLocomotionClass_TakeOffAnim, 0x5)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	if (const auto pAir = cast_to<AircraftClass*, false>(pThis->LinkedTo))
	{
		if (pAir->IsInAir())
			return 0x0;

		auto const pCell = pAir->GetCell();
		if (!pCell || pAir->GetHeight() > pCell->GetFloorHeight({ 1,1 }))
			return 0x0;

		if (auto pDecidedAnim = TechnoTypeExtContainer::Instance.Find(pAir->Type)->TakeOff_Anim.Get(RulesExtData::Instance()->Aircraft_TakeOffAnim.Get()))
		{
			auto const nCoord = pAir->GetCoords();
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedAnim, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
				pAir->GetOwningHouse(),
				nullptr,
				pAir,
				false, false
			);
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x4CEB51, FlyLocomotionClass_LandingAnim, 0x8)
{
	GET(AircraftClass*, pLinked, ECX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x48, 0x18));

	const auto pType = pLinked->Type;
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	{
		auto GetDefaultType = [pType]()
		{
			if (pType->IsDropship)
				return RulesExtData::Instance()->DropShip_LandAnim.Get();
			else if (pType->Carryall)
				return RulesExtData::Instance()->CarryAll_LandAnim.Get();

			return (AnimTypeClass*)nullptr;
		};

		const auto pCell = pLinked->GetCell();
		const auto pFirst = pCell->LandType == LandType::Water && !pCell->ContainsBridge() && pExt->Landing_AnimOnWater.Get()
			? pExt->Landing_AnimOnWater.Get() : pExt->Landing_Anim.Get(RulesExtData::Instance()->Aircraft_LandAnim.Get());

		if (AnimTypeClass* pDecidedType = pFirst ? pFirst : GetDefaultType())
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedType, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
				pLinked->GetOwningHouse(),
				nullptr,
				pLinked,
				false, false
			);
		}

		return 0x4CEC5D;
	}

	//return 0x0;
}

ASMJIT_PATCH(0x4DECBB, FootClass_Destroy_SpinSpeed, 0x5) //A
{
	GET(FootClass* const, pThis, ESI);

	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	pThis->RockingSidewaysPerFrame = static_cast<float>((ScenarioClass::Instance->Random.RandomDouble() * 0.15 + 0.1) * pExt->CrashSpinLevelRate.Get());

	if (!ScenarioClass::Instance->Random.RandomBool())
		pThis->RockingSidewaysPerFrame = -pThis->RockingSidewaysPerFrame;

	pThis->RockingForwardsPerFrame = static_cast<float>(ScenarioClass::Instance->Random.RandomDouble() * 0.1 * pExt->CrashSpinVerticalRate.Get());


	return 0x4DED4B;
}

ASMJIT_PATCH(0x4D42C4, FootClass_Mission_Patrol_IsCow, 0x6) //8
{
	enum { Skip = 0x4D42D2, SetMissionRate = 0x4D4569, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	if(const auto pInf = cast_to<InfantryClass*, false>(pThis)) {
		if (InfantryTypeExtContainer::Instance.Find(pInf->Type)->Is_Cow) {
			pThis->UpdateIdleAction();
			return pThis->Destination ? Skip : SetMissionRate;
		}
	}

	return Continue;
}

// ASMJIT_PATCH(0x51CE9A, InfantryClass_RandomAnim_IsCow, 0x5) //7
// {
// 	GET(InfantryClass*, pThis, ESI);
//
// 	R->EDI(R->EAX());
// 	R->BL(InfantryTypeExt::ExtMap.Find(pThis->Type)->Is_Cow);
// 	return 0x51CEAA;
// }


ASMJIT_PATCH(0x70FDC2, TechnoClass_Drain_LocalDrainAnim, 0x5) //A
{
	GET(TechnoClass*, Drainer, ESI);
	GET(TechnoClass*, pVictim, EDI);

	if (Drainer && pVictim)
	{
		if (auto const pAnimType = TechnoTypeExtContainer::Instance.Find(Drainer->GetTechnoType())->DrainAnimationType.Get(RulesClass::Instance->DrainAnimationType))
		{
			auto const nCoord = Drainer->GetCoords();
			auto const pDrainAnimCreated = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, false);
			AnimExtData::SetAnimOwnerHouseKind(pDrainAnimCreated, Drainer->Owner, pVictim->Owner, Drainer, false, false);
			R->EAX(pDrainAnimCreated);
			return 0x70FE07;
		}
	}

	return 0x0;
}

#pragma endregion