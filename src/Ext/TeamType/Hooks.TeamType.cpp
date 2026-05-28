#include "Body.h"

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/House/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <Locomotor/TunnelLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>
#include <RadarEventClass.h>
#include <TaskForceClass.h>

#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

ASMJIT_PATCH(0x65DD4E, TeamTypeClass_CreateGroub_MissingOwner, 0x7)
{
	//GET(TeamClass*, pCreated, ESI);
	GET(TeamTypeClass*, pType, EBX);

	const auto pHouse = pType->GetHouse();
	if (!pHouse)
	{
		Debug::FatalErrorAndExit("Creating Team[%s] groub without proper Ownership may cause crash , Please check !", pType->ID);
	}

	R->EAX(pHouse);
	return 0x65DD55;
}

TeamClass* FakeTeamTypeClass::_CreateOneOf(HouseClass* pHouse){

	if (!pHouse) {
		pHouse = this->Owner;
		if (!pHouse){
			if (HouseClass::Index_IsMP(this->idxHouse)) {
				pHouse = HouseClass::FindByPlayerAt(this->idxHouse);
			}
		}
	}

	if (!pHouse) { 
		return nullptr;
	}

	if (!Unsorted::ScenarioInit()) {
		if (this->Max >= 0) {
			if (SessionClass::Instance->GameMode != GameMode::Campaign) {
				if (pHouse->GetTeamCount(this) >= this->Max) {
					return nullptr;
				}
			} else if (this->cntInstances >= this->Max) {
				return nullptr;
			}
		}
	}

	const auto pTeam = GameCreate<TeamClass>(this, pHouse, false);

	Debug::LogInfo("[{0} - {1}] Creating a new team named [{2} -{3}].",
		pHouse->get_ID(), (void*)pHouse, this->ID, (void*)pTeam);

	return pTeam;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6F09C0, FakeTeamTypeClass::_CreateOneOf)

ASMJIT_PATCH(0x6F09C0, TeamTypeClass_CreateOneOf_Handled, 0x9)
{
	GET(TeamTypeClass*, pThis, ECX);
	GET_STACK(DWORD, caller, 0x0);
	GET_STACK(HouseClass*, pHouse, 0x4);

	if (!pHouse)
	{
		pHouse = pThis->Owner;
		if (!pHouse)
		{
			if (HouseClass::Index_IsMP(pThis->idxHouse))
			{
				pHouse = HouseClass::FindByPlayerAt(pThis->idxHouse);
			}
		}
	}

	if (!pHouse)
	{
		R->EAX<TeamClass*>(nullptr);
		return 0x6F0A2C;
	}

	if (!Unsorted::ScenarioInit())
	{
		if (pThis->Max >= 0)
		{
			if (SessionClass::Instance->GameMode != GameMode::Campaign)
			{
				if (pHouse->GetTeamCount(pThis) >= pThis->Max)
				{
					R->EAX<TeamClass*>(nullptr);
					return 0x6F0A2C;
				}
			}
			else if (pThis->cntInstances >= pThis->Max)
			{
				R->EAX<TeamClass*>(nullptr);
				return 0x6F0A2C;
			}
		}
	}

	const auto pTeam = GameCreate<TeamClass>(pThis, pHouse, false);
	Debug::LogInfo("[{0} - {1}] Creating a new team named [{2} -{3}] caller [{4:x}].",
		pHouse->get_ID(), (void*)pHouse, pThis->ID, (void*)pTeam, caller);
	R->EAX(pTeam);
	return 0x6F0A2C;
}

