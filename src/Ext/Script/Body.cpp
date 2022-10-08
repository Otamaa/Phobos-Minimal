#include "Body.h"

#include <Utilities/Cast.h>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Ext/Scenario/Body.h>
#include <Ext/ScriptType/Body.h>

#define TECHNO_IS_ALIVE(tech) TechnoExt::IsAlive(tech)
#ifdef ENABLE_NEWHOOKS
ScriptExt::ExtContainer ScriptExt::ExtMap;

void ScriptExt::ExtData::InitializeConstants() { }
#endif

ScriptActionNode ScriptExt::GetSpecificAction(ScriptClass* pScript, int nIdx)
{
	nIdx += pScript->CurrentMission;

	if (nIdx > pScript->Type->ActionsCount)
		nIdx = pScript->Type->ActionsCount;

	if (nIdx <= 49) {
		return pScript->Type->ScriptActions[nIdx];
	}
	//else {
		//auto const pTypeExt = ScriptTypeExt::ExtMap.Find(pScript->Type);
		//if (pTypeExt && !pTypeExt->PhobosNode.empty()) {
		//	return pTypeExt->PhobosNode[nIdx - 50];
		//}
	//}

	return pScript->Type->ScriptActions[49];
}

static inline bool IsEmpty(TeamClass* pTeam)
{
	int nCount = 0;

	for (auto const& UnitCount : pTeam->CountObjects)
		nCount += UnitCount;

	return nCount <= 0;
}
#ifdef ENABLE_NEWHOOKS
// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ScriptClass>::Serialize(Stm);
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ScriptClass>::Serialize(Stm);
	// Nothing yet
}

bool ScriptExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ScriptExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}
// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass") { }
void ScriptExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

ScriptExt::ExtContainer::~ExtContainer() = default;
#endif
/*
#include <SlaveManagerClass.h>

namespace AresHouseExt
{
	struct Data
	{
		 Data(HouseClass* pWho) :OwnerObject { pWho }
		 { }

		HouseClass* OwnerObject { nullptr };
		int TeamPowerAdd { 0 };

	};

	struct ExtMap
	{
		static AresHouseExt::Data* Find(HouseClass* Own)
		{
			return GameCreate<AresHouseExt::Data>(Own);
		}
	};
};

namespace AresTechnoExt
{
	struct Data
	{
		Data(TechnoClass* pWho) :OwnerObject { pWho }
		{ }

		TechnoClass* OwnerObject { nullptr };
		bool DriverKilled { false };
		TimerStruct CloakSkipTimer { };
		TimerStruct DisableWeaponTimer { };
	};

	struct ExtMap
	{
		static AresTechnoExt::Data* Find(TechnoClass* Own)
		{
			return GameCreate<AresTechnoExt::Data>(Own);
		}
	};
};

namespace TechnoTypeExtAres
{
	Nullable<double> ProtectedTreshold { };
	Valueable<bool> Protected { false };
	Promotable<bool> ProtectedRiver { false };
	Valueable<TechnoTypeClass*> Convert_Script { nullptr };
	Valueable<TechnoTypeClass*> Convert_Land { nullptr };
	Valueable<TechnoTypeClass*> Convert_Water { nullptr };

}

bool IsKillDriverAdvisable(FootClass* pThis, double HPTreshold)
{
	if (pThis->WhatAmI() == AbstractType::Infantry ||
		pThis->GetTechnoType()->Natural ||
		pThis->GetTechnoType()->Organic ||
		pThis->BeingWarpedOut ||
		pThis->IsIronCurtained()
		) {
		return false;
	}

	if (pThis->WhatAmI() == AbstractType::Aircraft) {
		auto const pAir = static_cast<AircraftClass*>(pThis);
		if (pAir->Type->AirportBound || pAir->Type->Dock.Count)
			return false;

	}else if (pThis->WhatAmI() == AbstractType::Unit) {

		if (const auto pBuilding = pThis->GetCell()->GetBuilding()) {
			if (pBuilding == pThis->GetRadioContact()) {
				auto const pType = pBuilding->Type;
				if (pType->WeaponsFactory && !pType->Naval)
					return false;
			}
		}
	}

	if (TechnoTypeExtAres::ProtectedRiver.Get(pThis))
		return false;

	const auto nOtherTresh = TechnoTypeExtAres::ProtectedTreshold.Get(TechnoTypeExtAres::Protected ? 0.0 : 1.0);

	if (nOtherTresh <= HPTreshold)
		HPTreshold = nOtherTresh;

	//Check if HP is good ?
	if (pThis->GetHealthPercentage() > HPTreshold)
		return false;

	return true;
}

bool ConvertType(TechnoClass* pThis, TechnoTypeClass* pToType)
{
	AbstractType nCurType = AbstractType::None;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
	{
		nCurType = AbstractType::InfantryType;
		break;
	}
	case AbstractType::Unit:
	{
		nCurType = AbstractType::UnitType;
		break;
	}
	case AbstractType::Aircraft:
	{
		nCurType = AbstractType::AircraftType;
		break;
	}
	default:
		return false;
	}

	if (nCurType == AbstractType::None || nCurType != pToType->WhatAmI())
		return false;

	if (auto pTemp = pThis->TemporalImUsing)
		if (pTemp->Target)
			pTemp->Detach();

	auto pOwner = pThis->Owner;

	if (!pThis->InLimbo)
	{
		pOwner->RegisterLoss(pThis, false);
	}

	pOwner->RemoveTracking(pThis);

	auto nHealth = pThis->Health/ pThis->GetType()->Strength ;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry: {
		static_cast<InfantryClass*>(pThis)->Type = static_cast<InfantryTypeClass*>(pToType);
		break;
	}
	case AbstractType::Unit: {
		static_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pToType);
		break;
	}
	case AbstractType::Aircraft: {
		static_cast<AircraftClass*>(pThis)->Type = static_cast<AircraftTypeClass*>(pToType);
		break;
	}
	}

	pThis->AdjustStrength(nHealth); //health adjusted after type change
	pThis->EstimatedHealth = pThis->Health;
	pOwner->AddTracking(pThis);

	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, true);

	pOwner->RecheckTechTree = true;

	//clearing some AresStateHere
	//
	//

	//ammo
	auto nNewAmmo = pToType->Ammo;
	if (nNewAmmo >= pThis->Ammo)
		nNewAmmo = pThis->Ammo;

	pThis->Ammo = nNewAmmo;

	//Update Ares BuildingLightClass

	//ROT
	pThis->PrimaryFacing.turn_rate(pToType->ROT);

	//Set Turrent Facing Here

	//Replace Locomotor
	GUID nCurID { };
	static_cast<LocomotionClass*>(static_cast<FootClass*>(pThis)->Locomotor.get())->GetClassID(&nCurID);

	if (nCurID != pToType->Locomotor)
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(static_cast<FootClass*>(pThis)->Locomotor)) { };

		if (auto NewLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
		{
			static_cast<FootClass*>(pThis)->Locomotor = std::move(NewLoco);
			static_cast<FootClass*>(pThis)->Locomotor->Link_To_Object(pThis);
		}

		// handling for Locomotor weapons: since we took this unit from the Magnetron
		// in an unfriendly way, set these fields here to unblock the unit
		if (static_cast<FootClass*>(pThis)->IsAttackedByLocomotor || static_cast<FootClass*>(pThis)->IsLetGoByLocomotor)
		{
			static_cast<FootClass*>(pThis)->IsAttackedByLocomotor = false;
			static_cast<FootClass*>(pThis)->IsLetGoByLocomotor = false;
			static_cast<FootClass*>(pThis)->FrozenStill = false;
		}
	}

	return true;
}

void TechnoClassUpdate(TechnoClass* pThis)
{
	auto const pCurType = pThis->GetTechnoType();

	if (TechnoTypeExtAres::Convert_Land || TechnoTypeExtAres::Convert_Water)
	{
		if (pThis->WhatAmI() != AbstractType::Aircraft || pThis->WhatAmI() != AbstractType::Building)
		{
			auto pNewType = pThis->OnBridge ? TechnoTypeExtAres::Convert_Land : TechnoTypeExtAres::Convert_Water;
			if (pCurType && pCurType != pNewType)
				ConvertType(pThis, pNewType);
		}
	}
}

bool KillTheDriver(TechnoClass* pVictim, TechnoClass* pDestroyer, HouseClass* pHouseAfter)
{
	if (!((pVictim->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		return false;

	auto pVictimExt = TechnoExt::ExtMap.Find(pVictim);

	auto const passive = pHouseAfter->Type->MultiplayPassive;
	pVictimExt->DriverKilled = passive;

	// exit if owner would not change
	if (pVictim->Owner == pHouseAfter) {
		return false;
	}
	auto pType = pVictim->GetTechnoType();
	auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto pTarget = pVictim;
	auto pSource = pDestroyer;
	// If this vehicle uses Operator=, we have to take care of actual "physical" drivers, rather than theoretical ones
	if (pVictimTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt && pVictim->Passengers.GetFirstPassenger())
	{
		// kill first passenger
		auto const pPassenger = pVictim->RemoveFirstPassenger();
		pPassenger->RegisterDestruction(pDestroyer);
		pPassenger->UnInit();
	}
	else if (auto const pOperatorType = pVictimTypeExt->Operator)
	{
		// find the driver cowardly hiding among the passengers, then kill him
		for (NextObject passenger(pVictim->Passengers.GetFirstPassenger()); passenger; ++passenger)
		{
			auto const pPassenger = static_cast<FootClass*>(*passenger);
			if (pPassenger->GetTechnoType() == pOperatorType)
			{
				pVictim->RemovePassenger(pPassenger);
				pPassenger->RegisterDestruction(pDestroyer);
				pPassenger->UnInit();
				break;
			}
		}
	}
	// if passengers remain in the vehicle, operator-using or not, they should leave
	if (pVictim->Passengers.GetFirstPassenger())
	{
		TechnoExt::EjectPassengers(pVictim, -1);
	}
	pTarget->HijackerInfantryType = -1;
	// If this unit is driving under influence, we have to free it first
	if (auto const pController = pTarget->MindControlledBy)
	{
		if (auto const pCaptureManager = pController->CaptureManager)
		{
			pCaptureManager->FreeUnit(pTarget);
		}
	}
	pTarget->MindControlledByAUnit = false;
	pTarget->MindControlledByHouse = nullptr;
	// remove the mind-control ring anim
	if (pTarget->MindControlRingAnim)
	{
		pTarget->MindControlRingAnim->UnInit();
		pTarget->MindControlRingAnim = nullptr;
	}
	// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
	if (pTarget->CaptureManager)
	{
		pTarget->CaptureManager->FreeAll();
	}
	// This unit will be freed of its duties
	static_cast<FootClass*>(pTarget)->LiberateMember();
	// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
	if (auto const pSpawnManager = pTarget->SpawnManager)
	{
		pSpawnManager->KillNodes();
		pSpawnManager->ResetTarget();
	}
	// If this unit enslaves stuff, we should free the slaves, since they still belong to the previous owner
			// <DCoder> SlaveManagerClass::Killed() sets the manager's Owner to NULL
			// <Renegade> okay, does Killed() also destroy the slave manager, or just unlink it from the unit?
			// <DCoder> unlink
			// <Renegade> so on principle, I could just re-link it?
			// <DCoder> yes you can
	if (auto const pSlaveManager = pTarget->SlaveManager)
	{
		pSlaveManager->Killed(pSource);
		pSlaveManager->ZeroOutSlaves();
		pSlaveManager->Owner = pTarget;
		if (passive)
		{
			pSlaveManager->SuspendWork();
		}
		else
		{
			pSlaveManager->ResumeWork();
		}
	}
	// Hand over to a different house
	pTarget->SetOwningHouse(pHouseAfter);
	if (passive) {
		pTarget->QueueMission(Mission::Harmless, true);
	}

	pTarget->SetTarget(nullptr);
	pTarget->SetDestination(nullptr, false);

	if (auto pTag = pTarget->AttachedTag) {
		pTag->RaiseEvent(static_cast<TriggerEvent>(0x44), pTarget, CellStruct::Empty, false, pDestroyer); //new
	}

	if(pTarget->IsAlive) {
		if (auto pTag = pTarget->AttachedTag) {
			pTag->RaiseEvent(static_cast<TriggerEvent>(0x43), pTarget, CellStruct::Empty, false, nullptr); //new
		}
	}

	return true;
}

bool ProcessAction_Ares(TeamClass* pTeam , ScriptActionNode nNode)
{
	auto const& [action, argument] = nNode;
	enum class AresScripts : int
	{
		AuxPower = 65 ,
		KillDriver = 66,
		TakeVehicle = 67,
		ConvertType = 68 ,
		DisableWeapon = 69 ,
		SonarReveal
	};

	//ares reuse this

	if (action > 64)
	{
		switch (static_cast<AresScripts>(action))
		{
		case AresScripts::AuxPower:
		{
			auto const pOwner = pTeam->Owner;
			auto const pExt = AresHouseExt::ExtMap::Find(pOwner);
			pExt->TeamPowerAdd += argument;
			pOwner->RecheckPower = 1;
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::KillDriver:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (pCur->Health > 0 && pCur->IsAlive && pCur->IsOnMap && !pCur->InLimbo)
				{
					auto pExt = AresTechnoExt::ExtMap::Find(pCur);
					if (!pExt->DriverKilled)
					{
						if (IsKillDriverAdvisable(pCur,1.0))
						{
							//KillTheDriver(pCur,nullptr,HouseExt::FindSpecial());
						}
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::TakeVehicle:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				//auto pUnitExt = TechnoExt::ExtMap.Find(pCur);
				//pUnitExt->TakeVehicle = true;//used on GarrisonStructure function hook , if yes replace it wit take vehicle func
				//if (pCur->GarrisonStructure()) {
				//	pTeam->LiberateMember(pCur,-1,1);
				//}
			}

			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::ConvertType:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pConvertType = TechnoTypeExtAres::Convert_Script.Get()) {
					ConvertType(pCur, pConvertType);
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::SonarReveal:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pExt = AresTechnoExt::ExtMap::Find(pCur))
				{
					auto const delay = Math::max(pExt->CloakSkipTimer.GetTimeLeft(), argument);
					pExt->CloakSkipTimer.Start(delay);

					// actually detect this
					if (pCur->CloakState != CloakState::Uncloaked)
					{
						pCur->Uncloak(true);
						pCur->NeedsRedraw = true;
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::DisableWeapon:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pExt = AresTechnoExt::ExtMap::Find(pCur))
				{
					auto const delay = Math::max(pExt->DisableWeaponTimer.GetTimeLeft(), argument);
					pExt->DisableWeaponTimer.Start(delay);

					// actually detect this
					if (pCur->CloakState != CloakState::Uncloaked)
					{
						pCur->Uncloak(true);
						pCur->NeedsRedraw = true;
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		}
	}else
	if (action == 64)
	{
		for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
		{
			if (pCur->GarrisonStructure()) {
				pTeam->LiberateMember(pCur,-1,1);
			}
		}
		pTeam->StepCompleted = 1;
		return true;
	}

	return false;
}*/

