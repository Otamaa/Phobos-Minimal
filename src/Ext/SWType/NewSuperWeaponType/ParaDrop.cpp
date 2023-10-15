#include "ParaDrop.h"

#include <Utilities/Helpers.h>
#include <Ext/House/Body.h>
#include <Ext/Aircraft/Body.h>

std::vector<const char*> SW_ParaDrop::GetTypeString() const
{
	return { "NewParaDrop" };
}

bool SW_ParaDrop::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ParaDrop) || (type == SuperWeaponType::AmerParaDrop);
}

bool SW_ParaDrop::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	if (pThis->IsCharged)
	{
		auto pTarget = MapClass::Instance->TryGetCellAt(Coords);
		// all set. send in the planes.
		return this->SendParadrop(pThis, pTarget);
	}

	return false;
}

void SW_ParaDrop::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::ParaDrop;
	// default for american paradrop
	if (pData->AttachedToObject->Type == SuperWeaponType::AmerParaDrop)
	{
		pData->AttachedToObject->Action = Action::AmerParaDrop;
		// the American paradrop will be the same for every country,
		// thus we use the SW's default here.
		auto& nData = pData->ParaDropDatas[pData->AttachedToObject];
		nData.push_back(std::move(std::make_unique<ParadropData>()));
		auto& pPlane = nData.back();

		auto const& Inf = RulesClass::Instance->AmerParaDropInf;
		pPlane->Types.insert(pPlane->Types.end(), Inf.begin(), Inf.end());

		auto const& Num = RulesClass::Instance->AmerParaDropNum;
		pPlane->Num.insert(pPlane->Num.end(), Num.begin(), Num.end());
	}

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_ReinforcementsReady);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = int(MouseCursorType::ParaDrop);
}

void SW_ParaDrop::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	char base[0x40];

	auto CreateParaDropBase = [](char* pID, auto& Buffer)
	{
		// put a string like "Paradrop.Americans" into the buffer
		if (pID && strlen(pID)) {
			IMPL_SNPRNINTF(Buffer, _TRUNCATE, "ParaDrop.%s", pID);
		}
		else {
			PhobosCRT::strCopy(Buffer, "ParaDrop");
		}
	};

	auto ParseParaDrop = [section, &exINI](char* pID, int Plane) -> std::unique_ptr<ParadropData>
	{
		auto pPlane = std::make_unique<ParadropData>();

		// create the plane part of this request. this will be
		// an empty string for the first plane for this is the default.
		char plane[0x10] = "";
		if (Plane)
		{
			IMPL_SNPRNINTF(plane, _TRUNCATE, ".Plane%d", Plane + 1);
		}

		// construct the full tag name base
		char base[0x40];
		IMPL_SNPRNINTF(base, _TRUNCATE, "%s%s", pID, plane);

		// parse the plane contents
		char key[0x40];
		IMPL_SNPRNINTF(key, _TRUNCATE, "%s.Aircraft", base);
		pPlane->Aircraft.Read(exINI, section, key);

		// a list of UnitTypes and InfantryTypes
		IMPL_SNPRNINTF(key, _TRUNCATE, "%s.Types", base);
		pPlane->Types.Read(exINI, section, key);

		// remove all types that cannot paradrop
		Helpers::Alex::remove_non_paradroppables(pPlane->Types, section, key);

		// don't parse nums if there are no types
		if (!pPlane->Aircraft && pPlane->Types.empty())
		{
			return nullptr;
		}

		// the number how many times each item is created
		IMPL_SNPRNINTF(key, _TRUNCATE, "%s.Num", base);
		pPlane->Num.Read(exINI, section, key);

		return pPlane;
	};

	auto GetParadropPlane = [=, &base](char const* pID, size_t defaultCount, AbstractTypeClass* pKey)
	{

		auto& ParaDrop = pData->ParaDropDatas[pKey];
		auto const lastCount = ParaDrop.size() ? ParaDrop.size() : defaultCount;

		// get the number of planes for this house or side
		char key[0x40];
		IMPL_SNPRNINTF(key, _TRUNCATE, "%s.Count", pID);
		auto const count = pINI->ReadInteger(section, key, lastCount);

		// parse every plane
		ParaDrop.resize(static_cast<size_t>(count));
		for (int i = 0; i < count; ++i)
		{
			if (auto pPlane = ParseParaDrop(base, i))
			{
				ParaDrop[i] = std::move(pPlane);
			}
		}
	};

	// now load the paradrops
	// 0: default
	// 1 to n: n sides
	// n+1 to n+m+1: m countries

	// default
	CreateParaDropBase(nullptr, base);
	GetParadropPlane(base, 1, pData->AttachedToObject);

	// put all sides into the hash table
	for (auto const& pSide : *SideClass::Array)
	{
		CreateParaDropBase(pSide->ID, base);
		GetParadropPlane(base, pData->ParaDropDatas[pData->AttachedToObject].size(), pSide);
	}

	// put all countries into the hash table
	for (auto const& pTHouse : *HouseTypeClass::Array)
	{
		CreateParaDropBase(pTHouse->ID, base);
		GetParadropPlane(base, pData->ParaDropDatas[SideClass::Array->Items[pTHouse->SideIndex]].size(), pTHouse);
	}
}

