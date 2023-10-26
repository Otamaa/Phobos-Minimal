#include "FPSCounter.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

const char* FPSCounterCommandClass::GetName() const
{
	return "FPS Counter";
}

const wchar_t* FPSCounterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_FPS_COUNTER", L"FPS Counter");
}

const wchar_t* FPSCounterCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* FPSCounterCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_FPS_COUNTER_DESC", L"Shows the current and an average of frames per second.");
}

void FPSCounterCommandClass::Execute(WWKey dwUnk) const
{
	RulesExtData::Instance()->FPSCounter = !RulesExtData::Instance()->FPSCounter;
}