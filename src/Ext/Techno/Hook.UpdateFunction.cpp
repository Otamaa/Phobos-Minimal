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

#include <Utilities/Macro.h>

#define ENABLE_THESE

void FakeTechnoClass::__HandleGattlingAudio(TechnoClass* pThis)
{
	if (GET_TECHNOTYPE(pThis)->IsGattling) {
		VocClass::PlayIfInRange(pThis->Location, &pThis->Audio3);
	}
}

void FakeTechnoClass::__HandleVoicePlayback(TechnoClass* pThis)
{
	if (pThis->QueuedVoiceIndex == -1) {
		return;
	}

	if (!pThis->Audio6.AudioEventHandleGet()) {
		int voiceToPlay = pThis->QueuedVoiceIndex;
		pThis->__LastVoicePlayed = voiceToPlay;
		VocClass::PlayGlobal(voiceToPlay, Panning::Center, 1.0, &pThis->Audio6);
	} else if (pThis->__LastVoicePlayed == pThis->QueuedVoiceIndex) {
		pThis->QueuedVoiceIndex = -1;
	}
}

void FakeTechnoClass::__HandleBerzerkState(TechnoClass* pThis)
{
	if (!pThis->Berzerk) {
		return;
	}

	pThis->BerzerkDurationLeft--;

	if (pThis->BerzerkDurationLeft <= 0)
	{
		pThis->Berzerk = 0;
		pThis->BerzerkDurationLeft = 0;
		pThis->SetTarget(0);
		TechnoExtData::SetMissionAfterBerzerk(pThis);
	}
}

void FakeTechnoClass::__HandleStrengthSmoothing(TechnoClass* pThis)
{
	int currentStrength = pThis->Health;

	// Clamp Strength2 to not exceed actual strength
	if (pThis->EstimatedHealth > currentStrength) {
		pThis->EstimatedHealth = currentStrength;
	}

	// Smoothly increase Strength2 every 4 frames
	if ((Unsorted::CurrentFrame() & 4) != 0)
	{
		if (pThis->EstimatedHealth < currentStrength) {
			if (pThis->EstimatedHealth + 30 < 0) {
				pThis->EstimatedHealth = -30;
			}
			++pThis->EstimatedHealth;
		}
	}
}

void FakeTechnoClass::__HandleTurretAudio(TechnoClass* pThis)
{
	auto pType = GET_TECHNOTYPE(pThis);

	if (!pType->Turret) {
		return;
	}

	if (pThis->TurretIsRotating) {
		if (pThis->__IsTurretTurning_49C) {
			pThis->Audio3.AudioEventHandleStop();
			VocClass::SafeImmedietelyPlayAt(pType->TurretRotateSound, &pThis->Location, &pThis->Audio3);

			pThis->__IsTurretTurning_49C = 0;
		}
	} else {
		pThis->Audio3.AudioEventHandleStop();
		pThis->__IsTurretTurning_49C = 1;
	}

	VocClass::PlayIfInRange(pThis->Location, &pThis->Audio3);
}

void FakeTechnoClass::__HandleVeterancyPromotion(TechnoClass* pThis)
{
	const auto currentRank = pThis->Veterancy.GetRemainingLevel();

	if (pThis->CurrentRanking == currentRank) {
		return;
	}

	// Only process promotions, not demotions
	if (pThis->CurrentRanking == Rank::Invalid) {
		pThis->CurrentRanking = currentRank;
		return;
	}

	if (currentRank == Rank::Invalid) {
		pThis->CurrentRanking = currentRank;
		return;
	}

	// Handle promotion to Elite
	if (currentRank == Rank::Veteran && pThis->Owner->ControlledByCurrentPlayer()) {
		VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->UpgradeVeteranSound, &pThis->Location);
		VoxClass::Play("EVA_UnitPromoted");
	}
	else if (pThis->Owner->ControlledByCurrentPlayer()) {
		VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->UpgradeEliteSound, &pThis->Location);
		VoxClass::Play("EVA_UnitPromoted");
		pThis->Flashing.DurationRemaining = RulesClass::Instance->EliteFlashTimer;
	}

	pThis->CurrentRanking = currentRank;
}

