#include <Phobos.h>

#include <Utilities/Macro.h>
#include <PreviewClass.h>
#include <Surface.h>
#include <CreditClass.h>
#include <ThemeClass.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/EnumFunctions.h>

ASMJIT_PATCH(0x777C41, UI_ApplyAppIcon, 0x9)
{
	GET(HINSTANCE , instance , ESI);

	if (!Phobos::AppIconPath.empty()) {
		Debug::LogInfo("Applying AppIcon from \"{}\"", Phobos::AppIconPath.c_str());
		R->EAX(LoadImageA(instance, Phobos::AppIconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
	}else{
		R->EAX(LoadIconA(instance, (LPCSTR)93));
	}

	return 0x777C4A;
}

ASMJIT_PATCH(0x640B8D, LoadingScreen_DisableEmptySpawnPositions, 0x6)
{
	GET(bool const, esi, ESI);
	return(Phobos::UI::DisableEmptySpawnPositions || !esi) ? 0x640CE2: 0x640B93;
}

 //Allow size = 0 for map previews
ASMJIT_PATCH(0x641B41, LoadingScreen_SkipPreview, 0x8)
{
	enum { Continue = 0x0 , return_true = 0x641D59 };

	GET(RectangleStruct*, pRect, EAX);
	return (pRect->Width > 0 && pRect->Height > 0)
		? Continue : return_true;
}

ASMJIT_PATCH(0x641EE0, PreviewClass_ReadPreview, 0x6)
{
	GET(PreviewClass*, pThis, ECX);
	GET_STACK(const char*, lpMapFile, 0x4);

	CCFileClass file { lpMapFile };
	bool bResult = false;

	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		CCINIClass ini {};
		ini.ReadCCFile(&file, true);
		ini.CurrentSection = nullptr;
		ini.CurrentSectionName = nullptr;

		ScenarioClass::Instance->ReadStartPoints(&ini);

		bResult = pThis->ReadPreviewPack(ini);
	}

	R->EAX(bResult);
	return 0x64203D;
}

ASMJIT_PATCH(0x4A25E3, CreditsClass_GraphicLogic_Additionals , 0x8)
{
	const auto pPlayer = HouseClass::CurrentPlayer();
	if (HouseExtData::IsObserverPlayer(pPlayer) || pPlayer->Defeated)
		return 0x0;

	const auto pSide = SideClass::Array->get_or_default(pPlayer->SideIndex);

	if (!pSide)
		return 0x0;

	//GET(CreditClass*, pThis, EBP);

	const auto pSideExt = SideExtContainer::Instance.Find(pSide);
	RectangleStruct vRect = DSurface::Sidebar->Get_Rect();
	const auto pHouseExt = HouseExtContainer::Instance.Find(pPlayer);
	static fmt::basic_memory_buffer<wchar_t , 50> counter;
	static fmt::basic_memory_buffer<wchar_t , 50> ShowPower;
	static fmt::basic_memory_buffer<wchar_t , 10> Harv;

	if (Phobos::UI::BattlePointsSidebar_AlwaysShow || pHouseExt->AreBattlePointsEnabled())
	{
		counter.clear();
		ColorStruct clrToolTip = pSideExt->Sidebar_BattlePoints_Color.Get(Drawing::TooltipColor);

		int points = pHouseExt->BattlePoints;

		if (Phobos::UI::BattlePointsSidebar_Label_InvertPosition)
			fmt::format_to(std::back_inserter(counter), L"{}{}", points, Phobos::UI::BattlePointsSidebar_Label);
		else
			fmt::format_to(std::back_inserter(counter), L"{}{}", Phobos::UI::BattlePointsSidebar_Label, points);

		counter.push_back(L'\0');
		Point2D vPos = {
			DSurface::Sidebar->Get_Width() / 2 - 70 + pSideExt->Sidebar_BattlePoints_Offset.Get().X,
			2 + pSideExt->Sidebar_BattlePoints_Offset.Get().Y
		};

		auto const TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_BattlePoints_Align.Get()));

		DSurface::Sidebar->DSurfaceDrawText(counter.data(), &vRect, &vPos, clrToolTip.ToInit(), 0, TextFlags);
	}

	if (Phobos::UI::ShowHarvesterCounter && Phobos::Config::ShowHarvesterCounter)
	 {
		Harv.clear();

	 	const auto nActive = HouseExtData::ActiveHarvesterCount(pPlayer);
	 	const auto nTotal = HouseExtData::TotalHarvesterCount(pPlayer);
	 	const auto nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;

	 	const ColorStruct clrToolTip = nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow
	 		? Drawing::TooltipColor() : nPercentage > Phobos::UI::HarvesterCounter_ConditionRed
	 		? pSideExt->Sidebar_HarvesterCounter_Yellow : pSideExt->Sidebar_HarvesterCounter_Red;

		fmt::format_to(std::back_inserter(Harv), L"{}{}/{}", Phobos::UI::HarvesterLabel, nActive, nTotal);
		Harv.push_back(L'\0');

	 	Point2D vPos {
	 		DSurface::Sidebar->Get_Width() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
	 		2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
	 	};

	 	DSurface::Sidebar->DSurfaceDrawText(
		Harv.data()
			, &vRect, &vPos, clrToolTip.ToInit(), 0,
	 		TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	 }

	if (Phobos::UI::ShowPowerDelta && Phobos::Config::ShowPowerDelta && pPlayer->Buildings.Count)
	{
		ColorStruct clrToolTip { };
		ShowPower.clear();

		if (pPlayer->PowerBlackoutTimer.InProgress())
		{
			clrToolTip = pSideExt->Sidebar_PowerDelta_Grey.Get();
			ShowPower.append(Phobos::UI::PowerBlackoutLabel,  Phobos::UI::PowerBlackoutLabel + std::wcslen(Phobos::UI::PowerBlackoutLabel));
		}
		else
		{
			const int delta = pPlayer->PowerOutput - pPlayer->PowerDrain;
			const double percent = pPlayer->PowerOutput != 0
				? (double)pPlayer->PowerDrain / (double)pPlayer->PowerOutput : pPlayer->PowerDrain != 0
				? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow;

			clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
				? pSideExt->Sidebar_PowerDelta_Green.Get() : LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
				? pSideExt->Sidebar_PowerDelta_Yellow.Get() : pSideExt->Sidebar_PowerDelta_Red;

			fmt::format_to(std::back_inserter(ShowPower),L"{}{}", Phobos::UI::PowerLabel, delta);

		}
		ShowPower.push_back(L'\0');

		const auto TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_PowerDelta_Align.Get()));

		Point2D vPos {
			DSurface::Sidebar->Get_Width() / 2 - 70 + pSideExt->Sidebar_PowerDelta_Offset.Get().X,
			2 + pSideExt->Sidebar_PowerDelta_Offset.Get().Y
		};

		DSurface::Sidebar->DSurfaceDrawText(
		ShowPower.data()
		, &vRect, &vPos, clrToolTip.ToInit(), 0, TextFlags);
	}

	if (Phobos::UI::WeedsCounter_Show && Phobos::Config::ShowWeedsCounter)
	{
		counter.clear();
		ColorStruct clrToolTip = pSideExt->Sidebar_WeedsCounter_Color.Get(Drawing::TooltipColor());

		fmt::format_to(std::back_inserter(counter), L"{}", static_cast<int>(pPlayer->OwnedWeed.GetTotalAmount()));
		counter.push_back(L'\0');

		Point2D vPos = {
			DSurface::Sidebar->Get_Width() / 2 + 50 + pSideExt->Sidebar_WeedsCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_WeedsCounter_Offset.Get().Y
		};

		DSurface::Sidebar->DSurfaceDrawText(counter.data(), &vRect, &vPos, clrToolTip.ToInit(), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	}
	return 0;
}

