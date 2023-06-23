#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <Misc/AresData.h>
#include <Notifications.h>

#include <Ext/House/Body.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>

void DetachSpecificSpawnee(TechnoClass* Spawnee, HouseClass* NewSpawneeOwner) {

	// setting up the nodes. Funnily, nothing else from the manager is needed
	const auto& SpawnNodes = Spawnee->SpawnOwner->SpawnManager->SpawnedNodes;

	//find the specific spawnee in the node
	for (auto SpawnNode : SpawnNodes) {

		if (Spawnee == SpawnNode->Unit) {

			SpawnNode->Unit = nullptr;
			Spawnee->SpawnOwner = nullptr;

			SpawnNode->Status = SpawnNodeStatus::Dead;

			Spawnee->SetOwningHouse(NewSpawneeOwner);
		}
	}
}

void FreeSpecificSlave(TechnoClass* Slave, HouseClass* Affector) {

	//If you're a slave, you're an InfantryClass. But since most functions use TechnoClasses and the check can be done in that level as well
	//it's easier to set up the recasting in this function
	//Anybody who writes 357, take note that SlaveManager uses InfantryClasses everywhere, SpawnManager uses TechnoClasses derived from AircraftTypeClasses
	//as I wrote it in http://bugs.renegadeprojects.com/view.php?id=357#c10331
	//So, expand that one instead, kthx.

	if (InfantryClass* pSlave = specific_cast<InfantryClass*>(Slave)) {
		auto Manager = pSlave->SlaveOwner->SlaveManager;

		//LostSlave can free the unit from the miner, so we're awesome.
		Manager->LostSlave(pSlave);
		pSlave->SlaveOwner = nullptr;

		//OK, delinked, Now relink it to the side which separated the slave from the miner
		pSlave->SetOwningHouse(Affector);
	}
}