void FakeTechnoClass::__HandleMoneyDrain(TechnoClass* pThis)
{
	if (!pThis->DrainingMe) {
		return;
	}

	TechnoTypeClass* techType = GET_TECHNOTYPE(pThis);

	if (!techType->ResourceDestination) {
		return;
	}

	if ((Unsorted::CurrentFrame() % RulesClass::Instance()->DrainMoneyFrameDelay) != 0) {
		return;
	}

	TechnoExtData::ApplyDrainMoney(pThis);
}

void FakeTechnoClass::__HandleDrainTarget(TechnoClass* pThis)
{
	TechnoClass* drainTarget = pThis->DrainTarget;

	if (!drainTarget) {
		return;
	}

	// Check if drain target became an ally
	if (!drainTarget->Owner->IsAlliedWith(pThis)) {
		return;
	}

	// Remove drain animation
	if (auto& pAnim =pThis->DrainAnim)
	{
		pAnim->UnInit();
		pAnim = 0;
	}

	// Clear drain target

	drainTarget->DrainingMe = 0;

	if (HouseClass* targetHouse = drainTarget->Owner) {
		targetHouse->RecheckPower = 1;
	}

	pThis->DrainTarget = 0;
}

void FakeTechnoClass::__HandleHiddenState(TechnoClass* pThis)
{
	if (pThis->InOpenToppedTransport)
		return;

	const auto pType = GET_TECHNOTYPEEXT(pThis);
	if (pType->IsDummy)
		return;

	if (const auto pBld = cast_to<BuildingClass*, false>(pThis)) {
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0) {
			return;
		}
	}

	CoordStruct centerCoord = pThis->GetCoords();
	CellStruct cellPos = CellClass::Coord2Cell(centerCoord);
	CellClass* cell = MapClass::Instance->GetCellAt(cellPos);

	TechnoTypeClass* techType = GET_TECHNOTYPE(pThis);

	bool shouldBeHidden = techType->CanBeHidden
		&& cell->IsCovered()
		&& pThis->WhatAmI() != AircraftClass::AbsID;

	if (!shouldBeHidden) {
		if (auto& pHidden = pThis->BehindAnim) {
			pHidden->UnInit();
			pHidden = 0;
		}

		return;
	}

	Point2D pos {
		.X = cellPos.X,
		.Y = cellPos.Y
	};

	pThis->DrawBehindMark(&pos, nullptr);
}

void FakeTechnoClass::__ClearInvalidAllyTarget(TechnoClass* pThis)
{
	if (!pThis->Target)
	{
		return;
	}

	if (auto pObj = flag_cast_to<ObjectClass*>(pThis->Target)) {
		if (!pObj->IsAlive) {
			pThis->SetTarget(0);
			return;
		}
	}

	HouseClass* controllingHouse = nullptr;
	bool isInOpenToppedTransport = false;

	// Check if in open-topped transport with different owner
	if (TechnoClass* transport = pThis->Transporter) {
		TechnoTypeClass* transportType = GET_TECHNOTYPE(transport);

		if(TechnoTypeExtContainer::Instance.Find(transportType)
			->Passengers_SyncOwner.Get()){
			if (transportType->OpenTopped && transport->MindControlledBy) {
				controllingHouse = transport->MindControlledBy->Owner;
				isInOpenToppedTransport = true;
			}
		}
	}

	const HouseClass* pOwner = !isInOpenToppedTransport ? pThis->Owner : controllingHouse;

	// Check if player has control
	if (pOwner->IsControlledByHuman()) {
		return;
	}

	bool isTargetAlly = false;
	if (const auto pTechTarget = flag_cast_to<ObjectClass*>(pThis->Target)) {
		if (const auto pTargetHouse = pTechTarget->GetOwningHouse()) {
			if (pOwner->IsAlliedWith(pTargetHouse)) {
				isTargetAlly = true;
			}
		}
	}

	auto pType = GET_TECHNOTYPE(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pThis->Berzerk &&  (pThis->Veterancy.IsElite() ? pTypeExt->AttackFriendlies.Y : pTypeExt->AttackFriendlies.X) && isTargetAlly && pTypeExt->AttackFriendlies_AutoAttack) {
		return;
	}

	const bool IsNegDamage = (pThis->CombatDamage() < 0);

	if(isTargetAlly != IsNegDamage) {
		// Check for special cases where ally targeting is allowed
		const bool isInfantry = pThis->WhatAmI() == InfantryClass::AbsID;
		const bool isTargetBuilding = pThis->Target->WhatAmI() == BuildingClass::AbsID;
		const bool canOccupy = isInfantry && isTargetBuilding && ((BuildingClass*)pThis->Target)->CanBeOccupyedBy((InfantryClass*)pThis);
		const bool isEngineer = isInfantry && ((InfantryClass*)pThis)->Type->Engineer;

		// Check for electric assault weapon against powered buildings
		bool canElectricAssault = false;
		WeaponStruct* weapon = pThis->GetWeapon(1);
		if (weapon->WeaponType && weapon->WeaponType->Warhead->ElectricAssault && isTargetBuilding) {
			canElectricAssault = ((BuildingClass*)pThis->Target)->Type->Overpowerable != 0;
		}

		// Clear target if not a valid ally target exception
		if (!isEngineer && !canElectricAssault && !canOccupy && !pThis->Berzerk) {
			pThis->SetTarget(0);
		}
	}
}

