#include "UnitDelivery.h"

#include <Misc/AresData.h>
#include <Misc/Ares/Hooks/Header.h>

std::vector<const char*> SW_UnitDelivery::GetTypeString() const
{
	return { "UnitDelivery" };
}

bool SW_UnitDelivery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW);

	int deferment = pData->SW_Deferment.Get(-1);
	if (deferment < 0)
	{
		deferment = 20;
	}

	this->newStateMachine(deferment, Coords, pThis);

	return true;
}

void SW_UnitDelivery::Initialize(SWTypeExt::ExtData* pData)
{
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
}

void SW_UnitDelivery::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->SW_Deliverables.Read(exINI, section, "Deliver.Types");
	pData->SW_BaseNormal.Read(exINI, section, "Deliver.BaseNormal");
	pData->SW_OwnerHouse.Read(exINI, section, "Deliver.Owner");
	pData->SW_DeliverBuildups.Read(exINI, section, "Deliver.BuildUp");
}

bool SW_UnitDelivery::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
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
	auto pData = SWTypeExt::ExtMap.Find(this->Super->Type);

	// some mod using this for an dummy SW , just skip everything if the SW_Deliverables
	// is empty
	if (pData->SW_Deliverables.empty())
		return;

	// get the house the units will belong to
	auto pOwner = HouseExt::GetHouseKind(pData->SW_OwnerHouse, false, this->Super->Owner);
	bool IsPlayerControlled = pOwner->ControlledByPlayer_();
	bool bBaseNormal = pData->SW_BaseNormal;
	bool bDeliverBuildup = pData->SW_DeliverBuildups;

	// create an instance of each type and place it
	// Otamaa : this thing bugged on debug mode , idk
	for (auto nPos = pData->SW_Deliverables.begin(); nPos != pData->SW_Deliverables.end(); ++nPos)
	{
		auto pType = *nPos;
		auto Item = static_cast<TechnoClass*>(pType->CreateObject(pOwner));

		if (!Item)
			continue;

		const auto ItemBuilding = specific_cast<BuildingClass*>(Item);

		// get the best options to search for a place
		short extentX = 1;
		short extentY = 1;
		SpeedType SpeedType = SpeedType::Track;
		MovementZone MovementZone = MovementZone::Normal;
		bool buildable = false;
		bool anywhere = false;
		auto PlaceCoords = this->Coords;

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
				if (!bBaseNormal)
					Is_FromSW(ItemBuilding) = true;

				if(bDeliverBuildup)
					ItemBuilding->QueueMission(Mission::Construction, false);
			}
			else
			{
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
				if (!TechnoExt_ExtData::IsPowered(Item) || (!Is_Operated(Item) && !TechnoExt_ExtData::IsOperated(Item)))
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
