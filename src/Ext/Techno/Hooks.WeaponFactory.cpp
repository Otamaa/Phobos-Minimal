#include "Body.h"

#include <Ext/BuildingType/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <FootClass.h>
#include <Locomotor/LocomotionClass.h>

CellStruct GetWeaponFactoryDoor(BuildingClass* pThis)
{
	auto cell = pThis->GetMapCoords();
	auto buffer = CoordStruct::Empty;
	pThis->GetExitCoords(&buffer, 0);
	const auto pType = pThis->Type;

	switch (BuildingTypeExtContainer::Instance.Find(pType)->WeaponsFactory_Dir.Get())
	{

	case BibDir::North:
	{
		cell.X = static_cast<short>(buffer.X / Unsorted::LeptonsPerCell);
		break;
	}

	case BibDir::East:
	{
		cell.X += static_cast<short>(pType->GetFoundationWidth() - 1);
		cell.Y = static_cast<short>(buffer.Y / Unsorted::LeptonsPerCell);
		break;
	}

	case BibDir::South:
	{
		cell.X = static_cast<short>(buffer.X / Unsorted::LeptonsPerCell);
		cell.Y += static_cast<short>(pType->GetFoundationHeight(false) - 1);
		break;
	}

	case BibDir::West:
	{
		cell.Y = static_cast<short>(buffer.Y / Unsorted::LeptonsPerCell);
		break;
	}

	default:
	{
		break;
	}

	}

	return cell;
}

ASMJIT_PATCH(0x73F5A7, UnitClass_IsCellOccupied_UnlimboDirection, 0x8)
{
	enum { NextObject = 0x73FA87, ContinueCheck = 0x73F5AF };

	GET(const bool, notPassable, EAX);

	if (notPassable)
		return ContinueCheck;

	GET(BuildingClass* const, pBuilding, ESI);

	const auto pType = pBuilding->Type;

	if (!pType->WeaponsFactory)
		return NextObject;

	GET(CellClass* const, pCell, EDI);

	auto buffer = CoordStruct::Empty;
	pBuilding->GetExitCoords(&buffer, 0);
	const auto cell = CellClass::Coord2Cell(buffer);
	const bool pathX = ((int)BuildingTypeExtContainer::Instance.Find(pType)->WeaponsFactory_Dir.Get() & 2) != 0; // 2,6/0,4
	const bool onPath = pathX ? pCell->MapCoords.Y == cell.Y : pCell->MapCoords.X == cell.X;

	return onPath ? NextObject : ContinueCheck;
}

ASMJIT_PATCH(0x44955D, BuildingClass_WeaponFactoryOutsideBusy_WeaponFactoryCell, 0x6)
{
	enum { StartCheck = 0x4495DF, NotBusy = 0x44969B };

	GET(BuildingClass* const, pThis, ESI);

	const auto pLink = pThis->GetNthLink();

	if (!pLink)
		return NotBusy;

	const auto pLinkType = pLink->GetTechnoType();

	if (pLinkType->JumpJet && pLinkType->BalloonHover)
		return NotBusy;

	REF_STACK(CoordStruct, coords, STACK_OFFSET(0x30, -0xC));

	const auto cell = GetWeaponFactoryDoor(pThis);
	coords = CellClass::Cell2Coord(cell);

	R->EAX(MapClass::Instance->GetCellAt(cell));

	return StartCheck;
}

DEFINE_JUMP(LJMP, 0x44DCC7, 0x44DD3C);

ASMJIT_PATCH(0x44E131, BuildingClass_Mission_Unload_WeaponFactoryFix1, 0x5)
{
	enum { SkipGameCode = 0x44E191 };

	GET(BuildingClass* const, pThis, EBP);
	GET(FootClass* const, pLink, EDI);
	//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	const auto cell = GetWeaponFactoryDoor(pThis);
	const auto coords = CellClass::Cell2Coord(cell);

	pLink->SetDestination(MapClass::Instance->GetCellAt(cell), true);
	return SkipGameCode;
}

ASMJIT_PATCH(0x44DF72, BuildingClass_Mission_Unload_WeaponFactoryFix2, 0x5)
{
	enum { SkipGameCode = 0x44E1AD };

	GET(BuildingClass* const, pThis, EBP);
	GET_STACK(FootClass* const, pLink, STACK_OFFSET(0x50, -0x30));
	//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	const auto cell = GetWeaponFactoryDoor(pThis);
	const auto coords = CellClass::Cell2Coord(cell);

	pLink->SetDestination(MapClass::Instance->GetCellAt(cell), true);
	R->EDI(pLink);

	return SkipGameCode;
}

