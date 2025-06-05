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
	if (!pThis || !pThis->IsCharged) {
		return false;
	}

	auto pTarget = MapClass::Instance->TryGetCellAt(Coords);
	if (!pTarget) {
		return false;
	}

	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pThis->Type);
	if (!pData) {
		return false;
	}

	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0) {
		this->SendParadrop(pThis, pTarget);
	} else {
		this->newStateMachine(nDeferement, Coords, pThis, pTarget);
	}

	return true;
}

void SW_ParaDrop::Initialize(SWTypeExtData* pData)
{
	if (!pData || !pData->AttachedToObject) {
		return;
	}

	pData->AttachedToObject->Action = pData->AttachedToObject->Type == SuperWeaponType::AmerParaDrop ?
		Action::AmerParaDrop : Action::ParaDrop;

	pData->SW_RadarEvent = false;
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_ReinforcementsReady);
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = int(MouseCursorType::ParaDrop);
}

void SW_ParaDrop::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	if (!pData || !pINI) {
		return;
	}

	const char* section = pData->get_ID();
	if (!section) {
		return;
	}

	INI_EX exINI(pINI);
	std::string _base = GameStrings::ParaDrop();
	_base.reserve(64); // Reserve space to avoid reallocations

	auto CreateParaDropBase = [](const char* pID, std::string& base) {
		if (pID && strlen(pID)) {
			base += ".";
			base += pID;
		}
	};

	auto ParseParaDrop = [section, &exINI](const char* pID, int Plane, ParadropData& out) {
		if (!pID) return;

		std::string _plane;
		_plane.reserve(32);
		
		if (Plane > 0) {
			_plane = ".Plane" + std::to_string(Plane + 1);
		}

		std::string _ID = pID;
		_ID.reserve(64);

		// Parse aircraft
		out.Aircraft.Read(exINI, section, (_ID + _plane + ".Aircraft").c_str());

		// Parse types
		out.Types.Read(exINI, section, (_ID + _plane + ".Types").c_str());

		// Don't parse nums if there are no types
		if (!out.Aircraft && out.Types.empty()) {
			return;
		}

		// Parse numbers
		out.Num.Read(exINI, section, (_ID + _plane + ".Num").c_str());
	};

	auto GetParadropPlane = [=, &_base](const char* pID, size_t defaultCount, AbstractTypeClass* pKey) {
		if (!pID || !pKey) return;

		auto& ParaDrop = pData->ParaDropDatas[pKey];
		auto const lastCount = ParaDrop.empty() ? defaultCount : ParaDrop.size();

		std::string _key = pID;
		auto const count = pINI->ReadInteger(section, (_key + ".Count").c_str(), static_cast<int>(lastCount));

		if (count <= 0) return;

		ParaDrop.resize(static_cast<size_t>(count));
		for (int i = 0; i < count; ++i) {
			ParseParaDrop(_base.c_str(), i, ParaDrop[i]);
		}
	};

	// Load paradrops: default, sides, countries
	CreateParaDropBase(nullptr, _base);
	GetParadropPlane(_base.c_str(), 1, pData->AttachedToObject);

	// Cache the base for reuse
	std::string originalBase = _base;

	// Process sides
	if (SideClass::Array) {
		for (auto const pSide : *SideClass::Array) {
			if (!pSide || !pSide->ID) continue;
			
			_base = originalBase;
			CreateParaDropBase(pSide->ID, _base);
			GetParadropPlane(_base.c_str(), pData->ParaDropDatas[pData->AttachedToObject].size(), pSide);
		}
	}

	// Process countries
	if (HouseTypeClass::Array && SideClass::Array) {
		for (auto const pTHouse : *HouseTypeClass::Array) {
			if (!pTHouse || !pTHouse->ID || pTHouse->SideIndex < 0 || 
				pTHouse->SideIndex >= SideClass::Array->Count) {
				continue;
			}
			
			_base = originalBase;
			CreateParaDropBase(pTHouse->ID, _base);
			GetParadropPlane(_base.c_str(), 
				pData->ParaDropDatas[SideClass::Array->Items[pTHouse->SideIndex]].size(), 
				pTHouse);
		}
	}
}

// Helper struct to avoid code duplication
struct ParadropPlaneData {
	AircraftTypeClass* PlaneType = nullptr;
	Iterator<TechnoTypeClass*> Types;
	Iterator<int> Nums;
};

