#include "Body.h"

#include <InfantryClass.h>
#include <UnitClass.h>
#include <TechnoTypeClass.h>
#include <WarheadTypeClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

/**
 * FakeTeleportLocomotionClass - Backported and improved implementation of
 * TeleportLocomotionClass internal helper function at 0x718260
 *
 * Original signature:
 *   bool __thiscall TeleportLocomotionClass_718260(
 *       TeleportLocomotionClass* this, int destX, int destY, int destZ, int mark)
 *
 * This is an INTERNAL helper function, not the public Mark_All_Occupation_Bits vtable function.
 * The public function extracts coordinates from ChronoDestCoords and calls this helper.
 *
 * This implementation addresses several issues in the original:
 * 1. Proper handling of bridge cells and height adjustments
 * 2. Integration with existing hooks at 0x718275, 0x7184CE, 0x7185DA
 * 3. Support for Chronoshift_Crushable and ChronoInfantryCrush rules
 * 4. Better collision detection and alternate location finding
 *
 * Note: The existing hooks in Hooks.Locomotor.cpp (0x718275) and
 * Bugfixes.cpp (0x7184CE, 0x7185DA) already patch parts of the original
 * function. When LJMP hook is enabled, those hooks become redundant.
 */

bool FakeTeleportLocomotionClass::IsDefaultCoords(const CoordStruct& coords)
{
	return coords.X == DefaultCoords.X
		&& coords.Y == DefaultCoords.Y
		&& coords.Z == DefaultCoords.Z;
}

void FakeTeleportLocomotionClass::ClearOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords)
{
	if (pLinkedTo)
	{
		pLinkedTo->UnmarkAllOccupationBits(coords);
	}
}

void FakeTeleportLocomotionClass::SetOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords)
{
	if (pLinkedTo)
	{
		pLinkedTo->MarkAllOccupationBits(coords);
	}
}

MovementZone FakeTeleportLocomotionClass::AdjustMovementZone(MovementZone mzone)
{
	// Convert special movement zones to passable equivalents for pathfinding
	// This matches the original game logic
	switch (mzone)
	{
	case MovementZone::Fly:
		return MovementZone::Normal;
	case MovementZone::Destroyer:
		return MovementZone::Normal;
	case MovementZone::AmphibiousDestroyer:
		return MovementZone::Amphibious;
	default:
		return mzone;
	}
}