ASMJIT_PATCH(0x44DF1C, BuildingClass_Mission_Unload_WeaponFactoryFix3, 0x7)
{
	enum { SkipGameCode = 0x44DF47 };

	GET(BuildingClass* const, pThis, EBP);
	GET_STACK(FootClass* const, pLink, STACK_OFFSET(0x50, -0x30));
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x50, -0x34));
	//	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x50, -0x1C));

	cell = GetWeaponFactoryDoor(pThis);

	R->ESI(pLink);

	return SkipGameCode;
}

ASMJIT_PATCH(0x742D98, UnitClass_SetDestination_WeaponFactoryCell, 0x6)
{
	enum { SkipGameCode = 0x742DFB };

	GET(BuildingClass* const, pLink, ESI);

	const auto cell = GetWeaponFactoryDoor(pLink);

	R->EAX(MapClass::Instance->GetCellAt(cell));

	return SkipGameCode;
}

ASMJIT_PATCH(0x516D3C, HoverLocomotionClass_IsIonSensitive_WeaponFactoryCell, 0x5)
{
	enum { Right = 0x516DFF, IsNot = 0x516DF6 };

	GET(BuildingClass* const, pBuilding, EAX);
	GET(ILocomotion* const, iLoco, ESI);

	const auto location = CellClass::Coord2Cell(static_cast<LocomotionClass*>(iLoco)->LinkedTo->Location);
	bool notIon = false;
	auto buffer = CoordStruct::Empty;
	pBuilding->GetExitCoords(&buffer, 0);
	const auto cell = CellClass::Coord2Cell(buffer);
	const auto pType = pBuilding->Type;

	switch (BuildingTypeExtContainer::Instance.Find(pType)->WeaponsFactory_Dir.Get())
	{

		case BibDir::North:
	{
		notIon |= (cell.X == location.X
			&& cell.Y != location.Y
			&& (pBuilding->Location.Y / Unsorted::LeptonsPerCell) != location.Y);

		break;
	}

	case BibDir::East:
	{
		notIon |= (cell.Y == location.Y
			&& cell.X != location.X
			&& (pBuilding->Location.X / Unsorted::LeptonsPerCell + pType->GetFoundationWidth() - 1) != location.X);

		break;
	}

	case BibDir::South:
	{
		notIon |= (cell.X == location.X
			&& cell.Y != location.Y
			&& (pBuilding->Location.Y / Unsorted::LeptonsPerCell + pType->GetFoundationHeight(false) - 1) != location.Y);

		break;
	}

	case BibDir::West:
	{
		notIon |= (cell.Y == location.Y
			&& cell.X != location.X
			&& (pBuilding->Location.X / Unsorted::LeptonsPerCell) != location.X);

		break;
	}

	default:
	{
		break;
	}

	}

	return notIon ? IsNot : Right;
}

DEFINE_JUMP(LJMP, 0x7443D9, 0x744463)

ASMJIT_PATCH(0x458A00, BuildingClass_IsCellNotPassable_ImpassableRowsDirection, 0x6)
{
	enum { SkipGameCode = 0x458A76 };

	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(CellClass* const, pCell, STACK_OFFSET(0x0, 0x4));

	auto isCellNotPassable = [pThis, pCell]() -> bool
		{
			if (pCell->GetBuilding() != pThis)
				return false;

			const auto pType = pThis->Type;

			if (pType->NumberImpassableRows == -1)
				return true;

			if (pType->Bunker && pThis->BunkerLinkedItem)
				return true;

			switch (BuildingTypeExtContainer::Instance.Find(pType)->NumberImpassableRows_Dir.Get())
			{

				case BibDir::North:
			{
				const int y = pThis->Location.Y / Unsorted::LeptonsPerCell;
				const int maxPassableY = y + pType->GetFoundationHeight(false) - 1 - pType->NumberImpassableRows;
				return pCell->MapCoords.Y > maxPassableY;
			}

			case BibDir::East:
			{
				const int x = pThis->Location.X / Unsorted::LeptonsPerCell;
				const int minPassableX = x + pType->NumberImpassableRows;
				return pCell->MapCoords.X < minPassableX;
			}

			case BibDir::South:
			{
				const int y = pThis->Location.Y / Unsorted::LeptonsPerCell;
				const int minPassableY = y + pType->NumberImpassableRows;
				return pCell->MapCoords.Y < minPassableY;
			}

			case BibDir::West:
			{
				const int x = pThis->Location.X / Unsorted::LeptonsPerCell;
				const int maxPassableX = x + pType->GetFoundationWidth() - 1 - pType->NumberImpassableRows;
				return pCell->MapCoords.X > maxPassableX;
			}

			default:
			{
				return true;
			}

			}
		};

	R->EAX(isCellNotPassable());

	return SkipGameCode;
}

