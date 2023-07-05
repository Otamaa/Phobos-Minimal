#include "SWStateMachine.h"

#include "NuclearMissile.h"
#include "Dominator.h"
#include "LightningStorm.h"

#include <Misc/AresData.h>

#include <Ext/House/Body.h>

std::vector<std::unique_ptr<SWStateMachine>> SWStateMachine::Array;

void SWStateMachine::UpdateAll()
{
	for (auto& Machine : SWStateMachine::Array) {

		if (Machine)
			Machine->Update();
	}

	Array.erase(std::remove_if(Array.begin(), Array.end(), 
	[](const std::unique_ptr<SWStateMachine>& ptr) {
		return ptr->Finished();
	}), Array.end());
}

void SWStateMachine::PointerGotInvalid(void* ptr, bool remove)
{

	for (auto& Machine : SWStateMachine::Array) {

		if(Machine)
			Machine->InvalidatePointer(ptr, remove);
	}

	AnnounceInvalidPointer(SW_NuclearMissile::CurrentNukeType, ptr);
	AnnounceInvalidPointer(SW_PsychicDominator::CurrentPsyDom, ptr);
	AnnounceInvalidPointer(SW_LightningStorm::CurrentLightningStorm, ptr);
}

void SWStateMachine::Clear()
{
	// hard-reset any super weapon related globals
	SWStateMachine::Array.clear();
	SW_NuclearMissile::CurrentNukeType = nullptr;
	SW_PsychicDominator::CurrentPsyDom = nullptr;
	SW_LightningStorm::CurrentLightningStorm = nullptr;
}

void UnitDeliveryStateMachine::Update()
{
	if (this->Finished())
	{
		CoordStruct coords = CellClass::Cell2Coord(this->Coords);

		auto pData = SWTypeExt::ExtMap.Find(this->Super->Type);

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		if (pData->SW_ActivationSound != -1)
		{
			VocClass::PlayAt(pData->SW_ActivationSound, coords, nullptr);
		}

		this->PlaceUnits();
	}
}

//This function doesn't skip any placeable items, no matter how
//they are ordered. Infantry is grouped. Units are moved to the
//center as close as they can.

void UnitDeliveryStateMachine::PlaceUnits()
{
	const auto pData = SWTypeExt::ExtMap.Find(this->Super->Type);
	// get the house the units will belong to
	const auto pOwner = HouseExt::GetHouseKind(pData->SW_OwnerHouse, false, this->Super->Owner);
	const bool IsPlayerControlled = pOwner->ControlledByPlayer_();
	const bool bBaseNormal = pData->SW_DeliverBuildups;
	const auto nPlace = this->Coords;

	// create an instance of each type and place it
	for (auto const& pType : pData->SW_Deliverables)
	{
		auto Item = static_cast<TechnoClass*>(pType->CreateObject(pOwner));

		if (!Item)
			continue;

		auto ItemBuilding = specific_cast<BuildingClass*>(Item);

		// get the best options to search for a place
		short extentX = 1;
		short extentY = 1;
		SpeedType SpeedType = SpeedType::Track;
		MovementZone MovementZone = MovementZone::Normal;
		bool buildable = false;
		bool anywhere = false;
		auto PlaceCoords = nPlace;

		if (ItemBuilding)
		{
			auto BuildingType = ItemBuilding->Type;
			extentX = BuildingType->GetFoundationWidth();
			extentY = BuildingType->GetFoundationHeight(true);
			anywhere = BuildingType->PlaceAnywhere;
			if (pType->SpeedType == SpeedType::Float)
			{
				SpeedType = SpeedType::Float;
			}
			else
			{
				buildable = true;
			}
		}
		else
		{
			// place aircraft types on ground explicitly
			if (pType->WhatAmI() != AbstractType::AircraftType)
			{
				SpeedType = pType->SpeedType;
				MovementZone = pType->MovementZone;
			}
		}

		// move the target cell so this object is centered on the actual location
		PlaceCoords = this->Coords - CellStruct{short(extentX / 2), short(extentY / 2)};

		// find a place to put this
		if (!anywhere)
		{
			int a5 = -1; // usually MapClass::CanLocationBeReached call. see how far we can get without it
			PlaceCoords = MapClass::Instance->NearByLocation(PlaceCoords,
				SpeedType, a5, MovementZone, false, extentX, extentY, true,
				false, false, false, CellStruct::Empty, false, buildable);
		}

		if (auto pCell = MapClass::Instance->TryGetCellAt(PlaceCoords))
		{
			Item->OnBridge = pCell->ContainsBridge();

			// set the appropriate mission
			if (ItemBuilding)
			{
				if(!bBaseNormal)
					Is_FromSW(ItemBuilding) = true;
				else
					ItemBuilding->QueueMission(Mission::Construction, false);
			} else {
				// only computer units can hunt
				Item->QueueMission(IsPlayerControlled ? Mission::Guard : Mission::Hunt, false);
			}

			// place and set up
			auto XYZ = pCell->GetCoordsWithBridge();
			if (Item->Unlimbo(XYZ, DirType(MapClass::GetCellIndex(pCell->MapCoords) & 7u)))
			{
				if (ItemBuilding)
				{
					if (bBaseNormal)
					{
						ItemBuilding->DiscoveredBy(pOwner);
						ItemBuilding->IsReadyToCommence = 1;
					}
				}
				else 
				{
					if (pType->BalloonHover || pType->JumpJet)
						Item->Scatter(CoordStruct::Empty, true, false);

					if (Item->CurrentMission == Mission::Area_Guard && !IsPlayerControlled)
						Item->QueueMission(Mission::Hunt, true);
				}
				if (!AresData::IsPowered(Item) || (!Is_Operated(Item) && !AresData::IsOperated(Item)))
				{
					Item->Deactivate();
					if (ItemBuilding)
					{
						Item->Owner->RecheckTechTree = true;
					}
				}
			}
			else
			{
				Item->UnInit();
			}
		}
	}
}


bool SWStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Clock)
		.Process(this->Super, RegisterForChange)
		.Process(this->Type, RegisterForChange)
		.Process(this->Coords)
		.Success();
}

bool SWStateMachine::Save(PhobosStreamWriter& Stm) const
{
	// used to instantiate in ObjectFactory
	Stm.Save(this->GetIdentifier());

	return Stm
		.Process(this->Clock)
		.Process(this->Super)
		.Process(this->Type)
		.Process(this->Coords)
		.Success();
}

bool SWStateMachine::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(Array)
		.Process(SW_NuclearMissile::CurrentNukeType)
		.Process(SW_PsychicDominator::CurrentPsyDom)
		.Process(SW_LightningStorm::CurrentLightningStorm)
		.Success();
}

bool SWStateMachine::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(Array)
		.Process(SW_NuclearMissile::CurrentNukeType)
		.Process(SW_PsychicDominator::CurrentPsyDom)
		.Process(SW_LightningStorm::CurrentLightningStorm)
		.Success();
}