ASMJIT_PATCH(0x715A4D, Replace_XXICON_With_New, 0x7)         //TechnoTypeClass::ReadINI
{
	char pFilename[0x20];
	strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
	_strlwr_s(pFilename);

	if (_stricmp(pFilename, GameStrings::XXICON_SHP())
		&& strstr(pFilename, GameStrings::dot_SHP())) {
		if (const auto pFile = FakeFileLoader::_Retrieve(RulesExtData::Instance()->MissingCameo.data(), false)) {
			R->EAX(pFile);
			return R->Origin() + 0xC;
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x6CE8AA, Replace_XXICON_With_New, 0x7)   //SWTypeClass::Load
ASMJIT_PATCH_AGAIN(0x6CEE31, Replace_XXICON_With_New, 0x7)   //SWTypeClass::ReadINI
ASMJIT_PATCH_AGAIN(0x716D13, Replace_XXICON_With_New, 0x7)   //TechnoTypeClass::Load


ASMJIT_PATCH(0x6A8463, StripClass_OperatorLessThan_CameoPriority, 5)
{
	enum {
		rTrue = 0x6A8692,
		rFalse = 0x6A86A0,
		rTrue_ = 0x6A8477,
		rFalse_ = 0x6A8468
	};

	GET_STACK(TechnoTypeClass* const, pLeft, STACK_OFFS(0x1C, 0x8));
	GET_STACK(TechnoTypeClass* const, pRight, STACK_OFFS(0x1C, 0x4));
	GET_STACK(int const, idxLeft, STACK_OFFS(0x1C, -0x8));
	GET_STACK(int const, idxRight, STACK_OFFS(0x1C, -0x10));
	GET_STACK(AbstractType const, rttiLeft, STACK_OFFS(0x1C, -0x4));
	GET_STACK(AbstractType const, rttiRight, STACK_OFFS(0x1C, -0xC));

	const auto pLeftTechnoExt = TechnoTypeExtContainer::Instance.TryFind(pLeft);
	const auto pRightTechnoExt = TechnoTypeExtContainer::Instance.TryFind(pRight);

	const auto pLeftSWExt = (rttiLeft == AbstractType::Special || rttiLeft == AbstractType::Super || rttiLeft == AbstractType::SuperWeaponType)
		? SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->get_or_default(idxLeft)) : nullptr;
	const auto pRightSWExt = (rttiRight == AbstractType::Special || rttiRight == AbstractType::Super || rttiRight == AbstractType::SuperWeaponType)
		? SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->get_or_default(idxRight)) : nullptr;

	if ((pLeftTechnoExt || pLeftSWExt) && (pRightTechnoExt || pRightSWExt))
	{
		const auto leftPriority = pLeftTechnoExt ? pLeftTechnoExt->CameoPriority : pLeftSWExt->CameoPriority;
		const auto rightPriority = pRightTechnoExt ? pRightTechnoExt->CameoPriority : pRightSWExt->CameoPriority;

		if (leftPriority > rightPriority)
			return rTrue;
		else if (rightPriority > leftPriority)
			return rFalse;
	}

	// Restore overridden instructions
	GET(AbstractType, rtti1, ESI);
	return rtti1 == AbstractType::Special ? rTrue_ : rFalse_;
}