// The input parameter of GetFoundationHeight is incorrect, and there is no call to input the incorrect parameter, so no need to consider it
ASMJIT_PATCH(0x73F7DD, BuildingClass_IsCellNotPassable_BibDirection, 0x8)
{
	GET(CellClass* const, pCell, EDI);
	GET(BuildingTypeClass* const, pType, EAX);
	CellStruct adj = CellSpread::AdjacentCell[(int)BuildingTypeExtContainer::Instance.Find(pType)->Bib_Dir.Get()];

	R->EAX(MapClass::Instance->GetCellAt(adj + pCell->MapCoords));

	return 0x73F814;
}

static void KickOutStuckUnits(BuildingClass* pThis)
{
	auto buffer = CoordStruct::Empty;
	pThis->GetExitCoords(&buffer, 0);

	auto cell = CellClass::Coord2Cell(buffer);

	bool upward = false;
	short* pCur = nullptr;
	short start = 0; // door

	const auto pType = pThis->Type;

	switch (BuildingTypeExtContainer::Instance.Find(pType)->WeaponsFactory_Dir.Get())
	{

	case BibDir::North: // North -> left+down/++Y
	{
		upward = false;
		pCur = &cell.Y;
		start = static_cast<short>(pThis->Location.Y / Unsorted::LeptonsPerCell + 1);
		break;
	}

	case BibDir::East: // East -> left+up/--X
	{
		upward = true;
		pCur = &cell.X;
		start = static_cast<short>(pThis->Location.X / Unsorted::LeptonsPerCell + pType->GetFoundationWidth() - 2);
		break;
	}

	case BibDir::South: // South -> right+up/--Y
	{
		upward = true;
		pCur = &cell.Y;
		start = static_cast<short>(pThis->Location.Y / Unsorted::LeptonsPerCell + pType->GetFoundationHeight(false) - 2);
		break;
	}

	case BibDir::West: // West -> right+down/++X
	{
		upward = false;
		pCur = &cell.X;
		start = static_cast<short>(pThis->Location.X / Unsorted::LeptonsPerCell + 1);
		break;
	}

	default: // Invalid direction
	{
		return;
	}

	}

	const short end = *pCur; // exit
	*pCur = start;
	auto pCell = MapClass::Instance->GetCellAt(cell);

	while (true)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pUnit = cast_to<UnitClass*, true>(pObject))
			{
				if (pThis->Owner != pUnit->Owner)
					continue;

				const auto pLocoDest = pUnit->Locomotor->Destination();

				if (pLocoDest != CoordStruct::Empty && pLocoDest != pUnit->Location)
					continue;

				const auto height = pUnit->GetHeight();

				if (height < 0 || height > Unsorted::CellHeight)
					continue;

				pThis->SendCommand(RadioCommand::RequestLink, pUnit);
				pThis->QueueMission(Mission::Unload, false);
				return; // one after another
			}
		}

		if (upward ? (--(*pCur) < end) : (++(*pCur) > end))
			return; // no stuck

		pCell = MapClass::Instance->GetCellAt(cell);
	}
}

// Attempt to kick the stuck unit out again by setting the destination
ASMJIT_PATCH(0x44E202, BuildingClass_Mission_Unload_CheckStuck, 0x6)
{
	enum { Waiting = 0x44E267, NextStatus = 0x44E20C };

	GET(BuildingClass*, pThis, EBP);

	if (!pThis->IsTethered)
		return NextStatus;

	if (const auto pUnit = cast_to<UnitClass*>(pThis->GetNthLink()))
	{
		// Detecting movement status
		if (pUnit->Locomotor->Destination() == CoordStruct::Empty)
		{
			// Evacuate the congestion at the entrance
			pThis->ClearFactoryBib();
			const auto cell = GetWeaponFactoryDoor(pThis);
			const auto pDest = MapClass::Instance->GetCellAt(cell);

			// Hover units may stop one cell behind their destination, should forcing them to advance one more cell
			pUnit->SetDestination((pUnit->Destination != pDest ? pDest : MapClass::Instance->GetCellAt(cell)), true);
		}
	}

	return Waiting;
}

// Check for any stuck units inside after successful unload each time. If there is, kick it out
ASMJIT_PATCH(0x44E260, BuildingClass_Mission_Unload_KickOutStuckUnits, 0x7)
{
	GET(BuildingClass*, pThis, EBP);

	KickOutStuckUnits(pThis);

	return 0;
}
