#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

#include <string>
// =============================
// container
AITriggerTypeExtContainer AITriggerTypeExtContainer::Instance;

int AITriggerTypeExtData::CheckConditions(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	int condition = (int)pThis->ConditionType;

	if (condition < -1) // Invalid, bail out early.
		return 0;
	else if (condition < (int)PhobosAINewConditionTypes::NumberOfTechBuildingsExist) // Not Phobos condition
		return -1;

	bool success = false;

	switch ((PhobosAINewConditionTypes)condition)
	{
	case PhobosAINewConditionTypes::NumberOfTechBuildingsExist:
		success = AITriggerTypeExtData::NumberOfTechBuildingsExist(pThis, pOwner);
		break;
	case PhobosAINewConditionTypes::NumberOfBridgeRepairHutsExist:
		success = AITriggerTypeExtData::NumberOfBridgeRepairHutsExist(pThis);
		break;
	}

	return success;
}

bool AITriggerTypeExtData::GetComparatorResult(int operand1, AITriggerConditionComparatorType operatorType, int operand2)
{
	switch (operatorType)
	{
	case AITriggerConditionComparatorType::Less:
		return operand1 < operand2;
		break;
	case AITriggerConditionComparatorType::LessOrEqual:
		return operand1 <= operand2;
		break;
	case AITriggerConditionComparatorType::Equal:
		return operand1 == operand2;
		break;
	case AITriggerConditionComparatorType::GreaterOrEqual:
		return operand1 >= operand2;
		break;
	case AITriggerConditionComparatorType::Greater:
		return operand1 > operand2;
		break;
	case AITriggerConditionComparatorType::NotEqual:
		return operand1 != operand2;
		break;
	default:
		return false;
		break;
	}
}

bool AITriggerTypeExtData::NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner)
{
	int count = 0;

	for (auto const pHouse : *HouseClass::Array)
	{
		if (pHouse->IsAlliedWith(pOwner))
			continue;

		// Could possibly be optimized with bespoke tracking but
		// it didn't seem to make much of a difference in testing.
		for (auto const pBuilding : pHouse->Buildings)
		{
			if (!pBuilding->IsAlive || pBuilding->InLimbo)
				continue;

			auto const pType = pBuilding->Type;

			if (pType->NeedsEngineer && pType->Capturable)
				count++;
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0].Type, pThis->Conditions[0].Operand);
}

bool AITriggerTypeExtData::NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis)
{
	int count = 0;
	auto const pHouse = HouseClass::FindCivilianSide();

	for (auto const pBuilding : pHouse->Buildings)
	{
		if (!pBuilding->IsAlive || pBuilding->InLimbo)
			continue;

		auto const pType = pBuilding->Type;

		if (pType->BridgeRepairHut && MapClass::Instance->IsLinkedBridgeDestroyed(pBuilding->GetMapCoords()))
			count++;
	}

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0].Type, pThis->Conditions[0].Operand);
}

template <typename T>
void AITriggerTypeExtData::Serialize(T& Stm)
{

}

#ifdef _NOT

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

#endif

void AITriggerTypeExtContainer::LoadFromINI(AITriggerTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		// Rules first 
		// Other files 
		// when this doesnt match the case it will causing weirdd issues like some value wont be initialized or replaced to default value after parsing
		switch (ptr->Initialized)
		{
		case InitState::Blank:
		{
			if (pINI == CCINIClass::INI_Rules())
			{
				ptr->SetInitState(InitState::Inited);
				//ptr->Initialize();
			}
			[[fallthrough]];
		}
		case InitState::Inited:
		case InitState::Ruled:
		{
			ptr->LoadFromINI(pINI, parseFailAddr);
			ptr->SetInitState(InitState::Ruled);
			[[fallthrough]];
		}
		default:
			break;
		}
	}

}

void AITriggerTypeExtContainer::WriteToINI(AITriggerTypeClass* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}


ASMJIT_PATCH(0x41E471, AITriggerTypeClass_CTOR, 0x7)
{
	GET(AITriggerTypeClass*, pThis, ESI);
	AITriggerTypeExtContainer::Instance.FindOrAllocate(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x41E4AF, AITriggerTypeClass_DTOR, 0x6)
{
	GET(AITriggerTypeClass*, pThis, ESI);
	AITriggerTypeExtContainer::Instance.Remove(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x41E8FF, AITriggerTypeClass_NewTeam_CheckConditions, 0x9) // ConditionMet() in YRpp
{
	enum { ContinueGameChecks = 0x41E908, ReturnFromFunction = 0x41E9E1, Success = 0x41E9EA };

	GET(AITriggerTypeClass*, pThis, ESI);
	GET(HouseClass*, pOwner, EDI);
	GET(HouseClass*, pEnemy, EBX);

	int result = AITriggerTypeExtData::CheckConditions(pThis, pOwner, pEnemy);

	switch (result)
	{
	case 0:
		return ReturnFromFunction;
		break;
	case 1:
		return Success;
		break;
	default:
		return ContinueGameChecks;
		break;
	}
}