// Extract common logic for getting paradrop data
std::vector<ParadropPlaneData> SW_ParaDrop::GetParadropData(SuperClass* pThis) const {
	if (!pThis || !pThis->Type || !pThis->Owner) {
		return {};
	}

	auto const pType = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);
	auto const pHouse = pThis->Owner;

	if (!pData) {
		return {};
	}

	// Fallback values
	AircraftTypeClass* pFallbackPlane = HouseExtData::GetParadropPlane(pHouse);
	const bool IsAmericanParadrop = pType->Type == SuperWeaponType::AmerParaDrop;

	Iterator<TechnoTypeClass*> FallbackTypes;
	Iterator<int> FallbackNum;

	if (IsAmericanParadrop) {
		FallbackTypes = make_iterator(RulesClass::Instance->AmerParaDropInf);
		FallbackNum = make_iterator(RulesClass::Instance->AmerParaDropNum);
	} else {
		HouseExtData::GetParadropContent(pHouse, FallbackTypes, FallbackNum);
	}

	// Get paradrop lists with null checks
	const std::vector<ParadropData>* drops[3] = { nullptr, nullptr, nullptr };
	
	if (pHouse->Type) {
		drops[0] = pData->ParaDropDatas.tryfind(pHouse->Type);
	}
	
	if (SideClass::Array && pHouse->Type && pHouse->Type->SideIndex >= 0 && 
		pHouse->Type->SideIndex < SideClass::Array->Count) {
		drops[1] = pData->ParaDropDatas.tryfind(SideClass::Array->Items[pHouse->Type->SideIndex]);
	}
	
	drops[2] = pData->ParaDropDatas.tryfind(pType);

	// Determine plane count
	int count = 1;
	for (auto const planes : drops) {
		if (planes && !planes->empty()) {
			count = static_cast<int>(planes->size());
			break;
		}
	}

	std::vector<ParadropPlaneData> result;
	result.reserve(count);

	// Process each plane
	for (int i = 0; i < count; ++i) {
		ParadropPlaneData planeData;
		
		// Try planes in order of precedence
		for (int j = 1; j >= 0; --j) {
			for (auto const planes : drops) {
				if (planeData.PlaneType && planeData.Types && planeData.Nums) {
					break; // All data found
				}

				if (!planes) continue;

				auto const index = static_cast<size_t>(i * j);
				if (planes->size() <= index) continue;

				auto const& pPlane = (*planes)[index];

				if (pPlane.Num.empty()) continue;

				// Get contents if not already set
				if ((!planeData.Types || !planeData.Nums) && 
					!pPlane.Types.empty() && !pPlane.Num.empty()) {
					planeData.Types = pPlane.Types;
					planeData.Nums = pPlane.Num;
				}

				// Get airplane if not set
				if (!planeData.PlaneType && pPlane.Aircraft) {
					planeData.PlaneType = pPlane.Aircraft;
				}
			}
		}

		// Apply fallbacks
		if (!planeData.Types || !planeData.Nums) {
			planeData.Types = FallbackTypes;
			planeData.Nums = FallbackNum;
		}

		if (!planeData.PlaneType) {
			planeData.PlaneType = pFallbackPlane;
		}

		// Only add if we have all required data
		if (planeData.Types && planeData.Nums && planeData.PlaneType) {
			result.push_back(planeData);
		}
	}

	return result;
}

bool SW_ParaDrop::SendParadrop(SuperClass* pThis, CellClass* pCell)
{
	if (!pThis || !pCell) {
		return false;
	}

	auto planeData = GetParadropData(pThis);
	if (planeData.empty()) {
		return false;
	}

	bool success = false;
	for (const auto& data : planeData) {
		if (this->SendPDPlane(pThis->Owner, pCell, data.PlaneType, data.Types, data.Nums)) {
			success = true;
		}
	}

	return success;
}

