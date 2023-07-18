#include <NetworkEvents.h>

#include <Ext/Building/Body.h>

#include <HouseClass.h>

#include <NetworkEvents.h>
#include <Networking.h>

#include <Misc/AresData.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include "Header.h"

// was desynct all time
// not sure WTF is happening
namespace Fix
{
	static constexpr reference<uint8_t, 0x8208ECu,46u> const EventLengthArr {};

	unsigned int EventLength_Hares(int nInput)
	{
		if (nInput == 0x80)
			return 10;
		if (nInput == 0x81 || nInput == 0x82)
			return 5;

		return NetworkEvent::EventLength[nInput];
	}

	int NOINLINE EventLength(uint8_t nInput)
	{
		if (nInput <= 0x2Eu) // default event
			return Fix::EventLengthArr[nInput];
		if (nInput == 0x60u) // TrenchRedirectClick
			return 10;
		if (nInput == 0x61u) // FirewallToggle
			return 5;

		return 0;
	}
};

//DEFINE_OVERRIDE_HOOK(0x64C314, sub_64BDD0_PayloadSize2, 0x8)
//{
//	GET(uint8_t, nSize, ESI);
//
//	const auto nFix = Fix::EventLength(nSize);
//
//	R->ECX(nFix);
//	R->EBP(nFix + (nSize == 4u));
//
//	return 0x64C321;
//}
//
//DEFINE_OVERRIDE_HOOK(0x64BE83, sub_64BDD0_PayloadSize1, 0x8)
//{
//	GET(uint8_t, nSize, EDI);
//
//	const auto nFix = Fix::EventLength(nSize);
//
//	R->ECX(nFix);
//	R->EBP(nFix);
//	R->Stack(0x20, nFix);
//
//	return nSize == 4 ? 0x64BF1A : 0x64BE97;
//}
//
//DEFINE_OVERRIDE_HOOK(0x64B704, sub_64B660_PayloadSize, 0x8)
//{
//	GET(uint8_t, nSize, EDI);
//
//	const auto nFix = Fix::EventLength(nSize);
//
//	R->EDX(nFix);
//	R->EBP(nFix);
//
//	return (nSize == 0x1Fu) ? 0x64B710 : 0x64B71D;
//}

class AresNetEvent {
public:
	enum class Events : unsigned char {
		TrenchRedirectClick = 0x60,
		FirewallToggle = 0x61,

		First = TrenchRedirectClick,
		Last = FirewallToggle
	};

	class Handlers {
	public:
		static void RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target);
		static void RespondToTrenchRedirectClick(NetworkEvent *Event);

		static void RaiseFirewallToggle(HouseClass *Source);
		static void RespondToFirewallToggle(NetworkEvent *Event);
	};
};

/*
 how to raise your own events
	NetworkEvent Event;
	Event.Kind = AresNetworkEvent::aev_blah;
	Event.HouseIndex = U->Owner->ArrayIndex;
	memcpy(Event.ExtraData, "Boom de yada", 0xkcd);
	Networking::AddEvent(&Event);
*/

void AresNetEvent::Handlers::RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target) {
	NetworkEvent Event {};

	if(Source->Owner->ArrayIndex >= 0){
		Event.Kind = NetworkEventType(AresNetEvent::Events::TrenchRedirectClick);
		Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	}

	byte *ExtraData = Event.ExtraData;

	NetID TargetCoords {};

	TargetCoords.Pack(Target);
	memcpy(ExtraData, &TargetCoords, sizeof(TargetCoords));
	ExtraData += sizeof(TargetCoords);

	NetID SourceObject{};

	SourceObject.Pack(Source);
	memcpy(ExtraData, &SourceObject, sizeof(SourceObject));
	ExtraData += sizeof(SourceObject);
	//the data is an array containing 2 stuffs
	Networking::AddEvent(&Event);
}

bool NOINLINE IsSameTrech(BuildingClass* currentBuilding , BuildingClass* targetBuilding)
{
	auto pThisTypeExt = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	if(pThisTypeExt->IsTrench <= 0) {
		return false;
	}

	return pThisTypeExt->IsTrench == BuildingTypeExt::ExtMap.Find(targetBuilding->Type)->IsTrench;
}