namespace BriefingTemp
{
	bool ShowBriefing = false;
}
FORCEDINLINE void ShowBriefing()
{
	if (BriefingTemp::ShowBriefing)
	{
		// Show briefing dialog.
		Game::SpecialDialog = 9;
		Game::ShowSpecialDialog();
		BriefingTemp::ShowBriefing = false;

		// Play scenario theme.
		int theme = ScenarioClass::Instance->ThemeIndex;

		if (theme == -1)
			ThemeClass::Instance->Stop(true);
		else
			ThemeClass::Instance->Queue(theme);
	}
}

// Check if briefing dialog should be played before starting scenario.
ASMJIT_PATCH(0x683E41, ScenarioClass_Start_ShowBriefing, 0x6)
{
	GET_STACK(bool, showBriefing, STACK_OFFSET(0xFC, -0xE9));

	// Don't show briefing dialog for non-campaign games or on restarts etc.
	if (!ScenarioExtData::Instance()->ShowBriefing || !showBriefing || !SessionClass::Instance->IsCampaign())
		return 0;

	BriefingTemp::ShowBriefing = true;

	int theme = ScenarioExtData::Instance()->BriefingTheme;

	if (theme == -1)
	{
		SideClass* pSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex);

		if (const auto pSideExt = SideExtContainer::Instance.Find(pSide))
			theme = pSideExt->BriefingTheme;
	}

	if (theme != -1)
		ThemeClass::Instance->Queue(theme);

	// Skip over playing scenario theme.
	return 0x683E6B;
}

