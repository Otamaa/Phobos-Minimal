#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>
#include <SlaveManagerClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
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

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <New/PhobosAttachedAffect/Functions.h>
#include <New/Entity/FlyingStrings.h>

#include <Misc/SyncLogging.h>

#include <BombClass.h>
#include <SpawnManagerClass.h>

// DEFINE_HOOK(0x448277 , BuildingClass_SetOwningHouse_Additionals , 5)
// {
// 	GET(BuildingClass* const, pThis, ESI);
// 	REF_STACK(bool, announce, STACK_OFFSET(0x58, 0x8));
//
// 	pThis->NextMission();
//
// 	announce = announce && !pThis->Type->IsVehicle();
//
// 	if(announce && (pThis->Type->Powered || pThis->Type->PoweredSpecial))
// 		pThis->UpdatePowerDown();
//
// 	return 0x0;
// }

//DEFINE_HOOK(0x4483C0, BuildingClass_SetOwningHouse_MuteSound, 0x6)
//{
//	GET(BuildingClass* const, pThis, ESI);
//	REF_STACK(bool, announce, STACK_OFFSET(0x60, 0x8));
//
//	return announce ? 0 : 0x44848F; //early bailout
//}

DEFINE_HOOK(0x51AA40, InfantryClass_Assign_Destination_DisallowMoving, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!SyncLogger::HooksDisabled)
		SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	//if (pExt->IsWebbed && pThis->ParalysisTimer.HasTimeLeft())
	//{
	//	if (pThis->Target)
	//	{
	//		pThis->SetTarget(nullptr);
	//		pThis->FootClass::SetDestination(nullptr, false);
	//		pThis->QueueMission(Mission::Sleep, false);
	//	}
	//
	//	return 0x51B1DE;
	//}

	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	TechnoExtData::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

// TunnelLocomotionClass_IsToHaveShadow, skip shadow on all but idle.
// TODO: Investigate if it is possible to fix the shadows not tilting on the burrowing etc. states.
//DEFINE_JUMP(LJMP, 0x72A070, 0x72A07F);

#include <Locomotor/TunnelLocomotionClass.h>

Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
{
	auto tLoco = static_cast<TunnelLocomotionClass*>(iloco);
	tLoco->LocomotionClass::Shadow_Matrix(ret, key);

	if (tLoco->State != TunnelLocomotionClass::State::IDLE)
	{
		double theta = 0.;
		switch (tLoco->State)
		{
		case TunnelLocomotionClass::State::DIGGING:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->Timer.Duration)
				theta *= 1.0 - double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_IN:
			theta = Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::PRE_DIG_OUT:
			theta = -Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::DIGGING_OUT:
			if (key)key->Invalidate();
			theta = -Math::HalfPi;
			if (auto total = tLoco->Timer.Duration)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_OUT:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->Timer.Duration)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		default:break;
		}
		ret->ScaleX((float)Math::cos(theta));// I know it's ugly
	}
	return ret;
}

DEFINE_JUMP(VTABLE, 0x7F5A4C, MiscTools::to_DWORD(&TunnelLocomotionClass_ShadowMatrix));

DEFINE_HOOK(0x708FC0, TechnoClass_ResponseMove_Pickup, 0x5)
{
	enum { SkipResponse = 0x709015 };

	GET(TechnoClass*, pThis, ECX);

	if (auto const pAircraft = cast_to<AircraftClass*, false>(pThis))
	{
		if (pAircraft->Type->Carryall && pAircraft->HasAnyLink() &&
			pAircraft->Destination && (pAircraft->Destination->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
		{

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pAircraft->Type);

			if (!pTypeExt->VoicePickup.empty())
			{

				pThis->QueueVoice(pTypeExt->VoicePickup[Random2Class::NonCriticalRandomNumber->Random() & pTypeExt->VoicePickup.size()]);

				R->EAX(1);
				return SkipResponse;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0x6) //C
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTransport = pThis->Owner->Transporter)
	{
		R->EDX(TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())
			->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
		return 0x71A838;
	}

	return 0;
}

#include <Ext/Super/Body.h>

DEFINE_HOOK(0x6F6CFE, TechnoClass_Unlimbo_LaserTrails, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!pExt->LaserTrails.empty())
	{
		for (auto& pLaserTrail : pExt->LaserTrails)
		{
			pLaserTrail.LastLocation.clear();
			pLaserTrail.Visible = true;
		}
	}

	TrailsManager::Hide(pThis);

	return 0;
}

DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount_UpdateSharedAmmo, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExtData::UpdateSharedAmmo(pThis);

	return 0;
}

DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pType->NoExtraSelfHealOrRepair)
		return SkipGameDrawing;

	// if (const auto pFoot = generic_cast<FootClass*>(pThis)){
	// 	if (const auto pParasiteFoot = pFoot->ParasiteEatingMe) {
	// 		TechnoExtData::DrawParasitedPips(pFoot, pLocation, pBounds);
	// 	}
	// }

	TechnoExtData::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x8) //6
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (pThis)
	{
		auto pType = pThis->GetTechnoType();

		if (TechnoTypeExtContainer::Instance.Find(pType)->UseDisguiseMovementSpeed)
			pType = TechnoExtData::GetSimpleDisguiseType(pThis, false, false);

		maxSpeed = pType->Speed;
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6) //0x8
{
	//Kill slave is buged because it doesnt do IgnoreDamage -Otamaa
	enum { KillTheSlave = 0x6B0BDF, SkipSetEax = 0x6B0BB4, LoopCheck = 0x6B0C0B };

	GET_STACK(const SlaveManagerClass*, pThis, STACK_OFFS(0x24, 0x10));
	GET(InfantryClass*, pSlave, ESI);
	GET(TechnoClass*, pKiller, EBX);
	GET_STACK(HouseClass*, pDefaultRetHouse, STACK_OFFS(0x24, 0x14));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSlave->Type);
	{
		if (pTypeExt->Death_WithMaster.Get() || pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide)
		{
			auto nStr = pSlave->Health;
			pSlave->ReceiveDamage(&nStr, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
			return LoopCheck;
		}

		const auto pVictim = pThis->Owner ? pThis->Owner->GetOwningHouse() : pSlave->GetOwningHouse();
		R->EAX(HouseExtData::GetSlaveHouse(pTypeExt->Slaved_ReturnTo, pKiller->GetOwningHouse(), pVictim ? pVictim : pDefaultRetHouse));
		return SkipSetEax;
	}

	//	return 0x0;
}

#include <Misc/Ares/Hooks/Header.h>

DEFINE_HOOK(0x6FD054, TechnoClass_RearmDelay_ForceFullDelay, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, currentBurstIdx, ECX);

	TechnoExtContainer::Instance.Find(pThis)->LastRearmWasFullDelay = false;

	bool rearm = currentBurstIdx >= pWeapon->Burst;

	// Currently only used with infantry, so a performance saving measure.
	if (const auto pInf = cast_to<InfantryClass*, false>(pThis))
	{
		const auto pInfExt = InfantryExtContainer::Instance.Find(pInf);
		if (pInfExt->ForceFullRearmDelay)
		{
			pInfExt->ForceFullRearmDelay = false;
			pThis->CurrentBurstIndex = 0;
			rearm = true;
		}
	}

	if (!rearm)
	{
		const int burstDelay = WeaponTypeExtData::GetBurstDelay(pWeapon, pThis->CurrentBurstIndex);

		if (burstDelay >= 0)
		{
			R->EAX(burstDelay);
			return 0x6FD099;
		}

		// Restore overridden instructions
		return currentBurstIdx <= 0 || currentBurstIdx > 4 ? 0x6FD084 : 0x6FD067;
	}

	TechnoExtContainer::Instance.Find(pThis)->LastRearmWasFullDelay = true;
	int nResult = 0;
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pExt->ROF_Random.Get())
	{
		const auto nDefault = Point2D { RulesExtData::Instance()->ROF_RandomDelay->X , RulesExtData::Instance()->ROF_RandomDelay->Y };
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pExt->Rof_RandomMinMax.Get(nDefault));
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeaponExt->ROF_RandomDelay.isset())
	{
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->ROF_RandomDelay);
	}

	int _ROF = int(
		(double)pWeapon->ROF *
		TechnoExtContainer::Instance.Find(pThis)->AE.ROFMultiplier *
		pThis->Owner->ROFMultiplier + nResult
	);

	if (pThis->HasAbility(AbilityType::ROF))
		_ROF = int(_ROF * RulesClass::Instance->VeteranROF);

	auto const Building = cast_to<BuildingClass*, false>(pThis);

	if (pThis->CanOccupyFire())
	{
		const auto occupant = pThis->GetOccupantCount();

		if (occupant > 0)
		{
			_ROF /= occupant;
		}

		auto OccupyRofMult = RulesClass::Instance->OccupyROFMultiplier;

		if (Building)
		{
			OccupyRofMult = BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyROFMult.Get(OccupyRofMult);
		}

		if (OccupyRofMult > 0.0)
			_ROF = int(float(_ROF) / OccupyRofMult);

	}

	if (pThis->BunkerLinkedItem && !Building)
	{
		auto BunkerMult = RulesClass::Instance->BunkerROFMultiplier;
		if (auto const pBunkerIsBuilding = cast_to<BuildingClass*, false>(pThis->BunkerLinkedItem))
		{
			BunkerMult = BuildingTypeExtContainer::Instance.Find(pBunkerIsBuilding->Type)->BuildingBunkerROFMult.Get(BunkerMult);
		}

		if (BunkerMult != 0.0)
			_ROF = int(float(_ROF) / BunkerMult);
	}

	R->EAX(_ROF);
	return 0x6FD1F5;
}

