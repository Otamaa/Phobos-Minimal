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

ASMJIT_PATCH(0x6E08DE, TActionClass_SellBack_LimboDelivered, 0x6)
{
	enum { forbidden = 0x6E0907, allow = 0x0 };

	GET(BuildingClass*, pBld, ESI);
	return BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0 ?
		forbidden : allow;
}

namespace RetintTemp
{
	bool UpdateLightSources = false;
}

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

#include <Ext/Rules/Body.h>

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome, Starkku

ASMJIT_PATCH(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Flag the light sources to update, actually do it later and only once to prevent redundancy.
	RetintTemp::UpdateLightSources = RulesExtData::Instance()->UseRetintFix;

	return 0;
}ASMJIT_PATCH_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
ASMJIT_PATCH_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
//	ScenarioExtData::Instance()->CurrentTint_Schemes = tint;
//	ScenarioExtData::Instance()->CurrentTint_Hashes = tint;
//
//	return 0;
//}