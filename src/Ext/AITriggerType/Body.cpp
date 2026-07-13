#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <string>
// =============================
// container
AITriggerTypeExtContainer AITriggerTypeExtContainer::Instance;

int AITriggerTypeExtData::CountOwnedType(TechnoTypeClass* pType, HouseClass* pHouse)
{
	if (!pType)
		return 0;

	const int heapID = pType->GetArrayIndex();

	switch (pType->WhatAmI())
	{
	case AbstractType::AircraftType:
		return pHouse->ActiveAircraftTypes.get_count(heapID);
	case AbstractType::BuildingType:
	{
		// Fix the issue that AITriggerTypes do not recognize building upgrades
		// Author: Uranusian
		//0x41EB43, AITriggerTypeClass_Condition_SupportPowersup, 0x7
		//0x41EEE3, AITriggerTypeClass_Condition_SupportPowersup, 0x7
		const auto pType = BuildingTypeClass::Array->Items[heapID];
		int count = BuildingTypeExtData::GetUpgradesAmount(pType, pHouse);

		if (count == -1)
			count = pHouse->ActiveBuildingTypes.get_count(heapID);

		if (count == -1)
			count = pHouse->ActiveBuildingTypes.get_count(heapID);

		return count;
	}
	case AbstractType::InfantryType:
		return pHouse->ActiveInfantryTypes.get_count(heapID);
	case AbstractType::UnitType:
		return pHouse->ActiveUnitTypes.get_count(heapID);
	default:
		return 0;
	}
}

bool AITriggerTypeExtData::SuperWeaponNearReady(HouseClass* pHouse, int swTypeIndex)
{
	if (!pHouse)
		return false;

	const int count = pHouse->Supers.Count;
	if (count <= 0)
		return false;

	SuperClass* pSW = nullptr;

	for (int i = 0; i < count; ++i)
	{
		auto pCand = pHouse->Supers.Items[i];

		if (!pCand)
			continue;

		if (pCand->Type->ArrayIndex == swTypeIndex)
		{
			if (!SWTypeExtData::IsAvailable(pHouse, pCand))
				continue;

			pSW = pCand;
			break;
		}
	}

	if (!pSW || !pSW->Granted)
		return false;

	// Assembly timer block — no goto version:
	int timeLeft = 0;
	const int started = pSW->RechargeTimer.StartTime;
	const int delayTime = pSW->RechargeTimer.TimeLeft;

	if (started == -1)
	{
		// Not yet started — full delay remaining.
		timeLeft = delayTime;
	}
	else
	{
		const int elapsed = Unsorted::CurrentFrame() - started;
		if (elapsed < delayTime)
			timeLeft = delayTime - elapsed;
		// else: elapsed >= delayTime → timeLeft stays 0 (fully charged)
	}

	const int rechargeTime = pSW->GetRechargeTime();

	const float readyThreshold = 1.0f - RulesClass::Instance->AIMinorSuperReadyPercent;
	const float chargeProgress = (rechargeTime > 0)
		? static_cast<float>(timeLeft) / static_cast<float>(rechargeTime)
		: 0.0f;

	return readyThreshold >= chargeProgress;
}

bool AITriggerTypeExtData::GetComparatorResult(int operand1, AITriggerConditionComparator& cond)
{
	switch (cond.Type)
	{
	case AITriggerConditionComparatorType::Less:
		return operand1 < cond.Operand;
	case AITriggerConditionComparatorType::LessOrEqual:
		return operand1 <= cond.Operand;
	case AITriggerConditionComparatorType::Equal:
		return operand1 == cond.Operand;
	case AITriggerConditionComparatorType::GreaterOrEqual:
		return operand1 >= cond.Operand;
	case AITriggerConditionComparatorType::Greater:
		return operand1 > cond.Operand;
	case AITriggerConditionComparatorType::NotEqual:
		return operand1 != cond.Operand;
	default:
		return false;
	}
}

