#include "ShowObjectCard.h"

#include <Ext/Observer/ObserverUI.h>
#include <Utilities/GeneralUtils.h>

const char* ShowObjectCardCommandClass::GetName() const
{
	return "ShowObjectCard";
}

const wchar_t* ShowObjectCardCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOW_OBJECT_CARD", L"Observer UI - Add Card");
}

const wchar_t* ShowObjectCardCommandClass::GetUICategory() const
{
	return CATEGORY_INFORMATION;
}

const wchar_t* ShowObjectCardCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SHOW_OBJECT_CARD_DESC", L"Add floating info card window for hovered or selected object.");
}

void ShowObjectCardCommandClass::Execute(WWKey eInput) const
{
	if (!Phobos::Config::DevelopmentCommands && !ObserverUIClass::IsActive())
		return;

	// If display mode is Hidden when adding a card, automatically switch to Minimal mode so the card renders
	if (ObserverUIClass::Instance.GetDisplayMode() == ObserverUIDisplayMode::Hidden)
	{
		ObserverUIClass::Instance.ToggleDisplayMode(); // Hidden -> Full or Minimal
	}

	ObserverUIClass::Instance.OpenFloatingWindowForSelectedObject();
}
