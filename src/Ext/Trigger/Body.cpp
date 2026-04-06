#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/TEvent/Body.h>

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>
#include <Utilities/Macro.h>
#include <Notifications.h>

// =============================
// load / save

template <typename T>
void TriggerExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Name)
		.Process(this->SortedEventsList)
		.Process(this->SequentialTimers)
		.Process(this->SequentialTimersOriginalValue)
		.Process(this->ParallelTimers)
		.Process(this->ParallelTimersOriginalValue)
		.Process(this->SequentialSwitchModeIndex)
		;
}

// =============================
// container
TriggerExtContainer TriggerExtContainer::Instance;

// =============================
// container hooks

ASMJIT_PATCH(0x72612C, TriggerClass_CTOR, 0x7)
{

	GET(TriggerClass*, pThis, ESI);
	TriggerExtContainer::Instance.Allocate(pThis);

	if (!SessionClass::IsCampaign()) {
		auto& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
		auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

		if (it != triggerOwners.end())
		{
			if (!HouseClass::FindByPlayerAt(it->second))
				pThis->Destroy();
		}
	}

	if (pThis->Type) {

		auto pExt = TriggerExtContainer::Instance.Find(pThis);
		auto pCurrentEvent = pThis->Type->FirstEvent;

		while (pCurrentEvent)
		{
			pExt->SortedEventsList.emplace_back(pCurrentEvent);
			pCurrentEvent = pCurrentEvent->NextEvent;
		}

		std::reverse(pExt->SortedEventsList.begin(), pExt->SortedEventsList.end());

		for (std::size_t i = 0; i < pExt->SortedEventsList.size(); i++)
		{
			pCurrentEvent = pExt->SortedEventsList[i];

			if (static_cast<PhobosTriggerEvent>(pCurrentEvent->EventKind) == PhobosTriggerEvent::ForceSequentialEvents)
			{
				pExt->SequentialSwitchModeIndex = i;
				continue;
			}

			if (pCurrentEvent->EventKind != TriggerEvent::ElapsedTime && pCurrentEvent->EventKind != TriggerEvent::RandomDelay)
				continue;

			int countdown = 0;

			if (pCurrentEvent->EventKind == TriggerEvent::ElapsedTime) // Event 13 "Elapsed Time..."
				countdown = pCurrentEvent->Value;
			else // Event 51 "Random delay..."
				countdown = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(pCurrentEvent->Value * 0.5), static_cast<int>(pCurrentEvent->Value * 1.5));

			if (pExt->SequentialSwitchModeIndex >= 0)
			{
				pExt->SequentialTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
				pExt->SequentialTimers[i].Start(15 * countdown);
				pExt->SequentialTimers[i].Pause();
			}
			else
			{
				pExt->ParallelTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
				pExt->ParallelTimers[i].Start(15 * countdown);
			}
		}
	}
	return 0;
}

ASMJIT_PATCH(0x72617D, TriggerClass_DTOR, 0xF)
{
	GET(TriggerClass*, pThis, ESI);

	TriggerExtContainer::Instance.Remove(pThis);

	return 0;
}

void FakeTriggerClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(pTarget, bRemove, pTarget->WhatAmI());

	this->TriggerClass::PointerExpired(pTarget, bRemove);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5880, FakeTriggerClass::_Detach)