#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif


#pragma region Otamaa
/* Dont Enable ! , broke targeting !
DEFINE_HOOK(0x6F7891, TechnoClass_TriggersCellInset_IgnoreVertical, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0xC));
	GET(TechnoClass*, pTarget, EBX);

	bool bRangeIgnoreVertical = false;
	if (auto const pExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		bRangeIgnoreVertical = pExt->Range_IgnoreVertical.Get();
	}

	if (pThis->IsInAir() && !bRangeIgnoreVertical) {
		nCoord.Z = pTarget->GetCoords().Z;
	}

	R->EAX(pThis->InRange(nCoord, pTarget, pWeapon));
	return 0x6F78BD;
}

DEFINE_HOOK(0x6F7893, TechnoClass_TriggersCellInset_IgnoreVertical, 0x5)
{
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(TechnoClass*, pThis, ESI);

	bool bRangeVertical = pThis->IsInAir();
	if (auto const pExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		bRangeVertical = bRangeVertical && !pExt->Range_IgnoreVertical.Get();
	}

	R->AL(bRangeVertical);
	return 0x6F7898;
}*/

DEFINE_HOOK(0x6FF329, TechnoCllass_FireAt_OccupyAnims, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);

	AnimTypeClass* pDecidedMuzzle = pWeapon->OccupantAnim;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (pWeaponExt->OccupantAnim_UseMultiple.Get())
	{
		if (!pWeaponExt->OccupantAnims.empty())
			pDecidedMuzzle = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
	}

	R->EDI(pDecidedMuzzle);
	return 0x6FF32F;
}

// this hook already inside loop function !
DEFINE_HOOK(0x709C84, TechnoClass_DrawPip_Occupants, 0x6)
{
	struct DrawPipDataStruct
	{
		int nOccupantsCount; int Y; SHPStruct* pShape; int nMaxOccupants;
	};

	GET(BuildingClass*, pThis, EBP);
	GET(int, nOccupantIdx, EDI);
	GET(int, nOffset_X, EBX);
	GET_STACK(int, nOffset_Y, STACK_OFFS(0x74, 0x50));
	GET_STACK(DrawPipDataStruct, nPipDataStruct, STACK_OFFS(0x74, 0x60));
	GET_STACK(Point2D, nDrawOffset, STACK_OFFS(0x74, 0x24));
	GET_STACK(Point2D, nOffsetadd, STACK_OFFS(0x74, 0x1C));
	GET_STACK(RectangleStruct*, pRect, STACK_OFFS(0x74, -0xC));
	GET(int, nOffsetY_Increment, ESI);

	int nPipFrameIndex = 6;
	SHPStruct* pPipFile = nPipDataStruct.pShape;
	ConvertClass* pPalette = FileSystem::THEATER_PAL;

	if (nOccupantIdx < nPipDataStruct.nMaxOccupants)
	{
		if (auto const pInfantry = pThis->Occupants.GetItem(nOccupantIdx))
		{
			const auto pExt = TechnoTypeExt::ExtMap.Find(pInfantry->Type);

			if (auto const pGarrisonPip = pExt->PipGarrison.Get(nullptr))
			{
				pPipFile = pGarrisonPip;
				nPipFrameIndex = pExt->PipGarrison_FrameIndex.Get();
				nPipFrameIndex = std::clamp(nPipFrameIndex, 0, (int)pGarrisonPip->Frames);
				pPalette = pExt->PipGarrison_Palette.GetOrDefaultConvert(pPalette);
			}
			else
			{
				nPipFrameIndex = (int)pInfantry->Type->OccupyPip;
			}
		}
	}

	Point2D nOffset { nOffset_X + nDrawOffset.X ,nDrawOffset.Y + nOffset_Y };
	if (pPipFile)
	{
		DSurface::Temp->DrawSHP(
			pPalette,
			pPipFile,
			nPipFrameIndex,
			&nOffset,
			pRect,
			BlitterFlags(0x600),
			0,
			0,
			ZGradient::None,
			1000,
			0,
			0,
			0,
			0,
			0);
	}

	++nOccupantIdx;
	nOffset_X += nOffsetadd.X;
	nOffset_Y += nOffsetY_Increment;

	// need to forward the value bacause it is needed for next loop
	R->EBX(nOffset_X);
	R->ECX(nOffset_Y);
	R->EDI(nOccupantIdx);
	R->EAX(nPipDataStruct.nOccupantsCount);

	return 0x709D11;
}

