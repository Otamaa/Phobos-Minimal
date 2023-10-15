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
#include <Ext/AnimType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#pragma region Otamaa

DEFINE_HOOK(0x6FF329, TechnoCllass_FireAt_OccupyAnims, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);

	AnimTypeClass* pDecidedMuzzle = pWeapon->OccupantAnim;

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
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
			const auto pExt = TechnoTypeExtContainer::Instance.Find(pInfantry->Type);

			if (const auto pGarrisonPip = pExt->PipGarrison.Get(nullptr))
			{
				pPipFile = pGarrisonPip;
				nPipFrameIndex = pExt->PipGarrison_FrameIndex.Get();
				nPipFrameIndex = std::clamp(nPipFrameIndex, 0, (int)pGarrisonPip->Frames);
				if (auto const pConvertData = pExt->PipGarrison_Palette)
					pPalette = pConvertData->GetConvert<PaletteManager::Mode::Default>();
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

void DetonateDeathWeapon(TechnoClass* pThis, TechnoTypeClass* pType, WeaponTypeClass* pDecided, int nMult, bool RulesDeath)
{
	if (pDecided)
	{
		auto const pBonus = RulesDeath ? (int)(pType->Strength * 0.5) : (int)(pDecided->Damage * pType->DeathWeaponDamageModifier);
		auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pDecided->Projectile);

		if (const auto pBullet = pBulletTypeExt->CreateBullet(pThis, pThis, pBonus + nMult, pDecided->Warhead, pDecided->Speed,
			WeaponTypeExtContainer::Instance.Find(pDecided)->GetProjectileRange(),
			pDecided->Bright || pDecided->Warhead->Bright, true))
		{
			pBullet->SetWeaponType(pDecided);
			BulletExtData::DetonateAt(pBullet, pThis, pThis, pThis->Location);
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
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	WeaponTypeClass* pWeaponResult = pTypeExt->DeathWeapon.GetOrDefault(pThis, pType->DeathWeapon);

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

DEFINE_HOOK_AGAIN(0x4DABAB, ObjectClass_WasFallingDown, 0x6)
DEFINE_HOOK(0x4DABBC, ObjectClass_WasFallingDown, 0x6)
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
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
					pTechno->GetOwningHouse(),
					nullptr,
					pTechno,
					false
				);
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

		if (auto pDecidedAnim = TechnoTypeExtContainer::Instance.Find(pAir->Type)->TakeOff_Anim.Get(RulesExtData::Instance()->Aircraft_TakeOffAnim.Get()))
		{
			auto const nCoord = pAir->GetCoords();
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
				pAir->GetOwningHouse(),
				nullptr,
				pAir,
				false
			);
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4CEB51, FlyLocomotionClass_LandingAnim, 0x8)
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
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDecidedType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0),
				pLinked->GetOwningHouse(),
				nullptr,
				pLinked,
				false
			);
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
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pExt->ROF_Random.Get())
	{
		const auto nDefault = Point2D{RulesExtData::Instance()->ROF_RandomDelay->X , RulesExtData::Instance()->ROF_RandomDelay->Y };
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pExt->Rof_RandomMinMax.Get(nDefault));
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if(pWeaponExt->ROF_RandomDelay.isset()){
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->ROF_RandomDelay);
	}

	R->EAX(nResult);
	return 0x6FD0B5;
}

DEFINE_HOOK(0x4DECBB, FootClass_Destroy_SpinSpeed, 0x5) //A
{
	GET(FootClass* const, pThis, ESI);

	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

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

	if(const auto pInf = specific_cast<InfantryClass*>(pThis)) {
		if (InfantryTypeExtContainer::Instance.Find(pInf->Type)->Is_Cow) {
			pThis->UpdateIdleAction();
			return pThis->Destination ? Skip : SetMissionRate;
		}
	}

	return Continue;
}

// DEFINE_HOOK(0x51CE9A, InfantryClass_RandomAnim_IsCow, 0x5) //7
// {
// 	GET(InfantryClass*, pThis, ESI);
//
// 	R->EDI(R->EAX());
// 	R->BL(InfantryTypeExt::ExtMap.Find(pThis->Type)->Is_Cow);
// 	return 0x51CEAA;
// }

