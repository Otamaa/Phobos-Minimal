#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Trigger/Body.h>

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
	if (auto it = triggerOwners.tryfind(pType->ArrayIndex)) {
		if (auto const pHouse = HouseClass::FindByPlayerAt(*it)) {
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

	if (auto it = triggerOwners.tryfind(pThis->Type->ArrayIndex)) {
		if (!HouseClass::FindByPlayerAt(*it)) {
			pThis->Destroy();
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

	ScenarioExtData::Instance()->TriggerTypePlayerAtXOwners.erase(pThis->Type->ArrayIndex);

	return 0;
}