bool AITriggerTypeExtData::OwnStuffs(TechnoTypeClass* pItem, TechnoClass* list)
{
	if (auto pItemUnit = type_cast<UnitTypeClass*, false>(pItem))
	{
		if (auto pListBld = cast_to<BuildingClass*, false>(list))
		{
			if (pItemUnit->DeploysInto == pListBld->Type)
				return true;

			if (pListBld->Type->UndeploysInto == pItemUnit)
				return true;
		}
	}

	if (auto pItemUnit = type_cast<BuildingTypeClass*, false>(pItem))
	{
		if (auto pListBld = cast_to<UnitClass*, false>(list))
		{
			if (pItemUnit->UndeploysInto == pListBld->Type)
				return true;

			if (pListBld->Type->DeploysInto == pItemUnit)
				return true;
		}
	}

	//check type
	//return TechnoExtContainer::Instance.Find(list)->CurrentType == pItem || list->GetTechnoType() == pItem;
	return TeamExtData::IsEligible(GET_TECHNOTYPE(list), pItem);
}

bool AITriggerTypeExtData::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;

}

bool AITriggerTypeExtData::IsValidTechno(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	bool isValid = !pTechno->Dirty
		&& IsUnitAvailable(pTechno, true)
		&& pTechno->Owner
		&& (pTechno->WhatAmI() == AbstractType::Infantry
			|| pTechno->WhatAmI() == AbstractType::Unit
			|| pTechno->WhatAmI() == AbstractType::Building
			|| pTechno->WhatAmI() == AbstractType::Aircraft);

	return isValid;
}

bool AITriggerTypeExtData::CountConditionMet(AITriggerTypeClass* pThis, int nObjects)
{
	bool result = true;

	if (nObjects < 0)
		return false;

	return AITriggerTypeExtData::GetComparatorResult(nObjects, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& OwnStuffs(pItem, pObject))
			{
				counter++;
			}
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, TechnoTypeClass* pItem)
{
	int counter = 0;

	// Count all objects of the list, like an OR operator

	for (auto pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject) || !pObject->Owner) continue;

		if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
			&& !pObject->Owner->Type->MultiplayPassive
			&& OwnStuffs(pItem, pObject))
		{
			counter++;
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, TechnoTypeClass* pItem)
{
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator

	for (auto const pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject) || !pObject->Owner) continue;

		if (pObject->Owner != pHouse
			&& (!pEnemy || !pHouse->IsAlliedWith(pObject->Owner))
			&& !pObject->Owner->Type->MultiplayPassive
			&& OwnStuffs(pItem, pObject))
		{
			counter++;
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto const pItem : list)
	{
		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || !pHouse->IsAlliedWith(pObject->Owner))
				&& !pObject->Owner->Type->MultiplayPassive
				&& OwnStuffs(pItem, pObject))
			{
				counter++;
			}
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::NeutralOwns(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;
	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	// Count all objects of the list, like an OR operator
	for (auto const pItem : list)
	{
		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (pObject->Owner == pCiv && OwnStuffs(pItem, pObject))
				counter++;
		}
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::NeutralOwns(AITriggerTypeClass* pThis, TechnoTypeClass* pItem)
{
	int counter = 0;
	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	for (auto const pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject) || !pObject->Owner) continue;

		if (pObject->Owner == pCiv && GET_TECHNOTYPE(pObject) == pItem)
			counter++;
	}

	return AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (pObject->Owner == pHouse && GET_TECHNOTYPE(pObject) == pItem)
				counter++;
		}

		result = AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
	}

	return result;
}

bool AITriggerTypeExtData::EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || !pHouse->IsAlliedWith(pObject->Owner))
				&& !pObject->Owner->Type->MultiplayPassive
				&& GET_TECHNOTYPE(pObject) == pItem)
			{
				counter++;
			}
		}

		result = AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
	}

	return result;
}

bool AITriggerTypeExtData::NeutralOwnsAll(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		int counter = 0;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject) || !pObject->Owner) continue;

			if (pObject->Owner == pCiv && GET_TECHNOTYPE(pObject) == pItem)
				counter++;
		}

		result = AITriggerTypeExtData::GetComparatorResult(counter, pThis->Conditions[0]);
	}

	return result;
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

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0]);
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

	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pEnemy)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pEnemy);
	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pOwner)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pOwner);
	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::NeutralOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	HouseClass* pCivilian = HouseExtData::FindFirstCivilianHouse();
	if (!pCivilian)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pCivilian);
	return AITriggerTypeExtData::GetComparatorResult(count, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::HouseOwnsCredits(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pEnemy)
		return false;

	const int credits = pEnemy->Available_Money();
	return AITriggerTypeExtData::GetComparatorResult(credits, pThis->Conditions[0]);
}

