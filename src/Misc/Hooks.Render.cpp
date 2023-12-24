#include <Utilities/Macro.h>

#include <New/Entity/ElectricBoltClass.h>
#include <New/Entity/FlyingStrings.h>

#ifdef COMPILE_PORTED_DP_FEATURES_
#include <Misc/DynamicPatcher/Others/TextManager.h>
#endif

#include <FootClass.h>
#include <TechnoClass.h>
#include <ObjectClass.h>

#include <IonBlastClass.h>
#include <VeinholeMonsterClass.h>

#include <Commands/ShowTechnoNames.h>
#include <Commands/ShowAnimNames.h>

DEFINE_HOOK(0x4F4583, GScreenClass_Render ,0x6) //B
{
	Phobos::DrawVersionWarning();
	return 0;
}

DEFINE_HOOK(0x6D4684, TacticalClass_Draw_Addition, 6)
{
	ShowTechnoNameCommandClass::AI();
	ShowAnimNameCommandClass::AI();
	FlyingStrings::UpdateAll();
	return 0;
}