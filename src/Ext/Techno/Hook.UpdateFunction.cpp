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

#define ENABLE_THESE

inline COMPILETIMEEVAL bool IsTimerExpired(const CDTimerClass& timer)
{
	if (timer.StartTime == -1)
	{
		return timer.TimeLeft == 0;
	}

	return (timer.CurrentTime - timer.StartTime) >= timer.TimeLeft;
}

inline COMPILETIMEEVAL int GetRemainingTime(const CDTimerClass& timer)
{
	if (timer.StartTime == -1)
	{
		return timer.TimeLeft;
	}

	int elapsed = timer.CurrentTime - timer.StartTime;
	return (elapsed >= timer.TimeLeft) ? 0 : (timer.TimeLeft - elapsed);
}

void FakeTechnoClass::__HandleGattlingAudio(TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->IsGattling) {
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
		pThis->QueuedVoiceIndex = -1;
	} else if (pThis->__LastVoicePlayed == pThis->QueuedVoiceIndex) {
		pThis->QueuedVoiceIndex = -1;
	}
}

void FakeTechnoClass::__HandleBerzerkState(TechnoClass* pThis)
{
	if (!pThis->Berzerk)
	{
		return;
	}

	pThis->BerzerkDurationLeft--;

	if (pThis->BerzerkDurationLeft <= 0)
	{
		pThis->Berzerk = 0;
		pThis->BerzerkDurationLeft = 0;
		pThis->SetTarget(0);

		Mission newMission = pThis->Owner->IsHumanPlayer ? Mission::Guard : Mission::Hunt;
		pThis->QueueMission(newMission, 0);
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
			pThis->EstimatedHealth++;
		}
	}
}

void FakeTechnoClass::__HandleTurretAudio(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();

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

	TechnoTypeClass* techType = pThis->GetTechnoType();

	if (!techType->ResourceDestination) {
		return;
	}

	if ((Unsorted::CurrentFrame() % RulesClass::Instance()->DrainMoneyFrameDelay) != 0) {
		return;
	}

	int drainAmount = RulesClass::Instance()->DrainMoneyAmount;
	int availableMoney = (int)pThis->Owner->Available_Money();

	if (availableMoney < drainAmount) {
		drainAmount = availableMoney;
	}

	pThis->Owner->TransactMoney(drainAmount);
	pThis->DrainingMe->Owner->TransactMoney(drainAmount);
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
	CoordStruct centerCoord = pThis->GetCoords();
	CellStruct cellPos = CellClass::Coord2Cell(centerCoord);
	CellClass* cell = MapClass::Instance->GetCellAt(cellPos);

	TechnoTypeClass* techType = pThis->GetTechnoType();

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

	pThis->DrawBehind(&pos, nullptr);
}

void FakeTechnoClass::__ClearInvalidAllyTarget(TechnoClass* pThis)
{
	if (!pThis->Target)
	{
		return;
	}

	HouseClass* controllingHouse = nullptr;
	bool isInOpenToppedTransport = false;

	// Check if in open-topped transport with different owner
	if (TechnoClass* transport = pThis->Transporter) {
		TechnoTypeClass* transportType = transport->GetTechnoType();

		if (transportType->OpenTopped && transport->MindControlledBy) {
			controllingHouse = transport->MindControlledBy->Owner;
			isInOpenToppedTransport = true;
		}
	}

	// Check if player has control
	const bool playerHasControl = isInOpenToppedTransport
		? controllingHouse->IsControlledByHuman()
		: pThis->Owner->IsControlledByHuman();

	if (playerHasControl) {
		return;
	}

	// Check if targeting an ally
	const bool isTargetAlly = isInOpenToppedTransport
		? controllingHouse->IsAlliedWith(pThis->Target)
		: pThis->Owner->IsAlliedWith(pThis->Target);

	if (!isTargetAlly) {
		return;
	}

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

void FakeTechnoClass::__CheckTargetInRange(TechnoClass* pThis)
{
	if (!pThis->Target) {
		return;
	}

	if ((Unsorted::CurrentFrame() % 16) != 0) {
		return;
	}

	const Mission mission = pThis->CurrentMission;
	if (mission == Mission::Capture || mission == Mission::Sabotage) {
		return;
	}

	const int weaponIndex = pThis->SelectWeapon(pThis->Target);
	const FireError threatResult = pThis->GetFireError(pThis->Target, weaponIndex, false);

	if (threatResult == FireError::ILLEGAL || threatResult == FireError::CANT) {
		pThis->SetTarget(nullptr);
	}
}

void FakeTechnoClass::__HandleTurretRecoil(TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->TurretRecoil) {
		return;
	}

	pThis->TurretRecoil.Update();
	pThis->BarrelRecoil.Update();
}