DEFINE_HOOK(0x4A7755, DiskLaserClass_Update_ChargedUpSound, 0x6) //B
{
	GET(DiskLaserClass* const, pThis, ESI);

	if (pThis && pThis->Owner)
	{
		R->ECX(TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType())->DiskLaserChargeUp.Get(RulesClass::Instance->DiskLaserChargeUp));
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
		if (auto const pAnimType = TechnoTypeExtContainer::Instance.Find(Drainer->GetTechnoType())->DrainAnimationType.Get(RulesClass::Instance->DrainAnimationType))
		{
			auto const nCoord = Drainer->GetCoords();
			auto const pDrainAnimCreated = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
			AnimExtData::SetAnimOwnerHouseKind(pDrainAnimCreated, Drainer->Owner, pVictim->Owner, Drainer, false);
			R->EAX(pDrainAnimCreated);
			return 0x70FE07;
		}
	}

	return 0x0;
}


DEFINE_HOOK(0x5184F7, InfantryClass_ReceiveDamage_NotHuman, 0x6)
{
	enum
	{
		Delete = 0x518619,
		DoOtherAffects = 0x518515,
		IsHuman = 0x5185C8,
		CheckAndReturnDamageResultDestroyed = 0x5185F1,
		PlayInfDeaths = 0x5185CE
	};

	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xD0, -0x4));
	GET(DWORD, InfDeath, EDI);

	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

	if (!pThis->Type->NotHuman)
	{
		--InfDeath;
		R->EDI(InfDeath);

		bool Handled = false;

		if (pThis->GetHeight() < 10)
		{
			if (pWarheadExt->InfDeathAnims.contains(pThis->Type->ArrayIndex)
				&& pWarheadExt->InfDeathAnims[pThis->Type->ArrayIndex])
			{
				AnimClass* Anim = GameCreate<AnimClass>(pWarheadExt->InfDeathAnims[pThis->Type->ArrayIndex],
				pThis->Location);

				HouseClass* const Invoker = (args.Attacker)
					? args.Attacker->Owner
					: args.SourceHouse
					;

				AnimExtData::SetAnimOwnerHouseKind(Anim, Invoker, pThis->Owner ,args.Attacker , false);

				Handled = true;
			}
			else
				if (AnimTypeClass* deathAnim = pWarheadExt->InfDeathAnim)
				{
					AnimClass* Anim = GameCreate<AnimClass>(deathAnim, pThis->Location);

					HouseClass* const Invoker = (args.Attacker)
						? args.Attacker->Owner
						: args.SourceHouse
						;

					//these were MakeInf stuffs  , to make sure no behaviour chages
					AnimExtData::SetAnimOwnerHouseKind(Anim, Invoker, pThis->Owner, args.Attacker , false);

					Handled = true;
				}
		}

		return (Handled || InfDeath >= 10)
			? CheckAndReturnDamageResultDestroyed
			: PlayInfDeaths
			;

		//return IsHuman;
	}

	R->ECX(pThis);

	if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
	{
		if (auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location))
		{
			auto pInvoker = args.Attacker ? args.Attacker->GetOwningHouse() : nullptr;
			AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), args.Attacker, true);
			pAnim->ZAdjust = pThis->GetZAdjustment();
		}
	}
	else
	{
		auto const& whSequence = pWarheadExt->NotHuman_DeathSequence;
		// Die1-Die5 sequences are offset by 10
		constexpr auto Die = [](int x) { return x + 10; };

		int resultSequence = Die(1);

		if (!whSequence.isset()
			&& TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NotHuman_RandomDeathSequence.Get())
		{
			resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));
		}
		else if (whSequence.isset())
		{
			resultSequence = std::clamp(Die(abs(whSequence.Get())), Die(1), Die(5));
		}

		InfantryExtContainer::Instance.Find(pThis)->IsUsingDeathSequence = true;

		//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
		if (pThis->PlayAnim(static_cast<DoType>(resultSequence), true))
		{
			return DoOtherAffects;
		}
	}

	return Delete;
}

#pragma endregion