// Sends the paradrop planes for the given country to the cell specified.
/*
	Every house can have several planes defined. If a plane is not defined by a
	house, this falls back to the side's planes defined for this SW. If that
	fails also it falls back to this SW's default paradrop. If that also fails,
	the paradrop defined by the house is used.

	\param pHouse The owner of this super weapon.
	\param pCell The paradrop target cell.

	\author AlexB
	\date 2010-07-19
*/
bool SW_ParaDrop::SendParadrop(SuperClass* pThis, CellClass* pCell)
{
	// sanity
	if (!pThis || !pCell)
	{
		return false;
	}

	auto const pType = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);
	auto const pHouse = pThis->Owner;

	// these are fallback values if the SW doesn't define them
	AircraftTypeClass* pFallbackPlane = nullptr;
	Iterator<TechnoTypeClass*> FallbackTypes;
	Iterator<int> FallbackNum;

	HouseExtData::GetParadropContent(pHouse , FallbackTypes, FallbackNum);
	pFallbackPlane = HouseExtData::GetParadropPlane(pHouse);

	// use paradrop lists from house, side and default
	const std::vector<std::unique_ptr<ParadropData>>* drops[3] {
		pData->ParaDropDatas.find(pHouse->Type),
		pData->ParaDropDatas.find(SideClass::Array->Items[pHouse->Type->SideIndex]),
		pData->ParaDropDatas.find(pType)
	};

	// how many planes shall we launch?
	int count = 1;
	for (auto const& planes : drops) {
		if (planes) {
			count = planes->size();
			break;
		}
	}

	// assemble each plane and its contents
	for (int i = 0; i < count; ++i)
	{ // i = index of plane
		Iterator<TechnoTypeClass*> ParaDropTypes;
		Iterator<int> ParaDropNum;
		AircraftTypeClass* pParaDropPlane = nullptr;

		// try the planes in order of precedence:
		// * country, explicit plane
		// * side, explicit plane
		// * default, explict plane
		// * country, default plane
		// * side, default plane
		// * default, default plane
		// * fill gaps with data from house/side/rules
		for (int j = 1; j >= 0; --j)
		{ // factor 1 or 0: "plane * j" => "plane" or "0" (default)
			for (auto const& planes : drops)
			{ // get the country/side-specific plane list

				// only do something if there is data missing
				if (!(ParaDropTypes && ParaDropNum && pParaDropPlane))
				{
					auto const index = static_cast<size_t>(i * j);

					if (!planes || planes->size() <= index)
					{
						continue;
					}

					// get the plane at specified index
					if (auto const& pPlane = (*planes)[index])
					{

						// get the contents, if not already set
						if (!ParaDropTypes || !ParaDropNum)
						{
							if (!pPlane->Types.empty() && !pPlane->Num.empty())
							{
								ParaDropTypes = pPlane->Types;
								ParaDropNum = pPlane->Num;
							}
						}

						// get the airplane, if it isn't set already
						if (!pParaDropPlane)
						{
							pParaDropPlane = pPlane->Aircraft;
						}
					}
				}
			}
		}

		// fallback for types and nums
		if (!ParaDropTypes || !ParaDropNum)
		{
			ParaDropTypes = FallbackTypes;
			ParaDropNum = FallbackNum;
		}

		// house fallback for the plane
		if (!pParaDropPlane) {
			pParaDropPlane = pFallbackPlane;
		}

		// finally, send the plane
		if (ParaDropTypes && ParaDropNum && pParaDropPlane)
		{
			this->SendPDPlane(pHouse, pCell, pParaDropPlane, ParaDropTypes, ParaDropNum);
		}
	}

	return true;
}

