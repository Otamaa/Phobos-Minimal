#include "HunterSeeker.h"

#include <Misc/AresData.h>
#include <Ext/Techno/Body.h>
#include <Ext/Rules/Body.h>

std::vector<const char*> SW_HunterSeeker::GetTypeString() const
{
	return { "HunterSeeker" };
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	HouseClass* pOwner = pThis->Owner;
	auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);

	// get the appropriate hunter seeker type
	UnitTypeClass* pType = pExt->HunterSeeker_Type.Get(HouseExt::GetHunterSeeker(pOwner));

	// no type found
	if (!pType)
	{
		Debug::Log("HunterSeeker super weapon \"%s\" could not be launched. "
			"No HunterSeeker unit type set for house \"%ls\".\n", pThis->Type->ID, pOwner->UIName);
		return false;
	}

	// get the appropriate launch buildings list
	auto HSBuilding = pExt->HunterSeeker_Buildings.size()  
		? &pExt->HunterSeeker_Buildings : &RulesExt::Global()->HunterSeekerBuildings;

	// the maximum number of buildings to fire. negative means all.
	const auto Count = (pExt->SW_MaxCount >= 0)
		? static_cast<size_t>(pExt->SW_MaxCount)
		: std::numeric_limits<size_t>::max();

	// only call on up to Count buildings that suffice IsEligible
	size_t Success = 0;
	Helpers::Alex::for_each_if_n(pOwner->Buildings.begin(), pOwner->Buildings.end(),
		Count, [&HSBuilding](BuildingClass* pBld) { return HSBuilding->Contains(pBld->Type); },
			   [=, &Success](BuildingClass* pBld)
	{
		auto cell = this->GetLaunchCell(pExt, pBld, pType);

		if (cell == CellStruct::Empty) {
			return;
		}

		// Otama : something is change here ? alex seems creating new function for these .//
		// create a hunter seeker
		if (auto pHunter = static_cast<UnitClass*>(pType->CreateObject(pOwner)))
		{
			if(pHunter->Type->HunterSeeker)
				TechnoExt::ExtMap.Find(pHunter)->LinkedSW = pThis;

			// put it on the map and let it go
			CoordStruct crd = CellClass::Cell2Coord(cell);

			if (pHunter->Unlimbo(crd, DirType::East))
			{
				pHunter->Locomotor->Acquire_Hunter_Seeker_Target();
				pHunter->QueueMission(pHunter->Type->Harvester ? Mission::Area_Guard : Mission::Attack, false);
				pHunter->NextMission();
				++Success;
			}
			else
			{
				GameDelete(pHunter);
			}
		}
	});

	// no launch building found
	if (!Success)
	{
		Debug::Log("HunterSeeker super weapon \"%s\" could not be launched. House \"%ls\" "
			"does not own any HSBuilding (%u types set).\n", pThis->Type->ID, pOwner->UIName, HSBuilding->size());
	}

	return Success != 0;
}

void SW_HunterSeeker::Initialize(SWTypeExt::ExtData* pData)
{	// Defaults to HunterSeeker values
	pData->SW_MaxCount = 1;

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_HunterSeekerDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_HunterSeekerReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_HunterSeekerLaunched");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::HunterSeeker;
	pData->SW_AffectsHouse = AffectedHouse::Enemies;

	pData->Text_Ready = CSFText("TXT_RELEASE");

	// hardcoded
	pData->Get()->Action = Action::None;
	pData->SW_RadarEvent = false;
}

void SW_HunterSeeker::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);

	pData->HunterSeeker_Type.Read(exINI, section, "HunterSeeker.Type");
	pData->HunterSeeker_RandomOnly.Read(exINI, section, "HunterSeeker.RandomOnly");
	pData->HunterSeeker_Buildings.Read(exINI, section, "HunterSeeker.Buildings");

	// hardcoded
	pData->Get()->Action = Action::None;
	pData->SW_RadarEvent = false;
}

CellStruct SW_HunterSeeker::GetLaunchCell(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding, UnitTypeClass* pHunter) const
{
	auto position = CellClass::Coord2Cell(pBuilding->GetCoords());

	auto cell = MapClass::Instance->NearByLocation(position, pHunter->SpeedType,
		-1, pHunter->MovementZone, false, 1, 1, false, false, false, true,
		CellStruct::Empty, false, false);

	return MapClass::Instance->IsWithinUsableArea(cell, true) ? cell : CellStruct::Empty;
}