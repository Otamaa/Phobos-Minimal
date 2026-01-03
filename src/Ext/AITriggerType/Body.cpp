#include "Body.h"

#include <string>
#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>


// =============================
// container
AITriggerTypeExt::ExtContainer AITriggerTypeExt::ExtMap;

void AITriggerTypeExt::ProcessCondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int type, int condition)
{
	//AITriggerType is disabled by default
	DisableAITrigger(pAITriggerType);
	switch (static_cast<PhobosAIConditionTypes>(type))
	{
	case PhobosAIConditionTypes::CustomizableAICondition:
		AITriggerTypeExt::CustomizableAICondition(pAITriggerType, pHouse, condition);
		break;
	default:
		break;
	}
	return;
}

void AITriggerTypeExt::DisableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::AIOwns;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

void AITriggerTypeExt::EnableAITrigger(AITriggerTypeClass* pAITriggerType)
{
	pAITriggerType->ConditionType = AITriggerCondition::Pool;
	pAITriggerType->ConditionObject = nullptr;
	return;
}

bool AITriggerTypeExt::ReadCustomizableAICondition(HouseClass* pHouse, int pickMode, int compareMode, int Number, TechnoTypeClass* TechnoType)
{
	//0 = pick enemies(except for neutral); 1 = pick allies(except for neutral); 2 = pick self; 3 = pick all(except for neutral);
	//4 = pick enemy human players; 5 = pick allied human players; 6 = pick all human players;
	//7 = pick enemy computer players(except for neutral); 8 = pick allied computer players(except for neutral); 9 = pick all computer players(except for neutral);
	//10 = pick neutral; 11 = pick all(including neutral);
	//int pickMode;

	//0 = "<"; 1 = "<="; 2 = "=="; 3 = ">="; 4 = ">"; 5 = "!=";
	//int compareMode;

	int count = 0;

	std::ranges::for_each(*TechnoClass::Array, [&](const TechnoClass* pTechno) {
		if (GET_TECHNOTYPE(pTechno) == TechnoType
			&& pTechno->IsAlive
			&& !pTechno->InLimbo
			&& pTechno->IsOnMap
			&& !pTechno->Absorbed
			&& pTechno->Owner
			&& ((!pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 0)
				|| (pTechno->Owner->IsAlliedWith(pHouse) && !pTechno->Owner->IsNeutral() && pickMode == 1)
				|| (pTechno->Owner == pHouse && pickMode == 2)
				|| (!pTechno->Owner->IsNeutral() && pickMode == 3)
				|| (pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 4)
				|| (pTechno->Owner->IsControlledByHuman() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 5)
				|| (pTechno->Owner->IsControlledByHuman() && pickMode == 6)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && !pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 7)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && pTechno->Owner->IsAlliedWith(pHouse) && pickMode == 8)
				|| (!pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsNeutral() && pickMode == 9)
				|| (pTechno->Owner->IsNeutral() && pickMode == 10)
				|| (pickMode == 11)
				))

		{
			count++;
		}
	});

	return ((count < Number && compareMode == 0)
		|| (count <= Number && compareMode == 1)
		|| (count == Number && compareMode == 2)
		|| (count >= Number && compareMode == 3)
		|| (count > Number && compareMode == 4)
		|| (count != Number && compareMode == 5)
		);
}

