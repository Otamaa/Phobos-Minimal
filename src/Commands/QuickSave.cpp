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
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_INTERFACE", L"Interface");
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game (Singleplayer only).");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		char fName[0x80];
		SYSTEMTIME time;

		Imports::GetLocalTime.get()(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		MessageListClass::Instance->PrintMessage(
		StringTable::LoadString(GameStrings::TXT_SAVING_GAME()),
		RulesClass::Instance->MessageDelay,
		HouseClass::CurrentPlayer->ColorSchemeIndex,
		true);

		wchar_t fDescription[0x80] = { 0 };
		if (SessionClass::Instance->GameMode == GameMode::Campaign)
			wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		else
			wcscpy_s(fDescription, ScenarioClass::Instance->Name);

		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved"));

		if (ScenarioClass::SaveGame(fName, fDescription)) {
			MessageListClass::Instance->PrintMessage(
			StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED()),
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true);
		}
		else {
			MessageListClass::Instance->PrintMessage(
			StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME()),
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true);
		}
	}
	else
	{
		MessageListClass::Instance->PrintMessage(
		StringTable::LoadString("MSG:NotAvailableInMultiplayer"),
		RulesClass::Instance->MessageDelay,
		HouseClass::CurrentPlayer->ColorSchemeIndex,
		true);
	}
}
