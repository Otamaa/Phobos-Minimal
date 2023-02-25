#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>
#include <JumpjetLocomotionClass.h>
#include <SlaveManagerClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

#include <New/Entity/ShieldObject.h>

DEFINE_HOOK(0x73DE90, UnitClass_SimpleDeployer_TransferLaserTrails, 0x6)
{
	GET(UnitClass*, pUnit, ESI);

	TechnoExt::InitializeLaserTrail(pUnit, true);
#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Construct(static_cast<TechnoClass*>(pUnit), true);
#endif
	//LineTrailExt::DeallocateLineTrail(pUnit);
	//LineTrailExt::ConstructLineTrails(pUnit);

	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto strength = TechnoExt::GetInitialStrength(pThis->Type , pThis->Type->Strength);
	pThis->Health = strength;
	pThis->EstimatedHealth = strength;

	return 0;
}

DEFINE_HOOK(0x7355BA, UnitClass_Init_InitialStrength, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	R->EAX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x7355C0;
}

DEFINE_HOOK(0x414051, AircraftClass_Init_InitialStrength, 0x6)
{
	GET(AircraftTypeClass*, pType, EAX);
	R->EAX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x414057;
}

DEFINE_HOOK(0x442C75, BuildingClass_Init_InitialStrength, 0x6)
{
	GET(BuildingTypeClass*, pType, EAX);
	R->ECX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x442C7B;
}

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_RearmDelay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	const int burstDelay = WeaponTypeExt::GetBurstDelay(pWeapon ,pThis->CurrentBurstIndex);

	if (burstDelay >= 0) {
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	// Restore overridden instructions
	GET(int, idxCurrentBurst, ECX);
	return idxCurrentBurst <= 0 || idxCurrentBurst > 4 ? 0x6FD084 : 0x6FD067;
}

// Issue #237 NotHuman additional animations support
// Author: Otamaa
DEFINE_HOOK(0x5184F7, InfantryClass_TakeDamage_NotHuman, 0x6)
{
	enum { Delete = 0x518619, DoOtherAffects = 0x518515, IsHuman = 0x5185C8 };

	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	if (!pThis->Type->NotHuman)
		return IsHuman;

	// Die1-Die5 sequences are offset by 10
	constexpr auto Die = [](int x) { return x + 10; };

	int resultSequence = Die(1);

	R->ECX(pThis);

	if (receiveDamageArgs.WH)
	{
		auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH);

		{
			if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
			{
				if (auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location))
				{
					auto pInvoker = receiveDamageArgs.Attacker ? receiveDamageArgs.Attacker->GetOwningHouse() : nullptr;
					AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), receiveDamageArgs.Attacker, true);
					pAnim->ZAdjust = pThis->GetZAdjustment();
					return Delete;
				}
			}
			else
			{
				if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->NotHuman_RandomDeathSequence.Get())
					resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));

				int whSequence = pWarheadExt->NotHuman_DeathSequence.Get();
				if (whSequence > 0)
					resultSequence = Math::min(Die(whSequence), Die(5));

				InfantryExt::ExtMap.Find(pThis)->IsUsingDeathSequence = true;

			}
		}
	}

	//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
	return (!pThis->PlayAnim(static_cast<DoType>(resultSequence), true)) ? Delete : DoOtherAffects;
}

// Customizable OpenTopped Properties
// Author: Otamaa
DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0x6) //C
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		R->EAX(TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())->OpenTopped_RangeBonus.Get(RulesGlobal->OpenToppedRangeBonus));
		return 0x6F72DE;
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0x6) //C
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTransport = pThis->Owner->Transporter)
	{
		R->EDX(TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())->OpenTopped_WarpDistance.Get(RulesGlobal->OpenToppedWarpDistance));
		return 0x71A838;
	}

	return 0;
}

DEFINE_HOOK(0x7098B9, TechnoClass_TargetSomethingNearby_AutoFire, 0x6)
{

	enum { Skip = 0x7099B8, Continue =  0x0};

	GET(TechnoClass* const, pThis, ESI);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt->AutoFire) {
		pThis->SetTarget(pExt->AutoFire_TargetSelf ? pThis : static_cast<AbstractClass*>(pThis->GetCell()));
		return Skip;
	}

	return Continue;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (pTechnoExt->LaserTrails.size())
	{
		for (auto const& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation.clear();
		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Hide(pTechno);
#endif

	return 0;
}

DEFINE_HOOK(0x6F6CFE, TechnoClass_Unlimbo_LaserTrails, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (!pTechnoExt->LaserTrails.empty())
	{
		for (auto const& pLaserTrail : pTechnoExt->LaserTrails)
		{
			if (pLaserTrail)
			{
				pLaserTrail->LastLocation.clear();
				pLaserTrail->Visible = true;
			}
		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Hide(pTechno);
#endif
	return 0;
}

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

	return 0;
}

DEFINE_HOOK(0x6FD446, TechnoClass_LaserZap_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (!pLaser->IsHouseColor && WeaponTypeExt::ExtMap.Find(pWeapon)->Laser_IsSingleColor)
		pLaser->IsHouseColor = true;

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	pLaser->IsSupported = pLaser->Thickness > 3;

	return 0;
}

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#endif

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (Phobos::Debug_DisplayDamageNumbers && *pDamage)
		TechnoExt::DisplayDamageNumberString(pThis, *pDamage, false);

#ifdef COMPILE_PORTED_DP_FEATURES

	GET(DamageState, damageState, EDI);
	GET(WarheadTypeClass*, pWH, EBP);

	GiftBoxFunctional::TakeDamage(TechnoExt::ExtMap.Find(pThis), TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()), pWH, damageState);
#endif

	return 0;
}