void FakeTechnoClass::__HandleChargeTurret(TechnoClass* pThis)
{
	TechnoTypeClass* techType = pThis->GetTechnoType();

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

	const int remainingTime = GetRemainingTime(pThis->RearmTimer);

	// Calculate which turret charge stage to display
	int chargeStage = remainingTime * techType->TurretCount / pThis->ROF;

	// Clamp to valid range
	if (chargeStage >= techType->TurretCount) {
		chargeStage = techType->TurretCount - 1;
	} else if (chargeStage < 0) {
		chargeStage = 0;
	}

	pThis->CurrentTurretNumber = chargeStage;
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
	if (!IsTimerExpired(pThis->TargetingTimer)) {
		return;
	}

	if (pThis->MegaMissionIsAttackMove()) {
		pThis->UpdateAttackMove();
		return;
	}

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

	if (auto pCapture = pThis->CaptureManager) {
		pCapture->HandleOverload();
	}
}

void FakeTechnoClass::__HandleSelfHealing(TechnoClass* pThis)
{
	if (!pThis->IsAlive) {
		return;
	}

	TechnoTypeClass* techType = pThis->GetTechnoType();
	int currentStrength = pThis->Health;
	int maxStrength = techType->Strength;
	bool isFullHealth = currentStrength >= maxStrength;

	// Check if unit should regenerate (underwater healing mechanic)
	if (pThis->ShouldSelfHealOneStep())
	{

		pThis->Health++;

		// Remove damage particles if healed enough or underwater
		if (pThis->IsGreenHP() ||
			pThis->GetHeight() < -10)
		{

			if (auto& pNat = pThis->Sys.Natural)
			{
				pNat->UnInit();
				pNat = nullptr;
			}
		}
	}

	if (isFullHealth || currentStrength == 0)
	{
		return;
	}

	auto unitType = pThis->WhatAmI();

	// Infantry self-healing
	if ((unitType == InfantryClass::AbsID || (unitType == UnitClass::AbsID && techType->Organic)))
	{
		if ((Unsorted::CurrentFrame() % RulesClass::Instance()->SelfHealInfantryFrames) == 0 &&
			pThis->Owner->DoInfantrySelfHeal())
		{

			int healAmount = pThis->Owner->GetInfSelfHealStep();
			int missingHealth = maxStrength - currentStrength;

			if (healAmount >= missingHealth)
			{
				healAmount = missingHealth;
			}

			pThis->Health += healAmount;
		}
	}
	// Unit self-healing (non-organic units)
	else if (unitType == UnitClass::AbsID && !techType->Organic)
	{
		if ((Unsorted::CurrentFrame() % RulesClass::Instance()->SelfHealUnitFrames) == 0 &&
			pThis->Owner->DoUnitsSelfHeal()) {

			int healAmount = pThis->Owner->GetUnitSelfHealStep();
			int missingHealth = maxStrength - currentStrength;

			if (healAmount >= missingHealth) {
				healAmount = missingHealth;
			}

			pThis->Health += healAmount;

			// Remove damage particles if healed enough
			if (pThis->IsGreenHP() || pThis->GetHeight() < -10) {

				if (auto& pPar = pThis->Sys.Natural) {
					pPar->UnInit();
					pPar = nullptr;
				}
			}
		}
	}
}

