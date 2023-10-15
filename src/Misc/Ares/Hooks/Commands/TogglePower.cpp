#include "TogglePower.h"

#include <Ext/Rules/Body.h>
#include <MapClass.h>
#include <Utilities/GeneralUtils.h>

const char* TogglePowerCommandClass::GetName() const
{
	return "TogglePower";
}

const wchar_t* TogglePowerCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_POWER", L"Toggle Power Mode");
}

const wchar_t* TogglePowerCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* TogglePowerCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_POWER_DESC", L"Turn toggle power mode on / off.");
}

void TogglePowerCommandClass::Execute(WWKey dwUnk) const
{
	if (RulesExtData::Instance()->TogglePowerAllowed)
	{
		MapClass::Instance->SetTogglePowerMode(-1);
	}
}