namespace FiringAITemp
{
	int weaponIndex;
}

DEFINE_HOOK(0x5206D2, InfantryClass_FiringAI_SetContext, 0x6)
{
	GET(int, weaponIndex, EDI);

	FiringAITemp::weaponIndex = weaponIndex;

	return 0;
}

DEFINE_HOOK(0x5209A7, InfantryClass_FiringAI_BurstDelays, 0x8)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	const auto pWeaponstruct = pThis->GetWeapon(FiringAITemp::weaponIndex);

	if (!pWeaponstruct)
		return ReturnFromFunction;

	const auto pWeapon = pWeaponstruct->WeaponType;

	if (!pWeapon)
		return ReturnFromFunction;

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int cumulativeDelay = 0;
	int projectedDelay = 0;

	// Calculate cumulative burst delay as well cumulative SellSounddelay after next shot (projected delay).
	if (pWeaponExt->Burst_FireWithinSequence)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			int burstDelay = WeaponTypeExtData::GetBurstDelay(pWeapon, i);
			int delay = 0;

			if (burstDelay > -1)
				delay = burstDelay;
			else
				delay = ScenarioClass::Instance->Random.RandomRanged(3, 5);

			// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
			if (i != 0)
				delay = MaxImpl(delay, 1);

			cumulativeDelay += delay;

			if (i == pThis->CurrentBurstIndex)
				projectedDelay = cumulativeDelay + delay;
		}
	}

	//IsFiring
	if (R->AL() && pThis->Animation.Value == firingFrame + cumulativeDelay)
	{
		if (pWeaponExt->Burst_FireWithinSequence)
		{
			int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (firingFrame + projectedDelay > frameCount)
			{
				InfantryExtContainer::Instance.Find(pThis)->ForceFullRearmDelay = true;
			}
		}

		R->EAX(FiringAITemp::weaponIndex); // Reuse the weapon index to save some time.
		return Continue;
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x702672, TechnoClass_ReceiveDamage_RevengeWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFSET(0xC4, 0xC));

	if (pSource)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
		auto SourCoords = pSource->GetCoords();

		if (!pWHExt->SuppressRevengeWeapons)
		{
			if (pTypeExt->RevengeWeapon &&
				EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner) &&
				!pWHExt->SuppressRevengeWeapons_Types.empty() && !pWHExt->SuppressRevengeWeapons_Types.Contains(pTypeExt->RevengeWeapon))
			{
				WeaponTypeExtData::DetonateAt(pTypeExt->RevengeWeapon.Get(), pSource, pThis, true, nullptr);
			}

			for (const auto& weapon : pExt->RevengeWeapons)
			{
				if (EnumFunctions::CanTargetHouse(weapon.ApplyToHouses, pThis->Owner, pSource->Owner) && !pWHExt->SuppressRevengeWeapons_Types.empty() && !pWHExt->SuppressRevengeWeapons_Types.Contains(weapon.Value))
					WeaponTypeExtData::DetonateAt(weapon.Value, pSource, pThis, true, nullptr);
			}
		}

		PhobosAEFunctions::ApplyRevengeWeapon(pThis, pSource, pWH);
	}

	if (pThis->AttachedBomb)
		pThis->AttachedBomb->Detonate();

	return 0x702684;
}