void NOINLINE DetonateDeathWeapon(TechnoClass* pThis , TechnoTypeClass* pType ,WeaponTypeClass* pDecided , int nMult,  bool RulesDeath)
{
	if (pDecided)
	{
		auto const pBonus = RulesDeath ? (int)(pType->Strength * 0.5) : (int)(pDecided->Damage * pType->DeathWeaponDamageModifier);
		auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pDecided->Projectile);

		if (const auto pBullet = pBulletTypeExt->CreateBullet(pThis, pThis, pBonus + nMult, pDecided->Warhead, pDecided->Speed, 0,
			pDecided->Bright || pDecided->Warhead->Bright, true))
		{
			pBullet->SetWeaponType(pDecided);
			BulletExt::DetonateAt(pBullet, pThis, pThis, pThis->Location);
		}
	}
}

DEFINE_HOOK(0x70D690, TechnoClass_FireDeathWeapon_Replace, 0x5) //4
{
	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0x0;

	auto const pType = pThis->GetTechnoType();
	GET_STACK(int, nMult, 0x4);

	// Using Promotable<WeaponTypeClass*>
	// tags : "%sDeathWeapon (%s replaced with rank level);
	WeaponTypeClass* pWeaponResult = TechnoTypeExt::ExtMap.Find(pType)->DeathWeapon.GetOrDefault(pThis, pType->DeathWeapon);

	if (!pWeaponResult) {
		auto const pPrimary = pThis->GetWeapon(0);

		if (pPrimary && pPrimary->WeaponType) {
			DetonateDeathWeapon(pThis, pType, pPrimary->WeaponType, nMult, false);
		} else {
			DetonateDeathWeapon(pThis, pType, RulesClass::Instance->DeathWeapon, nMult, true);
		}

	} else {
		DetonateDeathWeapon(pThis, pType, pWeaponResult, nMult, false);
	}

	return 0x70D796;
}

