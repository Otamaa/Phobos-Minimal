#include "Body.h"

#include <Ext/Scenario/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/CaptureManager/Body.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Utilities/Macro.h>

#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>
#include <TeamClass.h>
#include <TeamTypeClass.h>
#include <AircraftTrackerClass.h>

static int GetMultiWeaponRange(TechnoClass* pThis)
{
	int range = -1;
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->MultiWeapon) {
		int selectCount = MinImpl(pType->WeaponCount, pTypeExt->MultiWeapon_SelectCount);
		range = 0;

		for (int index = selectCount - 1; index >= 0; --index) {
			int weaponRange = pThis->GetWeaponRange(index);

			if (weaponRange > range)
				range = weaponRange;
		}
	}

	return range;
}
//#pragma optimize("", off )
AbstractClass* __fastcall FakeTechnoClass::__Greatest_Threat(TechnoClass* pThis, discard_t, ThreatType method, CoordStruct* coord, bool onlyEnemy)
{
	++TechnoClass::TargetScanCounter();
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto pOwner = pThis->Owner;
	const bool isTechnoPlayerControlled = pOwner->IsControlledByHuman();
	const bool attackFriendlies = pThis->Veterancy.IsElite() ? pTypeExt->AttackFriendlies.Y : pTypeExt->AttackFriendlies.X;

	// Early exit for NoAutoFire units under player control
	if ((pType->NoAutoFire || (TechnoExtContainer::Instance.Find(pThis)->GetPassiveAcquireMode()) == PassiveAcquireModes::Ceasefire) && isTechnoPlayerControlled)
	{
		return nullptr;
	}

	bool AU = (ExtendedThreatType(method) & ExtendedThreatType::Underground) != ExtendedThreatType::none;

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
		| ThreatType::Buildings
		| ThreatType::Capture))
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
		auto pTransportType = GET_TECHNOTYPE(transport);

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
					|| attackFriendlies
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
				if (FakeTechnoClass::__EvaluateObjectB(pThis, method, threatBitfield, -1, aircraft, &evalThreat, ZoneType::None, &emptyCoord, AU))
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
				|| attackFriendlies
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
			if (FakeTechnoClass::__EvaluateObjectB(pThis, method, threatBitfield, -1, techno, &evalThreat, zone, coord, AU))
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
		if(!pType->HasTurret() || pType->IsGattling){
			if (pType->Underwater && pType->Organic && pType->SelfHealing) {
					weaponRange = pType->GuardRange;
				} else {
					if(int MultiWeaponrange = GetMultiWeaponRange(pThis); MultiWeaponrange != -1) {
					    weaponRange =MultiWeaponrange;
					} else {
						const int range0 = pThis->GetWeaponRange(0);
						const int range1 = pThis->GetWeaponRange(1);
						weaponRange = (range0 > range1) ? range0 : range1;
				}
			}
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
				|| attackFriendlies
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
			if (FakeTechnoClass::__EvaluateObjectB(pThis, method, threatBitfield | 0x8002, threatRange, aircraft, &evalThreat, ZoneType::None, &emptyCoord, AU))
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

		const bool targetFriendly = attackFriendlies || pThis->Berzerk || hasRealOwner || combatDamage < 0;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (auto pCurrent : ScenarioExtData::Instance()->FallingDownTracker)
		{
			if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
				&& (!onlyEnemy || pCurrent->Owner->ArrayIndex == pThis->Owner->EnemyHouseIndex)
				&& FakeTechnoClass::__EvaluateObjectB(pThis, method, threatBitfield, threatRange, pCurrent, &threatBuffer, ZoneType::None, &tempCrd, AU))
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
		const bool targetFriendly = attackFriendlies || pThis->Berzerk || hasRealOwner || combatDamage < 0;
		int threatBuffer = 0;
		auto tempCrd = CoordStruct::Empty;

		for (const auto pCurrent : ScenarioExtData::Instance()->UndergroundTracker)
		{

			if ((!pOwner->IsAlliedWith(pCurrent) || targetFriendly)
				&& (!onlyEnemy || pCurrent->Owner->ArrayIndex == pOwner->EnemyHouseIndex)
				&& FakeTechnoClass::__EvaluateObjectB(pThis, method, threatBitfield, threatRange, pCurrent, &threatBuffer, ZoneType::None, &tempCrd, AU))
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
			CellStruct topCell((short)(centerCell.X + x), (short)(centerCell.Y - radius));
			EvaluateCellAndUpdate(&topCell);

			// Bottom edge
			CellStruct bottomCell((short)(centerCell.X + x), (short)(centerCell.Y + radius));
			EvaluateCellAndUpdate(&bottomCell);
		}

		// Scan left and right edges (excluding corners already done)
		for (int y = 1 - radius; y < radius; ++y)
		{
			// Left edge
			CellStruct leftCell((short)(centerCell.X - radius), (short)(centerCell.Y + y));
			EvaluateCellAndUpdate(&leftCell);

			// Right edge
			CellStruct rightCell((short)(centerCell.X + radius), (short)(centerCell.Y + y));
			EvaluateCellAndUpdate(&rightCell);
		}

		// Early exit if we found a target at certain radii
		if (bestTarget && !RulesExtData::Instance()->DisableOveroptimizationInTargeting)
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

DEFINE_FUNCTION_JUMP(LJMP, 0x6F8DF0, FakeTechnoClass::__Greatest_Threat);
DEFINE_FUNCTION_JUMP(CALL, 0x4D9942, FakeTechnoClass::__Greatest_Threat);//foot
DEFINE_FUNCTION_JUMP(CALL, 0x445F68, FakeTechnoClass::__Greatest_Threat);//building
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D24, FakeTechnoClass::__Greatest_Threat);//technoVtable