// Skip redrawing the screen if we're gonna show the briefing screen immediately after loading screen finishes on initially launched mission.
ASMJIT_PATCH(0x683F66, PauseGame_ShowBriefing, 0x5) {
	return BriefingTemp::ShowBriefing ? 0x683FAA : 0;
}

// Show the briefing dialog on starting a new scenario after clearing another.
ASMJIT_PATCH(0x55D14F, AuxLoop_ShowBriefing, 0x5)
{
	// Restore overridden instructions.
	SessionClass::Instance->Resume();
	ShowBriefing();
	return 0x55D159;
}

// Skip redrawing the screen if we're gonna show the briefing screen immediately after loading screen finishes on succeeding missions.
ASMJIT_PATCH(0x685D95, DoWin_ShowBriefing, 0x5) {
	return BriefingTemp::ShowBriefing ? 0x685D9F : 0;
}

// Set briefing dialog resume button text.
ASMJIT_PATCH(0x65F764, BriefingDialog_ShowBriefing, 0x5)
{
	if (BriefingTemp::ShowBriefing) {
		SendMessageA(GetDlgItem(R->ESI<HWND>(), 1059), 1202, 0, reinterpret_cast<LPARAM>(Phobos::UI::ShowBriefingResumeButtonLabel));
	}

	return 0;
}

// Set briefing dialog resume button status bar label.
ASMJIT_PATCH(0x604985, GetDialogUIStatusLabels_ShowBriefing, 0x5)
{
	if (BriefingTemp::ShowBriefing) {
		R->EAX(Phobos::UI::ShowBriefingResumeButtonStatusLabel);
		return 0x60498A;
	}

	return 0;
}

ASMJIT_PATCH(0x6A84DB, StripClass_OperatorLessThan_SortCameoByNameSW, 0x5)
{
	enum { rTrue = 0x6A8692, rFalse = 0x6A86A0 };

	GET(SuperWeaponTypeClass*, pLeftSW, EAX);
	GET(SuperWeaponTypeClass*, pRightSW, ECX);

	if (RulesExtData::Instance()->SortCameoByName)
	{
		const int result = strcmp(pLeftSW->Name, pRightSW->Name);

		if (result < 0)
			return rTrue;
		else if (result > 0)
			return rFalse;
	}

	return wcscmp(pLeftSW->UIName, pRightSW->UIName) <= 0 ? rTrue : rFalse;
}

ASMJIT_PATCH(0x6A86ED, StripClass_OperatorLessThan_SortCameoByNameTechno, 0x5)
{
	enum { rTrue = 0x6A8692, rFalse = 0x6A86A0 };

	GET(TechnoTypeClass*, pLeft, EDI);
	GET(TechnoTypeClass*, pRight, EBP);

	if (RulesExtData::Instance()->SortCameoByName)
	{
		const int result = strcmp(pLeft->Name, pRight->Name);

		if (result < 0)
			return rTrue;
		else if (result > 0)
			return rFalse;
	}

	return wcscmp(pLeft->UIName, pRight->UIName) <= 0 ? rTrue : rFalse;
}

