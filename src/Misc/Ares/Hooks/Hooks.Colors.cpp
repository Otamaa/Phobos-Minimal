#include "Header.h"

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x69B97D, Game_ProcessRandomPlayers_ObserverColor, 7)
{
	GET(NodeNameType* const, pStartingSpot, ESI);

	// observer uses last color, beyond the actual colors
	pStartingSpot->Color = AresGlobalData::colorCount;

	return 0x69B984;
}

ASMJIT_PATCH(0x69B949, Game_ProcessRandomPlayers_ColorsA, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B95E;
}

ASMJIT_PATCH(0x69BA13, Game_ProcessRandomPlayers_ColorsB, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69BA28;
}

ASMJIT_PATCH(0x69B69B, GameModeClass_PickRandomColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B6AF;
}

ASMJIT_PATCH(0x69B7FF, Session_SetColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B813;
}

ASMJIT_PATCH(0x60FAD7, Ownerdraw_PostProcessColors, 0xA)
{
	// copy original instruction
	*reinterpret_cast<int*>(0xAC1B90) = 0x443716;

	// update colors
	*reinterpret_cast<int*>(0xAC18A4) = AresGlobalData::uiColorText;
	*reinterpret_cast<int*>(0xAC184C) = AresGlobalData::uiColorCaret;
	*reinterpret_cast<int*>(0xAC4604) = AresGlobalData::uiColorSelection;
	*reinterpret_cast<int*>(0xAC1B98) = AresGlobalData::uiColorBorder1;
	*reinterpret_cast<int*>(0xAC1B94) = AresGlobalData::uiColorBorder2;
	*reinterpret_cast<int*>(0xAC1AF8) = AresGlobalData::uiColorDisabledObserver;
	*reinterpret_cast<int*>(0xAC1CB0) = AresGlobalData::uiColorTextObserver;
	*reinterpret_cast<int*>(0xAC4880) = AresGlobalData::uiColorSelectionObserver;
	*reinterpret_cast<int*>(0xAC1CB4) = AresGlobalData::uiColorDisabled;

	// skip initialization
	//CommonDialogStuff_Color_Shifts_Set_PCXes_Loaded
	bool inited = *reinterpret_cast<bool*>(0xAC48D4);
	return inited ? 0x60FB5D : 0x60FAE3;
}

ASMJIT_PATCH(0x612DA9, Handle_Button_Messages_Color, 6)
{
	R->EDI(AresGlobalData::uiColorTextButton);
	return 0x612DAF;
}

ASMJIT_PATCH(0x613072, Handle_Button_Messages_DisabledColor, 7)
{
	R->EDI(AresGlobalData::uiColorDisabledButton);
	return 0x613138;
}

ASMJIT_PATCH(0x61664C, Handle_Checkbox_Messages_Color, 5)
{
	R->EAX(AresGlobalData::uiColorTextCheckbox);
	return 0x616651;
}

ASMJIT_PATCH(0x616655, Handle_Checkbox_Messages_Disabled, 5)
{
	R->EAX(AresGlobalData::uiColorDisabledCheckbox);
	return 0x61665A;
}

ASMJIT_PATCH(0x616AF0, Handle_RadioButton_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextRadio);
	return 0x616AF6;
}

ASMJIT_PATCH(0x615DF7, Handle_Static_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextLabel);
	return 0x615DFD;
}

ASMJIT_PATCH(0x615AB7, Handle_Static_Messages_Disabled, 6)
{
	R->ECX(AresGlobalData::uiColorDisabledLabel);
	return 0x615ABD;
}

ASMJIT_PATCH(0x619A4F, Handle_Listbox_Messages_Color, 6)
{
	R->ESI(AresGlobalData::uiColorTextList);
	return 0x619A55;
}

ASMJIT_PATCH(0x6198D3, Handle_Listbox_Messages_DisabledA, 6)
{
	R->EBX(AresGlobalData::uiColorDisabledList);
	return 0x6198D9;
}

ASMJIT_PATCH(0x619A5F, Handle_Listbox_Messages_DisabledB, 6)
{
	R->ESI(AresGlobalData::uiColorDisabledList);
	return 0x619A65;
}

ASMJIT_PATCH(0x619270, Handle_Listbox_Messages_SelectionA, 5)
{
	R->EAX(AresGlobalData::uiColorSelectionList);
	return 0x619275;
}

ASMJIT_PATCH(0x619288, Handle_Listbox_Messages_SelectionB, 6)
{
	R->DL(BYTE(AresGlobalData::uiColorSelectionList >> 16));
	return 0x61928E;
}

ASMJIT_PATCH(0x617A2B, Handle_Combobox_Messages_Color, 6)
{
	R->EBX(AresGlobalData::uiColorTextCombobox);
	return 0x617A31;
}

ASMJIT_PATCH(0x617A57, Handle_Combobox_Messages_Disabled, 6)
{
	R->EBX(AresGlobalData::uiColorDisabledCombobox);
	return 0x617A5D;
}