static bool CanAttackMindControlled(TechnoClass* pControlled, TechnoClass* pRetaliator)
{
	const auto pMind = pControlled->MindControlledBy;

	if (!pMind || pRetaliator->Berzerk)
		return true;

	const auto pManager = pMind->CaptureManager;

	if (!pManager)
		return true;

	const auto pHome = pManager->GetOriginalOwner(pControlled);
	const auto pHouse = pRetaliator->Owner;

	if (!pHome || !pHouse || !pHouse->IsAlliedWith(pHome))
		return true;

	return TechnoExtContainer::Instance.Find(pControlled)->BeControlledThreatFrame <= Unsorted::CurrentFrame();
}

ASMJIT_PATCH(0x7089E8, TechnoClass_AllowedToRetaliate_AttackMindControlledDelay, 0x6)
{
	enum { CannotRetaliate = 0x708B17 };

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pAttacker, EBP);

	return CanAttackMindControlled(pAttacker, pThis) ? 0 : CannotRetaliate;
}

/**
 * Evaluates whether `target` is a valid threat candidate for the given techno.
 * Writes the computed priority value into `*value`.
 *
 * @param pThis              The evaluating techno (owner of this threat scan).
 * @param method             Bitmask of ThreatType flags controlling targeting behaviour.
 * @param mask               RTTI bitmask of allowed target types.
 * @param range              Maximum allowed distance in leptons (0 = use weapon range).
 * @param target             The candidate target to evaluate.
 * @param value              Output: computed threat priority value.
 * @param zone               Movement zone the target must belong to (ZoneType::None to skip).
 * @param coord              Coordinate hint forwarded to Coefficient_stuff_70CD10.
 * @param attackUnderground  When true, targets with height < -20 are not rejected.
 *                           Replaces the former SelectAutoTarget_Context::AttackUnderground static.
 *
 * @return true if `target` is a valid candidate; false to skip it.
 */