// Improved SendPDPlane function with better error handling and safety checks
bool SW_ParaDrop::SendPDPlane(HouseClass* pOwner, CellClass* pTarget, AircraftTypeClass* pPlaneType,
	Iterator<TechnoTypeClass*> const Types, Iterator<int> const Nums)
{
	// Validation
	if (!pOwner || !pTarget || !pPlaneType || Types.size() != Nums.size() || Types.empty()) {
		return false;
	}

	// Create plane
	++Unsorted::ScenarioInit;
	auto const pPlane = static_cast<AircraftClass*>(pPlaneType->CreateObject(pOwner));
	--Unsorted::ScenarioInit;

	if (!pPlane) {
		return false;
	}

	pPlane->Spawned = true;

	// Get spawn location
	auto edge = pOwner->GetHouseEdge();
	auto const spawn_cell = MapClass::Instance->PickCellOnEdge(
		edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
		MovementZone::Normal);

	pPlane->QueueMission(Mission::ParadropApproach, false);

	if (!AircraftExt::PlaceReinforcementAircraft(pPlane, spawn_cell)) {
		GameDelete<true, false>(pPlane);
		return false;
	}

	// Add passengers
	bool hasValidPassengers = false;
	
	for (auto i = 0u; i < Types.size(); ++i) {
		auto const pType = Types[i];
		if (!pType || Nums[i] <= 0) continue;

		// Only allow infantry and vehicles
		auto const abs = pType->WhatAmI();
		if (abs != AbstractType::UnitType && abs != AbstractType::InfantryType) {
			continue;
		}

		// Find suitable landing cell with safety limit
		CellClass* pDest = pTarget;
		bool allowBridges = GroundType::GetCost(LandType::Clear, pType->SpeedType) > 0.0;
		bool isBridge = allowBridges && pDest->ContainsBridge();
		
		// Prevent infinite loop with max iterations
		int maxIterations = 100;
		int iterations = 0;
		
		while (!pDest->IsClearToMove(pType->SpeedType, 0, 0, ZoneType::None, pType->MovementZone, -1, isBridge) &&
			   iterations < maxIterations) {
			
			auto newCoords = MapClass::Instance->NearByLocation(
				pDest->MapCoords, pType->SpeedType, ZoneType::None,
				pType->MovementZone, isBridge, 1, 1, true, false, false, isBridge, 
				CellStruct::Empty, false, false);
			
			auto newCell = MapClass::Instance->GetCellAt(newCoords);
			if (!newCell || newCell == pDest) {
				break; // Avoid infinite loop
			}
			
			pDest = newCell;
			isBridge = allowBridges && pDest->ContainsBridge();
			++iterations;
		}

		pPlane->SetTarget(pDest);

		// Create passengers
		for (int k = 0; k < Nums[i]; ++k) {
			auto const pNew = static_cast<FootClass*>(pType->CreateObject(pOwner));
			if (!pNew) continue;

			pNew->SetLocation(pPlane->Location);
			pNew->Limbo();

			if (pPlane->Type->OpenTopped) {
				pPlane->EnteredOpenTopped(pNew);
			}

			pNew->Transporter = pPlane;
			pPlane->AddPassenger(pNew);
			hasValidPassengers = true;
		}
	}

	if (hasValidPassengers) {
		pPlane->HasPassengers = true;
		pPlane->NextMission();
		return true;
	} else {
		// Clean up if no valid passengers were added
		GameDelete<true, false>(pPlane);
		return false;
	}
}

bool SW_ParaDrop::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!pData || !pBuilding || !this->IsLaunchsiteAlive(pBuilding)) {
		return false;
	}

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type)) {
		return true;
	}

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void ParaDropStateMachine::UpdateProperties()
{
	if (!this->Super) {
		return;
	}

	auto planeData = SW_ParaDrop{}.GetParadropData(this->Super);
	
	// Clear existing data
	this->PlaneType.clear();
	this->Types.clear();
	this->Nums.clear();
	
	// Reserve space to avoid reallocations
	this->PlaneType.reserve(planeData.size());
	this->Types.reserve(planeData.size());
	this->Nums.reserve(planeData.size());

	// Copy data
	for (const auto& data : planeData) {
		this->PlaneType.push_back(data.PlaneType);
		this->Types.push_back(data.Types);
		this->Nums.push_back(data.Nums);
	}
}

void ParaDropStateMachine::Update()
{
	if (!this->Finished() || !this->Super || !this->Target) {
		return;
	}

	auto pData = this->GetTypeExtData();
	if (!pData) {
		return;
	}

	pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

	auto const sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1) {
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	// Launch planes
	SW_ParaDrop paradrop;
	for (size_t i = 0; i < this->PlaneType.size() && i < this->Types.size() && i < this->Nums.size(); ++i) {
		paradrop.SendPDPlane(this->Super->Owner, this->Target, this->PlaneType[i], this->Types[i], this->Nums[i]);
	}
}