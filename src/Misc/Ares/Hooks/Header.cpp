#include "Header.h"
#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>
#include <Misc/AresData.h>

#include <Notifications.h>
#include <strsafe.h>
#include <Ares_TechnoExt.h>
#include "AresNetEvent.h"
#include <NetworkEvents.h>
#include <Networking.h>

DWORD FirewallFunctions::GetFirewallFlags(BuildingClass* pThis)
{
	auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
	DWORD flags = 0;
	for (size_t direction = 0; direction < 8; direction += 2)
	{
		auto pNeighbour = pCell->GetNeighbourCell((FacingType)direction);
		if (auto pBld = pNeighbour->GetBuilding())
		{
			if (pBld->Type->FirestormWall && pBld->Owner == pThis->Owner && !pBld->InLimbo && pBld->IsAlive)
			{
				flags |= 1 << (direction >> 1);
			}
		}
	}

	return flags;
}

void FirewallFunctions::ImmolateVictims(TechnoClass* pThis)
{
	auto const pCell = pThis->GetCell();
	for (NextObject object(pCell->FirstObject); object; ++object)
	{
		if (auto pFoot = abstract_cast<FootClass*>(*object))
		{
			if (!pFoot->GetType()->IgnoresFirestorm)
			{
				FirewallFunctions::ImmolateVictim(pThis, pFoot);
			}
		}
	}
}

bool FirewallFunctions::ImmolateVictim(TechnoClass* pThis, ObjectClass* const pVictim, bool const destroy)
{
	if (pVictim && pVictim->IsAlive && pVictim->Health > 0 && !pVictim->InLimbo)
	{
		auto const pRulesExt = RulesExt::Global();

		if (destroy)
		{
			const auto pWarhead = pRulesExt->FirestormWarhead.Get(
				RulesClass::Instance->C4Warhead);

			auto damage = pVictim->Health;
			pVictim->ReceiveDamage(&damage, 0, pWarhead, pThis, true, true,pThis->Owner);
		}

		auto const& pType = (pVictim->GetHeight() < 100)
			? pRulesExt->FirestormGroundAnim
			: pRulesExt->FirestormAirAnim;

		if (pType)
		{
			auto const crd = pVictim->GetCoords();
			HouseClass* pTarget = nullptr;
			switch (pVictim->WhatAmI())
			{
			case AbstractType::Building:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
			case AbstractType::Infantry:
				pTarget = ((TechnoClass*)pVictim)->Owner;
				break;
			case AbstractType::Bullet:
			{
				const auto pBlt = (BulletClass*)pVictim;
				pTarget = pBlt->Owner ? pBlt->Owner->Owner : BulletExt::ExtMap.Find(pBlt)->Owner;
			}
			break;
			default:
				break;
			}

			AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, -10, false),
				pThis->Owner,
				pTarget,
				pThis,
				false
			);

		}

		return true;
	}

	return false;
}

void FirewallFunctions::UpdateFirewall(BuildingClass* pThis, bool const changedState)
{
	if (!pThis->Type->FirestormWall)
	{
		return;
	}

	auto const active = pThis->Owner->FirestormActive;

	if (!changedState)
	{
		// update only the idle anim
		auto& Anim = pThis->GetAnim(BuildingAnimSlot::SpecialTwo);

		// (0b0101 || 0b1010) == part of a straight line
		auto const connections = pThis->FirestormWallFrame & 0xF;
		if (active && Unsorted::CurrentFrame & 7 && !Anim
			&& connections != 0b0101 && connections != 0b1010
			&& (ScenarioClass::Instance->Random.Random() & 0xF) == 0)
		{
			if (AnimTypeClass* pType = RulesExt::Global()->FirestormIdleAnim)
			{
				auto const crd = pThis->GetCoords() - CoordStruct { 740, 740, 0 };
				Anim = GameCreate<AnimClass>(pType, crd, 0, 1, 0x604, -10);
				Anim->IsBuildingAnim = true;
			}
		}
	}
	else
	{
		// update the frame, cell passability and active anim
		auto const idxFrame = FirewallFunctions::GetFirewallFlags(pThis)
			+ (active ? 32u : 0u);

		if (pThis->FirestormWallFrame != idxFrame)
		{
			pThis->FirestormWallFrame = idxFrame;
			pThis->GetCell()->Setup(0xFFFFFFFF);
			pThis->UpdatePlacement(PlacementType::Redraw);
		}

		auto& Anim = pThis->GetAnim(BuildingAnimSlot::Special);

		auto const connections = idxFrame & 0xF;
		if (active && connections != 0b0101 && connections != 0b1010 && !Anim)
		{
			if (auto const& pType = RulesExt::Global()->FirestormActiveAnim)
			{
				auto const crd = pThis->GetCoords() - CoordStruct { 128, 128, 0 };
				Anim = GameCreate<AnimClass>(pType, crd, 1, 0, 0x600, -10);
				Anim->IsFogged = pThis->IsFogged;
				Anim->IsBuildingAnim = true;
			}
		}
		else if (Anim)
		{
			Anim->UnInit();
		}
	}

	if (active)
	{
		FirewallFunctions::ImmolateVictims(pThis);
	}
}

