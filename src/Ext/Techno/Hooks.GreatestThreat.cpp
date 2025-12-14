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

// Helper function to initialize threat method based on unit type
ThreatType InitializeThreatMethod(TechnoClass* techno, ThreatType method, bool canHealOrRepair)
{
	AbstractType kind = techno->WhatAmI();

	if (kind == AbstractType::Infantry)
	{
		if (canHealOrRepair)
		{
			return (method & (ThreatType::Area | ThreatType::Range)) | ThreatType::Threattype_4000 | ThreatType(0x3C);
		}
		if (((InfantryClass*)techno)->Type->Engineer)
		{
			return method & ~(ThreatType::Vehicles | ThreatType::Infantry);
		}
	}
	else if (kind == AbstractType::Unit && canHealOrRepair)
	{
		return (method & (ThreatType::Area | ThreatType::Range)) | ThreatType::Threattype_4000 | ThreatType(0x3C);
	}

	return method;
}

// Helper function to build bigthreatbitfield from method
int BuildThreatBitfield(ThreatType method)
{
	int bitfield = 0;

	if ((method & ThreatType::Range) != ThreatType::Normal)
	{
		bitfield = 0x8042;
	}

	if ((method & ThreatType::Air) != ThreatType::Normal)
	{
		bitfield |= 4;
	}

	if ((method & (ThreatType::TechBuildings | ThreatType::OccupiableBuildings | ThreatType::Base_defenses |
		ThreatType::Factories | ThreatType::PowerFacilties | ThreatType::Capture |
		ThreatType::Tiberium | ThreatType::Buildings)) != ThreatType::Normal)
	{
		bitfield |= 0x40;
	}

	if ((method & ThreatType::Infantry) != ThreatType::Normal)
	{
		bitfield |= 0x8000;
	}

	if ((method & (ThreatType::Tiberium | ThreatType::Vehicles)) != ThreatType::Normal)
	{
		bitfield |= 2;
	}

	return bitfield;
}

// Helper function to calculate threat range
int CalculateThreatRange(TechnoClass* techno, ThreatType method, bool canHealOrRepair)
{
	int range = 0;

	if ((method & ThreatType::Range) != ThreatType::Normal)
	{
		range = techno->GetGuardRange(0);
	}
	else if ((method & ThreatType::Area) != ThreatType::Normal)
	{
		range = techno->GetGuardRange((techno->CurrentMission == Mission::Patrol) ? 2 : 1);
	}

	// Special case for MISSION_GUARD with negative combat damage
	if (canHealOrRepair && techno->CurrentMission == Mission::Guard)
	{
		range = 512;
	}

	return range;
}

// Helper function to calculate default weapon range
int CalculateDefaultRange(TechnoClass* techno)
{
	TechnoTypeClass* techType = techno->GetTechnoType();
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
		int rangePrimary = techno->GetWeaponRange(0);
		int rangeSecondary = techno->GetWeaponRange(1);
		range = (rangePrimary > rangeSecondary) ? rangePrimary : rangeSecondary;
	}

	return range / 256 + techType->AirRangeBonus / 256 + 1;
}

// Helper function to check if unit should attack friendlies
bool IsAttackingFriendlies(TechnoClass* techno, bool realOwner)
{
	TechnoTypeClass* techType = techno->GetTechnoType();
	return techType->AttackFriendlies || techno->Berzerk || realOwner;
}

// Helper function to check enemy filter
bool PassesEnemyFilter(bool a4, int targetHouseID, int ownEnemyID)
{
	if (!a4)
	{
		return true;
	}
	return targetHouseID == ownEnemyID;
}

// Helper function to check ally filter (for method bit 0x4000)
bool PassesAllyFilter(ThreatType method, HouseClass* ownHouse, HouseClass* targetHouse)
{
	if ((method & ThreatType::Threattype_4000) == ThreatType::Normal)
	{
		return true;
	}
	return ownHouse->IsAlliedWith(targetHouse);
}