// Dead code — superseded by DEFINE_FUNCTION_JUMP at 0x65D8E0; logic integrated into _DoReinforcement
//ASMJIT_PATCH(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
//{
//	GET(FootClass*, pFoot, EBP);
//	R->ECX(HouseExtData::GetParadropPlane(pFoot->Owner));
//	++Unsorted::ScenarioInit();
//	return 0x65DBD0;
//}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
//ASMJIT_PATCH(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
ASMJIT_PATCH(0x65EC4A, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if (pHouse &&
		(pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated))
	{
		return 0;
	}

	// no.
	return //(R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
	0x65F301;
}
// ============================================================================
// Full backport of _Create_Group (65DD30–65E00E)
// Integrates:
//   TeamTypeClass_CreateGroub_MissingOwner  (0x65DD4E) — dead after LJMP
//   TeamTypeClass_CreateGroup_IncreaseStorage (0x65DE6B) — dead after LJMP
//   TeamTypeClass_CreateMembers_LoadOntoTransport (0x65DF67) — dead after LJMP
// ============================================================================
FootClass* __fastcall FakeTeamTypeClass::_CreateGroup(TeamTypeClass* pType)
{
	// Integrate TeamTypeClass_CreateGroub_MissingOwner (0x65DD4E):
	HouseClass* pOwner = pType->GetHouse();
	if (!pOwner)
		Debug::FatalErrorAndExit("Creating Team[%s] groub without proper Ownership may cause crash , Please check !", pType->ID);

	auto* pTeam = GameCreate<TeamClass>(pType, pOwner, false);
	if (pTeam)
	{
		pTeam->IsForcedActive = true;
		pTeam->IsUnderStrength = false;
	}

	// Scan script for Unload mission
	bool hasUnload = false;
	if (auto* pScript = pType->ScriptType)
	{
		for (int i = 0; i < pScript->ActionsCount; ++i)
		{
			if (pScript->ScriptActions[i].Action == TeamMissionType::Unload)
			{
				hasUnload = true;
				break;
			}
		}
	}

	auto* pTaskForce = pType->TaskForce;
	const int classCount = pTaskForce->CountEntries;

	// Scan for naval units in task force
	bool hasNaval = false;
	for (int i = 0; i < classCount; ++i)
	{
		if (auto* pTType = pTaskForce->Entries[i].Type)
		{
			if (pTType->Naval)
			{
				hasNaval = true;
				break;
			}
		}
	}

	const bool hasAircraft = pTeam && pTeam->HasAircraft();

	FootClass* pTransport = nullptr;
	FootClass* pObject = nullptr;

	for (int index = 0; index < classCount; ++index)
	{
		auto* pTType = pTaskForce->Entries[index].Type;
		const int quantity = pTaskForce->Entries[index].Amount;

		for (int sub = 0; sub < quantity; ++sub)
		{
			++Unsorted::ScenarioInit();
			auto* pUnit = static_cast<FootClass*>(pTType->CreateObject(pOwner));
			--Unsorted::ScenarioInit();

			if (!pUnit)
				continue;

			// Integrate TeamTypeClass_CreateGroup_IncreaseStorage (0x65DE6B):
			// Original used StorageClass::Increase_Amount; replaced by TiberiumStorage.DecreaseLevel
			if (pTType->Storage > 0 && pType->Full)
				TechnoExtContainer::Instance.Find(pUnit)->TiberiumStorage.DecreaseLevel(float(pTType->Storage), 0);

			// Apply veterancy. VeteranLevel=1 is no-op per original code.
			switch (pType->VeteranLevel)
			{
			case 0: pUnit->Veterancy.SetRookie();  break;
			case 2: pUnit->Veterancy.SetVeteran(); break;
			case 3: pUnit->Veterancy.SetElite();   break;
			}

			if (pTeam)
			{
				++Unsorted::ScenarioInit();
				pTeam->AddMember(pUnit, false);
				--Unsorted::ScenarioInit();
				pUnit->IsTeamLeader = true;
			}

			// Classify unit as transport or payload
			const AbstractType kind = pUnit->WhatAmI();
			const bool isTransport = pTType->Passengers > 0
				&& pType->Full
				&& ((hasAircraft && kind == AbstractType::Aircraft)
					|| (!hasAircraft && kind == AbstractType::Unit))
				&& (pTType->Naval || !hasNaval);

			if (isTransport)
			{
				pUnit->NextObject = pTransport;
				pTransport = pUnit;
			}
			else
			{
				pUnit->NextObject = pObject;
				pObject = pUnit;
			}
		}
	}

	// Integrate TeamTypeClass_CreateMembers_LoadOntoTransport (0x65DF67):
	if (pTransport)
	{
		TechnoExtContainer::Instance.Find(pTransport)->PayloadCreated = false;

		if (!pObject || !pType->Full)
			return pTransport;

		// Enhanced passenger loading with OpenTopped/Gunner support
		const bool isOpenTopped = pTransport->GetTechnoType()->OpenTopped;
		FootClass* pGunner = nullptr;

		for (auto* pNext = pObject; pNext; pNext = static_cast<FootClass*>(pNext->NextObject))
		{
			if (pNext != pTransport && pNext->Team == pTeam)
			{
				pGunner = pNext;
				pNext->IsInPlayfield = true;
				pNext->Transporter = pTransport;
				if (isOpenTopped)
					pTransport->EnteredOpenTopped(pNext);
				pNext->SetLocation(pTransport->Location);
			}
		}

		pTransport->Passengers.AddPassenger(pObject);

		if (pTransport->GetTechnoType()->Gunner && pGunner)
			pTransport->ReceiveGunner(pGunner);

		// Original logic from 0x65DF8D: mark aircraft transport spawned if unloading
		if (!hasUnload || pTransport->WhatAmI() != AbstractType::Aircraft)
			return pTransport;

		pTransport->Spawned = true;
		return pTransport;
	}

	// No transport: clear PayloadCreated for all team members in the object chain
	for (auto* pNext = pObject; pNext && pNext->Team == pTeam;
		 pNext = static_cast<FootClass*>(pNext->NextObject))
	{
		TechnoExtContainer::Instance.Find(pNext)->PayloadCreated = false;
	}

	if (pObject)
		return pObject;

	GameDelete(pTeam);
	return nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x65DD30, FakeTeamTypeClass::_CreateGroup)