DEFINE_HOOK_AGAIN(0x4DABAB, ObjectClass_WasFallingDown, 0x6)
DEFINE_HOOK(0x4DABBC, ObjectClass_WasFallingDown, 0x6)
{
	GET(ObjectClass*, pThis, ESI);

	if (!pThis || pThis->IsFallingDown)
		return 0x0;

	if (Is_Aircraft(pThis) || pThis->AbstractFlags & AbstractFlags::Techno)
		return 0x0;

	if (auto const pTechno = static_cast<TechnoClass*>(pThis))
	{
		auto const pExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

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
				if (auto pAnim = GameCreate<AnimClass>(pDecidedAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, pTechno->GetOwningHouse(), nullptr, pTechno, false);
				}
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4CE689, FlyLocomotionClass_TakeOffAnim, 0x5)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	if (const auto pAir = specific_cast<AircraftClass*>(pThis->LinkedTo))
	{
		if (pAir->IsInAir())
			return 0x0;

		auto const pCell = pAir->GetCell();
		if (!pCell || pAir->GetHeight() > pCell->GetFloorHeight({ 1,1 }))
			return 0x0;

		if (auto pDecidedAnim = TechnoTypeExt::ExtMap.Find(pAir->Type)->TakeOff_Anim.Get(RulesExt::Global()->Aircraft_TakeOffAnim.Get()))
		{
			auto const nCoord = pAir->GetCoords();
			if (auto pAnim = GameCreate<AnimClass>(pDecidedAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pAir->GetOwningHouse(), nullptr, pAir, false);
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4CEB51, FlyLocomotionClass_LandingAnim, 0x8)
{
	GET(AircraftClass*, pLinked, ECX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x48, 0x18));

	const auto pType = pLinked->Type;
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	{
		auto GetDefaultType = [pType]()
		{
			if (pType->IsDropship)
				return RulesExt::Global()->DropShip_LandAnim.Get();
			else if (pType->Carryall)
				return RulesExt::Global()->CarryAll_LandAnim.Get();

			return (AnimTypeClass*)nullptr;
		};

		const auto pCell = pLinked->GetCell();
		const auto pFirst = pCell->LandType == LandType::Water && !pCell->ContainsBridge() && pExt->Landing_AnimOnWater.Get()
			? pExt->Landing_AnimOnWater.Get() : pExt->Landing_Anim.Get(RulesExt::Global()->Aircraft_LandAnim.Get());

		if (AnimTypeClass* pDecidedType = pFirst ? pFirst : GetDefaultType())
		{
			if (auto pAnim = GameCreate<AnimClass>(pDecidedType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pLinked->GetOwningHouse(), nullptr, pLinked, false);
			}
		}

		return 0x4CEC5D;
	}

	//return 0x0;
}

DEFINE_HOOK(0x6FD0A6, TechnoClass_RearmDelay_RandomROF, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	int nResult = 0;
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt->ROF_Random.Get()) {
		auto const& nData = pExt->Rof_RandomMinMax.Get({ 0,2 });
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(nData);
	}

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	auto const& range = pWeaponExt->ROF_RandomDelay.Get(RulesExt::Global()->ROF_RandomDelay);
	nResult += GeneralUtils::GetRangedRandomOrSingleValue(range);

	R->EAX(nResult);
	return 0x6FD0B5;
}

DEFINE_HOOK(0x4DECBB, FootClass_Destroy_SpinSpeed, 0x5) //A
{
	GET(FootClass* const, pThis, ESI);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	pThis->RockingSidewaysPerFrame = static_cast<float>((ScenarioClass::Instance->Random.RandomDouble() * 0.15 + 0.1) * pExt->CrashSpinLevelRate.Get());

	if (!ScenarioClass::Instance->Random.RandomBool())
		pThis->RockingSidewaysPerFrame = -pThis->RockingSidewaysPerFrame;

	pThis->RockingForwardsPerFrame = static_cast<float>(ScenarioClass::Instance->Random.RandomDouble() * 0.1 * pExt->CrashSpinVerticalRate.Get());


	return 0x4DED4B;
}

DEFINE_HOOK(0x4D42C4, FootClass_Mission_Patrol_IsCow, 0x6) //8
{
	enum { Skip = 0x4D42D2, SetMissionRate = 0x4D4569, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Is_Cow.Get())
	{
		pThis->UpdateIdleAction();
		return pThis->Destination ? Skip : SetMissionRate;
	}

	return Continue;
}

DEFINE_HOOK(0x51CE9A, InfantryClass_RandomAnim_IsCow, 0x5) //7
{
	GET(InfantryClass*, pThis, ESI);

	R->EDI(R->EAX());
	R->BL(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Is_Cow.Get());
	return 0x51CEAA;
}

DEFINE_HOOK(0x4A7755, DiskLaserClass_Update_ChargedUpSound, 0x6) //B
{
	GET(DiskLaserClass* const, pThis, ESI);

	if (pThis && pThis->Owner)
	{
		R->ECX(TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType())->DiskLaserChargeUp.Get(RulesClass::Instance->DiskLaserChargeUp));
		return 0x4A7760;
	}

	return 0x0;
}

