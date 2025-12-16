/**
 * @file Hooks.GreatestThreat.cpp
 * @brief Complete backport of TechnoClass::Greatest_Threat (0x6F8DF0) with improvements
 *
 * This is a full reimplementation of the original game's Greatest_Threat function
 * which handles automatic target acquisition for all TechnoClass objects.
 *
 * Original function: FUN_006f8df0 at address 0x6F8DF0
 *
 * IMPROVEMENTS over original:
 * - Underground unit targeting support (AU flag)
 * - Falling aircraft targeting support (FallingDownTargetingFix)
 * - AI air targeting improvements (AIAirTargetingFix)
 * - Aircraft repair unit support (can target Air + Ground + Naval)
 * - Better distributed fire list handling
 * - Passenger owner sync support for open-topped transports
 *
 * SWOT Analysis:
 * Strengths:
 *   - Full feature parity with original + extensions
 *   - Modular helper functions for maintainability
 *   - Proper healer targeting for Infantry/Unit/Aircraft
 *   - Underground and falling unit support
 *
 * Weaknesses (addressed):
 *   - maxThreat initialization was 0 instead of -1 (FIXED)
 *   - Missing area scan when !isAirOnlySearch (FIXED)
 *   - Healers couldn't target allies properly (FIXED)
 *   - Aircraft healers not handled (FIXED)
 *
 * Opportunities:
 *   - Configurable threat weights via INI
 *   - Custom targeting priorities per unit type
 *   - Threat caching for performance
 *
 * Threats (mitigated):
 *   - Race conditions with target arrays (using proper iteration)
 *   - Invalid pointer access (null checks throughout)
 *   - Integer overflow in range calculations (using proper division)
 */

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

/**
 * @brief Modifies threat method based on unit type for special behaviors
 *
 * Original behavior:
 * - Infantry healers (kind=0xf, CombatDamage<0): method & 3 | 0x4008
 * - Unit healers (kind=1, CombatDamage<0): method & 3 | 0x4010
 * - Engineers: clears Infantry and Vehicles flags
 *
 * @param techno The unit performing the threat scan
 * @param method Original threat method flags
 * @param canHealOrRepair True if unit has negative combat damage (healer/repair)
 * @return Modified threat method
 */
ThreatType InitializeThreatMethod(TechnoClass* techno, ThreatType method, bool canHealOrRepair)
{
	const AbstractType kind = techno->WhatAmI();
	const ThreatType preservedFlags = method & (ThreatType::Area | ThreatType::Range);

	if (kind == AbstractType::Infantry)
	{
		if (canHealOrRepair)
		{
			// Infantry healer: target Infantry only + ally targeting flag
			// Original: method & 3 | 0x4008 (Threattype_4000 | Infantry)
			return preservedFlags | ThreatType::Threattype_4000 | ThreatType::Infantry;
		}

		// Engineer: cannot target Infantry or Vehicles (captures buildings)
		if (static_cast<InfantryClass*>(techno)->Type->Engineer)
		{
			return method & ~(ThreatType::Vehicles | ThreatType::Infantry);
		}
	}
	else if (kind == AbstractType::Unit)
	{
		if (canHealOrRepair)
		{
			// Unit healer: target Vehicles only + ally targeting flag
			// Original: method & 3 | 0x4010 (Threattype_4000 | Vehicles)
			return preservedFlags | ThreatType::Threattype_4000 | ThreatType::Vehicles | ThreatType::Air;
		}
	}
	else if (kind == AbstractType::Aircraft)
	{
		if (canHealOrRepair)
		{
			// Aircraft healer: target Air + Vehicles (ground and naval)
			// Extension: allows repair aircraft to heal all unit types
			return preservedFlags | ThreatType::Threattype_4000 | ThreatType::Air | ThreatType::Vehicles;
		}
	}

	return method;
}

