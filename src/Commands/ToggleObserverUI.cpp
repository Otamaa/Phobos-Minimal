#include "ToggleObserverUI.h"

#include <Ext/Observer/ObserverUI.h>
#include <Utilities/GeneralUtils.h>

const char* ToggleObserverUICommandClass::GetName() const
{
	return "ToggleObserverUI";
}

const wchar_t* ToggleObserverUICommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_OBSERVER_UI", L"Observer UI");
}

const wchar_t* ToggleObserverUICommandClass::GetUICategory() const
{
	return CATEGORY_INFORMATION;
}

const wchar_t* ToggleObserverUICommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_OBSERVER_UI_DESC", L"Toggle Observer UI overlay display mode.");
}

void ToggleObserverUICommandClass::Execute(WWKey eInput) const
{
	if (!Phobos::Config::DevelopmentCommands && !ObserverUIClass::IsActive())
		return;

	ObserverUIClass::Instance.ToggleDisplayMode();
}
