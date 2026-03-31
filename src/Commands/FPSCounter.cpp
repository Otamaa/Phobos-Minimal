#include "FPSCounter.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>

const char* FPSCounterCommandClass::GetName() const
{
	return "FPS Counter";
}

const wchar_t* FPSCounterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FPS_COUNTER", L"FPS Counter");
}

const wchar_t* FPSCounterCommandClass::GetUICategory() const
{
	return CATEGORY_GUIDEBUG;
}

const wchar_t* FPSCounterCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FPS_COUNTER_DESC", L"Shows the current and an average of frames per second.");
}

void FPSCounterCommandClass::Execute(WWKey dwUnk) const
{
	const BYTE mode = (BYTE)(RulesExtData::Instance()->FPSCounter);
	RulesExtData::Instance()->FPSCounter = (FPSCounterMode)((mode + 1) % (BYTE)FPSCounterMode::count);

	// Debug print
	//switch (RulesExtData::Instance()->FPSCounter)
	//{
	//case FPSCounterMode::disabled: Debug::Log("FPS Counter: Disabled\n"); break;
	//case FPSCounterMode::Full:  Debug::Log("FPS Counter: Full\n"); break;
	//case FPSCounterMode::FPSOnly: Debug::Log("FPS Counter: FPS Only\n"); break;
	//case FPSCounterMode::FPSandAVG: Debug::Log("FPS Counter: FPS and AVG\n"); break;
	//}
}