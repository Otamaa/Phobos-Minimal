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
#include <Phobos.h>

#include <Helpers/Macro.h>
#include <HouseClass.h>

#include <Utilities/Debug.h>
#include <EventClass.h>

bool RageQuit = false;

bool IsQuitRequest(const MSG& msg) noexcept
{
	switch (msg.message)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		return true;

	case WM_SYSCOMMAND:
		// BUGFIX: the low 4 bits of a WM_SYSCOMMAND wParam are reserved by the system
		//         and may be non-zero when the command originates from an accelerator
		//         or a mouse action. The original compared wParam == 0xF060 exactly,
		//         which silently misses those cases.
		return (msg.wParam & 0xFFF0u) == SC_CLOSE;

	default:
		return false;
	}
}

constexpr LPARAM KeyContextCodeFlag = 1L << 29;

bool IsFullscreenToggle(const MSG& msg) noexcept
{
	return msg.message == WM_SYSKEYDOWN
		&& msg.wParam == VK_RETURN
		&& (msg.lParam & KeyContextCodeFlag) != 0;
}

// ---------------------------------------------------------------------------
// EXTENSION: drop every queued draw list before the toggle reaches the window
//            procedure, so no stale vertex/index data survives the device reset.
//
// SUSPECT: Alt+Enter is not the only path to a lost device. Alt+Tab in exclusive
//          fullscreen, WM_DISPLAYCHANGE, and driver-initiated resets all invalidate
//          the same buffers without producing this message. This clear belongs on
//          the device-lost / reset callback, not on a keystroke. Keeping it here
//          only mirrors the reference fork; treat it as a stopgap.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x5D4E3B, Windows_Message_Handler_Dispatch, 0x5)
{
	enum { Continue = 0x5D4E4D };

	LEA_STACK(MSG* const, pMsg, 0x10);

	if (IsQuitRequest(*pMsg) && SpawnerMain::GetMainConfigs()->QuickExit) {
		// DIFF: hard-terminates the process. Skips static destructors, DLL_PROCESS_DETACH,
		//       and any embedded-ReShade teardown. Preserved from the reference fork, but
		//       with ReShade living inside Phobos.dll this now leaks D3D9 resources on exit
		//       and bypasses the unified exception/terminate handlers. Consider routing
		//       through a proper shutdown path instead.
		ExitProcess(1u);
	}
	
	//resolution change clear the reshade resources
	// if (IsFullscreenToggle(*pMsg)
	// 	&& Phobos::Render::IsReShadeEnabled()
	// 	&& Phobos::Render::IsEnhancedLaserEnabled()) {
	// 	Phobos::Render::ClearAllDrawLists();
	// }

	TranslateMessage(pMsg);
	DispatchMessageA(pMsg);

	return Continue;
}

ASMJIT_PATCH(0x77786B, MainWindowProc_HandleRageQuit, 0x5)
{
	if (SpawnerMain::GetMainConfigs()->QuickExit) {

		if (Game::IsActive() && HouseClass::CurrentPlayer() && !Game::ScoreStuffLoad()) {
			RageQuit = true;
			//ASM_CALL(0x6471A0);
			EventClass e_DESTRUCT { HouseClass::CurrentPlayer->ArrayIndex, EventType::DESTRUCT };
			EventClass::AddEvent(&e_DESTRUCT);
			EventClass e_EXIT { HouseClass::CurrentPlayer->ArrayIndex, EventType::EXIT };
			EventClass::AddEvent(&e_EXIT);
		} else {
			Debug::ExitGame(0u);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x623125, OwnerDrawLoop_HandleRageQuit, 0x5)
{
	return RageQuit
		? 0x623157
		: 0;
}

ASMJIT_PATCH(0x6BE091, WinMain_AfterGameLoop_HandleQuickExit, 0x6)
{
	if (SpawnerMain::GetMainConfigs()->QuickExit)
		ExitProcess(0);

	return 0;
}

ASMJIT_PATCH(0x686570, DisconnectGracefully_HandleQuickExit, 0x5)
{
	if (SpawnerMain::GetMainConfigs()->QuickExit)
		ExitProcess(0);

	return 0;
}