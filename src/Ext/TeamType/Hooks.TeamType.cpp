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

	if (pTeam) {
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

	const bool hasAircraft = pTeam && ((FakeTeamClass*)pTeam)->_has_aircraft();

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
				((FakeTeamClass*)pTeam)->_Add2(pUnit, false);
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

	//call DTOR , dont delete the pointer immedietely , let the game process it
	// and handle it automatically
	GameDelete<true, false>(pTeam);
	return nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x65DD30, FakeTeamTypeClass::_CreateGroup)

bool __fastcall FakeTeamTypeClass::_TunnelMaybe(TeamTypeClass* teamtype, FootClass* group, CellStruct* pCell, bool onRadar)
{
	const bool isDropPod = teamtype->DropPod != 0;      // +0xB0

	// --- is every member a tunnel-locomotor unit? (empty list => true) -----
	bool allTunnel = true;
	for (auto* pCheck = group; pCheck; pCheck = static_cast<FootClass*>(pCheck->NextObject)) {
		auto* pCheckType = GET_TECHNOTYPE(pCheck);
		if (!pCheckType || pCheckType->Locomotor != TunnelLocomotionClass::ClassGUID.get())
		{
			allTunnel = false;
			break;
		}
	}


	// --- resolve working cell + direction base ----------------------------
	CellStruct cell2 = *pCell;
	int dirBase = 0;                                      // v57

	if (isDropPod || allTunnel || onRadar) {
		CellStruct closeTo = CellStruct::Empty;
		auto* pLeaderType = group ? GET_TECHNOTYPE(group) : nullptr;
		SpeedType speedType = pLeaderType ? pLeaderType->SpeedType : SpeedType::None;
		MapClass::Instance->NearByLocation(cell2, *pCell, speedType, ZoneType::None, MovementZone::Normal,
			false, 1, 1, false, false, false, true, closeTo, false, false);
	} else
	{
		Edge edge = Edge::North;
		if (auto* pOwner = teamtype->GetHouse())
		{
			edge = pOwner->GetCurrentEdge();
			if (edge < Edge::North || edge > Edge::West)
				edge = Edge::North;
		}

		dirBase = 2 * static_cast<int>(edge);
	}

	// --- pop the head off the list; set up the cursor ---------------------
	// SUSPECT (#3): group assumed non-null here.
	FootClass* unit = group;                              // v5
	FootClass* rest = group ? static_cast<FootClass*>(group->NextObject) : nullptr;                        // v13 / v55
	if (group)
		group->NextObject = nullptr;

	CellStruct place = cell2;                             // v54 (persistent cursor)
	bool placedAny = false;                               // a4 byte
	bool retVal = false;                               // v14 / bl

	if (!cell2.IsValid())
	{
		// loc_65E601: initial target is the invalid marker -> drop the head.
		if (unit)
			GameDelete<true, false>(unit);
		// retVal stays false
	}
	else
	{
		while (true)
		{
			// --- loop top (loc_65E15E) ---
			if (!place.IsValid() || !unit)
			{
				retVal = placedAny;                       // loc_65E611
				break;
			}

			// --- direction value (Kind_Of special-cases aircraft) ---
			std::uint16_t dirVal = (std::uint16_t)((unsigned)dirBase << 13);
			if (unit->WhatAmI() == AbstractType::Aircraft)         // vtable+0x2C; 2
				dirVal = (std::uint16_t)((((unsigned)dirBase << 13) - 0x6001u) & 0xE000u);
			++Unsorted::ScenarioInit();

			// --- attempt placement ---
			bool placed;
			if (isDropPod)
			{
				CoordStruct center = CellClass::Cell2Coord(place);
				unit->SetLocation(center);
				unit->SetDestination(MapClass::Instance->GetCellAt(place), true);
				unit->Locomotor->Move_To(center);
				unit->UpdateSight(false, 0, false, nullptr, 0);
				MapClass::Instance->RevealArea3(&unit->Location, 0, unit->LastSightRange + 3, false);
				placed = true;                            // droppod always proceeds to LABEL_34
			}
			else
			{
				const int dir = (((dirVal >> 7) + 1) >> 1) & 0xFF;
				if (allTunnel)
				{
					const CoordStruct cellCoord = MapClass::Instance->GetCellAt(place)->GetCoords();
					const CoordStruct unlimboCoord { cellCoord.X, cellCoord.Y, cellCoord.Z - 400 };
					placed = unit->Unlimbo(unlimboCoord, (DirType)dir);  // +0xD8
					if (placed)
					{
						int z = unit->GetZ();
						CoordStruct adj { unit->Location.X, unit->Location.Y,
										unit->Location.Z - 256 - z };
						unit->SetLocation(adj);
						unit->SetDestination(MapClass::Instance->GetCellAt(place), true);
						unit->SetSpeedPercentage(1.0);
						unit->Locomotor->Move_To(CellClass::Cell2Coord(place));
					}
				}
				else
				{
					const CoordStruct cellCoord = MapClass::Instance->GetCellAt(place)->GetCoordsWithBridge();
					placed = unit->Unlimbo(cellCoord, (DirType)dir);     // +0xD8
				}
			}

			if (placed)
			{
				// --- LABEL_34: success ---
				placedAny = true;
				if (unit->WhatAmI() != AbstractType::Aircraft)
				{
					unit->QueueMission(Mission::Guard, 0);   // +0x1E8; 5
					unit->NextMission();                         // +0x1EC
				}

				if (isDropPod)
				{
					// scatter: move the cursor to the first in-map neighbour
					CellStruct scatter = CellStruct::Empty;      // default when none found
					for (int i = 0; i < 8; ++i)
					{
						const auto& adj = CellSpread::AdjacentCell[i & 7];
						CellStruct candidate { static_cast<short>(place.X + adj.X), static_cast<short>(place.Y + adj.Y) };

						if (MapClass::Instance->CoordinatesLegal(candidate))
						{
							scatter = candidate;
							break;
						}
					}
					place = scatter;
				}
				// -> advance
			}
			else
			{
				// --- unlimbo failed: hunt an off-map edge cell we can enter ---
				bool found = false;
				for (int i = 0; i < 8; ++i)
				{
					const auto& adj = CellSpread::AdjacentCell[i & 7];
					CellStruct candidate { static_cast<short>(place.X + adj.X), static_cast<short>(place.Y + adj.Y) };
					if (!MapClass::Instance->CoordinatesLegal(candidate) &&
						unit->IsCellOccupied(MapClass::Instance->GetCellAt(candidate), i , -1 , nullptr , true) == Move::OK)  // +0x1AC
					{
						place = candidate;
						found = true;
						break;
					}
				}
				if (!found)
					place = CellStruct::Empty;

				if (found && place.IsValid())
				{
					--Unsorted::ScenarioInit();
					continue;                             // retry SAME unit at the new cell
				}

				GameDelete<true, false>(unit);                         // give up on this unit
				// -> advance
			}

			// --- advance to the next unit (LABEL_49 / loc_65E5AE) ---
			unit = rest;
			--Unsorted::ScenarioInit();
			if (rest)
			{
				FootClass* next = flag_cast_to<FootClass*>(rest->NextObject);
				rest->NextObject = nullptr;                     // detach the node now held in `unit`
				rest = next;
			}
		}
	}

	// --- cleanup: delete every unit left on the list (LABEL_57) -----------
	while (rest)
	{
		FootClass* next = static_cast<FootClass*>(rest->NextObject);
		rest->NextObject = nullptr;
		GameDelete<true, false>(rest);                                 // ORIG had a dead `if (rest)` guard
		rest = next;
	}

	return retVal;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x65E010, FakeTeamTypeClass::_TunnelMaybe)

