#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <OverlayTypeClass.h>
#include <LightSourceClass.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <ScenarioClass.h>
#include <TriggerClass.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <TriggerTypeClass.h>

ASMJIT_PATCH(0x6DD614, TActionClass_LoadFromINI_GetActionIndex_ParamAsName, 0x6)
{
	GET(TActionClass*, pThis, EBP);

	if (pThis->ActionKind == TriggerAction::PlayAnimAt)
	{
		GET(char*, pName, ESI);

		if (GeneralUtils::IsValidString(pName) && (pName[0] < '0' || pName[0] > '9'))
		{
			const int idx = AnimTypeClass::FindIndexById(pName);

			if (idx >= 0)
				R->EDX(idx);
		}
	}

	return 0;
}