DEFINE_HOOK(0x702603, TechnoClass_ReceiveDamage_Explodes, 0x6)
{
	enum { SkipExploding = 0x702672, SkipKillingPassengers = 0x702669 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (!BuildingTypeExtContainer::Instance.Find(((BuildingClass*)pThis)->Type)->Explodes_DuringBuildup && (pThis->CurrentMission == Mission::Construction || pThis->CurrentMission == Mission::Selling))
			return SkipExploding;
	}

	return !TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->Explodes_KillPassengers ? SkipKillingPassengers : 0x0;
}

// TODO :
// yeah , fuckers !!
  //struct VampireState {
	// struct Data {
	//   bool Enabled;
	//   bool AffectAir;
	//   AffectedHouse Affected_House;
	//   AbstractType Affected_abs;
	//   HelperedVector<TechnoTypeClass*> Affected_Types;
	//   HelperedVector<TechnoTypeClass*> Exclude_Types;
	//   Vector2D<double> Chances;
	//   double Percent;
	//   int TriggeredTimes;
	// };

	// HelperedVector<Data> Packed {};

	// consteval void Clear() {
	//	 Packed.clear();
	// }

	// constexpr bool Enabled() {
	//	 return !Packed.empty();
	// }

	// //only add the data that can affect this current techno
	// void Init()
	// {

	// }

	// //update trigger count
	// void Trigger() {
	//	 for (auto& data : Packed) {
	//		 if (data.Enabled && data.TriggeredTimes > 0) {
	//			 --data.TriggeredTimes;

	//			 if (data.TriggeredTimes <= 0)
	//				 data.Enabled = false;
	//		 }
	//	 }
	// }

	// // apply the multiplier to the attacker ??
	// void Apply() {

	// }
	// constexpr bool Eligible(TechnoClass* attacker, HouseClass* attackerOwner , bool isInAir) {




	//	 return true;
	// }
 //};

// void NOINLINE ApplyVampire(int* pRealDamage, WarheadTypeClass* pWH, DamageState damageState, TechnoClass* pAttacker, HouseClass* pAttackingHouse)
// {
// 	if(!pAttacker || !pAttacker->IsAlive || pAttacker->IsCrashing || pAttacker->IsSinking || pAttacker->TemporalTargetingMe)
// 		return;

// 	bool inAir = pTechno->IsInAir();

// 	vampireEffect->Trigger();
// 	int damage = -(int)(*pRealDamage * ae->AEData.Vampire.Percent);
// 	if (damage != 0) {
// 		pAttacker->TakeDamage(damage, pAttacker->GetTechnoType()->Crewed, true, pAttacker, pAttackingHouse);
// 	}
// }

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_AfterObjectClassCall, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);
	GET(WarheadTypeClass*, pWH, EBP);
	//GET_STACK(TechnoClass*, pAttacker, 0xD4);
	//GET_STACK(HouseClass*, pAttackingHouse, 0xE0);

	const bool Show = Phobos::Otamaa::IsAdmin || *pDamage;

	if (Phobos::Debug_DisplayDamageNumbers && Show)
		FlyingStrings::DisplayDamageNumberString(*pDamage, DamageDisplayType::Regular, pThis->GetRenderCoords(), TechnoExtContainer::Instance.Find(pThis)->DamageNumberOffset);

	GET(DamageState, damageState, EDI);

	GiftBoxFunctional::TakeDamage(TechnoExtContainer::Instance.Find(pThis), TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType()), pWH, damageState);

	if (damageState != DamageState::PostMortem && !pThis->IsAlive)
	{
		R->EAX(DamageState::NowDead);
		return 0x702688;
	}

	return 0;
}

