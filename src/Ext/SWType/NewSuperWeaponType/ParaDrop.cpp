#include "ParaDrop.h"

#include <Utilities/Helpers.h>
#include <Ext/House/Body.h>
#include <Ext/Aircraft/Body.h>

bool SW_ParaDrop::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	if (!pThis->IsCharged)
		return false;

	auto pTarget = MapClass::Instance->TryGetCellAt(Coords);
	if (!pTarget)
		return false;

	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pThis->Type);
	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0)
	{
		this->SendParadrop(pThis, pTarget);
	}
	else
	{
		this->newStateMachine(nDeferement, Coords, pThis, pTarget);
	}

	return true;
}

void SW_ParaDrop::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = pData->This()->Type == SuperWeaponType::AmerParaDrop ?
		Action::AmerParaDrop : Action::ParaDrop;

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_ReinforcementsReady);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = int(MouseCursorType::ParaDrop);
}

// ---------------------------------------------------------------------------
// INI loading — builds the per-SW paradrop lookup table
//
// The table (pData->ParaDropDatas) is keyed by AbstractTypeClass* and maps
// each key to a list of planes (std::vector<ParadropData>). Each plane entry
// holds an aircraft type, a list of passenger types, and their counts.
//
// Three tiers of keys are populated, in this order:
//   1. The SW type itself        — the global default for this superweapon
//   2. Each SideClass            — side-specific overrides
//   3. Each HouseTypeClass       — country-specific overrides
//
// When a paradrop fires, the resolution order is country → side → SW default,
// so more specific definitions shadow less specific ones.
// ---------------------------------------------------------------------------
void SW_ParaDrop::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();
	INI_EX exINI(pINI);
	std::string _base = GameStrings::ParaDrop();

	// Appends ".SomeID" to the base tag, e.g. "ParaDrop" → "ParaDrop.Americans".
	// This builds the INI key prefix used to look up plane definitions.
	auto CreateParaDropBase = [](char* pID, std::string& base)
		{
			if (pID && strlen(pID))
			{
				base += ".";
				base += pID;
			}
		};

	// Reads a single plane's contents from INI:
	//   [Section]
	//   <pID><.PlaneN>.Aircraft=  — the aircraft type for this plane
	//   <pID><.PlaneN>.Types=     — list of TechnoTypes to paradrop
	//   <pID><.PlaneN>.Num=       — count for each type
	//
	// Plane index 0 uses no ".PlaneN" suffix (it's the default plane).
	// Plane index 1+ appends ".Plane2", ".Plane3", etc.
	auto ParseParaDrop = [section, &exINI](const char* pID, int Plane, ParadropData& out)
		{
			std::string _plane(".Plane");
			if (Plane)
			{
				_plane += std::to_string(Plane + 1);
			}
			else
			{
				_plane.clear();
			}

			std::string _ID = pID;

			out.Aircraft.Read(exINI, section, (_ID + _plane + ".Aircraft").c_str());
			out.Types.Read(exINI, section, (_ID + _plane + ".Types").c_str());

			// No point reading Num if there's nothing to drop
			if (!out.Aircraft && out.Types.empty())
				return;

			out.Num.Read(exINI, section, (_ID + _plane + ".Num").c_str());
		};

	// Reads (or re-reads) the full plane list for a given key.
	// The ".Count" tag controls how many planes are defined.
	// Falls back to defaultCount if the tag is absent.
	auto GetParadropPlane = [=, &_base](char const* pID, size_t defaultCount, AbstractTypeClass* pKey)
		{
			auto& ParaDrop = pData->ParaDropDatas[pKey];
			auto const lastCount = ParaDrop.size() ? ParaDrop.size() : defaultCount;

			std::string _key = pID;
			auto const count = pINI->ReadInteger(section, (_key + ".Count").c_str(), lastCount);

			ParaDrop.resize(static_cast<size_t>(count));
			for (int i = 0; i < count; ++i)
			{
				ParseParaDrop(_base.c_str(), i, ParaDrop[i]);
			}
		};

	// --- Tier 1: SW-level default (no ID suffix) ---
	CreateParaDropBase(nullptr, _base);
	GetParadropPlane(_base.c_str(), 1, pData->This());

	// --- Tier 2: per-side overrides ---
	// Inherits plane count from the SW default if not specified.
	for (auto const pSide : *SideClass::Array)
	{
		CreateParaDropBase(pSide->ID, _base);
		GetParadropPlane(_base.c_str(), pData->ParaDropDatas[pData->This()].size(), pSide);
	}

	// --- Tier 3: per-country overrides ---
	// Inherits plane count from the country's parent side if not specified.
	for (auto const pTHouse : *HouseTypeClass::Array)
	{
		CreateParaDropBase(pTHouse->ID, _base);
		GetParadropPlane(_base.c_str(), pData->ParaDropDatas[SideClass::Array->Items[pTHouse->SideIndex]].size(), pTHouse);
	}
}