void ScriptExt::ProcessAction(TeamClass* pTeam)
{
	auto const&[action, argument] = pTeam->CurrentScript->GetCurrentAction();

	switch (static_cast<PhobosScripts>(action))
	{
	case PhobosScripts::TimedAreaGuard:
		ScriptExt::ExecuteTimedAreaGuardAction(pTeam);
		break;
	case PhobosScripts::LoadIntoTransports:
		ScriptExt::LoadIntoTransports(pTeam);
		break;
	case PhobosScripts::WaitUntilFullAmmo:
		ScriptExt::WaitUntilFullAmmoAction(pTeam);
		break;
	case PhobosScripts::RepeatAttackCloserThreat:
		// Threats that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 0, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFartherThreat:
		// Threats that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 1, -1, -1);
		break;
	case PhobosScripts::RepeatAttackCloser:
		// Closer targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 2, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFarther:
		// Farther targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 3, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloserThreat:
		// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 0, -1, -1);
		break;
	case PhobosScripts::SingleAttackFartherThreat:
		// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 1, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloser:
		// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 2, -1, -1);
		break;
	case PhobosScripts::SingleAttackFarther:
		// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 3, -1, -1);
		break;
	case PhobosScripts::DecreaseCurrentAITriggerWeight:
		ScriptExt::DecreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::IncreaseCurrentAITriggerWeight:
		ScriptExt::IncreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::RepeatAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 0, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 1, -1);
		break;
	case PhobosScripts::RepeatAttackTypeCloser:
		// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFarther:
		// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 3, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 0, -1);
		break;
	case PhobosScripts::SingleAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 1, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloser:
		// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 2, -1);
		break;
	case PhobosScripts::SingleAttackTypeFarther:
		// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 3, -1);
		break;
	case PhobosScripts::WaitIfNoTarget:
		ScriptExt::WaitIfNoTarget(pTeam, -1);
		break;
	case PhobosScripts::TeamWeightReward:
		ScriptExt::TeamWeightReward(pTeam, 0);
		break;
	case PhobosScripts::PickRandomScript:
		ScriptExt::PickRandomScript(pTeam, -1);
		break;
	case PhobosScripts::MoveToEnemyCloser:
		// Move to the closest enemy target
		ScriptExt::Mission_Move(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::MoveToEnemyFarther:
		// Move to the farther enemy target
		ScriptExt::Mission_Move(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyCloser:
		// Move to the closest friendly target
		ScriptExt::Mission_Move(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyFarther:
		// Move to the farther friendly target
		ScriptExt::Mission_Move(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyCloser:
		// Move to the closest specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 2, false, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyFarther:
		// Move to the farther specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 3, false, -1);
		break;
	case PhobosScripts::MoveToTypeFriendlyCloser:
		// Move to the closest specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 2, true, -1);
		break;
	case PhobosScripts::MoveToTypeFriendlyFarther:
		// Move to the farther specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 3, true, -1);
		break;
	case PhobosScripts::ModifyTargetDistance:
		// AISafeDistance equivalent for Mission_Move()
		ScriptExt::SetCloseEnoughDistance(pTeam, -1);
		break;
	case PhobosScripts::RandomAttackTypeCloser:
		// Pick 1 closer random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RandomAttackTypeFarther:
		// Pick 1 farther random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 3, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyCloser:
		// Pick 1 closer enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyFarther:
		// Pick 1 farther enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		// Pick 1 closer friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		// Pick 1 farther friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::SetMoveMissionEndMode:
		// Set the condition for ending the Mission_Move Actions.
		ScriptExt::SetMoveMissionEndMode(pTeam, -1);
		break;
	case PhobosScripts::UnregisterGreatSuccess:
		// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
		ScriptExt::UnregisterGreatSuccess(pTeam);
		break;
	case PhobosScripts::GatherAroundLeader:
		ScriptExt::Mission_Gather_NearTheLeader(pTeam, -1);
		break;
	case PhobosScripts::RandomSkipNextAction:
		ScriptExt::SkipNextAction(pTeam, -1);
		break;
	case PhobosScripts::StopForceJumpCountdown:
		// Stop Timed Jump
		ScriptExt::Stop_ForceJump_Countdown(pTeam);
		break;
	case PhobosScripts::NextLineForceJumpCountdown:
		// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, false, -1);
		break;
	case PhobosScripts::SameLineForceJumpCountdown:
		// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, true, -1);
		break;
	case PhobosScripts::ForceGlobalOnlyTargetHouseEnemy:
		ScriptExt::ForceGlobalOnlyTargetHouseEnemy(pTeam, -1);
		break;
	case PhobosScripts::ChangeTeamGroup:
		ScriptExt::TeamMemberSetGroup(pTeam, argument);
		break;
	case PhobosScripts::DistributedLoading:
		ScriptExt::DistributedLoadOntoTransport(pTeam, argument);
		break;
	case PhobosScripts::FollowFriendlyByGroup:
		ScriptExt::FollowFriendlyByGroup(pTeam, argument);
		break;
	case PhobosScripts::RallyUnitWithSameGroup:
		ScriptExt::RallyUnitInMap(pTeam, argument);
		break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (action > 70 && !IsExtVariableAction(action))
		{
			// Unknown new action. This action finished
			pTeam->StepCompleted = true;
			Debug::Log("[%s] [%s] (line %d): Unknown Script Action: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->GetCurrentAction().Action);
		}
		break;
	}

	if (IsExtVariableAction(action))
		VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
}

#pragma region TransportStuffs

void ScriptExt::TeamMemberSetGroup(TeamClass* pTeam, int group)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		pUnit->Group = group;
		pTeam->AddMember(pUnit, false);
	}

	pTeam->StepCompleted = true;
}

bool ScriptExt::StopTeamMemberMoving(TeamClass* pTeam)
{
	bool stillMoving = false;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->WhatAmI() == AbstractType::Unit)
			if (auto pCell = pUnit->GetCell())
				if (auto pBld = pCell->GetBuilding())
					if (!pBld->Type->Naval && pBld->Type->WeaponsFactory && !pUnit->IsInAir())
						continue;

		if (pUnit->CurrentMission == Mission::Move || pUnit->Locomotor->Is_Moving())
		{
			pUnit->ForceMission(Mission::Wait);
			pUnit->CurrentTargets.Clear();
			pUnit->SetTarget(nullptr);
			pUnit->SetFocus(nullptr);
			pUnit->SetDestination(nullptr, true);
			stillMoving = true;
		}
	}
	return stillMoving;
}