DEFINE_HOOK(0x4D9992, FootClass_PointerGotInvalid_Parasite, 0x7)
{
	enum { SkipGameCode = 0x4D99D3 };

	GET(FootClass*, pThis, ESI);
	GET(AbstractClass*, pAbstract, EDI);
	GET(FootClass*, pParasiteOwner, EAX);
	GET(bool, bRemoved, EBX);

	if (pParasiteOwner == pAbstract && (!pParasiteOwner->Health || !Game::InScenario2()))
	{
		pThis->ParasiteEatingMe = nullptr;
		return SkipGameCode;
	}

	if (!bRemoved)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		bRemoved = pTypeExt->Cloak_KickOutParasite.Get(RulesExtData::Instance()->Cloak_KickOutParasite);
	}

	if (pParasiteOwner && pParasiteOwner->Health > 0)
		pParasiteOwner->ParasiteImUsing->PointerExpired(pAbstract, bRemoved);

	if (pThis == pAbstract && bRemoved)
		pThis->ParasiteEatingMe = nullptr;

	return SkipGameCode;
}

DEFINE_HOOK(0x709B2E, TechnoClass_DrawPips_Sizes, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(int, pipWidth, STACK_OFFSET(0x74, -0x1C));

	const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;

	Point2D size = Point2D::Empty;
	const auto pType = pThis->GetTechnoType();

	if (pType->PipScale == PipScale::Ammo)
	{
		size = TechnoTypeExtContainer::Instance.Find(pType)->AmmoPipSize.Get((isBuilding ?
			RulesExtData::Instance()->Pips_Ammo_Buildings_Size : RulesExtData::Instance()->Pips_Ammo_Size));
	}
	else
	{
		size = (isBuilding ? RulesExtData::Instance()->Pips_Generic_Buildings_Size : RulesExtData::Instance()->Pips_Generic_Size).Get();
	}

	pipWidth = size.X;
	R->ESI(size.Y);

	return 0;
}

