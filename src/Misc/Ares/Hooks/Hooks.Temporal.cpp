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
#include <Ext/TechnoType/Body.h>
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

// TODO : still not correct !
//DEFINE_HOOK(718279, TeleportLocomotionClass_MakeRoom, 5)
//{
//	GET(CoordStruct*, pCoord, EAX);
//	GET(TeleportLocomotionClass*, pLoco, EBP);
//
//	auto const pLinked = pLoco->LinkedTo;
//	auto const pLinkedIsInf = pLinked->WhatAmI() == AbstractType::Infantry;
//	auto const pCell = Map.TryGetCellAt(*pCoord);
//
//	R->Stack(0x48, false);
//	R->EBX(pCell->OverlayTypeIndex);
//	R->EDI(0);
//
//	for (NextObject obj(pCell->GetContent()); obj; ++obj)
//	{
//
//		auto const pObj = (*obj);
//		auto const bIsObjFoot = pObj->AbstractFlags & AbstractFlags::Foot;
//		auto const pObjIsInf = pObj->WhatAmI() == AbstractType::Infantry;
//		auto bIsObjectInvicible = pObj->IsIronCurtained();
//		auto const pType = pObj->GetTechnoType();
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
//
//		if (pType && !pTypeExt->Chronoshift_Crushable)
//			bIsObjectInvicible = true;
//
//		if (!bIsObjectInvicible && pObjIsInf && pLinkedIsInf)
//		{
//			auto const bEligible = pLinked->Owner && !pLinked->Owner->IsAlliedWith(pObj);
//			auto const pAttackerHouse = bEligible ? pLinked->Owner : nullptr;
//			auto const pAttackerTechno = bEligible ? pLinked : nullptr;
//
//			auto nCoord = pObj->GetCoords();
//			if (nCoord == *pCoord)
//			{
//				auto nDamage = pObj->Health;
//				pObj->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//			}
//		}
//		else if (bIsObjectInvicible || !bIsObjFoot)
//		{
//			if (bIsObjectInvicible)
//			{
//				auto const pObjHouse = pObj->GetOwningHouse();
//				auto const pAttackerHouse = pObjHouse && !pObjHouse->IsAlliedWith(pObj) ? pObjHouse : nullptr;
//				auto const pAttackerTechno = reinterpret_cast<TechnoClass*>(pObj);
//
//				auto nDamage = pLinked->Health;
//				pLinked->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//			}
//			else if (!bIsObjFoot)
//			{
//				R->Stack(0x48, true);
//			}
//		}
//		else
//		{
//
//			auto const bEligible = pLinked->Owner && !pLinked->Owner->IsAlliedWith(pObj);
//			auto const pAttackerHouse = bEligible ? pLinked->Owner : nullptr;
//			auto const pAttackerTechno = bEligible ? pLinked : nullptr;
//			auto nDamage = pObj->Health;
//			pObj->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pAttackerTechno, true, false, pAttackerHouse);
//		}
//	}
//
//	auto const nFlag300 = CellFlags::Bridge | CellFlags::Unknown_200;
//	if ((pCell->Flags & nFlag300) == CellFlags::Bridge)
//		R->Stack(0x48, true);
//
//	R->Stack(0x20, pLinked->GetMapCoords());
//	R->EAX(true);
//
//	return 0x7184CE;
//}

#include <Ext/House/Body.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>

bool IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger)
{
	auto pThisType = pThis->GetTechnoType();
	auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);
	auto pThatType = pPassanger->GetTechnoType();

	if (pThatType->Size > pThisType->SizeLimit)
		return false;

	if (pThisTypeExt->Passengers_BySize.Get())
	{
		if (pThatType->Size > (pThisType->Passengers - pThis->Passengers.GetTotalSize()))
			return false;
	}
	else if (pThis->Passengers.NumPassengers >= pThisType->Passengers)
	{
		return false;
	}

	return true;
}

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

	if (Target->InLimbo || 
		Target->IsIronCurtained() || 
		Target->IsSinking || 
		!Target->IsAlive || 
		Is_DriverKilled(Target)) {
		return false;
	}

	//issue 1362
	if (TechnoExt::IsAbductorImmune(Target)) {
		return false;
	}

	if(pWHExt->CanAffectHouse(Attacker->Owner ,Target->GetOwningHouse())) {
		return false;
	}

	//Don't abduct the target if it has more life then the abducting percent
	if (pData->Abductor_AbductBelowPercent < Target->GetHealthPercentage()) {
		return false;
	}

	if (!TechnoTypeExt::PassangersAllowed(AttackerType, TargetType)) {
		return false;
	}

	// Don't abduct the target if it's too fat in general, or if there's not enough room left in the hold // alternatively, NumPassengers
	if (!IsEligibleSize(Attacker, Target)) {
		return false;
	}

	HouseClass* pDesiredOwner = nullptr;
	//if it's owner meant to be changed, do it here
	if (pData->Abductor_ChangeOwner && !TechnoExt::IsPsionicsImmune(Target)) {
		pDesiredOwner = (Attacker->Owner);
	}else {
		pDesiredOwner = HouseExt::FindSpecial();
	}

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
			pAnim->Owner = Attacker->Owner;
		}
	}

	Target->Locomotor->Force_Track(-1, CoordStruct::Empty);
	CoordStruct coordsUnitSource = Target->GetCoords();
	Target->Locomotor->Mark_All_Occupation_Bits(0);
	Target->MarkAllOccupationBits(coordsUnitSource);
	Target->ClearPlanningTokens(nullptr);
	Target->Flashing.DurationRemaining = 0;

	//if it's owner meant to be changed, do it here
	Target->SetOwningHouse(pDesiredOwner);

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

DEFINE_OVERRIDE_HOOK(0x71AAAC, TemporalClass_Update_Abductor, 6)
{
	GET(TemporalClass*, pThis, ESI);

	const auto pOwner = pThis->Owner;
	const auto nWeaponIDx = Ares_TemporalWeapon(pThis->Owner);
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	return conductAbduction(pWeaponExt , pOwner, pThis->Target , CoordStruct::Empty)
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
DEFINE_OVERRIDE_HOOK(0x71A900, TemporalClass_Update_WarpAway, 6)
{
	GET(TemporalClass*, pThis, ESI);
	const auto nWeaponIDx = Ares_TemporalWeapon(pThis->Owner);
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
	R->EDX<AnimTypeClass*>(WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Temporal_WarpAway.Get(RulesClass::Global()->WarpAway));
	return 0x71A906;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
// 
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

	if (TechnoExt::HasAbility(pTarget, PhobosAbilityType::Unwarpable))
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