// Helper function to check if target should be attacked
bool ShouldAttackTarget(TechnoClass* techno, HouseClass* targetHouse, bool realOwner, bool a4, ThreatType method)
{
	// Check basic enemy/ally relationship
	bool isEnemy = !techno->Owner->IsAlliedWith(targetHouse);
	bool attackingFriendlies = IsAttackingFriendlies(techno, realOwner);

	if (!isEnemy && !attackingFriendlies)
	{
		return false;
	}

	// Check enemy filter
	if (!PassesEnemyFilter(a4, targetHouse->ArrayIndex, techno->Owner->EnemyHouseIndex))
	{
		return false;
	}

	// Check ally filter
	if (!PassesAllyFilter(method, techno->Owner, targetHouse))
	{
		return false;
	}

	return true;
}

// Helper function to check special infantry ally attack case
bool ShouldInfantryAttackAlly(TechnoClass* techno, bool canHealOrRepair, bool isTechnoPlayerControlled)
{

	// Can't heal, is not player controlled, is infantry with special flag
	if (!canHealOrRepair)
	{
		return false;
	}

	if (isTechnoPlayerControlled)
	{
		return false;
	}

	if (auto pInf = cast_to <InfantryClass*, false>(techno))
	{
		if (pInf->Type->Engineer)
			return true;
	}

	return false;
}

// Helper function to add target to distributed fire lists
void NOINLINE AddToDistributedFireLists(TechnoClass* techno, AbstractClass* target, int threatValue)
{
	techno->CurrentTargets.push_back(target);
	techno->CurrentTargetThreatValues.push_back(threatValue);
}