void FakeTechnoClass::__CheckTargetInRange(TechnoClass* pThis)
{
	if (!pThis->Target) {
		return;
	}

	if ((Unsorted::CurrentFrame() % 16) != 0) {
		return;
	}

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		return;

	RadioClass* radio = pThis->GetRadioContact();
	const Mission mission = pThis->CurrentMission;

	if (!radio || radio->WhatAmI() != BuildingClass::AbsID || radio->CurrentMission != Mission::Unload) {
		if (mission != Mission::Capture && mission != Mission::Sabotage) {
			const int weaponIndex = pThis->SelectWeapon(pThis->Target);
			const FireError threatResult = pThis->GetFireError(pThis->Target, weaponIndex, false);


			if (threatResult == FireError::ILLEGAL || threatResult == FireError::CANT) {

				WeaponTypeClass* weapon = pThis->GetWeapon(weaponIndex)->WeaponType;
				const bool is_firing_particles = weapon && (
						(weapon->UseFireParticles && pThis->Sys.Fire)
					|| (weapon->IsRailgun && pThis->Sys.Railgun)
					|| (weapon->UseSparkParticles && pThis->Sys.Spark)
					|| (weapon->IsSonic && pThis->Wave));

				if (!is_firing_particles || threatResult == FireError::ILLEGAL || threatResult == FireError::CANT) {
					pThis->SetTarget(nullptr);
				}
			}
		}
	}

}

void FakeTechnoClass::__HandleTurretRecoil(TechnoClass* pThis)
{
	if (pThis->InLimbo || !GET_TECHNOTYPE(pThis)->TurretRecoil) {
		return;
	}

	pThis->TurretRecoil.Update();
	pThis->BarrelRecoil.Update();
}

void FakeTechnoClass::__HandleChargeTurret(TechnoClass* pThis)
{
	TechnoTypeClass* techType = GET_TECHNOTYPE(pThis);

	if (!techType->IsChargeTurret) {
		return;
	}

	if (!techType->HasTurret() || techType->IsGattling) {
		return;
	}

	if (pThis->ROF <= 0) {
		pThis->CurrentTurretNumber = 0;
		return;
	}

	auto const pType = GET_TECHNOTYPE(pThis);
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
}

void FakeTechnoClass::__HandleDoorAndTimers(TechnoClass* pThis)
{
	if (pThis->UnloadTimer.HasFinished()) {
		pThis->UnloadTimer.Update();
	}

	if (pThis->unknown_bool_41E > 0) {
		pThis->unknown_bool_41E--;
	}
}

