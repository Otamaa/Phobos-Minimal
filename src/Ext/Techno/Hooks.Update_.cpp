#include "Body.h"

#include <InfantryClass.h>
#include <AircraftClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>

#include <BombClass.h>
#include <SlaveManagerClass.h>
#include <CaptureManagerClass.h>
#include <SpawnManagerClass.h>
#include <TacticalClass.h>

#include <ExtraHeaders/StackVector.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Misc/Ares/Hooks/Header.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

#define ENABLE_THESE

ASMJIT_PATCH(0x6FA68B, TechnoClass_Update_AttackMovePaused, 0xA) // To make aircrafts not search for targets while resting at the airport, this is designed to adapt to loop waypoint
{
	enum { SkipGameCode = 0x6FA6F5 };

	GET(TechnoClass* const, pThis, ESI);

	const bool skip = RulesExtData::Instance()->ExpandAircraftMission
		&& pThis->WhatAmI() == AbstractType::Aircraft
		&& (!pThis->Ammo || !pThis->IsInAir());

	return skip ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x6FA67D, TechnoClass_Update_DistributeTargetingFrame, 0xA)
{
	enum { Targeting = 0x6FA687, SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);

	auto const pRulesExt = RulesExtData::Instance();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if ((!pThis->Owner->IsControlledByHuman() || !pRulesExt->DistributeTargetingFrame_AIOnly) && pTypeExt->DistributeTargetingFrame.Get(pRulesExt->DistributeTargetingFrame))
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (Unsorted::CurrentFrame % 16 != pExt->MyTargetingFrame)
		{
			return SkipTargeting;
		}
	}

	if (pThis->MegaMissionIsAttackMove())
	{
		if (!RulesExtData::Instance()->ExpandAircraftMission && pThis->WhatAmI() == AbstractType::Aircraft && (!pThis->Ammo || pThis->GetHeight() < Unsorted::CellHeight))
			return SkipTargeting;

		pThis->UpdateAttackMove();
		return SkipTargeting;
	}

	if (auto pInf = cast_to<InfantryClass*, false>(pThis))
	{
		if (pInf->Type->Slaved && pInf->SlaveOwner)
		{
			return SkipTargeting;
		}
	}

	if (!pThis->IsArmed())
		return SkipTargeting;

	return 0x6FA697;
}

ASMJIT_PATCH(0x6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	return (pThis->WhatAmI() == AbstractType::Aircraft) ? 0x6FA4D1 : 0;
}

ASMJIT_PATCH(0x6FAF0D, TechnoClass_Update_EMPLock, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// original code.
	const auto was = pThis->EMPLockRemaining;
	if (was > 0)
	{
		pThis->EMPLockRemaining = was - 1;
		if (was == 1)
		{
			// the forced vacation just ended. we use our own
			// function here that is quicker in retrieving the
			// EMP animation and does more stuff.
			AresEMPulse::DisableEMPEffect(pThis);
		}
		else
		{
			// deactivate units that were unloading afterwards
			if (!pThis->Deactivated && AresEMPulse::IsDeactivationAdvisable(pThis))
			{
				// update the current mission
				TechnoExtContainer::Instance.Find(pThis)->EMPLastMission = pThis->CurrentMission;
				pThis->Deactivate();
			}
		}
	}

	return 0x6FAFFD;
}

// this code somewhat broke targeting
// it created identically like ares but not working as expected , duh
ASMJIT_PATCH(0x6FA361, TechnoClass_Update_LoseTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pHouse, EDI);

	enum { ForceAttack = 0x6FA472, ContinueCheck = 0x6FA39D };

	const bool BLRes = R->BL();
	const HouseClass* pOwner = !BLRes ? pThis->Owner : pHouse;

	bool IsAlly = false;

	if (const auto pTechTarget = flag_cast_to<ObjectClass*>(pThis->Target))
	{
		if (const auto pTargetHouse = pTechTarget->GetOwningHouse())
		{
			if (pOwner->IsAlliedWith(pTargetHouse))
			{
				IsAlly = true;
			}
		}
	}

	auto pType = pThis->GetTechnoType();

	if (!pThis->Berzerk && pType->AttackFriendlies && IsAlly && TechnoTypeExtContainer::Instance.Find(pType)->AttackFriendlies_AutoAttack)
	{
		return ForceAttack;
	}

	//if(pThis->Berzerk && IsAlly) {
	//	return ForceAttack; // dont clear target
	//}

	const bool IsNegDamage = (pThis->CombatDamage() < 0);

	return IsAlly == IsNegDamage ? ForceAttack : ContinueCheck;
}

ASMJIT_PATCH(0x6FA054, TechnoClass_Update_Veterancy, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExperienceData::PromoteImmedietely(pThis, false, true);
	return 0x6FA14B;
}

