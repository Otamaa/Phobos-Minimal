#include <Helpers/Macro.h>
#include <WWMessageBox.h>
#include <LoadOptionsClass.h>
#include <ThemeClass.h>
#include <CCToolTip.h>
#include <GameOptionsClass.h>
#include <GScreenClass.h>
#include <EvadeClass.h>

#include <GameStrings.h>
#include <Utilities/Constructs.h>
#include <Utilities/Debug.h>

namespace RetryDialogFlag
{
	bool IsCalledFromRetryDialog = false;

	void ReloadUIParts() JMP_STD(0x72DFB0);
}

ASMJIT_PATCH(0x686092, DoLose_RetryDialogForCampaigns, 0x7)
{
	enum { OK = 0x6860F6, Cancel = 0x6860EE, LoadGame = 0x686231 };

	while (true)
	{
		// WWMessageBox buttons look like below:
		// Button1
		// Button3
		// Button2
		// I prefer to put the loadgame to the center of them - secsome
		// Did you??? NO, YOU DIDN'T. Bruhhhh
		switch (WWMessageBox::Instance().Process(
			StringTable::FetchString(GameStrings::TXT_TO_REPLAY()),
			StringTable::FetchString("GUI:LOADGAME"),
			StringTable::FetchString(GameStrings::TXT_CANCEL()),
			StringTable::FetchString(GameStrings::TXT_OK())))
		{
			case WWMessageBox::Result::Button3:
			return OK;

		default:
		case WWMessageBox::Result::Button2:
			return Cancel;

		case WWMessageBox::Result::Button1:
			auto pDialog = GameCreate<LoadOptionsClass>();
			RetryDialogFlag::IsCalledFromRetryDialog = true;
			const bool bIsAboutToLoad = pDialog->LoadDialog();
			RetryDialogFlag::IsCalledFromRetryDialog = false;
			GameDelete<true,false>(pDialog);

			if (!bIsAboutToLoad)
				continue;

			ThemeClass::Instance->Stop();
			break;
		}
	}

	EvadeClass::Instance->Do();

	if (CCToolTip::Instance())
		CCToolTip::Instance->SetState(GameOptionsClass::Instance->Tooltips);

	GScreenClass::Instance->Render();
	RetryDialogFlag::ReloadUIParts();

	return LoadGame;
}

ASMJIT_PATCH(0x558F4E, LoadOptionClass_Dialog_CenterListBox, 0x5)
{
	if (RetryDialogFlag::IsCalledFromRetryDialog)
	{
		GET(HWND, hListBox, EAX);
		GET(HWND, hDialog, EDI);

		HWND hLoadButton = Imports::GetDlgItem.invoke()(hDialog, 1039);

		RECT buttonRect {};
		Imports::GetWindowRect.invoke()(hLoadButton, &buttonRect);

		float scaleX = static_cast<float>(buttonRect.right - buttonRect.left) / 108;
		float scaleY = static_cast<float>(buttonRect.bottom - buttonRect.top) / 22;
		int X = buttonRect.left - static_cast<int>(346 * scaleX);
		int Y = buttonRect.top - static_cast<int>(44 * scaleY);

		Imports::SetWindowPos.invoke()(hListBox, NULL, X, Y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	}

	return 0;
}