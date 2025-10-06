/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"
#include <Utilities/Macro.h>
#include <HouseClass.h>

#pragma region HouseClass is Observer

// Skip set spawn point for all observers
ASMJIT_PATCH(0x5D69BF, MPGameMode_AssignStartingPositions_SetObserverSpawn, 0x5)
{
	return SpawnerMain::Configs::Enabled
		? 0x5D69D1
		: 0;
}

// Skip score field for all observers
ASMJIT_PATCH(0x5C98E5, MultiplayerScore_5C98A0_SkipObserverScore, 0x6)
{
	enum { Skip = 0x5C9A7E, Show = 0x5C98F1 };

	GET(HouseClass*, pHouse, EDI);
	return pHouse->IsInitiallyObserver()
		? Skip
		: Show;
}

// Use correct colors in diplomacy menu for all observers
ASMJIT_PATCH(0x6583B2, RadarClass__658330_SetObserverColorScheme, 0x5)
{
	if (!SpawnerMain::Configs::Enabled)
		return 0;

	GET(HouseClass*, pHouse, EBX);
	if (pHouse->IsHumanPlayer && pHouse->IsInitiallyObserver())
		return 0x658397;

	return 0;
}

// Use correct flag icon in diplomacy menu for all observers
ASMJIT_PATCH(0x658473, RadarClass_658330_SetObserverFlag, 0x5)
{
	if (!SpawnerMain::Configs::Enabled)
		return 0;

	GET(HouseClass*, pHouse, EBX);
	if (pHouse->IsHumanPlayer 
			&& pHouse->Defeated 
			&& pHouse->IsInitiallyObserver()
		) {
		R->ECX(-3);
	}

	return 0x658485;
}

#pragma endregion HouseClass is Observer

#pragma region Curent player is Observer

ASMJIT_PATCH(0x5533E0, LoadProgressMgr_Draw_SetBackground, 0x5)
{
	return Game::ObserverMode
		? 0x5533EF
		: 0;
}

ASMJIT_PATCH(0x5539E4, LoadProgressMgr_Draw_LoadBriefing, 0x5)
{
	return Game::ObserverMode
		? 0x5539F3
		: 0;
}

ASMJIT_PATCH(0x5536A0, LoadProgressMgr_Draw_CountryName, 0x5)
{
	return Game::ObserverMode
		? 0x5536AF
		: 0;
}

#pragma endregion Curent player is Observer

#pragma region Observer in Skirmish

// Use correct flag icon for observer on loading screen in skirmish
ASMJIT_PATCH(0x6439F4, ProgressScreenClass_643720, 0x6)
{
	enum { AllowObserver = 0x643A04, NotAllowObserver = 0x643A18 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

// Use correct loading screen colors for observer in skirmish
ASMJIT_PATCH(0x642B60, ProgressScreenClass_LoadTextColor3, 0x5)
{
	enum { AllowObserver = 0x642B6F, NotAllowObserver = 0x642B8B };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

// Use correct observer player color on loading screen in skirmish
ASMJIT_PATCH(0x642BC3, ProgressScreenClass_GetPlayerColorSchemes, 0x5)
{
	enum { AllowObserver = 0x642BCD, NotAllowObserver = 0x642BF3 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

// Enable observer sidebar in skirmish
ASMJIT_PATCH(0x6A557A, SidebarClass_InitIO, 0x5)
{
	enum { AllowObserver = 0x6A558D, NotAllowObserver = 0x6A5830 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

#pragma endregion Curent player is Observer

#pragma region Show house on Observer sidebar
bool OPTIONALINLINE ShowHouseOnObserverSidebar(HouseClass* pHouse)
{

	if (pHouse->Type->MultiplayPassive)
		return false;

	const bool bShowAI = (SpawnerMain::Configs::Enabled && SpawnerMain::GetGameConfigs()->Observer_ShowAIOnSidebar);

	if (!bShowAI && !pHouse->IsHumanPlayer)
		return false;

	if (pHouse->IsInitiallyObserver())
		return false;

	return true;
}

ASMJIT_PATCH(0x6A55AB, SidebarClass_InitIO_ShowHouseOnObserverSidebar1, 0xA)
{
	enum { Draw = 0x6A55C8, DontDraw = 0x6A55CF };
	GET(HouseClass*, pHouse, EAX);

	return ShowHouseOnObserverSidebar(pHouse)
		? Draw
		: DontDraw;
}

ASMJIT_PATCH(0x6A57E2, SidebarClass_InitIO_ShowHouseOnObserverSidebar2, 0xA)
{
	enum { Draw = 0x6A57FF, DontDraw = 0x6A580E };
	GET(HouseClass*, pHouse, EAX);

	return ShowHouseOnObserverSidebar(pHouse)
		? Draw
		: DontDraw;
}

// Don't set LightGrey ColorScheme for failed observers
DEFINE_JUMP(LJMP, 0x6A91F7, 0x6A9212); // SidebarClass_StripClass_AI

#pragma endregion Show house on Observer sidebar

// Set observer mode after game load
ASMJIT_PATCH(0x67E720, LoadGame_After, 0x5)
{
	if (!SpawnerMain::Configs::Enabled || SessionClass::IsCampaign())
		return 0;

	HouseClass* pCurrentPlayer = HouseClass::CurrentPlayer;
	if (pCurrentPlayer->Defeated)
	{
		if (pCurrentPlayer->IsInitiallyObserver())
			HouseClass::Observer = pCurrentPlayer;

		pCurrentPlayer->AcceptDefeat();
	}

	return 0;
}