//crash ?
DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis && pThis->Health > 0 && pThis->IsAlive && !pThis->InLimbo && !pThis->IsSinking)
	{
		TechnoExt::ApplyGainedSelfHeal(pThis);
	}

	return SkipGameSelfHeal;
}

DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	if (const auto pFoot = generic_cast<FootClass*>(pThis))
		if (const auto pParasiteFoot = pFoot->ParasiteEatingMe)
			if (const auto pParasite = pParasiteFoot->ParasiteImUsing)
				TechnoExt::DrawParasitedPips(pFoot, pLocation, pBounds);

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x98, -0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExt::DrawInsignia(pThis, pLocation, pBounds);

	return SkipGameCode;
}

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x8) //6
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (pThis)
	{	auto pType = pThis->GetTechnoType();

		if(TechnoTypeExt::ExtMap.Find(pType)->UseDisguiseMovementSpeed)
			pType = TechnoExt::GetDisguiseType(pThis, false, false).first;

		maxSpeed = pType->Speed;
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x6B73B9, SpawnManagerClass_AI_SpawnTimer, 0x5)
DEFINE_HOOK(0x6B73A8, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

		if (pTypeExt->Spawner_DelayFrames.isset())
			R->ECX(pTypeExt->Spawner_DelayFrames.Get());
	}

	return 0;
}

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching
		&& pThis->CountDockedSpawns() != 0)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
		if (pTypeExt->Spawner_DelayFrames.isset())
		{
			R->EAX(std::min(pTypeExt->Spawner_DelayFrames.Get(), 10));
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B743E , SpawnManagerClass_AI_SpawnOffsets , 0x6)
{
	GET(TechnoClass* , pOwner , ECX);
	//yes , i include the buffer just in case it used somewhere !
	LEA_STACK(CoordStruct* , pBuffer ,STACK_OFFS(0x68,0x18));
	LEA_STACK(CoordStruct* , pBuffer2 ,STACK_OFFS(0x68,0xC));

	auto const pExt= TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

	if(pExt->Spawner_SpawnOffsets.isset()) {
		if(pExt->Spawner_SpawnOffsets_OverrideWeaponFLH) {
			auto const pRet = pExt->Spawner_SpawnOffsets.GetEx();
			pBuffer = pRet;
			pBuffer2 = pRet;
			R->EAX(pRet);
		}
		else
		{
			CoordStruct FLH = pExt->Spawner_SpawnOffsets.Get();
			if ( pOwner->CurrentBurstIndex ) {
				auto const pRet = pOwner->GetFLH(pBuffer,R->EBP<bool>(),pExt->Get()->SecondSpawnOffset);
				pRet->X += FLH.X;
				pRet->Y += FLH.Y;
				pRet->Z += FLH.Z;
				R->EAX(pRet);
			}else{
				auto const pRet =pOwner->GetFLH(pBuffer2,R->EBP<bool>(),CoordStruct::Empty);
				pRet->X += FLH.X;
				pRet->Y += FLH.Y;
				pRet->Z += FLH.Z;
				R->EAX(pRet);
			}
		}

		return 0x6B7498;
	}

	return 0x0;
}

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6) //0x8
{
	//Kill slave is buged because it doesnt do IgnoreDamage -Otamaa
	enum { KillTheSlave = 0x6B0BDF, SkipSetEax = 0x6B0BB4, LoopCheck = 0x6B0C0B };

	GET_STACK(const SlaveManagerClass*, pThis, STACK_OFFS(0x24, 0x10));
	GET(InfantryClass*, pSlave, ESI);
	GET(TechnoClass*, pKiller, EBX);
	GET_STACK(HouseClass*, pDefaultRetHouse, STACK_OFFS(0x24, 0x14));

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->GetTechnoType());
	{
		if (pTypeExt->Death_WithMaster.Get() || pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide)
		{
			auto nStr = pSlave->Health;
			pSlave->ReceiveDamage(&nStr, 0, RulesGlobal->C4Warhead, nullptr, true, true, nullptr);
			return LoopCheck;
		}

		const auto pVictim = pThis->Owner ? pThis->Owner->GetOwningHouse() : pSlave->GetOwningHouse();
		R->EAX(HouseExt::GetSlaveHouse(pTypeExt->Slaved_ReturnTo, pKiller->GetOwningHouse(), pVictim ? pVictim : pDefaultRetHouse));
		return SkipSetEax;
	}

	//	return 0x0;
}

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(FootClass*, pFoot, EDI);

	if (pBuilding && pBuilding->Type->Cloning && pFoot)
	{
		if (auto const pTypeUnit = pFoot->GetTechnoType())
		{
			auto const& ranges = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType())->InitialStrength_Cloning.Get();

			if (ranges.X || ranges.Y)
			{
				const double percentage = ranges.X >= ranges.Y ? ranges.X :
					static_cast<double>(ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(ranges.X * 100), static_cast<int>(ranges.Y * 100)) / 100.0);
				const int strength = Math::clamp(static_cast<int>(pTypeUnit->Strength * percentage), 1, pTypeUnit->Strength);
				pFoot->Health = strength;
				pFoot->EstimatedHealth = strength;
			}
		}
	}

	return 0;
}

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A5, TechnoClass_GetTurretWeapon_LaserWeapon, 0x6)
{
	enum { ReturnResult = 0x70E1C7, Continue = 0x70E1AB };

	GET(TechnoClass* const, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);

		if (!pExt->CurrentLaserWeaponIndex.empty())
		{
			R->EAX(pThis->GetWeapon(pExt->CurrentLaserWeaponIndex.get()));
			return ReturnResult;
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());
	return Continue;
}

