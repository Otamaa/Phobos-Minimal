#include "Commands.h"

#include "CaptureObjects.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "ShowHealthPercent.h"
#include "DamageDisplay.h"

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
	return 0;
}

#undef Make