bool conductAbduction(WeaponTypeExt::ExtData* pData , TechnoClass* pOwner, AbstractClass* pTarget, CoordStruct nTargetCoords) {

	// ensuring a few base parameters
	if (!pData->Abductor || !pOwner || !pTarget) {
		return false;
	}

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pData->OwnerObject()->Warhead);
	const auto Target = abstract_cast<FootClass*>(pTarget);

	if (!Target) {
		// the target was not a valid passenger type
		return false;
	}

	if (nTargetCoords == CoordStruct::Empty)
		nTargetCoords = pTarget->GetCoords();

	const auto Attacker = pOwner;
	const auto TargetType = Target->GetTechnoType();
	const auto AttackerType = Attacker->GetTechnoType();

	if (!pWHExt->CanAffectHouse(Attacker->Owner, Target->GetOwningHouse())) {
		return false;
	}

	if (!TechnoExt::IsAbductable(Attacker, pData->OwnerObject() , Target)) {
		return false;
	}

	//if it's owner meant to be changed, do it here
	HouseClass* pDesiredOwner = Attacker->Owner ? Attacker->Owner : HouseExt::FindSpecial();

	//if it's owner meant to be changed, do it here
	if((pData->Abductor_ChangeOwner && !TechnoExt::IsPsionicsImmune(Target)))
		Target->SetOwningHouse(pDesiredOwner);

	// if we ended up here, the target is of the right type, and the attacker can take it
	// so we abduct the target...

	Target->StopMoving();
	Target->SetDestination(nullptr, true); // Target->UpdatePosition(int) ?
	Target->SetTarget(nullptr);
	Target->CurrentTargets.Clear(); // Target->ShouldLoseTargetNow ?
	Target->SetFocus(nullptr);
	Target->QueueMission(Mission::Sleep, true);
	Target->unknown_C4 = 0; // don't ask
	Target->unknown_5A0 = 0;
	Target->CurrentGattlingStage = 0;
	Target->SetCurrentWeaponStage(0);

	// the team should not wait for me
	if (Target->BelongsToATeam()) {
		Target->Team->LiberateMember(Target);
	}

	// if this unit is being mind controlled, break the link
	if (const auto MindController = Target->MindControlledBy) {
		if (const auto MC = MindController->CaptureManager) {
			MC->FreeUnit(Target);
		}
	}

	// if this unit is a mind controller, break the link
	if (Target->CaptureManager) {
		Target->CaptureManager->FreeAll();
	}

	// if this unit is currently in a state of temporal flux, get it back to our time-frame
	if (Target->TemporalTargetingMe) {
		Target->TemporalTargetingMe->Detach();
	}

	//if the target is spawned, detach it from it's spawner
	if (Target->SpawnOwner) {
		DetachSpecificSpawnee(Target, pDesiredOwner);
	}

	// if the unit is a spawner, kill the spawns
	if (Target->SpawnManager) {
		Target->SpawnManager->KillNodes();
		Target->SpawnManager->ResetTarget();
	}

	//if the unit is a slave, it should be freed
	if (Target->SlaveOwner) {
		FreeSpecificSlave(Target, pDesiredOwner);
	}

	// If the unit is a SlaveManager, free the slaves
	if (auto pSlaveManager = Target->SlaveManager) {
		pSlaveManager->Killed(Attacker);
		pSlaveManager->ZeroOutSlaves();
		Target->SlaveManager->Owner = Target;
	}

	// if we have an abducting animation, play it
	if (auto pAnimType = pData->Abductor_AnimType) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nTargetCoords)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, Attacker->Owner , Target->Owner, Attacker, false);

		}
	}

	Target->Locomotor->Force_Track(-1, CoordStruct::Empty);
	CoordStruct coordsUnitSource = Target->GetCoords();
	Target->Locomotor->Mark_All_Occupation_Bits(0);
	Target->MarkAllOccupationBits(coordsUnitSource);
	Target->ClearPlanningTokens(nullptr);
	Target->Flashing.DurationRemaining = 0;

	if (!Target->Limbo()) {
		Debug::Log("Abduction: Target unit %p (%s) could not be removed.\n", Target, Target->get_ID());
	}

	Target->OnBridge = false;
	Target->NextObject = 0;

	// because we are throwing away the locomotor in a split second, piggybacking
	// has to be stopped. otherwise the object might remain in a weird state.
	while (LocomotionClass::End_Piggyback(Target->Locomotor)) {};

	// throw away the current locomotor and instantiate
	// a new one of the default type for this unit.
	if (auto NewLoco = LocomotionClass::CreateInstance(TargetType->Locomotor)) {
		Target->Locomotor = std::move(NewLoco);
		Target->Locomotor->Link_To_Object(Target);
	}

	// handling for Locomotor weapons: since we took this unit from the Magnetron
	// in an unfriendly way, set these fields here to unblock the unit
	if (Target->IsAttackedByLocomotor || Target->IsLetGoByLocomotor) {
		Target->IsAttackedByLocomotor = false;
		Target->IsLetGoByLocomotor = false;
	}

	Target->Transporter = Attacker;
	if (AttackerType->OpenTopped && Target->Owner->IsAlliedWith(Attacker)) {
		Attacker->EnteredOpenTopped(Target);
	}

	if (Is_Building(Attacker)) {
		Target->Absorbed = true;
	}

	Attacker->AddPassenger(Target);
	Attacker->Undiscover();

	if (auto v29 = Target->AttachedTag)
		v29->RaiseEvent(TriggerEvent(AresTriggerEvents::Abducted_ByHouse), Target, CellStruct::Empty, false, Attacker);

	if (Target->IsAlive) {
		if (auto v30 = Target->AttachedTag)
			v30->RaiseEvent(TriggerEvent(AresTriggerEvents::Abducted), Target, CellStruct::Empty, false, nullptr);
	}

	if (auto v31 = Attacker->AttachedTag)
		v31->RaiseEvent(TriggerEvent(AresTriggerEvents::AbductSomething_OfHouse), Attacker, CellStruct::Empty, false, Target->GetOwningHouse());// pTarget->Owner

	if (Attacker->IsAlive)
	{
		if (auto v32 = Attacker->AttachedTag)
			v32->RaiseEvent(TriggerEvent(AresTriggerEvents::AbductSomething), Attacker, CellStruct::Empty, false, nullptr);
	}

	return true;
}

