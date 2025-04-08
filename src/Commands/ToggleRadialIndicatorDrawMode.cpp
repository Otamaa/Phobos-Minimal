#include "ToggleRadialIndicatorDrawMode.h"

#include <Utilities/GeneralUtils.h>

bool ToggleRadialIndicatorDrawModeClass::ShowForAll = false;

const char* ToggleRadialIndicatorDrawModeClass::GetName() const
{
	return "Toggle RadialIndicator Draw Mode";
}

const wchar_t* ToggleRadialIndicatorDrawModeClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TRIDRM", L"Toggle RadialIndicator Draw Mode");
}

const wchar_t* ToggleRadialIndicatorDrawModeClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleRadialIndicatorDrawModeClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TRIDRM_DESC", L"Toggle RadialIndicator Draw Mode.");
}

void ToggleRadialIndicatorDrawModeClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	//TODO : S/L ?
	ToggleRadialIndicatorDrawModeClass::ShowForAll = !ToggleRadialIndicatorDrawModeClass::ShowForAll;
}