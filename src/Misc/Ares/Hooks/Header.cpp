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

void AresNetEvent::Handlers::RaiseRevealMap(HouseClass* pSource)
{
	Debug::Log("Sending RevealMap to all clients\n");
	//NetworkEvent Event;
	//Event.Kind = static_cast<NetworkEventType>(AresNetEvent::Events::Revealmap);
	//Event.HouseIndex = byte(pSource->ArrayIndex);
	//Networking::AddEvent(&Event);
}

void AresNetEvent::Handlers::RespondRevealMap(NetworkEvent* Event)
{
	Debug::Log("Receiving RevealMap from a clients\n");
	//if (HouseClass* pSourceHouse = HouseClass::Array->GetItem(Event->HouseIndex))
	//{
	//	pSourceHouse->Visionary = true; //sync the state with other clients
	//}
}

//TODO : proper owner for the event ?
void NOINLINE AresNetEvent::Handlers::RaiseSetDriverKilledStatusToTrue(TechnoClass* Current)
{
	NetworkEvent Event;
	Debug::Log("Sending RaiseSetDriverKilledStatusToTrue to all clients\n");
	if (Current->Owner->ArrayIndex >= 0)
	{
		Event.Kind = NetworkEventType(AresNetEvent::Events::SetDriverKilledStatusToTrue);
		Event.HouseIndex = byte(Current->Owner->ArrayIndex);
	}

	byte* ExtraData = Event.ExtraData;
	NetID SourceObject {};
	SourceObject.Pack(Current);
	memcpy(ExtraData, &SourceObject, sizeof(SourceObject));
	ExtraData += sizeof(SourceObject);

	Networking::AddEvent(&Event);

}

void NOINLINE AresNetEvent::Handlers::ResponseToSetDriverKilledStatusToTrue(NetworkEvent* Event)
{
	NetID* ID = reinterpret_cast<NetID*>(Event->ExtraData);
	Debug::Log("Receiving ResponseToSetDriverKilledStatusToTrue from a clients\n");

	if (auto pTechno = ID->UnpackTechno())
	{
		pTechno->align_154->Is_DriverKilled = true;
	}
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