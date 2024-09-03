#include "ShowHealthPercent.h"

#include <Utilities/GeneralUtils.h>

// CommandClass
const char* ShowHealthPercentCommandClass::GetName() const
{
	return "Show Health Percent";
}

const wchar_t* ShowHealthPercentCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWHEALTHPERCENT", L"Show Health Percent");
}

const wchar_t* ShowHealthPercentCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ShowHealthPercentCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOWHEALTHPERCENT_DESC", L"Show health percent of all technos if available.");
}

void ShowHealthPercentCommandClass::Execute(WWKey eInput) const
{
	Phobos::Otamaa::ShowHealthPercentEnabled = !Phobos::Otamaa::ShowHealthPercentEnabled;
}
