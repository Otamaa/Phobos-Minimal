#include "Commands.h"

#include "CaptureObjects.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "ShowHealthPercent.h"
#include "DamageDisplay.h"
#include "FrameStep.h"
#include "ShowAnimNames.h"
#include "ShowTechnoNames.h"
#include "SetVeterancy.h"
#include "ShowTeamLeader.h"
#include "RevealMap.h"
#include "PlaceVeinholeMonster.h"
#include "ToggleRadialIndicatorDrawMode.h"
#include "ToggleDigitalDisplay.h"
#include "ToggleDesignatorRange.h"
#include "SaveVariablesToFile.h"
#include "DetachFromTeam.h"
#include "SelectCaptured.h"
#include "ToggleSWSidebar.h"
#include "FireTacticalSW.h"
#include "ManualReloadAmmo.h"
#include "AutoBuilding.h"
#include "DistributionMode.h"
#include "Harmless.h"
#include "ForceWin.h"
#include "ToggleMessageList.h"
#include "CeasefireModeClass.h"
#include "AggressiveModeClass.h"
#include "Deselect.h"
#include "ToggleSuperTimers.h"

#include <Commands/AIBasePlan.h>
#include <Commands/AIControl.h>
#include <Commands/DumpMemory.h>
#include <Commands/DumpTypes.h>
#include <Commands/FPSCounter.h>
#include <Commands/MapSnapshot.h>
#include <Commands/TogglePower.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>
#include <New/MessageHandler/MessageColumnClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include <WWKeyboardClass.h>

bool PhobosCommandClass::CheckDebugDeactivated() const
{
	auto const bAllow = Phobos::Config::DevelopmentCommands || Phobos::Otamaa::IsAdmin;

	if (!bAllow)
	{
		const wchar_t* text = StringTable::FetchString("TXT_COMMAND_DISABLED");
		wchar_t msg[0x100] = L"\0";
		wsprintfW(msg, text, this->GetUIName());
		MessageListClass::Instance->PrintMessage(msg);
		return true;
	}
	return false;
}

template <typename T>
FORCEDINLINE T* Make()
{
	T* command = GameCreate<T>();
	CommandClass::Array->push_back(command);
	return command;
};

ASMJIT_PATCH(0x532150, CommandClassCallback_Register, 5)
{
	Make<ManualReloadAmmoCommandClass>();

#pragma region Information
	Make<ShowAnimNameCommandClass>();
	Make<ShowTechnoNameCommandClass>();
	Make<ShowTeamLeaderCommandClass>();
	Make<ObjectInfoCommandClass>();
	Make<DamageDisplayCommandClass>();
	Make<FPSCounterCommandClass>();
	//Make<ShowHealthPercentCommandClass>();

#pragma endregion Information

#pragma region Adminexclusive
	if(Phobos::Otamaa::IsAdmin) {
		Make<PlaceVeinholeMonster>();
		Make<RevealMapCommandClass>();
		Make<CaptureObjectsCommandClass>();
		Make<SetVeterancyCommandClass>();
		Make<MemoryDumperCommandClass>();
		Make<DumperTypesCommandClass>();
		Make<MapSnapshotCommandClass>();
		Make<DetachFromTeamCommandClass>();
		Make<SelectCapturedCommandClass>();

		Make<FrameByFrameCommandClass>();
		FrameStepDispatch::Dispatch();

		Make<AIBasePlanCommandClass>();

		Make<AIControlCommandClass>();
		Make<HarmlessCommandClass>();

		Make<ForceWinCommandClass>();
	}
#pragma endregion Adminexclusive

	Make<AggressiveModeClass>();
	Make<AutoBuildingCommandClass>();
	Make<CeasefireModeClass>();

	Make<QuickSaveCommandClass>();
	Make<SaveVariablesToFileCommandClass>();
	Make<TogglePowerCommandClass>();
	Make<ToggleRadialIndicatorDrawModeClass>();
	Make<ToggleDigitalDisplayCommandClass>();
	Make<ToggleDesignatorRangeCommandClass>();
	Make<ToggleMessageListCommandClass>();
	Make<NextIdleHarvesterCommandClass>();

	Make<ToggleSuperTimersCommandClass>();

	if (Phobos::Config::AllowSwitchNoMoveCommand)
		Make<SwitchNoMoveCommandClass>();

	if (Phobos::Config::AllowDistributionCommand)
	{
		if (Phobos::Config::AllowDistributionCommand_SpreadMode)
			Make<DistributionModeSpreadCommandClass>();

		if (Phobos::Config::AllowDistributionCommand_FilterMode)
			Make<DistributionModeFilterCommandClass>();

		Make<DistributionModeHoldDownCommandClass>();
	}

#pragma region SWSidebar
	Make<ToggleSWSidebar>();
	FireTacticalSWDispatch::Dispatch();
#pragma endregion SWSidebar

	Make<DeselectObjectCommandClass>();
	Make<DeselectObject5CommandClass>();

	return 0x0;
}

#undef Make