bool FakeTechnoClass::__EvaluateObjectB(
	TechnoClass* pThis,
	ThreatType   method,
	int          mask,
	int          range,
	ObjectClass* target,
	int* value,
	ZoneType     zone,
	CoordStruct* coord,
	bool         attackUnderground
)
{

	// =========================================================================
	// 6. Basic object validity
	// =========================================================================
	if (!target || target->InLimbo || !target->Health)
		return false;

	auto* pTechnoTarget = flag_cast_to<TechnoClass*, false>(target);
	auto* pFootTarget = flag_cast_to<FootClass*, false>(target);
	auto* pUnitTarget = cast_to<UnitClass*, false>(target);
	auto* pInfantryTarget = cast_to<InfantryClass*, false>(target);
	auto* pAircraftTarget = cast_to<AircraftClass*, false>(target);
	auto* pBuildingTarget = cast_to<BuildingClass*, false>(target);

	auto* pThisTechno = flag_cast_to<TechnoClass*, false>(pThis);
	auto* pThisFoot = flag_cast_to<FootClass*, false>(pThis);
	auto* pThisUnit = cast_to<UnitClass*, false>(pThis);
	auto* pThisInfantry = cast_to<InfantryClass*, false>(pThis);
	auto* pThisAircraft = cast_to<AircraftClass*, false>(pThis);
	auto* pThisBuilding = cast_to<BuildingClass*, false>(pThis);

	// =========================================================================
	// 1. Weapon / threat-range setup
	// =========================================================================
	const bool hasThreatRange = pThis->IsEngineer();
	const int  weaponIndex = pThis->SelectWeapon(target);

	// lastWeapon: replaces the former static `LastWeapon` global.
	// Established here and consumed by the Verses, AttackFriendlies,
	// AttackNoThreatBuildings, and AggressiveAttackMove checks below.
	WeaponTypeClass* lastWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	// =========================================================================
	// 2. Fire-error / movement check
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7CE2]  TechnoClass_EvaluateObject_DisallowMoving
	//   Extends the vanilla fire-error-5 check with three custom paths:
	//   • Immobile units (TechnoExtData::CannotMove) use their own GetFireError
	//     so they are not forced to move before firing.
	//   • Zero-speed infantry likewise use GetFireError directly.
	//   • Techno targets: enforce IronCurtain-retaliation rules via
	//     CanRetaliateICUnit; reject immediately on failure.
	//   All other cases fall through to the vanilla mTechnoClass_6FC090 call.
	// -------------------------------------------------------------------------
	if ((method & (ThreatType::TechBuildings | ThreatType::OccupiableBuildings | ThreatType::Capture))
			== ThreatType::Normal)
	{
		if (pTechnoTarget && !TechnoExtData::CanRetaliateICUnit(pThis,
				reinterpret_cast<FakeWeaponTypeClass*>(lastWeapon), pTechnoTarget)) {
			// IC-retaliation: reject immediately if we cannot fire at an
			// IronCurtained target with this weapon.
			return false;
		}

		bool ignoreRange = false;
		if ((pThisUnit && TechnoExtData::CannotMove(pThisUnit) )|| (pThisInfantry && pThisInfantry->Type->Speed <= 0)) {
			ignoreRange= true;
		}

		const FireError  fireError = pThis->GetFireError(target, weaponIndex, ignoreRange);

		if (fireError == FireError::ILLEGAL)
			return false;
	}

	TechnoTypeClass* pType = GET_TECHNOTYPE(pThis);
	const auto* pThisTypeExt = GET_TECHNOTYPEEXT(pThis);

	// =========================================================================
	// 3. VHP scan: skip dead targets when scanning for living ones
	// =========================================================================
	if (target->EstimatedHealth <= 0 && pType->VHPScan == 2)
		return false;

	// =========================================================================
	// 4. Armor / warhead verses check
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7D3D]  TechnoClass_EvaluateObject_Verses
	//   Replaces the vanilla `Modifier[armor] <= 0.02` threshold check with:
	//   • TechnoExtData::GetTechnoArmor for extended/custom armor type resolution.
	//   • The warhead's PassiveAcquire flag as the rejection criterion.
	// -------------------------------------------------------------------------
	FakeWeaponTypeClass* pFakeWeapon = nullptr;
	FakeWarheadTypeClass* pFakeWH = nullptr;

	if (lastWeapon && lastWeapon->Warhead)
	{
		pFakeWeapon = reinterpret_cast<FakeWeaponTypeClass*>(lastWeapon);

		if(lastWeapon->Warhead){
			pFakeWH = reinterpret_cast<FakeWarheadTypeClass*>(lastWeapon->Warhead);

			const Armor armor = TechnoExtData::GetTechnoArmor(target, pFakeWH);
			if (!pFakeWH->GetVersesData(armor)->Flags.PassiveAcquire)
				return false;
		}
	}

	// =========================================================================
	// 5. Land-targeting restriction (LandTargeting == 2 → water-only attacker)
	// =========================================================================
	if (pType->LandTargeting == LandTargetingType::Land_secondary
		&& target->IsOnFloor()
		&& target->GetCell()->LandType != LandType::Water)
	{
		return false;
	}

	// =========================================================================
	// 7. Cloaked-target visibility
	// =========================================================================
	if (pTechnoTarget && pTechnoTarget->CloakState == CloakState::Cloaked)
	{
		const CellClass* tCell = target->GetCell();
		if (!tCell->Sensors_InclHouse(pThis->Owner->ArrayIndex)
			&& pThis->Owner != pTechnoTarget->Owner)
			return false;
	}

	// =========================================================================
	// 8. Mission / lock / height checks
	// =========================================================================
	if(pTechnoTarget){
		if (!pTechnoTarget->IsInPlayfield)
			return false;

		if (pTechnoTarget->GetCurrentMissionControl()->NoThreat)
			return false;
	}

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7E1E]  TechnoClass_EvaluateObject_AU  (AttackUnderground)
	//   Vanilla unconditionally rejects targets with height < -20.
	//   With attackUnderground = true those targets are allowed through.
	//   `attackUnderground` is a function parameter, replacing the former
	//   static SelectAutoTarget_Context::AttackUnderground.
	// -------------------------------------------------------------------------
	if (target->GetHeight() < -20 && !attackUnderground)
		return false;

	// =========================================================================
	// 9. Movement-zone check
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7E24]  TechnoClass_EvaluateObject_MapZone
	//   Completely replaces the vanilla movement-zone check with
	//   TechnoExtData::AllowedTargetByZone, which additionally honours
	//   pThisTypeExt->TargetZoneScanType and per-weapon zone overrides.
	//   ZoneType::None (-1) is the "skip zone check" sentinel value.
	// -------------------------------------------------------------------------
	{
		if (!TechnoExtData::AllowedTargetByZone(pThis, target,
			pThisTypeExt->TargetZoneScanType, lastWeapon, zone))
			return false;
	}

	// =========================================================================
	// 10. Open-topped transport setup
	// =========================================================================
	bool        inOpenTopped = false;
	HouseClass* transportOwnerHouse = nullptr;

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7EC2]  TechnoClass_EvaluateObject_ThreatEvals_OpenToppedOwner
	//   When Passengers_SyncOwner is active on pThis unit's transport, ownership
	//   is synchronized elsewhere; the vanilla per-passenger owner check is
	//   skipped to avoid double-filtering.
	// -------------------------------------------------------------------------
	if(pTechnoTarget){
		if (TechnoClass* transport = pTechnoTarget->Transporter) {
			auto pTransporTypeExt = GET_TECHNOTYPEEXT(transport);
			auto pTransportType = GET_TECHNOTYPE(transport);

			const bool syncOwnerActive = pTransporTypeExt->Passengers_SyncOwner.Get();

			if (!syncOwnerActive && pTransportType->OpenTopped)
			{
				if (TechnoClass* origOwner = transport->MindControlledBy)
				{
					transportOwnerHouse = origOwner->Owner;
					inOpenTopped = true;
				}
			}
		}
	}

	// =========================================================================
	// 11. Allied-target filtering (berserk / open-topped / normal modes)
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F7EF4]  TechnoClass_EvaluateObject_AttackFriendlies
	//   Extends the vanilla AttackFriendlies flag with two overrides:
	//   1. Weapon-level AttackFriendlies (WeaponTypeExt) takes precedence over
	//      the type-level flag when lastWeapon is set.
	//   2. AttackFriendlies_WeaponIdx restricts the behaviour to a single
	//      weapon slot; other slots behave as if the flag is false.
	// -------------------------------------------------------------------------
	{
		bool  attackFriendlies = pType->AttackFriendlies;

		if (pFakeWeapon) {
			attackFriendlies = pFakeWeapon->_GetExtData()->AttackFriendlies.Get(attackFriendlies);
		}

		if (attackFriendlies
			&& pThisTypeExt->AttackFriendlies_WeaponIdx > -1
			&& pThisTypeExt->AttackFriendlies_WeaponIdx != weaponIndex)
		{
			attackFriendlies = false;
		}

		const bool bAttackingMode =
			(pThisInfantry && pThisInfantry->PermanentBerzerk)
			|| attackFriendlies
			|| pThis->Berzerk;

		if (bAttackingMode || inOpenTopped)
		{
			// Berserk / attack-all / open-topped: only filter on transport
			// owner's alliances, not pThis unit's own house.
			if (inOpenTopped && transportOwnerHouse
				&& transportOwnerHouse->IsAlliedWith(target))
				return false;
		}
		else
		{
			if (pThis->Owner->IsAlliedWith(target))
			{
				// Reject unless doing negative damage (repair) with threat-range active.
				if (pThis->CombatDamage(-1) >= 0 && !hasThreatRange)
					return false;

				// -----------------------------------------------------------------
				// [HOOK 0x6F7F4F]  TechnoClass_EvaluateObject_NegativeDamage
				//   Replaces the vanilla `Health_Ratio == ConditionGreen` check.
				//   For TechnoClass targets: IronCurtain status and FiringAllowed
				//   rules gate whether friendly fire is permitted.
				//   For non-Techno objects: reject at >= ConditionGreen health.
				// -----------------------------------------------------------------
				if (pTechnoTarget) {
					if (pTechnoTarget->IsIronCurtained() || !TechnoExtData::FiringAllowed(pThis, pTechnoTarget,pFakeWeapon))
						return false;

				} else {
					if (target->GetHealthPercentage_() >= RulesClass::Instance->ConditionGreen)
						return false;
				}

				if (pAircraftTarget && pThisUnit)
				{
					// Allied aircraft: reject if airborne or sitting over a building.
					if (target->GetHeight() > 0)
						return false;

					CoordStruct tmp;
					target->GetCoords(&tmp);

					if (MapClass::Instance->GetCellAt(tmp)->GetBuilding())
						return false;
				}
				else
				{
					// -----------------------------------------------------------------
					// [PATCH 0x6F7FC5 → 0x6F7FDF]  DEFINE_JUMP(LJMP, 0x6F7FC5, 0x6F7FDF)
					//   Vanilla code at loc_6F7FC5 checked Kind_Of() == RTTI_UNIT and
					//   only rejected the target when Is_Vehicle_Can_Undeploy() was
					//   false, allowing undeployable allies through so a unit could
					//   "nudge" them.  The patch jumps past the entire block, making
					//   a unit ALWAYS reject non-aircraft allied targets here,
					//   regardless of their undeploy capability.
					// -----------------------------------------------------------------
					return false;
				}
			}
		}
	}

	// =========================================================================
	// 12. Self-targeting guard
	// =========================================================================
	if (pThis == target)
		return false;

	// =========================================================================
	// 13. Harvester protection in special scenario mode (Scen->Specials bit 11)
	// =========================================================================
	if (ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune && pUnitTarget) {
		if (RulesClass::Instance->HarvesterUnit.contains(pUnitTarget->Type))
			return false;
	}

	// =========================================================================
	// 14. Distance calculation (2-D for airborne targets, 3-D otherwise)
	// =========================================================================
	const bool targetInAir = target->IsInAir();

	CoordStruct tPos, mPos, dist;
	target->GetCoords(&tPos);
	pThis->GetCoords(&mPos);

	dist = (tPos - mPos);

	const int distance = int(targetInAir ?
		dist.LengthXY() :// 2-D Euclidean distance: ignore altitude delta for aircraft.
		dist.Length());// 3-D Euclidean distance for ground / naval targets.

	// =========================================================================
	// 15. Range check
	// =========================================================================
	if (range > 0)
	{
		if (distance > range)
			return false;
	}
	else
	{
		// range == 0: fall back to weapon cell-range or the raw ThreatRange field.
		if (pThis->IsArmed())
		{
			if (!pThis->IsCloseEnough(target, weaponIndex))
				return false;
		}
		else if (distance > pType->GuardRange)
		{
			return false;
		}
	}

	// =========================================================================
	// 16. Player / campaign visibility restriction
	// =========================================================================
	if(pTechnoTarget){
		if (pThis->Owner->ControlledByCurrentPlayer()
			&& !pTechnoTarget->IsOwnedByCurrentPlayer
			&& !pTechnoTarget->DiscoveredByCurrentPlayer
			&& SessionClass::Instance->GameMode == GameMode::Campaign
			&& !pAircraftTarget) {
			return false;
		}
	}

	// Invisible-in-game buildings are never valid targets.
	if (pBuildingTarget && pBuildingTarget->Type->InvisibleInGame)
		return false;

	// =========================================================================
	// 17. RTTI mask check
	// =========================================================================
	const AbstractType targetKind = target->WhatAmI();

	if (((1 << (DWORD)targetKind) & mask) == 0
		&& ((mask & 2) == 0 || !target->IsStrange()))
	{
		return false;
	}

	// =========================================================================
	// 18. Legal-target flag
	// =========================================================================


	// -------------------------------------------------------------------------
	// [HOOK 0x6F8260]  TechnoClass_EvaluateObject_LegalTarget_AI
	//   Adds a per-type AI-specific IsLegalTarget override via TechnoTypeExt.
	//   When AI_LegalTarget is set and the controlling house is AI:
	//     • true  → treat as legal (vanilla IsLegalTarget flag skipped)
	//     • false → hard reject
	//   Human-controlled houses always use the vanilla flag.
	// -------------------------------------------------------------------------
	{

		if (pTechnoTarget)
		{
			const auto* pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoTarget->GetTechnoType());

			if(pTargetTypeExt->AI_LegalTarget.isset() && !pThis->Owner->IsControlledByHuman()){
				if (!pTargetTypeExt->AI_LegalTarget.Get())
				return false;
			} else if (!target->GetType()->LegalTarget) {
				return false;
			}
		}
		else if (!target->GetType()->LegalTarget) {
			return false;
		}
	}

	// =========================================================================
	// 19. Multiplayer-passive and insignificant-unit checks
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F833E / 0x6F845D]  TechnoClass_EvaluateObject_Garrisonable
	//   Applied identically at both lambda call sites (PATCH_AGAIN).
	//   Garrison targeting is only permitted when:
	//     • The method covers the full map (neither Range nor Area bit is set), OR
	//     • This unit is currently executing an attack-move order.
	// -------------------------------------------------------------------------
	const auto CanInfantryAttackPassiveBuilding = [&]() -> bool
		{
			if (pThis->Owner->IsControlledByHuman()
				|| !pBuildingTarget
				|| !pThisInfantry)
				return false;

			if (SessionClass::Instance->GameMode == GameMode::Campaign)
				return true;

			bool hasIsBaseDefenseteam = false;
			if (auto pTeam = pThis->OldTeam)
				hasIsBaseDefenseteam = pTeam->Type->IsBaseDefense;

			bool targetNeedsEngineer =
				pBuildingTarget->Type->NeedsEngineer
				&& pThisInfantry->Type->Engineer
				&& pBuildingTarget->Owner != pThisInfantry->Owner;

			bool canOccupy = pBuildingTarget->Type->CanBeOccupied
				&& pBuildingTarget->CanBeOccupyedBy(pThisInfantry);

			// [HOOK 0x6F833E / 0x6F845D] Garrisonable:
			// Restrict garrison targeting to full-map scans or attack-move orders.
			const bool isFullMapScan =
				(method & (ThreatType::Range | ThreatType::Area)) == ThreatType::Normal;
			canOccupy = canOccupy && (isFullMapScan || pThis->MegaMissionIsAttackMove());

			return  hasIsBaseDefenseteam || targetNeedsEngineer || canOccupy;
		};

	// -------------------------------------------------------------------------
	// [HOOK 0x6F826E]  TechnoClass_EvaluateObject_CivilianEnemy
	//   Fires after MultiplayPassive is confirmed; shared between both blocks.
	//   Returns true (ConsiderEnemy) in two cases:
	//   1. CivilianEnemy flag is set on the target type → always an enemy.
	//   2. The target is actively attacking an allied unit AND the appropriate
	//      AutoRepel(Player/AI) rule is enabled → treat as enemy to prevent
	//      civilians from overrunning a player uncontested.
	//   When true the entire passive/insignificant check is bypassed.
	//   Undecided (false) falls through to CanInfantryAttackPassiveBuilding.
	// -------------------------------------------------------------------------
	const auto IsForcedEnemy = [&]() -> bool
		{

			// Hard override: type is always an enemy regardless of passive/insignificant flag.
			if (GET_TECHNOTYPEEXT(pTechnoTarget)->CivilianEnemy)
				return true;

				// Auto-repel: passive unit is actively attacking an ally.
			if (const auto* pTargetTarget = flag_cast_to<TechnoClass*>(pTechnoTarget->Target)) {
				if (pThis->Owner->IsAlliedWith(pTargetTarget)) {
						const auto* pData = RulesExtData::Instance();
						const bool autoRepel = pThis->Owner->IsControlledByHuman()
							? pData->AutoRepelPlayer
							: pData->AutoRepelAI;

						if (autoRepel)
							return true;
				}
			}

			return false;
		};

	// Multiplayer-passive house: only engineers/garrisoners or forced enemies allowed.
	if (SessionClass::Instance->GameMode != GameMode::Campaign && pTechnoTarget && pTechnoTarget->Owner->Type->MultiplayPassive)
	{
		if (!IsForcedEnemy() && !CanInfantryAttackPassiveBuilding())
			return false;
	}

	// Insignificant units (e.g. civilian vehicles): same rules apply.
	// IsForcedEnemy() is re-evaluated because a purely insignificant target
	// (non-passive house) still needs the check independently.
	if (pTechnoTarget && pTechnoTarget->GetTechnoType()->Insignificant
		&& !pTechnoTarget->MindControlledBy
		&& !pTechnoTarget->MindControlledByAUnit
		&& (!pBuildingTarget || pBuildingTarget->Owner->Type->MultiplayPassive))
	{
		if (!IsForcedEnemy() && !CanInfantryAttackPassiveBuilding())
			return false;
	}

	// =========================================================================
	// 20. Train-passenger restriction
	// =========================================================================
	if (pUnitTarget && pUnitTarget->Type->IsTrain
		&& pThisInfantry
		&& pThisInfantry->Type->VehicleThief)
	{
		return false;
	}

	// =========================================================================
	// 21. Disguise / mirage detection
	// =========================================================================
	if (pTechnoTarget && pTechnoTarget->IsDisguisedAs(pThis->Owner) && !pType->DetectDisguise) {

		if (!pTechnoTarget->DisguiseBlinkTimer.GetTimeLeft()
			|| pThis->Owner->IsControlledByHuman()
			|| ScenarioClass::Instance->Random.RandomRanged(0, 99)
			   > RulesClass::Instance->DisabledDisguiseDetectionPercent[(int)pThis->Owner->AIDifficulty])
		{
			return false;
		}
	}

	// =========================================================================
	// 22. Civilian-threat type: mark owner then reject
	// =========================================================================
	if ((method & ThreatType::Civilians) != ThreatType::Normal)
	{
		// ????
		target->GetOwningHouse();
		return false;
	}

	// =========================================================================
	// 23. Capture flag: target must be a capturable building
	//     0x200 → ThreatType::Capture
	// =========================================================================
	if ((method & ThreatType::Capture) != ThreatType::Normal
		&& (!pBuildingTarget || !pBuildingTarget->Type->Capturable))
	{
		return false;
	}

	// =========================================================================
	// 24. Target viability / "can fight back" filter
	// =========================================================================
	{
		const bool isFoot = (pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;
		bool isViable = (isFoot && pThis->QueuedMission != Mission::Sleep);

		if (!isViable)
		{
			// -----------------------------------------------------------------
			// [HOOK 0x6F85AB]  TechnoClass_EvaluateObject_AggressiveAttackMove
			//   During an attack-move order with aggressive config, or for units
			//   with IsStrange(), or when a FakeEngineer warhead would affect
			//   the target type: bypass the player-control/building-weapon filter
			//   and treat the target as viable.
			// -----------------------------------------------------------------
			bool aggressiveBypass = false;

			if (pThis->MegaMissionIsAttackMove())
			{
				if (pThisTypeExt->AttackMove_Aggressive.Get(
					RulesExtData::Instance()->AttackMove_UpdateTarget))
					aggressiveBypass = true;
			}

			if (!aggressiveBypass && pThis->IsStrange())
				aggressiveBypass = true;

			if (!aggressiveBypass)
			{
				AffectedTechno affectedTypes =
					AffectedTechno::Aircraft | AffectedTechno::Infantry | AffectedTechno::Unit;
				if (lastWeapon)
				{
					auto* pWHExt = WarheadTypeExtContainer::Instance.Find(lastWeapon->Warhead);
					if (pWHExt->IsFakeEngineer)
						affectedTypes |= AffectedTechno::Building;
				}
				if (EnumFunctions::CanAffectTechnoResult(target->WhatAmI(), affectedTypes))
					aggressiveBypass = true;
			}

			if (aggressiveBypass)
			{
				isViable = true;
			}
			else
			{
				// Vanilla viability path.
				if (!pThis->Owner->IsControlledByHuman()
					|| target->IsStrange()
					|| !pBuildingTarget)
				{
					isViable = true;
				}
				else
				{
					// Player-controlled attacker vs. non-undeployable building:
					// ---------------------------------------------------------
					// [HOOK 0x6F85CF]  TechnoClass_EvaluateObject_AttackNoThreatBuildings
					//   Rules/weapon-level flags can force targeting of unarmed
					//   buildings (e.g. power plants, refineries) before the
					//   vanilla turret-weapon + Risk() check.
					// ---------------------------------------------------------
					bool canAttackNoThreat =
						RulesExtData::Instance()->AutoTarget_NoThreatBuildings;

					if (lastWeapon)
					{
						auto* pWeapExt = WeaponTypeExtContainer::Instance.Find(lastWeapon);
						canAttackNoThreat =
							pWeapExt->AttackNoThreatBuildings.Get(canAttackNoThreat);
					}

					if (canAttackNoThreat)
					{
						isViable = true;
					}
					else if(pTechnoTarget)
					{
						// Vanilla: viable only if building has an armed turret with Risk.
						WeaponStruct* turret = pTechnoTarget->GetTurrentWeapon();

						if (turret && turret->WeaponType && pTechnoTarget->GetThreatValue())
							isViable = true;
					}
				}
			}
		}

		if (!isViable)
		{
			if (!hasThreatRange)
				return false;

			// hasThreatRange mode: still reject non-buildings and allied
			// buildings that are healthy or free to rebuild.
			if (!pBuildingTarget
				|| (pThis->Owner->IsAlliedWith(pBuildingTarget->Owner)
					&& (target->GetHealthPercentage() > RulesClass::Instance->ConditionRed
						|| !pBuildingTarget->Type->GetActualCost(pThis->Owner))))
			{
				return false;
			}
		}
	}

	// =========================================================================
	// Past all eligibility checks → compute and adjust threat score
	// =========================================================================

	// =========================================================================
	// 25. Tiberium-storage flag
	//     0x40 → ThreatType::Tiberium
	// =========================================================================
	if (pTechnoTarget && (method & ThreatType::Tiberium) != ThreatType::Normal && !pTechnoTarget->GetTechnoType()->Storage)
		return false;

	// =========================================================================
	// 26. Bridge-layer elevation mismatch
	//     Reject cross-bridge-layer targeting (both cells flagged, different layers).
	// =========================================================================
	{
		const CellClass* myCell = MapClass::Instance->GetCellAt(pThis->Location);
		const CellClass* tgtCell = MapClass::Instance->GetCellAt(target->Location);

		if ((myCell->ContainsBridge()) &&
			(tgtCell->ContainsBridge()) &&
			 pThis->OnBridge != target->OnBridge)
			return false;
	}

	// =========================================================================
	// 27. Base threat coefficient
	// =========================================================================
	*value = static_cast<int>(pThis->ThreatCoeffients(target, coord));

	// =========================================================================
	// 28. VHP scan adjustment
	//     VHPScan == 1: reward damaged targets, penalise already-dead ones.
	// =========================================================================
	if (pType->VHPScan == 1 && pTechnoTarget)
	{
		if (target->EstimatedHealth <= 0)
			*value /= 2;
		else if (target->EstimatedHealth <= pTechnoTarget->GetTechnoType()->Strength / 2)
			*value *= 2;
	}

	// =========================================================================
	// 29. Hunt-specific-enemy reduction
	//     When AllToHunt is active and target is not the designated enemy house,
	//     lower the score to 1 so the primary enemy is always preferred.
	// =========================================================================
	{
		HouseClass* myHouse = pThis->Owner;
		if (myHouse->AllToHunt && pTechnoTarget)
		{
			const int enemyID = myHouse->EnemyHouseIndex;
			if (enemyID != -1 && pTechnoTarget->Owner != HouseClass::Array->Items[enemyID])
				*value = 1;
		}
	}

	// =========================================================================
	// 30. Mind-control attack delay check
	// =========================================================================

	// -------------------------------------------------------------------------
	// [HOOK 0x6F88BF]  TechnoClass_EvaluateObject_AttackMindControlledDelay
	//   Rejects mind-controlled targets that cannot yet be attacked due to the
	//   configurable mind-control attack delay.  Evaluated before value
	//   adjustments to avoid scoring targets we will reject anyway.
	// -------------------------------------------------------------------------
	if (pTechnoTarget)
	{
		if (!CanAttackMindControlled(pTechnoTarget, pThis))
			return false;
	}

	// =========================================================================
	// 31. Power-plant bonus
	//     0x800 → ThreatType::PowerFacilties
	// =========================================================================
	if ((method & ThreatType::PowerFacilties) != ThreatType::Normal)
	{
		if (pBuildingTarget) {
			int totalPower = 0;

			for (const auto type : pBuildingTarget->GetTypes()) {
				if (type) {
					totalPower += BuildingTypeExtData::GetEnhancedPower(type, type->PowerBonus, pBuildingTarget->Owner);
				}
			}

			if (totalPower <= 0)
				*value = 0;
			else
				*value += 1000 * totalPower;
		}
	}

	// =========================================================================
	// 32. Occupant-slot bonus
	//     0x8000 → ThreatType::OccupiableBuildings  (was SBYTE1(method) < 0)
	// =========================================================================
	if ((method & ThreatType::OccupiableBuildings) != ThreatType::Normal)
	{
		if (!pBuildingTarget) return false;
		const int maxOccupants = pBuildingTarget->Type->MaxNumberOccupants;
		if (maxOccupants <= 0) return false;
		*value += 1000 * maxOccupants;
	}

	// =========================================================================
	// 33. Tech-building bonus
	//     0x10000 → ThreatType::TechBuildings
	// =========================================================================
	if ((method & ThreatType::TechBuildings) != ThreatType::Normal)
	{
		if (!pBuildingTarget || !pBuildingTarget->Type->NeedsEngineer) return false;
		*value += 1000;
	}

	// =========================================================================
	// 34. Zero-out if building produces nothing (ToBuild == RTTI_NONE)
	//     0x1000 → ThreatType::Factories
	// =========================================================================
	if ((method & ThreatType::Factories) != ThreatType::Normal
		&& pBuildingTarget
		&& pBuildingTarget->Type->Factory == AbstractType::None)
	{
		*value = 0;
	}

	// =========================================================================
	// 35. Occupiable / armed-building check for infantry garrison targeting
	//     0x2000 → ThreatType::Base_defenses
	//     Note: despite the enum name, pThis flag path governs garrison/occupy
	//     targeting rather than base-defence structures specifically.
	// =========================================================================
	if ((method & ThreatType::Base_defenses) != ThreatType::Normal) {
		if (pBuildingTarget && pBuildingTarget->Type->CanBeOccupied) {
			if (!pBuildingTarget->GetOccupantCount())
				return false;
		} else if (pTechnoTarget) {
			WeaponStruct* primaryWeapon = pTechnoTarget->GetPrimaryWeapon();
			if (!primaryWeapon || !primaryWeapon->WeaponType || !pBuildingTarget)
				return false;
		}
	}

	// =========================================================================
	// 36. Area modifier
	// =========================================================================
	{
		CoordStruct finalCoord = target->GetCoords();
		CellStruct cellForArea = CellClass::Coord2Cell(finalCoord);

		const double areaModifier = pThis->ShouldSuppress(&cellForArea);
		if (areaModifier != 1.0)
			*value = static_cast<int>(*value * areaModifier);
	}

	// =========================================================================
	// 37. Final value clamping
	// =========================================================================
	if (*value == 0)
	{
		*value = 0;
		return false;
	}

	if (*value < 1)
		*value = 1;

	return true;
}

