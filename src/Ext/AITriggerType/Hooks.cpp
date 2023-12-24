#include <Utilities/Macro.h>
#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>

// TODO
static bool CheckPrereq(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pHouse2)
{
	if (const auto pItem = pThis->ConditionObject)
		return HouseExtData::PrereqValidate(pHouse2, pItem, false, true) == CanBuildResult::Buildable;

	return false;
}

static bool CheckBridgeCondition(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pHouse2) {
	if (auto const pCiv = HouseClass::FindBySideIndex(RulesExtData::Instance()->CivilianSideIndex)) {
		return pCiv->Buildings.any_of([](BuildingClass* const pBld) {
			return pBld->Type->BridgeRepairHut && pBld->Type->Repairable && MapClass::Instance->IsBrideRepairNeeded(pBld->InlineMapCoords());
		});
	}

	return false;
}

DEFINE_HOOK_AGAIN(0x41E8DD, Phobos_AITrigger_Handler, 0x8)
DEFINE_HOOK(0x41E8F0, Phobos_AITrigger_Handler, 0x8)
{
	GET(AITriggerTypeClass*, pAITriggerType, ESI);
	GET(HouseClass*, pHouse, EDI);

	// ES Stuffs
	if(R->Origin() == 0x41E8F0) {

		GET(HouseClass*, pHouse2, EBX);

		if (pHouse2) {
			switch (static_cast<PhobosAINewConditionTypes>(pAITriggerType->ConditionType))
			{
			case PhobosAINewConditionTypes::CheckPrereq: //it seems to check prereq stuffs
			{
				R->AL(CheckPrereq(pAITriggerType,pHouse,pHouse2));
				return 0x41E9C7;
			}
			}
		}
	}

	//get Condition String
	//char ConditionString[68];
	//int idx = 0;
	//char* condStr = ConditionString;
	//auto buf = reinterpret_cast<const byte*>(&pAITriggerType->Conditions);
	//do
	//{
	//	sprintf_s(condStr, 4, "%02x", *buf);
	//	++buf;
	//	++idx;
	//	condStr += 2;
	//}
	//while (idx < 0x20);
	//*condStr = '\0';
	//
	////this is when Phobos strats to work
	//if ((pAITriggerType->ConditionType == AITriggerCondition::Pool || pAITriggerType->ConditionType == AITriggerCondition::AIOwns)
	//	&& pAITriggerType->ConditionObject == nullptr)
	//{
	//	const std::string ConditionString2 = ConditionString;
	//	int PhobosAIConditionType = atoi(ConditionString2.substr(56, 4).c_str());
	//	int PhobosAIConditionList = atoi(ConditionString2.substr(60, 4).c_str());
	//
	//	if (PhobosAIConditionType > 0)
	//	{
	//		AITriggerTypeExt::ProcessCondition(pAITriggerType, pHouse, PhobosAIConditionType, PhobosAIConditionList);
	//	}
	//}

	return 0;
}