void FakeTechnoClass::__ClearTargetForInvalidMissions(TechnoClass* pThis)
{
	if (!pThis->Target || !pThis->ShouldLoseTargetNow) {
		return;
	}

	Mission mission = pThis->CurrentMission;

	// List of missions that should not have targets
	if (mission == Mission::Sleep ||
		mission == Mission::Enter ||
		mission == Mission::Stop ||
		mission == Mission::Ambush ||
		mission == Mission::Unload ||
		mission == Mission::Construction ||
		mission == Mission::Selling ||
		mission == Mission::Repair ||
		mission == Mission::Missile ||
		mission == Mission::Harmless ||
		mission == Mission::Wait ||
		mission == Mission::Open)
	{
		pThis->SetTarget(nullptr);
	}
}

void FakeTechnoClass::__HandleTargetAcquisition(TechnoClass* pThis)
{
	if (!pThis->TargetingTimer.Expired()) {
		return;
	}
	auto const
		pRulesExt = RulesExtData::Instance();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if ((!pThis->Owner->IsControlledByHuman() || !pRulesExt->DistributeTargetingFrame_AIOnly) && pTypeExt->DistributeTargetingFrame.Get(pRulesExt->DistributeTargetingFrame)) {
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (Unsorted::CurrentFrame % 16 != pExt->MyTargetingFrame) {
			return;
		}
	}

	if (pThis->MegaMissionIsAttackMove()) {
		const bool skip = pRulesExt->ExpandAircraftMission
			&& pThis->WhatAmI() == AbstractType::Aircraft
			&& (!pThis->Ammo || !pThis->IsInAir());

		if(!skip)
			pThis->UpdateAttackMove();

		return;
	}

	if (auto pInf = cast_to<InfantryClass*, false>(pThis)) {
		if (pInf->Type->Slaved && pInf->SlaveOwner) {
			return;
		}
	}

	if (!pThis->IsArmed())
		return;

	Mission mission = pThis->CurrentMission;
	if ((mission == Mission::Move || mission == Mission::Harvest || mission == Mission::Guard)
		&& pThis->TechnoClass_709290()) {

		AbstractClass* oldTarget = pThis->Target;
		pThis->__creationframe_4FC = Unsorted::CurrentFrame();

		CoordStruct centerCoord = pThis->GetCoords();

		if (pThis->TargetAndEstimateDamage(&centerCoord, ThreatType::Range)) {
			if (pThis->Target != oldTarget) {
				pThis->ShouldLoseTargetNow = 1;
			}
		}
	}
}

void FakeTechnoClass::__HandleAttachedBomb(TechnoClass* pThis)
{
	BombClass* bomb = pThis->AttachedBomb;

	if (!bomb || pThis->InLimbo) {
		return;
	}

	if (bomb->TimeToExplode()) {
		bomb->Detonate();
	}
}

void FakeTechnoClass::__HandleManagers(TechnoClass* pThis)
{
	if (auto pSlave  = pThis->SlaveManager) {
		pSlave->Update();
	}

	TechnoExtData::UpdateMCOverloadDamage(pThis);
}