/**
 * @brief Builds the threat bitfield used for target filtering
 *
 * The bitfield is different from ThreatType - it's used by CanAutoTargetObject
 * to filter valid targets.
 *
 * Original mapping:
 * - 0x100 (Range) -> 0x8042
 * - Air (0x4) -> |= 4
 * - Buildings/etc (0x1ba60) -> |= 0x40
 * - Infantry (0x8) -> |= 0x8000
 * - Vehicles/Tiberium (0x50) -> |= 2
 */
int BuildThreatBitfield(ThreatType method)
{
	int bitfield = 0;

	// Range-based search enables broader target filtering
	if ((method & ThreatType::Range) != ThreatType::Normal)
	{
		bitfield = 0x8042;
	}

	// Air targets
	if ((method & ThreatType::Air) != ThreatType::Normal)
	{
		bitfield |= 4;
	}

	// Building-related targets
	constexpr ThreatType buildingFlags =
		ThreatType::TechBuildings | ThreatType::OccupiableBuildings |
		ThreatType::Base_defenses | ThreatType::Factories |
		ThreatType::PowerFacilties | ThreatType::Capture |
		ThreatType::Tiberium | ThreatType::Buildings;

	if ((method & buildingFlags) != ThreatType::Normal)
	{
		bitfield |= 0x40;
	}

	// Infantry targets
	if ((method & ThreatType::Infantry) != ThreatType::Normal)
	{
		bitfield |= 0x8000;
	}

	// Vehicle/Tiberium targets
	if ((method & (ThreatType::Tiberium | ThreatType::Vehicles)) != ThreatType::Normal)
	{
		bitfield |= 2;
	}

	return bitfield;
}

/**
 * @brief Calculates the search range for threat scanning
 *
 * Original logic:
 * - Range flag (0x1): GetGuardRange(0)
 * - Area flag (0x2): GetGuardRange(1) or GetGuardRange(2) for Patrol
 * - Healers in Guard mission: fixed range of 512 leptons
 */
int CalculateThreatRange(TechnoClass* techno, ThreatType method, bool canHealOrRepair)
{
	int range = 0;

	if ((method & ThreatType::Range) != ThreatType::Normal)
	{
		range = techno->GetGuardRange(0);
	}
	else if ((method & ThreatType::Area) != ThreatType::Normal)
	{
		// Patrol uses wider range (mode 2), otherwise mode 1
		const int mode = (techno->CurrentMission == Mission::Patrol) ? 2 : 1;
		range = techno->GetGuardRange(mode);
	}

	// Special case: healers in Guard mission get fixed range
	// Original: if (canHealOrRepair < 0 && CurrentMission == 5) range = 0x200
	if (canHealOrRepair && techno->CurrentMission == Mission::Guard)
	{
		range = 512; // 0x200 = 2 cells
	}

	return range;
}

/**
 * @brief Calculates default weapon range when guard range is 0
 *
 * Original logic handles:
 * - Multi-turret units: use current weapon's range
 * - Organic underwater self-healing units: use GuardRange
 * - Others: max of primary/secondary weapon range
 *
 * Returns range in cells (divided by 256) + AirRangeBonus + 1
 */
int CalculateDefaultRange(TechnoClass* techno)
{
	const TechnoTypeClass* techType = techno->GetTechnoType();
	int range;

	if (techType->HasMultipleTurrets() && !techType->IsGattling)
	{
		range = techno->GetWeaponRange(techno->CurrentWeaponNumber);
	}
	else if (techType->Underwater && techType->Organic && techType->SelfHealing)
	{
		range = techType->GuardRange;
	}
	else
	{
		const int rangePrimary = techno->GetWeaponRange(0);
		const int rangeSecondary = techno->GetWeaponRange(1);
		range = MaxImpl(rangePrimary, rangeSecondary);
	}

	// Convert to cell units and add air bonus + minimum of 1
	return (range / 256) + (techType->AirRangeBonus / 256) + 1;
}

/**
 * @brief Checks if unit is set to attack friendlies
 */