ASMJIT_PATCH(0x60DDA6, Handle_Combobox_Dropdown_Messages_SelectionA, 5)
{
	R->EAX(AresGlobalData::uiColorSelectionCombobox);
	return 0x60DDAB;
}

ASMJIT_PATCH(0x60DDB6, Handle_Combobox_Dropdown_Messages_SelectionB, 6)
{
	R->DL(BYTE(AresGlobalData::uiColorSelectionCombobox >> 16));
	return 0x60DDBC;
}

ASMJIT_PATCH(0x61E2A5, Handle_Slider_Messages_Color, 5)
{
	R->EAX(AresGlobalData::uiColorTextSlider);
	return 0x61E2AA;
}

ASMJIT_PATCH(0x61E2B1, Handle_Slider_Messages_Disabled, 5)
{
	R->EAX(AresGlobalData::uiColorDisabledSlider);
	return 0x61E2B6;
}

ASMJIT_PATCH(0x61E8A0, Handle_GroupBox_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextGroupbox);
	return 0x61E8A6;
}

ASMJIT_PATCH(0x614FF2, Handle_NewEdit_Messages_Color, 6)
{
	R->EDX(AresGlobalData::uiColorTextEdit);
	return 0x614FF8;
}

// reset the colors
ASMJIT_PATCH(0x4E43C0, Game_InitDropdownColors, 5)
{
	// mark all colors as unused (+1 for the  observer)
	for (auto i = 0; i < AresGlobalData::colorCount + 1; ++i)
	{
		AresGlobalData::Colors[i].selectedIndex = -1;
	}

	return 0;
}
#include <Misc/Spawner/Main.h>

ASMJIT_PATCH(0x69A310, SessionClass_GetPlayerColorScheme, 7)
{
	GET_STACK(int const, idx, 0x4);
	GET_STACK(DWORD, caller, 0x0);

	int ret = 0;

	// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
	// if spawner feeds us a number, it will be used to look up color scheme directly
	// Original Author : Morton

	if (SpawnerMain::Configs::Enabled && Phobos::UI::UnlimitedColor && idx != -2) {
		ret = Math::abs(idx) << 1;
	} else {

		{

			// get the slot
			AresGlobalData::ColorData* slot = nullptr;
			if (idx == -2 || idx == AresGlobalData::colorCount)
			{
				// observer color
				slot = &AresGlobalData::Colors[0];
			}
			else if (idx < AresGlobalData::colorCount)
			{
				// house color
				slot = &AresGlobalData::Colors[idx + 1];
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
	else if (idxColor > AresGlobalData::colorCount)
	{
		return 0x4E43B7;
	}

	R->EAX(AresGlobalData::Colors[(idxColor + 1) % (AresGlobalData::colorCount + 1)].sttToolTipSublineText);
	return 0x4E43B9;
}

// handle adding colors to combo box
ASMJIT_PATCH(0x4E46BB, hWnd_PopulateWithColors, 7)
{
	GET(HWND const, hWnd, ESI);
	GET_STACK(int const, idxPlayer, 0x14);

	// add all colors
	auto curSel = 0;
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto const& Color = AresGlobalData::Colors[i + 1];
		auto const isCurrent = Color.selectedIndex == idxPlayer;

		if (isCurrent || Color.selectedIndex == -1)
		{
			int idx = SendMessageA(hWnd, WW_CB_ADDITEM, 0, 0x822B78);
			SendMessageA(hWnd, WW_SETCOLOR, idx, Color.colorRGB);
			SendMessageA(hWnd, CB_SETITEMDATA, idx, i);

			if (isCurrent)
			{
				curSel = idx;
			}
		}
	}

	SendMessageA(hWnd, CB_SETCURSEL, curSel, 0);
	SendMessageA(hWnd, 0x4F1, 0, 0);

	return 0x4E4749;
}

// update the color in the combo drop-down lists
ASMJIT_PATCH(0x4E4A41, hWnd_SetPlayerColor_A, 7)
{
	GET(int const, idxPlayer, EAX);

	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto& Color = AresGlobalData::Colors[i + 1];
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

	AresGlobalData::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4B4E;
}

ASMJIT_PATCH(0x4E4556, hWnd_GetSlotColorIndex, 7)
{
	GET(int const, idxPlayer, ECX);

	auto ret = -1;
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto const& Color = AresGlobalData::Colors[i + 1];
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
	R->AL(AresGlobalData::Colors[idxColor + 1].selectedIndex == -1);
	return 0x4E4592;
}

ASMJIT_PATCH(0x4E4C9D, hWnd_UpdatePlayerColors_A, 7)
{
	GET(int const, idxPlayer, EAX);

	// check players and reset used color for this player
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto& Color = AresGlobalData::Colors[i + 1];
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
	AresGlobalData::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4D6E;
}