/*
enum VanillaDialogs : uint16_t
{
	SingleplayerGameOptionsDialog = 181,
	MultiplayerGameOptionsDialog = 3002
};
*/
// For now this can only contain copies of the original dialogs with same IDs
enum SpawnerCustomDialogs : uint16_t
{
	MultiplayerGameOptionsDialog = 3002, // added a save button

	First = 3002,
	Last = 3002
};


class Dialogs
{
public:

};

/*
	So here's a first example on how to modify the game's dialogs! :3
	To port a dialog from YR to the DLL, add a new .rc file and don't bother editing it
	using MSVC's inbuilt tools.
	Open gamemd.exe in ResHacker, open the selected dialog and copy the whole resource script to
	the new .rc file.
	Don't edit the dialog ID. That would be adding a new dialog and that's a pain to get working
	apparently (I don't know how to do that yet).
	Add #include <windows.h> to the first line of the script.
	If STYLE doesn't list WS_CAPTION, remove the CAPTION "" line.
	Adding new controls is best done in ResHacker, as MSVC wants to add too much stupid
	stuff to things (resource.h and crap like that).
	Also remember to set the control properties like the game originals to avoid problems.
	Finally, be aware that MSVC will automatically add WS_VISIBLE to the controls.
	I don't know why, but I know it sucks, as you'll see when you use the RMG with this version.
	I've yet to find a way to suppress this behavior.
	If you add buttons, they'll look like shit until you register them in the dialog function.
	Adding new dialogs requires many funny hacks I'll try to figure later.
	To make our systems consitent:
	New control IDs within a dialog start with 5000.
	Each dialog code should get its own cpp file to avoid major confusion.
	This central source file contains an addition to YR's "FetchResource"
	that's able to find resources within this DLL as well.
	It's important for it to be just an addition so other DLLs can do this as well!
	Happy GUI modding!
	-pd
*/

//4A3B4B, 9 - NOTE: This overrides a call, but it's absolute, so don't worry.
ASMJIT_PATCH(0x4A3B4B, FetchResource, 0x9)
{
	HMODULE hModule = static_cast<HMODULE>(Phobos::hInstance); //hModule and hInstance are technically the same...
	GET(LPCTSTR, lpName, ECX);
	GET(LPCTSTR, lpType, EDX);

	if (HRSRC hResInfo = FindResource(hModule, lpName, lpType))
	{
		if (HGLOBAL hResData = LoadResource(hModule, hResInfo))
		{
			LockResource(hResData);
			R->EAX(hResData);

			return 0x4A3B73; //Resource locked and loaded (omg what a pun), return!
		}
	}
	return 0; //Nothing was found, try the game's own resources.
}

ASMJIT_PATCH(0x609299, UI_IsStaticAndOrOwnerDraw_MultiplayerGameOptionsDialog, 0x5)
{
	enum { RetFalse = 0x609664, RetTrue = 0x609693 };

	GET(int, dlgCtrlID, EAX);
	return (dlgCtrlID == 1314 || dlgCtrlID == 1313 || dlgCtrlID == 1311) ? RetTrue : RetFalse;
}

#pragma region Colors

ASMJIT_PATCH(0x69B97D, Game_ProcessRandomPlayers_ObserverColor, 7)
{
	GET(NodeNameType* const, pStartingSpot, ESI);

	// observer uses last color, beyond the actual colors
	pStartingSpot->Color = Phobos::Config::colorCount;

	return 0x69B984;
}

ASMJIT_PATCH(0x69B949, Game_ProcessRandomPlayers_ColorsA, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Phobos::Config::colorCount - 1));
	return 0x69B95E;
}

ASMJIT_PATCH(0x69BA13, Game_ProcessRandomPlayers_ColorsB, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Phobos::Config::colorCount - 1));
	return 0x69BA28;
}

ASMJIT_PATCH(0x69B69B, GameModeClass_PickRandomColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Phobos::Config::colorCount - 1));
	return 0x69B6AF;
}

