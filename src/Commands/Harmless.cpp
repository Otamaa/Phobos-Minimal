#include "Harmless.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Event/Body.h>

bool HarmlessCommandClass::InHarmlessMode = false;

const char* HarmlessCommandClass::GetName() const
{
	return "Make all non player controlled Harmless";
}

const wchar_t* HarmlessCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_Harmless", L"Harmless");
}

const wchar_t* HarmlessCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* HarmlessCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_Harmless_DESC", L"Make all non player controlled Harmless.");
}

void HarmlessCommandClass::AI()
{
	if (!InHarmlessMode)
		return;

	for (auto pTechno : *TechnoClass::Array) {

		if(pTechno->Owner != HouseClass::CurrentPlayer())
			pTechno->QueueMission(Mission::Harmless, true);
	}
}

void HarmlessCommandClass::Execute(WWKey eInput) const
{
	InHarmlessMode = !InHarmlessMode;
}
