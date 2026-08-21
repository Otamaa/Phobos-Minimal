#include "HerosInfo.h"

#include <New/UniqueButton/UniqueTechnoColumnClass.h>

#include <Utilities/GeneralUtils.h>

const char* HerosInfoCommandClass::GetName() const
{
	return "Heros Display";
}

const wchar_t* HerosInfoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_HEROS_INFO", L"Heros display");
}

const wchar_t* HerosInfoCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* HerosInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_HEROS_INFO_DESC", L"Switch between visible/invisible modes for heros info display");
}

void HerosInfoCommandClass::Execute(WWKey eInput) const
{
	UniqueTechnoColumnClass::Instance.SwitchVisible();
}
