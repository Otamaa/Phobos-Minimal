#include "ClearObserverUICards.h"

#include <Ext/Observer/ObserverUI.h>
#include <Utilities/GeneralUtils.h>

const char* ClearObserverUICardsCommandClass::GetName() const
{
	return "ClearObserverUICards";
}

const wchar_t* ClearObserverUICardsCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CLEAR_OBSERVER_UI_CARDS", L"Observer UI - Clear Cards");
}

const wchar_t* ClearObserverUICardsCommandClass::GetUICategory() const
{
	return CATEGORY_INFORMATION;
}

const wchar_t* ClearObserverUICardsCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CLEAR_OBSERVER_UI_CARDS_DESC", L"Clear all open floating info card windows.");
}

void ClearObserverUICardsCommandClass::Execute(WWKey eInput) const
{
	if (!Phobos::Config::DevelopmentCommands && !ObserverUIClass::IsActive())
		return;

	ObserverUIClass::Instance.ClearFloatingWindows();
}
