#include "QuickSave.h"

#include <ScenarioClass.h>
#include <SessionClass.h>
#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>

const char* QuickSaveCommandClass::GetName() const
{
	return "Quicksave";
}

const wchar_t* QuickSaveCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_QUICKSAVE_DESC", L"Save the current game (Singleplayer only).");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{

	if (SessionClass::IsSingleplayer())
	{
		ScenarioClass::ScenarioSaved = false;
		Phobos::ShouldQuickSave = true;

		if (SessionClass::IsCampaign())
			Phobos::CustomGameSaveDescription = ScenarioClass::Instance->UINameLoaded;
		else
			Phobos::CustomGameSaveDescription = ScenarioClass::Instance->Name;
		Phobos::CustomGameSaveDescription += L" - ";
		Phobos::CustomGameSaveDescription += GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_QUICKSAVE_SUFFIX", L"Quicksaved");
	}
	else
	{
		auto pMessage = GeneralUtils::LoadStringUnlessMissingNoChecks("MSG:NotAvailableInMultiplayer", L"QuickSave is not available in multiplayer");
		GeneralUtils::PrintMessage(pMessage);
	}
}