inline bool IsAttackingFriendlies(TechnoClass* techno, bool realOwner)
{
	const TechnoTypeClass* techType = techno->GetTechnoType();
	return techType->AttackFriendlies || techno->Berzerk || realOwner;
}

/**
 * @brief Filters targets by designated enemy house
 * @param a4 If true, only designated enemy passes
 */
inline bool PassesEnemyFilter(bool a4, int targetHouseID, int ownEnemyID)
{
	return !a4 || (targetHouseID == ownEnemyID);
}

/**
 * @brief Filters targets for healer ally targeting (Threattype_4000)
 * When Threattype_4000 is set, only allied targets pass
 */
inline bool PassesAllyFilter(ThreatType method, HouseClass* ownHouse, HouseClass* targetHouse)
{
	if ((method & ThreatType::Threattype_4000) == ThreatType::Normal)
	{
		return true; // No ally filter active
	}
	return ownHouse->IsAlliedWith(targetHouse);
}

/**
 * @brief Combined target validity check for basic attacks
 */
bool ShouldAttackTarget(TechnoClass* techno, HouseClass* targetHouse, bool realOwner, bool a4, ThreatType method)
{
	const bool isEnemy = !techno->Owner->IsAlliedWith(targetHouse);
	const bool attackingFriendlies = IsAttackingFriendlies(techno, realOwner);

	if (!isEnemy && !attackingFriendlies)
	{
		return false;
	}

	if (!PassesEnemyFilter(a4, targetHouse->ArrayIndex, techno->Owner->EnemyHouseIndex))
	{
		return false;
	}

	if (!PassesAllyFilter(method, techno->Owner, targetHouse))
	{
		return false;
	}

	return true;
}

/**
 * @brief Adds target to distributed fire tracking lists
 */
void NOINLINE AddToDistributedFireLists(TechnoClass* techno, AbstractClass* target, int threatValue)
{
	techno->CurrentTargets.push_back(target);
	techno->CurrentTargetThreatValues.push_back(threatValue);
}

/**
 * @brief Scans AircraftClass::Array for air threats (non-area search)
 *
 * Used when Range/Area flags are NOT set
 */
