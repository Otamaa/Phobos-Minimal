#include "Body.h"

#include <NetworkEvents.h>

enum class PhobosNetworkEvents : int
{
	RepairAll = 0x200 ,
	NextPendingBuilding = 0x201 ,
	Count
};

struct NetworkEventExt
{
	static std::pair<bool , const char*> GetEventName(int nEvent)
	{
		switch (static_cast<PhobosNetworkEvents>(nEvent))
		{
		case PhobosNetworkEvents::RepairAll:
			return { true , "REPAIR ALL" };
		case PhobosNetworkEvents::NextPendingBuilding:
			return { true , "NEXT PENDING BUILDING" };
		default:
			return { false , NONE_STR };
			break;
		}
	}

	static bool Execute(NetworkEvent* pEvent)
	{
		const auto pHouse = HouseClass::Array->GetItem(pEvent->HouseIndex);
		switch (static_cast<PhobosNetworkEvents>(pEvent->Kind))
		{
		case PhobosNetworkEvents::RepairAll:
		{
			HouseExtContainer::Instance.Find(pHouse)->AllRepairEventRiggered = true; //where it is cleared ?
			return true;
		}
		break;
		case PhobosNetworkEvents::NextPendingBuilding:
		{
			return true;
		}
			break;
		}

		return false;
	}
};

DEFINE_HOOK_AGAIN(0x4C672F , NetworkEvent_GetNameString, 0x7)
DEFINE_HOOK_AGAIN(0x4C65EF , NetworkEvent_GetNameString, 0x7)
DEFINE_HOOK(0x4C66CF, NetworkEvent_GetNameString, 0x7)
{
	GET(int, nEvent, EBX);

	auto const& [New, Name] = NetworkEventExt::GetEventName(nEvent);

	if (New) {
		R->ECX(Name);
		return R->Origin() + 0x7;
	}

	return 0x0;
}

DEFINE_HOOK(0x4C6CC4, NetworkEvent_Execute, 0x6)
{
	return NetworkEventExt::Execute(R->ESI<NetworkEvent*>()) ? 0x4C6CE5 : 0x0;
}

DEFINE_HOOK(0x45064B, HouseClass_RepairAI_AutoRepair, 0x6)
{
	GET(HouseClass*, pThis, EAX);

	if(auto pExt = HouseExtContainer::Instance.Find(pThis))
		if(pExt->AllRepairEventRiggered)
			return 0x450659;

	return 0x0;
}