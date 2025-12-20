#include <Ext/Scenario/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>

#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>

#include <AircraftTrackerClass.h>

namespace SelectAutoTarget_Context
{
	bool AU = false;
}

AbstractClass* __fastcall FakeTechnoClass::__Greatest_Threat(TechnoClass* pThis, discard_t, ThreatType method, CoordStruct* coord, bool onlyEnemy)
{
	++TechnoClass::TargetScanCounter();
	const auto pType = pThis->GetTechnoType();
	const auto pOwner = pThis->Owner;
	const bool isTechnoPlayerControlled = pOwner->IsControlledByHuman();

	// Early exit for NoAutoFire units under player control
	if (pType->NoAutoFire && isTechnoPlayerControlled)
	{
		return nullptr;
	}

	bool AU = SelectAutoTarget_Context::AU = (ExtendedThreatType(method) & ExtendedThreatType::Underground) != ExtendedThreatType::none;

	// Initialize variables - FIXED: bestThreat = -1, not 0!
	TechnoClass* bestTarget = nullptr;
	int bestThreat = -1;
	ZoneType zone = ZoneType::None;
	const AbstractType what = pThis->WhatAmI();
	CoordStruct emptyCoord = CoordStruct::Empty;

	// Determine zone for non-building, non-aircraft units without ThreatType::Range
	if ((method & ThreatType::Range) == ThreatType::Normal
		&& what != AbstractType::Building
		&& what != AbstractType::Aircraft)
	{
		CoordStruct centerCoord = pThis->GetCoords();
		CellStruct cellPos = CellClass::Coord2Cell(centerCoord);

		zone = MapClass::Instance->GetMovementZoneType(&cellPos, pType->MovementZone, 1);
	}

	const int combatDamage = pThis->CombatDamage(-1);
	constexpr ThreatType ThreatType_HealerTargets = ThreatType::Air | ThreatType::Infantry | ThreatType::Vehicles | ThreatType::Buildings;

	// Adjust method based on unit type and combat damage
	if (what == AbstractType::Infantry)
	{
		if (combatDamage < 0)
		{
			// Negative damage infantry - target other infantry only
			method = (method & (ThreatType::Area | ThreatType::Range)) | ThreatType::Threattype_4000 | ThreatType_HealerTargets;
		}
		else if (((InfantryClass*)pThis)->Type->Engineer)
		{
			// Engineers don't target infantry or vehicles
			method &= ~(ThreatType::Vehicles | ThreatType::Infantry);
		}
	}
	else if (what == AbstractType::Unit && combatDamage < 0)
	{
		// Negative damage units - target other vehicles only
		method = (method & (ThreatType::Area | ThreatType::Range)) | ThreatType::Threattype_4000 | ThreatType_HealerTargets;
	}

	// Build threat bitfield based on method flags
	int threatBitfield = 0;

	// FIXED: test bh, 1 = method & 0x100 = THREAT_CIVILIANS, not THREAT_VEHICLES!
	if (method & ThreatType::Civilians)
	{
		threatBitfield = 0x8042;
	}

	if (method & ThreatType::Air)
	{
		threatBitfield |= 0x4;
	}

	// FIXED: test eax, 1BA60h - exact mask from assembly
	// 0x1BA60 = TechBuildings | OccupiableBuildings | Base_defenses | Factories | PowerFacilties | Tiberium | Buildings
	// Note: Does NOT include Capture (0x200)
	if (method & (ThreatType::TechBuildings
		| ThreatType::OccupiableBuildings
		| ThreatType::Base_defenses
		| ThreatType::Factories
		| ThreatType::PowerFacilties
		| ThreatType::Tiberium
		| ThreatType::Buildings))
	{
		threatBitfield |= 0x40;
	}

	if (method & ThreatType::Infantry)
	{
		threatBitfield |= 0x8000;
	}

	if (method & (ThreatType::Tiberium | ThreatType::Vehicles))
	{
		threatBitfield |= 0x2;
	}

	// Clear distributed fire targets
	if (pType->DistributedFire)
	{
		pThis->CurrentTargets.clear();
		pThis->CurrentTargetThreatValues.clear();
	}

	if (AU)
	{
		threatBitfield |= 1 << (int)AbstractType::Infantry;
		threatBitfield |= 1 << (int)AbstractType::Unit;
		threatBitfield |= 1 << (int)AbstractType::Aircraft;
	}

	// Check if in open-topped transport with original owner
	bool hasRealOwner = false;
	if (auto transport = pThis->Transporter)
	{
		auto pTransportType = transport->GetTechnoType();

		if (pTransportType->OpenTopped && !TechnoTypeExtContainer::Instance.Find(pTransportType)->Passengers_SyncOwner)
		{
			hasRealOwner = (transport->OriginallyOwnedByHouse != nullptr);
		}
	}

	// No range or area restriction - scan all units directly
	if ((method & (ThreatType::Area | ThreatType::Range)) == ThreatType::Normal)
	{
		// Scan aircraft if targeting air
		if ((threatBitfield & 0x4) != 0)
		{

			for (int i = 0; i < AircraftClass::Array->Count; ++i)
			{
				auto aircraft = AircraftClass::Array->Items[i];

				const bool canTarget = !pOwner->IsAlliedWith(aircraft)
					|| pType->AttackFriendlies
					|| pThis->Berzerk
					|| hasRealOwner;

				if (!canTarget)
					continue;

				if (onlyEnemy && aircraft->Owner->ArrayIndex != pOwner->EnemyHouseIndex)
					continue;

				// FIXED: test ah, 40h = method & 0x4000 = ThreatType_4000, NOT Tiberium!
				if ((method & ThreatType::Threattype_4000) && !pOwner->IsAlliedWith(aircraft->Owner))
					continue;

				int evalThreat = 0;
				if (pThis->CanAutoTargetObject(method, threatBitfield, -1, aircraft, &evalThreat, ZoneType::None, &emptyCoord))
				{
					if (evalThreat > bestThreat)
					{
						bestTarget = aircraft;
						bestThreat = evalThreat;
					}
				}
			}
		}

		// Add air targeting to bitfield for vehicle scan
		if (method & ThreatType::Vehicles)
		{
			threatBitfield |= 0x4;
		}

		// Scan TechnoVector (ground units)
		for (int i = 0; i < TechnoClass::Array->Count; ++i)
		{
			auto techno = TechnoClass::Array->Items[i];

			const bool canTarget1 = !pOwner->IsAlliedWith(techno)
				|| combatDamage < 0
				|| (!isTechnoPlayerControlled && what == AbstractType::Infantry && ((InfantryTypeClass*)pType)->Engineer)
				|| pType->AttackFriendlies
				|| pThis->Berzerk
				|| hasRealOwner;

			if (!canTarget1)
				continue;

			if (!RulesExtData::Instance()->AIAirTargetingFix)
			{
				if (techno->LastLayer != Layer::Ground)
				{
					continue;
				}
			}
			else
			{
				const bool canTarget2 = ((method & ThreatType::Air) != ThreatType::Normal) ?
					techno->LastLayer != Layer::Underground : techno->LastLayer == Layer::Ground;

				if (!canTarget2)
					continue;
			}

			if (onlyEnemy && techno->Owner->ArrayIndex != pOwner->EnemyHouseIndex)
				continue;

			// FIXED: test ah, 40h = method & 0x4000 = ThreatType_4000, NOT Tiberium!
			if ((method & ThreatType::Threattype_4000) && !pOwner->IsAlliedWith(techno->Owner))
				continue;

			int evalThreat = 0;
			if (pThis->CanAutoTargetObject(method, threatBitfield, -1, techno, &evalThreat, zone, coord))
			{
				if (evalThreat > bestThreat)
				{
					bestTarget = techno;
					bestThreat = evalThreat;
				}
			}
		}

		return bestTarget;
	}

	// Calculate threat range
	int threatRange = 0;

	if (method & ThreatType::Range)
	{
		threatRange = pThis->GetGuardRange(0);
	}
	else if (method & ThreatType::Area)
	{
		if (pThis->CurrentMission == Mission::Patrol)
		{
			threatRange = pThis->GetGuardRange(2);
		}
		else
		{
			threatRange = pThis->GetGuardRange(1);
		}
	}

	// Override for negative damage units in guard mode
	if (combatDamage < 0 && pThis->CurrentMission == Mission::Guard)
	{
		threatRange = 512;
	}

	// Calculate cell range
	int cellRange = threatRange / 256;

	// If no range, calculate from weapon range
	if (threatRange == 0)
	{

		int weaponRange {};

		if (pType->HasTurret() && !pType->IsGattling)
		{
			weaponRange = pThis->GetWeaponRange(pThis->CurrentWeaponNumber);
		}
		else if (pType->Underwater && pType->Organic && pType->SelfHealing)
		{
			weaponRange = pType->GuardRange;
		}
		else
		{
			const int range0 = pThis->GetWeaponRange(0);
			const int range1 = pThis->GetWeaponRange(1);
			weaponRange = (range0 > range1) ? range0 : range1;
		}

		cellRange = weaponRange / 256 + pType->AirRangeBonus / 256 + 1;
	}

	// Get center cell
	CellStruct centerCell = CellClass::Coord2Cell(coord);

	// Adjust for occupied buildings
	if (pThis->CanOccupyFire())
	{
		cellRange = pThis->GetOccupyRangeBonus() + RulesClass::Instance->OccupyWeaponRange + 1;
	}

	// Scan aircraft in range
	if (!RulesExtData::Instance()->AIAirTargetingFix && method & ThreatType::Air)
	{

		AircraftTrackerClass::Instance->FillCurrentVector(MapClass::Instance->GetCellAt(centerCell), cellRange);

		for (auto aircraft = AircraftTrackerClass::Instance->Get(); aircraft; aircraft = AircraftTrackerClass::Instance->Get())
		{

			const bool canTarget3 = !pOwner->IsAlliedWith(aircraft)
				|| pType->AttackFriendlies
				|| pThis->Berzerk
				|| hasRealOwner;

			if (!canTarget3)
				continue;

			// FIXED: test ah, 40h = method & 0x4000 = ThreatType_4000, NOT Tiberium!
			if ((method & ThreatType::Threattype_4000) && !pOwner->IsAlliedWith(aircraft->Owner))
				continue;

			if (!aircraft->IsOnMap)
				continue;

			if (aircraft->InWhichLayer() == Layer::Ground)
				continue;

			if (onlyEnemy && aircraft->Owner->ArrayIndex != pOwner->EnemyHouseIndex)
				continue;

			int evalThreat = 0;
			if (pThis->CanAutoTargetObject(method, threatBitfield | 0x8002, threatRange, aircraft, &evalThreat, ZoneType::None, &emptyCoord))
			{
				// Distributed fire - add to target lists
				if (pType->DistributedFire)
				{
					pThis->CurrentTargets.emplace_back(aircraft);
					pThis->CurrentTargetThreatValues.emplace_back(evalThreat);
				}

				if (evalThreat > bestThreat)
				{
					bestTarget = aircraft;
					bestThreat = evalThreat;
				}
			}
		}
	}

	// Add air flag for vehicle scanning
	if (method & ThreatType::Vehicles)
	{
		threatBitfield |= 0x4;
	}

	// Early exit for air-only range scan
	const bool isAirOnlySearch = (method == (ThreatType::Air | ThreatType::Range));

	if (RulesExtData::Instance()->FallingDownTargetingFix && isAirOnlySearch)
	{
		if (!AU)
		{
			threatBitfield |= 1 << (int)InfantryClass::AbsID;
			threatBitfield |= 1 << (int)UnitClass::AbsID;
			threatBitfield |= 1 << (int)AircraftClass::AbsID;
		}

		const bool targetFriendly = pType->AttackFriendlies || pThis->Berzerk || hasRealOwner || combatDamage < 0;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (auto pCurrent : ScenarioExtData::Instance()->FallingDownTracker)
		{
			if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
				&& (!onlyEnemy || pCurrent->Owner->ArrayIndex == pThis->Owner->EnemyHouseIndex)
				&& pThis->CanAutoTargetObject(method, threatBitfield, threatRange, pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
			{
				if (pType->DistributedFire)
				{
					pThis->CurrentTargets.emplace_back(pCurrent);
					pThis->CurrentTargetThreatValues.emplace_back(threatBuffer);
				}

				if (threatBuffer > bestThreat)
				{
					bestTarget = pCurrent;
					bestThreat = threatBuffer;
				}
			}
		}

	}
	else if (isAirOnlySearch)
	{
		return bestTarget;
	}

	if (AU)
	{
		const bool targetFriendly = pType->AttackFriendlies || pThis->Berzerk || hasRealOwner || combatDamage < 0;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (const auto pCurrent : ScenarioExtData::Instance()->UndergroundTracker)
		{

			if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
				&& (!onlyEnemy || pCurrent->Owner->ArrayIndex == pOwner->EnemyHouseIndex)
				&& pThis->CanAutoTargetObject(method, threatBitfield, threatRange, pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
			{
				if (pType->DistributedFire)
				{
					pThis->CurrentTargets.emplace_back(pCurrent);
					pThis->CurrentTargetThreatValues.emplace_back(threatBuffer);
				}

				if (threatBuffer > bestThreat)
				{
					bestTarget = pCurrent;
					bestThreat = threatBuffer;
				}
			}
		}
	}

	// Spiral cell scan
	CellStruct bestCell = { 0, 0 };
	int bestCellThreat = 0;

	if (cellRange <= 0)
	{
		return bestTarget;
	}

	// Helper function to evaluate a cell and update best target
	auto EvaluateCellAndUpdate = [&](CellStruct* cellToCheck) -> void
		{
			if (!MapClass::Instance->CoordinatesLegal(cellToCheck))
				return;

			TechnoClass* cellTarget = nullptr;
			int cellThreat = 0;

			if (pThis->TryAutoTargetObject(method, threatBitfield, cellToCheck, threatRange, &cellTarget, &cellThreat, zone))
			{

				if (cellTarget)
				{
					if (onlyEnemy && cellTarget->Owner->ArrayIndex != pOwner->EnemyHouseIndex)
						return;

					// FIXED: test bh, 40h = method & 0x4000 = ThreatType_4000, NOT Tiberium!
					if ((method & ThreatType::Threattype_4000) && !pOwner->IsAlliedWith(cellTarget->Owner))
						return;

					// Distributed fire - add to target lists
					if (pType->DistributedFire)
					{
						pThis->CurrentTargets.emplace_back(cellTarget);
						pThis->CurrentTargetThreatValues.emplace_back(cellThreat);
					}

					if (cellThreat > bestThreat)
					{
						bestThreat = cellThreat;
						bestTarget = cellTarget;
					}
				}
			}

			// If no target yet, evaluate cell itself
			if (!bestTarget)
			{
				int justCellThreat = pThis->EvaluateJustCell(cellToCheck);

				if (justCellThreat > bestCellThreat)
				{
					bestCellThreat = justCellThreat;
					bestCell = *cellToCheck;
				}
			}
		};

	for (int radius = 0; radius < cellRange; ++radius)
	{

		// Scan top and bottom edges of current radius
		for (int x = -radius; x <= radius; ++x)
		{
			// Top edge
			CellStruct topCell;
			topCell.X = centerCell.X + x;
			topCell.Y = centerCell.Y - radius;
			EvaluateCellAndUpdate(&topCell);

			// Bottom edge
			CellStruct bottomCell;
			bottomCell.X = centerCell.X + x;
			bottomCell.Y = centerCell.Y + radius;
			EvaluateCellAndUpdate(&bottomCell);
		}

		// Scan left and right edges (excluding corners already done)
		for (int y = 1 - radius; y < radius; ++y)
		{
			// Left edge
			CellStruct leftCell;
			leftCell.X = centerCell.X - radius;
			leftCell.Y = centerCell.Y + y;
			EvaluateCellAndUpdate(&leftCell);

			// Right edge
			CellStruct rightCell;
			rightCell.X = centerCell.X + radius;
			rightCell.Y = centerCell.Y + y;
			EvaluateCellAndUpdate(&rightCell);
		}

		// Early exit if we found a target at certain radii
		if (bestTarget)
		{
			if (radius == cellRange / 4 || radius == cellRange / 2)
			{
				return bestTarget;
			}
		}

		// If we found a good cell (no target but interesting cell), return it
		if (bestCell.X != 0 || bestCell.Y != 0)
		{
			return MapClass::Instance->GetCellAt(bestCell);
		}
	}

	return bestTarget;
}

ASMJIT_PATCH(0x6F7E1E, TechnoClass_CanAutoTargetObject_AU, 0x6)
{
	enum { Continue = 0x6F7E24, ReturnFalse = 0x6F894F };

	//GET(TechnoClass*, pTarget, ESI);
	GET(int, height, EAX);

	return height >= -20
		|| SelectAutoTarget_Context::AU ? Continue : ReturnFalse;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F8DF0, FakeTechnoClass::__Greatest_Threat);
DEFINE_FUNCTION_JUMP(CALL, 0x4D9942, FakeTechnoClass::__Greatest_Threat);//foot
DEFINE_FUNCTION_JUMP(CALL, 0x445F68, FakeTechnoClass::__Greatest_Threat);//building
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D24, FakeTechnoClass::__Greatest_Threat);//technoVtable

DEFINE_HOOK(0x6F90F8, TechnoClass_SelectAutoTarget_Demacroize, 6)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x6F9116;
}

DEFINE_HOOK_AGAIN(0x6F8F1F, TechnoClass_SelectAutoTarget_Heal, 6)
DEFINE_HOOK(0x6F8EE3, TechnoClass_SelectAutoTarget_Heal, 6)
{
	GET(unsigned int, nVal, EBX);

	nVal |= 0x403Cu;

	R->EBX(nVal);
	return 0x6F8F25;
}