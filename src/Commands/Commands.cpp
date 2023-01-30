#include "Commands.h"

#include "CaptureObjects.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "ShowHealthPercent.h"
#include "DamageDisplay.h"
#include "FrameStep.h"
#include "ShowTechnoNames.h"

#define Make(arg)\
		Make<arg>();

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	Make(ObjectInfoCommandClass);
	Make(NextIdleHarvesterCommandClass);
	Make(QuickSaveCommandClass);
	Make(CaptureObjectsCommandClass);
	Make(ShowHealthPercentCommandClass);
	Make(DamageDisplayCommandClass);
	Make(ShowTechnoNameCommandClass);
	Make(FrameByFrameCommandClass);
	Make(FrameStepCommandClass<1>); // Single step in
	Make(FrameStepCommandClass<5>); // Speed 1
	Make(FrameStepCommandClass<10>); // Speed 2
	Make(FrameStepCommandClass<15>); // Speed 3
	Make(FrameStepCommandClass<30>); // Speed 4
	Make(FrameStepCommandClass<60>); // Speed 5
	return 0;
}

#undef Make