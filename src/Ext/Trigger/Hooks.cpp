#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Scenario/Body.h>

#pragma region PlayerAtX

// Handle mapping player slot index for trigger to HouseClass pointer in logic.

ASMJIT_PATCH(0x7265F7, TriggerClass_Logic_PlayerAtX, 0x6)
{
	enum { SkipGameCode = 0x726602 };

	GET(TriggerTypeClass*, pType, EDX);

	if (SessionClass::IsCampaign())
		return 0;

	auto const& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.get_key_iterator(pType->ArrayIndex);

	if (it != triggerOwners.end())
	{
		if (auto const pHouse = HouseClass::FindByPlayerAt(it->second))
		{
			R->EAX(pHouse);
			return SkipGameCode;
		}
	}

	return 0;
}

// Remove destroyed triggers from the map.
ASMJIT_PATCH(0x726727, TriggerClass_Destroy_PlayerAtX, 0x5)
{
	GET(TriggerClass*, pThis, ESI);

	if (SessionClass::IsCampaign())
		return 0;

	auto& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

	if (it != triggerOwners.end())
		triggerOwners.erase(it);

	return 0;
}

#pragma endregion

// TriggerClass::RegisterEvent(...) rewrite
ASMJIT_PATCH(0x7264C0, TriggerClass_RegisterEvent_ForceSequentialEvents, 0x7)
{
	enum { SkipGameCode = 0x7265B8 };

	GET(TriggerClass*, pThis, ECX);
	GET_STACK(TriggerEvent, nEvent, 0x4);
	GET_STACK(TechnoClass*, pTechno, 0x8);
	GET_STACK(bool, skipStuff, 0xC);
	GET_STACK(bool, isPersistant, 0x10);
	GET_STACK(ObjectClass*, pPayback, 0x14);

	if (!pThis->Enabled || pThis->Destroyed)
	{
		R->AL(false);
		return SkipGameCode;
	}
	auto pExt = TriggerExtContainer::Instance.Find(pThis);
	bool isSequentialMode = false; // Flag: Controls if short-circuit is active for subsequent events
	bool allEventsSuccessful = true;
	int nPredecessorEventsCompleted = 0;

	if (!skipStuff)
	{
		// Check status of the trigger events in sequential logic (INI order)
		for (std::size_t i = 0; i < pExt->SortedEventsList.size(); i++)
		{
			const auto pCurrentEvent = pExt->SortedEventsList[i];
			bool alreadyOccured = pThis->HasEventOccured(i);
			bool triggeredNow = false;
			auto eventTimer = pThis->Timer; // Fallback

			if (pExt->ParallelTimers.contains(i))
			{
				eventTimer = pExt->ParallelTimers[i];
			}
			else if (pExt->SequentialTimers.contains(i))
			{
				eventTimer = pExt->SequentialTimers[i];

				if (eventTimer.HasTimeLeft()
				&& !eventTimer.InProgress()
				&& !eventTimer.Completed())
				{
					pExt->SequentialTimers[i].Resume();
					eventTimer.Resume();
				}
			}

			// *** 1. Lógica del Interruptor de Modo (Evento 1000) ***
			if (static_cast<PhobosTriggerEvent>(pCurrentEvent->EventKind) == PhobosTriggerEvent::ForceSequentialEvents)
			{
				bool predecessorsCompleted = false;

				if (nPredecessorEventsCompleted >= pExt->SequentialSwitchModeIndex)
					predecessorsCompleted = true;

				if (predecessorsCompleted)
				{
					pThis->MarkEventAsOccured(i);
					alreadyOccured = true;
					triggeredNow = true;
					isSequentialMode = true; // Activate sequential mode for the rest of the INI events
				}
				else
				{
					allEventsSuccessful = false;
					R->AL(false);
					return SkipGameCode; // Short-circuit
				}
			}

			if (pExt->SequentialTimers.contains(i)
				&& eventTimer.HasTimeLeft()
				&& !eventTimer.InProgress()
				&& !eventTimer.Completed())
			{
				pExt->SequentialTimers[i].Resume();
				eventTimer = pExt->SequentialTimers[i];
			}

			if (!alreadyOccured)
			{
				// Default: resolve owner from country name (matches original game behavior).
				HouseClass* pEventOwner = HouseClass::FindByCountryName(pThis->Type->House->ID);

				// In MP, override with the PlayerAtX-mapped house if one is registered for this trigger type.
				if (!SessionClass::IsCampaign()){

					auto const& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
					auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

					if (it != triggerOwners.end()) {
						if (auto const pHouse = HouseClass::FindByPlayerAt(it->second)) {
							pEventOwner = pHouse;
							break;
						}
					}

				} else {
					pEventOwner = HouseClass::FindByCountryName(pThis->Type->House->ID);
				}
		
				triggeredNow = ((FakeTEventClass*)pCurrentEvent)->_Occured(
									nEvent,
									pEventOwner,
									pTechno,
									&eventTimer,
									&isPersistant,
									pPayback);
			}

			if (alreadyOccured || triggeredNow)
			{
				HouseClass* pNewHouse = pCurrentEvent->House;

				if (pNewHouse)
					pThis->House = pNewHouse;

				if (isPersistant && pCurrentEvent->GetStateA() && pCurrentEvent->GetStateB())
					pThis->MarkEventAsOccured(i); //pThis->OccuredEvents |= eventBit;

				nPredecessorEventsCompleted++;
			}
			else
			{
				// Conditional short-circuit on Failure
				allEventsSuccessful = false;

				if (isSequentialMode)
				{
					R->AL(false);
					return SkipGameCode;
				}
			}
		}
	}

	if (allEventsSuccessful || skipStuff)
	{
		if (isPersistant)
		{
			pThis->ResetTimers(); // Is really needed now? Maybe, because YRpp is incomplete and looks that each event have its own timer inside a struct... or something similar. I'll preserve this for now that doesn't hurt having this here...

			for (auto& [i, timerValue] : pExt->ParallelTimersOriginalValue)
			{
				int value = timerValue;

				if (value < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					value = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(value) * 0.5), static_cast<int>(std::abs(value) * 1.5));
				}

				pExt->ParallelTimers[i].Start(15 * value);
			}

			for (auto& [i, timerValue] : pExt->SequentialTimersOriginalValue)
			{
				int value = timerValue;

				if (value < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					value = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(value) * 0.5), static_cast<int>(std::abs(value) * 1.5));
				}

				pExt->SequentialTimers[i].Start(15 * value);
				pExt->SequentialTimers[i].Pause();
			}
		}
	}

	R->AL(allEventsSuccessful);

	return SkipGameCode;
}