DEFINE_HOOK(0x70A36E, TechnoClass_DrawPips_Ammo, 0x6)
{
	enum { SkipGameDrawing = 0x70A4EC };

	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET(int, pipWrap, EBX);
	GET_STACK(int, pipCount, STACK_OFFSET(0x74, -0x54));
	GET_STACK(int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(int, yOffset, ESI);
	GET_STACK(SHPStruct*, pDefault, 0x74 - 0x48);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	Point2D position = { offset->X + pTypeExt->AmmoPipOffset->X, offset->Y + pTypeExt->AmmoPipOffset->Y };
	ConvertClass* pConvert = pTypeExt->AmmoPip_Palette ?
		pTypeExt->AmmoPip_Palette->GetOrDefaultConvert<PaletteManager::Mode::Default>(FileSystem::PALETTE_PAL)
		: FileSystem::PALETTE_PAL();

	auto pSHApe = pTypeExt->AmmoPip_shape.Get(pDefault);

	if (pipWrap > 0)
	{
		int levels = maxPips / pipWrap - 1;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = pTypeExt->PipWrapAmmoPip;

			if (levels >= 0)
			{
				int counter = i + pipWrap * levels;
				int frameCounter = levels;
				bool calculateFrame = true;

				while (counter >= pThis->Ammo)
				{
					frameCounter--;
					counter -= pipWrap;

					if (frameCounter < 0)
					{
						calculateFrame = false;
						break;
					}
				}

				if (calculateFrame)
					frame = frameCounter + frame + 1;
			}

			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
	else
	{
		int ammoFrame = pTypeExt->AmmoPip;
		int emptyFrame = pTypeExt->EmptyAmmoPip;

		for (int i = 0; i < maxPips; i++)
		{
			if (i >= pipCount && emptyFrame < 0)
				break;

			int frame = i >= pipCount ? emptyFrame : ammoFrame;
			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	return SkipGameDrawing;
}

DEFINE_HOOK_AGAIN(0x5F4718, ObjectClass_Select, 0x7)
DEFINE_HOOK(0x5F46AE, ObjectClass_Select, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if (RulesExtData::Instance()->SelectFlashTimer > 0 && pThis->GetOwningHouse() && pThis->GetOwningHouse()->ControlledByCurrentPlayer())
		pThis->Flash(RulesExtData::Instance()->SelectFlashTimer);

	return 0x0;
}

#include <EventClass.h>

// Do not explicitly reset target for KeepTargetOnMove vehicles when issued move command.
DEFINE_HOOK(0x4C7462, EventClass_Execute_KeepTargetOnMove, 0x5)
{
	enum { SkipGameCode = 0x4C74C0 };

	GET(EventClass*, pThis, ESI);
	GET(TechnoClass*, pTechno, EDI);
	GET(AbstractClass*, pTarget, EBX);

	if (pTechno->WhatAmI() != AbstractType::Unit)
		return 0;

	auto const mission = static_cast<Mission>(pThis->Data.MegaMission.Mission);

	if ((mission == Mission::Move)
		&& TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType())->KeepTargetOnMove
		&& pTechno->Target && !pTarget)
	{
		pTechno->SetDestination(pThis->Data.MegaMission.Destination.As_Abstract(), true);
		return SkipGameCode;
	}

	return 0;
}

// Reset the target if beyond weapon range.
// This was originally in UnitClass::Mission_Move() but because that
// is only checked every ~15 frames, it can cause responsiveness issues.
DEFINE_HOOK(0x736480, UnitClass_AI_KeepTargetOnMove, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->KeepTargetOnMove && pThis->Target && pThis->CurrentMission == Mission::Move)
	{
		if (pTypeExt->KeepTargetOnMove_ExtraDistance.isset())
		{
			int weaponIndex = pThis->SelectWeapon(pThis->Target);

			if (auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
			{
				auto pExt = TechnoExtContainer::Instance.Find(pThis);
				pExt->AdditionalRange = static_cast<int>(pTypeExt->KeepTargetOnMove_ExtraDistance.Get());

				if (!pThis->IsCloseEnough(pThis->Target, weaponIndex))
					pThis->SetTarget(nullptr);

				pExt->AdditionalRange.clear();
			}
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B769F, SpawnManagerClass_AI_InitDestination, 0x7)
DEFINE_HOOK(0x6B7600, SpawnManagerClass_AI_InitDestination, 0x6)
{
	enum { SkipGameCode1 = 0x6B760E, SkipGameCode2 = 0x6B76DE };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(AircraftClass* const, pSpawnee, EDI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());

	if (pTypeExt->Spawner_AttackImmediately)
	{
		pSpawnee->SetTarget(pThis->Target);
		pSpawnee->QueueMission(Mission::Attack, true);
		pSpawnee->IsReturningFromAttackRun = false;
	}
	else
	{
		auto const mapCoords = pThis->Owner->GetMapCoords();
		auto const pCell = MapClass::Instance->GetCellAt(mapCoords);
		pSpawnee->SetDestination(pCell->GetNeighbourCell(FacingType::North), true);
		pSpawnee->QueueMission(Mission::Move, false);
	}

	return R->Origin() == 0x6B7600 ? SkipGameCode1 : SkipGameCode2;
}

void DrawFactoryProgress(TechnoClass* pThis, RectangleStruct* pBounds)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	if (!RulesExtData::Instance()->FactoryProgressDisplay)
		return;

	if (!HouseClass::IsCurrentPlayerObserver())
		return;

	BuildingClass* const pBuilding = static_cast<BuildingClass*>(pThis);

	if(pBuilding->Type->InvisibleInGame || pBuilding->Type->Invisible )
		return;

	CellClass* pCell = pBuilding->GetCell();
	BuildingTypeClass* const pBuildingType = pBuilding->Type;
	HouseClass* const pHouse = pBuilding->Owner;
	FactoryClass* pPrimaryFactory = nullptr;
	FactoryClass* pSecondaryFactory = nullptr;

	if (pHouse->IsControlledByHuman())
	{
		if (!pBuilding->IsPrimaryFactory)
			return;

		switch (pBuilding->Type->Factory)
		{
		case AbstractType::BuildingType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare);
			pSecondaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat);
			break;
		case AbstractType::InfantryType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::InfantryType, false, BuildCat::Combat);
			break;
		case AbstractType::UnitType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::UnitType, pBuildingType->Naval, BuildCat::Combat);
			break;
		case AbstractType::AircraftType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::Combat);
			break;
		default:
			return;
		}
	}
	else // AIs have no Primary factories
	{
		pPrimaryFactory = pBuilding->Factory;

		if (!pPrimaryFactory)
			return;
	}

	const bool havePrimary = pPrimaryFactory && pPrimaryFactory->Object;
	const bool haveSecondary = pSecondaryFactory && pSecondaryFactory->Object;

	if (!havePrimary && !haveSecondary)
		return;

	const int maxLength = pBuildingType->GetFoundationHeight(false) * 15 >> 1;
	const Point2D location = TechnoExtData::GetBuildingSelectBracketPosition(pBuilding, BuildingSelectBracketPosition::Top) + Point2D { 5, 3 };

	if (havePrimary)
	{
		const int curLength = std::clamp(static_cast<int>((static_cast<double>(pPrimaryFactory->GetProgress()) / 54) * maxLength), 0, maxLength);
		Point2D position = location;

		for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 3, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	if (haveSecondary)
	{
		const int curLength = std::clamp(static_cast<int>((static_cast<double>(pSecondaryFactory->GetProgress()) / 54) * maxLength), 0, maxLength);
		Point2D position = havePrimary ? location + Point2D { 6, 3 } : location;

		for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 3, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void DrawSuperProgress(TechnoClass* pThis, RectangleStruct* pBounds)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	if (!RulesExtData::Instance()->FactoryProgressDisplay)
		return;

	if (!HouseClass::IsCurrentPlayerObserver())
		return;

	BuildingClass* const pBuilding = static_cast<BuildingClass*>(pThis);

	if(pBuilding->Type->InvisibleInGame || pBuilding->Type->Invisible )
		return;

	CellClass* pCell = pBuilding->GetCell();
	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (pBuildingType->SuperWeapon == -1)
		return;

	SuperClass* const pSuper = pThis->Owner->Supers.Items[pBuildingType->SuperWeapon];

	if (!pSuper || pSuper->RechargeTimer.TimeLeft <= 1)
		return;

	const int maxLength = pBuildingType->GetFoundationHeight(false) * 15 >> 1;
	const int curLength = std::clamp(static_cast<int>((static_cast<double>(pSuper->RechargeTimer.TimeLeft - pSuper->RechargeTimer.GetTimeLeft()) / pSuper->RechargeTimer.TimeLeft) * maxLength), 0, maxLength);
	Point2D position = TechnoExtData::GetBuildingSelectBracketPosition(pBuilding, BuildingSelectBracketPosition::Top) + Point2D { 5, 3 };

	for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 5, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

DEFINE_HOOK(0x6F5EE3, TechnoClass_DrawExtras_DrawAboveHealth, 0x9)
{
	GET(TechnoClass*, pThis, EBP);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x98, 0x8));

	DrawFactoryProgress(pThis, pBounds);
	DrawSuperProgress(pThis, pBounds);

	return 0;
}

