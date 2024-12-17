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

#include <Misc/Ares/Hooks/Commands/AIBasePlan.h>
#include <Misc/Ares/Hooks/Commands/AIControl.h>
#include <Misc/Ares/Hooks/Commands/DumpMemory.h>
#include <Misc/Ares/Hooks/Commands/DumpTypes.h>
#include <Misc/Ares/Hooks/Commands/FPSCounter.h>
#include <Misc/Ares/Hooks/Commands/MapSnapshot.h>
#include <Misc/Ares/Hooks/Commands/TogglePower.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>

template <typename T>
FORCEINLINE T* Make()
{
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
	return command;
};

DEFINE_HOOK(0x532150, CommandClassCallback_Register, 5)
{
	Make<PlaceVeinholeMonster>();
	Make<RevealMapCommandClass>();
	Make<ObjectInfoCommandClass>();
	Make<NextIdleHarvesterCommandClass>();
	Make<QuickSaveCommandClass>();
	Make<CaptureObjectsCommandClass>();
	//Make<ShowHealthPercentCommandClass>();
	Make<DamageDisplayCommandClass>();
	Make<ShowAnimNameCommandClass>();
	Make<ShowTechnoNameCommandClass>();
	Make<SetVeterancyCommandClass>();
	Make<FrameByFrameCommandClass>();
	Make<ShowTeamLeaderCommandClass>();
	Make<ToggleRadialIndicatorDrawModeClass>();
	Make<ToggleDigitalDisplayCommandClass>();
	Make<ToggleDesignatorRangeCommandClass>();
	Make<ToggleSWSidebar>();
	Make<SaveVariablesToFileCommandClass>();

	Make<FrameStepCommandClass<1>>(); // Single step in
	Make<FrameStepCommandClass<5>>(); // Speed 1
	Make<FrameStepCommandClass<10>>(); // Speed 2
	Make<FrameStepCommandClass<15>>(); // Speed 3
	Make<FrameStepCommandClass<30>>(); // Speed 4
	Make<FrameStepCommandClass<60>>(); // Speed 5

#ifndef ARES_CMD
	Make<AIBasePlanCommandClass>();

	if (Phobos::Otamaa::AllowAIControl)
		Make<AIControlCommandClass>();

	Make<MemoryDumperCommandClass>();
	Make<DumperTypesCommandClass>();
	Make<FPSCounterCommandClass>();
	Make<MapSnapshotCommandClass>();
	Make<TogglePowerCommandClass>();
#endif

	Make<DetachFromTeamCommandClass>();
	Make<SelectCapturedCommandClass>();

	SWSidebarClass::Commands[0] = Make<FireTacticalSWCommandClass<0>>();
	SWSidebarClass::Commands[1] = Make<FireTacticalSWCommandClass<1>>();
	SWSidebarClass::Commands[2] = Make<FireTacticalSWCommandClass<2>>();
	SWSidebarClass::Commands[3] = Make<FireTacticalSWCommandClass<3>>();
	SWSidebarClass::Commands[4] = Make<FireTacticalSWCommandClass<4>>();
	SWSidebarClass::Commands[5] = Make<FireTacticalSWCommandClass<5>>();
	SWSidebarClass::Commands[6] = Make<FireTacticalSWCommandClass<6>>();
	SWSidebarClass::Commands[7] = Make<FireTacticalSWCommandClass<7>>();
	SWSidebarClass::Commands[8] = Make<FireTacticalSWCommandClass<8>>();
	SWSidebarClass::Commands[9] = Make<FireTacticalSWCommandClass<9>>();

	return 0x0;
}

#undef Make