// Helper function to scan aircraft threats (no area search)
TechnoClass* ScanAircraftThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								  int range, bool realOwner, bool a4, int* maxThreat)
{
	TechnoClass* bestTarget = nullptr;

	for (int i = 0; i < AircraftClass::Array->Count; ++i)
	{
		AircraftClass* aircraft = AircraftClass::Array->Items[i];
		int threat = 0;

		// Check if we should attack this aircraft
		if (!aircraft->IsAlive || !ShouldAttackTarget(techno, aircraft->Owner, realOwner, a4, method))
		{
			continue;
		}

		auto emptyCoord = CoordStruct::Empty;
		// Evaluate the aircraft as a threat
		bool isValidThreat = techno->CanAutoTargetObject(
			method, bigthreatbitfield, -1,
			aircraft, &threat, ZoneType::None, &emptyCoord
		);

		if (!isValidThreat)
		{
			continue;
		}

		// Update best target if this is a bigger threat
		if (threat > *maxThreat)
		{
			bestTarget = aircraft;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

// Helper function to scan ground unit threats
TechnoClass* ScanGroundUnitThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								   ZoneType zone, CoordStruct* arg_4, bool realOwner,
								   bool a4, int* maxThreat, bool canHealOrRepair, bool isTechnoPlayerControlled)
{
	TechnoClass* bestTarget = nullptr;

	for (int i = 0; i < TechnoClass::Array->Count; ++i)
	{
		TechnoClass* target = TechnoClass::Array->Items[i];
		int threat = 0;

		// Check if target is on ground layer
		// TODO : add range here to fix the targeting bug

		if (!RulesExtData::Instance()->AIAirTargetingFix)
		{
			if (target->LastLayer != Layer::Ground)
			{
				continue;
			}
		}
		else
		{
			const bool canTarget = ((method & ThreatType::Air) != ThreatType::Normal) ?
				target->LastLayer != Layer::Underground : target->LastLayer == Layer::Ground;

			if (!canTarget)
				continue;
		}

		if (!target->IsAlive)
		{
			continue;
		}

		// Check basic attack conditions
		bool isAlly = techno->Owner->IsAlliedWith(target->Owner);
		bool shouldAttack = false;

		if (!isAlly)
		{
			// Enemy target
			shouldAttack = true;
		}
		else
		{
			// Allied target - check special conditions
			if (ShouldInfantryAttackAlly(techno, canHealOrRepair, isTechnoPlayerControlled))
			{
				shouldAttack = true;
			}
			else if (IsAttackingFriendlies(techno, realOwner))
			{
				shouldAttack = true;
			}
		}

		if (!shouldAttack)
		{
			continue;
		}

		// Check enemy filter
		if (!PassesEnemyFilter(a4, target->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
		{
			continue;
		}

		// Check ally filter
		if (!PassesAllyFilter(method, techno->Owner, target->Owner))
		{
			continue;
		}

		// Evaluate the target
		bool isValidThreat = techno->CanAutoTargetObject(
			method, bigthreatbitfield, -1,
		   target, &threat, zone, arg_4
		);

		if (!isValidThreat)
		{
			continue;
		}

		// Update best target if this is a bigger threat
		if (threat > *maxThreat)
		{
			bestTarget = target;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

// Helper function to process aircraft in area
TechnoClass* ProcessAircraftInArea(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								   int range, CellStruct* centerCell, bool realOwner, bool a4,
								   int* maxThreat)
{
	TechnoClass* bestTarget = nullptr;
	bool hasDistributedFire = techno->GetTechnoType()->DistributedFire;

	// Initialize aircraft tracker
	CellClass* cell = MapClass::Instance->GetCellAt(centerCell);
	AircraftTrackerClass::Instance->AircraftTrackerClass_logics_412B40(cell, range / 256);

	// Iterate through tracked aircraft
	for (FootClass* aircraft = AircraftTrackerClass::Instance->Get();
		 aircraft != nullptr;
		 aircraft = AircraftTrackerClass::Instance->Get())
	{

		// Check if we should attack this aircraft
		if (!aircraft->IsAlive || !ShouldAttackTarget(techno, aircraft->Owner, realOwner, a4, method))
		{
			continue;
		}

		// Check if aircraft is down and not on ground
		if (!aircraft->IsOnMap)
		{
			continue;
		}

		Layer aircraftLayer = aircraft->InWhichLayer();
		if (aircraftLayer == Layer::Ground)
		{
			continue;
		}

		// Evaluate the aircraft
		int threat = 0;
		auto emptyCoord = CoordStruct::Empty;
		int modifiedBitfield = bigthreatbitfield | (int)ThreatType::OccupiableBuildings | (int)ThreatType::Area;
		bool isValidThreat = techno->CanAutoTargetObject(
			method, modifiedBitfield, range,
			aircraft, &threat, ZoneType::None, &emptyCoord
		);

		if (!isValidThreat)
		{
			continue;
		}

		// Add to distributed fire lists if enabled
		if (hasDistributedFire)
		{
			AddToDistributedFireLists(techno, aircraft, threat);
		}

		// Update best target if this is a bigger threat
		if (threat > *maxThreat)
		{
			bestTarget = aircraft;
			*maxThreat = threat;
		}
	}

	return bestTarget;
}

// Helper function to evaluate a single cell for threats
bool EvaluateCellForThreat(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
						   CellStruct* cell, int range, ZoneType zone, bool a4,
						   TechnoClass** outTarget, int* outThreat, TechnoClass** bestTarget,
						   int* maxThreat, CellStruct* bestCell, int* bestCellValue)
{
	// Check if cell is in map bounds
	if (!MapClass::Instance->CoordinatesLegal(cell))
	{
		return false;
	}

	// Evaluate cell for threats
	int threat = 0;

	bool cellHasThreat = techno->TryAutoTargetObject(
		method, bigthreatbitfield, cell, range,
		outTarget, &threat, zone
	);

	if (!cellHasThreat)
	{
		// No threat in cell, evaluate cell value as fallback
		if (*bestTarget == nullptr)
		{
			int cellValue = techno->EvaluateJustCell(cell);
			if (cellValue > *bestCellValue)
			{
				*bestCellValue = cellValue;
				*bestCell = *cell;
			}
		}
		return false;
	}

	// Check if we got a valid target
	if (*outTarget == nullptr)
	{
		return false;
	}

	// Check enemy filter
	if (!PassesEnemyFilter(a4, (*outTarget)->Owner->ArrayIndex, techno->Owner->EnemyHouseIndex))
	{
		return false;
	}

	// Check ally filter
	if (!PassesAllyFilter(method, techno->Owner, (*outTarget)->Owner))
	{
		return false;
	}

	// Add to distributed fire lists if enabled
	bool hasDistributedFire = techno->GetTechnoType()->DistributedFire;
	if (hasDistributedFire)
	{
		AddToDistributedFireLists(techno, (*outTarget), threat);
	}

	// Update best target if this is a bigger threat
	if (threat > *maxThreat)
	{
		*maxThreat = threat;
		*bestTarget = (*outTarget);
	}

	*outThreat = threat;
	return true;
}

// Helper function to scan area in rings
AbstractClass* ScanAreaThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								 int scanRadius, int range, CellStruct* centerCell,
								 ZoneType zone, bool a4, int* maxThreat, bool realOwner, bool AU, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	TechnoTypeClass* technoType = techno->GetTechnoType();
	const auto pOwner = techno->Owner;
	CellStruct bestCell = CellStruct::Empty;
	int bestCellValue = 0;

	if (AU)
	{
		const bool targetFriendly = technoType->AttackFriendlies || techno->Berzerk || realOwner || canHealOrRepair;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (const auto pCurrent : ScenarioExtData::Instance()->UndergroundTracker)
		{
			if (!pCurrent->IsAlive)
				continue;

			if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
				&& (!a4 || pCurrent->Owner->ArrayIndex == pOwner->EnemyHouseIndex)
				&& techno->CanAutoTargetObject(method, bigthreatbitfield, range, pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
			{
				if (technoType->DistributedFire) {
					AddToDistributedFireLists(techno, pCurrent, threatBuffer);
				}

				if (threatBuffer > *maxThreat)
				{
					bestTarget = pCurrent;
					*maxThreat = threatBuffer;
				}
			}
		}
	}

	for (int radius = 0; radius < scanRadius; ++radius)
	{
		// Scan horizontal edges (top and bottom)
		for (short x = ((short)-radius); x <= ((short)radius); ++x)
		{
			CellStruct topCell = { (short)(centerCell->X + x), (short)(centerCell->Y - radius) };
			CellStruct bottomCell = { (short)(centerCell->X + x), (short)(centerCell->Y + radius) };

			TechnoClass* cellTarget = nullptr;
			int cellThreat = 0;

			EvaluateCellForThreat(
				techno, method, bigthreatbitfield, &topCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat,
				&bestCell, &bestCellValue
			);

			EvaluateCellForThreat(
				techno, method, bigthreatbitfield, &bottomCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat,
				&bestCell, &bestCellValue
			);
		}

		// Scan vertical edges (left and right, excluding corners)
		for (short y = ((short)-radius) + 1; y < ((short)radius); ++y)
		{
			CellStruct leftCell = { (short)(centerCell->X - radius), (short)(centerCell->Y + y) };
			CellStruct rightCell = { (short)(centerCell->X + radius), (short)(centerCell->Y + y) };

			TechnoClass* cellTarget = nullptr;
			int cellThreat = 0;

			EvaluateCellForThreat(
				techno, method, bigthreatbitfield, &leftCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat,
				&bestCell, &bestCellValue
			);

			EvaluateCellForThreat(
				techno, method, bigthreatbitfield, &rightCell, range, zone, a4,
				&cellTarget, &cellThreat, &bestTarget, maxThreat,
				&bestCell, &bestCellValue
			);
		}

		// Early exit conditions - found target at 1/4 or 1/2 radius
		if (bestTarget != nullptr)
		{
			bool isQuarterRadius = (radius == scanRadius / 4);
			bool isHalfRadius = (radius == scanRadius / 2);

			if (isQuarterRadius || isHalfRadius)
			{
				return bestTarget;
			}
		}

		// If we found a good cell location (and no target), return it
		bool foundGoodCell = (bestCell.X != CellStruct::Empty.X || bestCell.Y != CellStruct::Empty.Y);
		if (foundGoodCell)
		{
			return MapClass::Instance->GetCellAt(bestCell);
		}
	}

	return bestTarget;
}

AbstractClass* ScanAreaAirThreats(TechnoClass* techno, ThreatType method, int bigthreatbitfield,
								 int scanRadius, int range, CellStruct* centerCell,
								 ZoneType zone, bool a4, int* maxThreat, bool realOwner, bool AU, bool canHealOrRepair)
{
	TechnoClass* bestTarget = nullptr;
	TechnoTypeClass* technoType = techno->GetTechnoType();
	const auto pOwner = techno->Owner;
	CellStruct bestCell = CellStruct::Empty;

	if (!AU)
	{
		bigthreatbitfield |= 1 << (int)InfantryClass::AbsID;
		bigthreatbitfield |= 1 << (int)UnitClass::AbsID;
		bigthreatbitfield |= 1 << (int)AircraftClass::AbsID;
	}

	const bool targetFriendly = technoType->AttackFriendlies || techno->Berzerk || realOwner || canHealOrRepair;
	int threatBuffer = 0;
	auto tempCrd = CoordStruct::Empty;

	for (const auto pCurrent : ScenarioExtData::Instance()->FallingDownTracker)
	{
		if (!pCurrent->IsAlive)
			continue;

		if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
			&& (!a4 || pCurrent->Owner->ArrayIndex == techno->Owner->EnemyHouseIndex)
			&& techno->CanAutoTargetObject(method, bigthreatbitfield, range, pCurrent, &threatBuffer, ZoneType::None, &tempCrd))
		{
			if (technoType->DistributedFire) {
				AddToDistributedFireLists(techno, pCurrent, threatBuffer);
			}

			if (threatBuffer > *maxThreat)
			{
				bestTarget = pCurrent;
				*maxThreat = threatBuffer;
			}
		}
	}

	return bestTarget;
}

namespace SelectAutoTarget_Context
{
	bool AU = false;
}

// Main function
AbstractClass* __fastcall FakeTechnoClass::__Greatest_Threat(TechnoClass* techno, discard_t, ThreatType method,
												CoordStruct* location, bool a4)
{
	++TechnoClass::TargetScanCounter();

	// Early exit for player controlled units with NoAutoFire
	TechnoTypeClass* techType = techno->GetTechnoType();
	const bool isTechnoPlayerControlled = techno->Owner->IsControlledByHuman();
	bool AU = SelectAutoTarget_Context::AU = (ExtendedThreatType(method) & ExtendedThreatType::Underground) != ExtendedThreatType::none;

	if (techType->NoAutoFire && isTechnoPlayerControlled)
	{
		return nullptr;
	}

	// Determine zone for ground units
	ZoneType zone = ZoneType::None;
	bool needsZoneCalculation = ((method & ThreatType::Range) == ThreatType::Normal);
	const bool canHealOrRepair = (techno->CombatDamage(-1) < 0);

	if (needsZoneCalculation)
	{
		AbstractType kind = techno->WhatAmI();

		bool isGroundUnit = (kind != AbstractType::Building && kind != AbstractType::Aircraft);

		if (isGroundUnit)
		{
			CoordStruct* center = techno->GetCoords(location);
			CellStruct cell = { (short)(center->X / 256), (short)(center->Y / 256) };
			zone = MapClass::Instance->GetMovementZoneType(cell, techType->MovementZone, true);
		}
	}

	// Initialize threat method based on unit type
	method = InitializeThreatMethod(techno, method, canHealOrRepair);

	// Build threat bitfield
	int bigthreatbitfield = BuildThreatBitfield(method);

	// Initialize distributed fire lists if needed
	if (techType->DistributedFire)
	{
		techno->CurrentTargets.reset();
		techno->CurrentTargetThreatValues.reset();
	}

	// Check if in open-topped transport
	bool realOwner = false;
	bool hasTransport = (techno->Transporter != nullptr);

	if (AU)
	{
		bigthreatbitfield |= 1 << (int)AbstractType::Infantry;
		bigthreatbitfield |= 1 << (int)AbstractType::Unit;
		bigthreatbitfield |= 1 << (int)AbstractType::Aircraft;
	}

	if (hasTransport)
	{
		TechnoTypeClass* transportType = techno->Transporter->GetTechnoType();
		if (transportType->OpenTopped && !TechnoTypeExtContainer::Instance.Find(transportType)->Passengers_SyncOwner)
		{
			realOwner = (techno->Transporter->OriginallyOwnedByHouse != nullptr);
		}
	}

	int maxThreat = 0;
	AbstractClass* bestTarget = nullptr;

	// Handle non-range/area based threat search
	bool isRangeOrAreaSearch = ((method & (ThreatType::Area | ThreatType::Range)) != ThreatType::Normal);

	if (!isRangeOrAreaSearch)
	{
		// Scan aircraft if needed
		bool shouldScanAircraft = ((bigthreatbitfield & 4) != 0);
		if (shouldScanAircraft)
		{
			bestTarget = ScanAircraftThreats(
				techno, method, bigthreatbitfield, -1,
				realOwner, a4, &maxThreat
			);
		}

		// Include vehicles in bitfield if needed
		if ((method & ThreatType::Vehicles) != ThreatType::Normal)
		{
			bigthreatbitfield |= 4;
		}

		// Scan ground units
		TechnoClass* groundTarget = ScanGroundUnitThreats(
			techno, method, bigthreatbitfield,
			zone, location, realOwner, a4, &maxThreat
			, canHealOrRepair, isTechnoPlayerControlled);

		if (groundTarget != nullptr)
		{
			bestTarget = groundTarget;
		}

		return bestTarget;
	}

	// Handle range/area based threat search
	int range = CalculateThreatRange(techno, method, canHealOrRepair);
	int scanRadius = range / 256;

	// Calculate default range if not specified
	if (range == 0)
	{
		scanRadius = CalculateDefaultRange(techno);
	}

	// Get center cell
	CellStruct centerCell = { (short)(location->X / 256), (short)(location->Y / 256) };

	// Adjust range for occupied buildings
	bool isOccupied = techno->CanOccupyFire();
	if (isOccupied)
	{
		int baseRange = techno->GetOccupyRangeBonus();
		scanRadius = baseRange + RulesClass::Instance->OccupyWeaponRange + 1;
	}

	if(!RulesExtData::Instance()->AIAirTargetingFix){
		// Process aircraft in area
		const bool shouldSearchAir = ((method & ThreatType::Air) != ThreatType::Normal);

		if (shouldSearchAir) {
			bestTarget = ProcessAircraftInArea(
				techno, method, bigthreatbitfield, range,
				&centerCell, realOwner, a4, &maxThreat
			);
		}
	}

	// Include vehicles in bitfield if needed
	if ((method & ThreatType::Vehicles) != ThreatType::Normal)
	{
		bigthreatbitfield |= 4;
	}

	// Early exit for air-only search
	bool isAirOnlySearch = (method == (ThreatType::Air | ThreatType::Range));
	if (RulesExtData::Instance()->FallingDownTargetingFix && isAirOnlySearch) {
		AbstractClass* areaTarget = ScanAreaAirThreats(
		techno, method, bigthreatbitfield, scanRadius,
		range, &centerCell, zone, a4, &maxThreat, realOwner, AU, canHealOrRepair);

		if (areaTarget != nullptr)
		{
			bestTarget = areaTarget;
		}

		return bestTarget;
	}else if(!isAirOnlySearch)
		return bestTarget ;

	// Scan area for ground threats
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