// ---------------------------------------------------------------------------
// Paradrop resolution — assembles and sends all planes
//
// For each plane index, the code resolves aircraft type, passenger types,
// and passenger counts using a 3-tier fallback:
//
//   Priority 1: Country-specific definition (HouseTypeClass)
//   Priority 2: Side-specific definition    (SideClass)
//   Priority 3: SW-level default            (SuperWeaponTypeClass)
//
// Within each tier, there are two sub-priorities:
//   a) The exact plane at index i   (the "explicit" plane)
//   b) The first plane at index 0   (the "default" plane for that tier)
//
// The full search order for a plane at index i is therefore:
//   1. Country, plane i   →  2. Side, plane i   →  3. SW default, plane i
//   4. Country, plane 0   →  5. Side, plane 0   →  6. SW default, plane 0
//   7. House/Rules fallback (ParaDrop types from house or AmerParaDrop rules)
//
// Aircraft, Types, and Nums are resolved independently — it's possible
// for the aircraft to come from one tier and the contents from another.
// ---------------------------------------------------------------------------
bool SW_ParaDrop::SendParadrop(SuperClass* pThis, CellClass* pCell)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);
	auto const pHouse = pThis->Owner;

	// --- Gather engine-level fallback data ---
	// Used only if no SW-defined paradrop data is found at any tier.
	AircraftTypeClass* pFallbackPlane = HouseExtData::GetParadropPlane(pHouse);
	const bool IsAmericanParadrop = pType->Type == SuperWeaponType::AmerParaDrop;

	Iterator<TechnoTypeClass*> FallbackTypes;
	Iterator<int> FallbackNum;

	if (IsAmericanParadrop)
	{
		FallbackTypes = make_iterator(RulesClass::Instance->AmerParaDropInf);
		FallbackNum = make_iterator(RulesClass::Instance->AmerParaDropNum);
	}
	else
	{
		HouseExtData::GetParadropContent(pHouse, FallbackTypes, FallbackNum);
	}

	// --- Build the 3-tier lookup table ---
	// Index 0 = country (try parent country if this one has no data)
	// Index 1 = side
	// Index 2 = SW-level default
	const auto pCountry = pData->ParaDropDatas.tryfind(pHouse->Type);

	const std::vector<ParadropData>* drops[3] {
		pCountry ? pCountry : pData->ParaDropDatas.tryfind(pHouse->Type->FindParentCountry()),
		pData->ParaDropDatas.tryfind(SideClass::Array->Items[pHouse->Type->SideIndex]),
		pData->ParaDropDatas.tryfind(pType)
	};

	// --- Determine how many planes to launch ---
	// Use the plane count from the most specific tier that has data.
	int count = 1;
	for (auto const& planes : drops)
	{
		if (planes && !planes->empty())
		{
			count = planes->size();
			break;
		}
	}

	// --- Resolve and send each plane ---
	for (int i = 0; i < count; ++i)
	{
		Iterator<TechnoTypeClass*> ParaDropTypes;
		Iterator<int> ParaDropNum;
		AircraftTypeClass* pParaDropPlane = nullptr;

		// Two passes:
		//   j=1: try the exact plane at index i from each tier
		//   j=0: try the default plane (index 0) from each tier
		// Aircraft, Types, and Nums are filled independently —
		// the first valid source for each field wins.
		for (int j = 1; j >= 0; --j)
		{
			for (auto const& planes : drops)
			{
				// Stop early if everything is already resolved
				if (ParaDropTypes && ParaDropNum && pParaDropPlane)
					break;

				auto const index = static_cast<size_t>(i * j);

				if (!planes || planes->size() <= index)
					continue;

				auto const pPlane = &(*planes)[index];

				if (pPlane->Num.empty())
					continue;

				// Fill passenger types/counts if still missing
				if (!ParaDropTypes || !ParaDropNum)
				{
					if (!pPlane->Types.empty() && !pPlane->Num.empty())
					{
						ParaDropTypes = pPlane->Types;
						ParaDropNum = pPlane->Num;
					}
				}

				// Fill aircraft type if still missing
				if (!pParaDropPlane)
				{
					pParaDropPlane = pPlane->Aircraft;
				}
			}
		}

		// --- Final fallback: use house/rules defaults ---
		if (!ParaDropTypes || !ParaDropNum)
		{
			ParaDropTypes = FallbackTypes;
			ParaDropNum = FallbackNum;
		}

		if (!pParaDropPlane)
		{
			pParaDropPlane = pFallbackPlane;
		}

		// --- Launch ---
		if (ParaDropTypes && ParaDropNum && pParaDropPlane)
		{
			this->SendPDPlane(pHouse, pCell, pParaDropPlane, ParaDropTypes, ParaDropNum);
		}
	}

	return true;
}

