#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Scenario/Body.h>

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