//A new SendPDPlane function
//Allows vehicles, sends one single plane for all types
void SW_ParaDrop::SendPDPlane(HouseClass* pOwner, CellClass* pTarget, AircraftTypeClass* pPlaneType,
	Iterator<TechnoTypeClass*> const Types, Iterator<int> const Nums)
{
	if (Nums.size() != Types.size() || !Nums.size()
		|| !pOwner || !pPlaneType || !pTarget)
	{
		return;
	}

	++Unsorted::ScenarioInit;
	auto const pPlane = static_cast<AircraftClass*>(pPlaneType->CreateObject(pOwner));
	--Unsorted::ScenarioInit;

	if (!pPlane)
		return;

	pPlane->Spawned = true;

	//Get edge (direction for plane to come from)
	auto edge = pOwner->GetHouseEdge();

	// seems to retrieve a random cell struct at a given edge
	auto const spawn_cell = MapClass::Instance->PickCellOnEdge(
		edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
		MovementZone::Normal);

	pPlane->QueueMission(Mission::ParadropApproach, false);

	auto const bSpawned = AircraftExt::PlaceReinforcementAircraft(pPlane, spawn_cell);

	if (!bSpawned)
	{
		GameDelete(pPlane);
		return;
	}

	for (auto i = 0u; i < Types.size(); ++i)
	{
		auto const pType = Types[i];

		// find the nearest cell the paradrop troopers can land on
		// the movement zone etc is checked within first types of the passanger
		CellClass* pDest = pTarget;
		bool allowBridges = GroundType::GetCost(LandType::Clear,pType->SpeedType) > 0.0;
		bool isBridge = allowBridges && pDest->ContainsBridge();

		while (!pDest->IsClearToMove(pType->SpeedType, 0, 0, (int)MovementZone::None, pType->MovementZone, -1, isBridge)) {
			pDest = MapClass::Instance->GetCellAt(
				MapClass::Instance->NearByLocation(
					pDest->MapCoords,
					pType->SpeedType,
					-1,
					pType->MovementZone, isBridge, 1, 1, true, false, false, isBridge, CellStruct::Empty, false, false));

			isBridge = allowBridges && pDest->ContainsBridge();
		}

		pTarget = pDest;

		pPlane->SetTarget(pTarget);

		//only allow infantry and vehicles
		auto const abs = pType->WhatAmI();
		if (abs == AbstractType::UnitType || abs == AbstractType::InfantryType)
		{
			for (int k = 0; k < Nums[i]; ++k)
			{
				auto const pNew = static_cast<FootClass*>(pType->CreateObject(pOwner));

				if (!pNew) {
					continue;
				}

				pNew->SetLocation(pPlane->Location);
				pNew->Limbo();

				if (pPlane->Type->OpenTopped) {
					pPlane->EnteredOpenTopped(pNew);
				}

				pNew->Transporter = pPlane;
				pPlane->AddPassenger(static_cast<FootClass*>(pNew));
			}
		}
	}

	pPlane->HasPassengers = true;
	pPlane->NextMission();
}

bool SW_ParaDrop::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}