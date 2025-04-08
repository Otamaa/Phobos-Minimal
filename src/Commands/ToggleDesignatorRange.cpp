#include "ToggleDesignatorRange.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

bool ToggleDesignatorRangeCommandClass::ShowDesignatorRange = false;

const char* ToggleDesignatorRangeCommandClass::GetName() const
{
	return "Toggle Designator Range";
}

const wchar_t* ToggleDesignatorRangeCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DESIGNATOR_RANGE", L"Toggle Designator Range");
}

const wchar_t* ToggleDesignatorRangeCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleDesignatorRangeCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DESIGNATOR_RANGE_DESC", L"Show/hide designator range when targeting superweapons.");
}

void ToggleDesignatorRangeCommandClass::Execute(WWKey eInput) const
{
	ShowDesignatorRange = !ShowDesignatorRange;
}