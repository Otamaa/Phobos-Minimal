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

TriggerExtData::TriggerExtData(TriggerClass* pObj) : AbstractExtended(pObj)
{
	this->Name = pObj->Type->ID;
	this->AbsType = TriggerClass::AbsID;

	if (!SessionClass::IsCampaign())
	{
		auto& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
		auto it = triggerOwners.get_key_iterator(pObj->Type->ArrayIndex);

		if (it != triggerOwners.end())
		{
			if (!HouseClass::FindByPlayerAt(it->second)) { 
				pObj->Destroy();
				//return;
			}
		}
	}

	auto pCurrentEvent = pObj->Type->FirstEvent;

	while (pCurrentEvent)
	{
		this->SortedEventsList.emplace_back(pCurrentEvent);
		pCurrentEvent = pCurrentEvent->NextEvent;
	}

	std::reverse(this->SortedEventsList.begin(), this->SortedEventsList.end());

	for (std::size_t i = 0; i < this->SortedEventsList.size(); i++)
	{
		pCurrentEvent = this->SortedEventsList[i];

		if (static_cast<PhobosTriggerEvent>(pCurrentEvent->EventKind) == PhobosTriggerEvent::ForceSequentialEvents)
		{
			this->SequentialSwitchModeIndex = i;
			continue;
		}

		if (pCurrentEvent->EventKind != TriggerEvent::ElapsedTime && pCurrentEvent->EventKind != TriggerEvent::RandomDelay)
			continue;

		int countdown = 0;

		if (pCurrentEvent->EventKind == TriggerEvent::ElapsedTime) // Event 13 "Elapsed Time..."
			countdown = pCurrentEvent->Value;
		else // Event 51 "Random delay..."
			countdown = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(pCurrentEvent->Value * 0.5), static_cast<int>(pCurrentEvent->Value * 1.5));

		if (this->SequentialSwitchModeIndex >= 0)
		{
			this->SequentialTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
			this->SequentialTimers[i].Start(15 * countdown);
			this->SequentialTimers[i].Pause();
		}
		else
		{
			this->ParallelTimersOriginalValue[i] = pCurrentEvent->EventKind == TriggerEvent::ElapsedTime ? pCurrentEvent->Value : pCurrentEvent->Value * -1;
			this->ParallelTimers[i].Start(15 * countdown);
		}
	}
}

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

	if (!Phobos::Otamaa::DoingLoadGame)
		TriggerExtContainer::Instance.Allocate(pThis);

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