ASMJIT_PATCH(0x69B7FF, Session_SetColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Phobos::Config::colorCount - 1));
	return 0x69B813;
}

ASMJIT_PATCH(0x60FAD7, Ownerdraw_PostProcessColors, 0xA)
{
	// copy original instruction
	*reinterpret_cast<int*>(0xAC1B90) = 0x443716;

	// update colors
	*reinterpret_cast<int*>(0xAC18A4) = Phobos::UI::uiColorText;
	*reinterpret_cast<int*>(0xAC184C) = Phobos::UI::uiColorCaret;
	*reinterpret_cast<int*>(0xAC4604) = Phobos::UI::uiColorSelection;
	*reinterpret_cast<int*>(0xAC1B98) = Phobos::UI::uiColorBorder1;
	*reinterpret_cast<int*>(0xAC1B94) = Phobos::UI::uiColorBorder2;
	*reinterpret_cast<int*>(0xAC1AF8) = Phobos::UI::uiColorDisabledObserver;
	*reinterpret_cast<int*>(0xAC1CB0) = Phobos::UI::uiColorTextObserver;
	*reinterpret_cast<int*>(0xAC4880) = Phobos::UI::uiColorSelectionObserver;
	*reinterpret_cast<int*>(0xAC1CB4) = Phobos::UI::uiColorDisabled;

	// skip initialization
	//CommonDialogStuff_Color_Shifts_Set_PCXes_Loaded
	bool inited = *reinterpret_cast<bool*>(0xAC48D4);
	return inited ? 0x60FB5D : 0x60FAE3;
}

ASMJIT_PATCH(0x612DA9, Handle_Button_Messages_Color, 6)
{
	R->EDI(Phobos::UI::uiColorTextButton);
	return 0x612DAF;
}

ASMJIT_PATCH(0x613072, Handle_Button_Messages_DisabledColor, 7)
{
	R->EDI(Phobos::UI::uiColorDisabledButton);
	return 0x613138;
}

ASMJIT_PATCH(0x61664C, Handle_Checkbox_Messages_Color, 5)
{
	R->EAX(Phobos::UI::uiColorTextCheckbox);
	return 0x616651;
}

ASMJIT_PATCH(0x616655, Handle_Checkbox_Messages_Disabled, 5)
{
	R->EAX(Phobos::UI::uiColorDisabledCheckbox);
	return 0x61665A;
}

ASMJIT_PATCH(0x616AF0, Handle_RadioButton_Messages_Color, 6)
{
	R->ECX(Phobos::UI::uiColorTextRadio);
	return 0x616AF6;
}

ASMJIT_PATCH(0x615DF7, Handle_Static_Messages_Color, 6)
{
	R->ECX(Phobos::UI::uiColorTextLabel);
	return 0x615DFD;
}

ASMJIT_PATCH(0x615AB7, Handle_Static_Messages_Disabled, 6)
{
	R->ECX(Phobos::UI::uiColorDisabledLabel);
	return 0x615ABD;
}

ASMJIT_PATCH(0x619A4F, Handle_Listbox_Messages_Color, 6)
{
	R->ESI(Phobos::UI::uiColorTextList);
	return 0x619A55;
}

ASMJIT_PATCH(0x6198D3, Handle_Listbox_Messages_DisabledA, 6)
{
	R->EBX(Phobos::UI::uiColorDisabledList);
	return 0x6198D9;
}

ASMJIT_PATCH(0x619A5F, Handle_Listbox_Messages_DisabledB, 6)
{
	R->ESI(Phobos::UI::uiColorDisabledList);
	return 0x619A65;
}

ASMJIT_PATCH(0x619270, Handle_Listbox_Messages_SelectionA, 5)
{
	R->EAX(Phobos::UI::uiColorSelectionList);
	return 0x619275;
}