DEFINE_HOOK(0x4D9F8A, FootClass_Sell_Sellsound, 0x5)
{
	enum { SkipVoxVocPlay = 0x4D9FB5 };
	GET(FootClass*, pThis, ESI);

	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndexById(GameStrings::EVA_UnitSold())));
		//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesGlobal->SellSound), pThis->Location);
	}

	return SkipVoxVocPlay;
}


DEFINE_HOOK(0x70265F, TechnoClass_ReceiveDamage_Explodes, 0x6)
{
	enum { SkipKillingPassengers = 0x702669 };

	GET(TechnoClass*, pThis, ESI);
	return !TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Explodes_KillPassengers ? SkipKillingPassengers : 0x0;
}

DEFINE_HOOK(0x6FD054, TechnoClass_RearmDelay_ForceFullDelay, 0x6)
{
	enum { ApplyFullRearmDelay = 0x6FD09E };

	GET(TechnoClass*, pThis, ESI);

	// Currently only used with infantry, so a performance saving measure.
	if (auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		const auto pExt = InfantryExt::ExtMap.Find(pInf);
		if (pExt->ForceFullRearmDelay)
		{
			pExt->ForceFullRearmDelay = false;
			pThis->CurrentBurstIndex = 0;
			return ApplyFullRearmDelay;
		}
	}

	return 0;
}

DEFINE_HOOK(0x5209A7, InfantryClass_FiringAI_BurstDelays, 0x8)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	const auto pWeaponstruct = pThis->GetWeapon(weaponIndex);

	if (!pWeaponstruct)
		return ReturnFromFunction;

	const auto pWeapon = pWeaponstruct->WeaponType;

	if (!pWeapon)
		return ReturnFromFunction;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int cumulativeDelay = 0;
	int projectedDelay = 0;

	// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
	if (pWeaponExt->Burst_FireWithinSequence)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			int burstDelay = WeaponTypeExt::GetBurstDelay(pWeapon , i);
			int delay = 0;

			if (burstDelay > -1)
				delay = burstDelay;
			else
				delay = ScenarioClass::Instance->Random.RandomRanged(3, 5);

			// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
			if (i != 0)
				delay = Math::max(delay, 1);

			cumulativeDelay += delay;

			if (i == pThis->CurrentBurstIndex)
				projectedDelay = cumulativeDelay + delay;
		}
	}

	if (pThis->IsFiring && pThis->Animation.Value == firingFrame + cumulativeDelay)
	{
		if (pWeaponExt->Burst_FireWithinSequence)
		{
			int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (firingFrame + projectedDelay > frameCount)
			{
				InfantryExt::ExtMap.Find(pThis)->ForceFullRearmDelay = true;
			}
		}

		R->EAX(weaponIndex); // Reuse the weapon index to save some time.
		return Continue;
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x702672, TechnoClass_ReceiveDamage_RevengeWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));

	if (pSource)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		auto const pTypeExt =	TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt && pTypeExt->RevengeWeapon.isset() &&
			EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
		{
			WeaponTypeExt::DetonateAt(pTypeExt->RevengeWeapon.Get(), pSource, pThis);
		}

		for (const auto& weapon : pExt->RevengeWeapons)
		{
			if (EnumFunctions::CanTargetHouse(weapon.ApplyToHouses, pThis->Owner, pSource->Owner))
				WeaponTypeExt::DetonateAt(weapon.Value, pSource, pThis);
		}
	}

	return 0;
}