bool applyOccupantDamage(BulletClass* pThis)
{
	auto const pBuilding = abstract_cast<BuildingClass*>(pThis->Target);

	// if that pointer is null, something went wrong
	if (!pBuilding) {
		return false;
	}

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	auto const occupants = pBuilding->Occupants.Count;
	auto const& passThrough = pBldTypeExt->UCPassThrough;

	if (!occupants || !passThrough) {
		return false;
	}

	auto& Random = ScenarioClass::Instance->Random;
	if (pTypeExt->SubjectToTrenches && Random.RandomDouble() >= passThrough) {
		return false;
	}

	auto const idxPoorBastard = Random.RandomFromMax(occupants - 1);
	auto const pPoorBastard = pBuilding->Occupants[idxPoorBastard];

	auto const& fatalRate = pBldTypeExt->UCFatalRate;
	if (fatalRate > 0.0 && Random.RandomDouble() < fatalRate)
	{
		pPoorBastard->Destroyed(pThis->Owner);
		pPoorBastard->UnInit();
		pBuilding->Occupants.RemoveAt(idxPoorBastard);
		pBuilding->UpdateThreatInCell(pBuilding->GetCell());
	}
	else
	{
		auto const& multiplier = pBldTypeExt->UCDamageMultiplier.Get();
		auto adjustedDamage = static_cast<int>(std::ceil(pThis->Health * multiplier));
		pPoorBastard->ReceiveDamage(&adjustedDamage, 0, pThis->WH, pThis->Owner, false, true, pThis->GetOwningHouse());
	}

	if (pBuilding->FiringOccupantIndex >= pBuilding->GetOccupantCount()) {
		pBuilding->FiringOccupantIndex = 0;
	}

	// if the last occupant was killed and this building was raided,
	// it needs to be returned to its owner. (Bug #700)
	AresData::EvalRaidStatus(pBuilding);

	return true;
}

#include <Misc/PhobosGlobal.h>

DEFINE_OVERRIDE_HOOK(0x46920B, BulletClass_Detonate, 6)
{
	enum { CheckIvanBomb = 0x469343, ConituneMindControlCheck = 0x46921F, SkipEverything = 0x469AA4, Continue = 0x0 };

	GET(BulletClass* const, pThis, ESI);
	GET_BASE(const CoordStruct* const, pCoordsDetonation, 0x8);

	auto const pWarhead = pThis->WH;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead); 
	auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pThis->WeaponType);

	auto const pTechno = pThis->Owner ? pThis->Owner : nullptr;
	auto const pOwnerHouse = pTechno ? pTechno->Owner : nullptr;

	WarheadTypeExt::ExtMap.Find(pThis->WH)->Detonate(pTechno, pOwnerHouse, pThis, *pCoordsDetonation);
	PhobosGlobal::Instance()->DetonateDamageArea = false;

	// this snapping stuff does not belong here. it should go into BulletClass::Fire
	auto coords = *pCoordsDetonation;
	auto snapped = false;

	static auto const SnapDistance = 64;
	if (pThis->Target && pThis->DistanceFrom(pThis->Target) < SnapDistance) {
		coords = pThis->Target->GetCoords();
		snapped = true;
	}

	// these effects should be applied no matter what happens to the target
	//AresData::applyIonCannon(pWarhead , &coords);
	 WarheadTypeExt::CreateIonBlast(pWarhead, coords);

	bool targetStillOnMap = true;
	if (snapped && pWeaponExt && conductAbduction(pWeaponExt, pThis->Owner, pThis->Target, coords)) {
		// ..and neuter the bullet, since it's not supposed to hurt the prisoner after the abduction
		pThis->Health = 0;
		pThis->DamageMultiplier = 0;
		pThis->Limbo();
		targetStillOnMap = false;
	}

	// if the target gets abducted, there's nothing there to apply IC, EMP, etc. to
	// mind that conductAbduction() neuters the bullet, so if you wish to change
	// this check, you have to fix that as well
	if (targetStillOnMap) {

		//auto const damage = pThis->WeaponType ? pThis->WeaponType->Damage : 0;
		//AresData::applyIC(pWarhead, &coords, pOwnerHouse, damage);
		AresData::applyEMP(pWarhead, &coords, pThis->Owner);
		AresData::applyAE(pWarhead, &coords, pOwnerHouse);

		if (snapped && applyOccupantDamage(pThis)) {
			// ..and neuter the bullet, since it's not supposed to hurt the prisoner after the abduction
			pThis->Health = 0;
			pThis->DamageMultiplier = 0;
			pThis->Limbo();
		}
	}

	return pWHExt->applyPermaMC(pOwnerHouse, pThis->Target) ? 0x469AA4u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x71AAAC, TemporalClass_Update_Abductor, 6)
{
	GET(TemporalClass*, pThis, ESI);

	const auto pOwner = pThis->Owner;
	const auto nWeaponIDx = Ares_TemporalWeapon(pThis->Owner);
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	return pWeaponExt->Abductor_Temporal && conductAbduction(pWeaponExt , pOwner, pThis->Target , CoordStruct::Empty)
		? 0x71AAD5 : 0x0;
}