DEFINE_HOOK(0x70FDC2, TechnoClass_Drain_LocalDrainAnim, 0x5) //A
{
	GET(TechnoClass*, Drainer, ESI);
	GET(TechnoClass*, pVictim, EDI);

	if (Drainer && pVictim)
	{
		if (auto const pAnimType = TechnoTypeExt::ExtMap.Find(Drainer->GetTechnoType())->DrainAnimationType.Get(RulesClass::Instance->DrainAnimationType))
		{
			auto const nCoord = Drainer->GetCoords();
			if (auto const pDrainAnimCreated = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
			{
				AnimExt::SetAnimOwnerHouseKind(pDrainAnimCreated, Drainer->Owner, pVictim->Owner, Drainer, false);
				R->EAX(pDrainAnimCreated);
				return 0x70FE07;
			}
		}
	}

	return 0x0;
}

/*
DEFINE_HOOK(0x4B387A, DriveLocoClass_ClearNavCom2_Empty_WTF, 0x4)
{
	GET(UnitClass*, pLinked, ECX);

	if (pLinked->Destination)
	{
		if (auto pDest = specific_cast<AircraftClass*>(pLinked->Destination))
		{
			return 0x4B3607;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4B05EE, DriveLocoClass_InfCheck_Extend , 0x5)
{
	GET(AbstractClass*, pDest, ECX);

	return pDest->WhatAmI() == AbstractType::Infantry || pDest->WhatAmI() == AbstractType::Aircraft ? 0x4B05F8 : 0x4B063D;
}*/


#ifdef COMPILE_PORTED_DP_FEATURES

#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#endif
// init inside type check 
// should be no problem here


DEFINE_HOOK(0x6F42ED, TechnoClass_Init_DP, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	auto pType = pThis->GetTechnoType();

	if (!pType)
		return 0x0;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

#ifdef COMPILE_PORTED_DP_FEATURES
	//if (pTypeExt->VirtualUnit.Get())
	//	pExt->VirtualUnit = true;

	//if (pExt->VirtualUnit)
	//{
	//	pThis->UpdatePlacement(PlacementType::Remove);
	//	pThis->IsOnMap = false;
	//	pType->DontScore = true;
	//	pType->Selectable = false;
	//	pType->Immune = true;
	//}

	AircraftDiveFunctional::Init(pExt, pTypeExt);
#endif
	TechnoExt::InitializeItems(pThis , pType);

	return 0x0;
}

/*
DEFINE_HOOK(0x6F3B2E, TechnoClass_Transform_FLH, 0x6)
{
	GET(WeaponStruct*, nWeaponStruct, EAX);
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, idxWeapon, 0x8);

	CoordStruct nRet = nWeaponStruct->FLH;

	if (auto const pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (pInf->Crawling)
		{
			if (auto const pExt = TechnoTypeExt::ExtMap.Find(pInf->Type))
			{
				if (!pThis->Veterancy.IsElite())
				{
					if (idxWeapon == 0)
						nRet = pExt->PrimaryCrawlFLH.Get(nWeaponStruct->FLH);
					else
						nRet = pExt->SecondaryCrawlFLH.Get(nWeaponStruct->FLH);
				}
				else
				{
					if (idxWeapon == 0)
						nRet = pExt->Elite_PrimaryCrawlFLH.Get(nWeaponStruct->FLH);
					else
						nRet = pExt->Elite_SecondaryCrawlFLH.Get(nWeaponStruct->FLH);
				}
			}
		}
	}

	R->ECX(nRet.X);
	R->EBP(nRet.Y);
	R->EAX(nRet.Z);

	return 0x6F3B37;
}*/

// DEFINE_HOOK(0x702E9D, TechnoClass_RegisterDestruction, 0x6)
// {
// 	GET(TechnoClass*, pVictim, ESI);
// 	GET(TechnoClass*, pKiller, EDI);
// 	GET(int, cost, EBP);

// 	const auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());
// 	const auto pKillerTypeExt = TechnoTypeExt::ExtMap.Find(pKiller->GetTechnoType());
// 	const double giveExpMultiple = pVictimTypeExt->Experience_VictimMultiple.Get();
// 	const double gainExpMultiple = pKillerTypeExt->Experience_KillerMultiple.Get();

// 	R->EBP(int(cost * giveExpMultiple * gainExpMultiple));

// 	return 0;
// }

#pragma endregion