bool __fastcall FakeTeamTypeClass::_TunnelMaybe(TeamTypeClass* pType, FootClass* pGroup, CellStruct waypointCell, bool inRadar)
{
	const bool isDroppod = pType->DropPod;
	bool allTunnel = true;

	for (auto* pCheck = pGroup; pCheck; pCheck = static_cast<FootClass*>(pCheck->NextObject))
	{
		auto* pCheckType = GET_TECHNOTYPE(pCheck);
		if (!pCheckType || pCheckType->Locomotor != TunnelLocomotionClass::ClassGUID.get())
		{
			allTunnel = false;
			break;
		}
	}

	int edgeDir = 0;
	CellStruct spawnCell = waypointCell;

	if (isDroppod || allTunnel || inRadar)
	{
		CellStruct closeTo = CellStruct::Empty;
		auto* pLeaderType = pGroup ? GET_TECHNOTYPE(pGroup) : nullptr;
		const SpeedType speedType = pLeaderType ? pLeaderType->SpeedType : SpeedType::None;
		MapClass::Instance->NearByLocation(spawnCell, waypointCell, speedType, ZoneType::None, MovementZone::Normal,
			false, 1, 1, false, false, false, true, closeTo, false, false);
	}
	else
	{
		Edge edge = Edge::North;
		if (auto* pOwner = pType->GetHouse())
		{
			edge = pOwner->GetCurrentEdge();
			if (edge < Edge::North || edge > Edge::West)
				edge = Edge::North;
		}

		edgeDir = 2 * static_cast<int>(edge);
	}

	bool didPlaceAny = false;
	FootClass* pCurrent = pGroup;
	FootClass* pRemaining = pCurrent ? static_cast<FootClass*>(pCurrent->NextObject) : nullptr;
	if (pCurrent)
		pCurrent->NextObject = nullptr;

	while (pCurrent && (spawnCell.X != waypointCell.X || spawnCell.Y != waypointCell.Y))
	{
		const int baseFacing = (edgeDir << 13);
		const int rawFacing = (pCurrent->WhatAmI() == AbstractType::Aircraft)
			? ((baseFacing - 0x6001) & 0xE000)
			: baseFacing;
		const auto dir = static_cast<DirType>(static_cast<unsigned char>((((rawFacing >> 7) + 1) >> 1) & 0xFF));

		++Unsorted::ScenarioInit();

		bool placed = false;
		if (isDroppod)
		{
			const CoordStruct targetCoord { spawnCell.X * 256 + 128, spawnCell.Y * 256 + 128, 0 };
			pCurrent->SetLocation(targetCoord);
			pCurrent->SetDestination(MapClass::Instance->GetCellAt(spawnCell), true);
			if (pCurrent->Locomotor)
				pCurrent->Locomotor->Move_To(targetCoord);
			pCurrent->UpdateSight(false, 0, false, nullptr, 0);
			placed = true;
		}
		else if (allTunnel)
		{
			auto* pCell = MapClass::Instance->GetCellAt(spawnCell);
			const CoordStruct cellCoord = pCell->GetCoordsWithBridge();
			const CoordStruct unlimboCoord { cellCoord.X, cellCoord.Y, cellCoord.Z - 400 };

			placed = pCurrent->Unlimbo(unlimboCoord, dir);
			if (placed)
			{
				pCurrent->SetDestination(MapClass::Instance->GetCellAt(spawnCell), true);
				pCurrent->SetSpeedPercentage(1.0);
				const CoordStruct moveTo { spawnCell.X * 256 + 128, spawnCell.Y * 256 + 128, 0 };
				if (pCurrent->Locomotor)
					pCurrent->Locomotor->Move_To(moveTo);
			}
		}
		else
		{
			auto* pCell = MapClass::Instance->GetCellAt(spawnCell);
			const CoordStruct cellCoord = pCell->GetCoordsWithBridge();
			placed = pCurrent->Unlimbo(cellCoord, dir);
		}

		if (placed)
		{
			didPlaceAny = true;
			if (pCurrent->WhatAmI() != AbstractType::Aircraft)
			{
				pCurrent->QueueMission(Mission::Guard, false);
				pCurrent->NextMission();
			}

			if (isDroppod)
			{
				bool found = false;
				for (int i = 0; i < 8; ++i)
				{
					const auto& adj = CellSpread::AdjacentCell[i & 7];
					CellStruct candidate { static_cast<short>(spawnCell.X + adj.X), static_cast<short>(spawnCell.Y + adj.Y) };
					if (MapClass::Instance->CoordinatesLegal(candidate))
					{
						spawnCell = candidate;
						found = true;
						break;
					}
				}
				if (!found)
					spawnCell = waypointCell;
			}
		}
		else
		{
			bool found = false;
			for (int i = 0; i < 8; ++i)
			{
				const auto& adj = CellSpread::AdjacentCell[i & 7];
				CellStruct candidate { static_cast<short>(spawnCell.X + adj.X), static_cast<short>(spawnCell.Y + adj.Y) };

				if (!MapClass::Instance->CoordinatesLegal(candidate))
					continue;

				if (pCurrent->Locomotor && pCurrent->Locomotor->Can_Enter_Cell(candidate) == Move::OK)
				{
					spawnCell = candidate;
					found = true;
					break;
				}
			}

			if (!found || (spawnCell.X == waypointCell.X && spawnCell.Y == waypointCell.Y))
			{
				spawnCell = waypointCell;
				GameDelete(pCurrent);
				pCurrent = nullptr;
			}
		}

		if (pCurrent)
		{
			pCurrent = pRemaining;
			pRemaining = pCurrent ? static_cast<FootClass*>(pCurrent->NextObject) : nullptr;
			if (pCurrent)
				pCurrent->NextObject = nullptr;
		}

		--Unsorted::ScenarioInit();
	}

	if (pCurrent)
		GameDelete(pCurrent);

	while (pRemaining)
	{
		auto* pNext = static_cast<FootClass*>(pRemaining->NextObject);
		pRemaining->NextObject = nullptr;
		GameDelete(pRemaining);
		pRemaining = pNext;
	}

	return didPlaceAny;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x65E010, FakeTeamTypeClass::_TunnelMaybe)

