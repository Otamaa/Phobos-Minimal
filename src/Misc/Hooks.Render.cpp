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

static void __fastcall _eb_DrawAll_Impl()
{ JMP_STD(0x4C2830); }

static void __fastcall ___eb_DrawAll()
{
	_eb_DrawAll_Impl();
	ElectricBoltManager::Draw_All();
}

DEFINE_HOOK(0x6D466E, TacticalClass_Render_ReplaceEbolt, 0x5)
{
	EBolt::DrawAll();
	ElectricBoltManager::Draw_All();
	return 0x6D4673;
}

//DEFINE_JUMP(CALL,0x6D466E, GET_OFFSET(___eb_DrawAll));
//DEFINE_HOOK(0x6D466E , TacticalClass_Render_BeforeGameEBolt, 0xA)
//{
//	ElectricBoltManager::Draw_All();
//	VeinholeMonsterClass::DrawAll();
//	return 0x0;
//}

//DEFINE_HOOK(0x6D4656 , TacticalClass_Render_BeforeIonBlast , 0x7)
//{
//	VeinholeMonsterClass::DrawAll();
//	return 0x0;
//}

static	void __fastcall TacticalClass_Render()
{
	IonBlastClass::DrawAll();
	VeinholeMonsterClass::DrawAll();
}

//DEFINE_JUMP(CALL,0x6D4656, GET_OFFSET(TacticalClass_Render));
DEFINE_HOOK(0x6D4656, TacticalClass_Render_ReplaceIonBlast, 0x5)
{
	IonBlastClass::DrawAll();
	VeinholeMonsterClass::DrawAll();
	return 0x6D465B;
}

DEFINE_HOOK(0x4F4583, GScreenClass_Render ,0xB)
{
#ifdef COMPILE_PORTED_DP_FEATURES_
	PrintTextManager::PrintAllText();
#endif
	FlyingStrings::UpdateAll();
	return 0;
}

/*
DEFINE_HOOK(0x6D4684, TacticalClass_Draw_FlyingStrings, 0x6)
{

	return 0;
}*/