void ScriptExt::DistributedLoadOntoTransport(TeamClass* pTeam, int nArg)
{
	const int T_NO_GATHER = 0, T_COUNTDOWN = 1, T_AVGPOS = 2;
	/*
	type 0: stop member from gathering and begin load now
	type 1: don't stop, maintain previous action, countdown LOWORD seconds then begin load
	type 2: don't stop, gether around first member's position range LOWORD then begin load
	*/

	// can proceed to load calculate logic
	const int R_CAN_PROCEED = 1;
	// type 1 waiting stage, start timer and wait timer stop, when stop, proceed to loading
	const int R_WAITING = 2;
	// type 2 waiting stage, start timer and wait timer stop, when stop, check average position
	const int R_WAIT_POS = 3;

	int nType = HIWORD(nArg);
	int nNum = LOWORD(nArg);
	auto pExt = TeamExt::ExtMap.Find(pTeam);

	if (!pExt) {
		pTeam->StepCompleted = true;
		return;
	}

	const auto remainingSize = [](FootClass* src) {
		auto type = src->GetTechnoType();
		return type->Passengers - src->Passengers.GetTotalSize();
	};

	auto pFoot = pTeam->FirstUnit;

	if (!pFoot) {
		pTeam->StepCompleted = true;
		return;
	}

	auto timer = pFoot->BlockagePathTimer;
	// Wait for timer stop
	if (pTeam->GuardAreaTimer.TimeLeft > 0)
	{
		pTeam->GuardAreaTimer.TimeLeft--;
		return;
	}
	// type 1 times up
	else if (pExt->GenericStatus == R_WAITING)
	{
		bool stillMoving = StopTeamMemberMoving(pTeam);
		if (!stillMoving)
		{
			pExt->GenericStatus = R_CAN_PROCEED;
			goto beginLoad;
		}
	}
	// type 2 times up, check distance
	else if (pExt->GenericStatus == R_WAIT_POS)
	{
		bool canProceed = true;
		auto pCell = pFoot->GetCell();

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			// no blockage, just keep moving
			if (pUnit->Locomotor->Is_Moving_Now())
			{
				canProceed = false;
				continue;
			}

			if (pUnit->DistanceFrom(pFoot) / 256 <= nNum)
			{
				pUnit->StopMoving();
				pUnit->CurrentTargets.Clear();
				pUnit->SetTarget(nullptr);
				pUnit->SetFocus(nullptr);
				pUnit->SetDestination(nullptr, true);
			}
			else
			{
				auto nCoord = pCell->GetCoords();
				pUnit->MoveTo(&nCoord);
				canProceed = false;
			}
		}
		if (canProceed)
		{
			pExt->GenericStatus = R_CAN_PROCEED;
			return;
		}
		else
		{
			pTeam->GuardAreaTimer.Start(5);
			return;
		}
	}

	// Check if this script can skip this frame
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		// If anyone is entering transport, means this script is now proceeding
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
	}

	// Status jump
	if (pExt->GenericStatus == R_CAN_PROCEED)
		goto beginLoad;

	// switch mode
	// 0: stop member from gathering
	if (nType == T_NO_GATHER)
	{
		// If anyone is moving, stop now, and add timer
		// why team member will converge upon team creation
		// fuck you westwood
		pTeam->Focus = nullptr;
		pTeam->QueuedFocus = nullptr;
		bool stillMoving = StopTeamMemberMoving(pTeam);
		if (stillMoving)
		{
			pTeam->GuardAreaTimer.Start(45);
			return;
		}
	}
	// 1: don't stop, maintain previous action
	// default: gather near team center position (upon team creation, it's auto), countdown timer LOWORD seconds
	else if (nType == T_COUNTDOWN)
	{
		if (pExt->GenericStatus == R_WAITING)
			return;
		pTeam->GuardAreaTimer.Start(nNum * 15);
		pExt->GenericStatus = R_WAITING;
		return;
	}
	// 2: don't stop, manually gather around first member's position
	else if (nType == T_AVGPOS)
	{
		auto pCell = pFoot->GetCell();
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto nCoord = pCell->GetCoords();
			pUnit->MoveTo(&nCoord);
		}
		pTeam->GuardAreaTimer.Start(5);
		pExt->GenericStatus = R_WAIT_POS;
		return;
	}

beginLoad:
	// Now we're talking
	std::vector<FootClass*> transports, passengers;
	std::unordered_map<FootClass*, double> transportSpaces;
	// Find max SizeLimit to determine which type is considered as transport
	double maxSizeLimit = 0;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pType = pUnit->GetTechnoType();
		maxSizeLimit = std::max(maxSizeLimit, pType->SizeLimit);
	}
	// No transports remaining
	if (maxSizeLimit == 0)
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
		return;
	}
	// All member share this SizeLimit will consider as transport
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pType = pUnit->GetTechnoType();
		if (pType->SizeLimit == maxSizeLimit)
		{
			int space = remainingSize(pUnit);
			transports.push_back(pUnit);
			transportSpaces[pUnit] = space;
		}
		else
			passengers.push_back(pUnit);
	}
	// If there are no passengers
	// then this script is done
	if (passengers.empty())
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
		return;
	}
	// If transport is on building, scatter, and discard this frame
	for (auto const& pUnit : transports)
	{
		if (pUnit->GetCell()->GetBuilding())
		{
			pUnit->Scatter(pUnit->GetCoords(), true, false);
			return;
		}
	}

	// Load logic
	// range prioritize
	bool passengerLoading = false;
	// larger size first
	std::sort(passengers.begin(), passengers.end(), [](FootClass* const a, FootClass* const b) {
		return a->GetTechnoType()->Size > b->GetTechnoType()->Size;
	});

	for (auto const& pPassenger : passengers)
	{
		auto pPassengerType = pPassenger->GetTechnoType();
		// Is legal loadable unit ?
		if (pPassengerType->WhatAmI() != AbstractType::AircraftType &&
			!pPassengerType->ConsideredAircraft &&
			TECHNO_IS_ALIVE(pPassenger))
		{
			FootClass* targetTransport = nullptr;
			double distance = INFINITY;
			for (auto pTransport : transports)
			{
				auto pTransportType = pTransport->GetTechnoType();
				double currentSpace = transportSpaces[pTransport];
				// Can unit load onto this car ?
				if (currentSpace > 0 &&
					pPassengerType->Size > 0 &&
					pPassengerType->Size <= pTransportType->SizeLimit &&
					pPassengerType->Size <= currentSpace)
				{
					double d = pPassenger->DistanceFrom(pTransport);
					if (d < distance)
					{
						targetTransport = pTransport;
						distance = d;
					}
				}
			}
			// This is nearest available transport
			if (targetTransport)
			{
				// Get on the car
				if (pPassenger->GetCurrentMission() != Mission::Enter)
				{
					pPassenger->QueueMission(Mission::Enter, true);
					pPassenger->SetTarget(nullptr);
					pPassenger->SetDestination(targetTransport, false);
					transportSpaces[targetTransport] -= pPassengerType->Size;
					passengerLoading = true;
				}
			}
		}
	}
	// If no one is loading, this script is done
	if (!passengerLoading)
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
	}
	// Load logic
	// speed prioritize
	//for (auto pTransport : transports)
	//{
	//	DynamicVectorClass<FootClass*> loadedUnits;
	//	auto pTransportType = pTransport->GetTechnoType();
	//	double currentSpace = pTransportType->Passengers - pTransport->Passengers.GetTotalSize();
	//	if (currentSpace == 0) continue;
	//	for (auto pUnit : passengers)
	//	{
	//		auto pUnitType = pUnit->GetTechnoType();
	//		// Is legal loadable unit ?
	//		if (pUnitType->WhatAmI() != AbstractType::AircraftType &&
	//			!pUnit->InLimbo &&
	//			!pUnitType->ConsideredAircraft &&
	//			pUnit->Health > 0 &&
	//			pUnit != pTransport)
	//		{
	//			// Can unit load onto this car ?
	//			if (pUnitType->Size > 0
	//				&& pUnitType->Size <= pTransportType->SizeLimit
	//				&& pUnitType->Size <= currentSpace)
	//			{
	//				// Get on the car
	//				if (pUnit->GetCurrentMission() != Mission::Enter)
	//				{
	//					transportsNoSpace = false;
	//					pUnit->QueueMission(Mission::Enter, true);
	//					pUnit->SetTarget(nullptr);
	//					pUnit->SetDestination(pTransport, false);
	//					currentSpace -= pUnitType->Size;
	//					loadedUnits.AddItem(pUnit);
	//				}
	//			}
	//		}
	//		if (currentSpace == 0) break;
	//	}
	//	for (auto pRemove : loadedUnits) passengers.Remove(pRemove);
	//	loadedUnits.Clear();
	//}
	//if (transportsNoSpace)
	//{
	//	pTeam->StepCompleted = true;
	//	return;
	//}
}

bool ScriptExt::IsValidFriendlyTarget(TeamClass* pTeam, int group, TechnoClass* target, bool isSelfNaval, bool isSelfAircraft, bool isFriendly)
{
	if (!target) return false;
	if (TECHNO_IS_ALIVE(target) && target->Group == group)
	{
		auto pType = target->GetTechnoType();
		// Friendly?
		if (isFriendly ^ pTeam->Owner->IsAlliedWith(target->Owner))
			return false;
		// Only aircraft team can follow friendly aircraft
		if (isSelfAircraft)
			return true;
		else if (pType->ConsideredAircraft)
			return false;
		// If team is naval, only follow friendly naval
		if (isSelfNaval ^ pType->Naval)
			return false;
		// No underground
		if (target->InWhichLayer() == Layer::Underground)
			return false;

		return true;
	}
	return false;
}

void ScriptExt::FollowFriendlyByGroup(TeamClass* pTeam, int group)
{
	bool isSelfNaval = true, isSelfAircraft = true;
	double distMin = std::numeric_limits<double>::infinity();
	CellStruct* teamPosition = nullptr;
	TechnoClass* target = nullptr;
	// Use timer to reduce unnecessary cycle
	if (pTeam->GuardAreaTimer.TimeLeft <= 0)
	{
		// If all member is naval, will only follow friendly navals
		for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
		{
			if (!teamPosition)
				teamPosition = &(pMember->GetCell()->MapCoords);
			auto pMemberType = pMember->GetTechnoType();
			isSelfNaval = isSelfNaval && pMemberType->Naval;
			isSelfAircraft = isSelfAircraft && pMemberType->ConsideredAircraft;
		}
		// If previous target is valid, skip this check
		const auto dest = abstract_cast<TechnoClass*>(pTeam->Focus);
		if (IsValidFriendlyTarget(pTeam, group, dest, isSelfNaval, isSelfAircraft, true))
		{
			for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
			{
				double d = pMember->GetMapCoords().DistanceFrom(dest->GetCell()->MapCoords);
				if (d * 256 > RulesClass::Instance->CloseEnough)
				{
					if (isSelfAircraft)
					{
						pMember->SetDestination(dest, false);
						pMember->QueueMission(Mission::Move, true);
					}
					else
					{
						pMember->QueueMission(Mission::Area_Guard, true);
						pMember->SetTarget(nullptr);
						pMember->SetFocus(dest);
					}
				}
				else
				{
					if (!isSelfAircraft)
					{
						pMember->SetDestination(nullptr, false);
						pMember->CurrentMission = Mission::Area_Guard;
						pMember->Focus = nullptr;
					}
					else
						pMember->QueueMission(Mission::Area_Guard, true);
				}
			}
			pTeam->GuardAreaTimer.Start(30);
			return;
		}
		// Now looking for target
		for (int i = 0; i < TechnoClass::Array->Count; i++)
		{
			auto pTechno = TechnoClass::Array->GetItem(i);

			if (IsValidFriendlyTarget(pTeam, group, pTechno, isSelfNaval, isSelfAircraft, true))
			{
				// candidate
				auto coord = pTechno->GetCell();
				if (!teamPosition) {
					auto pScript = pTeam->CurrentScript;
					pTeam->StepCompleted = true;
					Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d) Attempting To derefence nullptr teamPosition ! )\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument);
					return;
				}

				double distance = coord->MapCoords.DistanceFromSquared(*teamPosition);
				if (distance < distMin)
				{
					target = pTechno;
					distMin = distance;
				}
			}
		}
		if (target)
		{
			if (isSelfAircraft)
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					pMember->SetDestination(target, false);
					pMember->QueueMission(Mission::Move, true);
				}
			}
			else
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					pMember->QueueMission(Mission::Area_Guard, true);
					pMember->SetTarget(nullptr);
					pMember->SetFocus(target);
				}
			}
			pTeam->Focus = target;
			pTeam->GuardAreaTimer.Start(30);
		}
		// If there's no valid target, continue script
		else
			pTeam->StepCompleted = true;
	}
	else
		pTeam->GuardAreaTimer.TimeLeft--;
}