ASMJIT_PATCH(0x6FA726, TechnoClass_AI_MCOverload, 0x6)
{
	enum
	{
		SelfHeal = 0x6FA743, //continue ares check here
		DoNotSelfHeal = 0x6FA941,
		ReturnFunc = 0x6FAFFD,
		ContineCheckUpdateSelfHeal = 0x6FA75A,
		Continue = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	TechnoExtData::UpdateMCOverloadDamage(pThis);

	if (!pThis->IsAlive)
		return ReturnFunc;

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	// prevent crashing and sinking technos from self-healing
	if (pType->NoExtraSelfHealOrRepair 
		|| pThis->InLimbo 
		|| pThis->IsCrashing
		|| pThis->IsSinking 
		|| TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled)
	{
		return DoNotSelfHeal;
	}

	const auto nUnit = cast_to<UnitClass*, false>(pThis);
	if (nUnit && nUnit->DeathFrameCounter > 0)
	{
		return DoNotSelfHeal;
	}

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	const auto nAmount = TechnoExt_ExtData::GetSelfHealAmount(pThis);
	bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
	if (nAmount > 0 || nAmount != 0)
	{
		pThis->Health += nAmount;
	}

	//this one take care of the visual stuffs
	//if the techno health back to normal
	TechnoExtData::ApplyGainedSelfHeal(pThis, wasDamaged);

	//handle everything
	return !pThis->IsAlive ? ReturnFunc : DoNotSelfHeal;
}

ASMJIT_PATCH(0x6FA540, TechnoClass_AI_ChargeTurret, 0x6)
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
	int timeLeft = pThis->RearmTimer.GetTimeLeft();

	if (pExt->ChargeTurretTimer.HasStarted())
		timeLeft = pExt->ChargeTurretTimer.GetTimeLeft();
	else if (pExt->ChargeTurretTimer.Expired())
		pExt->ChargeTurretTimer.Stop();

	int turretCount = pType->TurretCount;
	int turretIndex = MaxImpl(0, timeLeft * turretCount / pThis->ROF);

	if (turretIndex >= turretCount)
		turretIndex = turretCount - 1;

	pThis->CurrentTurretNumber = turretIndex;
	return SkipGameCode;
}

ASMJIT_PATCH(0x6FABC4, TechnoClass_AI_AnimationPaused, 0x6)
{
	enum { SkipGameCode = 0x6FAC31 };

	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->DelayedFireSequencePaused)
		return SkipGameCode;

	return 0;
}

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	return TechnoTypeExtContainer::Instance.Find(pThis->Transporter->GetTechnoType())->Passengers_SyncOwner.Get() ?  ret : 0; }

SET_THREATEVALS(0x6FA33C, ESI, TechnoClass_AI_ThreatEvals_OpenToppedOwner, 0x6, 0x6FA37A)

#undef SET_THREATEVALS


ASMJIT_PATCH(0x6F9E5B, TechnoClass_AI_Early, 0x6)
{
	enum { retDead = 0x6FAFFD, Continue = 0x6F9EBB };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->IsMouseHovering)
		pThis->IsMouseHovering = false;

	if (!pThis->IsAlive)
		return retDead;

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto IsBuilding = pThis->WhatAmI() == BuildingClass::AbsID;

	TechnoExt_ExtData::Ares_technoUpdate(pThis);

	if (!pThis->IsAlive)
		return retDead;

	if (!IsBuilding)
	{
		pExt->UpdateLaserTrails();
		TrailsManager::AI((FootClass*)pThis);
	}

	HugeBar::InitializeHugeBar(pThis);

	PhobosAEFunctions::UpdateAttachEffects(pThis);

	if (!pThis->IsAlive)
		return retDead;

	//type may already change ,..
	auto const pType = pThis->GetTechnoType();

	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	bool IsInLimboDelivered = false;

	if (IsBuilding)
	{
		IsInLimboDelivered = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LimboID >= 0;
	}

#ifdef ENABLE_THESE
	if (pThis->IsAlive && pThis->Location == CoordStruct::Empty || pThis->InlineMapCoords() == CellStruct::Empty)
	{
		if (!pType->Spawned && !IsInLimboDelivered && !pThis->InLimbo)
		{
			Debug::LogInfo("Techno[{} : {}] With Invalid Location ! , Removing ! ", (void*)pThis, pThis->get_ID());
			TechnoExtData::HandleRemove(pThis, nullptr, false, false);
			return retDead;
		}
	}
#endif

	// Update tunnel state on exit, TechnoClass::AI is only called when not in tunnel.
	auto pState = pExt;

	if (pState->IsInTunnel)
	{
		pState->IsInTunnel = false;

		if (auto pShieldData = pExt->GetShield())
			pShieldData->SetAnimationVisibility(true);
	}

