#include "Body.h"
#include <Ext/Techno/Body.h>

TeamExt::ExtContainer TeamExt::ExtMap;

bool TeamExt::ExtData::InvalidateIgnorable(void* const ptr) const {

	switch (GetVtableAddr(ptr))
	{
	case AircraftClass::vtable:
	case UnitClass::vtable:
	case InfantryClass::vtable:
	case ScriptClass::vtable:
	{
		return false;
	}
	}

	return true;
}

void TeamExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (this->InvalidateIgnorable(ptr))
		return;

	AnnounceInvalidPointer(TeamLeader, ptr);
	AnnounceInvalidPointer(MapPath_StartTechno, ptr);
	AnnounceInvalidPointer(MapPath_EndTechno, ptr);
	AnnounceInvalidPointer(MapPath_BridgeRepairHuts, ptr);
	AnnounceInvalidPointer(MapPath_ValidBridgeRepairHuts, ptr);
	AnnounceInvalidPointer(MapPath_CheckedBridgeRepairHuts, ptr);
	AnnounceInvalidPointer(PreviousScriptList, ptr);
}

void TeamExt::ExtData::InitializeConstants() {
	PreviousScriptList.reserve(10);
	MapPath_Grid.reserve(100);
	MapPath_Queue.reserve(100);
	MapPath_BridgeRepairHuts.reserve(100);
	MapPath_ValidBridgeRepairHuts.reserve(100);
	MapPath_CheckedBridgeRepairHuts.reserve(100);
}

void TeamExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }


bool TeamExt::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto const& pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExt::IsAlive(pObject))
				continue;

			if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto const& pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExt::IsAlive(pObject))
				continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	for (auto pHouse : *HouseClass::Array)
	{
		if (IS_SAME_STR_(SideClass::Array->GetItem(pHouse->Type->SideIndex)->Name, GameStrings::Civilian()))
			continue;

		// Count all objects of the list, like an OR operator
		for (auto const& pItem : list)
		{
			for (auto pObject : *TechnoClass::Array)
			{
				if (!TechnoExt::IsAlive(pObject))
					continue;

				if (pObject->Owner == pHouse
					&& pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (list.empty())
		return false;

	// Count all objects of the list, like an AND operator
	for (auto const& pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExt::IsAlive(pObject))
				continue;

			if (pObject->Owner == pHouse &&
				pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions[0].ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions[0].ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions[0].ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions[0].ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions[0].ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions[0].ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions[0].ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExt::EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	if (list.empty())
		return false;

	// Count all objects of the list, like an AND operator
	for (auto const& pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExt::IsAlive(pObject) || !pObject->Owner)
				continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions[0].ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions[0].ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions[0].ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions[0].ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions[0].ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions[0].ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions[0].ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExt::NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (list.empty())
		return false;

	// Any neutral house should be capable to meet the prerequisites
	for (auto pHouse : *HouseClass::Array)
	{
		if (!result)
			break;

		bool foundAll = true;

		if (IS_SAME_STR_(SideClass::Array->GetItem(pHouse->Type->SideIndex)->Name, GameStrings::Civilian()))
			continue;

		// Count all objects of the list, like an AND operator
		for (auto const& pItem : list)
		{
			if (!foundAll)
				break;

			int counter = 0;

			for (auto pObject : *TechnoClass::Array)
			{
				if (!TechnoExt::IsAlive(pObject))
					continue;

				if (pObject->Owner == pHouse &&
					pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}

			switch (pThis->Conditions[0].ComparatorOperand)
			{
			case 0:
				foundAll = counter < pThis->Conditions[0].ComparatorType;
				break;
			case 1:
				foundAll = counter <= pThis->Conditions[0].ComparatorType;
				break;
			case 2:
				foundAll = counter == pThis->Conditions[0].ComparatorType;
				break;
			case 3:
				foundAll = counter >= pThis->Conditions[0].ComparatorType;
				break;
			case 4:
				foundAll = counter > pThis->Conditions[0].ComparatorType;
				break;
			case 5:
				foundAll = counter != pThis->Conditions[0].ComparatorType;
				break;
			default:
				break;
			}
		}

		if (!foundAll)
			result = false;
	}

	return result;
}

// =============================
// load / save

template <typename T>
void TeamExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->WaitNoTargetAttempts)
		.Process(this->NextSuccessWeightAward)
		.Process(this->IdxSelectedObjectFromAIList)
		.Process(this->CloseEnough)
		.Process(this->Countdown_RegroupAtLeader)
		.Process(this->MoveMissionEndMode)
		.Process(this->WaitNoTargetCounter)
		.Process(this->WaitNoTargetTimer)
		.Process(this->ForceJump_Countdown)
		.Process(this->ForceJump_InitialCountdown)
		.Process(this->ForceJump_RepeatMode)
		.Process(this->TeamLeader)
		.Process(this->GenericStatus)
		.Process(this->FailedCounter)

		.Process(this->AngerNodeModifier)
		.Process(this->OnlyTargetHouseEnemy)
		.Process(this->OnlyTargetHouseEnemyMode)

		.Process(this->ConditionalJump_Evaluation)
		.Process(this->ConditionalJump_ComparatorMode)
		.Process(this->ConditionalJump_ComparatorValue)
		.Process(this->ConditionalJump_EnabledKillsCount)
		.Process(this->ConditionalJump_Counter)
		.Process(this->AbortActionAfterKilling)
		.Process(this->ConditionalJump_Index)
		.Process(this->ConditionalJump_ResetVariablesIfJump)

		.Process(this->PreviousScriptList)

		.Process(this->TriggersSideIdx)
		.Process(this->TriggersHouseIdx)

		.Process(this->MapPath_Grid)
		.Process(this->MapPath_Queue)
		.Process(this->MapPath_InProgress)
		.Process(this->MapPath_StartTechno)
		.Process(this->MapPath_EndTechno)
		.Process(this->MapPath_BridgeRepairHuts)
		.Process(this->MapPath_ValidBridgeRepairHuts)
		.Process(this->MapPath_CheckedBridgeRepairHuts)
		;
}

void TeamExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TeamExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TeamExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TeamExt::ExtContainer::ExtContainer() : Container("TeamClass") { }
TeamExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//Everything InitEd
DEFINE_HOOK(0x6E8D05, TeamClass_CTOR, 0x5)
{
	GET(TeamClass*, pThis, ESI);
	TeamExt::ExtMap.Allocate(pThis);
	return 0;
}

DEFINE_HOOK(0x6E8ECB, TeamClass_DTOR, 0x7)
{
	GET(TeamClass*, pThis, ESI);
	TeamExt::ExtMap.Remove(pThis);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6EC450, TeamClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6EC540, TeamClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TeamClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TeamExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6EC52F, TeamClass_Load_Suffix, 0x6)
{
	TeamExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6EC55A, TeamClass_Save_Suffix, 0x5)
{
	TeamExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x6EAEC7, TeamClass_Detach, 0x5)
{
	GET(TeamClass*, pThis, ECX);
	GET(void*, target, EAX);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pExt = TeamExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->Target == target ? 0x6EAECC : 0x6EAECF;
}
