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
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game (Singleplayer only).");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign
		|| SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		auto nMessage = StringTable::LoadString(GameStrings::TXT_SAVING_GAME());
		auto pUI = UI::sub_623230((LPARAM)nMessage, 0, 0);
		WWMouseClass::Instance->HideCursor();

		if (pUI)
		{
			UI::FocusOnWindow(pUI);
		}

		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};


		SYSTEMTIME time {};
		Imports::GetLocalTime.get()(&time);
		const std::string fName = std::format("Map.{:04}{:02}{:02}-{:02}{:02}{:02}-{:05}.sav",
			time.wYear,
			time.wMonth,
			time.wDay,
			time.wHour,
			time.wMinute,
			time.wSecond,
			time.wMilliseconds
		);

		const std::wstring fDesc = std::format(L"{} - {}"
			, SessionClass::Instance->GameMode == GameMode::Campaign ? ScenarioClass::Instance->UINameLoaded : ScenarioClass::Instance->Name
			, GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved")
		);

		bool Status = ScenarioClass::SaveGame(fName.c_str(), fDesc.c_str());

		WWMouseClass::Instance->ShowCursor();

		if (pUI)
		{
			UI::EndDialog(pUI);
		}

		if (Status)
			PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
		else
			PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
	}
	else
	{
		MessageListClass::Instance->PrintMessage(
		GeneralUtils::LoadStringUnlessMissing("MSG:NotAvailableInMultiplayer", L"QuickSave is not available in multiplayer"),
		RulesClass::Instance->MessageDelay,
		HouseClass::CurrentPlayer->ColorSchemeIndex,
		true);
	}
}