#ifdef ENABLE_THESE
	if (pExt->UpdateKillSelf_Slave())
	{
		return retDead;
	}

	if (pExt->CheckDeathConditions())
	{
		return retDead;
	}

	pExt->UpdateBuildingLightning();
	pExt->UpdateShield();
	if (!pThis->IsAlive)
	{
		return retDead;
	}
	pExt->UpdateInterceptor();

	//pExt->UpdateFireSelf();
	pExt->UpdateTiberiumEater();
	pExt->UpdateMCRangeLimit();
	pExt->UpdateRecountBurst();
	pExt->UpdateRearmInEMPState();

	if (pExt->AttackMoveFollowerTempCount)
	{
		pExt->AttackMoveFollowerTempCount--;
	}

	pExt->UpdateSpawnLimitRange();
	pExt->UpdateEatPassengers();
	if (!pThis->IsAlive)
	{
		return retDead;
	}
	pExt->UpdateGattlingOverloadDamage();
	if (!pThis->IsAlive)
	{
		return retDead;
	}

	//TODO : improve this to better handle delay anims !
	//pExt->UpdateDelayFireAnim();

	pExt->UpdateRevengeWeapons();
	if (!pThis->IsAlive)
	{
		return retDead;
	}

	pExt->DepletedAmmoActions();

#endif

	if (pType->IsGattling)
	{
		VocClass::PlayIfInRange(pThis->Location, &pThis->Audio4);
	}

	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();

	PassengersFunctional::AI(pThis);
	if (!pThis->IsAlive)
	{
		return retDead;
	}

	SpawnSupportFunctional::AI(pThis);

	if (!pThis->IsAlive)
	{
		return retDead;
	}

	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);

	if (!pThis->IsAlive)
	{
		return retDead;
	}

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	GiftBoxFunctional::AI(pExt, pTypeExt);

	if (!pThis->IsAlive)
	{
		return retDead;
	}

	pExt->PaintBallStates.erase_all_if([pThis, IsBuilding](auto& pb)
 {
	 if (pb.second.timer.GetTimeLeft())
	 {
		 if (IsBuilding)
		 {
			 BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LighningNeedUpdate = true;
		 }
		 return false;
	 }

	 return true;
	});

	if (auto& pDSState = pExt->DamageSelfState) {
		pDSState->TechnoClass_Update_DamageSelf(pThis);
	}

	if (!pThis->IsAlive)
	{
		return retDead;
	}

	return Continue;
}

ASMJIT_PATCH(0x6FA2CF, TechnoClass_AI_DrawBehindAnim, 0x9) //was 4
{
	GET(TechnoClass*, pThis, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(RectangleStruct*, pBound, EAX);

	if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
	{
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
		{
			return 0x6FA30C;
		}
	}

	if (pThis->InOpenToppedTransport)
		return 0x6FA30C;

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pType->IsDummy)
		return 0x6FA30C;

	pThis->DrawBehindMark(pPoint, pBound);

	return 0x6FA30C;
}

ASMJIT_PATCH(0x6FA4E5, TechnoClass_AI_RecoilUpdate, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return !pThis->InLimbo ? 0x0 : 0x6FA4FB;
}

ASMJIT_PATCH(0x6FA232, TechnoClass_AI_LimboSkipRocking, 0xA)
{
	return !R->ESI<TechnoClass* const>()->InLimbo ? 0x0 : 0x6FA24A;
}

ASMJIT_PATCH(0x6F9F42, TechnoClass_AI_Berzerk_SetMissionAfterDone, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::SetMissionAfterBerzerk(pThis);
	return 0x6F9F6E;
}

ASMJIT_PATCH(0x6FA167, TechnoClass_AI_DrainMoney, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::ApplyDrainMoney(pThis);
	return 0x6FA1C5;
}

// make damage sparks customizable, using game setting as default.
ASMJIT_PATCH(0x6FACD9, TechnoClass_AI_DamageSparks, 6)
{
	GET(TechnoClass*, pThis, ESI);

	if (!pThis->Sys.Spark)
	{
		auto _HPRatio = pThis->GetHealthPercentage();

		if (!(_HPRatio >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10))
		{

			auto pType = pThis->GetTechnoType();
			const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pExt->DamageSparks.Get(pType->DamageSparks))
			{

				StackVector<ParticleSystemTypeClass*, 0x25> Systems {};

				if (auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems))
				{
					auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

					for (auto pSystem : it)
					{
						if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Spark)
						{
							Systems->push_back(pSystem);
						}
					}
				}

				if (!Systems->empty())
				{

					const double _probability = _HPRatio >= RulesClass::Instance->ConditionRed ?
						RulesClass::Instance->ConditionYellowSparkingProbability : RulesClass::Instance->ConditionRedSparkingProbability;
					const auto _rand = ScenarioClass::Instance->Random.RandomDouble();

					if (_rand < _probability)
					{
						CoordStruct _offs = pThis->Location + pType->GetParticleSysOffset();
						pThis->Sys.Spark =
							GameCreate<ParticleSystemClass>(Systems[ScenarioClass::Instance->Random.RandomFromMax(Systems->size() - 1)], _offs, nullptr, pThis);
					}
				}
			}
		}
	}

	return 0x6FAF01;
}

