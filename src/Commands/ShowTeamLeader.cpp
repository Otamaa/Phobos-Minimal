#include "ShowTeamLeader.h"

#include <Utilities/GeneralUtils.h>

bool ShowTeamLeaderCommandClass::Show = false;

const char* ShowTeamLeaderCommandClass::GetName() const
{
	return "Show Techno Names";
}

const wchar_t* ShowTeamLeaderCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TMLDR", L"Show TeamLeader");
}

const wchar_t* ShowTeamLeaderCommandClass::GetUICategory() const
{
	return CATEGORY_GUIDEBUG;
}

const wchar_t* ShowTeamLeaderCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TMLDR_DESC", L"Show TeamLeader.");
}

void ShowTeamLeaderCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	ShowTeamLeaderCommandClass::Show = !ShowTeamLeaderCommandClass::Show;
}

bool ShowTeamLeaderCommandClass::IsActivated()
{
	return ShowTeamLeaderCommandClass::Show;
}