#pragma once
#include <YRPP.h>
#include <ASMMacros.h>
#include <GameOptionsClass.h>
#include <WWMouseClass.h>
#include <Helpers/Macro.h>
#include <SidebarClass.h>

#pragma region GlobalVarDeclaration
extern GScreenClass& GScreen;
extern BombListClass& BombList;
extern MouseClass& Map;
extern CellClass& WorkingCellInstance;
extern RulesClass*& RulesGlobal;
extern ScenarioClass*& ScenarioGlobal;
extern Random2Class& Random2Global;
extern ParticleSystemClass*& ParticleSystemGlobal;
extern GameOptionsClass& GameOptions;
extern GameModeOptionsClass& GameModeOptions;
extern TacticalClass*& TacticalGlobal;
extern MessageListClass& MessageListGlobal;
extern SessionClass& SessionGlobal;
extern WWMouseClass*& WWMouse;
extern DynamicVectorClass<ULONG> &ClassFactories;
#pragma endregion GlobalVarDeclaration

extern LPCRITICAL_SECTION& CRT_Critical_Sections;
extern LPCRITICAL_SECTION& _87C2EC_Critical_Sections;
extern LPCRITICAL_SECTION& _87C2DC_Critical_Sections;
extern LPCRITICAL_SECTION& _87C2CC_Critical_Sections;
extern LPCRITICAL_SECTION& _87C2AC_Critical_Sections;

extern HANDLE& CRT_Heap;
extern volatile LONG& _unguarded_readlc_active;

#pragma region Array
ARRAY2D_DEC(short, Wave_LUT_Pythagoras, 300, 300);
ARRAY2D_DEC(Point2D, LaserClass_DrawData, 8, 2);
ARRAY2D_DEC(char, AlphaShapeArray, 256, 256);
ARRAY2D_DEC(SelectClass*, SelectButton, 1, 14);

static constexpr reference<short, 0xB45E68u, 496u> const Wave_MagneticBeamSineTable {};
static constexpr reference<short, 0xB46254u, 500u> const Wave_SonicBeamSineTable {};
static constexpr reference<int, 0xB46648u, 14u> const Wave_LUT_Linear1 {};
static constexpr reference<Matrix3D, 0xB45DA8u, 4u> const WaveMatrix {};
static constexpr reference<Matrix3D, 0xB45CA0u, 4u> const MagneticWaveMatrix {};
static constexpr reference<Point2D, 0x8A0180u, 15u> const DiscLaserPoint {};
static constexpr reference<Point2D, 0x89F6D8 ,8u> AdjacentCoord {};
#pragma endregion Array

//static void __fastcall Detach_This_From_All(AbstractClass* target, bool all = true) { JMP_STD(0x7258D0); }