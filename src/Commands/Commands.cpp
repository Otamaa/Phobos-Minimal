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

#include <Misc/Ares/Hooks/Commands/AIBasePlan.h>
#include <Misc/Ares/Hooks/Commands/AIControl.h>
#include <Misc/Ares/Hooks/Commands/DumpMemory.h>
#include <Misc/Ares/Hooks/Commands/DumpTypes.h>
#include <Misc/Ares/Hooks/Commands/FPSCounter.h>
#include <Misc/Ares/Hooks/Commands/MapSnapshot.h>
#include <Misc/Ares/Hooks/Commands/TogglePower.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>

bool PhobosCommandClass::CheckDebugDeactivated() const
{
	auto const bAllow = Phobos::Config::DevelopmentCommands || Phobos::Otamaa::IsAdmin;

	if (!bAllow)
	{
		if (const wchar_t* text = StringTable::LoadString("TXT_COMMAND_DISABLED"))
		{
			wchar_t msg[0x100] = L"\0";
			wsprintfW(msg, text, this->GetUIName());
			MessageListClass::Instance->PrintMessage(msg);
		}
		return true;
	}
	return false;
}

template <typename T>
FORCEDINLINE T* Make()
{
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
	return command;
};

DEFINE_HOOK(0x532150, CommandClassCallback_Register, 5)
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
		Make<FrameStepCommandClass<1>>(); // Single step in
		Make<FrameStepCommandClass<5>>(); // Speed 1
		Make<FrameStepCommandClass<10>>(); // Speed 2
		Make<FrameStepCommandClass<15>>(); // Speed 3
		Make<FrameStepCommandClass<30>>(); // Speed 4
		Make<FrameStepCommandClass<60>>(); // Speed 5

		Make<AIBasePlanCommandClass>();

		Make<AIControlCommandClass>();
	}
#pragma endregion Adminexclusive

	Make<NextIdleHarvesterCommandClass>();
	Make<QuickSaveCommandClass>();
	Make<SaveVariablesToFileCommandClass>();
	Make<AutoBuildingCommandClass>();
	Make<ToggleRadialIndicatorDrawModeClass>();
	Make<ToggleDigitalDisplayCommandClass>();
	Make<ToggleDesignatorRangeCommandClass>();
	Make<DistributionMode1CommandClass>();
	Make<DistributionMode2CommandClass>();
	Make<TogglePowerCommandClass>();

#pragma region SWSidebar
	if(SWSidebarClass::IsEnabled()){
		Make<ToggleSWSidebar>();

		SWSidebarClass::Commands[0] = Make<FireTacticalSWCommandClass<1>>();
		SWSidebarClass::Commands[1] = Make<FireTacticalSWCommandClass<2>>();
		SWSidebarClass::Commands[2] = Make<FireTacticalSWCommandClass<3>>();
		SWSidebarClass::Commands[3] = Make<FireTacticalSWCommandClass<4>>();
		SWSidebarClass::Commands[4] = Make<FireTacticalSWCommandClass<5>>();
		SWSidebarClass::Commands[5] = Make<FireTacticalSWCommandClass<6>>();
		SWSidebarClass::Commands[6] = Make<FireTacticalSWCommandClass<7>>();
		SWSidebarClass::Commands[7] = Make<FireTacticalSWCommandClass<8>>();
		SWSidebarClass::Commands[8] = Make<FireTacticalSWCommandClass<9>>();
		SWSidebarClass::Commands[9] = Make<FireTacticalSWCommandClass<10>>();
	}
#pragma endregion SWSidebar

	return 0x0;
}

#undef Make

#include <Helpers/Macro.h>
#include <WWKeyboardClass.h>

DEFINE_HOOK(0x533F50, Game_ScrollSidebar_Skip, 0x5)
{
	enum { SkipScrollSidebar = 0x533FC3 };

	if (!Phobos::Config::ScrollSidebarStripWhenHoldKey)
	{
		const auto pInput = InputManagerClass::Instance();

		if (pInput->IsForceFireKeyPressed() || pInput->IsForceMoveKeyPressed() || pInput->IsForceSelectKeyPressed())
			return SkipScrollSidebar;
	}

	if (!Phobos::Config::ScrollSidebarStripInTactical)
	{
		const auto pMouse = WWMouseClass::Instance();

		if (pMouse->XY1.X < Make_Global<int>(0xB0CE30)) // TacticalClass::view_bound.Width
			return SkipScrollSidebar;
	}

	return 0;
}