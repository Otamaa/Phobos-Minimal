#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TaskForce/Body.h>

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
		int count = BuildingTypeExtData::GetUpgradesAmount(
			(BuildingTypeClass*)pType, pHouse);

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
	if (nObjects < 0)
		return false;

	return pThis->Conditions[0].EvaluateComparator(nObjects, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(counter, false);
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

		result = pThis->Conditions[0].EvaluateComparator(counter, false);
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

		result = pThis->Conditions[0].EvaluateComparator(counter, false);
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

		result = pThis->Conditions[0].EvaluateComparator(counter, false);
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

	return pThis->Conditions[0].EvaluateComparator(count, false);
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

	return pThis->Conditions[0].EvaluateComparator(count, false);
}

bool AITriggerTypeExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pEnemy)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pEnemy);
	return pThis->Conditions[0].EvaluateComparator(count, false);
}

bool AITriggerTypeExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pOwner)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pOwner);
	return pThis->Conditions[0].EvaluateComparator(count, false);
}

bool AITriggerTypeExtData::NeutralOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	HouseClass* pCivilian = HouseExtData::FindFirstCivilianHouse();
	if (!pCivilian)
		return false;

	const int count = CountOwnedType(pThis->ConditionObject, pCivilian);
	return pThis->Conditions[0].EvaluateComparator(count, false);
}

bool AITriggerTypeExtData::HouseOwnsCredits(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	if (!pEnemy)
		return false;

	const int credits = pEnemy->Available_Money();
	return pThis->Conditions[0].EvaluateComparator(credits, false);
}

bool AITriggerTypeExtData::IronCurtainNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	return SuperWeaponNearReady(pOwner, 1);
}