ASMJIT_PATCH(0x619288, Handle_Listbox_Messages_SelectionB, 6)
{
	R->DL(BYTE(Phobos::UI::uiColorSelectionList >> 16));
	return 0x61928E;
}

ASMJIT_PATCH(0x617A2B, Handle_Combobox_Messages_Color, 6)
{
	R->EBX(Phobos::UI::uiColorTextCombobox);
	return 0x617A31;
}

ASMJIT_PATCH(0x617A57, Handle_Combobox_Messages_Disabled, 6)
{
	R->EBX(Phobos::UI::uiColorDisabledCombobox);
	return 0x617A5D;
}

ASMJIT_PATCH(0x60DDA6, Handle_Combobox_Dropdown_Messages_SelectionA, 5)
{
	R->EAX(Phobos::UI::uiColorSelectionCombobox);
	return 0x60DDAB;
}

ASMJIT_PATCH(0x60DDB6, Handle_Combobox_Dropdown_Messages_SelectionB, 6)
{
	R->DL(BYTE(Phobos::UI::uiColorSelectionCombobox >> 16));
	return 0x60DDBC;
}

ASMJIT_PATCH(0x61E2A5, Handle_Slider_Messages_Color, 5)
{
	R->EAX(Phobos::UI::uiColorTextSlider);
	return 0x61E2AA;
}

ASMJIT_PATCH(0x61E2B1, Handle_Slider_Messages_Disabled, 5)
{
	R->EAX(Phobos::UI::uiColorDisabledSlider);
	return 0x61E2B6;
}

ASMJIT_PATCH(0x61E8A0, Handle_GroupBox_Messages_Color, 6)
{
	R->ECX(Phobos::UI::uiColorTextGroupbox);
	return 0x61E8A6;
}

ASMJIT_PATCH(0x614FF2, Handle_NewEdit_Messages_Color, 6)
{
	R->EDX(Phobos::UI::uiColorTextEdit);
	return 0x614FF8;
}

// reset the colors
ASMJIT_PATCH(0x4E43C0, Game_InitDropdownColors, 5)
{
	// mark all colors as unused (+1 for the  observer)
	for (auto i = 0; i < Phobos::Config::colorCount + 1; ++i)
	{
		Phobos::UI::Colors[i].selectedIndex = -1;
	}

	return 0;
}
#include <Misc/Spawner/Main.h>

ASMJIT_PATCH(0x69A310, SessionClass_GetPlayerColorScheme, 7)
{
	GET_STACK(PlayerColorSlot, idx, 0x4);
	GET_STACK(DWORD, caller, 0x0);

	int ret = 0;

	// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
	// if spawner feeds us a number, it will be used to look up color scheme directly
	// Original Author : Morton

	if (SpawnerMain::Configs::Enabled && Phobos::UI::UnlimitedColor && idx != PlayerColorSlot::Random)
	{
		ret = Math::abs((int)idx) << 1;
	}
	else
	{

		{

			// get the slot
			ColorData* slot = nullptr;
			if (idx == PlayerColorSlot::Random || idx == Phobos::Config::colorCount)
			{
				// observer color
				slot = &Phobos::UI::Colors[0];
			}
			else if (idx < Phobos::Config::colorCount)
			{
				// house color
				slot = &Phobos::UI::Colors[idx + 1];
			}

			// retrieve the color scheme index

			if (slot)
			{
				if (slot->colorSchemeIndex == -1)
				{
					slot->colorSchemeIndex = ColorScheme::FindIndex(slot->colorScheme);

					if (slot->colorSchemeIndex == -1)
					{
						Debug::LogInfo("Color scheme \"{}\" not found.", slot->colorScheme);
						slot->colorSchemeIndex = 4;
					}
				}

				ret = slot->colorSchemeIndex;
			}
		}
	}

	ret += 1;
	const int ColorShemeArrayCount = ColorScheme::Array->Count;

	if ((size_t)ret >= (size_t)ColorShemeArrayCount)
		Debug::FatalErrorAndExit("Address[%x] Trying To get Player Color[idx %d , %d(%d)] that more than ColorScheme Array Count [%d]!", caller, idx, ret, ret - 1, ColorShemeArrayCount);

	R->EAX(ret);
	return 0x69A334;
}