void FakeTechnoClass::__HandleSelfHealing(TechnoClass* pThis)
{
	const auto pType = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	// prevent crashing and sinking technos from self-healing
	if (pType->NoExtraSelfHealOrRepair || pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking || TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled) {
		return;
	}

	const auto nUnit = cast_to<UnitClass*, false>(pThis);
	if (nUnit && nUnit->DeathFrameCounter > 0) {
		return;
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

}

void FakeTechnoClass::__HandleCloaking(TechnoClass* pThis)
{
	FakeTechnoClass::_Cloaking_AI(pThis, discard_t(), false);

	if (auto pSpawn = pThis->SpawnManager) {
		pSpawn->Update();
	}

	CoordStruct centerCoord = pThis->GetCoords();
	CellClass* cell = MapClass::Instance->GetCellAt(centerCoord);

	// Handle uncloaking in cloaked cells
	if (pThis->CloakState == CloakState::Uncloaking) {
		if (cell->CellClass_cloak_4870B0((char)pThis->Owner->ArrayIndex)) {
			pThis->Sensed();
		}
	}

	// Handle cloaking in non-cloaked cells
	if (pThis->CloakState == CloakState::Cloaked) {
		if (!cell->CellClass_cloak_4870B0((char)pThis->Owner->ArrayIndex)) {
			pThis->Sensed();
		}
	}
}

void FakeTechnoClass::__ClearTargetIfNoDamage(TechnoClass* pThis)
{
	if (!pThis->Target) {
		return;
	}

	const bool IsAlly = pThis->Owner->IsAlliedWith(pThis->Target);

	if (IsAlly)
		return;

	const bool IsCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
	const bool IsHumanControlled = IsCampaign
		? pThis->Owner->ControlledByCurrentPlayer() : pThis->Owner->IsHumanPlayer;

	if (
		// Campaign AI: Clear target if no damage can be dealt
		(IsCampaign && !IsHumanControlled)
		||
		// Multiplayer: Human player units clear target if no damage
		(!IsCampaign && IsHumanControlled)
		){
		if (pThis->CombatDamage() < 0) {
			pThis->SetTarget(0);
			return;
		}
	}
}

void FakeTechnoClass::__ClearAircraftTarget(TechnoClass* pThis)
{
	auto target = cast_to<AircraftClass*>(pThis->Target);

	// Only units need special handling for aircraft targets
	if (!target || pThis->WhatAmI() != UnitClass::AbsID) {
		return;
	}

	if (pThis->CombatDamage() > 0) {
		return;
	}

	// Clear target if aircraft is in air or on a building
	bool shouldClearTarget = false;

	if (target->GetHeight() > 0) {
		shouldClearTarget = true;
	} else {
		if (target->GetCell()->GetBuilding()) {
			shouldClearTarget = true;
		}
	}

	if (shouldClearTarget) {
		pThis->SetTarget(0);
	}
}

void FakeTechnoClass::__CheckTargetReachability(TechnoClass* pThis)
{
	// Aircraft can always reach their targets
	if (pThis->WhatAmI() == AircraftClass::AbsID || !pThis->Target) {
		return;
	}

	// Units in teams have different targeting logic
	if (pThis->BelongsToATeam()) {
		return;
	}

	// Check if this is a FootClass with navigation
	FootClass* foot = flag_cast_to<FootClass*>(pThis);
	if (foot && !foot->Destination)
		foot = nullptr;

	// If not in same zone as target and not in open-topped transport
	if (!pThis->IsInSameZoneAs(pThis->Target) || (pThis->InOpenToppedTransport && pThis->Transporter))
	{
		// Try to approach target if possible
		if (foot) {
			foot->ApproachTarget(0);
		}

		// If still can't navigate to target, check if in weapon range
		if (!foot || !foot->Destination) {
			if (!pThis->IsCloseEnough(pThis->Target, pThis->SelectWeapon(pThis->Target))) {

				pThis->SetTarget(0);
			}
		}
	}
}

void FakeTechnoClass::__UpdateAnimationStage(TechnoClass* pThis)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	// Buildings don't use this animation system
	if (pThis->WhatAmI() == BuildingClass::AbsID || pExt->DelayedFireSequencePaused) {
		return;
	}

	pThis->Animation.Update();
}

void FakeTechnoClass::__HandleFlashing(TechnoClass* pThis)
{
	int oldFlashCount = pThis->Flashing.DurationRemaining;
	bool wasFlashingVisible = (oldFlashCount & 2) == 2;

	// Process flash state
	if (pThis->Flashing.Update()) {
		pThis->Mark(MarkType::Change);
	}

	// Special handling for building flash state changes
	if (oldFlashCount &&
		oldFlashCount != pThis->Flashing.DurationRemaining &&
		pThis->WhatAmI() == BuildingClass::AbsID)
	{

		const bool isFlashingVisible = (pThis->Flashing.DurationRemaining & 2) == 2;

		// Only redraw if visibility changed
		if (wasFlashingVisible != isFlashingVisible)
		{
			RectangleStruct dimensions;
			pThis->GetRenderDimensions(&dimensions);
			TacticalClass::Instance->RegisterDirtyArea(dimensions, false);
			pThis->Airstrike_0x452000();
		}
	}
}