// DEFINE_HOOK(0x6B77B4, SpawnManagerClass_Update_RecycleSpawned, 0x7)
// {
// 	//enum { RecycleIsOk = 0x6B77FF, RecycleIsNotOk = 0x6B7838 };
//
// 	GET(SpawnManagerClass* const, pThis, ESI);
// 	GET(TechnoClass* const, pSpawned, EDI);
// 	GET(CellStruct* const, pSpawnerMapCrd, EBP);
// 	GET(CellStruct* const , pSpawnedMapCrd, EAX);
//
// 	const auto pSpawner = pThis->Owner;
// 	const auto pSpawnerType = pSpawner->GetTechnoType();
// 	const auto pSpawnerExt = TechnoTypeExtContainer::Instance.Find(pSpawnerType);
// 	const auto SpawnerCrd = pSpawner->Location;
// 	const auto SpawnedCrd = pSpawned->Location;
// 	const auto DeltaCrd = SpawnedCrd - SpawnerCrd;
// 	const int RecycleRange = pSpawnerExt->Spawner_RecycleRange;
//
// 	const auto what = pSpawner->WhatAmI();
// 	const bool bShouldRecycleSpawned = (RecycleRange == -1 && (what == AbstractType::Building && DeltaCrd.X <= 182 && DeltaCrd.Y <= 182 && DeltaCrd.Z < 20 ||
// 			what != AbstractType::Building && pSpawnedMapCrd->X == pSpawnerMapCrd->X && pSpawnedMapCrd->Y == pSpawnerMapCrd->Y && DeltaCrd.Z < 20)) ||
// 			Math::sqrt(DeltaCrd.X * DeltaCrd.X + DeltaCrd.Y * DeltaCrd.Y + DeltaCrd.Z * DeltaCrd.Z) <= RecycleRange;
//
// 	if (bShouldRecycleSpawned) {
//
// 		if (auto pAnim = pSpawnerExt->Spawner_RecycleAnim) {
// 			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, SpawnedCrd),
// 				pSpawner->GetOwningHouse(),
// 				nullptr,
// 				pSpawner,
// 				false
// 			);
// 		}
//
// 		pSpawned->SetLocation(SpawnerCrd);
// 		R->EAX(pSpawnerMapCrd);
// 	}
//
// 	return 0;
// }

