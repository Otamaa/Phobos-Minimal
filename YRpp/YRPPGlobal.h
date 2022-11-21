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
#pragma endregion GlobalVarDeclaration
