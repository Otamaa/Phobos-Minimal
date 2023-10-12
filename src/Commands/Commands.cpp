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

#include <Misc/Ares/Hooks/Commands/AIBasePlan.h>
#include <Misc/Ares/Hooks/Commands/AIControl.h>
#include <Misc/Ares/Hooks/Commands/DumpMemory.h>
#include <Misc/Ares/Hooks/Commands/DumpTypes.h>
#include <Misc/Ares/Hooks/Commands/FPSCounter.h>
#include <Misc/Ares/Hooks/Commands/MapSnapshot.h>
#include <Misc/Ares/Hooks/Commands/TogglePower.h>

#define Make(arg)\
		CommandClass::Array->AddItem(GameCreateUnchecked<arg>());

DEFINE_HOOK(0x532150, CommandClassCallback_Register, 5)
{
	Make(PlaceVeinholeMonster);
	Make(RevealMapCommandClass);
	Make(ObjectInfoCommandClass);
	Make(NextIdleHarvesterCommandClass);
	Make(QuickSaveCommandClass);
	Make(CaptureObjectsCommandClass);
	//Make(ShowHealthPercentCommandClass);
	Make(DamageDisplayCommandClass);
	Make(ShowAnimNameCommandClass);
	Make(ShowTechnoNameCommandClass);
	Make(SetVeterancyCommandClass);
	Make(FrameByFrameCommandClass);
	Make(ShowTeamLeaderCommandClass);
	Make(ToggleRadialIndicatorDrawModeClass);
	Make(ToggleDigitalDisplayCommandClass);
	Make(ToggleDesignatorRangeCommandClass);
	Make(SaveVariablesToFileCommandClass);
	Make(FrameStepCommandClass<1>); // Single step in
	Make(FrameStepCommandClass<5>); // Speed 1
	Make(FrameStepCommandClass<10>); // Speed 2
	Make(FrameStepCommandClass<15>); // Speed 3
	Make(FrameStepCommandClass<30>); // Speed 4
	Make(FrameStepCommandClass<60>); // Speed 5

	Make(AIBasePlanCommandClass);

	if (Phobos::Otamaa::AllowAIControl)
		Make(AIControlCommandClass);

	Make(MemoryDumperCommandClass);
	Make(DumperTypesCommandClass);
	Make(FPSCounterCommandClass);
	Make(MapSnapshotCommandClass);
	Make(TogglePowerCommandClass);

	return 0x0;
}

#undef Make