bool __fastcall Do_Reinforcements_TunnelMaybe(
	TeamTypeClass* teamtype,
	FootClass* edx0, 
	CellStruct* arg0l, 
	bool inRadar
) {
	JMP_FAST(0x65E010);
}

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

	const bool isDroppod = pType->DropPod != 0;
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

	auto findPopOutBuilding = [](const CellStruct& at) {
		BuildingClass* found = nullptr;
		CellClass* base = MapClass::Instance->GetCellAt(at);
		for (int f = -1; f < 8; ++f)                                   // self + 8 neighbours
		{
			CellClass* c = (f == -1) ? base : base->GetAdjacentCell((FacingType)f);
			BuildingClass* b = c->GetBuilding();
			if (b && b->Health > 0 && b->HasValidExitCell())
				found = b;
		}

		return found; 
	};

	bool deliver = isDroppod || !spawnCell.IsValid();

	// Infantry-from-building pop path
	if (!deliver)
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

		if (!infantryOnly) {
			deliver = true;                                           // fall through to delivery
		}else if (BuildingClass* candidate = findPopOutBuilding(spawnCell)) {
			int exitCount = 0;
			FootClass* pCurrent = pGroup;
			while (pCurrent) {
				FootClass* pNext = static_cast<FootClass*>(pCurrent->NextObject);
				pCurrent->NextObject = nullptr;

				const int kind = (int)pCurrent->WhatAmI();               // SUSPECT: constant, re-evaled (vanilla)
				if (kind >= 1 && kind <= 2) {
					candidate->Passengers.AddPassenger(pCurrent);
					pCurrent->Undiscover();                                    // vftable+0x11C
					pCurrent->SetLocation(candidate->Location);
					candidate->QueueMission(Mission::Unload, 0);     // vftable+0x1E8; 0x10
					++exitCount;
				} else if (kind == 6) {
					if (candidate->KickOutUnit(pCurrent, spawnCell) == KickOutResult::Succeeded) {
						candidate->SendToFirstLink(RadioCommand::NotifyUnlink);
						++exitCount;
					} else {
						GameDelete<true, false>(pCurrent);
					}
				}
				else   // kind <= 0, or 3/4/5/7+  (ORIG had a dead `if (v24)` guard)
				{
					GameDelete<true, false>(pCurrent);
				}

				pCurrent = pNext;
			}
			return exitCount > 0;
		}
		else
		{
			deliver = true;                                           // no candidate -> delivery 
		}
	}

	// LABEL_45: droppod OR invalid cell OR non-infantry OR no building candidate found
	const bool inRadar = hasSpecificWaypoint && MapClass::Instance->IsWithinUsableArea(spawnCell, true);
	bool doRadarEvent = false;

	if (isDroppod) {
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
		if (pType->UseTransportOrigin) {
			pType->GetTransportWaypoint(&planeBuf);
		} else {
			// Integrate Do_Reinforcement_ValidateHouse (0x65DC11) edge logic:
			Edge spawnEdge;

			if (!pGroup->Owner) {
				spawnEdge = Edge::North;
			}
			else if (pGroup->Owner->StaticData.StartingEdge < Edge::North
				  || pGroup->Owner->StaticData.StartingEdge > Edge::West) {
				spawnEdge = pGroup->Owner->GetHouseEdge();
			} else {
				spawnEdge = pGroup->Owner->StaticData.StartingEdge;
			}

			planeBuf = AircraftExtData::PickEdgeCellForPlane(pPlaneType, spawnCell, spawnEdge);
		}

		pPlane->QueueMission(Mission::ParadropApproach, false);
		pPlane->SetDestination(nullptr, true);
		pPlane->SetTarget(MapClass::Instance->GetCellAt(spawnCell));

		const CoordStruct spawnCoord { planeBuf.X * 256 + 128, planeBuf.Y * 256 + 128, 0 };
		const bool placed = AircraftExtData::PlaceReinforcementAircraft(pPlane, CellClass::Coord2Cell(spawnCoord));

		if (!placed)
 {
			GameDelete<true, false>(pPlane);
			return true;
		}

		pPlane->HasPassengers = true;
		pPlane->EnterAsPassenger(pGroup);
		pPlane->NextMission();
		doRadarEvent = true;
	}
	else {
		doRadarEvent = Do_Reinforcements_TunnelMaybe(pType, pGroup, &spawnCell, inRadar);
	}

	if (doRadarEvent) {
		HouseClass* pTeamOwner = pType->GetHouse();
		if (pTeamOwner && pTeamOwner->IsAlliedWith(HouseClass::CurrentPlayer.get()))
			RadarEventClass::Create(spawnCell);
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x65D8E0, FakeTeamTypeClass::_DoReinforcement)