bool __fastcall FakeTechnoClass::__EvaluateObject(
	TechnoClass* pThis,
	discard_t ,
	ThreatType targetFlags,
	int mask,
	int wantedDistance,
	ObjectClass* pTarget,
	int* pThreatPosed,
	ZoneType dwUnk,
	CoordStruct* pSourceCoords)
{
	return FakeTechnoClass::__EvaluateObjectB(pThis, targetFlags, mask, wantedDistance, pTarget, pThreatPosed, dwUnk, pSourceCoords,false);
}
//#pragma optimize("", on )

DEFINE_FUNCTION_JUMP(CALL, 0x6F8C00, FakeTechnoClass::__EvaluateObject);
DEFINE_FUNCTION_JUMP(LJMP, 0x6F7CA0, FakeTechnoClass::__EvaluateObject);

ASMJIT_PATCH(0x6F90F8, TechnoClass_SelectAutoTarget_Demacroize, 6)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x6F9116;
}

ASMJIT_PATCH_AGAIN(0x6F8F1F, TechnoClass_SelectAutoTarget_Heal, 6)
ASMJIT_PATCH(0x6F8EE3, TechnoClass_SelectAutoTarget_Heal, 6)
{
	GET(unsigned int, nVal, EBX);

	nVal |= 0x403Cu;

	R->EBX(nVal);
	return 0x6F8F25;
}