DEFINE_HOOK(0x6FA540, TechnoClass_AI_ChargeTurret, 0x6)
{
	enum { SkipGameCode = 0x6FA5BE };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->ROF <= 0)
	{
		pThis->CurrentTurretNumber = 0;
		return SkipGameCode;
	}

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	int timeLeft = pThis->DiskLaserTimer.GetTimeLeft();

	if (pExt->ChargeTurretTimer.HasStarted())
		timeLeft = pExt->ChargeTurretTimer.GetTimeLeft();
	else if (pExt->ChargeTurretTimer.Expired())
		pExt->ChargeTurretTimer.Stop();

	int turretCount = pType->TurretCount;
	int turretIndex = std::max(0, timeLeft * turretCount / pThis->ROF);

	if (turretIndex >= turretCount)
		turretIndex = turretCount - 1;

	pThis->CurrentTurretNumber = turretIndex;
	return SkipGameCode;
}

//DEFINE_HOOK(0x5F4032, ObjectClass_FallingDown_ToDead, 0x6)
//{
//	GET(ObjectClass*, pThis, ESI);
//
//	if (auto const pTechno = abstract_cast<TechnoClass*>(pThis))
//	{
//		auto pCell = pTechno->GetCell();
//		auto pType = pTechno->GetTechnoType();
//		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
//
//		if (!pCell || !pCell->IsClearToMove(pType->SpeedType, true, true, ZoneType::None, pType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
//			return 0;
//
//
//		double ratio = pCell->Tile_Is_Water() && !pTechno->OnBridge ?
//				pTypeExt->FallingDownDamage_Water.Get(pTypeExt->FallingDownDamage.Get())
//				: pTypeExt->FallingDownDamage.Get();
//
//		int damage = 0;
//
//		if (ratio < 0.0)
//			damage = int(pThis->Health * abs(ratio));
//		else if (ratio >= 0.0 && ratio <= 1.0)
//			damage = int(pThis->GetTechnoType()->Strength * ratio);
//		else
//			damage = int(ratio);
//
//		pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
//
//		if (pThis->Health > 0 && pThis->IsAlive)
//		{
//			pThis->IsABomb = false;
//
//			if (pThis->WhatAmI() == AbstractType::Infantry)
//			{
//				auto pInf = static_cast<InfantryClass*>(pTechno);
//				const bool isWater = pCell->Tile_Is_Water();
//
//				if (isWater && pInf->SequenceAnim != DoType::Swim)
//					pInf->PlayAnim(DoType::Swim, true, false);
//				else if (!isWater && pInf->SequenceAnim != DoType::Guard)
//					pInf->PlayAnim(DoType::Guard, true, false);
//			}
//		} else {
//			pTechno->UpdatePosition((int)PCPType::During);
//		}
//
//		return 0x5F405B;
//	}
//
//	return 0;
//}