// issue #1437: crash when warping out buildings infantry wants to garrison
DEFINE_OVERRIDE_HOOK(0x71AA52, TemporalClass_Update_AnnounceInvalidPointer, 0x8)
{
	GET(TechnoClass*, pVictim, ECX);
	pVictim->IsAlive = false;
	return 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(0x71A8BD, TemporalClass_Update_WarpAway, 5)
{
	GET(TemporalClass*, pThis, ESI);
	const auto nWeaponIDx = Ares_TemporalWeapon(pThis->Owner);
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;

	const auto pTarget = pThis->Target;

	if(auto pAnimType = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Temporal_WarpAway.Get(RulesClass::Instance()->WarpAway)) {
		if(auto pAnim = GameCreate<AnimClass>(pAnimType,pTarget->Location ,0,1, AnimFlag(0x600),0,0)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->Owner : nullptr, pTarget ? pTarget->Owner : nullptr, pThis->Owner, false);
		}
	}

	return 0x71A90E;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
// TODO :add DP stuffs here
DEFINE_OVERRIDE_HOOK(0x71A917, TemporalClass_Update_Erase, 5)
{
	GET(TemporalClass*, pThis, ESI);

	auto pOwner = pThis->Owner;
	auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;
	auto pOwnerExt = TechnoExt::ExtMap.Find(pOwner);
	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWarheadExt->Supress_LostEva)
		pOwnerExt->SupressEVALost = true;

	return 0x71A97D;
}

int NOINLINE GetWarpPerStep(TemporalClass* pThis, int nStep)
{
	int nAddStep = 0;
	int nStepR = 0;
	
	if (!pThis)
		return 0;

	for (TemporalClass* pTemp = pThis; pTemp; pTemp = pTemp->PrevTemporal)
	{
		if (nStep > 50)
			break;

		++nStep;
		auto const pWeapon = pTemp->Owner->GetWeapon(Ares_TemporalWeapon(pTemp->Owner))->WeaponType;

		//if (auto const pTarget = pTemp->Target)
		//	nStepR = MapClass::GetTotalDamage(pWeapon->Damage, pWeapon->Warhead, pTarget->GetTechnoType()->Armor, 0);
		//else
			nStepR = pWeapon->Damage;

		nAddStep += nStepR;
		pTemp->WarpPerStep = nStepR;
	}

	return nAddStep;
}

DEFINE_OVERRIDE_HOOK(0x71AB10, TemporalClass_GetWarpPerStep, 6)
{
	GET_STACK(int, nStep, 0x4);
	GET(TemporalClass*, pThis, ESI);
	R->EAX(GetWarpPerStep(pThis, nStep));
	return 0x71AB57;
}

// bugfix #874 A: Temporal warheads affect Warpable=no units
// skip freeing captured and destroying spawned units,
// as it is not clear here if this is warpable at all.
DEFINE_OVERRIDE_SKIP_HOOK(0x71AF2B, TemporalClass_Fire_UnwarpableA, 0xA, 71AF4D)

DEFINE_HOOK(0x71AC50, TemporalClass_LetItGo_ExpireEffect, 0x5)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		pTarget->UpdatePlacement(PlacementType::Redraw);

		auto nTotal = pThis->GetWarpPerStep();
		if (nTotal)
		{
			auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;

			if (auto const Warhead = pWeapon->Warhead)
			{
				auto const pTempOwner = pThis->Owner;
				auto const peWHext = WarheadTypeExt::ExtMap.Find(Warhead);

				if (auto pExpireAnim = peWHext->TemporalExpiredAnim.Get())
				{
					auto nCoord = pTarget->GetCenterCoords();

					if (auto const pAnim = GameCreate<AnimClass>(pExpireAnim, nCoord))
					{
						pAnim->ZAdjust = pTarget->GetZAdjustment() - 3;
						AnimExt::SetAnimOwnerHouseKind(pAnim, pTempOwner->GetOwningHouse()
							, pTarget->GetOwningHouse(), pThis->Owner, false);
					}
				}

				if (peWHext->TemporalExpiredApplyDamage.Get())
				{
					auto const pTargetStreght = pTarget->GetTechnoType()->Strength;

					if (pThis->WarpRemaining > 0)
					{
						auto damage = int((pTargetStreght * ((1.0 - pThis->WarpRemaining / 10.0 / pTargetStreght)
							* (pWeapon->Damage * peWHext->TemporalDetachDamageFactor.Get()) / 100)));

						if (pTarget->IsAlive && !pTarget->IsSinking && !pTarget->IsCrashing)
							pTarget->ReceiveDamage(&damage, pTempOwner->DistanceFrom(pTarget), Warhead, pTempOwner, false, ScenarioClass::Instance->Random.RandomBool(), pTempOwner->Owner);
					}
				}
			}
		}
	}

	return 0x71AC5D;
}