bool NOINLINE canTraverseTo(BuildingClass* currentBuilding , BuildingClass* targetBuilding) {
	if(targetBuilding != currentBuilding) {
		BuildingTypeClass* pTargetType = targetBuilding->Type;
		if (pTargetType->CanBeOccupied && targetBuilding->Occupants.Count < pTargetType->MaxNumberOccupants) {
			if(currentBuilding->Occupants.Count && IsSameTrech(currentBuilding , targetBuilding)) {
				if(targetBuilding->Location.DistanceFrom(currentBuilding->Location) <= 256.0)
					return true;
			}
		}
	}

	return false;
}

void NOINLINE doTraverseTo(BuildingClass* currentBuilding ,BuildingClass* targetBuilding) {
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	// depending on Westwood's handling, this could explode when Size > 1 units are involved...but don't tell the users that
	while(currentBuilding->Occupants.Count && (targetBuilding->Occupants.Count < targetBuildingType->MaxNumberOccupants)) {
		targetBuilding->Occupants.AddItem(currentBuilding->Occupants.GetItem(0));
		currentBuilding->Occupants.RemoveAt(0); // maybe switch Add/Remove if the game gets pissy about multiple of them walking around
	}

	// fix up firing index, as decrementing the source occupants can invalidate it
	if(currentBuilding->FiringOccupantIndex >= currentBuilding->GetOccupantCount()) {
		currentBuilding->FiringOccupantIndex = 0;
	}

	TechnoExt_ExtData::EvalRaidStatus(currentBuilding); // if the traversal emptied the current building, it'll have to be returned to its owner
}

void AresNetEvent::Handlers::RespondToTrenchRedirectClick(NetworkEvent *Event) {
	NetID *ID = reinterpret_cast<NetID *>(Event->ExtraData);
	if(CellClass * pTargetCell = ID->UnpackCell()) {
		++ID;
		if(BuildingClass * pSourceBuilding = ID->UnpackBuilding()) {
			/*
				pSourceBuilding == selected building the soldiers are in
				pTargetCell == cell the user clicked on; event fires only on buildings which showed the enter cursor
			*/
			BuildingClass* targetBuilding = pTargetCell->GetBuilding();
			if(canTraverseTo(pSourceBuilding, targetBuilding)) // check has happened before the enter cursor appeared
				doTraverseTo(pSourceBuilding , targetBuilding);
		}
	}
}

void AresNetEvent::Handlers::RaiseFirewallToggle(HouseClass *Source) {
	NetworkEvent Event;
	Event.Kind = static_cast<NetworkEventType>(AresNetEvent::Events::FirewallToggle);
	Event.HouseIndex = byte(Source->ArrayIndex);

	Networking::AddEvent(&Event);
}

void AresNetEvent::Handlers::RespondToFirewallToggle(NetworkEvent *Event) {
	if(HouseClass * pSourceHouse = HouseClass::Array->GetItem(Event->HouseIndex)) {
		AresData::AresNetEvent_Handlers_RespondToFirewallToggle(pSourceHouse , !pSourceHouse->FirestormActive);
	}
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_OVERRIDE_HOOK(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = specific_cast<BuildingClass*>(T)) {
		if(canTraverseTo(pThis ,targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
			R->EAX(Action::Enter);
			return 0x447273;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x443414, BuildingClass_ActionOnObject, 6)
{
	GET(Action, action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	// part of deactivation logic
	if(pThis->Deactivated) {
		R->EAX(1);
		return 0x44344D;
	}

	// trenches
	if(action == Action::Enter) {
		if(BuildingClass *pTargetBuilding = specific_cast<BuildingClass *>(pTarget)) {
			CoordStruct XYZ = pTargetBuilding->GetCoords();
			CellStruct tgt = CellClass::Coord2Cell(XYZ);
			AresNetEvent::Handlers::RaiseTrenchRedirectClick(pThis, &tgt);
			R->EAX(1);
			return 0x44344D;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4C6CCD, Networking_RespondToEvent, 0)
{
	GET(DWORD, EventKind, EAX);
	GET(NetworkEvent *, Event, ESI);

	auto kind = static_cast<AresNetEvent::Events>(EventKind);
	if(kind >= AresNetEvent::Events::First) {
		// Received Ares event, do something about it
		switch(kind) {
			case AresNetEvent::Events::TrenchRedirectClick:
				AresNetEvent::Handlers::RespondToTrenchRedirectClick(Event);
				break;
			case AresNetEvent::Events::FirewallToggle:
				AresNetEvent::Handlers::RespondToFirewallToggle(Event);
				break;
		}
	}

	--EventKind;
	R->EAX(EventKind);
	return (EventKind > 0x2D)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}