// ============================================================================
// Full backport of Do_Reinforcements (65D8E0–65DD25)
// Integrates:
//   TeamTypeClass_ValidateHouse         (0x65D8FB) — dead at this address after LJMP
//   TeamTypeClass_CreateInstance_Plane  (0x65DBB3) — dead after LJMP
//   Do_Reinforcement_ValidateHouse      (0x65DC11) — dead after LJMP
// Note: ASMJIT_PATCH_AGAIN at 0x65EC4A (chrono reinforcements) is still active.
// ============================================================================
bool __fastcall FakeTeamTypeClass::_DoReinforcement(TeamTypeClass* pType, int waypoint)
{
	// Integrate TeamTypeClass_ValidateHouse (0x65D8FB):
	HouseClass* pOwner = pType->GetHouse();
	if (!pOwner ||
		(!pType->Owner && SessionClass::Instance->GameMode != GameMode::Campaign && pOwner->Defeated))
		return false;

	if (!pType->TaskForce || pType->TaskForce->CountEntries == 0)
		return false;

	// Ensure the script has at least one action (Guard) so the team doesn't idle permanently
	auto* pScript = pType->ScriptType;
	if (!pScript || pScript->ActionsCount == 0)
	{
		if (!pScript)
		{
			pScript = GameCreate<ScriptTypeClass>(nullptr);
			pType->ScriptType = pScript;
		}
		if (pScript && pScript->ActionsCount < ScriptTypeClass::MaxActions)
		{
			pScript->ScriptActions[pScript->ActionsCount] = ScriptActionNode(TeamMissionType::Guard, 0);
			++pScript->ActionsCount;
		}
	}

	const bool isDroppod = pType->DropPod;
	auto* pGroup = FakeTeamTypeClass::_CreateGroup(pType);

	if (!pGroup)
		return false;

	if (pGroup->Team)
		pGroup->Team->IsTransient = false;

	// Determine spawn cell
	CellStruct spawnCell;
	const bool hasSpecificWaypoint = (waypoint != -1);
	if (hasSpecificWaypoint)
		ScenarioClass::Instance->GetWaypointCoords(&spawnCell, waypoint);
	else
		pType->GetWaypoint(&spawnCell);

	const bool isInvalidCell = (spawnCell.X == -1 && spawnCell.Y == -1);

	// Infantry-from-building pop path
	if (!isDroppod && !isInvalidCell)
	{
		bool infantryOnly = true;
		for (auto* pUnit = pGroup; pUnit; pUnit = static_cast<FootClass*>(pUnit->NextObject))
		{
			if (pUnit->WhatAmI() != AbstractType::Infantry)
			{
				infantryOnly = false;
				break;
			}
		}

		if (infantryOnly)
		{
			CellClass* pCell = MapClass::Instance->GetCellAt(spawnCell);
			BuildingClass* pCandidate = nullptr;

			// Check the cell and its 8 neighbours for a suitable building exit
			for (int f = -1; f < 8; ++f)
			{
				CellClass* pCheck = (f == -1) ? pCell : pCell->GetAdjacentCell(static_cast<FacingType>(f));
				if (!pCheck)
					continue;
				BuildingClass* pBld = pCheck->GetBuilding();
				if (pBld && pBld->Health > 0)
				{
					using HasExitCellFn = bool(__thiscall*)(BuildingClass*);
					if (reinterpret_cast<HasExitCellFn>(0x459CA0)(pBld))
						pCandidate = pBld;
				}
			}

			if (pCandidate)
			{
				int exitCount = 0;
				FootClass* pCurrent = pGroup;
				while (pCurrent)
				{
					FootClass* pNext = static_cast<FootClass*>(pCurrent->NextObject);
					pCurrent->NextObject = nullptr;

					if (pCandidate->KickOutUnit(pCurrent, spawnCell) == KickOutResult::Succeeded)
					{
						pCandidate->SendToFirstLink(RadioCommand::NotifyUnlink);
						++exitCount;
					}
					else
					{
						GameDelete(pCurrent);
					}

					pCurrent = pNext;
				}
				return exitCount > 0;
			}
		}
	}

	// LABEL_45: droppod OR invalid cell OR non-infantry OR no building candidate found
	const bool inRadar = hasSpecificWaypoint && MapClass::Instance->IsWithinUsableArea(spawnCell, true);
	bool doRadarEvent = false;

	if (isDroppod)
	{
		// Integrate TeamTypeClass_CreateInstance_Plane (0x65DBB3):
		// Use per-house paradrop plane (HouseExtData) instead of global PDPLANE
		auto* pPlaneType = HouseExtData::GetParadropPlane(pGroup->Owner);
		if (!pPlaneType)
			return true;

		++Unsorted::ScenarioInit();
		auto* pPlane = static_cast<AircraftClass*>(pPlaneType->CreateObject(pGroup->Owner));
		--Unsorted::ScenarioInit();

		if (!pPlane)
			return true;

		pPlane->Spawned = true;

		// Determine which map edge to spawn the paradrop plane on
		CellStruct planeBuf;
		if (pType->UseTransportOrigin)
		{
			pType->GetTransportWaypoint(&planeBuf);
		}
		else
		{
			// Integrate Do_Reinforcement_ValidateHouse (0x65DC11) edge logic:
			Edge spawnEdge;
			if (!pGroup->Owner)
			{
				spawnEdge = Edge::North;
			}
			else if (pGroup->Owner->StaticData.StartingEdge < Edge::North
				  || pGroup->Owner->StaticData.StartingEdge > Edge::West)
			{
				spawnEdge = pGroup->Owner->GetHouseEdge();
			}
			else
			{
				spawnEdge = pGroup->Owner->StaticData.StartingEdge;
			}

			static constexpr CellStruct kNoCell { -1, -1 };
			MapClass::Instance->PickCellOnEdge(planeBuf, spawnEdge,
				kNoCell, kNoCell, SpeedType::Winged, true, MovementZone::Normal);
		}

		pPlane->QueueMission(Mission::ParadropApproach, false);
		pPlane->SetDestination(nullptr, true);
		pPlane->SetTarget(MapClass::Instance->GetCellAt(spawnCell));

		const CoordStruct spawnCoord { planeBuf.X * 256 + 128, planeBuf.Y * 256 + 128, 0 };
		++Unsorted::ScenarioInit();
		const bool placed = pPlane->Unlimbo(spawnCoord, DirType::North);
		--Unsorted::ScenarioInit();

		if (!placed)
		{
			GameDelete(pPlane);
			return true;
		}

		pPlane->HasPassengers = true;
		pPlane->EnterAsPassenger(pGroup);
		pPlane->NextMission();
		doRadarEvent = true;
	}
	else
	{
		doRadarEvent = FakeTeamTypeClass::_TunnelMaybe(pType, pGroup, spawnCell, inRadar);
	}

	if (doRadarEvent)
	{
		HouseClass* pTeamOwner = pType->GetHouse();
		if (pTeamOwner && pTeamOwner->IsAlliedWith(HouseClass::CurrentPlayer.get()))
			RadarEventClass::Create(spawnCell);
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x65D8E0, FakeTeamTypeClass::_DoReinforcement)