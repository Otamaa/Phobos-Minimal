#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= (int)PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= (int)PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::LocalSet) ?
		0x72706E :
		0x727069;
}

ASMJIT_PATCH(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable ||
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

ASMJIT_PATCH(0x72652D, TriggerClass_Logic_PlayerAtX, 0x6)
{
	enum { SkipGameCode1 = 0x726538, SkipGameCode2 = 0x726602 };

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
			return R->Origin() == 0x72652D ? SkipGameCode1 : SkipGameCode2;
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x7265F7, TriggerClass_Logic_PlayerAtX, 0x6)

// Destroy triggers with Player @ X owners if they are not present in scenario.
ASMJIT_PATCH(0x725FC7, TriggerClass_CTOR_PlayerAtX, 0x7)
{
	GET(TriggerClass*, pThis, ESI);

	if (SessionClass::IsCampaign())
		return 0;

	auto& triggerOwners = ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners;
	auto it = triggerOwners.get_key_iterator(pThis->Type->ArrayIndex);

	if (it != triggerOwners.end())
	{
		if (!HouseClass::FindByPlayerAt(it->second))
			pThis->Destroy();
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

ASMJIT_PATCH(0x71ECE1, TriggerClass_SpyAsInfantryOrHouse, 0x8)			// SpyAsInfantry
{
	GET(const int, iEvent, ESI);

	// This might form a unique condition that specifies the Country and InfantryType.
	if (iEvent == 53 || iEvent == 54)
		return R->Origin() + 0x8;

	return 0x71F163;
}ASMJIT_PATCH_AGAIN(0x71ED5E, TriggerClass_SpyAsInfantryOrHouse, 0x8)		// SpyAsHouse

#pragma endregion