void FakeTechnoClass::__HandleCloaking(TechnoClass* pThis)
{
	pThis->UpdateCloak();

	if (auto pSpawn = pThis->SpawnManager) {
		pSpawn->Update();
	}

	CoordStruct centerCoord = pThis->GetCoords();
	CellClass* cell = MapClass::Instance->GetCellAt(centerCoord);

	// Handle uncloaking in cloaked cells
	if (pThis->CloakState == CloakState::Uncloaking) {
		if (cell->CellClass_cloak_4870B0(pThis->Owner->ArrayIndex)) {
			pThis->Sensed();
		}
	}

	// Handle cloaking in non-cloaked cells
	if (pThis->CloakState == CloakState::Cloaked) {
		if (!cell->CellClass_cloak_4870B0(pThis->Owner->ArrayIndex)) {
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
	if (!foot->Destination)
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
	// Buildings don't use this animation system
	if (pThis->WhatAmI() == BuildingClass::AbsID) {
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
	TechnoTypeClass* techType = pThis->GetTechnoType();

	if (!techType->DamageSparks) {
		return;
	}

	auto hp = pThis->GetHealthPercentage();

	if (hp >= RulesClass::Instance()->ConditionYellow) {
		return;
	}

	if (pThis->GetHeight() <= -10) {
		return;
	}

	// Build list of available spark particle systems
	std::vector<ParticleSystemTypeClass*> sparks;
	sparks.reserve(techType->DamageParticleSystems.Count);

	// Filter for spark-type particle systems (BehavesLike == 3)
	for (int i = 0; i < techType->DamageParticleSystems.Count; i++) {
		ParticleSystemTypeClass* psType = techType->DamageParticleSystems.Items[i];

		if (psType->BehavesLike == ParticleSystemTypeBehavesLike::Spark) {
			sparks.push_back(psType);
		}
	}

	// Create spark particle system if needed
	if (!pThis->Sys.Spark && !sparks.empty()) {

		const double sparkProbability = hp >= RulesClass::Instance()->ConditionRed ?
			RulesClass::Instance()->ConditionYellowSparkingProbability
			:
			RulesClass::Instance()->ConditionRedSparkingProbability;

		const double randomValue = ScenarioClass::Instance->Random.RandomDouble();

		if (randomValue < sparkProbability) {

			const CoordStruct particleOffset = techType->GetParticleSysOffset();
			const CoordStruct centerCoord = pThis->GetCoords();
			const CoordStruct spawnCoord = particleOffset + centerCoord;
			const int randomIndex = ScenarioClass::Instance->Random.RandomFromMax(sparks.size() - 1);

			pThis->Sys.Spark = GameCreate<ParticleSystemClass>(
					sparks[randomIndex],
					spawnCoord,
					nullptr,
					pThis,
					CoordStruct::Empty,
					nullptr);
		}
	}
}

void FakeTechnoClass::__HandleEMPEffect(TechnoClass* pThis)
{
	if (pThis->EMPLockRemaining-- <= 0) {
		return;
	}

	// EMP effect ended - restore unit functionality
	if (auto pBld = cast_to<BuildingClass*>(pThis))
	{
		// FIXME: Magic offset 5889 - likely building property
		if (!pBld->Type->InvisibleInGame)
		{
			pBld->EnableStuff();

			if (pBld->Type->Radar) {
				pThis->Owner->RecheckRadar = 1;
			}
		}
	}
	else
	{
		auto pFoot = static_cast<FootClass*>(pThis);

		if (pFoot->Locomotor) {
			pFoot->Locomotor->Power_On();
		}

		for (int i = 0; i < AnimClass::Array->Count; i++) {
			AnimClass* anim = AnimClass::Array->Items[i];

			if (anim &&
				anim->OwnerObject == pThis &&
				anim->Type == RulesClass::Instance()->EMPulseSparkles) {
				anim->RemainingIterations = 0;
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

	// Handle various AI subsystems
	__HandleGattlingAudio(pThis);
	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();
	__HandleVoicePlayback(pThis);
	__HandleBerzerkState(pThis);
	__HandleStrengthSmoothing(pThis);
	__HandleTurretAudio(pThis);
	__HandleVeterancyPromotion(pThis);
	__HandleMoneyDrain(pThis);
	__HandleDrainTarget(pThis);

	// Handle voxel rocking for vehicles
	if (pThis->IsVoxel()) {
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


ASMJIT_PATCH(0x6FA68B, TechnoClass_Update_AttackMovePaused, 0xA) // To make aircrafts not search for targets while resting at the airport, this is designed to adapt to loop waypoint
{
	enum { SkipGameCode = 0x6FA6F5 };

	GET(TechnoClass* const, pThis, ESI);

	const bool skip = RulesExtData::Instance()->ExpandAircraftMission
		&& pThis->WhatAmI() == AbstractType::Aircraft
		&& (!pThis->Ammo || !pThis->IsInAir());

	return skip ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x70E92F, TechnoClass_UpdateAirstrikeTint, 0x5)
{
	enum { ContinueIn = 0x70E96E, Skip = 0x70EC9F };

	GET(TechnoClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueIn : Skip;
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
				TechnoExtContainer::Instance.Find(pThis)->Get_EMPStateComponent()->LastMission = pThis->CurrentMission;
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
	if (pType->NoExtraSelfHealOrRepair || pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking || TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDriverKilled)
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

	if (pExt->Get_TechnoStateComponent()->DelayedFireSequencePaused)
		return SkipGameCode;

	return 0;
}

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	return TechnoTypeExtContainer::Instance.Find(pThis->Transporter->GetTechnoType())->Passengers_SyncOwner.Get() ?  ret : 0; }

SET_THREATEVALS(0x6FA33C, ESI, TechnoClass_AI_ThreatEvals_OpenToppedOwner, 0x6, 0x6FA37A)

#undef SET_THREATEVALS

#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

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
	auto pState = pExt->Get_TechnoStateComponent();

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

	DelayFireManager::TechnoClass_Update_CustomWeapon(pThis);

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

	if (auto pDSState = pExt->Get_DamageSelfState())
	{
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