DEFINE_OVERRIDE_HOOK(0x71AFB2, TemporalClass_Fire_HealthFactor, 5)
{
	GET(TechnoClass*, pTarget, ECX);
	GET(TemporalClass*, pThis, ESI);
	GET(int, nStreght, EAX);

	auto const pWeapon = pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))->WeaponType;;
	const auto pWarhead = pWeapon->Warhead;
	const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	const double nCalc = (1.0 - pTarget->Health / pTarget->GetTechnoType()->Strength) * pWarheadExt->Temporal_HealthFactor.Get();
	const double nCalc_b = (1.0 - nCalc) * (10 * nStreght) + nCalc * 0.0;

	R->EAX(nCalc_b <= 1.0 ? 1 : static_cast<int>(nCalc_b));
	return 0x71AFB7;
}

bool NOINLINE Warpable(TechnoClass* pTarget)
{
	if (!pTarget || pTarget->IsSinking || pTarget->IsCrashing || pTarget->IsIronCurtained())
		return false;

	if (TechnoExt::IsUnwarpable(pTarget))
		return false;

	if (Is_Building(pTarget))
	{
		if (Ares_AboutToChronoshift(pTarget))
		{
			return false;
		}
	}
	else
	{
		if (TechnoExt::IsInWarfactory(pTarget , true))
			return false;

		if (TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTarget)))
			return false;
	}

	return true;
}

DEFINE_OVERRIDE_HOOK(0x71AE50, TemporalClass_CanWarpTarget, 8)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);
	R->EAX(Warpable(pTarget));
	return 0x71AF19;
}

DEFINE_OVERRIDE_HOOK(0x71944E, TeleportLocomotionClass_ILocomotion_Process, 6)
{
	GET(FootClass*, pObject, ECX);
	GET(CoordStruct*, XYZ, EDX);
	*XYZ = pObject->GetCoords();
	R->EAX<CoordStruct*>(XYZ);

	if (auto pType = pObject->GetTechnoType())
	{
		if (const auto pImage = pType->AlphaImage)
		{
			Point2D xy;
			TacticalClass::Instance->CoordsToClient(XYZ, &xy);
			RectangleStruct ScreenArea = TacticalClass::Instance->VisibleArea();
			Point2D off = { ScreenArea.X - (pImage->Width / 2), ScreenArea.Y - (pImage->Height / 2) };
			xy += off;
			RectangleStruct Dirty =
			{ xy.X - ScreenArea.X, xy.Y - ScreenArea.Y,
			  pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}

	return 0x719454;
}

DEFINE_HOOK(0x71AFD0 , TemporalClass_Logic_Unit_OreMinerUnderAttack, 0x5)
{
	GET(TemporalClass* , pThis ,ESI);

	if(auto pTarget = (UnitClass*)pThis->Target) {
		if(pTarget->Type->Harvester) {
			const auto nWeaponIDx = Ares_TemporalWeapon(pThis->Owner);
			auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
			const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

			if(!pWHExt->Malicious && pTarget->Owner == HouseClass::CurrentPlayer) {
				auto nDest = pTarget->GetDestination();
				if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(nDest))) {
					VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack(), -1, -1);
				}
			}
		}
	}

	return 0x071B064;
}

DEFINE_HOOK(0x71B0AE, TemporalClass_Logic_Building_UnderAttack, 0x7)
{
	GET(TemporalClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	BuildingExt::ExtMap.Find(pBld)->ReceiveDamageWarhead = 
		pThis->Owner->GetWeapon(Ares_TemporalWeapon(pThis->Owner))
			 ->WeaponType->Warhead;

	return 0x0;
}