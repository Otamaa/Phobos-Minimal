#include "HunterSeeker.h"

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/SWType/Body.h>

std::vector<const char*> SW_HunterSeeker::GetTypeString() const
{
	return { "HunterSeeker" };
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	HouseClass* pOwner = pThis->Owner;
	auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);

	// get the appropriate hunter seeker type
	UnitTypeClass* pType = pExt->HunterSeeker_Type.Get(HouseExtData::GetHunterSeeker(pOwner));

	// no type found
	if (!pType)
	{
		Debug::LogInfo("HunterSeeker super weapon \"{}\" could not be launched. "
			"No HunterSeeker unit type set for house \"{}\".", pThis->Type->ID, pOwner->Type->ID);
		return false;
	}

	// the maximum number of buildings to fire. negative means all.
	const auto Count = (pExt->SW_MaxCount >= 0)
		? static_cast<size_t>(pExt->SW_MaxCount)
		: std::numeric_limits<size_t>::max();

	// only call on up to Count buildings that suffice IsEligible
	// create hunterseeker regardless building alive state
	// only check if cell is actually eligible
	size_t Success = 0;
	Helpers::Alex::for_each_if_n(pOwner->Buildings.begin(), pOwner->Buildings.end(),
		Count,
		[=](BuildingClass* pBld) { return this->IsLaunchSite_HS(pExt, pBld); },
		[=, &Success](BuildingClass* pBld) { auto cell = this->GetLaunchCell(pExt, pBld, pType);

			if (cell == CellStruct::Empty) {
				return;
			}

			// create a hunter seeker
			if (auto pHunter = (UnitClass*)pType->CreateObject(pOwner))
			{
				TechnoExtContainer::Instance.Find(pHunter)->LinkedSW = pThis;

				// put it on the map and let it go

				if (pHunter->Unlimbo(CellClass::Cell2Coord(cell), DirType::East))
				{
					pHunter->Locomotor->Acquire_Hunter_Seeker_Target();
					pHunter->QueueMission((pHunter->Type->Harvester || pHunter->Type->ResourceGatherer) ? Mission::Area_Guard : Mission::Attack, false);
					pHunter->NextMission();
					++Success;
				}
				else
				{
					GameDelete<true, false>(pHunter);
				}
			}
		});

	// no launch building found
	if (!Success) {
		Debug::LogInfo("HunterSeeker super weapon \"{}\" could not be launched. House \"{}\" "
			"does not own any HSBuilding or No Buildings attached with this HSType Superweapon.", pThis->Type->ID, pOwner->Type->ID);
	}

	return Success != 0;
}

void SW_HunterSeeker::Initialize(SWTypeExtData* pData)
{	// Defaults to HunterSeeker values
	pData->SW_MaxCount = 1;

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_HunterSeekerDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_HunterSeekerReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_HunterSeekerLaunched");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::HunterSeeker;
	pData->SW_AffectsHouse = AffectedHouse::Enemies;

	pData->Text_Ready = CSFText("TXT_RELEASE");

	// hardcoded
	pData->AttachedToObject->Action = Action::None;
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

	// hardcoded
	pData->AttachedToObject->Action = Action::None;
	pData->SW_RadarEvent = false;
}

bool SW_HunterSeeker::IsLaunchSite_HS(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	// don't further question the types in this list
		// get the appropriate launch buildings list
	const auto HSBuilding = !pData->HunterSeeker_Buildings.empty()
		? make_iterator(pData->HunterSeeker_Buildings) : make_iterator(RulesExtData::Instance()->HunterSeekerBuildings);

	if (HSBuilding.contains(pBuilding->Type))
		return true;

	// added new tag so it wont break the default behaviour
	if (pData->HunterSeeker_AllowAttachedBuildingAsFallback)
		return this->IsSWTypeAttachedToThis(pData, pBuilding);

	return false;
}

bool SW_HunterSeeker::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	return this->IsLaunchSite_HS(pData , pBuilding);
}

CellStruct SW_HunterSeeker::GetLaunchCell(SWTypeExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const
{
	const auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
	CellStruct cell;

	if (pBldExt->LimboID != -1)
	{
		//Get edge (direction for hunterseeker to come from)
		const auto edge = pBuilding->Owner->GetHouseEdge();

		// seems to retrieve a random cell struct at a given edge
		cell = MapClass::Instance->PickCellOnEdge(
			edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Foot, true,
			MovementZone::Normal);

	} else {

		auto position = CellClass::Coord2Cell(pBuilding->GetCoords());

		cell = MapClass::Instance->NearByLocation(position, SpeedType::Foot,
			ZoneType::None, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
	}

	return MapClass::Instance->IsWithinUsableArea(cell, true) ? cell : CellStruct::Empty;
}