void FirewallFunctions::UpdateFirewallLinks(BuildingClass* pThis)
{
	if (pThis->Type->FirestormWall)
	{
		// update this
		if (!pThis->InLimbo && pThis->IsAlive)
		{
			FirewallFunctions::UpdateFirewall(pThis, true);
		}

		// and all surrounding buildings
		auto const pCell = MapClass::Instance->GetCellAt(pThis->Location);
		for (size_t i = 0u; i < 8; i += 2)
		{
			auto const pNeighbour = pCell->GetNeighbourCell((FacingType)i);
			if (auto const pBld = pNeighbour->GetBuilding())
			{
				FirewallFunctions::UpdateFirewall(pThis, true);
			}
		}
	}
}

bool FirewallFunctions::IsActiveFirestormWall(BuildingClass* const pBuilding, HouseClass const* const pIgnore)
{
	if (IsAnySFWActive && pBuilding && pBuilding->Owner != pIgnore && pBuilding->Owner->FirestormActive)
	{
		if (!pBuilding->InLimbo && pBuilding->IsAlive)
		{
			return pBuilding->Type->FirestormWall;
		}
	}

	return false;
}

bool FirewallFunctions::sameTrench(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	const auto currentTypeExtData = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	const auto targetTypeExtData = BuildingTypeExt::ExtMap.Find(targetBuilding->Type);

	return ((currentTypeExtData->IsTrench > -1) && (currentTypeExtData->IsTrench == targetTypeExtData->IsTrench));
}

bool FirewallFunctions::canLinkTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	// Different owners // and owners not allied
	if ((currentBuilding->Owner != targetBuilding->Owner) && !currentBuilding->Owner->IsAlliedWith(targetBuilding->Owner))
	{ //<-- see thread 1424
		return false;
	}

	BuildingTypeExt::ExtData* currentTypeExtData = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	BuildingTypeExt::ExtData* targetTypeExtData = BuildingTypeExt::ExtMap.Find(targetBuilding->Type);

	// Firewalls
	if (currentBuilding->Type->FirestormWall && targetBuilding->Type->FirestormWall)
	{
		return true;
	}

	// Trenches
	if (FirewallFunctions::sameTrench(currentBuilding, targetBuilding))
	{
		return true;
	}

	return false;
}

void FirewallFunctions::BuildLines(BuildingClass* theBuilding, CellStruct selectedCell, HouseClass* buildingOwner)
{
	// check if this building is linkable at all and abort if it isn't
	if (!BuildingTypeExt::IsLinkable(theBuilding->Type))
	{
		return;
	}

	short maxLinkDistance = static_cast<short>(theBuilding->Type->GuardRange / 256); // GuardRange governs how far the link can go, is saved in leptons

	for (size_t direction = 0; direction <= 7; direction += 2)
	{
		// the 4 straight directions of the simple compass
		CellStruct directionOffset = CellSpread::GetNeighbourOffset(direction); // coordinates of the neighboring cell in the given direction relative to the current cell (e.g. 0,1)
		int linkLength = 0; // how many cells to build on from center in direction to link up with a found building

		CellStruct cellToCheck = selectedCell;
		for (short distanceFromCenter = 1; distanceFromCenter <= maxLinkDistance; ++distanceFromCenter)
		{
			cellToCheck += directionOffset; // adjust the cell to check based on current distance, relative to the selected cell

			CellClass* cell = MapClass::Instance->TryGetCellAt(cellToCheck);

			if (!cell)
			{ // don't parse this cell if it doesn't exist (duh)
				break;
			}

			if (BuildingClass* OtherEnd = cell->GetBuilding())
			{ // if we find a building...
				if (FirewallFunctions::canLinkTo(theBuilding, OtherEnd))
				{ // ...and it is linkable, we found what we needed
					linkLength = distanceFromCenter - 1; // distanceFromCenter directly would be on top of the found building
					break;
				}

				break; // we found a building, but it's not linkable
			}

			if (!cell->CanThisExistHere(theBuilding->Type->SpeedType, theBuilding->Type, buildingOwner))
			{ // abort if that buildingtype is not allowed to be built there
				break;
			}
		}

		// build a line of this buildingtype from the found building (if any) to the newly built one
		CellStruct cellToBuildOn = selectedCell;
		for (int distanceFromCenter = 1; distanceFromCenter <= linkLength; ++distanceFromCenter)
		{
			cellToBuildOn += directionOffset;

			if (CellClass* cell = MapClass::Instance->GetCellAt(cellToBuildOn))
			{
				if (BuildingClass* tempBuilding = specific_cast<BuildingClass*>(theBuilding->Type->CreateObject(buildingOwner)))
				{
					CoordStruct coordBuffer = CellClass::Cell2Coord(cellToBuildOn);

					++Unsorted::ScenarioInit; // put the building there even if normal rules would deny - e.g. under units
					bool Put = tempBuilding->Unlimbo(coordBuffer, DirType::North);
					--Unsorted::ScenarioInit;

					if (Put)
					{
						tempBuilding->QueueMission(Mission::Construction, false);
						tempBuilding->DiscoveredBy(buildingOwner);
						tempBuilding->IsReadyToCommence = 1;
					}
					else
					{
						Debug::Log(__FUNCTION__"Called!\n");
						TechnoExt::HandleRemove(tempBuilding, nullptr, true, true);
					}
				}
			}
		}
	}
}