void ScriptExt::RallyUnitInMap(TeamClass* pTeam, int nArg)
{
	auto pType = pTeam->Type;
	for (int i = 0; i < FootClass::Array->Count; i++)
	{
		auto pFoot = FootClass::Array->GetItem(i);

		// Must be owner and with same group
		if (pFoot
			&& TECHNO_IS_ALIVE(pFoot)
			&& pFoot->Owner == pTeam->Owner
			&& pFoot->Group == pType->Group
			&& IsValidRallyTarget(pTeam, pFoot, nArg))
		{
			// This will bypass any recruiting restrictions, except group (this action's purpose)
			if (pType->Recruiter)
				pTeam->AddMember(pFoot, true);
			// Only rally unit's parent team set Recruitable = yes and Priority less than this team
			else if (pFoot->RecruitableB)
			{
				if (pFoot->Team)
				{
					if (pFoot->Team->Type->Priority < pType->Priority)
						pTeam->AddMember(pFoot, true);
				}
				else
					pTeam->AddMember(pFoot, true);
			}
		}
	}
	pTeam->StepCompleted = true;
}

bool ScriptExt::IsValidRallyTarget(TeamClass* pTeam, FootClass* pFoot, int nType)
{
	const auto type = pFoot->WhatAmI();
	auto pTechnoType = pFoot->GetTechnoType();
	TaskForceClass* pTaskforce = pTeam->Type->TaskForce;
	if (!pTechnoType)
		return false;

	switch (nType)
	{
	case 0: // Anything
		return true;
	case 1: // Infantry
		return type == AbstractType::Infantry;
	case 2: // Vehicles
		return type == AbstractType::Unit;
	case 3: // Air Units
		return pTechnoType->ConsideredAircraft || type == AbstractType::Aircraft;
	case 4: // Naval units
		return type == AbstractType::Unit && pTechnoType->Naval;
	case 5: // Ground units
		return (type == AbstractType::Unit || type == AbstractType::Infantry)
			&& (!pTechnoType->ConsideredAircraft && !pTechnoType->Naval);
	case 6: // Dockable fighters
		return type == AbstractType::Aircraft && static_cast<AircraftClass*>(pFoot)->Type->AirportBound;
	case 7: // Same type in taskforce
		if (pTaskforce)
		{
			for (auto const& pEntry : pTaskforce->Entries)
			{
				if (pEntry.Type && pEntry.Type == pTechnoType)
					return true;
			}
		}
		return false;
	default:
		return false;
	}
}
#pragma endregion

/*
	if (auto v28 = pUnit->Passengers.FirstPassenger)
		{
			do
			{
				pTeam->AddMember(v28, 0);
				v28 = static_cast<FootClass*>(v28->NextObject);
			}
			while (v28 && (v28->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None);
		}
*/

void ScriptExt::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine = false, int count = 0)
{
	bool bSucceeded = false;
	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (count <= 0)
					//PR #724
			count = 15 * pTeam->CurrentScript->GetCurrentAction().Argument;

		if (count > 0)
		{
			pTeamData->ForceJump_InitialCountdown = count;
			pTeamData->ForceJump_Countdown.Start(count);
			pTeamData->ForceJump_RepeatMode = repeatLine;
		}
		else
		{
			pTeamData->ForceJump_InitialCountdown = -1;
			pTeamData->ForceJump_Countdown.Stop();
			pTeamData->ForceJump_Countdown = -1;
			pTeamData->ForceJump_RepeatMode = false;
		}

		bSucceeded = true;

	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = bSucceeded;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d) %s Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, bSucceeded ? "Done" : "Failed",count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	bool bSucceeded = false;
	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		pTeamData->ForceJump_InitialCountdown = -1;
		pTeamData->ForceJump_Countdown.Stop();
		pTeamData->ForceJump_Countdown = -1;
		pTeamData->ForceJump_RepeatMode = false;
		bSucceeded = true;
	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = bSucceeded;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): %s Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument , bSucceeded ? "Done" : "Failed");
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit->WhatAmI() == AbstractType::Unit)
				if (auto pCell = pUnit->GetCell())
					if (auto pBld = pCell->GetBuilding())
						if (!pBld->Type->Naval && pBld->Type->WeaponsFactory && !pUnit->IsInAir())
							continue;

			pUnit->QueueMission(Mission::Area_Guard, true);
		}

		pTeam->GuardAreaTimer.Start(15 * pTeam->CurrentScript->GetCurrentAction().Argument);
	}

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	std::vector<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
		auto const pUnitType = pUnit->GetTechnoType();
		if (pUnitType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers) {
			transports.push_back(pUnit);
		}
	}

	// Now load units into transports
	for (auto const& pTransport : transports)
	{
		if (!TechnoExt::IsActive(pTransport, true ,true))
			continue;

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!TechnoExt::IsActive(pUnit, true, true))
				continue;

			auto const pTransportType = pTransport->GetTechnoType();
			auto const pUnitType = pUnit->GetTechnoType();
			if (pTransport != pUnit
				&& pUnitType->WhatAmI() != AbstractType::AircraftType
				&& !pUnitType->ConsideredAircraft)
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					pUnit->IsTeamLeader = true;
					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pTransportType->OpenTopped)
						{
							pUnit->EnteredOpenTopped(pTransport);
							pUnit->Transporter = pTransport;
						}

						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

						return;
					}
				}
			}
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;

	// This action finished
	if (pTeam->CurrentScript->HasNextMission())
		++pTeam->CurrentScript->CurrentMission;

	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!TechnoExt::IsActive(pUnit, true, true))
			continue;

		if (!pUnit->Spawned && pUnit->Owner)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (pUnit->WhatAmI() == AbstractType::Aircraft)
				{
					auto pAircraft = static_cast<AircraftClass*>(pUnit);

					if (pAircraft->Type->AirportBound) {
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pAircraft->SetTarget(nullptr);
						pAircraft->LastTarget = nullptr;
						auto const nContactIter = make_iterator(pAircraft->RadioLinks);

						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pAircraft->IsInAir() && pAircraft->CurrentMission != Mission::Enter)
						if(auto pCell = pAircraft->GetCell() )
							if(auto const pBld = pCell->GetBuilding())
								if(pBld->Type->Helipad && !nContactIter.empty())
									if(nContactIter.contains(pBld))
										pAircraft->QueueMission(Mission::Enter, true);

						return;
					}
				}
				else if (pUnit->GetTechnoType()->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass *pTeam, int countdown = -1)
{
	FootClass *pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->GetCurrentAction().Argument;
	bool gatherUnits = false;
	auto pExt = TeamExt::ExtMap.Find(pTeam);

	// This team has no units! END
	if(!pExt || (IsEmpty(pTeam) && !pExt->TeamLeader)) {
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Load countdown
	if (pExt->Countdown_RegroupAtLeader >= 0)
		countdown = pExt->Countdown_RegroupAtLeader;

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
		gatherUnits = true;

	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else if (countdown == 0) // Countdown ended
			countdown = -1;
		else // Start countdown.
		{
			countdown = initialCountdown * 15;
			gatherUnits = true;
		}

		// Save counter
		pExt->Countdown_RegroupAtLeader = countdown;
	}

	if (!gatherUnits)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
		int nTogether = 0;
		int nUnits = -1; // Leader counts here
		double closeEnough;

		// Find the Leader
		pLeaderUnit = pExt->TeamLeader;
		if (!TechnoExt::IsActive(pLeaderUnit,false,true))
		{
			pLeaderUnit = FindTheTeamLeader(pTeam);
			pExt->TeamLeader = pLeaderUnit;
		}

		if (!pLeaderUnit)
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pExt->CloseEnough > 0)
		{
			closeEnough = pExt->CloseEnough;
			pExt->CloseEnough = -1; // This a one-time-use value
		}
		else
		{
			closeEnough = RulesClass::Instance->CloseEnough / 256.0;
		}

		// The leader should stay calm & be the group's center
		bool AllowToStop = true;
		if(pLeaderUnit->WhatAmI() == AbstractType::Unit)
			if (auto pCell = pLeaderUnit->GetCell())
				if (auto pBld = pCell->GetBuilding())
					if (!pBld->Type->Naval && pBld->Type->WeaponsFactory && !pLeaderUnit->IsInAir())
						AllowToStop = false;

		if (pLeaderUnit->Locomotor->Is_Moving_Now() && AllowToStop)
			pLeaderUnit->SetDestination(nullptr, false);

		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!TechnoExt::IsActive(pUnit, true, true))
				continue;

			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (!pTypeUnit)
					continue;

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				// Aircraft case
				if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = static_cast<AircraftClass*>(pUnit);

					if (pAircraft->Type->AirportBound) {
						// This aircraft won't count for the script action
						pAircraft->QueueMission(Mission::Return, false);
						pAircraft->Mission_Enter();

						continue;
					}
				}

				nUnits++;

				if(pUnit->WhatAmI() == AbstractType::Unit)
					if (auto pCell = pUnit->GetCell())
						if (auto pBld = pCell->GetBuilding())
							if (!pBld->Type->Naval && pBld->Type->WeaponsFactory && !pUnit->IsInAir())
								continue;

				if (pUnit->DistanceFrom(pLeaderUnit) / 256.0 > closeEnough)
				{
					// Leader's location is too far from me. Regroup
					if (pUnit->Destination != pLeaderUnit)
					{
						pUnit->SetDestination(pLeaderUnit, false);
						pUnit->QueueMission(Mission::Move, false);
					}
				}
				else
				{

					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}


		if (nUnits >= 0
			&& nUnits == nTogether
			&& (initialCountdown == 0
				|| (initialCountdown > 0
					&& countdown <= 0)))
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}
	}
}

std::pair<WeaponTypeClass*, WeaponTypeClass*> ScriptExt::GetWeapon(TechnoClass* pTechno)
{
	auto pTechnoType = pTechno->GetTechnoType();
	if (!pTechno || !pTechnoType)
		return { nullptr , nullptr };

	WeaponTypeClass* WeaponType1 = pTechno->Veterancy.IsElite() ?
		pTechnoType->EliteWeapon[0].WeaponType :
		pTechnoType->Weapon[0].WeaponType;

	WeaponTypeClass* WeaponType2 = pTechno->Veterancy.IsElite() ?
		pTechnoType->EliteWeapon[1].WeaponType :
		pTechnoType->Weapon[1].WeaponType;

	WeaponTypeClass* WeaponType3 = WeaponType1;

	if (pTechnoType->IsGattling)
	{
		WeaponType3 = pTechno->Veterancy.IsElite() ?
			pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
			pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

		WeaponType1 = WeaponType3;
	}

	return { WeaponType1,WeaponType2 };
}

