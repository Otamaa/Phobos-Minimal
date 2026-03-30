#include "HunterSeeker.h"

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Helpers.h>

// ---------------------------------------------------------------------------
// Activate — launches hunter seeker units from eligible buildings
//
// For each eligible building owned by the house (up to SW_MaxCount
// successful launches), the code:
//   1. Checks if the building is a valid launch site (type match)
//   2. Resolves a launch cell (from building position, or map edge if limbo)
//   3. Creates the hunter seeker unit and places it on the map
//   4. Assigns it the Acquire_Hunter_Seeker_Target locomotor behavior
//
// Only successful launches count against the SW_MaxCount budget.
// Buildings that fail at any step (bad cell, Unlimbo failure, etc.)
// are skipped without consuming a launch slot.
// ---------------------------------------------------------------------------
bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	HouseClass* pOwner = pThis->Owner;
	auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);

	// Get the hunter seeker unit type — falls back to house default
	UnitTypeClass* pType = pExt->HunterSeeker_Type.Get(HouseExtData::GetHunterSeeker(pOwner));

	if (!pType)
	{
		Debug::LogInfo("HunterSeeker super weapon \"{}\" could not be launched. "
			"No HunterSeeker unit type set for house \"{}\".", pThis->Type->ID, pOwner->Type->ID);
		return false;
	}

	// The maximum number of *successful* launches. Negative means unlimited.
	const auto MaxCount = (pExt->SW_MaxCount >= 0)
		? static_cast<size_t>(pExt->SW_MaxCount)
		: std::numeric_limits<size_t>::max();

	size_t Success = 0;

	for (auto pBld : pOwner->Buildings)
	{
		// Budget reached — stop looking
		if (Success >= MaxCount)
			break;

		// Check if this building is a valid launch site (type match only)
		if (!this->IsLaunchSite_HS(pExt, pBld))
			continue;

		// Resolve launch cell — can fail for spatial reasons
		auto cell = this->GetLaunchCell(pExt, pBld, pType);
		if (cell == CellStruct::Empty)
			continue; // skip, don't count against budget

		// Create the hunter seeker unit
		auto pHunter = static_cast<UnitClass*>(pType->CreateObject(pOwner));
		if (!pHunter)
			continue;

		TechnoExtContainer::Instance.Find(pHunter)->LinkedSW = pThis;

		// Place it on the map
		if (pHunter->Unlimbo(CellClass::Cell2Coord(cell), DirType::East))
		{
			pHunter->Locomotor->Acquire_Hunter_Seeker_Target();
			pHunter->QueueMission(
				(pHunter->Type->Harvester || pHunter->Type->ResourceGatherer)
					? Mission::Area_Guard
					: Mission::Attack,
				false);
			pHunter->NextMission();
			++Success; // only count successful launches
		}
		else
		{
			GameDelete<true, false>(pHunter);
			// Unlimbo failed — skip, don't count against budget
		}
	}

	if (!Success)
	{
		Debug::LogInfo("HunterSeeker super weapon \"{}\" could not be launched. House \"{}\" "
			"does not own any HSBuilding or No Buildings attached with this HSType Superweapon.",
			pThis->Type->ID, pOwner->Type->ID);
	}

	return Success != 0;
}

void SW_HunterSeeker::Initialize(SWTypeExtData* pData)
{
	// Defaults to HunterSeeker values
	pData->SW_MaxCount = 1;

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_HunterSeekerDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_HunterSeekerReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_HunterSeekerLaunched");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::HunterSeeker;
	pData->SW_AffectsHouse = AffectedHouse::Enemies;

	pData->Text_Ready = CSFText("TXT_RELEASE");

	// Hardcoded — hunter seekers auto-fire, no player interaction
	pData->This()->Action = Action::None;
	pData->SW_RadarEvent = false;
}

void SW_HunterSeeker::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);

	pData->HunterSeeker_Type.Read(exINI, section, "HunterSeeker.Type");
	pData->HunterSeeker_RandomOnly.Read(exINI, section, "HunterSeeker.RandomOnly");
	pData->HunterSeeker_Buildings.Read(exINI, section, "HunterSeeker.Buildings");
	pData->HunterSeeker_AllowAttachedBuildingAsFallback.Read(exINI, section, "HunterSeeker.AllowAttachedBuildingAsFallback");

	// Hardcoded — always enforced regardless of INI
	pData->This()->Action = Action::None;
	pData->SW_RadarEvent = false;
}

// ---------------------------------------------------------------------------
// IsLaunchSite_HS — checks if a building is a valid hunter seeker launch type
//
// Checks against the HunterSeeker.Buildings list (or the rules default).
// If HunterSeeker.AllowAttachedBuildingAsFallback is set, also accepts
// buildings that have this SW type directly attached to them.
// ---------------------------------------------------------------------------
bool SW_HunterSeeker::IsLaunchSite_HS(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	// Check the explicit launch buildings list (SW-specific or rules default)
	const auto HSBuilding = !pData->HunterSeeker_Buildings.empty()
		? make_iterator(pData->HunterSeeker_Buildings)
		: make_iterator(RulesExtData::Instance()->HunterSeekerBuildings);

	if (HSBuilding.contains(pBuilding->Type))
		return true;

	// Fallback: accept buildings that have this SW type attached directly
	if (pData->HunterSeeker_AllowAttachedBuildingAsFallback)
		return this->IsSWTypeAttachedToThis(pData, pBuilding);

	return false;
}

// IsLaunchSite — full eligibility check (alive + type match)
bool SW_HunterSeeker::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	return this->IsLaunchSite_HS(pData, pBuilding);
}

// ---------------------------------------------------------------------------
// GetLaunchCell — resolves the cell where the hunter seeker will spawn
//
// Two cases:
//   - Limbo buildings (LimboID >= 0): the building isn't physically on the
//     map, so spawn the unit from a random map edge cell instead
//   - Normal buildings: find a walkable cell near the building's position
//
// Returns CellStruct::Empty if no usable cell is found.
// ---------------------------------------------------------------------------
CellStruct SW_HunterSeeker::GetLaunchCell(SWTypeExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const
{
	const auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
	CellStruct cell;

	if (pBldExt->LimboID >= 0)
	{
		// Limbo building — pick a random edge cell for the seeker to emerge from
		const auto edge = pBuilding->Owner->GetHouseEdge();

		cell = MapClass::Instance->PickCellOnEdge(
			edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Foot, true,
			MovementZone::Normal);
	}
	else
	{
		// Normal building — find a nearby walkable cell
		auto position = CellClass::Coord2Cell(pBuilding->GetCoords());

		cell = MapClass::Instance->NearByLocation(
			position, SpeedType::Foot,
			ZoneType::None, MovementZone::Normal,
			false, 1, 1, false, false, false, true,
			CellStruct::Empty, false, false);
	}

	return MapClass::Instance->IsWithinUsableArea(cell, true) ? cell : CellStruct::Empty;
}