bool AITriggerTypeExtData::ChronosphereNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy)
{
	int idx = FakeRulesClass::Instance()->AIChronoSphereSW;

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
	auto& AIConditionsLists = FakeRulesClass::Instance()->AIConditionsLists;

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

std::string_view GetConditionString(PhobosAINewConditionTypes condition)
{
	switch (condition)
	{
		// PR #2119
	case PhobosAINewConditionTypes::NumberOfTechBuildingsExist:
		return "NumberOfTechBuildingsExist";
	case PhobosAINewConditionTypes::NumberOfBridgeRepairHutsExist:
		return "NumberOfBridgeRepairHutsExist";

	case PhobosAINewConditionTypes::CheckPrereq:
		return "CheckPrereq";
	case PhobosAINewConditionTypes::CheckBridgeCondition:
		return "CheckBridgeCondition";

	case PhobosAINewConditionTypes::EnemyOwnsConditionObject:
		return "EnemyOwnsConditionObject";
	case PhobosAINewConditionTypes::HouseOwnsConditionObject:
		return "HouseOwnsConditionObject";
	case PhobosAINewConditionTypes::NeutralOwnsConditionObject:
		return "NeutralOwnsConditionObject";
	case PhobosAINewConditionTypes::AllEnemyOwnsConditionObject:
		return "AllEnemyOwnsConditionObject";
	case PhobosAINewConditionTypes::EnemyOwnsAITargetTypesLists:
		return "EnemyOwnsAITargetTypesLists";
	case PhobosAINewConditionTypes::HouseOwnsAITargetTypesLists:
		return "HouseOwnsAITargetTypesLists";
	case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesLists:
		return "NeutralOwnsAITargetTypesLists";
	case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesLists:
		return "AllEnemyOwnsAITargetTypesLists";
	case PhobosAINewConditionTypes::AllyOwnsAITargetTypesLists:
		return "AllyOwnsAITargetTypesLists";

	case PhobosAINewConditionTypes::EnemyOwnsAITargetTypesListsComp:
		return "EnemyOwnsAITargetTypesListsComp";
	case PhobosAINewConditionTypes::HouseOwnsAITargetTypesListsComp:
		return "HouseOwnsAITargetTypesListsComp";
	case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesListsComp:
		return "NeutralOwnsAITargetTypesListsComp";
	case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesListsComp:
		return "AllEnemyOwnsAITargetTypesListsComp";

	case PhobosAINewConditionTypes::DestroyedBridgeCount:
		return "DestroyedBridgeCount";
	case PhobosAINewConditionTypes::UndamagedBridgeCount:
		return "UndamagedBridgeCount";

	default:
		return "";
	};
}

std::string_view GetAITriggerConditionString(AITriggerCondition condition)
{
	switch (condition)
	{
	case AITriggerCondition::Pool:
		return "Pool";
	case AITriggerCondition::AIOwns:
		return "AIOwns";
	case AITriggerCondition::EnemyOwns:
		return "EnemyOwns";
	case AITriggerCondition::EnemyYellowPower:
		return "EnemyYellowPower";
	case AITriggerCondition::EnemyRedPower:
		return "EnemyRedPower";
	case AITriggerCondition::EnemyCashExceeds:
		return "EnemyCashExceeds";
	case AITriggerCondition::IronCharged:
		return "IronCharged";
	case AITriggerCondition::ChronoCharged:
		return "ChronoCharged";
	case AITriggerCondition::NeutralOwns:
		return "NeutralOwns";
	default:
		return "";
	}
}

bool NOINLINE AITriggerTypeExtData::CheckConditionType(AITriggerTypeClass* pThis,
	   AITriggerCondition condType,
	   HouseClass* house1,
	   HouseClass* house2,
	   bool lessThanZeroIsNotAllowed
) {
	bool conditionMet = false;

	std::string_view name = GetAITriggerConditionString(AITriggerCondition(condType));

	if (name.empty())
		name = GetConditionString((PhobosAINewConditionTypes)condType);

	if (name.empty())
		name = "Unknown";

	Debug::LogInfo("AITriggerType[{} - {}] triggering [{} - {}]", (void*)pThis, pThis->ID, (int)condType, name);

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
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwns(pThis, house1, house2, false,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//10
		case PhobosAINewConditionTypes::HouseOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwns(pThis, house1, false,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//11
		case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = NeutralOwns(pThis, FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//12
		case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwns(pThis, house1, nullptr, false,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//13
		case PhobosAINewConditionTypes::AllyOwnsAITargetTypesLists:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwns(pThis, house1, true,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//14
		case PhobosAINewConditionTypes::EnemyOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwnsAll(pThis, house1, house2,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//15
		case PhobosAINewConditionTypes::HouseOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = HouseOwnsAll(pThis, house1,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//16
		case PhobosAINewConditionTypes::NeutralOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = NeutralOwnsAll(pThis, FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//17
		case PhobosAINewConditionTypes::AllEnemyOwnsAITargetTypesListsComp:
		{
			if ((size_t)pThis->Conditions[3].Operand < FakeRulesClass::Instance()->AITargetTypesLists.size())
				conditionMet = EnemyOwnsAll(pThis, house1, nullptr,
					FakeRulesClass::Instance()->AITargetTypesLists[pThis->Conditions[3].Operand]);

			break;
		}
		//18
		case PhobosAINewConditionTypes::DestroyedBridgeCount:
		{
			int destroyedBridgesCount = 0;

			for (auto const pBuilding : *BuildingClass::Array) {
				if (!IsValidTechno(pBuilding)) continue;

				if (pBuilding && pBuilding->Type->BridgeRepairHut) {
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

				//auto const pBuildingType = pBuilding->Type;
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
  // isBaseDefense = true if Team1 OR Team2 has IsBaseDefense set.
	TeamTypeClass* pTeamOne = this->Team1;
	TeamTypeClass* pTeamTwo = this->Team2;

	const bool isBaseDefense = (pTeamOne && pTeamOne->IsBaseDefense)
		|| (pTeamTwo && pTeamTwo->IsBaseDefense);

	// Assembly 0x41E756: if Team1 == null, return false immediately.
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
	if (this->Type == AITriggerType::Global
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
	// OwnerHouseType: 0=any, 1=specific house, 2=any (skip check)
	switch (this->OwnerHouseType)
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

	// Assembly 0x41E9EA-0x41E9FC: check Team1 passes mzone/base center check.
	if (!AITriggerTypeExtData::CheckBaseCenterMZone(this, pTeamOne, house1, house2))
		return false;

	// Assembly 0x41EA05-0x41EA21: if Team2 exists, check it too.
	if (pTeamTwo && !AITriggerTypeExtData::CheckBaseCenterMZone(this, pTeamTwo, house1, house2))
		return false;

	// Assembly 0x41EA24-0x41EA3A: house must be able to instantiate Team1.
	if (!house1->CanInstantiateTeam(pTeamOne))
		return false;

	// Assembly 0x41EA3D-0x41EA57: if Team2 exists, check it too.
	if (pTeamTwo && !house1->CanInstantiateTeam(pTeamTwo))
		return false;

	// Assembly 0x41EA5A-0x41EA8A:
	// Team1 MaxAllowed check: if MaxAllowed >= 0 and current count >= max → return false.
	// MaxAllowed < 0 means unlimited.
	if (pTeamOne && pTeamOne->Max >= 0
		&& house1->TeamTypeCount(pTeamOne) >= pTeamOne->Max)
		return false;

	// Assembly 0x41EA8D-0x41EABD: same check for Team2.
	if (pTeamTwo && pTeamTwo->Max >= 0
		&& house1->TeamTypeCount(pTeamTwo) >= pTeamTwo->Max)
		return false;

	// Assembly 0x41EAC0: all checks passed.
	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x41E720, FakeAITriggerTypeClass::_NewTeam)

TechnoTypeClass* ResolveTechType(AITriggerTypeClass* pThis , const char* name)
{
	TechnoTypeClass* pResult = InfantryTypeClass::Find(name);

	if(!pResult){
		pResult = UnitTypeClass::Find(name);
	}

	if (!pResult) {
		pResult = AircraftTypeClass::Find(name);
	}

	if (!pResult) {
		pResult = BuildingTypeClass::Find(name);
	}

	if (Phobos::Otamaa::IsAdmin && !GameStrings::IsNone(name))
		Debug::LogInfo("Condition Object[{} - {}] for [{}]", name, pResult ? pResult->GetThisClassName() : GameStrings::NoneStrb(), pThis->ID);

	return pResult;
}

bool FakeAITriggerTypeClass::_SaveToINI(CCINIClass* pINI)
{
	// --- resolve the four name fields (all default to "<none>") -----------
	const char* pTeamOne = GameStrings::NoneStr;   // v12
	const char* pTeamTwo = GameStrings::NoneStr;   // v13
	const char* pOwnerHouse = GameStrings::NoneStr;   // v3
	const char* pConditionObject = GameStrings::NoneStr;   // v14

	if (this->Team1)
		pTeamOne = this->Team1->ID;

	if (this->Team2)
		pTeamTwo = this->Team2->ID;

	if (this->OwnerHouseType == AITriggerHouseType::Single)
	{
		if (this->HouseIndex != -1)
			pOwnerHouse = HouseTypeClass::Array->Items[this->HouseIndex]->ID;  // ORIG: (*(&HouseTypes+1))[i]->at.IniName
	}
	else if (this->OwnerHouseType == AITriggerHouseType::Any)
	{
		pOwnerHouse = GameStrings::AllStr;                                   // "<all>"
	}

	if (this->ConditionObject)
		pConditionObject = this->ConditionObject->ID;               // ORIG: v7->ot.at.IniName

	// --- condition list: 32 bytes, each 2 lowercase hex chars -------------
	std::string conditions;
	conditions.reserve(64);
	for (int i = 0; i < 32; ++i)
		conditions += fmt::format("{:02x}", static_cast<unsigned int>(
			this->_Conditions[i])); // ORIG: sprintf(p,"%02x",..); p+=2

	// --- serialize the full comma record ----------------------------------
	// ORIG: "%s,%s,%s,%d,%d,%s,%s,%lf,%lf,%lf,%d,%d,%d,%d,%s,%d,%d,%d"
	const std::string line = fmt::format(
		"{},{},{},{},{},{},{},{:f},{:f},{:f},{},{},{},{},{},{},{},{}",
		this->Name,                                   // %s  Name
		pTeamOne,                                     // %s  TeamOne
		pOwnerHouse,                                  // %s  owner house
		this->TechLevel,                              // %d
		this->ConditionType,                          // %d
		pConditionObject,                             // %s  condition object
		conditions,                                   // %s  condition hex list
		this->Weight_Current,                              // %lf
		this->Weight_Minimum,                              // %lf
		this->Weight_Maximum,                              // %lf
		static_cast<int>(this->IsForSkirmish),        // %d
		0,                                            // %d  literal 0 (unused field 11)
		this->SideIndex,                          // %d
		static_cast<int>(this->IsForBaseDefense),     // %d
		pTeamTwo,                                      // %s  TeamTwo
		static_cast<int>(this->Enabled_Easy),        // %d
		static_cast<int>(this->Enabled_Normal),      // %d
		static_cast<int>(this->Enabled_Hard));       // %d  ORIG: !(Enabled_Hard == 0)

	return pINI->WriteString("AITriggerTypes", this->ID, line.c_str());
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2AB8, FakeAITriggerTypeClass::_SaveToINI)
DEFINE_FUNCTION_JUMP(LJMP, 0x41FB10, FakeAITriggerTypeClass::_SaveToINI)

bool FakeAITriggerTypeClass::_LoadFromINI(CCINIClass* pINI)
{
	pINI->Reset();

	char line[512];
	// ORIG default arg: &Wstring::EmptyString
	if (!pINI->ReadString("AITriggerTypes", this->ID, "", line))
		return false;

	static COMPILETIMEEVAL size_t AITriggerFieldCount = 18;   // indices 0..17

	// index -> human name, used verbatim in the log lines
	static COMPILETIMEEVAL const char* AITriggerFieldNames[AITriggerFieldCount] =
	{
		"Name",            // 0
		"Team1",           // 1
		"OwnerHouse",      // 2
		"TechLevel",       // 3  (read then discarded by vanilla)
		"ConditionType",   // 4
		"ConditionObject", // 5
		"ConditionList",   // 6
		"Weight_Current",  // 7
		"Weight_Minimum",  // 8
		"Weight_Maximum",  // 9
		"IsForSkirmish",   // 10
		"Unused",          // 11
		"SideIndex",       // 12
		"IsForBaseDefense",// 13
		"Team2",           // 14
		"Enabled_Easy",    // 15
		"Enabled_Normal",  // 16
		"Enabled_Hard"     // 17
	};

	// ---- identity token for the log --------------------------------------------------
	// Prefers the section ID; falls back to the pointer when the ID is not usable yet.
	auto AITriggerTag = [](const AITriggerTypeClass* pThis)
		{
			if (pThis && pThis->ID[0] != '\0')
				return std::string(pThis->ID);

			return fmt::format("{:#010x}", reinterpret_cast<uintptr_t>(pThis));
		};

	auto LogFieldFail = [AITriggerTag](const AITriggerTypeClass* pThis, size_t idx)
		{
			// [AITrigger] 04010001-G - failed to parse 'SideIndex' field
			Debug::LogInfo("[AITrigger] {} - failed to parse '{}' field",
				AITriggerTag(pThis), AITriggerFieldNames[idx]);
		};

	auto LogFieldFailB = [AITriggerTag](const AITriggerTypeClass* pThis, size_t idx, std::string_view value)
		{
			// [AITrigger] 04010001-G - failed to parse 'Team1' field ["03010001-G"]
			Debug::LogInfo("[AITrigger] {} - failed to parse '{}' field [\"{}\"]",
				AITriggerTag(pThis), AITriggerFieldNames[idx], value);
		};

	// ---- trimming ---------------------------------------------------------------------
	auto TrimView = [](std::string_view src)
		{
			const auto isWS = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };

			while (!src.empty() && isWS(src.front()))
				src.remove_prefix(1);

			while (!src.empty() && isWS(src.back()))
				src.remove_suffix(1);

			return src;
		};

	// ORIG: strncpy(buf, tok, cap); buf[cap - 1] = '\0'; CRT::strtrim(buf);
	// Truncation is PRESERVED - vanilla clips over-long ids and Find() sees the clipped form.
	auto TrimmedField = [TrimView](std::string_view src, size_t cap)
		{
			return std::string(TrimView(src.substr(0, cap)));
		};

	auto CopyName = [](char* pDest, std::string_view src, size_t cap)
		{
			src = src.substr(0, cap);
			std::memcpy(pDest, src.data(), src.size());
			pDest[src.size()] = '\0';
		};

	// ---- atoi / atof with a failure signal ---------------------------------------------
	// Semantics deliberately match the CRT calls they replace:
	//   * leading whitespace skipped, leading '+' accepted (from_chars rejects both)
	//   * a partial parse ("40.000000" -> 40) is SUCCESS, exactly like atoi()
	//   * only "nothing consumable at all" counts as a failure
	// On failure the vanilla result (0) is still written, so behaviour is unchanged -
	// the only addition is the log line.
	auto TryParseInt = [TrimView](std::string_view src, int& out)
		{
			out = 0;
			std::string_view s = TrimView(src);

			if (!s.empty() && s.front() == '+')
				s.remove_prefix(1);

			if (s.empty())
				return false;

			const auto res = std::from_chars(s.data(), s.data() + s.size(), out);
			return res.ptr != s.data();      // at least one char consumed
		};

	auto TryParseDouble = [TrimView](std::string_view src, double& out)
		{
			out = 0.0;
			std::string_view s = TrimView(src);

			if (!s.empty() && s.front() == '+')
				s.remove_prefix(1);

			if (s.empty())
				return false;

			// VERIFY: floating-point std::from_chars needs MSVC 19.24+ / VS2019 16.4.
			//         If the toolset is older, swap for strtod on a null-terminated copy.
			const auto res = std::from_chars(s.data(), s.data() + s.size(), out);
			return res.ptr != s.data();
		};


	const auto fields = PhobosCRT::SplitStringFixed<AITriggerFieldCount>(line, ",", true);

	if (fields.Overflow > 0)
	{
		Debug::LogInfo("[AITrigger] {} - {} extra field(s) past index {} ignored",
			AITriggerTag(this), fields.Overflow, AITriggerFieldCount - 1);
	}

	// --- field 0: friendly name -------------------------------------------
	if (!fields.IsPresent(0))
	{
		LogFieldFail(this, 0);
		return false;
	}

	CopyName(this->Name, fields[0], 0x30);

	// --- field 1: Team1 ----------------------------------------------------
	if (!fields.IsPresent(1))
	{
		LogFieldFail(this, 1);
		return false;
	}

	{
		const std::string name = TrimmedField(fields[1], 0x17);
		this->Team1 = nullptr;

		if (_strcmpi(name.c_str(), GameStrings::NoneStr) != 0)          // != "<none>"
		{
			this->Team1 = TeamTypeClass::Find(name.c_str());

			// EXTENSION: vanilla silently leaves a null Team1 on a bad id.
			if (!this->Team1)
				LogFieldFailB(this, 1, name);
		}
	}

	// --- field 2: owner house ----------------------------------------------
	if (!fields.IsPresent(2))
	{
		LogFieldFail(this, 2);
		return false;
	}

	{
		const std::string name = TrimmedField(fields[2], 0x17);
		this->OwnerHouseType = AITriggerHouseType::None;
		this->HouseIndex = -1;

		if (_strcmpi(name.c_str(), GameStrings::AllStr) == 0)           // "<all>"
		{
			this->OwnerHouseType = AITriggerHouseType::Any;
		}
		else if (_strcmpi(name.c_str(), GameStrings::NoneStr) != 0)     // not "<none>"
		{
			this->HouseIndex = HouseTypeClass::FindIndexById(name.c_str());

			if (this->HouseIndex != -1)
				this->OwnerHouseType = AITriggerHouseType::Single;
			else
				LogFieldFailB(this, 2, name);                            // EXTENSION
		}
	}

	// --- field 3: INI tech level is read then DISCARDED --------------------
	if (!fields.IsPresent(3))
	{
		LogFieldFail(this, 3);
		return false;
	}

	this->TechLevel = 0; // ORIG: atoi(tok) computed, never stored

	// --- field 4: condition type -------------------------------------------
	if (!fields.IsPresent(4))
	{
		LogFieldFail(this, 4);
		return false;
	}

	{
		int value = 0;

		if (!TryParseInt(fields[4], value))
			LogFieldFailB(this, 4, fields[4]);

		this->ConditionType = static_cast<AITriggerCondition>(value);
	}

	// --- field 5: condition object = first matching techno type ------------
	if (!fields.IsPresent(5))
	{
		LogFieldFail(this, 5);
		return false;
	}

	{
		const std::string name = TrimmedField(fields[5], 0x17);
		this->ConditionObject = ResolveTechType(this, name.c_str());

		// EXTENSION: "<none>" is a legitimate value in the shipped INI, don't warn on it.
		if (!this->ConditionObject && _strcmpi(name.c_str(), GameStrings::NoneStr) != 0)
			LogFieldFailB(this, 5, name);
	}

	// --- field 6: condition list (opt. spaces, 2-char hex each, max 32) -----
	if (!fields.IsPresent(6))
	{
		LogFieldFail(this, 6);
		return false;
	}

	{
		const std::string_view list = fields[6];

		size_t i = 0;
		int n = 0;

		while (i < list.size())
		{
			// skip a run of spaces
			while (i < list.size() && std::isspace(static_cast<unsigned char>(list[i])))
				++i;

			// BUG: vanilla has no bounds test here. A field of pure whitespace made it
			//      read *p == '\0', then `*++p` stepped one byte PAST the terminator and
			//      the do/while re-tested that out-of-bounds byte. It also wrote one
			//      bogus 0 into _Conditions before that. BUGFIX: bail out instead.
			if (i >= list.size())
				break;

			// two-char scratch, always NUL-terminated (ORIG: strcpy(code, "00"))
			char code[3] = { list[i], '\0', '\0' };
			++i;

			if (i < list.size())
			{
				code[1] = list[i];
				++i;
			}

			if (n >= 0x20)                                  // cap at 32 conditions
			{
				// EXTENSION: vanilla drops the surplus silently.
				Debug::LogInfo("[AITrigger] {} - '{}' field has more than 32 entries, "
					"surplus ignored", AITriggerTag(this), AITriggerFieldNames[6]);
				break;
			}

			// VERIFY: strtol reconstructed from IDA std::lower_bound(v37, &v38, 0x10).
			// A trailing space landing in code[1] still parses as a 1-digit value, same
			// as vanilla. Lowercase hex ("50c30000...") is handled by base 16.
			char* endptr = nullptr;
			const long value = std::strtol(code, &endptr, 16);

			if (endptr == code)                             // EXTENSION: non-hex garbage
				LogFieldFailB(this, 6, std::string_view(code));

			this->_Conditions[n++] = static_cast<char>(value);
		}
	}

	// --- fields 7/8/9: weights (doubles) -----------------------------------
	// ORIG assigns only when the token exists, so IsPresent() gates each one.
	{
		const auto ReadWeight = [this, &fields, TryParseDouble, LogFieldFailB](size_t idx, double& target)
			{
				if (!fields.IsPresent(idx))
					return;

				double value = 0.0;

				if (!TryParseDouble(fields[idx], value))
					LogFieldFailB(this, idx, fields[idx]);

				target = value;
			};

		ReadWeight(7, this->Weight_Current);
		ReadWeight(8, this->Weight_Minimum);
		ReadWeight(9, this->Weight_Maximum);
	}

	// --- fields 10..17: flags and indices ----------------------------------
	{
		const auto ReadInt = [this, &fields, TryParseInt, LogFieldFailB](size_t idx, int& target)
			{
				if (!fields.IsPresent(idx))
					return;

				int value = 0;

				if (!TryParseInt(fields[idx], value))
					LogFieldFailB(this, idx, fields[idx]);

				target = value;
			};

		const auto ReadBool = [this, &fields, TryParseInt, LogFieldFailB](size_t idx, bool& target)
			{
				if (!fields.IsPresent(idx))
					return;

				int value = 0;

				if (!TryParseInt(fields[idx], value))
					LogFieldFailB(this, idx, fields[idx]);

				target = value != 0;
			};

		// field 10: for-skirmish
		ReadBool(10, this->IsForSkirmish);

		// field 11: unused. ORIG burned one strtok() call to stay aligned; with an
		//           index-based split no placeholder call is needed.

		// field 12: owning country (side). Confirmed by the shipped INI - Allied rows
		//           carry 1 here, Soviet rows carry 2.
		ReadInt(12, this->SideIndex);

		// field 13: for base defense
		ReadBool(13, this->IsForBaseDefense);

		// field 14: Team2 (only touched when the field exists)
		if (fields.IsPresent(14))
		{
			const std::string two = TrimmedField(fields[14], 0x17);
			this->Team2 = nullptr;

			if (_strcmpi(two.c_str(), GameStrings::NoneStr) != 0)       // != "<none>"
			{
				this->Team2 = TeamTypeClass::Find(two.c_str());

				if (!this->Team2)
					LogFieldFailB(this, 14, two);                        // EXTENSION
			}
		}

		// fields 15/16/17: per-difficulty enable
		ReadBool(15, this->Enabled_Easy);
		ReadBool(16, this->Enabled_Normal);
		ReadBool(17, this->Enabled_Hard);
	}

	// --- derive tech level = max(0, req(Team1), req(Team2)) ----------------
	// ORIG: two if/else "keep-or-raise" blocks -> plain max() each.
	if (this->Team1)
		this->TechLevel = std::max(this->TechLevel, this->Team1->TaskForce->TechLevelRequired());

	if (this->Team2)
		this->TechLevel = std::max(this->TechLevel, this->Team2->TaskForce->TechLevelRequired());

	return true;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2AB4, FakeAITriggerTypeClass::_LoadFromINI)
DEFINE_FUNCTION_JUMP(LJMP, 0x41F580, FakeAITriggerTypeClass::_LoadFromINI)

void __fastcall FakeAITriggerTypeClass::_ReadScenarioINI(CCINIClass* pINI, AITriggerType isGlobal) {

	const int count = pINI->GetKeyCount("AITriggerTypes");

	for (int i = 0; i < count; ++i) {
		const char* pEntry = pINI->GetKeyName("AITriggerTypes", i);

		AITriggerTypeClass* pTrigger = AITriggerTypeClass::FindOrAllocate(pEntry);

		pTrigger->LoadFromINI(pINI);
		pTrigger->Type = isGlobal;

		if (isGlobal == AITriggerType::Global)
			pTrigger->IsEnabled = true;
	}

	if (isGlobal == AITriggerType::Local)
		return;

	const int enableCount = pINI->GetKeyCount("AITriggerTypesEnable");

	for (int i = 0; i < enableCount; ++i) {
		const char* pEntry = pINI->GetKeyName("AITriggerTypesEnable", i);

		if (AITriggerTypeClass* pTrigger = AITriggerTypeClass::Find(pEntry)) {
			pTrigger->IsEnabled =
				pINI->ReadBool("AITriggerTypesEnable", pEntry, false) || (SessionClass::Instance->GameMode != GameMode::Campaign);
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x41F2E0, FakeAITriggerTypeClass::_ReadScenarioINI)

void __fastcall FakeAITriggerTypeClass::_WriteScenarioINI(CCINIClass* pINI, AITriggerType isGlobal) {

	const int count = pINI->GetKeyCount("AITriggerTypes");
	for (int i = 0; i < count; ++i) {
		const char* pEntry = pINI->GetKeyName("AITriggerTypes", i);

		char value[32];
		pINI->ReadString("AITriggerTypes", pEntry, "", value); // ORIG default: &Wstring::EmptyString
		pINI->Clear(value , nullptr);   // Clear(section = value)
	}
	pINI->Clear("AITriggerTypes" , nullptr);

	// 2) Re-emit every trigger whose IsGlobal matches this pass.
	for (int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
		AITriggerTypeClass* pType = AITriggerTypeClass::Array->Items[i];
		if (pType->Type == isGlobal)
			pType->SaveToINI(pINI);      // virtual AbstractTypeClass::Write_INI
	}

	// Global pass writes no enable flags.  (ORIG: if(!v2))
	if (isGlobal == AITriggerType::Local)
		return;

	// 3) Scenario pass also rewrites [AITriggerTypesEnable].
	pINI->Clear("AITriggerTypesEnable", nullptr);
	for (int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
		AITriggerTypeClass* pType = AITriggerTypeClass::Array->Items[i];
		pINI->WriteBool("AITriggerTypesEnable", pType->ID, pType->IsEnabled);  // ORIG: IniName
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x41F490, FakeAITriggerTypeClass::_WriteScenarioINI)