void AITriggerTypeExt::CustomizableAICondition(AITriggerTypeClass* pAITriggerType, HouseClass* pHouse, int condition)
{
	auto& AIConditionsLists = RulesExtData::Instance()->AIConditionsLists;

	int essentialRequirementsCount = -1;
	int leastOptionalRequirementsCount = -1;
	int essentialRequirementsMetCount = 0;
	int optionalRequirementsMetCount = 0;

	if ((size_t)condition < AIConditionsLists.size())
	{
		auto& thisAICondition = AIConditionsLists[condition];

		if (thisAICondition.size() < 2)
		{
			pAITriggerType->IsEnabled = false;
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
			return;
		}

		//parse first string
		char* context = nullptr;
		char* cur[3] {};
		cur[0] = strtok_s(thisAICondition[0].data(), Phobos::readDelims, &context);
		int j = 0;
		while (cur[j])
		{
			j++;
			cur[j] = strtok_s(NULL, Phobos::readDelims, &context);
		}

		if (cur[0])
			essentialRequirementsCount = atoi(cur[0]);
		else
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Essential Requirements Count [0] !.");

		if (cur[1])
			leastOptionalRequirementsCount = atoi(cur[1]);
		else
			Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Least Optional Requirements Count [1] !.");

		//parse other strings
		for (int i = 1; i < (int)thisAICondition.size(); i++)
		{
			int pickMode = -1;
			int compareMode = -1;
			int Number = -1;
			TechnoTypeClass* TechnoType;

			char* cur2[5] {};
			cur2[0] = strtok_s(thisAICondition[i].data(), Phobos::readDelims, &context);
			int k = 0;
			while (cur2[k])
			{
				k++;
				cur2[k] = strtok_s(NULL, Phobos::readDelims, &context);
			}
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur2[3], &buffer))
			{
				if (cur2[0])
					pickMode = atoi(cur2[0]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Pick [0] !.");

				if (cur2[1])
					compareMode = atoi(cur2[1]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Compare [1] !.");

				if (cur2[2])
					Number = atoi(cur2[2]);
				else
					Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition : [AIConditionsList]: Error parsing Number [2] !.");

				TechnoType = buffer;
			}
			else
			{
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList][{}]: Error parsing [{}]", condition, cur2[3]);
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}

			if (essentialRequirementsCount > -1
				&& leastOptionalRequirementsCount > -1
				&& essentialRequirementsCount + leastOptionalRequirementsCount < (int)thisAICondition.size()
				&& pickMode >= 0 && pickMode <= 11
				&& compareMode >= 0 && compareMode <= 5
				&& Number >= 0)
			{
				//essential requirements judgment
				if (i <= essentialRequirementsCount)
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						essentialRequirementsMetCount++;
				}
				//optional requirements judgment
				else
				{
					if (ReadCustomizableAICondition(pHouse, pickMode, compareMode, Number, TechnoType))
						optionalRequirementsMetCount++;
				}
			}
			else
			{
				Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Error parsing line [{}].", condition);
				pAITriggerType->IsEnabled = false;
				return;
			}
		}
	}
	else
	{
		//thoroughly disable it
		pAITriggerType->IsEnabled = false;
		Debug::LogInfo("AITriggerTypeExt - CustomizableAICondition: [AIConditionsList]: Condition number overflew!.");
		return;
	}
	if (essentialRequirementsCount == essentialRequirementsMetCount && leastOptionalRequirementsCount <= optionalRequirementsMetCount)
		EnableAITrigger(pAITriggerType);

	return;
}

//ASMJIT_PATCH(0x41E471, AITriggerTypeClass_CTOR, 0x7)
//{
//	GET(AITriggerTypeClass*, pThis, ESI);
//	AITriggerTypeExt::ExtMap.Allocate(pThis);
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x41E4AF, AITriggerTypeClass_DTOR, 0x6)
//{
//	GET(AITriggerTypeClass*, pThis, ESI);
//	AITriggerTypeExt::ExtMap.Remove(pThis);
//	return 0x0;
//}
//
//ASMJIT_PATCH_AGAIN(0x41E540 , AITriggerTypeClass_SaveLoad_Prefix, 0x5)
//ASMJIT_PATCH(0x41E5C0, AITriggerTypeClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(AITriggerTypeClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	AITriggerTypeExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//// BEfore -> 41E5A1
//// After -> 41E5B2
//// Better -> 41E5A1
//ASMJIT_PATCH(0x41E5B2, AITriggerTypeClass_Load_Suffix, 0x6)
//{
//	AITriggerTypeExt::ExtMap.LoadStatic();
//	return 0;
//}
//// BEfore -> 41E5DA
//// After -> 41E5D8
//// Better -> 41E5D4
//ASMJIT_PATCH(0x41E5D8, AITriggerTypeClass_Save_Suffix, 0x5)
//{
//	AITriggerTypeExt::ExtMap.SaveStatic();
//	return 0;
//}