void ScriptExt::Mission_Attack(TeamClass *pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->GetCurrentAction().Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass *pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	auto pHouseExt = HouseExt::ExtMap.Find(pTeam->Owner);
	if (!pHouseExt)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (IsEmpty(pTeam) && !pTeamData->TeamLeader)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);
	if (!TechnoExt::IsActive(pFocus ,false, false))
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pUnit);
		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
				pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pFocus = nullptr;
			pTeam->Focus = nullptr;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pTeamUnit = pTeam->FirstUnit; pTeamUnit; pTeamUnit = pTeamUnit->NextTeamMember)
				{
					// Let's reset all Team Members objective
					if(auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pTeamUnit))
						pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pTeamUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
					{
						pTeamUnit->SetTarget(nullptr);
						pTeamUnit->LastTarget = nullptr;
						pTeamUnit->SetFocus(nullptr); // Lets see if this works or bug my little aircrafts
						pTeamUnit->CurrentTargets.Clear(); // Lets see if this works or bug my little aircrafts
						pTeamUnit->QueueMission(Mission::Guard, true);
					}
				}

				pTeamData->IdxSelectedObjectFromAIList = -1;

				// This action finished
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

				return;
			}
		}
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!TechnoExt::IsActive(pUnit, true, true))
			continue;

		{
			if (auto pUnitType = pUnit->GetTechnoType())
			{
				if (pUnit->WhatAmI() == AbstractType::Aircraft)
				{
					auto pAir = static_cast<AircraftClass*>(pUnit);
					if(!pAir->IsInAir() && pAir->Type->AirportBound && pAir->Ammo < pAir->Type->Ammo)
					{
						bAircraftsWithoutAmmo = true;
						pUnit->CurrentTargets.Clear();
					}
				}

				auto [pUnitCW1, pUnitCW2] = ScriptExt::GetWeapon(pUnit);
				pacifistTeam = (!pUnitCW1 && !pUnitCW2);

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if (pUnitType->WhatAmI() == AbstractType::InfantryType)
				{
					auto pTypeInf = static_cast<InfantryTypeClass*>(pUnitType);
					if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					{
						agentMode = true;
					}
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!TechnoExt::IsActive(pLeaderUnit,false,true))
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	bool leaderWeaponsHaveAA = false;
	bool leaderWeaponsHaveAG = false;
	auto [WeaponType1, WeaponType2] = ScriptExt::GetWeapon(pLeaderUnit);

	// Weapon check used for filtering Leader targets.
	// Note: the Team Leader is picked for this task, be careful with leadership rating values in your mod
	if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
		leaderWeaponsHaveAA = true;

	if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG) || agentMode)
		leaderWeaponsHaveAG = true;

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0) {
		for (NextObject j(pLeaderUnit->Passengers.FirstPassenger->NextObject); abstract_cast<FootClass*>(*j); ++j) {
			auto [passengerWeaponType1, passengerWeaponType2] = ScriptExt::GetWeapon(static_cast<FootClass*>(*j));

			// Used for filtering targets.
			// Note: the units inside a openTopped Leader are used for this task
			if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AA)
				|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AA))
			{
				leaderWeaponsHaveAA = true;
			}

			if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AG)
				|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AG))
			{
				leaderWeaponsHaveAG = true;
			}
		}
	}

	bool onlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;

	if (pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
		onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		if (onlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = scriptArgument;
		selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (!TechnoExt::IsActive(pUnit, true, true))
					continue;
				{
					auto pUnitType = pUnit->GetTechnoType();
					if (pUnitType && pUnit != selectedTarget && pUnit->Target != selectedTarget)
					{
						pUnit->CurrentTargets.Clear();
						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_not_okay
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType
							&& pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
						{
							pUnit->SetDestination(selectedTarget, false);
							pUnit->QueueMission(Mission::Attack, true);
						}

						pUnit->SetTarget(selectedTarget);

						if (pUnit->IsEngineer())
						{
							pUnit->QueueMission(Mission::Capture, true);
						}
						else
						{
							// Aircraft hack. I hate how this game auto-manages the aircraft missions.
							if (pUnitType->WhatAmI() != AbstractType::AircraftType)
							{
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);

								if (pUnit->GetCurrentMission() != Mission::Attack)
									pUnit->Mission_Attack();

								if (pUnit->GetCurrentMission() == Mission::Move && pUnitType->JumpJet)
									pUnit->Mission_Attack();
							}
						}

						// Spy case
						if (pUnitType->WhatAmI() == AbstractType::InfantryType)
						{
							auto pInfantryType = static_cast<InfantryTypeClass*>(pUnitType);

							if (pInfantryType && pInfantryType->Infiltrate && pInfantryType->Agent)
							{
								// Check if target is an structure and see if spiable
								if (pUnit->GetCurrentMission() != Mission::Enter)
									pUnit->Mission_Enter();
							}
						}

						// Tanya / Commando C4 case
						if ((pUnitType->WhatAmI() == AbstractType::InfantryType
							&& (static_cast<InfantryTypeClass*>(pUnitType)->C4 || pUnit->HasAbility(AbilityType::C4)))
							&& pUnit->GetCurrentMission() != Mission::Sabotage)
						{
							pUnit->Mission_Attack();
							pUnit->QueueMission(Mission::Sabotage, true);
						}
					}
					else
					{
						pUnit->QueueMission(Mission::Attack, true);
						pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);
						pUnit->Mission_Attack();
					}
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.
			if (!noWaitLoop)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0)
			{
				// No target? let's wait some frames
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);

				return;
			}

			// This action finished

			if (pTeamData->FailedCounter <= 0 || pTeamData->FailedCounter < 4) {
				pTeamData->FailedCounter++;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID);
			} else {

				if(pTeamData->FailedCounter == 4) {
					pTeam->RemoveMember(pTeamData->TeamLeader);
					pTeamData->TeamLeader = FindTheTeamLeader(pTeam);
					pTeamData->FailedCounter = -1;
				}
			}

			pTeam->StepCompleted = true;

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit

		if (TechnoExt::IsActive(pFocus ,false ,false)
			&& !pFocus->GetTechnoType()->Immune
			&& ((pFocus->IsInAir() && leaderWeaponsHaveAA)
				|| (!pFocus->IsInAir() && leaderWeaponsHaveAG))

			&& pFocus->Owner
			&& pFocus->Owner != pLeaderUnit->Owner
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus)
				|| (pLeaderUnit->Owner->IsAlliedWith(pFocus)
					&& pFocus->IsMindControlled()
					&& !pLeaderUnit->Owner->IsAlliedWith(pFocus->MindControlledBy))))
		{

			bool bForceNextAction = false;

			for (auto pUnit = pTeam->FirstUnit; TechnoExt::IsActive(pUnit, true, true) && !bForceNextAction; pUnit = pUnit->NextTeamMember)
			{
				if (auto pUnitType = pUnit->GetTechnoType())
				{
					// Aircraft case 1
					if ((pUnitType->WhatAmI() == AbstractType::AircraftType
						&& static_cast<AircraftTypeClass*>(pUnitType)->AirportBound)
						&& pUnit->Ammo > 0
						&& (pUnit->Target != pFocus && !pUnit->InAir))
					{
						pUnit->SetTarget(pFocus);
						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have nti-ground weapons. Ignore the attack
					if (pUnitType->Underwater
						&& pUnitType->LandTargeting == LandTargetingType::Land_not_okay
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						pUnit->CurrentTargets.Clear();
						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);
						bForceNextAction = true;

						continue;
					}

					// Aircraft case 2
					if (pUnitType->WhatAmI() == AbstractType::AircraftType
						&& pUnit->GetCurrentMission() != Mission::Attack
						&& pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pUnit->InAir)
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}
						else
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}

						continue;
					}

					// Tanya / Commando C4 case
					if ((pUnitType->WhatAmI() == AbstractType::InfantryType
						&& static_cast<InfantryTypeClass*>(pUnitType)->C4
						|| pUnit->HasAbility(AbilityType::C4)) && pUnit->GetCurrentMission() != Mission::Sabotage)
					{
						pUnit->Mission_Attack();
						pUnit->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (pUnitType->WhatAmI() != AbstractType::AircraftType)
					{
						if (pUnit->Target != pFocus)
							pUnit->SetTarget(pFocus);

						if (pUnit->GetCurrentMission() != Mission::Attack
							&& pUnit->GetCurrentMission() != Mission::Unload
							&& pUnit->GetCurrentMission() != Mission::Selling)
						{
							pUnit->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass *pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	if (!pTechno || !pTechno->Owner || !pTechno->GetTechnoType())
		return nullptr;

	TechnoClass *bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;
	auto pTechnoType = pTechno->GetTechnoType();

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();

		if (!TechnoExt::IsActive(object,false,false) || !objectType || !object->Owner)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		int weaponIndex = pTechno->SelectWeapon(object);
		auto weaponType = pTechno->GetWeapon(weaponIndex)->WeaponType;

		if (weaponType && weaponType->Projectile->AA)
			unitWeaponsHaveAA = true;

		if ((weaponType && weaponType->Projectile->AG) || agentMode)
			unitWeaponsHaveAG = true;

		int weaponDamage = 0;

		if (weaponType)
		{
			auto nArmor = objectType->Armor;
			if (auto pObjectExt = TechnoExt::ExtMap.Find(object))
				if (pObjectExt->GetShield() && pObjectExt->GetShield()->IsActive() && pObjectExt->CurrentShieldType)
					nArmor = pObjectExt->GetShield()->GetType()->Armor;

			if (weaponType->AmbientDamage > 0)
				weaponDamage = MapClass::GetTotalDamage(weaponType->AmbientDamage, weaponType->Warhead, nArmor, 0) + MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, nArmor, 0);
			else
				weaponDamage = MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, nArmor, 0);
		}

		// If the target can't be damaged then isn't a valid target
		if (weaponType && weaponDamage <= 0 && !agentMode)
			continue;

		if (!agentMode)
		{
			if (object->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!object->IsInAir() && !unitWeaponsHaveAG)
				continue;

			if (object->GetType()->Immune)
				continue;
		}

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never || pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;

		if (object != pTechno
			&& TechnoExt::IsActive(object, false)
&& (object->Owner != pTechno->Owner || (object->Owner == pTechno->Owner && attackAITargetType == -1 && (method == 39 || method == 40)))
&& (!pTechno->Owner->IsAlliedWith(object) || (object->Owner == pTechno->Owner && attackAITargetType == -1 && (method == 39 || method == 40)) || (pTechno->Owner->IsAlliedWith(object)
					&& object->IsMindControlled()
					&& !pTechno->Owner->IsAlliedWith(object->MindControlledBy))))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{

				bool isGoodTarget = false;
				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass *pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass *pTeamLeader = nullptr)
{
	if (!pTechno || !pTechno->Owner || (pTeamLeader && !pTeamLeader->Owner))
		return false;

	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	if (!pTargetTypeExt || pTargetTypeExt->IsDummy.Get())
		return false;

	if (auto pBuilding = specific_cast<BuildingClass*>(pTechno)) {
		if (BuildingExt::ExtMap.Find(pBuilding)->IsInLimboDelivery)
			return false;
	}

	if (pTeamLeader) {
		auto const nFireError = pTeamLeader->GetFireError(pTechno, pTeamLeader->SelectWeapon(pTechno), false);
		if(nFireError == FireError::NONE || nFireError == FireError::ILLEGAL || nFireError == FireError::CANT)
		return false;
	}

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypesLists.Count > 0) {
		auto const& nVec = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);
		return std::find(nVec.begin(), nVec.end(), pTechnoType) != nVec.end();
	}

	// mask shoud be replaced with proper enum class
	// it is more readable
	switch (mask)
	{
	case 1:
		// Anything ;-)
			return !pTechno->Owner->IsNeutral();
	case 2:
		// Building
	{
		if (!pTechno->Owner->IsNeutral()) {
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				auto pBldExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

				return !(pBld->Type->Artillary
						|| pBld->Type->TickTank
						|| pBld->Type->ICBMLauncher
						|| pBld->Type->SensorArray
						|| (pBldExt && pBldExt->IsJuggernaut)
						) ;
			}
		}
		return false;
	}
	case 3:
	{
		// Harvester
		if (!pTechno->Owner->IsNeutral())
		{
			auto const pWhat = pTechno->WhatAmI();
			if (pWhat == AbstractType::Unit) {
				auto pType = static_cast<UnitClass*>(pTechno)->Type;
				return pType->Harvester || pType->Weeder;
			}

			if (pWhat == AbstractType::Building) {
				auto pBldHere = static_cast<BuildingClass*>(pTechno);
				return pBldHere->SlaveManager && pTechnoType->ResourceGatherer && pBldHere->Type->Enslaves;
			}

			if (pWhat == AbstractType::Infantry) {
				auto pInfHere = static_cast<InfantryClass*>(pTechno);
				return pInfHere->Type->Slaved && pInfHere->SlaveOwner && pTechnoType->ResourceGatherer;
			}
		}
		return false;
	}
	case 4:
		// Infantry
		return !pTechno->Owner->IsNeutral() && pTechno->WhatAmI() == AbstractType::Infantry;
	case 5:
	{
		// Vehicle, Aircraft, Deployed vehicle into structure
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				if(pBld->Type->UndeploysInto){
					auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
					return (pBld->Type->Artillary
						|| pBld->Type->TickTank
						|| pBld->Type->ICBMLauncher
						|| pBld->Type->SensorArray
						|| (pExt && pExt->IsJuggernaut));
				}
			}

			return (pTechno->WhatAmI() == AbstractType::Aircraft || pTechno->WhatAmI() == AbstractType::Unit) &&
				pTechnoType->MovementZone != MovementZone::Amphibious && pTechnoType->MovementZone != MovementZone::AmphibiousDestroyer;
		}
		return false;
	}
	case 6:
		// Factory
	{
		if (!pTechno->Owner->IsNeutral()) {
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
				return pBld->Factory != nullptr;
		}
		return false;
	}
	case 7:
	{
		// Defense
		if (!pTechno->Owner->IsNeutral()){
			if(auto pBld = specific_cast<BuildingClass*>(pTechno)) {
				return pBld->Type->IsBaseDefense;
			}
		}

		return false;
	}
	case 8:
	{
		if (pTeamLeader) {

			if(auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target)){
				// The possible Target is aiming against me? Revenge!
				if (pTarget != pTeamLeader )
					return pTarget->Target == pTeamLeader
					|| pTarget->Owner && HouseClass::Array->GetItem(pTarget->Owner->EnemyHouseIndex) == pTeamLeader->Owner;
			}

			auto const curtargetiter = make_iterator(pTechno->CurrentTargets);
			if (!curtargetiter.empty()) {
				auto const itif = std::find_if(curtargetiter.begin(), curtargetiter.end(),
				[pTeamLeader](AbstractClass* pTarget) {
					auto const pTech = abstract_cast<TechnoClass*>(pTarget);
					 return pTech && pTech->GetOwningHouse() && pTech->GetOwningHouse() == pTeamLeader->Owner;

				});

				return (*itif) && itif != pTechno->CurrentTargets.end();
			}

			if (!pTechno->Owner->IsNeutral()){
				// Then check if this possible target is too near of the Team Leader
				auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;
				auto nRange1 = pTechno->GetWeaponRange(pTechnoType->IsGattling ? pTechno->CurrentWeaponNumber:0);
				auto nRange2 = !pTechnoType->IsGattling ?  pTechno->GetWeaponRange(1):0;


				return (nRange1 > 0 && distanceToTarget <= (nRange1 / 256.0 * 4.0))
					|| (nRange2 > 0 && distanceToTarget <= (nRange2 / 256.0 * 4.0))
					|| (pTeamLeader->GetTechnoType()->GuardRange > 0
						&& distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0));
			}
		}

		return false;
	}
	case 9:
	{
		// Power Plant
		if (!pTechno->Owner->IsNeutral())
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
				return pBld->Type->PowerBonus > 0
					|| (pBld->Upgrades[0] && pBld->Upgrades[0]->PowerBonus > 0)
					|| (pBld->Upgrades[1] && pBld->Upgrades[1]->PowerBonus > 0)
					|| (pBld->Upgrades[2] && pBld->Upgrades[2]->PowerBonus > 0)
				;

		return false;
	}
	case 10:
	{
		// Occupied Building
		if (auto pBld = specific_cast<BuildingClass*>(pTechno)) {
			return (pBld->Occupants.Count > 0);
		}

		return false;
	}
	case 11:
	{
		// Civilian Tech
		if (auto pBld = specific_cast<BuildingClass*>(pTechno)){

			if (pTechnoType->WhatAmI() == AbstractType::BuildingType) {
				auto const neutralIter = make_iterator(RulesClass::Instance->NeutralTechBuildings);

				return (!neutralIter.empty() && neutralIter.contains(pBld->Type));
			}

			// Other cases of civilian Tech Structures
			return (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& !pBld->Type->InvisibleInGame
				&& !pBld->Type->Immune
				&& pBld->Type->Unsellable
				&& pBld->Type->Capturable
				&& pBld->Type->TechLevel < 0
				&& pBld->Type->NeedsEngineer
				&& !pBld->Type->BridgeRepairHut);
		}
		return false;
	}
	case 12:
	{
		// Refinery
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnit = specific_cast<UnitClass*>(pTechno)) {
				return !(pUnit->Type->Harvester || pUnit->Type->Weeder)
					&& pUnit->Type->ResourceGatherer
					&& pUnit->Type->DeploysInto
					;
			}

			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno)) {
				return pBuilding->Type->ResourceGatherer
					|| (pBuilding->Type->Refinery || (pBuilding->SlaveManager && pBuilding->Type->Enslaves));
			}

		}

		return false;
	}
	case 13:
	{
		if (!pTechno->Owner->IsNeutral()){
			auto [WeaponType1, WeaponType2] = ScriptExt::GetWeapon(pTechno);

			if (WeaponType1) {
				auto pWHExt = WarheadTypeExt::ExtMap.Find(WeaponType1->Warhead);
				return pWHExt && pWHExt->PermaMC.Get() || WeaponType1->Warhead->MindControl;
			}

			if (WeaponType2) {
				auto pWHExt = WarheadTypeExt::ExtMap.Find(WeaponType2->Warhead);
				return pWHExt && pWHExt->PermaMC.Get() || WeaponType2->Warhead->MindControl;
			}
		}
		return false;
	}
	case 14:
	{
		// Aircraft and Air Unit
		return (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::AircraftType
				|| pTechnoType->JumpJet
				|| pTechnoType->BalloonHover
				|| pTechno->IsInAir()));
	}
	case 15:
	{
		// Naval Unit & Structure
		return (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval
				|| (pTechno->GetCell()->LandType == LandType::Water)));
	}
	case 16:
	{
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		if (!pTechno->Owner->IsNeutral())
		{
			auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
			auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
			return ((pTechnoTypeExt
				&& (pTechnoTypeExt->RadarJamRadius > 0
					|| pTechnoTypeExt->InhibitorRange.isset()))
					|| (pTypeBuilding && (pTypeBuilding->GapGenerator
					|| pTypeBuilding->CloakGenerator)));
		}
		return false;
	}
	case 17:
	{
		if (!pTechno->Owner->IsNeutral() && !pTechnoType->Naval && !pTechno->IsInAir()) {
			if (auto pBld = specific_cast<BuildingClass*>(pTechno)) {
				if (pBld->Type->UndeploysInto) {
					return (pBld->Type->UndeploysInto
						&& !pBld->Type->BaseNormal);
				}
			}

			return pTechno->WhatAmI() == AbstractType::Unit && !pTechno->IsInAir() && !pTechnoType->Naval;
		}

		return false;
	}
	case 18:
	{
		// Economy: Harvester, Refinery or Resource helper
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnitT = specific_cast<UnitTypeClass*>(pTechnoType))
				return pUnitT->Harvester || pUnitT->ResourceGatherer;

			if (auto pInfT = specific_cast<InfantryTypeClass*>(pTechnoType))
				return pInfT->ResourceGatherer && ( pInfT->Slaved && pTechno->SlaveOwner);

			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				return (pBld->Upgrades[0] && pBld->Upgrades[0]->ProduceCashAmount > 0)
					|| (pBld->Upgrades[1] && pBld->Upgrades[1]->ProduceCashAmount > 0)
					|| (pBld->Upgrades[2] && pBld->Upgrades[2]->ProduceCashAmount > 0)
					|| pBld->Type->Refinery
					|| pBld->Type->ProduceCashAmount
					|| pBld->Type->OrePurifier
					|| pBld->Type->ResourceGatherer
					|| (pTechno->SlaveManager  && pBld->Type->Enslaves)
					;
			}
		}

		return false;
	}
	case 19:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);
		// Infantry Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::InfantryType);
	}
	case 20:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::UnitType
			&& !pBuildingType->Naval);
	}
	case 21:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// is Aircraft Factory
		return (!pTechno->Owner->IsNeutral()
			&& (pBuildingType
				&& (pBuildingType->Factory == AbstractType::AircraftType
					|| pBuildingType->Helipad)));
	}
	case 22:
	{
		auto pBld = specific_cast<BuildingClass*>(pTechno);
		// Radar & SpySat
		return (!pTechno->Owner->IsNeutral()
			&& (pBld && pBld->Type
				&& (pBld->Type->Radar
					|| pBld->Type->SpySat
					|| (pBld->Upgrades[0] && (pBld->Upgrades[0]->SpySat || pBld->Upgrades[0]->Radar))
					|| (pBld->Upgrades[1] && (pBld->Upgrades[1]->SpySat || pBld->Upgrades[1]->Radar))
					|| (pBld->Upgrades[2] && (pBld->Upgrades[2]->SpySat || pBld->Upgrades[2]->Radar)))));
	}
	case 23:
	{
		// Buildable Tech
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			auto const BuildTechIter = make_iterator(RulesClass::Instance->BuildTech);
			return (!BuildTechIter.empty() && BuildTechIter.contains(static_cast<BuildingTypeClass*>(pTechnoType)));
		}

		return false;
	}
	case 24:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::UnitType
			&& pBuildingType->Naval);
	}
	case 25:
	{
		// Super Weapon building
		bool IsOK = false;
		if (!pTechno->Owner->IsNeutral()){
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBld->Type);


				int nSuperWeapons = !pBuildingExt ? 0 : pBuildingExt->SuperWeapons.Count;

				IsOK = (pBld->Type->SuperWeapon >= 0
					|| pBld->Type->SuperWeapon2 >= 0
					|| nSuperWeapons > 0);

				if (!IsOK && pBld->Upgrades[0])
				{
					pBuildingExt = BuildingTypeExt::ExtMap.Find(pBld->Upgrades[0]);
					nSuperWeapons = !pBuildingExt ? 0 : pBuildingExt->SuperWeapons.Count;

					IsOK = (pBld->Upgrades[0]->SuperWeapon >= 0
						|| pBld->Upgrades[0]->SuperWeapon2 >= 0
						|| nSuperWeapons > 0);
				}

				if (!IsOK && pBld->Upgrades[1])
				{
					pBuildingExt = BuildingTypeExt::ExtMap.Find(pBld->Upgrades[1]);
					nSuperWeapons = !pBuildingExt ? 0 : pBuildingExt->SuperWeapons.Count;

					IsOK = (pBld->Upgrades[1]->SuperWeapon >= 0
						|| pBld->Upgrades[1]->SuperWeapon2 >= 0
						|| nSuperWeapons > 0);
				}

				if (!IsOK && pBld->Upgrades[2])
				{
					pBuildingExt = BuildingTypeExt::ExtMap.Find(pBld->Upgrades[2]);
					nSuperWeapons = !pBuildingExt ? 0 : pBuildingExt->SuperWeapons.Count;

					IsOK = (pBld->Upgrades[2]->SuperWeapon >= 0
						|| pBld->Upgrades[2]->SuperWeapon2 >= 0
						|| nSuperWeapons > 0);
				}
			}
		}

		return IsOK;
	}
	case 26:
	{

		// Construction Yard
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType)) {
				return (pTypeBuilding && pTypeBuilding->Factory == AbstractType::BuildingType && pTypeBuilding->ConstructionYard);
			}

			if (pTechnoType->WhatAmI() == AbstractType::UnitType) {
				auto const BaseUnitIter = make_iterator(RulesClass::Instance->BaseUnit);
				return (!BaseUnitIter.empty() && BaseUnitIter.contains(static_cast<UnitTypeClass*>(pTechnoType)));
			}
		}

		return false;
	}
	case 27:
		// Any Neutral object
			return pTechno->Owner->IsNeutral();
	case 28:
	{
		// Cloak Generator & Gap Generator
		bool IsOk = false;
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
			{
				IsOk = pBuilding->Type->GapGenerator
					|| pBuilding->Type->CloakGenerator;

				if (!IsOk && pBuilding->Upgrades[0])
				{
					IsOk = pBuilding->Upgrades[0]->GapGenerator
						|| pBuilding->Upgrades[0]->CloakGenerator;
				}

				if (!IsOk && pBuilding->Upgrades[1])
				{
					IsOk = pBuilding->Upgrades[1]->GapGenerator
						|| pBuilding->Upgrades[1]->CloakGenerator;
				}

				if (!IsOk && pBuilding->Upgrades[2])
				{
					IsOk = pBuilding->Upgrades[2]->GapGenerator
						|| pBuilding->Upgrades[2]->CloakGenerator;
				}
			}
		}

		return IsOk;
	}
	case 29:
	{
		// Radar Jammer
		auto pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral() &&
			(pTypeTechnoExt && (pTypeTechnoExt->RadarJamRadius > 0)));
	}
	case 30:
	{
		// Inhibitor
		auto pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& pTypeTechnoExt->InhibitorRange.isset()));
	}
	case 31:
	{
		// Naval Unit
		return (!pTechno->Owner->IsNeutral()
			&& pTechno->WhatAmI() == AbstractType::Unit
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water));
	}
	case 32:
	{
		// Any non-building unit
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnit = specific_cast<UnitClass*>(pTechno)) {
				return !pUnit->Type->DeploysInto;
			}

			if (auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType))
			{
				auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pTypeBuilding);
				return pTypeBuilding->UndeploysInto &&
					(pTypeBuilding->Artillary
					|| pTypeBuilding->TickTank
					|| (pBuildingExt && pBuildingExt->IsJuggernaut)
					|| pTypeBuilding->ICBMLauncher
					|| pTypeBuilding->SensorArray
					|| pTypeBuilding->ResourceGatherer);
			}
		}

		return false;
	}
	case 33:
	{
		auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Capturable Structure or Repair Hut
		return (pTypeBuilding
			&& (pTypeBuilding->Capturable
				|| (pTypeBuilding->BridgeRepairHut
					&& pTypeBuilding->Repairable)));
	}
	case 34:
	{
		if (!pTechno->Owner->IsNeutral() && pTeamLeader)
		{
			// Inside the Area Guard of the Team Leader
			auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons

			return (pTeamLeader->GetTechnoType()->GuardRange > 0
					&& distanceToTarget <= ((pTeamLeader->GetTechnoType()->GuardRange / 256.0) * 2.0));
		}

		return false;
	}
	case 35:
	{
		auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
		// Land Vehicle Factory & Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pTypeBuilding
			&& pTypeBuilding->Factory == AbstractType::UnitType);
	}
	case 36:
	{
		// Building that isn't a defense
		auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
		auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pTypeBuilding);

		return (!pTechno->Owner->IsNeutral()
			&& pTypeBuilding
			&& !pTypeBuilding->IsBaseDefense
			&& !(pTypeBuilding->Artillary
				|| pTypeBuilding->TickTank
				|| (pBuildingExt && pBuildingExt->IsJuggernaut)
				|| pTypeBuilding->ICBMLauncher
				|| pTypeBuilding->SensorArray));
	}
	case 39:
		// Occupyable Civilian  Building
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			auto pBTypeHere = static_cast<BuildingTypeClass*>(pTechnoType);

			if (pBuilding && pBTypeHere->CanBeOccupied && pBuilding->Occupants.Count == 0 && pBuilding->Owner->IsNeutral() && pBTypeHere->CanOccupyFire && pBTypeHere->TechLevel == -1 && pBuilding->GetHealthStatus() != HealthState::Red)
				return true;
			if (pBuilding && pBTypeHere->CanBeOccupied && pBuilding->Occupants.Count < pBTypeHere->MaxNumberOccupants && pBuilding->Owner == pTeamLeader->Owner && pBTypeHere->CanOccupyFire)
				return true;
		}
		break;
	case 40:
		// Self Building with Grinding=yes
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			if(auto pBuilding = abstract_cast<BuildingClass*>(pTechno)) {
				if(!pBuilding->Owner->IsNeutral()){
					auto pBTypeHere = static_cast<BuildingTypeClass*>(pTechnoType);
					if (pBTypeHere->Grinding && pBuilding->Owner == pTeamLeader->Owner)
					{
						return true;
					}
				}
			}
		}

		break;

	case 41:
		// Building with Spyable=yes
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
			{
				auto pBTypeHere = static_cast<BuildingTypeClass*>(pTechnoType);
				if (!pTechno->Owner->IsNeutral() && pBTypeHere->Spyable)
				{
					return true;
				}
			}
		}

		break;


	/*
	case 100:
		pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Useable Repair Hut
		if (pTypeBuilding)
		{
			auto cell = pTechno->GetMapCoords();
			if (pTypeBuilding->BridgeRepairHut && pTypeBuilding->Repairable)
				if (MapClass::Instance->IsBridgeRepairable(&cell))
					return true;
		}

		break;*/
	case 37:
	{
		if (!pTechno->Owner->IsNeutral()) {
			if (pTechno->WhatAmI() == AbstractType::Infantry) {
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
				return pTypeExt->IsHero.Get();
			}
		}
		break;
	}
	}

	return false;
}

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	auto pTeamType = pTeam->Type;

	auto const Iter = std::find_if(AITriggerTypeClass::Array->begin(), AITriggerTypeClass::Array->end(), [pTeamType](AITriggerTypeClass* const pTrig) {
		return (pTeamType
		&& ((pTrig->Team1 && pTrig->Team1 == pTeamType)
			|| (pTrig->Team2 && pTrig->Team2 == pTeamType)));
	});

	if (Iter != AITriggerTypeClass::Array->end()) {
		AITriggerTypeClass*  pTriggerType = *Iter;

		pTriggerType->Weight_Current += modifier;

		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum) {
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
		} else {
			if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
				pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
		}
	}
}

