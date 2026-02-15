#include "Body.h"

#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>
#include <TechnoTypeClass.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <AnimClass.h>
#include <VocClass.h>
#include <ParasiteClass.h>
#include <SlaveManagerClass.h>
#include <FacingClass.h>
#include <HouseClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>
#include <Memory.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// =====================================================
// Static member definitions
// =====================================================

const CoordStruct FakeTeleportLocomotionClass::DefaultCoords { 0, 0, 0 };

// =====================================================
// Helper methods
// =====================================================

bool FakeTeleportLocomotionClass::IsDefaultCoords(const CoordStruct& coords)
{
	return coords == DefaultCoords;
}

void FakeTeleportLocomotionClass::ClearOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords)
{
	if (pLinkedTo)
		pLinkedTo->UnmarkAllOccupationBits(coords);
}

void FakeTeleportLocomotionClass::SetOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords)
{
	if (pLinkedTo)
		pLinkedTo->MarkAllOccupationBits(coords);
}

// [Improvement] Integrated from Hooks.Transport.cpp 0x718F1E/0x7190B0
// Converts special movement zones to ground-equivalent for pathfinding
MovementZone FakeTeleportLocomotionClass::AdjustMovementZone(MovementZone mzone)
{
	switch (mzone)
	{
	case MovementZone::Fly:
	case MovementZone::Destroyer:
		return MovementZone::Normal;
	case MovementZone::AmphibiousDestroyer:
		return MovementZone::Amphibious;
	default:
		return mzone;
	}
}

// [Improvement] Integrated from Hooks.Unit.cpp 0x718871
// Determines if a unit type can float on water based on its movement characteristics
bool FakeTeleportLocomotionClass::CanFloatOnWater(TechnoTypeClass* pType, FootClass* pUnit)
{
	if (!pType)
		return false;

	switch (pType->MovementZone)
	{
	case MovementZone::Amphibious:
	case MovementZone::AmphibiousCrusher:
	case MovementZone::AmphibiousDestroyer:
	case MovementZone::WaterBeach:
		return true;
	default:
		break;
	}

	if (pType->SpeedType == SpeedType::Hover)
	{
		// Hover units can float unless they are powered units without power centers
		if (!pType->PoweredUnit)
			return true;

		// PoweredUnit hover - check if the house has active power centers for this type
		// HasPoweredCenter (0x50E1B0) actually returns BOOL (this->PoweredUnitCenters > 0)
		// YRpp declares it as void, so we call through a typed function pointer.
		if (pUnit && pUnit->Owner)
		{
			using HasPoweredCenterFunc = BOOL(__thiscall*)(const HouseClass*, TechnoTypeClass*);
			return reinterpret_cast<HasPoweredCenterFunc>(0x50E1B0)(pUnit->Owner, const_cast<TechnoTypeClass*>(pType)) != 0;
		}

		return false;
	}

	return false;
}

void FakeTeleportLocomotionClass::SinkUnit(FootClass* pUnit)
{
	if (!pUnit)
		return;

	pUnit->IsSinking = true;
	pUnit->Stun();

	if (auto pSlaveManager = pUnit->SlaveManager)
	{
		pSlaveManager->Killed(nullptr, nullptr);

		if (pUnit->SlaveManager)
		{
			GameDelete<true>(pUnit->SlaveManager);
			pUnit->SlaveManager = nullptr;
		}
	}
}

void FakeTeleportLocomotionClass::PlayChronoSound(FootClass* pUnit, int typeSound, int rulesSound)
{
	int sound = typeSound;
	if (sound == -1)
		sound = rulesSound;

	if (sound != -1)
	{
		CoordStruct coords = pUnit->Location;
		VocClass::PlayIndexAtPos(sound, coords, false);
	}
}

// =====================================================
// 0x718260 - InternalMark
// Handles collision detection and cell occupation during teleportation.
// [Improvement] Integrates hooks: 0x718275, 0x7184CE, 0x7185DA, 0x71872C, 0x7187DA
// =====================================================