void FakeTechnoClass::__HandleDamageSparks(TechnoClass* pThis)
{
	if (!pThis->Sys.Spark)
	{
		auto _HPRatio = pThis->GetHealthPercentage();

		if (!(_HPRatio >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10))
		{

			auto pType = GET_TECHNOTYPE(pThis);
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
}

void FakeTechnoClass::__HandleEMPEffect(TechnoClass* pThis)
{
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
}

void __fastcall FakeTechnoClass::__AI(TechnoClass* pThis)
{
	// Clear moused-over flag
	if (pThis->IsMouseHovering) {
		pThis->IsMouseHovering = 0;
	}

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto IsBuilding = pThis->WhatAmI() == BuildingClass::AbsID;

	TechnoExt_ExtData::Ares_technoUpdate(pThis);

	if (!pThis->IsAlive)
		return;

	if (!IsBuilding) {
		pExt->UpdateLaserTrails();
		TrailsManager::AI((FootClass*)pThis);
	}

	HugeBar::InitializeHugeBar(pThis);

	PhobosAEFunctions::UpdateAttachEffects(pThis);

	if (!pThis->IsAlive)
		return;

	auto const pType = GET_TECHNOTYPE(pThis);
	bool IsInLimboDelivered = false;

	if (IsBuilding) {
		IsInLimboDelivered = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LimboID >= 0;
	}

	if (pExt->IsInTunnel)
	{
		pExt->IsInTunnel = false;

		if (auto pShieldData = pExt->GetShield())
			pShieldData->SetAnimationVisibility(true);
	}

	if (pExt->UpdateKillSelf_Slave())
	{
		return;
	}

	if (pExt->CheckDeathConditions())
	{
		return;
	}

	pExt->UpdateBuildingLightning();
	pExt->UpdateShield();
	if (!pThis->IsAlive)
	{
		return;
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
		return;
	}
	pExt->UpdateGattlingOverloadDamage();
	if (!pThis->IsAlive)
	{
		return;
	}

	//TODO : improve this to better handle delay anims !
	//pExt->UpdateDelayFireAnim();

	pExt->UpdateRevengeWeapons();
	if (!pThis->IsAlive)
	{
		return;
	}

	// Handle various AI subsystems
	__HandleGattlingAudio(pThis);
	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();

	PassengersFunctional::AI(pThis);
	if (!pThis->IsAlive)
	{
		return;
	}

	SpawnSupportFunctional::AI(pThis);

	if (!pThis->IsAlive)
	{
		return;
	}

	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);

	if (!pThis->IsAlive)
	{
		return;
	}

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	GiftBoxFunctional::AI(pExt, pTypeExt);

	if (!pThis->IsAlive)
	{
		return;
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

	if (!pThis->IsAlive) {
		return;
	}

	__HandleVoicePlayback(pThis);
	__HandleBerzerkState(pThis);
	__HandleStrengthSmoothing(pThis);
	__HandleTurretAudio(pThis);
	TechnoExperienceData::PromoteImmedietely(pThis, false, true);
	__HandleMoneyDrain(pThis);
	__HandleDrainTarget(pThis);

	// Handle voxel rocking for vehicles
	if (!pThis->InLimbo && pThis->IsVoxel()) {
		pThis->RockingAI();
		if (!pThis->IsAlive) {
			return;
		}
	}

	// Handle hiding/revealing based on terrain
	__HandleHiddenState(pThis);

	// Target validation and clearing
	__ClearInvalidAllyTarget(pThis);
	__CheckTargetInRange(pThis);

	// Weapon systems
	__HandleTurretRecoil(pThis);
	__HandleChargeTurret(pThis);
	__HandleDoorAndTimers(pThis);

	// Mission and target management
	__ClearTargetForInvalidMissions(pThis);

	++pThis->MissionAccumulateTime;
	pThis->MissionClass::Update();

	__HandleTargetAcquisition(pThis);
	__HandleAttachedBomb(pThis);
	__HandleManagers(pThis);

	if (!pThis->IsAlive) {
		return;
	}

	// Health and healing
	__HandleSelfHealing(pThis);

	if (!pThis->IsAlive) {
		return;
	}

	// Cloaking and stealth
	__HandleCloaking(pThis);

	// Combat target validation
	__ClearTargetIfNoDamage(pThis);
	__ClearAircraftTarget(pThis);
	__CheckTargetReachability(pThis);

	// Visual effects
	__UpdateAnimationStage(pThis);
	__HandleFlashing(pThis);
	__HandleDamageSparks(pThis);

	// Sensors and special effects
	pThis->RadarTrackingUpdate(false);
	__HandleEMPEffect(pThis);
}
#pragma optimize("", off )
void __fastcall FakeTechnoClass::_Cloaking_AI(TechnoClass* pThis, discard_t, bool something)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pThis->CloakState != CloakState::Uncloaked)
	{
		// === Currently in a cloak state — tick stage counter ===
		pThis->CloakProgress.Update();

		if (pThis->CloakProgress.Stage < 0)
			pThis->CloakProgress.Stage = 0;

		switch (pThis->CloakState)
		{
		case CloakState::Cloaking: {
			pThis->Mark(MarkType::Change);

			// Kickstart cloak animation if Rate was zero
			if (pThis->CloakProgress.Timer.Rate == 0) {
				pThis->CloakProgress.Start(1);
			}

			VisualType visual = pThis->VisualCharacter(true, nullptr);

			if (visual == VisualType::Darken) {
				// Damaged units may spontaneously uncloak
				if (pThis->IsRedHP()
					&& ScenarioClass::Instance->Random.RandomRanged(0, 99) < 10) {
					pThis->Uncloak(true);
				}
			}
			else if (visual == VisualType::Shadowy || visual == VisualType::Hidden)
			{
				// Cloaking complete — transition to fully cloaked
				pThis->CloakState = CloakState::Cloaked;
				pThis->CloakProgress.Start(0, 0, 0);
				pThis->Mark(MarkType::Change);

				// Units carrying a captured flag can never fully cloak
				if (pThis->WhatAmI() == AbstractType::Unit
					&& ((UnitClass*)(pThis))->FlagHouseIndex != -1)
				{
					pThis->Reveal();
				}
				else
				{
					// Collect technos targeting us that can still sense us
					DynamicVectorClass<TechnoClass*> retargetList;

					for (int i = TechnoClass::Array->Count - 1; i >= 0; i--)
					{
						TechnoClass* pTechno = TechnoClass::Array->Items[i];

						// === Hook: TechnoClass_CloakingAI_detachsensed (0x6FBB35) ===
						// Extended validity checks replacing simple TarCom check
						if (!pTechno
							|| pTechno->Target != pThis
							|| !pTechno->Owner)
							continue;

						if (!pTechno->IsAlive
							|| pTechno->IsCrashing
							|| pTechno->IsSinking)
							continue;

						int houseIdx = pTechno->Owner->ArrayIndex;
						CoordStruct center = pThis->GetCoords();
						CellClass* pCell = MapClass::Instance->GetCellAt(center);

						if (pCell->Sensors_InclHouse(houseIdx)
							|| pTechno->Owner == pThis->Owner)
						{
							retargetList.emplace_back(pTechno);
						}
					}

					// === Hook: TechnoClass_Cloak_BeforeDetach (0x6FBBC3) ===

					pExt->UpdateMindControlAnim();
					pExt->IsDetachingForCloak = true;

					pThis->AnnounceExpiredPointer(false);

					// === Hook: TechnoClass_Cloak_AfterDetach (0x6FBBCE) ===
					pExt->IsDetachingForCloak = false;

					for (int j = retargetList.Count - 1; j >= 0; j--) {
						retargetList[j]->SetTarget(pThis);
					}
				}

				// === Hook: LJMP 0x6FBC0B → 0x6FBC80 ===
				// AI scatter after cloaking removed entirely
			}
			break;
		}

		case CloakState::Cloaked:
		{
			if (pThis->ShouldNotBeCloaked())
			{
				pThis->Uncloak(false);
			}
			break;
		}

		case CloakState::Uncloaking:
		{
			pThis->Mark(MarkType::Change);

			VisualType visual = pThis->VisualCharacter(true, nullptr);

			if (visual == VisualType::Normal)
			{
				// Fully uncloaked — reset cloak device
				pThis->CloakProgress.Start(0,0,0);
				pThis->CloakState = CloakState::Uncloaked;

				// === Hook: TechnoClass_Cloak_RestoreMCAnim (0x6FB9D7) ===
				pExt->UpdateMindControlAnim();

				// Start cloak delay timer
				int cloakDelay = static_cast<int>(RulesClass::Instance->CloakDelay * TICKS_PER_MINUTE);
				pThis->CloakDelayTimer.Start(cloakDelay);
				pThis->Mark(MarkType::Change);
			}
			else if (visual == VisualType::Indistinct) {
				if (pThis->IsReadyToCloak()) {
					pThis->Cloak(true);
				}
			}
			break;
		}

		default:
			break;
		}
	}
	else
	{

		// === Not cloaked — check if we should begin cloaking ===
		// === Hook: TechnoClass_UpdateCloak (0x6FB757) ===
		// Extension pre-check: if not disallowed, fast-path past all vanilla checks
		bool canCloak = !TechnoExt_ExtData::CloakDisallowed(pThis, false);

		if(!canCloak)
		{

			canCloak = !pThis->IsUnderEMP()
				 && !pThis->IsParalyzed()
				 && !pThis->IsBeingWarpedOut()
				 && !pThis->IsWarpingIn();

			// Veteran/Elite cloak ability override
			if (!canCloak) {
				canCloak = pThis->HasAbility(AbilityType::Cloak);
				if (!canCloak)
					return;
			}
		}

		// Don't cloak while docked in a weapons factory
		TechnoClass* contact = pThis->RadioLinks.Items[0];

		if (contact != nullptr
			&& contact->WhatAmI() == AbstractType::Building
			&& static_cast<BuildingClass*>(contact)->Type->WeaponsFactory)
		{
			return;
		}

		// Tick stage counter
		pThis->CloakProgress.Update();

		// Initiate cloaking if ready and healthy enough
		if (!pThis->IsReadyToCloak())
			return;

		if (!pThis->IsRedHP()) {
			pThis->Cloak(false);
		}
		else if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < 4) {
			// Damaged units have a small chance to cloak anyway
			pThis->Cloak(false);
		}
	}
}
#pragma optimize("", on )
DEFINE_FUNCTION_JUMP(LJMP, 0x6FB740, FakeTechnoClass::_Cloaking_AI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E26B4, FakeTechnoClass::_Cloaking_AI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E90A4, FakeTechnoClass::_Cloaking_AI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB468, FakeTechnoClass::_Cloaking_AI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D70, FakeTechnoClass::_Cloaking_AI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6080, FakeTechnoClass::_Cloaking_AI);

bool __fastcall FakeTechnoClass::_ShouldNotBeCloaked(TechnoClass* pThis)
{
	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	return TechnoExt_ExtData::CloakDisallowed(pThis, true);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6FBC90, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(CALL, 0x4578C9, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2548, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8F38, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB2FC, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4C04, FakeTechnoClass::_ShouldNotBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F14, FakeTechnoClass::_ShouldNotBeCloaked);
//BuildingClass has it own implementation


bool __fastcall FakeTechnoClass::_ShouldBeCloaked(TechnoClass * pThis)
{
	return TechnoExt_ExtData::CloakAllowed(pThis);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6FBDC0, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(CALL, 0x457779, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2544, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8F34, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB2F8, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4C00, FakeTechnoClass::_ShouldBeCloaked);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F10, FakeTechnoClass::_ShouldBeCloaked);
//BuildingClass has it own implementation

ASMJIT_PATCH(0x70E92F, TechnoClass_UpdateAirstrikeTint, 0x5)
{
	enum { ContinueIn = 0x70E96E, Skip = 0x70EC9F };

	GET(TechnoClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueIn : Skip;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F9E50, FakeTechnoClass::__AI);