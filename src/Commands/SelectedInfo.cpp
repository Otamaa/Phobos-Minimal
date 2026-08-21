#include "SelectedInfo.h"

#include <New/SelectedButton/SelectedInfoClass.h>
#include <Utilities/GeneralUtils.h>

const char* SelectedInfoCommandClass::GetName() const
{
	return "Selected Technos Display";
}

const wchar_t* SelectedInfoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECTED_INFO", L"Selected technos display");
}

const wchar_t* SelectedInfoCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* SelectedInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECTED_INFO_DESC", L"Switch between visible/invisible modes for selected technos info display");
}

void SelectedInfoCommandClass::Execute(WWKey eInput) const
{
	SelectedInfoClass::Instance.SwitchVisible();
}

const char* SelectedExpandCommandClass::GetName() const
{
	return "Selected Technos Display Expand";
}

const wchar_t* SelectedExpandCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECTED_EXPAND", L"Selected technos display expand");
}

const wchar_t* SelectedExpandCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* SelectedExpandCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECTED_EXPAND_DESC", L"Switch between expand/storage modes for selected technos info display");
}

void SelectedExpandCommandClass::Execute(WWKey eInput) const
{
	SelectedInfoClass::Instance.SwitchExpand();
}