bool FakeTeleportLocomotionClass::HandleDestinationCollisions(
	TeleportLocomotionClass* pLoco,
	const CoordStruct& destCoord,
	CellClass* pDestCell)
{
	if (!pLoco || !pLoco->LinkedTo || !pDestCell)
		return false;

	FootClass* pLinkedTo = pLoco->LinkedTo;
	bool needAlternateLocation = false;
	const bool bLinkedIsInfantry = (pLinkedTo->WhatAmI() == AbstractType::Infantry);

	// Get the appropriate occupier list based on bridge status
	// Original: checks Bitfield2 & 0x100 to decide OccupierPtr vs AltOccupierPtr
	ObjectClass* pOccupier = pDestCell->GetContentB();

	for (ObjectClass* pObj = pOccupier; pObj; pObj = pObj->NextObject)
	{
		// [Improvement] Integrated from Hooks.Unit.cpp 0x7187DA - prevent self-crush
		if (pObj == pLinkedTo)
			continue;

		const bool bObjIsInfantry = (pObj->WhatAmI() == AbstractType::Infantry);
		bool bIsImmune = pObj->IsIronCurtained();

		// [Improvement] Integrated from Hooks.Locomotor.cpp 0x718275 - Chronoshift_Crushable
		TechnoTypeClass* pType = pObj->GetTechnoType();
		if (pType)
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.TryFind(pType);
			if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
				bIsImmune = true;
		}

		// [Improvement] Integrated from Hooks.Locomotor.cpp 0x718275 - ChronoInfantryCrush
		if (!RulesExtData::Instance()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry)
		{
			int damage = pLinkedTo->GetTechnoType()->Strength;
			pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			break;
		}

		if (!bIsImmune && bObjIsInfantry && bLinkedIsInfantry)
		{
			// Infantry vs Infantry at exact same coordinates - target takes damage
			CoordStruct objCoord = pObj->GetCoords();
			if (objCoord == destCoord)
			{
				int damage = pObj->GetTechnoType()->Strength;
				pObj->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
		}
		else if (bIsImmune || ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		{
			if ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
			{
				// Non-foot object blocking (e.g. building) - need alternate location
				needAlternateLocation = true;
			}
			else if (bIsImmune)
			{
				// Iron curtained or immune target - teleporting unit takes damage
				int damage = pLinkedTo->GetTechnoType()->Strength;
				pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				break;
			}
		}
		else
		{
			// Regular foot unit at destination - they take damage
			int damage = pObj->GetTechnoType()->Strength;
			pObj->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	// Check if bridge exists but body is destroyed (0x100 set, 0x200 not set)
	if (pDestCell->ContainsBridge() && !pDestCell->ContainsBridgeBody())
		needAlternateLocation = true;

	return needAlternateLocation;
}

bool FakeTeleportLocomotionClass::FindAlternateLocation(
	TeleportLocomotionClass* pLoco,
	const CoordStruct& destCoord,
	CellClass* pSourceCell,
	bool sourceCellHasBridge)
{
	if (!pLoco || !pLoco->LinkedTo || !pSourceCell)
		return false;

	FootClass* pLinkedTo = pLoco->LinkedTo;
	TechnoTypeClass* pType = pLinkedTo->GetTechnoType();
	if (!pType)
		return false;

	// [Improvement] Integrated from Hooks.Transport.cpp 0x718F1E - proper movement zone
	MovementZone mzone = AdjustMovementZone(pType->MovementZone);

	CellStruct destCell = CellClass::Coord2Cell(destCoord);
	CellClass* pDestCell = MapClass::Instance->GetCellAt(destCell);

	bool destHasBridge = pDestCell->ContainsBridge();

	// [Improvement] Integrated from Spawner/Bugfixes.cpp 0x7184CE - correct zone lookup
	ZoneType zone = MapClass::Instance->GetMovementZoneType(
		pSourceCell->MapCoords,
		mzone,
		sourceCellHasBridge);

	CellStruct closeTo = CellStruct::Empty;
	CellStruct resultCell;

	MapClass::Instance->NearByLocation(
		resultCell,
		destCell,
		SpeedType::Track,
		zone,
		mzone,
		destHasBridge,
		1, 1,       // SpaceSizeX, SpaceSizeY
		false,      // disallowOverlay
		false,      // a11
		false,      // requireBurrowable
		true,       // allowBridge
		closeTo,
		false,      // a15
		false       // buildable
	);

	// [Improvement] Integrated from Spawner/Bugfixes.cpp 0x7185DA - fallback to current position
	if (!resultCell.IsValid())
	{
		pLinkedTo->ChronoDestCoords = pLinkedTo->Location;
		Debug::Log("[TeleportLoco] FindAlternateLocation: No valid cell found, falling back to current position for %s\n",
			pLinkedTo->GetTechnoType()->get_ID());
		return true;
	}

	// Calculate offset from original destination cell center to the actual coord
	CellClass* pOrigDestCell = MapClass::Instance->GetCellAt(destCoord);
	CoordStruct origCellCenter = pOrigDestCell->GetCoordsWithBridge();
	int offsetX = destCoord.X - origCellCenter.X;
	int offsetY = destCoord.Y - origCellCenter.Y;
	int offsetZ = destCoord.Z - origCellCenter.Z;

	// Apply offset to new cell center
	CellClass* pNewCell = MapClass::Instance->GetCellAt(resultCell);
	CoordStruct newCellCenter = pNewCell->GetCoordsWithBridge();

	pLinkedTo->ChronoDestCoords.X = newCellCenter.X + offsetX;
	pLinkedTo->ChronoDestCoords.Y = newCellCenter.Y + offsetY;
	pLinkedTo->ChronoDestCoords.Z = newCellCenter.Z + offsetZ;

	return true;
}

void FakeTeleportLocomotionClass::FinalizeDestination(TeleportLocomotionClass* pLoco)
{
	if (!pLoco || !pLoco->LinkedTo)
		return;

	FootClass* pLinkedTo = pLoco->LinkedTo;

	// Clear old occupation bit
	if (IsDefaultCoords(pLoco->LastCoords))
	{
		CoordStruct currentCoord = pLinkedTo->Location;
		ClearOccupyBit(pLinkedTo, currentCoord);
	}
	else
	{
		ClearOccupyBit(pLinkedTo, pLoco->LastCoords);
	}

	// Update LastCoords to new chrono destination
	pLoco->LastCoords = pLinkedTo->ChronoDestCoords;

	// Get proper Z height for the destination
	pLoco->LastCoords.Z = MapClass::Instance->GetCellFloorHeight(pLoco->LastCoords);

	// Handle bridge adjustment
	CellClass* pDestCell = MapClass::Instance->GetCellAt(pLoco->LastCoords);

	if (!pDestCell->ContainsBridge() || pLinkedTo->OnBridge)
	{
		pLinkedTo->OnBridge = false;
	}
	else
	{
		pLinkedTo->OnBridge = true;
		pLoco->LastCoords.Z += CellClass::BridgeHeight;
	}

	// Set new occupation bit
	SetOccupyBit(pLinkedTo, pLoco->LastCoords);
}

bool __fastcall FakeTeleportLocomotionClass::Hook_InternalMark(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	int destX, int destY, int destZ, int mark)
{
	Debug::Log("[TeleportLoco] InternalMark(0x718260): Coords(%d,%d,%d) Mark=%d Unit=%s\n",
		destX, destY, destZ, mark,
		pThis && pThis->LinkedTo ? pThis->LinkedTo->GetTechnoType()->get_ID() : "NULL");

	if (!pThis || !pThis->LinkedTo)
		return false;

	FootClass* pLinkedTo = pThis->LinkedTo;
	CoordStruct destCoord { destX, destY, destZ };

	if (mark != 0)
	{
		// Finalizing teleport: clear old occupation and set new
		FinalizeDestination(pThis);
	}
	else
	{
		// Starting teleport: check for collisions at destination
		CellClass* pDestCell = MapClass::Instance->GetCellAt(destCoord);

		bool needAlternate = HandleDestinationCollisions(pThis, destCoord, pDestCell);

		if (needAlternate)
		{
			// Get source cell info for pathfinding
			CoordStruct sourceCoord = pLinkedTo->GetCoords();
			CellStruct sourceCell = CellClass::Coord2Cell(sourceCoord);
			CellClass* pSourceCell = MapClass::Instance->GetCellAt(sourceCell);
			bool sourceCellHasBridge = pSourceCell->ContainsBridge();

			FindAlternateLocation(pThis, destCoord, pSourceCell, sourceCellHasBridge);

			// Return false = destination was changed, caller should retry
			return false;
		}
	}

	// [Improvement] Integrated from Hooks.BugFixes.cpp 0x71872C - occupation fix
	// Only set occupation if the unit is valid and not sinking/dead
	if (!pLinkedTo->InLimbo && pLinkedTo->IsAlive && pLinkedTo->Health > 0 && !pLinkedTo->IsSinking)
	{
		CoordStruct finalCoord;
		if (IsDefaultCoords(pThis->LastCoords))
			finalCoord = pLinkedTo->Location;
		else
			finalCoord = pThis->LastCoords;

		SetOccupyBit(pLinkedTo, finalCoord);
	}

	return true;
}

// =====================================================
// 0x7187A0 - Unwarp
// Handles the consequences of a unit teleporting into a cell.
// Deals with water landing, iron curtain collisions, sinking, etc.
// [Improvement] Integrates hooks: 0x7187DA, 0x7188F2, 0x718871
// =====================================================

void __fastcall FakeTeleportLocomotionClass::Hook_Unwarp(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	int coordX, int coordY, int coordZ)
{
	if (!pThis || !pThis->LinkedTo)
		return;

	CoordStruct destCoord { coordX, coordY, coordZ };
	FootClass* pLinkedTo = pThis->LinkedTo;
	TechnoTypeClass* pType = pLinkedTo->GetTechnoType();
	if (!pType)
		return;

	Debug::Log("[TeleportLoco] Unwarp(0x7187A0): Coords(%d,%d,%d) Unit=%s\n",
		coordX, coordY, coordZ, pType->get_ID());

	CellClass* pDestCell = MapClass::Instance->GetCellAt(destCoord);

	// Check for iron curtained units at destination - they kill the teleporter
	for (ObjectClass* pObj = pDestCell->FirstObject; pObj; pObj = pObj->NextObject)
	{
		if (pObj->IsIronCurtained())
		{
			int damage = pType->Strength;
			pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	// Check if destination is water and unit is naval
	bool isNavalOnLand = false;
	if (pType->Naval)
	{
		if (pDestCell->ContainsBridge() || pDestCell->LandType == LandType::Water)
			isNavalOnLand = true;
	}

	// Determine if unit can float/hover on water
	// [Improvement] Integrated from Hooks.Unit.cpp 0x718871
	bool canFloat = CanFloatOnWater(pType, pLinkedTo);

	CellStruct destCellStruct = CellClass::Coord2Cell(destCoord);
	CellClass* pDestCellByStruct = MapClass::Instance->GetCellAt(destCellStruct);

	// Check if destination is water
	bool destIsWater = (pDestCellByStruct->LandType == LandType::Water);

	if (destIsWater && !canFloat && !pType->Naval
		&& pLinkedTo->WhatAmI() != AbstractType::Infantry
		&& !pDestCell->ContainsBridge()
		&& pDestCellByStruct->LandType != LandType::Water) // double check via cell struct
	{
		// Fix: re-check properly. The original logic compares different cell lookups
	}

	// Main landing logic
	if (destIsWater && !canFloat && !pType->Naval
		&& pLinkedTo->WhatAmI() != AbstractType::Infantry
		&& !pDestCell->ContainsBridge())
	{
		// Unit lands on water it cannot traverse -> sink it
		SinkUnit(pLinkedTo);

		// Credit kills
		if (pLinkedTo->LocomotorSource)
		{
			// Killed by the magnetron/chrono source
			pLinkedTo->RegisterDestruction(pLinkedTo->LocomotorSource);
		}
		else if (pLinkedTo->MindControlledByHouse)
		{
			// Killed by chrono house
			pLinkedTo->RegisterDestruction(static_cast<TechnoClass*>(nullptr));
		}
	}
	else
	{
		// Unit can exist at destination - check if it can enter the cell
		CellClass* pEnterCell = MapClass::Instance->GetCellAt(destCellStruct);
		auto canEnter = pLinkedTo->IsCellOccupied(pEnterCell, FacingType::None, -1, nullptr, true);

		if (canEnter != Move::OK || isNavalOnLand)
		{
			// Cannot enter cell
			// [Improvement] Integrated from Hooks.Unit.cpp 0x7188F2 - sink JumpJets on water
			bool cellIsWet = pEnterCell->Tile_Is_Wet();

			if (cellIsWet && pLinkedTo->WhatAmI() != AbstractType::Infantry)
			{
				// Check if this is a deactivated unit or a JumpJet hover
				bool shouldSink = false;
				if (auto pUnit = cast_to<UnitClass*>(pLinkedTo))
				{
					if (pUnit->Deactivated || TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
						shouldSink = true;

					if (pUnit->Type->SpeedType == SpeedType::Hover && pUnit->Type->JumpJet)
						shouldSink = true;
				}

				if (shouldSink || !canFloat)
				{
					// Sink the unit
					SinkUnit(pLinkedTo);
				}
			}
			else
			{
				// Not wet or is infantry -> take lethal damage
				int damage = pType->Strength;
				pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
		}
	}
}

// =====================================================
// 0x718B70 - ComputeDestination
// Validates and computes the destination for teleportation.
// Handles infantry, buildings, pathfinding, and bridge logic.
// [Improvement] Integrates hooks: 0x718F1E/0x7190B0 (MovementZone replacement)
// =====================================================

bool __fastcall FakeTeleportLocomotionClass::Hook_ComputeDestination(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	int coordX, int coordY, int coordZ)
{
	if (!pThis || !pThis->LinkedTo)
		return false;

	FootClass* pLinkedTo = pThis->LinkedTo;
	CoordStruct destCoord { coordX, coordY, coordZ };
	CoordStruct* pLastCoords = &pThis->LastCoords;

	Debug::Log("[TeleportLoco] ComputeDestination(0x718B70): Dest(%d,%d,%d) Unit=%s\n",
		coordX, coordY, coordZ, pLinkedTo->GetTechnoType()->get_ID());

	// Clear old occupation
	if (IsDefaultCoords(*pLastCoords))
	{
		CoordStruct curCoord = pLinkedTo->Location;
		ClearOccupyBit(pLinkedTo, curCoord);
	}
	else
	{
		ClearOccupyBit(pLinkedTo, *pLastCoords);
	}

	// Check if destination is default (no destination)
	if (IsDefaultCoords(destCoord))
	{
		*pLastCoords = destCoord;
		// Fall through to set occupy at end
	}
	else
	{
		// Check for bridge height
		CellClass* pDestCell = MapClass::Instance->GetCellAt(destCoord);
		bool isAboveBridge = false;

		if (pDestCell->ContainsBridge())
		{
			CoordStruct curCoord = pLinkedTo->Location;
			int bridgeThreshold = 3 * CellClass::BridgeHeight + MapClass::Instance->GetCellFloorHeight(destCoord);
			if (curCoord.Z > bridgeThreshold)
				isAboveBridge = true;
		}

		// Check if linked unit is infantry (RTTI check)
		if (pLinkedTo->WhatAmI() == AbstractType::Infantry)
		{
			// Infantry pathfinding with special handling for missions
			bool enterTarget = false;

			// Check if the linked unit should commence its mission
			if (pLinkedTo->GetCurrentMission() == Mission::Guard || pLinkedTo->GetCurrentMission() == Mission::Area_Guard)
			{
				// Ready to commence
			}

			Mission currentMission = pLinkedTo->GetCurrentMission();

			// Check for capture/enter/eat/patrol missions
			if (currentMission == Mission::Capture
				|| currentMission == Mission::Eaten
				|| currentMission == Mission::Enter
				|| currentMission == Mission::Patrol)
			{
				AbstractClass* pNavTarget = pLinkedTo->Destination;

				// Check if NavCom target is at the destination
				if (pNavTarget)
				{
					AbstractType navType = pNavTarget->WhatAmI();

					if (navType == AbstractType::Unit || navType == AbstractType::Aircraft)
					{
						CellStruct navCell = CellClass::Coord2Cell(pNavTarget->GetCoords());
						CellStruct destCellStruct = CellClass::Coord2Cell(destCoord);
						if (navCell == destCellStruct)
							enterTarget = true;
					}

					if (navType == AbstractType::Building)
					{
						CellClass* pTargetCell = MapClass::Instance->GetCellAt(destCoord);
						if (pTargetCell->GetBuilding() == static_cast<BuildingClass*>(pNavTarget))
							enterTarget = true;
					}
				}
			}

			// [Improvement] Better infantry sub-location finding
			// Find closest free infantry spot at destination
			CoordStruct freeSpot = MapClass::PickInfantrySublocation(destCoord, enterTarget);
			pLastCoords->X = freeSpot.X;
			pLastCoords->Y = freeSpot.Y;
			pLastCoords->Z = freeSpot.Z;

			// Check if the unit can enter that cell
			CellStruct newCellStruct = CellClass::Coord2Cell(*pLastCoords);
			CellClass* pNewCell = MapClass::Instance->GetCellAt(newCellStruct);

			auto canEnter = pLinkedTo->IsCellOccupied(pNewCell, FacingType::None, -1, nullptr, false);
			if (canEnter != Move::OK)
			{
				// Can't enter - reset to default
				*pLastCoords = DefaultCoords;
			}

			// Try finding alternate from navcom target
			if (IsDefaultCoords(*pLastCoords))
			{
				AbstractClass* pNavTarget = pLinkedTo->Destination;
				if (pNavTarget && (pNavTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
				{
					MovementZone mzone = pLinkedTo->GetTechnoType()->MovementZone;

					CoordStruct targetCoord = pNavTarget->GetCoords();
					CellStruct targetCell = CellClass::Coord2Cell(targetCoord);
					CellClass* pTargetCell = MapClass::Instance->GetCellAt(targetCell);

					CoordStruct unitCoord = pLinkedTo->GetCoords();
					CellClass* pUnitCell = MapClass::Instance->GetCellAt(unitCoord);
					bool unitOnBridge = pLinkedTo->IsOnBridge();

					CellStruct destCellStruct2 = CellClass::Coord2Cell(destCoord);
					CellClass* pDestCell2 = MapClass::Instance->GetCellAt(destCellStruct2);
					bool destHasBridge = pDestCell2->ContainsBridge();

					ZoneType zone = MapClass::Instance->GetMovementZoneType(
						pUnitCell->MapCoords, mzone, unitOnBridge);

					CellStruct closeTo = CellStruct::Empty;
					CellStruct resultCell;

					MapClass::Instance->NearByLocation(
						resultCell, targetCell,
						pLinkedTo->GetTechnoType()->SpeedType,
						zone, mzone, destHasBridge,
						1, 1, false, true, false, true,
						closeTo, false, false
					);

					if (resultCell.IsValid())
					{
						CellClass* pResultCell = MapClass::Instance->GetCellAt(resultCell);
						CoordStruct resultCenter = pResultCell->GetCoordsWithBridge();
						CoordStruct spot = MapClass::PickInfantrySublocation(resultCenter, enterTarget);
						pLastCoords->X = spot.X;
						pLastCoords->Y = spot.Y;
						pLastCoords->Z = spot.Z;
					}
				}
			}
		}
		else
		{
			// Non-infantry unit
			if (pLinkedTo->IsImmobilized)
			{
				// Immobilized unit - just set occupy at current LastCoords
				SetOccupyBit(pLinkedTo, *pLastCoords);
				return true; // early return - no destination change
			}

			// [Improvement] Integrated from Hooks.Transport.cpp 0x718F1E - proper movement zone
			MovementZone mzone = AdjustMovementZone(pLinkedTo->GetTechnoType()->MovementZone);

			CellStruct destCellStruct = CellClass::Coord2Cell(destCoord);

			if (destCellStruct.IsValid())
			{
				CellClass* pDestCellCheck = MapClass::Instance->GetCellAt(destCellStruct);
				auto canEnter = pLinkedTo->IsCellOccupied(pDestCellCheck, FacingType::None, -1, nullptr, true);

				if (canEnter != Move::OK)
				{
					// Find alternate location
					ZoneType zone = MapClass::Instance->GetMovementZoneType(
						destCellStruct, mzone, isAboveBridge);

					CellStruct closeTo = CellStruct::Empty;
					CellStruct resultCell;

					MapClass::Instance->NearByLocation(
						resultCell, destCellStruct,
						pLinkedTo->GetTechnoType()->SpeedType,
						zone, mzone, isAboveBridge,
						1, 1, false, false, false, true,
						closeTo, false, false
					);

					destCellStruct = resultCell;
				}
			}

			if (destCellStruct.IsValid())
			{
				ClearOccupyBit(pLinkedTo, *pLastCoords);

				CellClass* pFinalCell = MapClass::Instance->GetCellAt(destCellStruct);
				CoordStruct center = pFinalCell->GetCoordsWithBridge();

				// Snap to cell center
				CellStruct centerCell = CellClass::Coord2Cell(center);
				pLastCoords->X = (centerCell.X * 256) + 128;
				pLastCoords->Y = (centerCell.Y * 256) + 128;
				pLastCoords->Z = 0;

				// Get proper ground height
				pLastCoords->Z = MapClass::Instance->GetCellFloorHeight(*pLastCoords);

				SetOccupyBit(pLinkedTo, *pLastCoords);
			}
		}
	}

	// Final occupation bit management
	if (IsDefaultCoords(*pLastCoords))
	{
		// No valid destination found - restore occupation at current position
		CoordStruct curCoord = pLinkedTo->Location;
		SetOccupyBit(pLinkedTo, curCoord);
		return false;
	}
	else
	{
		SetOccupyBit(pLinkedTo, *pLastCoords);
		return true;
	}
}

// =====================================================
// 0x718100 - Move_To
// ILocomotion::Move_To implementation (__stdcall via ILocomotion vtable).
// Validates the move request and initiates teleportation.
// [Improvement] Integrates hook: 0x71810D (deactivated check)
// Note: pILocoThis is the ILocomotion subobject pointer (base + 4).
// We adjust by -4 to get the TeleportLocomotionClass base.
// =====================================================

// Reference implementation - NOT actively hooked.
// Move_To is an ILocomotion __stdcall function. The existing ASMJIT_PATCH
// at 0x71810D provides the deactivated/powered/driver-killed check.
void __fastcall FakeTeleportLocomotionClass::Hook_MoveTo(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	int coordX, int coordY, int coordZ)
{
	if (!pThis || !pThis->LinkedTo)
		return;

	FootClass* pLinkedTo = pThis->LinkedTo;
	CoordStruct destCoord { coordX, coordY, coordZ };

	Debug::Log("[TeleportLoco] MoveTo(0x718100): Dest(%d,%d,%d) Unit=%s\n",
		coordX, coordY, coordZ, pLinkedTo->GetTechnoType()->get_ID());

	// Check if unit can move
	// Original: Cant_Move || Is_Paralyzed || 4264E0 || 4264F0
	// [Improvement] Integrated from Hooks.Unit.cpp 0x71810D - also check deactivated/powered/driver-killed
	bool cantMove = false;

	if (pLinkedTo->Deactivated
		|| !pLinkedTo->Locomotor.GetInterfacePtr()->Is_Powered()
		|| TechnoExtContainer::Instance.Find(pLinkedTo)->Is_DriverKilled)
	{
		cantMove = true;
	}

	if (!cantMove)
	{
		cantMove = pLinkedTo->IsParalyzed()
			|| pLinkedTo->IsBeingWarpedOut()
			|| pLinkedTo->IsUnderEMP();
	}

	if (cantMove)
	{
		// Clear the NavCom/destination
		pLinkedTo->SetDestination(nullptr, true);
		return;
	}

	// Handle infantry incoming notification
	if (pLinkedTo->FrozenStill && pLinkedTo->WhatAmI() == AbstractType::Infantry)
	{
		CellClass* pDestCell = MapClass::Instance->GetCellAt(destCoord);
		pDestCell->ScatterContent(DefaultCoords, true, true, false);
	}

	// Compute and validate destination
	Hook_ComputeDestination(pThis, nullptr, coordX, coordY, coordZ);

	// Check if we got a valid destination
	if (IsDefaultCoords(pThis->MovingDestination))
	{
		// No valid destination computed
		pLinkedTo->SetDestination(nullptr, true);
	}
	else
	{
		// Set the destination for teleportation
		pThis->Moving = true;
		pThis->MovingDestination = pThis->LastCoords;
	}
}

// =====================================================
// 0x718230 - Stop_Moving
// =====================================================

void __fastcall FakeTeleportLocomotionClass::Hook_StopMoving(
	TeleportLocomotionClass* pThis, void* /*edx*/)
{
	if (!pThis)
		return;

	pThis->MovingDestination = DefaultCoords;
	pThis->Moving = false;
	pThis->unknown_bool_36 = false;
}

// =====================================================
// 0x7192C0 - Do_Turn
// =====================================================

bool __fastcall FakeTeleportLocomotionClass::Hook_DoTurn(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	DirStruct dir)
{
	if (!pThis || !pThis->LinkedTo)
		return false;

	return pThis->LinkedTo->PrimaryFacing.Set_Desired(dir);
}

// =====================================================
// 0x71A090 - Mark_All_Occupation_Bits
// =====================================================

void __fastcall FakeTeleportLocomotionClass::Hook_MarkAllOccupationBits(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	int mark)
{
	if (!pThis || !pThis->LinkedTo)
		return;

	// Only handle mark == 0 (clear)
	if (mark == 0)
	{
		CoordStruct dest = pThis->Destination();
		pThis->LinkedTo->UnmarkAllOccupationBits(dest);
	}
}

// =====================================================
// 0x718080 - Is_Moving
// Simple getter - original implementation is fine.
// Kept as backport reference but not hooked by default.
// =====================================================

bool __fastcall FakeTeleportLocomotionClass::Hook_IsMoving(
	TeleportLocomotionClass* pThis, void* /*edx*/)
{
	if (!pThis)
		return false;

	return pThis->Moving;
}

// =====================================================
// 0x718090 - IsStill (thiscall through main vtable)
// =====================================================

bool __fastcall FakeTeleportLocomotionClass::Hook_IsStill(
	TeleportLocomotionClass* pThis, void* /*edx*/)
{
	if (!pThis)
		return true;

	return !pThis->Moving;
}

// =====================================================
// 0x7180A0 - Destination
// Simple getter - original implementation is fine.
// Kept as backport reference but not hooked by default.
// =====================================================

CoordStruct* __fastcall FakeTeleportLocomotionClass::Hook_Destination(
	TeleportLocomotionClass* pThis, void* /*edx*/,
	CoordStruct* pOutBuffer)
{
	if (!pThis || !pOutBuffer)
		return pOutBuffer;

	if (pThis->Moving)
	{
		*pOutBuffer = pThis->MovingDestination;
	}
	else
	{
		if (pThis->LinkedTo)
			*pOutBuffer = pThis->LinkedTo->Location;
		else
			*pOutBuffer = DefaultCoords;
	}

	return pOutBuffer;
}

// =====================================================
// 0x719E20 - In_Which_Layer
// =====================================================

Layer __fastcall FakeTeleportLocomotionClass::Hook_InWhichLayer(
	TeleportLocomotionClass* /*pThis*/, void* /*edx*/)
{
	return Layer::Ground;
}

// =====================================================
// 0x719C60 - GetClassID
// =====================================================

HRESULT __fastcall FakeTeleportLocomotionClass::Hook_GetClassID(
	TeleportLocomotionClass* /*pThis*/, void* /*edx*/,
	CLSID* pClassID)
{
	if (!pClassID)
		return E_POINTER;

	// {4A582747-9839-11d1-B709-00A024DDAFD1}
	*pClassID = TeleportLocomotionClass::ClassGUID();
	return S_OK;
}

// =====================================================
// 0x7192F0 - Process
// The main teleportation state machine.
// State 0: Idle/ready to teleport
// State 1: Waiting (warped by external chrono)
// State 2: Warp out animation + sound
// State 3: Travel (retry destination)
// State 4: Place at destination
// State 5: Warp in + finalize
// State 6: Waiting callback
// State 7: Cleanup
//
// [Improvement] Integrates hooks from Hooks.Teleport.cpp:
//   - 0x7193F6 (WarpOut anim with custom types)
//   - 0x719742 (WarpIn anim with custom types)
//   - 0x719827 (WarpAway anim)
//   - 0x71997B (ChronoDelay)
//   - 0x7197DF (ChronospherePreDelay)
//   - 0x719BD9 (ChronosphereDelay2)
// [Improvement] Integrates hook: 0x7196BB (MarkDown fix for transports)
// =====================================================

bool __fastcall FakeTeleportLocomotionClass::Hook_Process(
	TeleportLocomotionClass* pThis, void* /*edx*/)
{
	if (!pThis || !pThis->Owner)
		return false;

	FootClass* pLinkedTo = pThis->Owner;
	int state = pThis->State;

	// If WarpingOut is already happening and we're in state 0 with no external trigger
	if (pLinkedTo->WarpingOut && state == 0 && !pLinkedTo->unknown_280)
	{
		// Jump to state 6 (waiting callback)
		pThis->Piggybackee->Process();
		return false;
	}

	// External warp trigger
	if (state == 0 && pLinkedTo->unknown_280)
	{
		pThis->State = pLinkedTo->unknown_280;
		return false;
	}

	// Check immobilized state
	if (pLinkedTo->IsImmobilized)
	{
		if (state == 0)
		{
			// Begin warp-out for immobilized unit
			pLinkedTo->BeingWarpedOut = true;
			pThis->Timer.Start(60);

			// [Improvement] Integrated from Hooks.Teleport.cpp 0x7197DF - per-type pre-delay
			auto pTypeExtData = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());
			if (pTypeExtData)
			{
				int preDelay = pTypeExtData->ChronoSpherePreDelay.Get(
					RulesExtData::Instance()->ChronoSpherePreDelay);
				pThis->Timer.Start(preDelay);
			}

			pThis->State = 1;
			return false;
		}
	}
	else if (state <= 0)
	{
		// State 0: Check if we should begin teleporting
		if (!pThis->Dirty)
			return false;

		FootClass* pOwner = pThis->LinkedTo;

		// Check if we've arrived at destination or destination is default
		if ((pOwner->Location.X == pThis->MovingDestination.X
			&& pOwner->Location.Y == pThis->MovingDestination.Y
			&& pOwner->Location.Z == pThis->MovingDestination.Z)
			|| IsDefaultCoords(pThis->MovingDestination))
		{
			// Already at destination or no destination
			pOwner->SetDestination(nullptr, true);
			pThis->Clear_Coords();
			return false;
		}

		// Begin teleportation sequence
		pOwner->ClearAllTarget();

		// Retarget bullets aimed at this unit
		for (int i = BulletClass::Array->Count - 1; i >= 0; --i)
		{
			BulletClass* pBullet = BulletClass::Array->Items[i];
			if (pBullet && pBullet->Target == pLinkedTo)
			{
				pBullet->LoseTarget();
			}
		}

		// Play warp-out animation
		// [Improvement] Integrated from Hooks.Teleport.cpp 0x7193F6
		auto pTechnoType = pOwner->GetTechnoType();
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

		TechnoExtData::PlayAnim(
			pTypeExt->WarpOut.GetOrDefault(pOwner, RulesClass::Instance->WarpOut),
			pOwner);

		// Fire WarpOut weapon if configured
		if (const auto pWeapon = pTypeExt->WarpOutWeapon.Get(pOwner))
			WeaponTypeExtData::DetonateAt1(pWeapon, pOwner, pOwner, true, nullptr);

		// Calculate distance for chrono delay
		CoordStruct ownerCenter = pOwner->GetCoords();
		int dx = ownerCenter.X - pThis->MovingDestination.X;
		int dy = ownerCenter.Y - pThis->MovingDestination.Y;
		int dz = ownerCenter.Z - pThis->MovingDestination.Z;
		int distance = (int)Math::sqrt(
			(double)dx * dx + (double)dy * dy + (double)dz * dz);

		auto pTechExt = TechnoExtContainer::Instance.Find(pOwner);
		pTechExt->LastWarpDistance = distance;

		// Calculate timer duration
		// [Improvement] Integrated from Hooks.Teleport.cpp 0x7193F6
		int duree = pTypeExt->ChronoMinimumDelay.GetOrDefault(pOwner, RulesClass::Instance->ChronoMinimumDelay);
		const auto factor = pTypeExt->ChronoRangeMinimum.GetOrDefault(pOwner, RulesClass::Instance->ChronoRangeMinimum);

		if (distance >= factor
			&& pTypeExt->ChronoTrigger.GetOrDefault(pOwner, RulesClass::Instance->ChronoTrigger))
		{
			const auto f_factor = pTypeExt->ChronoDistanceFactor.GetOrDefault(pOwner, RulesClass::Instance->ChronoDistanceFactor);
			duree = MaxImpl(distance / MaxImpl(f_factor, 1), duree);
		}

		pThis->Timer.Start(duree);
		pOwner->WarpingOut = true;

		// Special case: harvesters/weeders teleport instantly
		if (auto pUnit = cast_to<UnitClass*, false>(pOwner))
		{
			if (pUnit->Type->Harvester || pUnit->Type->Weeder)
			{
				pThis->Timer.Start(0);
				pUnit->WarpingOut = false;
			}
		}

		pTechExt->LastWarpInDelay = std::max(pThis->Timer.GetTimeLeft(), pTechExt->LastWarpInDelay);

		// Remove parasite if present
		if (auto pParasiteEating = pOwner->ParasiteEatingMe)
		{
			if (pParasiteEating->ParasiteImUsing)
				pParasiteEating->ParasiteImUsing->ExitUnit();
		}

		// Mark up (remove from map display temporarily)
		pOwner->Mark(MarkType::Up);

		// Play chrono-out sound
		// [Improvement] Using per-type sounds
		int chronoOutSound = pTechnoType->ChronoOutSound;
		if (chronoOutSound == -1)
			chronoOutSound = RulesClass::Instance->ChronoOutSound;
		if (chronoOutSound != -1)
			VocClass::PlayIndexAtPos(chronoOutSound, pOwner->Location, false);

		// Move unit to destination coordinates
		pOwner->SetLocation(pThis->MovingDestination);

		// Update bridge status at destination
		CellStruct destCell = CellClass::Coord2Cell(pThis->MovingDestination);
		CellClass* pDestCell = MapClass::Instance->GetCellAt(destCell);

		pOwner->SetLocation(pThis->MovingDestination);
		pOwner->OnBridge = pDestCell->ContainsBridge();

		// Set height to ground level
		pOwner->SetHeight(0);

		// Mark down at new position
		// [Improvement] Integrated from Hooks.Transport.cpp 0x7196BB
		{
			auto shouldMarkDown = [pLinkedTo]()
			{
				if (pLinkedTo->GetCurrentMission() != Mission::Enter)
					return true;

				const auto pEnter = pLinkedTo->GetNthLink();
				return (!pEnter || pEnter->GetTechnoType()->Passengers <= 0);
			};

			if (shouldMarkDown())
				pOwner->Mark(MarkType::Down);
		}

		// Play chrono-in sound
		// [Improvement] Integrated from Hooks.Teleport.cpp 0x719742
		TechnoExtData::PlayAnim(
			pTypeExt->WarpIn.GetOrDefault(pOwner, RulesClass::Instance->WarpOut),
			pOwner);

		// Fire WarpIn weapon if configured
		{
			const auto pTechnoExt2 = TechnoExtContainer::Instance.Find(pOwner);
			const auto Rank = pOwner->CurrentRanking;
			const auto pWarpInWeapon = pTypeExt->WarpInWeapon.GetFromSpecificRank(Rank);

			const auto pWeapon = pTechnoExt2->LastWarpDistance < pTypeExt->ChronoRangeMinimum.GetOrDefault(pOwner, RulesClass::Instance->ChronoRangeMinimum)
				? pTypeExt->WarpInMinRangeWeapon.GetFromSpecificRank(Rank)->Get(pWarpInWeapon) : pWarpInWeapon;

			if (pWeapon)
			{
				const int damage = pTypeExt->WarpInWeapon_UseDistanceAsDamage.Get(pOwner) ?
					(pTechnoExt2->LastWarpDistance / Unsorted::LeptonsPerCell) : pWeapon->Damage;
				WeaponTypeExtData::DetonateAt2(pWeapon, pOwner, pOwner, damage, true, nullptr);
			}
		}

		int chronoInSound = pTechnoType->ChronoInSound;
		if (chronoInSound == -1)
			chronoInSound = RulesClass::Instance->ChronoInSound;
		if (chronoInSound != -1)
			VocClass::PlayIndexAtPos(chronoInSound, pOwner->Location, false);

		// Process cell entry
		pOwner->UpdatePosition(PCPType::End);
		pThis->Clear_Coords();

		// Goodie check
		pDestCell->CollectCrate(pOwner);

		// Clear destination
		pOwner->SetDestination(nullptr, true);

		// Play warp-away animation
		// [Improvement] Integrated from Hooks.Teleport.cpp 0x719827
		TechnoExtData::PlayAnim(
			pTypeExt->WarpAway.GetOrDefault(pOwner, RulesClass::Instance->WarpOut),
			pOwner);

		pOwner->unknown_280 = 0;

		return false;
	}

	// Handle other states (chrono-warp states)
	switch (state)
	{
	case 1:
		// State 1: Waiting - delegate to piggyback
		pThis->Piggybackee->Process();
		return false;

	case 2:
	{
		// State 2: Warp out animation + sound
		auto pTechnoType = pLinkedTo->GetTechnoType();
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

		TechnoExtData::PlayAnim(
			pTypeExt->WarpOut.GetOrDefault(pLinkedTo, RulesClass::Instance->WarpOut),
			pLinkedTo);

		pLinkedTo->Mark(MarkType::Up);

		int chronoOutSound = pTechnoType->ChronoOutSound;
		if (chronoOutSound == -1)
			chronoOutSound = RulesClass::Instance->ChronoOutSound;
		if (chronoOutSound != -1)
			VocClass::PlayIndexAtPos(chronoOutSound, pLinkedTo->Location, false);

		pLinkedTo->WarpingOut = true;
		pLinkedTo->IsImmobilized = false;
		pLinkedTo->BeingWarpedOut = false;
		pLinkedTo->OnBridge = false;

		// Try to mark at destination (collision detection)
		bool result = Hook_InternalMark(pThis, nullptr,
			pLinkedTo->ChronoDestCoords.X,
			pLinkedTo->ChronoDestCoords.Y,
			pLinkedTo->ChronoDestCoords.Z, 0);

		pThis->State++;
		if (result)
			pThis->State++; // Skip state 3 if mark succeeded

		return false;
	}

	case 3:
	{
		// State 3: Retry destination mark (collision detection)
		bool result = Hook_InternalMark(pThis, nullptr,
			pLinkedTo->ChronoDestCoords.X,
			pLinkedTo->ChronoDestCoords.Y,
			pLinkedTo->ChronoDestCoords.Z, 0);

		if (result)
			pThis->State++;

		// [Improvement] Integrated from Hooks.Teleport.cpp 0x71997B
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());
		pLinkedTo->ChronoLockRemaining = pTypeExt->ChronoDelay.GetOrDefault(pLinkedTo, RulesClass::Instance->ChronoDelay);

		return false;
	}

	case 4:
	{
		// State 4: Place unit at destination
		Hook_InternalMark(pThis, nullptr,
			pLinkedTo->ChronoDestCoords.X,
			pLinkedTo->ChronoDestCoords.Y,
			pLinkedTo->ChronoDestCoords.Z, 1);

		pLinkedTo->SetLocation(pThis->MovingDestination);
		pLinkedTo->SetHeight(0);
		pLinkedTo->Mark(MarkType::Down);

		pThis->State++;
		return false;
	}

	case 5:
	{
		// State 5: Warp in + finalize
		pLinkedTo->SetLocation(pThis->MovingDestination);
		pLinkedTo->SetHeight(0);
		pLinkedTo->Mark(MarkType::Down);

		// Play chrono-in sound
		auto pTechnoType = pLinkedTo->GetTechnoType();
		int chronoInSound = pTechnoType->ChronoInSound;
		if (chronoInSound == -1)
			chronoInSound = RulesClass::Instance->ChronoInSound;
		if (chronoInSound != -1)
			VocClass::PlayIndexAtPos(chronoInSound, pLinkedTo->Location, false);

		// Check radar visibility
		CellStruct cellCoords = CellClass::Coord2Cell(pLinkedTo->Location);
		if (MapClass::Instance->IsLocationShrouded(pLinkedTo->Location))
			pLinkedTo->IsInPlayfield = false;

		// Unwarp (handle landing consequences)
		if (!pLinkedTo->unknown_280)
		{
			CoordStruct unwarpCoords = pThis->MovingDestination;
			Hook_Unwarp(pThis, nullptr, unwarpCoords.X, unwarpCoords.Y, unwarpCoords.Z);
		}

		if (pLinkedTo->IsAlive)
		{
			pLinkedTo->UpdatePosition(PCPType::End);
			pThis->Clear_Coords();

			pLinkedTo->LocomotorSource = nullptr;
			pLinkedTo->MindControlledByHouse = nullptr;
			pLinkedTo->SetArchiveTarget(nullptr);
			pLinkedTo->SetDestination(nullptr, true);

			// [Improvement] Integrated from Hooks.Teleport.cpp 0x71997B
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
			int chronoDelay = pTypeExt->ChronoDelay.GetOrDefault(pLinkedTo, RulesClass::Instance->ChronoDelay);
			pThis->Timer.Start(chronoDelay);

			// Play warp-away animation
			// [Improvement] Integrated from Hooks.Teleport.cpp 0x719827
			TechnoExtData::PlayAnim(
				pTypeExt->WarpAway.GetOrDefault(pLinkedTo, RulesClass::Instance->WarpOut),
				pLinkedTo);

			pThis->State++;
		}

		return false;
	}

	case 6:
		// State 6: Waiting - delegate to piggyback
		pThis->Piggybackee->Process();
		return false;

	case 7:
	{
		// State 7: Cleanup
		pLinkedTo->WarpingOut = false;
		pLinkedTo->SetArchiveTarget(nullptr);
		pLinkedTo->SetDestination(nullptr, true);
		pThis->Moving = false;
		pLinkedTo->unknown_280 = 0;
		pThis->State = 0;

		// [Improvement] Integrated from Hooks.Teleport.cpp 0x719BD9
		auto const pExt = TechnoExtContainer::Instance.Find(pThis->Owner);
		if (pExt->IsBeingChronoSphered)
		{
			auto pTypeExtData = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());
			int delay = pTypeExtData->ChronoSphereDelay.Get(RulesExtData::Instance()->ChronoSphereDelay);

			if (delay > 0)
			{
				pThis->Owner->WarpingOut = true;
				pExt->HasRemainingWarpInDelay = true;
				pExt->LastWarpInDelay = std::max(delay, pExt->LastWarpInDelay);
			}
			else
			{
				pExt->IsBeingChronoSphered = false;
			}
		}

		return false;
	}

	default:
		return false;
	}
}

// =====================================================
// 0x719BF0 - ProcessTimerCompletion
// Called after the chrono timer expires to handle
// post-teleport idle behavior.
// =====================================================

void __stdcall FakeTeleportLocomotionClass::Hook_ProcessTimerCompletion(
	TeleportLocomotionClass* pThis)
{
	if (!pThis || !pThis->LinkedTo)
		return;

	FootClass* pLinkedTo = pThis->LinkedTo;

	// Check if timer has expired
	int timeLeft = pThis->Timer.GetTimeLeft();
	if (timeLeft > 0)
		return;

	// Timer expired - unit is no longer warping
	pLinkedTo->WarpingOut = false;

	// If no target, enter idle mode
	if (!pLinkedTo->Target)
	{
		// Try to find a target
		pLinkedTo->TargetAndEstimateDamage(nullptr, ThreatType::Normal);

		if (!pLinkedTo->Target)
		{
			pLinkedTo->EnterIdleMode(false, true);
		}
	}

	// Increment state if in chrono-warp mode
	if (pThis->State > 0)
		pThis->State++;
}

// =====================================================
// DEFINE_FUNCTION_JUMP hooks
// These replace the original game functions with our implementations.
//
// Address ranges and what they replace:
// - 0x718260: InternalMark (makes 0x718275, 0x7184CE, 0x7185DA, 0x71872C redundant)
// - 0x7187A0: Unwarp (makes 0x7187DA, 0x7188F2, 0x718871 redundant)
// - 0x718B70: ComputeDestination (makes 0x718F1E, 0x7190B0 redundant)
// - 0x718100: Move_To (makes 0x71810D redundant)
// - 0x718230: Stop_Moving
// - 0x7192C0: Do_Turn
// - 0x71A090: Mark_All_Occupation_Bits
// - 0x718080: Is_Moving
// - 0x718090: IsStill
// - 0x7180A0: Destination
// - 0x719E20: In_Which_Layer
// - 0x719C60: GetClassID
//
// Note: The following hooks remain ACTIVE and are NOT conflicting:
// - 0x719CBC (LJMP in Hooks.BugFixes.cpp) - Load fix, separate from our hooks
// - 0x719F17 (Hooks.BugFixes.cpp) - End_Piggyback PowerOn, shared with other locos
//
// The following hooks in Hooks.Teleport.cpp operate WITHIN Process() at specific
// instruction offsets. Since we replace the entire Process function, those hook
// addresses no longer exist in executing code. They are harmless dead hooks:
// - 0x7193F6, 0x719742, 0x719827, 0x71997B, 0x7197DF, 0x719BD9
//
// The following hooks in Hooks.Unit.cpp operate WITHIN functions we replace.
// They become dead hooks when our LJMP is active:
// - 0x71810D (inside Move_To), 0x7187DA, 0x7188F2, 0x718871 (inside Unwarp)
//
// The following hooks in Hooks.Transport.cpp operate WITHIN functions we replace:
// - 0x7196BB (inside Process), 0x718F1E, 0x7190B0 (inside ComputeDestination)
//
// The following hooks in Hooks.BugFixes.cpp operate WITHIN functions we replace:
// - 0x71872C (inside InternalMark)
// =====================================================

// Active hooks: Internal __thiscall functions that are safe to LJMP
// These are called internally by the game engine, NOT through the ILocomotion vtable.
DEFINE_FUNCTION_JUMP(LJMP, 0x718260, FakeTeleportLocomotionClass::Hook_InternalMark);
DEFINE_FUNCTION_JUMP(LJMP, 0x7187A0, FakeTeleportLocomotionClass::Hook_Unwarp);
DEFINE_FUNCTION_JUMP(LJMP, 0x718B70, FakeTeleportLocomotionClass::Hook_ComputeDestination);

// ILocomotion vtable functions: These use __stdcall calling convention via COM
// interface dispatch. They are NOT hooked via LJMP because:
// 1. The calling convention mismatch would corrupt the stack
// 2. The existing ASMJIT_PATCH hooks provide the necessary improvements
// 3. The original implementations are simple and work correctly
//
// To hook these, use VTABLE patches on the ILocomotion vtable entries:
//   TeleportLocomotionClass::ILoco_vtable (0x7F5000) + offset
//   e.g., DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5000 + 0x3C, MyMoveTo) for Move_To
//
// Addresses for reference (NOT hooked):
// 0x718100 - Move_To       (has active ASMJIT_PATCH at 0x71810D for deactivated check)
// 0x718230 - Stop_Moving   (original works fine)
// 0x7192C0 - Do_Turn       (original works fine)
// 0x71A090 - Mark_All_Occupation_Bits (original works fine)
// 0x718080 - Is_Moving     (original works fine)
// 0x718090 - IsStill       (original works fine)
// 0x7180A0 - Destination   (original works fine)
// 0x719E20 - In_Which_Layer (original works fine)
// 0x719C60 - GetClassID    (original works fine)
//
// Process (0x7192F0) - NOT hooked because Hooks.Teleport.cpp provides essential
// per-type customization at specific instruction offsets within the function.
// ProcessTimerCompletion (0x719BF0) - NOT hooked, works with original Process.