void ScriptExt::WaitIfNoTarget(TeamClass *pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam)) {
		if (attempts <= 0)
			pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
		else
			pTeamData->WaitNoTargetAttempts = attempts;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass *pTeam, double award = 0)
{
	if (award <= 0)
		award = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		if (award > 0)
			pTeamData->NextSuccessWeightAward = award;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
{
	if (idxScriptsList <= 0)
		idxScriptsList = pTeam->CurrentScript->GetCurrentAction().Argument;

	bool changeFailed = true;

	if (idxScriptsList >= 0 && RulesExt::Global()->AIScriptsLists.Count > 0) {
		auto const& objectsList = RulesExt::Global()->AIScriptsLists.GetItem(idxScriptsList);
		if (idxScriptsList < objectsList.Count && objectsList.Count > 0) {
			const int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);
			if(ScriptTypeClass* pNewScript = objectsList.GetItem(IdxSelectedObject)) {
				if (pNewScript->ActionsCount > 0) {
					changeFailed = false;
					pTeam->CurrentScript = nullptr;
					pTeam->CurrentScript = GameCreate<ScriptClass>(pNewScript);

					// Ready for jumping to the first line of the new script
					pTeam->CurrentScript->CurrentMission = -1;
					pTeam->StepCompleted = true;

					return;

				} else {
					pTeam->StepCompleted = true;
					Debug::Log("DEBUG: [%s] Aborting Script change because [%s] has 0 Action scripts!\n", pTeam->Type->ID, pNewScript->ID);

					return;
				}
			}
		}
	}

	// This action finished
	if (changeFailed) {
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Failed to change the Team Script with a random one!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	}
}