#include <Ext/Techno/Body.h>

// ---------------------------------------------------------------------------
// SendPDPlane — creates a single paradrop aircraft and fills it with cargo
//
// Unlike the vanilla ParadropPlane logic, this supports vehicles as well as
// infantry. All passenger types are loaded into one aircraft.
//
// The aircraft is spawned at a random edge cell appropriate for flying units,
// given the ParadropApproach mission, and sent toward the target cell.
// Passengers are placed at the nearest valid landing cell for their movement
// type, so mixed infantry/vehicle drops land correctly.
// ---------------------------------------------------------------------------
void SW_ParaDrop::SendPDPlane(HouseClass* pOwner
	, CellClass* pTarget
	, AircraftTypeClass* pPlaneType
	, Iterator<TechnoTypeClass*> Types
	, Iterator<int> Nums)
{
	if (Nums.size() != Types.size() || !Nums.size()
		|| !pOwner || !pPlaneType)
	{
		return;
	}

	// Create the aircraft outside normal game flow (ScenarioInit bypass)
	++Unsorted::ScenarioInit;
	auto const pPlane = static_cast<AircraftClass*>(pPlaneType->CreateObject(pOwner));
	--Unsorted::ScenarioInit;

	if (!pPlane)
		return;

	pPlane->Spawned = true;

	// Pick a random map-edge cell suitable for air units to spawn from
	auto const spawn_cell = MapClass::Instance->PickCellOnEdge(
		pOwner->GetHouseEdge(), CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
		MovementZone::Normal);

	pPlane->QueueMission(Mission::ParadropApproach, false);

	for (auto i = 0u; i < Types.size(); ++i)
	{
		auto const pType = Types[i];

		// Find the nearest cell where this unit type can actually land.
		// We check the movement zone for each type individually because
		// infantry and vehicles may have different terrain requirements.
		CellClass* pDest = pTarget;
		bool allowBridges = GroundType::GetCost(LandType::Clear, pType->SpeedType) > 0.0f;
		bool isBridge = allowBridges && pDest->ContainsBridge();

		while (!pDest->IsClearToMove(pType->SpeedType, 0, 0, ZoneType::None, pType->MovementZone, -1, isBridge))
		{
			pDest = MapClass::Instance->GetCellAt(
				MapClass::Instance->NearByLocation(
					pDest->MapCoords,
					pType->SpeedType,
					ZoneType::None,
					pType->MovementZone, isBridge, 1, 1, true, false, false, isBridge, CellStruct::Empty, false, false));

			isBridge = allowBridges && pDest->ContainsBridge();
		}

		pTarget = pDest;
		pPlane->SetTarget(pTarget);

		// Only infantry and vehicles can be paradropped
		auto const abs = pType->WhatAmI();
		if (abs == AbstractType::UnitType || abs == AbstractType::InfantryType)
		{
			for (int k = 0; k < Nums[i]; ++k)
			{
				auto const pNew = static_cast<FootClass*>(pType->CreateObject(pOwner));
				pNew->SetLocation(pPlane->Location);
				pNew->Limbo();

				if (pPlane->Type->OpenTopped)
				{
					pPlane->EnteredOpenTopped(pNew);
				}

				if (pType->Gunner)
					pPlane->ReceiveGunner(pNew);

				pNew->Transporter = pPlane;
				pPlane->AddPassenger(static_cast<FootClass*>(pNew));
			}
		}
	}

	pPlane->HasPassengers = true;

	auto const bSpawned = AircraftExtData::PlaceReinforcementAircraft(pPlane, spawn_cell);

	if (!bSpawned)
	{
		GameDelete<true, false>(pPlane);
		return;
	}

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

// Copies the contents of an Iterator into a new row of a 2D vector.
// Used by ParaDropStateMachine to snapshot resolved paradrop data.
template<typename T>
COMPILETIMEEVAL void insertFromIterator(std::vector<std::vector<T>>& vec2d, const Iterator<T>& iter)
{
	std::vector<T> newRow;
	newRow.reserve(iter.size());

	for (size_t i = 0; i < iter.size(); ++i)
	{
		newRow.push_back(iter[i]);
	}

	vec2d.push_back(std::move(newRow));
}

// ---------------------------------------------------------------------------
// ParaDropStateMachine::UpdateProperties
//
// Deferred version of SendParadrop — instead of sending planes immediately,
// this resolves the same 3-tier fallback chain and stores the results
// (PlaneType, Types, Nums) for later dispatch in Update().
//
// The resolution logic is identical to SW_ParaDrop::SendParadrop.
// See that function's comments for the full fallback order.
// ---------------------------------------------------------------------------
void ParaDropStateMachine::UpdateProperties()
{
	auto const pType = this->Super->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);
	auto const pHouse = this->Super->Owner;

	// --- Gather engine-level fallback data ---
	AircraftTypeClass* pFallbackPlane = HouseExtData::GetParadropPlane(pHouse);
	const bool IsAmericanParadrop = pType->Type == SuperWeaponType::AmerParaDrop;

	Iterator<TechnoTypeClass*> FallbackTypes;
	Iterator<int> FallbackNum;

	if (IsAmericanParadrop)
	{
		FallbackTypes = make_iterator(RulesClass::Instance->AmerParaDropInf);
		FallbackNum = make_iterator(RulesClass::Instance->AmerParaDropNum);
	}
	else
	{
		HouseExtData::GetParadropContent(pHouse, FallbackTypes, FallbackNum);
	}

	// --- Build the 3-tier lookup (country → side → SW default) ---
	const std::vector<ParadropData>* drops[3] {
		pData->ParaDropDatas.tryfind(pHouse->Type),
		pData->ParaDropDatas.tryfind(SideClass::Array->Items[pHouse->Type->SideIndex]),
		pData->ParaDropDatas.tryfind(pType)
	};

	// --- Determine plane count from the most specific tier ---
	int count = 1;
	for (auto const& planes : drops)
	{
		if (planes && !planes->empty())
		{
			count = planes->size();
			break;
		}
	}

	// --- Resolve each plane and store for deferred dispatch ---
	for (int i = 0; i < count; ++i)
	{
		Iterator<TechnoTypeClass*> ParaDropTypes;
		Iterator<int> ParaDropNum;
		AircraftTypeClass* pParaDropPlane = nullptr;

		// Two passes: j=1 tries exact plane index, j=0 tries default (index 0)
		for (int j = 1; j >= 0; --j)
		{
			for (auto const& planes : drops)
			{
				if (ParaDropTypes && ParaDropNum && pParaDropPlane)
					break;

				auto const index = static_cast<size_t>(i * j);

				if (!planes || planes->size() <= index)
					continue;

				auto const pPlane = &(*planes)[index];

				if (pPlane->Num.empty())
					continue;

				if (!ParaDropTypes || !ParaDropNum)
				{
					if (!pPlane->Types.empty() && !pPlane->Num.empty())
					{
						ParaDropTypes = pPlane->Types;
						ParaDropNum = pPlane->Num;
					}
				}

				if (!pParaDropPlane)
				{
					pParaDropPlane = pPlane->Aircraft;
				}
			}
		}

		// --- Final fallback ---
		if (!ParaDropTypes || !ParaDropNum)
		{
			ParaDropTypes = FallbackTypes;
			ParaDropNum = FallbackNum;
		}

		if (!pParaDropPlane)
		{
			pParaDropPlane = pFallbackPlane;
		}

		// --- Store resolved data for later dispatch ---
		if (ParaDropTypes && ParaDropNum && pParaDropPlane)
		{
			this->PlaneType.push_back(pParaDropPlane);
			insertFromIterator(this->Types, ParaDropTypes);
			insertFromIterator(this->Nums, ParaDropNum);
		}
	}
}

// Called each frame by the state machine. Once the deferment timer expires,
// plays the activation message/sound and dispatches all resolved planes.
void ParaDropStateMachine::Update()
{
	if (!this->Finished())
		return;

	auto pData = this->GetTypeExtData();

	pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

	auto const sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1)
	{
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	for (size_t i = 0; i < this->PlaneType.size(); ++i)
	{
		SW_ParaDrop::SendPDPlane(this->Super->Owner, this->Target, this->PlaneType[i], this->Types[i], this->Nums[i]);
	}
}