bool AITriggerTypeExtData::IronCurtainNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	return SuperWeaponNearReady(pOwner, 1);
}

bool AITriggerTypeExtData::ChronosphereNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	int idx = RulesExtData::Instance()->AIChronoSphereSW;

	return SuperWeaponNearReady(pOwner, idx >= 0 ? idx : 3);
}

bool AITriggerTypeExtData::CheckBaseCenterMZone(
	AITriggerTypeClass* pThis,
	TeamTypeClass* pTeam,
	HouseClass* pOwner,
	HouseClass* pEnemy)
{
	// Assembly 0x41FEE0: if !bool_F0 || !pEnemy → return true immediately.
	if (!pTeam->field_F0 || !pEnemy) 
		return true;

	// suggests it resolves the base center cell relative to another house.
	CellStruct ownerCenter = pOwner->GetBaseCenter();
	CellStruct enemyCenter = pEnemy->GetBaseCenter();

	const MovementZone mzone = pTeam->MovementZone;

	if (!pTeam->RequireDifferentZone)
	{
		// Normal case: both centers must be in the same movement zone.
		const int ownerZone = MapClass::Instance->GetMapZone(&ownerCenter, mzone, 0);
		const int enemyZone = MapClass::Instance->GetMapZone(&enemyCenter, mzone, 0);
		return ownerZone == enemyZone;
	}

	// Inverted case: centers must be in DIFFERENT movement zones.
	const int ownerZone = MapClass::Instance->GetMapZone(&ownerCenter, mzone, 0);
	const int enemyZone = MapClass::Instance->GetMapZone(&enemyCenter, mzone, 0);
	if (ownerZone == enemyZone)
		return false;

	// Additionally: both centers must be in the same AMPHIBIOUS zone.
	// Assembly: secondary check with MZONE_AMPH to ensure amphib connectivity.
	const int ownerAmph = MapClass::Instance->GetMapZone(&ownerCenter, MovementZone::Amphibious, 0);
	const int enemyAmph = MapClass::Instance->GetMapZone(&enemyCenter, MovementZone::Amphibious, 0);

	return ownerAmph == enemyAmph;
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

	std::ranges::for_each(*TechnoClass::Array, [&](const TechnoClass* pTechno)
 {
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
	if (!Phobos::Otamaa::DoingLoadGame)
		AITriggerTypeExtContainer::Instance.Allocate(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x41E4AF, AITriggerTypeClass_DTOR, 0x6)
{
	GET(AITriggerTypeClass*, pThis, ESI);
	AITriggerTypeExtContainer::Instance.Remove(pThis);
	return 0x0;
}

HRESULT __stdcall FakeAITriggerTypeClass::__Load(IStream* pStm)
{
	HRESULT hr = this->AITriggerTypeClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		if (!AITriggerTypeExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2A64, FakeAITriggerTypeClass::__Load)

HRESULT __stdcall FakeAITriggerTypeClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->AITriggerTypeClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr))
	{
		if (!AITriggerTypeExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2A68, FakeAITriggerTypeClass::__Save)

bool NOINLINE AITriggerTypeExtData::CheckConditionType(AITriggerTypeClass* pThis,
	   AITriggerCondition condType,
	   HouseClass* house1,
	   HouseClass* house2,
	   bool lessThanZeroIsNotAllowed
) {
	bool conditionMet = false;

	// Assembly 0x41E908 jump table (ConditionType -1..7):
	switch (condType)
	{
	case AITriggerCondition::Pool:
	{
		// Assembly 0x41E9D7: ConditionType==-1 with house2 → always true (goto LABEL_70).
		conditionMet = !lessThanZeroIsNotAllowed;
		break;
	}

	case AITriggerCondition::AIOwns:
	{
		// Assembly 0x41E90F: Enemy_Owns(house1, house2)
		conditionMet = AITriggerTypeExtData::EnemyOwns(pThis, house1, house2);
		break;
	}

	case AITriggerCondition::EnemyOwns:
	{
		// Assembly 0x41E91D: House_Owns(house1, house2)
		conditionMet = AITriggerTypeExtData::HouseOwns(pThis, house1, house2);
		break;
	}

	case AITriggerCondition::EnemyYellowPower:
	{
		// Assembly 0x41E92B-0x41E963:
		// (Power_Output - Power_Drain) < 100 → conditionMet = true
		// Uses IHouse vtable: Power_Drain at [vtbl+0x24], Power_Output at [vtbl+0x20]
		// Threshold: dbl_7E2AC0 = 100.0
		const auto powerSurplus = house2->Power_Output() - house2->Power_Drain();
		conditionMet = powerSurplus < 100.0;
		break;
	}

	case AITriggerCondition::EnemyRedPower:
	{
		// Assembly 0x41E965-0x41E999:
		// (Power_Output - Power_Drain) < 0 → conditionMet = true
		// Threshold: FLOAT_0_0 = 0.0
		const int powerSurplus = house2->Power_Output() - house2->Power_Drain();
		conditionMet = powerSurplus < 0;
		break;
	}

	case AITriggerCondition::EnemyCashExceeds:
	{
		// Assembly 0x41E99B: House_Owns_Credits(house1, house2)
		conditionMet = AITriggerTypeExtData::HouseOwnsCredits(pThis, house1, house2);
		break;
	}

	case AITriggerCondition::IronCharged:
	{
		// Assembly 0x41E9A6: Iron_Curtain_Near_Ready(house1, house2)
		conditionMet = AITriggerTypeExtData::IronCurtainNearReady(pThis, house1, house2);
		break;
	}

	case AITriggerCondition::ChronoCharged:
	{
		// Assembly 0x41E9B1: Chronosphere_Near_Ready(house1, house2)
		conditionMet = AITriggerTypeExtData::ChronosphereNearReady(pThis, house1, house2);
		break;
	}

	case AITriggerCondition::NeutralOwns:
	{
		// Assembly 0x41E9BC: Neutral_Owns(house1, house2)
		conditionMet = AITriggerTypeExtData::NeutralOwns(pThis, house1, house2);
		break;
	}

	default:
	{
		switch ((PhobosAINewConditionTypes)condType)
		{
		case PhobosAINewConditionTypes::NumberOfTechBuildingsExist:
		{
			conditionMet = AITriggerTypeExtData::NumberOfTechBuildingsExist(pThis, house1);
			break;
		}

		case PhobosAINewConditionTypes::NumberOfBridgeRepairHutsExist:
		{
			conditionMet = AITriggerTypeExtData::NumberOfBridgeRepairHutsExist(pThis);
			break;
		}

		case PhobosAINewConditionTypes::CheckPrereq:
		{
			if (const auto pItem = pThis->ConditionObject)
				conditionMet = house2 && HouseExtData::PrereqValidate(house2, pItem, false, true) == CanBuildResult::Buildable;

			break;
		}

		case PhobosAINewConditionTypes::CheckBridgeCondition:
		{
			if (auto const pCiv = HouseExtData::FindFirstCivilianHouse())
			{
				conditionMet = pCiv->Buildings.any_of([](BuildingClass* const pBld)
 {
	 return pBld->Type->BridgeRepairHut && pBld->Type->Repairable && MapClass::Instance->IsBrideRepairNeeded(pBld->InlineMapCoords());
				});
			}
			break;
		}
		//0
		case PhobosAINewConditionTypes::EnemyOwnsConditionObject:
		{
			if (auto pItem = pThis->ConditionObject)
				conditionMet = EnemyOwns(pThis, house1, house2, true, pItem);

			break;
		}
		//1
		case PhobosAINewConditionTypes::HouseOwnsConditionObject:
		{
			if (auto pItem = pThis->ConditionObject)
				conditionMet = HouseOwns(pThis, house1, false, pItem);

			break;
		}
		//7
		case PhobosAINewConditionTypes::NeutralOwnsConditionObject:
		{
			if (auto pItem = pThis->ConditionObject)
				conditionMet = NeutralOwns(pThis, pItem);

			break;
		}
		//8
		case PhobosAINewConditionTypes::AllEnemyOwnsConditionObject:
		{
			if (auto pItem = pThis->ConditionObject)
				conditionMet = EnemyOwns(pThis, house1, nullptr, false, pItem);

			break;
		}
		//9
		case PhobosAINewConditionTypes::EnemyOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwns(pThis, house1, house2, false,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//10
		case PhobosAINewConditionTypes::HouseOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwns(pThis, house1, false,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//11
		case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = NeutralOwns(pThis, RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//12
		case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwns(pThis, house1, nullptr, false,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//13
		case PhobosAINewConditionTypes::AllyOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwns(pThis, house1, true,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//14
		case PhobosAINewConditionTypes::EnemyOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwnsAll(pThis, house1, house2,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//15
		case PhobosAINewConditionTypes::HouseOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwnsAll(pThis, house1,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//16
		case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = NeutralOwnsAll(pThis, RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//17
		case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < RulesExtData::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwnsAll(pThis, house1, nullptr,
					RulesExtData::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//18
		case PhobosAINewConditionTypes::DestroyedBridgeCount:
		{
			int destroyedBridgesCount = 0;

			for (auto const pBuilding : *BuildingClass::Array)
			{
				if (!IsValidTechno(pBuilding)) continue;

				auto const pBuildingType = pBuilding->Type;
				if (pBuilding && pBuilding->Type->BridgeRepairHut)
				{
					if (MapClass::Instance->IsLinkedBridgeDestroyed(pBuilding->GetCell()->MapCoords))
						destroyedBridgesCount++;
				}
			}

			conditionMet = CountConditionMet(pThis, destroyedBridgesCount);

			break;
		}
		//19
		case PhobosAINewConditionTypes::UndamagedBridgeCount:
		{
			int undamagedBridgesCount = 0;

			for (auto const pBuilding : *BuildingClass::Array)
			{
				if (!IsValidTechno(pBuilding)) continue;

				auto const pBuildingType = pBuilding->Type;
				if (pBuilding && pBuilding->Type->BridgeRepairHut)
				{
					if (!MapClass::Instance->IsLinkedBridgeDestroyed(pBuilding->GetCell()->MapCoords))
						undamagedBridgesCount++;
				}
			}

			conditionMet = CountConditionMet(pThis, undamagedBridgesCount);
			break;
		}
		default:
			break;
		}
	}
	}

	return conditionMet;
}

bool FakeAITriggerTypeClass::_NewTeam(HouseClass* house1, HouseClass* house2, bool skip)
{
	// Assembly 0x41E726-0x41E752:
  // isBaseDefense = true if TeamTypeOne OR TeamTypeTwo has IsBaseDefense set.
	TeamTypeClass* pTeamOne = this->Team1;
	TeamTypeClass* pTeamTwo = this->Team2;

	const bool isBaseDefense = (pTeamOne && pTeamOne->IsBaseDefense)
		|| (pTeamTwo && pTeamTwo->IsBaseDefense);

	// Assembly 0x41E756: if TeamTypeOne == null, return false immediately.
	if (!pTeamOne)
		return false;

	// Assembly 0x41E760-0x41E7B0:
	// Three paths converge at the global check (LABEL_17):
	//   A) house2 present, enough base defenders, not base defense team → pass through
	//   B) house2 present, enough base defenders, is base defense team, !skip → pass through
	//   C) house2 present, below min defenders, is base defense team, !skip → pass through
	//   D) no house2, is base defense team, !skip → pass through
	// All other combinations → return false.
	//
	// Restructured as: compute whether we must run the base-defense guard,
	// then apply it. Both paths then fall through to the global check.
	HouseClass* pHouse = house1;

	// Determine if we need to enforce the base-defense/skip gate.
	// Assembly 0x41E762-0x41E78E:
	//   no house2 → always enforce gate
	//   house2 present + below min defenders → enforce gate
	//   house2 present + enough defenders → skip gate (but still apply skip-only block if isBaseDefense)
	const bool enforceBaseDefenseGate = !house2
		|| (RulesClass::Instance->UseMinDefenseRule
			&& house1->BaseDefenseTeamCount < RulesClass::Instance->MinimumAIDefensiveTeams.Items[(int)house1->AIDifficulty]);

	if (enforceBaseDefenseGate)
	{
		// Assembly 0x41E794: must be a base defense team to proceed.
		if (!isBaseDefense)
			return false;

		// Assembly 0x41E7A5: base defense team with skip flag → return false.
		if (skip)
			return false;
	}
	else
	{
		// Assembly 0x41E7A1-0x41E7AB:
		// Enough defenders, but if this IS a base defense team and skip is set → return false.
		if (isBaseDefense && skip)
			return false;
	}

	// Both paths now converge here (LABEL_17 in assembly).
	// Assembly 0x41E7B1-0x41E7CB:
	// If IsGlobal == 1 AND Scen->IgnoreGlobalAITriggers == 1 → return false.
	if (this->IsGlobal == 1
		&& ScenarioClass::Instance->IgnoreGlobalAITriggers == 1)
		return false;

	// Assembly 0x41E7CD: if !Enabled → return false.
	if (!this->IsEnabled)
		return false;

	// Assembly 0x41E7D7-0x41E854: difficulty gating.
	// Two parallel paths: multiplayer uses house->Difficulty, campaign uses Scen->Difficulty.
	// Assembly difficulty values: 0=Easy, 1=Medium, 2=Hard
	// SUSPECT: pseudocode labels LABEL_28/LABEL_31/LABEL_34 appear swapped.
	//          Assembly confirmed: diff==0→[0xD2], diff==1→[0xD3], diff==2→[0xD4].
	if (SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		// Multiplayer/skirmish: must be flagged for skirmish.
		if (!this->IsForSkirmish)
			return false;

		const auto diff = pHouse->AIDifficulty;
		if (diff == AIDifficulty::Hard && !this->Enabled_Easy) 
			return false;
		if (diff == AIDifficulty::Normal && !this->Enabled_Normal)
			return false;
		if (diff == AIDifficulty::Easy && !this->Enabled_Hard)
			return false;
	}
	else
	{
		// Campaign: uses scenario difficulty.
		const AIDifficulty diff = ScenarioClass::Instance->Difficulty1;
		if (diff == AIDifficulty::Hard && !this->Enabled_Easy)
			return false;
		if (diff == AIDifficulty::Normal && !this->Enabled_Normal)
			return false;
		if (diff == AIDifficulty::Easy && !this->Enabled_Hard)
			return false;
	}

	// Assembly 0x41E855-0x41E88A: owning house type check.
	// ownHouseType: 0=any, 1=specific house, 2=any (skip check)
	switch (const AITriggerHouseType ownHouseType = this->OwnerHouseType)
	{
	case AITriggerHouseType::None:
		return false;
	case AITriggerHouseType::Single:
	{
		if (pHouse->Type->ArrayIndex != this->HouseIndex)
			return false;

		break;
	}
	default:
		break;
	}

	// Assembly 0x41E88D-0x41E8D4: owning country (side) filter.
	// OwningCountry: 1=Allied only, 2=Soviet only, 3=ThirdSide only
	// Uses house->ActLike [edi+0x1E8]: 0=Allied, 1=Soviet, 2=ThirdSide
	// Added hook : 0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA
	// to accomodate the more side
	const int owningCountry = this->SideIndex;
	if (owningCountry != 0)
	{
		if ((owningCountry - 1) != pHouse->SideIndex)
			return false;
	}
	// Assembly 0x41E8D7-0x41E8ED:
	// If trigger's TechLevel > house's TechLevel → return false.
	if (this->TechLevel > pHouse->StaticData.TechLevel)
		return false;

	// Assembly 0x41E8F0: ConditionType switch.
	// Switch uses (ConditionType + 1) as index, so ConditionType -1..7 → cases 0..8.
	const auto condType = this->ConditionType;

	bool conditionMet = AITriggerTypeExtData::CheckConditionType(this,
		condType, house1, house2, false);


	if (!conditionMet)
		return false;

	// Assembly 0x41E9EA-0x41E9FC: check TeamTypeOne passes mzone/base center check.
	if (!AITriggerTypeExtData::CheckBaseCenterMZone(this, pTeamOne, house1, house2))
		return false;

	// Assembly 0x41EA05-0x41EA21: if TeamTypeTwo exists, check it too.
	if (pTeamTwo && !AITriggerTypeExtData::CheckBaseCenterMZone(this, pTeamTwo, house1, house2))
		return false;

	// Assembly 0x41EA24-0x41EA3A: house must be able to instantiate TeamTypeOne.
	if (!house1->CanInstantiateTeam(pTeamOne))
		return false;

	// Assembly 0x41EA3D-0x41EA57: if TeamTypeTwo exists, check it too.
	if (pTeamTwo && !house1->CanInstantiateTeam(pTeamTwo))
		return false;

	// Assembly 0x41EA5A-0x41EA8A:
	// TeamTypeOne MaxAllowed check: if MaxAllowed >= 0 and current count >= max → return false.
	// MaxAllowed < 0 means unlimited.
	if (pTeamOne && pTeamOne->Max >= 0
		&& house1->TeamTypeCount(pTeamOne) >= pTeamOne->Max)
		return false;

	// Assembly 0x41EA8D-0x41EABD: same check for TeamTypeTwo.
	if (pTeamTwo && pTeamTwo->Max >= 0
		&& house1->TeamTypeCount(pTeamTwo) >= pTeamTwo->Max)
		return false;

	// Assembly 0x41EAC0: all checks passed.
	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x41E720, FakeAITriggerTypeClass::_NewTeam)

//AbstractTypeClass* ResolveTechType(const char* name)
//{
//	int idx = InfantryTypeClass::From_Name(name);
//	if (idx != -1)
//		return InfantryTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	idx = UnitTypeClass::From_Name(name);
//	if (idx != -1)
//		return UnitTypes.Vector[idx];     // VERIFY: YRpp accessor name
//
//	idx = AircraftTypeClass::From_Name(name);
//	if (idx != -1)
//		return AircraftTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	idx = BuildingTypeClass::From_Name(name);
//	if (idx != -1)
//		return BuildingTypes.Vector[idx]; // VERIFY: YRpp accessor name
//
//	return nullptr;
//}
//
//void ParseConditions(const char* str, AITriggerTypeClass* self)
//{
//	// Vanilla lookup table initializer from stack:
//	//   var_270 = word_818170 (global 2-byte value)
//	//   var_270+2 = byte_818172 (global 1-byte value)
//	//   anonymous_0 (qword) = 0
//	// Together: a 3-entry sorted array used as the binary search range.
//	// VERIFY: these globals and their exact layout in YRpp/assembly
//	unsigned short lookup[2];
//	lookup[0] = '00'; // VERIFY: global name
//	reinterpret_cast<uint8_t*>(lookup)[2] = '0'; // VERIFY: global name
//	const unsigned short* const lookupEnd = reinterpret_cast<const unsigned short*>(
//		reinterpret_cast<const uint8_t*>(lookup) + sizeof(unsigned short) + 1);
//
//	unsigned int count = 0;
//	const char* p = str;
//
//	while (*p && count < 0x20)
//	{
//		// skip whitespace
//		while (*p && std::isspace(static_cast<unsigned char>(*p)))
//			++p;
//
//		// read two characters as a pair
//		unsigned short pair = 0;
//		reinterpret_cast<char*>(&pair)[0] = *p;
//		if (*p) ++p;
//
//		const char second = *p;
//		reinterpret_cast<char*>(&pair)[1] = second ? second : '\0';
//		if (second) ++p;
//
//		// binary search into lookup table
//		// vanilla: std::lower_bound(&var_270, &anonymous_0, 0x10)
//		// stores result byte (al) into Conditions[count]
//		const unsigned short* found = std::lower_bound(lookup, lookupEnd, static_cast<unsigned short>(0x10));
//		self->Conditions[count] = static_cast<uint8_t>(
//			reinterpret_cast<const uint8_t*>(found)[0]); // VERIFY: result extraction
//
//		++count;
//
//		if (!*p)
//			break;
//	}
//}

bool FakeAITriggerTypeClass::_SaveToINI(CCINIClass* pINI)
{
	/*
	*
	char v44[512];

	INIClass::Clear_Section_Cache(iniHandle);

	if (!INIClass::Get_String(iniHandle, "AITriggerTypes", this->IniName, // VERIFY: field name
		&Wstring::EmptyString, v44, 512))
		return 0;

	// --- token 1: name (48 chars + null) ---
	// Vanilla: strncpy into destination[48]+sentinel, then rep movsd(x12)+movsb
	// into this->at.Name. Stack bounce preserved semantically via strncpy limit.
	const char* tok = strtok(v44, ",");
	if (!tok)
		return 0;

	std::strncpy(this->Name, tok, 48u); // VERIFY: field name in YRpp
	this->Name[48] = '\0';

	// --- token 2: TeamTypeOne ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf); // VERIFY: strtrim signature

		this->TeamTypeOne = nullptr; // VERIFY: field name
		if (_strcmpi(buf, none_str)) // VERIFY: none_str global
			this->TeamTypeOne = TeamTypeClass::From_Name(buf);
	}

	// --- token 3: owning house ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		this->OwnHouseType = 0;  // VERIFY: field name
		this->OwningHouse = -1; // VERIFY: field name

		if (!_strcmpi(buf, alllstring)) // VERIFY: alllstring global
		{
			this->OwnHouseType = 2;
		}
		else if (_strcmpi(buf, none_str))
		{
			const int houseIdx = HouseTypeClass::From_Name(buf);
			this->OwningHouse = houseIdx;
			if (houseIdx != -1)
				this->OwnHouseType = 1;
		}
	}

	// --- token 4: discarded (reserved field) ---
	// Assembly: strtok called, result checked for null but value unused.
	if (!strtok(nullptr, ","))
		return 0;

	// --- token 5: TechLevel (always initialized to 0 before token consumption) ---
	this->TechLevel = 0; // VERIFY: field name
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;
	this->ConditionType = std::atoi(tok); // VERIFY: field name

	// --- token 6: ConditionType ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;
	this->ConditionType = std::atoi(tok); // VERIFY: field name — assembly: [ebp+98h]

	// --- token 7: ConditionObject (tech type name) ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		// Vanilla: tries all four type vectors in order, stores raw pointer
		// Assembly: esi = resolved pointer, stored at [ebp+0D8h]
		this->ConditionObject = ResolveTechType(buf); // VERIFY: field name
	}

	// --- token 8: Conditions hex-pair string ---
	tok = strtok(nullptr, ",");
	if (!tok)
		return 0;

	ParseConditions(tok, this);

	// --- token 9: WeightCur (optional from here) ---
	// Vanilla: atof -> __ftol -> fild -> fstp double
	// Replaced with std::stod for equivalent precision.
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightCur = std::stod(tok); // VERIFY: field name, [ebp+0B8h]

	// --- token 10: WeightMin ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightMin = std::stod(tok); // VERIFY: field name, [ebp+0C0h]

	// --- token 11: WeightMax ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->WeightMax = std::stod(tok); // VERIFY: field name, [ebp+0C8h]

	// --- token 12: IsForSkirmish ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->IsForSkirmish = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D0h]

	// --- token 13: discarded (second reserved slot) ---
	// Assembly: 0x41F93C — two consecutive strtok calls; first result unused.
	strtok(nullptr, ",");

	// --- token 14: OwningCountry ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->OwningCountry = std::atoi(tok); // VERIFY: field name, [ebp+0ACh]

	// --- token 15: IsForBaseDefense ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->IsForBaseDefense = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D1h]

	// --- token 16: TeamTypeTwo ---
	tok = strtok(nullptr, ",");
	if (tok)
	{
		char buf[24];
		std::strncpy(buf, tok, 23u);
		buf[23] = '\0';
		strtrim(buf);

		this->TeamTypeTwo = nullptr; // VERIFY: field name, [ebp+0E0h]
		if (_strcmpi(buf, none_str))
			this->TeamTypeTwo = TeamTypeClass::From_Name(buf);
	}

	// --- token 17: EnabledInEasy ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInEasy = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D2h]

	// --- token 18: EnabledInMedium ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInMedium = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D3h]

	// --- token 19: EnabledInHard ---
	tok = strtok(nullptr, ",");
	if (tok)
		this->EnabledInHard = std::atoi(tok) != 0; // VERIFY: field name, [ebp+0D4h]

	// --- TechLevel derivation from TaskForce requirements ---
	// Vanilla: TechLevel = max(TechLevel, TaskForce::Tech_Level_Required(team->TaskForce))
	// Assembly: 0x41FA5C-0x41FAE3; both teams checked independently.
	// [TeamTypeClass+0E4h] = TaskForce pointer  VERIFY in YRpp

	if (this->TeamTypeOne)
	{
		const int required = TaskForceClass::Tech_Level_Required(
			this->TeamTypeOne->TaskForce); // VERIFY: field name
		this->TechLevel = std::max(this->TechLevel, required);
	}

	if (this->TeamTypeTwo)
	{
		const int required = TaskForceClass::Tech_Level_Required(
			this->TeamTypeTwo->TaskForce); // VERIFY: field name
		this->TechLevel = std::max(this->TechLevel, required);
	}
	*/
	return 1;
}

bool FakeAITriggerTypeClass::_LoadFromINI(CCINIClass* pINI)
{
	return true;
}

bool FakeAITriggerTypeClass::_ReadScenarioINI(CCINIClass* pINI) { return true; }
bool FakeAITriggerTypeClass::_WriteScenarioINI(CCINIClass* pINI) { return true; }