ASMJIT_PATCH(0x6FA2CF, TechnoClass_AI_DrawBehindAnim, 0x9) //was 4
{
	GET(TechnoClass*, pThis, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(RectangleStruct*, pBound, EAX);

	if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
	{
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
		{
			return 0x6FA30C;
		}
	}

	if (pThis->InOpenToppedTransport)
		return 0x6FA30C;

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pType->IsDummy)
		return 0x6FA30C;

	pThis->DrawBehindMark(pPoint, pBound);

	return 0x6FA30C;
}

ASMJIT_PATCH(0x6FA4E5, TechnoClass_AI_RecoilUpdate, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return !pThis->InLimbo ? 0x0 : 0x6FA4FB;
}

ASMJIT_PATCH(0x6FA232, TechnoClass_AI_LimboSkipRocking, 0xA)
{
	return !R->ESI<TechnoClass* const>()->InLimbo ? 0x0 : 0x6FA24A;
}

ASMJIT_PATCH(0x6F9F42, TechnoClass_AI_Berzerk_SetMissionAfterDone, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::SetMissionAfterBerzerk(pThis);
	return 0x6F9F6E;
}

ASMJIT_PATCH(0x6FA167, TechnoClass_AI_DrainMoney, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::ApplyDrainMoney(pThis);
	return 0x6FA1C5;
}

// make damage sparks customizable, using game setting as default.
ASMJIT_PATCH(0x6FACD9, TechnoClass_AI_DamageSparks, 6)
{
	GET(TechnoClass*, pThis, ESI);

	if (!pThis->Sys.Spark)
	{
		auto _HPRatio = pThis->GetHealthPercentage();

		if (!(_HPRatio >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10))
		{

			auto pType = pThis->GetTechnoType();
			const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pExt->DamageSparks.Get(pType->DamageSparks))
			{

				StackVector<ParticleSystemTypeClass*, 0x25> Systems {};

				if (auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems))
				{
					auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

					for (auto pSystem : it)
					{
						if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Spark)
						{
							Systems->push_back(pSystem);
						}
					}
				}

				if (!Systems->empty())
				{

					const double _probability = _HPRatio >= RulesClass::Instance->ConditionRed ?
						RulesClass::Instance->ConditionYellowSparkingProbability : RulesClass::Instance->ConditionRedSparkingProbability;
					const auto _rand = ScenarioClass::Instance->Random.RandomDouble();

					if (_rand < _probability)
					{
						CoordStruct _offs = pThis->Location + pType->GetParticleSysOffset();
						pThis->Sys.Spark =
							GameCreate<ParticleSystemClass>(Systems[ScenarioClass::Instance->Random.RandomFromMax(Systems->size() - 1)], _offs, nullptr, pThis);
					}
				}
			}
		}
	}

	return 0x6FAF01;
}

ASMJIT_PATCH(0x6FA47C, TechnoClass_Update_Cleartarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if ((Unsorted::CurrentFrame() % 16) != 0)
	{
		return 0x6FA4D1;
	}

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		return 0x6FA4D1;

	RadioClass* radio = pThis->GetRadioContact();
	const Mission mission = pThis->CurrentMission;

	if (!radio || radio->WhatAmI() != BuildingClass::AbsID || radio->CurrentMission != Mission::Unload)
	{
		if (mission != Mission::Capture && mission != Mission::Sabotage)
		{
			const int weaponIndex = pThis->SelectWeapon(pThis->Target);
			const FireError threatResult = pThis->GetFireError(pThis->Target, weaponIndex, false);

			if (threatResult == FireError::ILLEGAL || threatResult == FireError::CANT)
			{
				WeaponTypeClass* weapon = pThis->GetWeapon(weaponIndex)->WeaponType;
				const bool is_firing_particles = weapon && (
						(weapon->UseFireParticles && pThis->Sys.Fire)
					|| (weapon->IsRailgun && pThis->Sys.Railgun)
					|| (weapon->UseSparkParticles && pThis->Sys.Spark)
					|| (weapon->IsSonic && pThis->Wave));

				if (!is_firing_particles || threatResult == FireError::ILLEGAL || threatResult == FireError::CANT)
				{
					pThis->SetTarget(nullptr);
				}
			}
		}
	}

	return 0x6FA4D1;
}