// return the tool tip describing this color
ASMJIT_PATCH(0x4E42A0, GameSetup_GetColorTooltip, 5)
{
	GET(int const, idxColor, ECX);

	if (idxColor == -2)
	{
		return 0x4E42A5;// random
	}
	else if (idxColor > Phobos::Config::colorCount)
	{
		return 0x4E43B7;
	}

	R->EAX(Phobos::UI::Colors[(idxColor + 1) % (Phobos::Config::colorCount + 1)].sttToolTipSublineText);
	return 0x4E43B9;
}

// handle adding colors to combo box
ASMJIT_PATCH(0x4E46BB, hWnd_PopulateWithColors, 7)
{
	GET(HWND const, hWnd, ESI);
	GET_STACK(int const, idxPlayer, 0x14);

	// add all colors
	auto curSel = 0;
	for (auto i = 0; i < Phobos::Config::colorCount; ++i)
	{
		auto const& Color = Phobos::UI::Colors[i + 1];
		auto const isCurrent = Color.selectedIndex == idxPlayer;

		if (isCurrent || Color.selectedIndex == -1)
		{
			int idx = Imports::SendMessageA.invoke()(hWnd, WW_CB_ADDITEM, 0, 0x822B78);
			Imports::SendMessageA.invoke()(hWnd, WW_SETCOLOR, idx, Color.colorRGB);
			Imports::SendMessageA.invoke()(hWnd, CB_SETITEMDATA, idx, i);

			if (isCurrent)
			{
				curSel = idx;
			}
		}
	}

	Imports::SendMessageA.invoke()(hWnd, CB_SETCURSEL, curSel, 0);
	Imports::SendMessageA.invoke()(hWnd, 0x4F1, 0, 0);

	return 0x4E4749;
}

// update the color in the combo drop-down lists
ASMJIT_PATCH(0x4E4A41, hWnd_SetPlayerColor_A, 7)
{
	GET(int const, idxPlayer, EAX);

	for (auto i = 0; i < Phobos::Config::colorCount; ++i)
	{
		auto& Color = Phobos::UI::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4A6D;
}

ASMJIT_PATCH(0x4E4B47, hWnd_SetPlayerColor_B, 7)
{
	GET(int const, idxColor, EBP);
	GET(int const, idxPlayer, ESI);

	Phobos::UI::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4B4E;
}

ASMJIT_PATCH(0x4E4556, hWnd_GetSlotColorIndex, 7)
{
	GET(int const, idxPlayer, ECX);

	auto ret = -1;
	for (auto i = 0; i < Phobos::Config::colorCount; ++i)
	{
		auto const& Color = Phobos::UI::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			ret = i + 1;
			break;
		}
	}

	R->EAX(ret);
	return 0x4E4570;
}

ASMJIT_PATCH(0x4E4580, hWnd_IsAvailableColor, 5)
{
	GET(int const, idxColor, ECX);
	R->AL(Phobos::UI::Colors[idxColor + 1].selectedIndex == -1);
	return 0x4E4592;
}

ASMJIT_PATCH(0x4E4C9D, hWnd_UpdatePlayerColors_A, 7)
{
	GET(int const, idxPlayer, EAX);

	// check players and reset used color for this player
	for (auto i = 0; i < Phobos::Config::colorCount; ++i)
	{
		auto& Color = Phobos::UI::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4CC9;
}

ASMJIT_PATCH(0x4E4D67, hWnd_UpdatePlayerColors_B, 7)
{
	GET(int const, idxColor, EAX);
	GET(int const, idxPlayer, ESI);

	// reserve the color for a player. skip the observer
	Phobos::UI::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4D6E;
}


#pragma endregion