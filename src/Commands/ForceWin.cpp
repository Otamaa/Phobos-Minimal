#include "ForceWin.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* ForceWinCommandClass::GetName() const
{
	return "Force Win";
}

const wchar_t* ForceWinCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FOCE_WIN", L"Toggle Force Win");
}

const wchar_t* ForceWinCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* ForceWinCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FOCE_WIN_DESC", L"Force Current Player to WIN the battle");
}

void ForceWinCommandClass::Execute(WWKey eInput) const
{
	HouseClass::CurrentPlayer->Win(false);
}