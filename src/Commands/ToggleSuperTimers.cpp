#include "ToggleSuperTimers.h"

#include <Utilities/GeneralUtils.h>

bool ToggleSuperTimersCommandClass::ShowSuperWeaponTimers = true;

const char* ToggleSuperTimersCommandClass::GetName() const
{
	return "ToggleSuperTimers";
}

const wchar_t* ToggleSuperTimersCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("Toggle_Special_Timers", L"Toggle Special Timers");
}

const wchar_t* ToggleSuperTimersCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleSuperTimersCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("Toggle_Special_Timers_DESC", L"Toggles the visibility of the special weapon timers on the tactical view.") ;
}

void ToggleSuperTimersCommandClass::Execute(WWKey dwUnk) const
{
	ShowSuperWeaponTimers = !ShowSuperWeaponTimers;
}