void ScriptExt::Mission_Attack_List(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0) {
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move(TeamClass *pTeam, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->GetCurrentAction().Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	bool noWaitLoop = false;
	FootClass *pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->Type->ID, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (IsEmpty(pTeam) && !pTeamData->TeamLeader)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (TechnoExt::IsActive(pUnit,true,true)) {
			if (auto pUnitType = pUnit->GetTechnoType()) {
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& static_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!TechnoExt::IsActive(pLeaderUnit,false , true))
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo)
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;

		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reasons: No Leader | Aircrafts without ammo)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		int targetMask = scriptArgument;
		selectedTarget = FindBestObject(pLeaderUnit, targetMask, calcThreatMode, pickAllies, attackAITargetType, idxAITargetTypeItem);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (TechnoExt::IsActive(pUnit, true,true))
				{
					auto pUnitType = pUnit->GetTechnoType();

					if (pUnit && pUnitType)
					{
						pUnit->CurrentTargets.Clear();

						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_not_okay && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						pUnit->SetDestination(selectedTarget, false);

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
							pUnit->QueueMission(Mission::Move, false);

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() != AbstractType::AircraftType)
						{
							pUnit->QueueMission(Mission::Move, false);
							pUnit->ObjectClickedAction(Action::Move, selectedTarget, false);

							if (pUnit->GetCurrentMission() != Mission::Move)
								pUnit->Mission_Move();
						}
					}
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.

			if (!noWaitLoop)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30); // No target? let's wait some frames

				return;
			}

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (new target NOT FOUND)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Move" mission in each team unit

		int moveDestinationMode = 0;
		moveDestinationMode = pTeamData->MoveMissionEndMode;
		bool bForceNextAction = ScriptExt::MoveMissionEndStatus(pTeam, pFocus, pLeaderUnit, moveDestinationMode);

		if (bForceNextAction)
		{
			pTeamData->MoveMissionEndMode = 0;
			pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Reached destination)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);

			return;
		}
	}
}

