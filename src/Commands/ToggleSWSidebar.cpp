#include "ToggleSWSidebar.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Scenario/Body.h>

#include <New/SuperWeaponSidebar/ToggleSWButtonClass.h>
#include <New/SuperWeaponSidebar/SWSidebarClass.h>

const char* ToggleSWSidebar::GetName() const
{
	return "Toggle SuperWeapon Sidebar";
}

const wchar_t* ToggleSWSidebar::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TOGGLE_SW_SIDEBAR", L"Toggle SuperWeapon Sidebar");
}

const wchar_t* ToggleSWSidebar::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleSWSidebar::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TOGGLE_SW_SIDEBAR_DESC", L"Toggle SuperWeapon Sidebar.");
}

void ToggleSWSidebar::Execute(WWKey eInput) const
{
	ToggleSWButtonClass::SwitchSidebar();

	const auto pMessage = ScenarioExtData::Instance()->SWSidebar_Enable ?
		GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_EX_SW_BAR_VISIBLE", L"Set exclusive SW sidebar visible.") :
		GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_EX_SW_BAR_INVISIBLE", L"Set exclusive SW sidebar invisible.");

	GeneralUtils::PrintMessage(pMessage);

	if (SWSidebarClass::Global()->CurrentColumn)
		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}