ASMJIT_PATCH(0x533F50, Game_ScrollSidebar_Skip, 0x5)
{
	enum { SkipScrollSidebar = 0x533FC3 };

	if (DistributionModeHoldDownCommandClass::Enabled)
		return SkipScrollSidebar;

	if (!Phobos::Config::ScrollSidebarStripWhenHoldKey)
	{
		const auto pInput = InputManagerClass::Instance();

		if (pInput->IsForceFireKeyPressed() || pInput->IsForceMoveKeyPressed() || pInput->IsForceSelectKeyPressed())
			return SkipScrollSidebar;
	}

	if (!Phobos::Config::ScrollSidebarStripInTactical) {
		const auto pMouse = WWMouseClass::Instance();

		if (pMouse->XY1.X < Make_Global<int>(0xB0CE30)) // TacticalClass::view_bound.Width
			return SkipScrollSidebar;
	}

	if(MessageColumnClass::Instance.IsHovering())
		return SkipScrollSidebar ;

	return 0;
}

ASMJIT_PATCH(0x777998, Game_WndProc_ScrollMouseWheel, 0x6)
{
	GET(const WPARAM, WParam, ECX);

	if (WParam & 0x80000000u) {

		if (DistributionModeHoldDownCommandClass::Enabled && Phobos::Config::AllowDistributionCommand_SpreadModeScroll)
			DistributionModeHoldDownCommandClass::DistributionSpreadModeReduce();

		 if(MessageColumnClass::Instance.IsHovering())
			 MessageColumnClass::Instance.ScrollDown();
	} else {

		if (DistributionModeHoldDownCommandClass::Enabled && Phobos::Config::AllowDistributionCommand_SpreadModeScroll)
			DistributionModeHoldDownCommandClass::DistributionSpreadModeExpand();

		if (MessageColumnClass::Instance.IsHovering())
			MessageColumnClass::Instance.ScrollUp();
	}

	return 0;
}

//this is still 0.A code , need check the new one ,..
void __fastcall ScreenCaptureCommandClass_Process(CommandClass* pThis, DWORD)
{
	RECT Viewport = {};
	if (Imports::GetWindowRect.invoke()(Game::hWnd, &Viewport))
	{
		POINT TL = { Viewport.left, Viewport.top }, BR = { Viewport.right, Viewport.bottom };
		if (Imports::ClientToScreen.invoke()(Game::hWnd, &TL) && Imports::ClientToScreen.invoke()(Game::hWnd, &BR))
		{
			RectangleStruct ClipRect = { TL.x, TL.y, Viewport.right + 1, Viewport.bottom + 1 };

			DSurface* Surface = DSurface::Primary;

			int width = Surface->Get_Width();
			int height = Surface->Get_Height();

			size_t arrayLen = width * height;

			if (width < ClipRect.Width)
			{
				ClipRect.Width = width;
			}

			if (height < ClipRect.Height)
			{
				ClipRect.Height = height;
			}

			WWMouseClass::Instance->HideCursor();

			if (WORD* buffer = reinterpret_cast<WORD*>(Surface->Lock(0, 0)))
			{
				//char fName[0x80];
				const std::string fName = "SCRN." + Debug::GetCurTimeA() + ".BMP";
				CCFileClass ScreenShot { fName.c_str() };
				ScreenShot.Open(FileAccessMode::Write);

#pragma pack(push, 1)
				struct bmpfile_full_header
				{
					unsigned char magic[2];
					DWORD filesz;
					WORD creator1;
					WORD creator2;
					DWORD bmp_offset;
					DWORD header_sz;
					DWORD width;
					DWORD height;
					WORD nplanes;
					WORD bitspp;
					DWORD compress_type;
					DWORD bmp_bytesz;
					DWORD hres;
					DWORD vres;
					DWORD ncolors;
					DWORD nimpcolors;
					DWORD R; //
					DWORD G; //
					DWORD B; //
				} h {};
#pragma pack(pop)

				h.magic[0] = 'B';
				h.magic[1] = 'M';

				h.creator1 = h.creator2 = 0;

				h.header_sz = 40;
				h.width = width;
				h.height = -height; // magic! no need to reverse rows this way
				h.nplanes = 1;
				h.bitspp = 16;
				h.compress_type = BI_BITFIELDS;
				h.bmp_bytesz = arrayLen * 2;
				h.hres = 4000;
				h.vres = 4000;
				h.ncolors = h.nimpcolors = 0;

				h.R = 0xF800;
				h.G = 0x07E0;
				h.B = 0x001F; // look familiar?

				h.bmp_offset = sizeof(h);
				h.filesz = h.bmp_offset + h.bmp_bytesz;

				ScreenShot.WriteBytes(&h, sizeof(h));
				std::vector<WORD> _pixelData(arrayLen);
				WORD* pixels = _pixelData.data();
				int pitch = Surface->VideoSurfaceDescription->lPitch;
				for (int r = 0; r < height; ++r)
				{
					memcpy(pixels, reinterpret_cast<void*>(buffer), width * 2);
					pixels += width;
					buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
				}

				ScreenShot.WriteBytes(_pixelData.data(), arrayLen * 2);
				ScreenShot.Close();
				Debug::LogInfo("Wrote screenshot to file {}", fName);
				Surface->Unlock();
			}

			WWMouseClass::Instance->ShowCursor();
		}
	}
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EBF24, ScreenCaptureCommandClass_Process);

ASMJIT_PATCH(0x730E39, GuardCommandClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || pType->Weeder);
	return 0x730E3F;
}

// AttackMove Only for Foot
//CommandClass_Attack_Move
DEFINE_PATCH_TYPED(BYTE, 0x731B67, 4u);