bool FakeTeleportLocomotionClass::HandleDestinationCollisions(
	TeleportLocomotionClass* pLoco,
	const CoordStruct& destCoord,
	CellClass* pDestCell)
{
	if (!pLoco || !pLoco->LinkedTo || !pDestCell)
		return false;

	FootClass* pLinkedTo = pLoco->LinkedTo;
	bool needAlternateLocation = false;
	const bool bLinkedIsInfantry = pLinkedTo->WhatAmI() == AbstractType::Infantry;

	// Get the appropriate occupier list based on bridge status
	ObjectClass* pOccupier = pDestCell->GetContentB();

	for (ObjectClass* pObj = pOccupier; pObj; pObj = pObj->NextObject)
	{
		const bool bObjIsInfantry = pObj->WhatAmI() == AbstractType::Infantry;
		bool bIsImmune = pObj->IsIronCurtained();

		// Check for Chronoshift_Crushable from TechnoTypeExt
		TechnoTypeClass* pType = pObj->GetTechnoType();
		if (pType)
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.TryFind(pType);
			if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
			{
				bIsImmune = true;
			}
		}

		// Check ChronoInfantryCrush rule - if disabled, infantry can't crush non-infantry
		if (!RulesExtData::Instance()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry)
		{
			// Infantry teleporting onto non-infantry = self damage
			int damage = pLinkedTo->GetType()->Strength;
			pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			break;
		}

		if (!bIsImmune && bObjIsInfantry && bLinkedIsInfantry)
		{
			// Infantry vs Infantry at exact same coordinates
			// The target infantry takes damage (gets crushed by teleport)
			CoordStruct objCoord = pObj->GetCoords();
			if (objCoord.X == destCoord.X && objCoord.Y == destCoord.Y && objCoord.Z == destCoord.Z)
			{
				int damage = pObj->GetType()->Strength;
				pObj->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
		}
		else if (bIsImmune || ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		{
			if ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
			{
				// Non-foot object (like a building) is blocking
				needAlternateLocation = true;
			}
			else if (bIsImmune)
			{
				// Iron curtained or immune target - teleporting unit takes damage
				int damage = pLinkedTo->GetType()->Strength;
				pLinkedTo->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				break;
			}
		}
		else
		{
			// Regular foot unit - they take damage
			int damage = pObj->GetType()->Strength;
			pObj->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	// Check if the cell has a bridge without a body (destroyed bridge)
	// This condition: (Flags & 0x300) == 0x100 means bridge present but body destroyed
	CellFlags bridgeFlags = pDestCell->Flags & CellFlags::BridgeWithBody;
	if (bridgeFlags == CellFlags::Bridge)
	{
		// Bridge exists but body is destroyed - can't land here
		needAlternateLocation = true;
	}

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

	// Get and adjust movement zone
	MovementZone mzone = AdjustMovementZone(pType->MovementZone);

	// Get destination cell
	CellStruct destCell = CellClass::Coord2Cell(destCoord);
	CellClass* pDestCell = MapClass::Instance->GetCellAt(destCell);

	// Check if destination has a bridge
	bool destHasBridge = pDestCell->ContainsBridge();

	// Get zone from source cell for pathfinding
	ZoneType zone = MapClass::Instance->GetMovementZoneType(
		pSourceCell->MapCoords,
		mzone,
		sourceCellHasBridge);

	// Find nearby valid location
	CellStruct closeTo = CellStruct::Empty;
	CellStruct resultCell;

	MapClass::Instance->NearByLocation(
		resultCell,
		destCell,
		SpeedType::Track,  // Use track for ground-based pathfinding
		zone,
		mzone,
		destHasBridge,
		1,  // SpaceSizeX
		1,  // SpaceSizeY
		false,  // disallowOverlay
		false,  // a11
		false,  // requireBurrowable
		true,   // allowBridge
		closeTo,
		false,  // a15
		false   // buildable
	);

	// Check if we found a valid alternate location
	if (!resultCell.IsValid())
	{
		// Cannot find location - set destination to current location
		// This matches the fix in Bugfixes.cpp at 0x7185DA
		pLinkedTo->ChronoDestCoords = pLinkedTo->GetCoords();
		return true;
	}

	// Calculate offset from original destination cell center to the coord
	CoordStruct origCellCenter = pDestCell->GetCoordsWithBridge();
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
		// LastCoords not initialized - use current position
		CoordStruct currentCoord = pLinkedTo->GetCoords();
		ClearOccupyBit(pLinkedTo, currentCoord);
	}
	else
	{
		ClearOccupyBit(pLinkedTo, pLoco->LastCoords);
	}

	// Update LastCoords to new destination
	pLoco->LastCoords = pLinkedTo->ChronoDestCoords;

	// Get proper Z height for the destination
	pLoco->LastCoords.Z = MapClass::Instance->GetCellFloorHeight(pLoco->LastCoords);

	// Handle bridge adjustment
	CellClass* pDestCell = MapClass::Instance->GetCellAt(pLoco->LastCoords);

	if (!pDestCell->ContainsBridge() || pLinkedTo->OnBridge)
	{
		// No bridge at destination, or unit was already on a bridge
		pLinkedTo->OnBridge = false;
	}
	else
	{
		// There's a bridge at destination - put unit on it
		pLinkedTo->OnBridge = true;
		pLoco->LastCoords.Z += CellClass::BridgeHeight;
	}

	// Set new occupation bit
	SetOccupyBit(pLinkedTo, pLoco->LastCoords);
}

bool FakeTeleportLocomotionClass::Mark_All_Occupation_Bits(
	TeleportLocomotionClass* pLoco,
	const CoordStruct& destCoord,
	int mark)
{
	if (!pLoco || !pLoco->LinkedTo)
		return false;

	FootClass* pLinkedTo = pLoco->LinkedTo;

	if (mark != 0)
	{
		// Finalizing teleport - clear old occupation and set new
		FinalizeDestination(pLoco);
		// Fall through to set occupation bit at the end
	}
	else
	{
		// Starting teleport - check for collisions at destination
		CellClass* pDestCell = MapClass::Instance->GetCellAt(destCoord);

		bool needAlternate = HandleDestinationCollisions(pLoco, destCoord, pDestCell);

		// Check bridge state
		if (pDestCell->ContainsBridge() && !pDestCell->ContainsBridgeBody())
		{
			// Bridge exists but body is destroyed
			needAlternate = true;
		}

		if (needAlternate)
		{
			// Get source cell info
			CoordStruct sourceCoord = pLinkedTo->GetCoords();
			CellStruct sourceCell = CellClass::Coord2Cell(sourceCoord);
			CellClass* pSourceCell = MapClass::Instance->GetCellAt(sourceCell);
			bool sourceCellHasBridge = pSourceCell->ContainsBridge();

			// Find alternate location and update ChronoDestCoords
			FindAlternateLocation(pLoco, destCoord, pSourceCell, sourceCellHasBridge);

			// Return false to indicate destination was changed
			return false;
		}
	}

	// Set occupation bit at final location
	CoordStruct finalCoord;
	if (IsDefaultCoords(pLoco->LastCoords))
	{
		finalCoord = pLinkedTo->GetCoords();
	}
	else
	{
		finalCoord = pLoco->LastCoords;
	}

	SetOccupyBit(pLinkedTo, finalCoord);
	return true;
}

// Wrapper function matching the original __thiscall signature
// Original: bool __thiscall TeleportLocomotionClass_718260(TeleportLocomotionClass* this, int x, int y, int z, int mark)
bool __fastcall FakeTeleportLocomotionClass::TeleportLocomotionClass_InternalMark(
	TeleportLocomotionClass* pThis,
	void* edx_unused,  // __fastcall uses EDX for second param, but we ignore it
	int destX,
	int destY,
	int destZ,
	int mark)
{
	// This function is called INTERNALLY by TeleportLocomotionClass during:
	// 1. Process() when actively teleporting (calls with mark=0 to check collisions, then mark=1 to finalize)
	// 2. When the Chronosphere or ChronoWarp superweapon activates
	// 3. When units with Locomotor={4A582747-9839-11d1-B709-00A024DDAFD1} (Teleport) move
	
	//Debug::Log("[Phobos] TeleportLocomotion backport @ 0x718260 - Coords: (%d, %d, %d), Mark: %d, Unit: %s\n", 
	//	destX, destY, destZ, mark, 
	//	pThis && pThis->LinkedTo ? pThis->LinkedTo->GetTechnoType()->get_ID() : "NULL");
	
	CoordStruct destCoord { destX, destY, destZ };
	return Mark_All_Occupation_Bits(pThis, destCoord, mark);
}

// Hook to replace the entire original function
// This makes the existing hooks at 0x718275, 0x7184CE, 0x7185DA redundant
// Comment this out if you want to use the original function with partial hooks instead
DEFINE_FUNCTION_JUMP(LJMP, 0x718260, FakeTeleportLocomotionClass::TeleportLocomotionClass_InternalMark);
