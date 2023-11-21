#include "UnitDelivery.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Ares_TechnoExt.h>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>

std::vector<const char*> SW_UnitDelivery::GetTypeString() const
{
	return { "UnitDelivery" };
}

bool SW_UnitDelivery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW);

	const int deferment = pData->SW_Deferment.Get(-1);
	this->newStateMachine(deferment < 0 ? 20 : deferment, Coords, pThis);

	return true;
}

void SW_UnitDelivery::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
}

void SW_UnitDelivery::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->SW_Deliverables.Read(exINI, section, "Deliver.Types" , true);
	pData->SW_Deliverables_Facing.Read(exINI, section, "Deliver.TypesFacing");
	pData->SW_BaseNormal.Read(exINI, section, "Deliver.BaseNormal");
	pData->SW_OwnerHouse.Read(exINI, section, "Deliver.Owner");
	pData->SW_DeliverBuildups.Read(exINI, section, "Deliver.BuildUp");
	pData->SW_DeliverableScatter.Read(exINI, section, "Deliver.Scatter");
}

bool SW_UnitDelivery::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
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

		const auto pData = SWTypeExtContainer::Instance.Find(this->Super->Type);

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
	const auto pData = SWTypeExtContainer::Instance.Find(this->Super->Type);

	// some mod using this for an dummy SW , just skip everything if the SW_Deliverables
	// is empty
	if (pData->SW_Deliverables.empty())
		return;

	// get the house the units will belong to
	const auto pOwner = HouseExtData::GetHouseKind(pData->SW_OwnerHouse, false, this->Super->Owner);
	const bool IsPlayerControlled = pOwner->IsControlledByHuman();
	const bool bBaseNormal = pData->SW_BaseNormal;
	const bool bDeliverBuildup = pData->SW_DeliverBuildups;
	const size_t facingsize = pData->SW_Deliverables_Facing.size();

	//Debug::Log("PlaceUnits for [%s] - Owner[%s] \n", pData->get_ID(), pOwner->get_ID());
	// create an instance of each type and place it
	for (size_t i = 0; i < pData->SW_Deliverables.size(); ++i)
	{
		auto pType = pData->SW_Deliverables[i];

		if (!pType || pType->Strength == 0)
			continue;

		auto Item = static_cast<TechnoClass*>(pType->CreateObject(pOwner));

		if (!Item)
			continue;

		//Debug::Log("PlaceUnits for [%s] - Owner[%s] After CreateObj[%s] \n", pData->get_ID(), pOwner->get_ID() , pType->ID);

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
					BuildingExtContainer::Instance.Find(ItemBuilding)->IsFromSW = true;

				if(bDeliverBuildup)
					ItemBuilding->QueueMission(Mission::Construction, false);
			}
			else
			{
				const auto mission = IsPlayerControlled ? Mission::Guard : Mission::Hunt;
				//Debug::Log("PlaceUnits for [%s] - Owner[%s] SwtMission[%s - %s] \n", pData->get_ID(), pOwner->get_ID(), pType->ID , MissionClass::MissionToString(mission));

				// only computer units can hunt
				Item->QueueMission(mission, false);
			}

			// place and set up
			auto XYZ = pCell->GetCoordsWithBridge();
			const unsigned short dir = !facingsize || i > facingsize ? MapClass::GetCellIndex(pCell->MapCoords) & 7u : (unsigned char)pData->SW_Deliverables_Facing[i];
			/*
			*	DirType -> Raw (<< 8)
			*	FacingType -> Raw (<< 13)
			*	FacingType -> DirType ( << 5 )
			*/
			if (Item->Unlimbo(XYZ, DirType((dir << 5))))
			{
				//Debug::Log("PlaceUnits for [%s] - Owner[%s] After Unlimbo[%s] \n", pData->get_ID(), pOwner->get_ID(), pType->ID);

				if (ItemBuilding)
				{
					if (bDeliverBuildup)
					{
						ItemBuilding->DiscoveredBy(pOwner);
						ItemBuilding->IsReadyToCommence = 1;

						//Debug::Log("PlaceUnits for [%s] - Owner[%s] After DiscoverBld[%s] \n", pData->get_ID(), pOwner->get_ID(), pType->ID);
					}
				}
				else
				{
					if (pData->SW_DeliverableScatter.Get(pType->BalloonHover || pType->JumpJet))
						Item->Scatter(CoordStruct::Empty, true, false);

					if (Item->CurrentMission == Mission::Area_Guard && !IsPlayerControlled)
						Item->QueueMission(Mission::Hunt, true);
				}

				if (!TechnoExt_ExtData::IsPowered(Item) || !TechnoExt_ExtData::IsOperatedB(Item))
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
				Debug::Log(__FUNCTION__"\n");
				TechnoExtData::HandleRemove(Item, nullptr, true, true);
				//Item->UnInit();
			}
		}
	}

	//Debug::Log("PlaceUnits for [%s] - Owner[%s] Done \n", pData->get_ID(), pOwner->get_ID());
}