TechnoClass* ScriptExt::FindBestObject(TechnoClass *pTechno, int method, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TechnoClass *bestObject = nullptr;
	double bestVal = -1;
	HouseClass* enemyHouse = nullptr;
	auto pTechnoType = pTechno->GetTechnoType();

	if (!pTechno->Owner || !pTechnoType) {
		Debug::Log(__FUNCTION__" Error Occured ! \n");
		return nullptr;
	}

	// Favorite Enemy House case. If set, AI will focus against that House
	if (!pickAllies && pTechno->BelongsToATeam()) {
		if (auto pFoot = abstract_cast<FootClass*>(pTechno)) {
			if(pFoot->Team &&  pFoot->Team->FirstUnit->Owner) {
				int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;

				bool onlyTargetHouseEnemy = pFoot->Team->Type->OnlyTargetHouseEnemy;
				auto pHouseExt = HouseExt::ExtMap.Find(pFoot->Team->Owner);

				if (pHouseExt && pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
					onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

				if (onlyTargetHouseEnemy && enemyHouseIndex >= 0) {
						enemyHouse = HouseClass::Array->GetItem(enemyHouseIndex);
				}
			}
		}
	}

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++) {

		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();

		if ( !object || object == pTechno  || !objectType  || !object->Owner)
			continue;

		if (enemyHouse && !enemyHouse->Defeated && enemyHouse != object->Owner)
			continue;
		if(TechnoExt::IsActive(object, false)
			&& (((pickAllies && pTechno->Owner->IsAlliedWith(object))
				|| (!pickAllies && !pTechno->Owner->IsAlliedWith(object))
				|| (method == 39 && attackAITargetType == -1))))
		{
		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
				|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

void ScriptExt::Mission_Attack_List1Random(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		return;
	}

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		auto const& objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);
		auto const objectsListIter = make_iterator(objectsList);

		if (idxSelectedObject < 0 && !objectsListIter.empty() && !selected)
		{
			// Finding the objects from the list that actually exists in the map

			auto const objectFromIter = std::find_if(TechnoClass::Array->begin(), TechnoClass::Array->end(), [objectsListIter, pTeam](TechnoClass* objectFromList)
					{
						if (objectFromList && (objectsListIter.contains(objectFromList->GetTechnoType())
							&& TechnoExt::IsActive(objectFromList, false)))
						{
							if (pTeam->FirstUnit->Owner)
							{
								if (!objectFromList->IsMindControlled())
									return !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList)
									|| pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList);
								else
									return !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList->MindControlledBy) ||
									pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList->MindControlledBy);
							}
							else
								return true;
						}

						return false;

					});

			if (objectFromIter != TechnoClass::Array->end() && (*objectFromIter))
				validIndexes.push_back(objectsList.FindItemIndex((*objectFromIter)->GetTechnoType()));


			if (!validIndexes.empty())
			{
				idxSelectedObject = validIndexes[(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.size() - 1))];
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, attackAITargetType, validIndexes.size());
	}
}

void ScriptExt::Mission_Move_List(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType)
{
	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0) {
		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move_List1Random(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		return;
	}

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		auto const& objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);
		auto const objectsListIter = make_iterator(objectsList);

		// Still no random target selected
		if (idxSelectedObject < 0 && !objectsListIter.empty() && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			auto const objectFromIter = std::find_if(TechnoClass::Array->begin(), TechnoClass::Array->end(), [objectsListIter, pTeam, pickAllies](TechnoClass* objectFromList)
			{
				if (objectFromList && (objectsListIter.contains(objectFromList->GetTechnoType())
					&& TechnoExt::IsActive(objectFromList, false)))
				{
					if (pTeam->FirstUnit->Owner) {
						return (pickAllies && pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList)) || (!pickAllies
							&& !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList));
					}
					else
						return !pickAllies;
				}

				return false;
			});

			if ((*objectFromIter) && objectFromIter != TechnoClass::Array->end())
				validIndexes.push_back(objectsList.FindItemIndex((*objectFromIter)->GetTechnoType()));

			if (!validIndexes.empty())
			{
				idxSelectedObject = validIndexes[(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.size() - 1))];
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, attackAITargetType, validIndexes.size());
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass *pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (distance > 0)
			pTeamData->CloseEnough = distance;

		if (distance <= 0)
			pTeamData->CloseEnough = RulesClass::Instance->CloseEnough / 256.0;

	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, int mode = 0)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (mode >= 0 && mode <= 2)
			pTeamData->MoveMissionEndMode = mode;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader = nullptr, int mode = 0)
{
	if (!pTeam || !pFocus || mode < 0)
		return false;

	if (mode != 2 && mode != 1 && !pLeader)
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough / 256.0;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction;

	if (mode == 2)
		bForceNextAction = true;
	else
		bForceNextAction = false;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
		if (TechnoExt::IsActive(pUnit , true,true)) {
			if (!pUnit->Locomotor->Is_Moving_Now())
				pUnit->SetDestination(pFocus, false);

			if (mode == 2)
			{
				// Default mode: all members in range
				if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

					continue;
				}
				else
				{
					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
					{
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

						continue;
					}
				}
			}
			else
			{
				if (mode == 1)
				{
					// Any member in range
					if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
					{
						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, false);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
						{
							pUnit->QueueMission(Mission::Return, false);
							pUnit->Mission_Enter();

							continue;
						}
					}
				}
				else
				{
					// All other cases: Team Leader mode in range
					if (pLeader)
					{
						if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
						{
							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, false);

							continue;
						}
						else
						{
							if (pUnit == pLeader)
								bForceNextAction = true;

							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
							{
								pUnit->QueueMission(Mission::Return, false);
								pUnit->Mission_Enter();

								continue;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	return bForceNextAction;
}

void ScriptExt::SkipNextAction(TeamClass * pTeam, int successPercentage = 0)
{
	// This team has no units! END
	auto pTeamExt = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamExt || (IsEmpty(pTeam) && !pTeamExt->TeamLeader))
	{
		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: ScripType: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No TeamExt / No team members alive)\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->CurrentMission + 1, pTeam->CurrentScript->GetNextAction().Action,
			pTeam->CurrentScript->GetNextAction().Argument);

		return;
	}

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	if (ScenarioGlobal->Random.PercentChance(successPercentage))
	{
		Debug::Log("DEBUG: ScripType: [%s] [%s] (line: %d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->CurrentMission + 2,
			ScriptExt::GetSpecificAction(pTeam->CurrentScript, 2).Action,
			ScriptExt::GetSpecificAction(pTeam->CurrentScript, 2).Argument);
		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
{
	struct operation_set { int operator()(const int& a, const int& b) { return b; } };
	struct operation_add { int operator()(const int& a, const int& b) { return a + b; } };
	struct operation_minus { int operator()(const int& a, const int& b) { return a - b; } };
	struct operation_multiply { int operator()(const int& a, const int& b) { return a * b; } };
	struct operation_divide { int operator()(const int& a, const int& b) { return a / b; } };
	struct operation_mod { int operator()(const int& a, const int& b) { return a % b; } };
	struct operation_leftshift { int operator()(const int& a, const int& b) { return a << b; } };
	struct operation_rightshift { int operator()(const int& a, const int& b) { return a >> b; } };
	struct operation_reverse { int operator()(const int& a, const int& b) { return ~a; } };
	struct operation_xor { int operator()(const int& a, const int& b) { return a ^ b; } };
	struct operation_or { int operator()(const int& a, const int& b) { return a | b; } };
	struct operation_and { int operator()(const int& a, const int& b) { return a & b; } };

	int nLoArg = LOWORD(nArg);
	int nHiArg = HIWORD(nArg);

	switch (eAction)
	{
	case PhobosScripts::LocalVariableSet:
		VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	}
}

template<bool IsGlobal, class _Pr>
void ScriptExt::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(nVariable);
	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		itr->second.Value = _Pr()(itr->second.Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}
	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExt::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	auto itr = ScenarioExt::Global()->Variables[IsSrcGlobal].find(nVarToOperate);
	if (itr != ScenarioExt::Global()->Variables[IsSrcGlobal].end())
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;
	bool teamLeaderFound = false;

	if (!pTeam) {
		return pLeaderUnit;
	}

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit || !pUnit->Owner)
			continue;

		if (pUnit->WhatAmI() == AbstractType::Unit)
		if (auto pCell = pUnit->GetCell())
			if (auto pBld = pCell->GetBuilding())
				if (!pBld->Type->Naval && pBld->Type->WeaponsFactory && !pUnit->IsInAir())
					continue;

		// Preventing >1 leaders in teams
		if (teamLeaderFound)
		{
			pUnit->IsTeamLeader = false;
			continue;
		}

		if (TechnoExt::IsActive(pUnit,true,true))
		{
			if (pUnit->IsTeamLeader)
			{
				pLeaderUnit = pUnit;
				teamLeaderFound = true;
				continue;
			}

			if (auto pUnitType = pUnit->GetTechnoType())
			{
				// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
				int unitLeadershipRating = pUnitType->LeadershipRating;
				if (unitLeadershipRating > bestUnitLeadershipValue)
				{
					pLeaderUnit = pUnit;
					bestUnitLeadershipValue = unitLeadershipRating;
				}
			}
		}
		else
		{
			pUnit->IsTeamLeader = false;
		}
	}

	if (pLeaderUnit)
		pLeaderUnit->IsTeamLeader = true;

	return pLeaderUnit;
}

bool ScriptExt::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	if (const auto pHouseExt = HouseExt::ExtMap.Find(pTeam->Owner))
	{
		if (mode < 0 || mode > 2)
			mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

		if (mode < -1 || mode > 2)
			mode = -1;

		HouseExt::ForceOnlyTargetHouseEnemy(pTeam->Owner, mode);

	}

	// This action finished
	pTeam->StepCompleted = true;
}

#undef TECHNO_IS_ALIVE