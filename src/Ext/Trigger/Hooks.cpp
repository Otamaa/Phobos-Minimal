#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::LocalSet) ?
		0x72706E :
		0x727069;
}

ASMJIT_PATCH(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::GlobalSet) ?
		0x72702E :
		0x727029;
}

#pragma region PlayerAtX

// Store player slot index for trigger type if such value is used in scenario INI.
ASMJIT_PATCH(0x727292, TriggerTypeClass_ReadINI_PlayerAtX, 0x5)
{
	GET(TriggerTypeClass*, pThis, EBP);
	GET(const char*, pID, ESI);

	// Bail out early in campaign mode or if the name does not start with <
	if (SessionClass::IsCampaign() || *pID != '<')
		return 0;

	const int playerAtIndex = HouseClass::GetPlayerAtFromString(pID);

	if (playerAtIndex != -1)
	{
		ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners
			.insert(pThis->ArrayIndex, playerAtIndex);

		// Override the name to prevent Ares whining about non-existing HouseType names.
		R->ESI(GameStrings::NoneStr());
	}

	return 0;
}

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

// Destroy triggers with Player @ X owners if they are not present in scenario.
// ASMJIT_PATCH(0x725FC7, TriggerClass_CTOR_PlayerAtX, 0x7)
// {
// 	GET(TriggerClass*, pThis, ESI);

// 	if (SessionClass::IsCampaign())
// 		return 0;

// 	auto& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
// 	auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

// 	if (it != triggerOwners.end())
// 	{
// 		if (!HouseClass::FindByPlayerAt(it->second))
// 			pThis->Destroy();
// 	}

// 	return 0;
// }

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

ASMJIT_PATCH(0x71ECE1, TriggerClass_SpyAsInfantryOrHouse, 0x8)			// SpyAsInfantry
{
	GET(const int, iEvent, ESI);

	// This might form a unique condition that specifies the Country and InfantryType.
	if (iEvent == 53 || iEvent == 54)
		return R->Origin() + 0x8;

	return 0x71F163;
}ASMJIT_PATCH_AGAIN(0x71ED5E, TriggerClass_SpyAsInfantryOrHouse, 0x8)		// SpyAsHouse

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
				HouseClass* pEventOwner = nullptr;
				if (!SessionClass::IsCampaign()){

					auto const& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
					auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

					if (it != triggerOwners.end()) {
						if (auto const pHouse = HouseClass::FindByPlayerAt(it->second)) {
							pEventOwner = pHouse;
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

			for (std::size_t i = 0; i < pExt->ParallelTimersOriginalValue.size(); i++)
			{
				int timerValue = pExt->ParallelTimersOriginalValue[i];

				if (timerValue < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					timerValue = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(timerValue) * 0.5), static_cast<int>(std::abs(timerValue) * 1.5));
				}

				pExt->ParallelTimers[i].Start(15 * timerValue);
			}

			for (std::size_t i = 0; i < pExt->SequentialTimersOriginalValue.size(); i++)
			{
				int timerValue = pExt->SequentialTimersOriginalValue[i];

				if (timerValue < 0)
				{
					// Generate random value for event 51 "Delayed timer"
					timerValue = ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(std::abs(timerValue) * 0.5), static_cast<int>(std::abs(timerValue) * 1.5));
				}

				pExt->SequentialTimers[i].Start(15 * timerValue);
				pExt->SequentialTimers[i].Pause();
			}
		}
	}

	R->AL(allEventsSuccessful);

	return SkipGameCode;
}