int FirewallFunctions::GetImageFrameIndex(BuildingClass* pThis)
{
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Type->FirestormWall)
	{
		return static_cast<int>(pThis->FirestormWallFrame);

		/* this is the code the game uses to calculate the firewall's frame number when you place/remove sections... should be a good base for trench frames

			int frameIdx = 0;
			CellClass *Cell = this->GetCell();
			for(int direction = 0; direction <= 7; direction += 2) {
				if(BuildingClass *B = Cell->GetNeighbourCell(direction)->GetBuilding()) {
					if(B->IsAlive && !B->InLimbo) {
						frameIdx |= (1 << (direction >> 1));
					}
				}
			}

		*/
	}

	if (pData->IsTrench > -1)
	{
		return 0;
	}

	return -1;
}

NOINLINE BSurface* TechnoTypeExt_ExtData::GetPCXSurface(TechnoTypeClass* pType, HouseClass* pHouse)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto eliteCameo = TechnoTypeExt_ExtData::CameoIsElite(pType, pHouse);

	return eliteCameo ? pTypeExt->AltCameoPCX.GetSurface() : pTypeExt->CameoPCX.GetSurface();
}

bool NOINLINE TechnoTypeExt_ExtData::CameoIsElite(TechnoTypeClass* pType, HouseClass* pHouse)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if ((!pType->AltCameo && !pTypeExt->AltCameoPCX.GetSurface()) || !pHouse)
		return false;

	const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
	const auto pCountry = pHouse->Type;
	switch (pType->WhatAmI())
	{
	case AbstractType::InfantryType:
	{
		if (pHouse->BarracksInfiltrated && !pType->Naval && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranInfantry.FindItemIndex(static_cast<InfantryTypeClass*>(pType)) != -1;

	}
	case AbstractType::UnitType:
	{
		if (pHouse->WarFactoryInfiltrated && !pType->Naval && pType->Trainable)
		{
			return true;
		}
		else if (pHouseExt->Is_NavalYardSpied && pType->Naval && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranUnits.Contains((UnitTypeClass*)pType);
	}
	case AbstractType::AircraftType:
	{
		if (pHouseExt->Is_AirfieldSpied && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranAircraft.Contains((AircraftTypeClass*)(pType));
	}
	case AbstractType::BuildingType:
	{
		if (auto const pItem = pType->UndeploysInto)
		{
			return pCountry->VeteranUnits.Contains((UnitTypeClass*)(pItem));
		}
		else if (pHouseExt->Is_ConstructionYardSpied && pType->Trainable)
		{
			return true;
		}

		return HouseTypeExt::ExtMap.Find(pCountry)->VeteranBuildings.Contains((BuildingTypeClass*)(pType));
	}
	}

	return false;
}

bool NOINLINE TechnoExt_ExtData::IsOperated(TechnoClass* pThis)
{
	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt->Operators.empty())
	{
		if (pExt->Operator_Any)
			return pThis->Passengers.GetFirstPassenger() != nullptr;

		pThis->align_154->Is_Operated = true;
		return true;
	}
	else
	{
		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (pExt->Operators.Contains((TechnoTypeClass*)object->GetType()))
			{
				// takes a specific operator and someone is present AND that someone is the operator, therefore it is operated
				return true;
			}
		}
	}

	return false;
}

bool TechnoExt_ExtData::IsOperatedB(TechnoClass* pThis)
{
	return pThis->align_154->Is_Operated || TechnoExt_ExtData::IsOperated(pThis);
}

bool NOINLINE TechnoExt_ExtData::IsPowered(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();

	if (pType && pType->PoweredUnit)
	{
		for (const auto& pBuilding : pThis->Owner->Buildings)
		{
			if (pBuilding->Type->PowersUnit == pType
				&& pBuilding->RegisteredAsPoweredUnitSource
				&& !pBuilding->IsUnderEMP()) // alternatively, HasPower, IsPowerOnline()
			{
				return true;
			}
		}
		// if we reach this, we found no building that currently powers this object
		return false;
	}
	else if (auto pPower = pThis->align_154->PoweredUnitUptr)
	{
		// #617
		return pPower->Powered;
	}

	// object doesn't need a particular powering structure, therefore, for the purposes of the game, it IS powered
	return true;
}

void TechnoExt_ExtData::EvalRaidStatus(BuildingClass* pThis)
{
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	// if the building is still marked as raided, but unoccupied, return it to its previous owner
	if (pExt->OwnerBeforeRaid && !pThis->Occupants.Count)
	{
		// Fix for #838: Only return the building to the previous owner if he hasn't been defeated
		if (!pExt->OwnerBeforeRaid->Defeated)
		{
			pThis->SetOwningHouse(pExt->OwnerBeforeRaid, false);
		}

		pExt->OwnerBeforeRaid = nullptr;
	}
}