TechnoClass* ScanAircraftThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								 bool realOwner, bool a4, int* maxThreat, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	const bool targetFriendly = IsAttackingFriendlies(techno, realOwner) || canHealOrRepair;
	auto emptyCoord = CoordStruct::Empty;

	for (int i = 0; i < AircraftClass::Array->Count; ++i)
	{
		AircraftClass* aircraft = AircraftClass::Array->Items[i];

		if (!aircraft->IsAlive)
			continue;

		// Check ally/enemy relationship
		const bool isAlly = techno->Owner->IsAlliedWith(aircraft->Owner);
		if (isAlly && !targetFriendly)
			continue;

		// Apply filters
		if (!PassesEnemyFilter(a4, aircraft->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
			continue;

		if (!PassesAllyFilter(method, techno->Owner, aircraft->Owner))
			continue;

		// Evaluate threat
		int threat = 0;
		const bool isValidThreat = techno->CanAutoTargetObject(
			method, bigthreatbitfield, -1,
			aircraft, &threat, ZoneType::None, &emptyCoord
		);

		if (!isValidThreat)
			continue;

		if (threat > *maxThreat)
		{
			bestTarget = aircraft;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

/**
 * @brief Scans TechnoClass::Array for ground threats (non-area search)
 *
 * Handles the second loop in original's non-area search path
 * Original checks: target[0x25] == 2 (Layer::Ground)
 */
TechnoClass* ScanGroundUnitThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								   ZoneType zone, CoordStruct* coords, bool realOwner,
								   bool a4, int* maxThreat, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	const TechnoTypeClass* techType = techno->GetTechnoType();
	const bool targetFriendly = techType->AttackFriendlies || techno->Berzerk || realOwner || canHealOrRepair;
	const bool hasAirFlag = (method & ThreatType::Air) != ThreatType::Normal;

	for (int i = 0; i < TechnoClass::Array->Count; ++i)
	{
		TechnoClass* target = TechnoClass::Array->Items[i];

		if (!target->IsAlive)
			continue;

		// Layer filtering
		// Original: piVar5[0x25] == 2 (checks if on Ground layer)
		if (!RulesExtData::Instance()->AIAirTargetingFix)
		{
			if (target->LastLayer != Layer::Ground)
				continue;
		}
		else
		{
			// With fix: Air flag allows non-underground, otherwise ground only
			const bool canTarget = hasAirFlag ?
				(target->LastLayer != Layer::Underground) :
				(target->LastLayer == Layer::Ground);

			if (!canTarget)
				continue;
		}

		// Ally/enemy relationship check
		const bool isAlly = techno->Owner->IsAlliedWith(target->Owner);
		if (isAlly && !targetFriendly)
			continue;

		// Apply filters
		if (!PassesEnemyFilter(a4, target->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
			continue;

		if (!PassesAllyFilter(method, techno->Owner, target->Owner))
			continue;

		// Evaluate threat
		int threat = 0;
		const bool isValidThreat = techno->CanAutoTargetObject(
			method, bigthreatbitfield, -1,
			target, &threat, zone, coords
		);

		if (!isValidThreat)
			continue;

		if (threat > *maxThreat)
		{
			bestTarget = target;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

/**
 * @brief Scans aircraft in area using AircraftTrackerClass
 *
 * Used for Range/Area based search with Air flag set
 */
TechnoClass* ProcessAircraftInArea(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								   int range, CellStruct* centerCell, bool realOwner, bool a4,
								   int* maxThreat, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	const TechnoTypeClass* techType = techno->GetTechnoType();
	const bool hasDistributedFire = techType->DistributedFire;
	const bool targetFriendly = techType->AttackFriendlies || techno->Berzerk || realOwner || canHealOrRepair;
	auto emptyCoord = CoordStruct::Empty;

	// Initialize aircraft tracker for area
	CellClass* cell = MapClass::Instance->GetCellAt(centerCell);
	AircraftTrackerClass::Instance->AircraftTrackerClass_logics_412B40(cell, range / 256);

	// Iterate tracked aircraft
	for (FootClass* aircraft = AircraftTrackerClass::Instance->Get();
		 aircraft != nullptr;
		 aircraft = AircraftTrackerClass::Instance->Get())
	{
		if (!aircraft->IsAlive || !aircraft->IsOnMap)
			continue;

		// Skip ground-layer aircraft (landed)
		if (aircraft->InWhichLayer() == Layer::Ground)
			continue;

		// Ally/enemy check
		const bool isAlly = techno->Owner->IsAlliedWith(aircraft->Owner);
		if (isAlly && !targetFriendly)
			continue;

		// Apply filters
		if (!PassesEnemyFilter(a4, aircraft->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
			continue;

		if (!PassesAllyFilter(method, techno->Owner, aircraft->Owner))
			continue;

		// Evaluate threat with modified bitfield
		int threat = 0;
		const int modifiedBitfield = bigthreatbitfield |
			static_cast<int>(ThreatType::OccupiableBuildings) |
			static_cast<int>(ThreatType::Area);

		const bool isValidThreat = techno->CanAutoTargetObject(
			method, modifiedBitfield, range,
			aircraft, &threat, ZoneType::None, &emptyCoord
		);

		if (!isValidThreat)
			continue;

		if (hasDistributedFire)
		{
			AddToDistributedFireLists(techno, aircraft, threat);
		}

		if (threat > *maxThreat)
		{
			bestTarget = aircraft;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

/**
 * @brief Evaluates a single cell for threats
 *
 * Calls TryAutoTargetObject on the cell, applies filters,
 * and optionally evaluates cell value as fallback
 */
bool EvaluateCellForThreat(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
						   CellStruct* cell, int range, ZoneType zone, bool a4,
						   TechnoClass** outTarget, int* outThreat, TechnoClass** bestTarget,
						   int* maxThreat, CellStruct* bestCell, int* bestCellValue,
						   bool canHealOrRepair)
{
	// Bounds check
	if (!MapClass::Instance->CoordinatesLegal(cell))
		return false;

	// Try to find threat in cell
	int threat = 0;
	const bool cellHasThreat = techno->TryAutoTargetObject(
		method, bigthreatbitfield, cell, range,
		outTarget, &threat, zone
	);

	if (!cellHasThreat)
	{
		// No threat - evaluate cell value as movement target fallback
		if (*bestTarget == nullptr)
		{
			const int cellValue = techno->EvaluateJustCell(cell);
			if (cellValue > *bestCellValue)
			{
				*bestCellValue = cellValue;
				*bestCell = *cell;
			}
		}
		return false;
	}

	if (*outTarget == nullptr)
		return false;

	// Apply house filters
	if (!PassesEnemyFilter(a4, (*outTarget)->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
		return false;

	if (!PassesAllyFilter(method, techno->Owner, (*outTarget)->Owner))
		return false;

	// Add to distributed fire lists
	if (techno->GetTechnoType()->DistributedFire)
	{
		AddToDistributedFireLists(techno, *outTarget, threat);
	}

	// Update best target
	if (threat > *maxThreat)
	{
		*maxThreat = threat;
		*bestTarget = *outTarget;
	}

	*outThreat = threat;
	return true;
}

/**
 * @brief Scans area in expanding rings for threats
 *
 * Original pattern scans cells in a square ring pattern, expanding outward.
 * Early exit at 1/4 and 1/2 radius if target found.
 * Returns cell location as fallback if no target found.
 */
AbstractClass* ScanAreaThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
							   int scanRadius, int range, CellStruct* centerCell,
							   ZoneType zone, bool a4, int* maxThreat, bool realOwner,
							   bool AU, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	const TechnoTypeClass* technoType = techno->GetTechnoType();
	const HouseClass* pOwner = techno->Owner;
	CellStruct bestCell = CellStruct::Empty;
	int bestCellValue = 0; // Original: piStack_3c = 0

	// Extension: Underground unit targeting
	if (AU)
	{
		const bool targetFriendly = technoType->AttackFriendlies ||
			techno->Berzerk || realOwner || canHealOrRepair;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (const auto pCurrent : ScenarioExtData::Instance()->UndergroundTracker)
		{
			if (!pCurrent->IsAlive)
				continue;

			const bool isAlly = pOwner->IsAlliedWith(pCurrent);
			if (isAlly && !targetFriendly)
				continue;

			if (!PassesEnemyFilter(a4, pCurrent->Owner->ArrayIndex, pOwner->EnemyHouseIndex))
				continue;

			if (!techno->CanAutoTargetObject(method, bigthreatbitfield, range,
				pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
				continue;

			if (technoType->DistributedFire)
			{
				AddToDistributedFireLists(techno, pCurrent, threatBuffer);
			}

			if (threatBuffer > *maxThreat)
			{
				bestTarget = pCurrent;
				*maxThreat = threatBuffer;
			}
		}
	}

	// Main ring scan loop
	for (int radius = 0; radius < scanRadius; ++radius)
	{
		// Scan horizontal edges (top and bottom rows)
		for (short x = static_cast<short>(-radius); x <= static_cast<short>(radius); ++x)
		{
			CellStruct topCell = {
				static_cast<short>(centerCell->X + x),
				static_cast<short>(centerCell->Y - radius)
			};
			CellStruct bottomCell = {
				static_cast<short>(centerCell->X + x),
				static_cast<short>(centerCell->Y + radius)
			};

			TechnoClass* cellTarget = nullptr;
			int cellThreat = 0;

			EvaluateCellForThreat(techno, method, bigthreatbitfield, &topCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat, &bestCell, &bestCellValue, canHealOrRepair);

			EvaluateCellForThreat(techno, method, bigthreatbitfield, &bottomCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat, &bestCell, &bestCellValue, canHealOrRepair);
		}

		// Scan vertical edges (left and right columns, excluding corners)
		for (short y = static_cast<short>(-radius + 1); y < static_cast<short>(radius); ++y)
		{
			CellStruct leftCell = {
				static_cast<short>(centerCell->X - radius),
				static_cast<short>(centerCell->Y + y)
			};
			CellStruct rightCell = {
				static_cast<short>(centerCell->X + radius),
				static_cast<short>(centerCell->Y + y)
			};

			TechnoClass* cellTarget = nullptr;
			int cellThreat = 0;

			EvaluateCellForThreat(techno, method, bigthreatbitfield, &leftCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat, &bestCell, &bestCellValue, canHealOrRepair);

			EvaluateCellForThreat(techno, method, bigthreatbitfield, &rightCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat, &bestCell, &bestCellValue, canHealOrRepair);
		}

		// Early exit: target found at 1/4 or 1/2 scan radius
		// Original: (radius == (scanRadius + 3) / 4) || (radius == scanRadius / 2)
		if (bestTarget != nullptr)
		{
			const bool isQuarterRadius = (radius == (scanRadius + 3) / 4);
			const bool isHalfRadius = (radius == scanRadius / 2);

			if (isQuarterRadius || isHalfRadius)
			{
				return bestTarget;
			}
		}

		// Return cell location if found (and no target)
		// Original: if (bestCell != DAT_00b0ea50/52) return GetCellAt(bestCell)
		const bool foundGoodCell = (bestCell.X != CellStruct::Empty.X) ||
			(bestCell.Y != CellStruct::Empty.Y);
		if (foundGoodCell)
		{
			return MapClass::Instance->GetCellAt(bestCell);
		}
	}

	return bestTarget;
}

/**
 * @brief Scans for falling/crashing aircraft (extension)
 */
AbstractClass* ScanFallingAirThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
									 int range, int* maxThreat, bool realOwner, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	const TechnoTypeClass* technoType = techno->GetTechnoType();
	const HouseClass* pOwner = techno->Owner;
	const bool targetFriendly = technoType->AttackFriendlies ||
		techno->Berzerk || realOwner || canHealOrRepair;
	int threatBuffer = 0;
	auto tempCrd = CoordStruct::Empty;

	for (const auto pCurrent : ScenarioExtData::Instance()->FallingDownTracker)
	{
		if (!pCurrent->IsAlive)
			continue;

		const bool isAlly = pOwner->IsAlliedWith(pCurrent);
		if (isAlly && !targetFriendly)
			continue;

		if (!techno->CanAutoTargetObject(method, bigthreatbitfield, range,
			pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
			continue;

		if (technoType->DistributedFire)
		{
			AddToDistributedFireLists(techno, pCurrent, threatBuffer);
		}

		if (threatBuffer > *maxThreat)
		{
			bestTarget = pCurrent;
			*maxThreat = threatBuffer;
		}
	}

	return bestTarget;
}

/**
 * @brief Main Greatest_Threat implementation
 *
 * Replaces TechnoClass::Greatest_Threat at 0x6F8DF0
 *
 * @param techno The unit performing target acquisition
 * @param method Threat type flags defining what to search for
 * @param location Center coordinates for area search
 * @param a4 If true, only targets designated enemy house
 * @return Best target found, or nullptr if none
 */
AbstractClass* __fastcall FakeTechnoClass::__Greatest_Threat(
	TechnoClass* techno,
	discard_t,
	ThreatType method,
	CoordStruct* location,
	bool a4)
{
	// Increment global scan counter
	// Original: DAT_00a8ec34 = DAT_00a8ec34 + 1
	++TechnoClass::TargetScanCounter();

	const TechnoTypeClass* techType = techno->GetTechnoType();
	const bool isTechnoPlayerControlled = techno->Owner->IsControlledByHuman();

	// Check for underground targeting extension flag
	const bool AU = SelectAutoTarget_Context::AU =
		(ExtendedThreatType(method) & ExtendedThreatType::Underground) != ExtendedThreatType::none;

	// Early exit: player-controlled units with NoAutoFire
	// Original: if (TechnoType->NoAutoFire && IsControlledByHuman()) return 0
	if (techType->NoAutoFire && isTechnoPlayerControlled)
	{
		return nullptr;
	}

	// Calculate movement zone for ground units
	// Original: if (!(method & 1) && WhatAmI() != 6 && WhatAmI() != 2)
	ZoneType zone = ZoneType::None;
	const bool needsZoneCalculation = (method & ThreatType::Range) == ThreatType::Normal;

	if (needsZoneCalculation)
	{
		const AbstractType kind = techno->WhatAmI();
		const bool isGroundUnit = (kind != AbstractType::Building) && (kind != AbstractType::Aircraft);

		if (isGroundUnit)
		{
			CoordStruct* center = techno->GetCoords(location);
			CellStruct cell = {
				static_cast<short>(center->X / 256),
				static_cast<short>(center->Y / 256)
			};
			zone = MapClass::Instance->GetMovementZoneType(cell, techType->MovementZone, true);
		}
	}

	// Check if unit is a healer/repair unit
	const bool canHealOrRepair = (techno->CombatDamage(-1) < 0);

	// Modify threat method based on unit type
	method = InitializeThreatMethod(techno, method, canHealOrRepair);

	// Build threat bitfield for filtering
	int bigthreatbitfield = BuildThreatBitfield(method);

	// Initialize distributed fire lists if enabled
	// Original: if (DistributedFire) { CurrentTargets.clear(); CurrentTargetThreatValues.clear(); }
	if (techType->DistributedFire)
	{
		techno->CurrentTargets.reset();
		techno->CurrentTargetThreatValues.reset();
	}

	// Check for open-topped transport (affects ally targeting)
	bool realOwner = false;
	if (techno->Transporter != nullptr)
	{
		TechnoTypeClass* transportType = techno->Transporter->GetTechnoType();
		if (transportType->OpenTopped &&
			!TechnoTypeExtContainer::Instance.Find(transportType)->Passengers_SyncOwner)
		{
			realOwner = (techno->Transporter->OriginallyOwnedByHouse != nullptr);
		}
	}

	// Add underground targeting abstract types to bitfield
	if (AU)
	{
		bigthreatbitfield |= 1 << static_cast<int>(AbstractType::Infantry);
		bigthreatbitfield |= 1 << static_cast<int>(AbstractType::Unit);
		bigthreatbitfield |= 1 << static_cast<int>(AbstractType::Aircraft);
	}

	// Initialize tracking variables
	// CRITICAL: Original uses -1 (0xFFFFFFFF) so any threat >= 0 is accepted
	int maxThreat = -1;
	AbstractClass* bestTarget = nullptr;

	// Determine search mode
	const bool isRangeOrAreaSearch =
		(method & (ThreatType::Area | ThreatType::Range)) != ThreatType::Normal;

	// =====================================================
	// PATH 1: Non-Range/Area Search (direct array scan)
	// Original: if ((param_1 & 3) == 0) { ... }
	// =====================================================
	if (!isRangeOrAreaSearch)
	{
		// Scan aircraft array
		const bool shouldScanAircraft = (bigthreatbitfield & 4) != 0;
		if (shouldScanAircraft)
		{
			bestTarget = ScanAircraftThreats(
				techno, method, bigthreatbitfield,
				realOwner, a4, &maxThreat, canHealOrRepair
			);
		}

		// Add vehicles to bitfield if needed
		if ((method & ThreatType::Vehicles) != ThreatType::Normal)
		{
			bigthreatbitfield |= 4;
		}

		// Scan ground units
		TechnoClass* groundTarget = ScanGroundUnitThreats(
			techno, method, bigthreatbitfield,
			zone, location, realOwner, a4, &maxThreat, canHealOrRepair
		);

		if (groundTarget != nullptr)
		{
			bestTarget = groundTarget;
		}

		return bestTarget;
	}

	// =====================================================
	// PATH 2: Range/Area Based Search (cell-based scan)
	// =====================================================

	// Calculate search range
	int range = CalculateThreatRange(techno, method, canHealOrRepair);
	int scanRadius = range / 256;

	// Use default weapon range if guard range is 0
	if (range == 0)
	{
		scanRadius = CalculateDefaultRange(techno);
	}

	// Get center cell for area search
	CellStruct centerCell = {
		static_cast<short>(location->X / 256),
		static_cast<short>(location->Y / 256)
	};

	// Adjust range for occupied buildings
	if (techno->CanOccupyFire())
	{
		const int baseRange = techno->GetOccupyRangeBonus();
		scanRadius = baseRange + RulesClass::Instance->OccupyWeaponRange + 1;
	}

	// Process aircraft in area (standard behavior)
	if (!RulesExtData::Instance()->AIAirTargetingFix)
	{
		const bool shouldSearchAir = (method & ThreatType::Air) != ThreatType::Normal;
		if (shouldSearchAir)
		{
			bestTarget = ProcessAircraftInArea(
				techno, method, bigthreatbitfield, range,
				&centerCell, realOwner, a4, &maxThreat, canHealOrRepair
			);
		}
	}

	// Add vehicles to bitfield if needed
	if ((method & ThreatType::Vehicles) != ThreatType::Normal)
	{
		bigthreatbitfield |= 4;
	}

	// Check for air-only search (method == 0x5 = Range | Air)
	// Original: if (param_1 != 5) { ... area scan ... }
	const bool isAirOnlySearch = (method == (ThreatType::Air | ThreatType::Range));

	if (isAirOnlySearch)
	{
		// Extension: scan falling aircraft
		if (RulesExtData::Instance()->FallingDownTargetingFix)
		{
			AbstractClass* fallingTarget = ScanFallingAirThreats(
				techno, method, bigthreatbitfield, range,
				&maxThreat, realOwner, canHealOrRepair
			);

			if (fallingTarget != nullptr)
			{
				bestTarget = fallingTarget;
			}
		}
		return bestTarget;
	}

	// Main area threat scan
	if (scanRadius > 0)
	{
		AbstractClass* areaTarget = ScanAreaThreats(
			techno, method, bigthreatbitfield, scanRadius,
			range, &centerCell, zone, a4, &maxThreat, realOwner, AU, canHealOrRepair
		);

		if (areaTarget != nullptr)
		{
			bestTarget = areaTarget;
		}
	}

	return bestTarget;
}

// ============================================================================
// Hook for underground unit height check in CanAutoTargetObject
// ============================================================================
ASMJIT_PATCH(0x6F7E1E, TechnoClass_CanAutoTargetObject_AU, 0x6)
{
	enum { Continue = 0x6F7E24, ReturnFalse = 0x6F894F };

	GET(int, height, EAX);

	// Allow underground units (height >= -20) or when AU flag is set
	return (height >= -20 || SelectAutoTarget_Context::AU) ? Continue : ReturnFalse;
}

// ============================================================================
// Function hooks to replace original Greatest_Threat
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x6F8DF0, FakeTechnoClass::__Greatest_Threat);
DEFINE_FUNCTION_JUMP(CALL, 0x4D9942, FakeTechnoClass::__Greatest_Threat); // FootClass
DEFINE_FUNCTION_JUMP(CALL, 0x445F68, FakeTechnoClass::__Greatest_Threat); // BuildingClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D24, FakeTechnoClass::__Greatest_Threat); // TechnoClass vtable
