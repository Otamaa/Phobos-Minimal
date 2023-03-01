#include <Utilities/Macro.h>

#include <New/Entity/ElectricBoltClass.h>
#include <New/Entity/FlyingStrings.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/TextManager.h>
#endif

#include <FootClass.h>
#include <TechnoClass.h>
#include <ObjectClass.h>

#include <IonBlastClass.h>
#include <VeinholeMonsterClass.h>

#include <Commands/ShowTechnoNames.h>

DEFINE_HOOK(0x6D466E, TacticalClass_Render_ReplaceEbolt, 0x5)
{
	EBolt::DrawAll();
	ElectricBoltManager::Draw_All();
	return 0x6D4673;
}

DEFINE_HOOK(0x6D4656, TacticalClass_Render_ReplaceIonBlast, 0x5)
{
	IonBlastClass::DrawAll();
	VeinholeMonsterClass::DrawAll();
	return 0x6D465B;
}

DEFINE_HOOK(0x4F4583, GScreenClass_Render ,0x6) //B
{
#ifdef COMPILE_PORTED_DP_FEATURES_
	PrintTextManager::PrintAllText();
#endif
	FlyingStrings::UpdateAll();
	ShowTechnoNameCommandClass::AI();

	return 0;
}

/*
DEFINE_HOOK(0x6D4684, TacticalClass_Draw_FlyingStrings, 0x6)
{

	return 0;
}*/