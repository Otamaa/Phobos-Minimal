#include <Helpers/Macro.h>

#include "Body.h"

#include <TagClass.h>

DEFINE_HOOK(0x685670, DoWin_SaveVariables, 0x5)
{
	if (Phobos::Config::SaveVariablesOnScenarioEnd)
	{
		ScenarioExt::SaveVariablesToFile(false);
		ScenarioExt::SaveVariablesToFile(true);
	}

	return 0;
}

DEFINE_HOOK(0x685DC0, DoLose_SaveVariables, 0x5)
{
	if (Phobos::Config::SaveVariablesOnScenarioEnd)
	{
		ScenarioExt::SaveVariablesToFile(false);
		ScenarioExt::SaveVariablesToFile(true);
	}

	return 0;
}

DEFINE_HOOK(0x689910, ScenarioClass_SetLocalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExt::Global()->SetVariableToByID(false, nIndex, bState);

	return 0x689955;
}

DEFINE_HOOK(0x689A00, ScenarioClass_GetLocalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExt::Global()->GetVariableStateByID(false, nIndex, pOut);

	return 0x689A26;
}

DEFINE_HOOK(0x689B20, ScenarioClass_ReadLocalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExt::Global()->ReadVariables(false, pINI);

	return 0x689C4B;
}

DEFINE_HOOK(0x689670, ScenarioClass_SetGlobalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExt::Global()->SetVariableToByID(true, nIndex, bState);

	return 0x6896AF;
}

DEFINE_HOOK(0x689760, ScenarioClass_GetGlobalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExt::Global()->GetVariableStateByID(true, nIndex, pOut);

	return 0x689786;
}

// Called by MapGeneratorClass
DEFINE_HOOK(0x689880, ScenarioClass_ReadGlobalVariables, 0x6)
{
	GET_STACK(CCINIClass* const, pINI, 0x4);

	ScenarioExt::Global()->ReadVariables(true, pINI);

	return 0x6898FF;
}

// ScenarioClass_ReadGlobalVariables inlined in Read_Scenario_INI
DEFINE_HOOK(0x6876CE, ReadScenarioINI_Inlined_ReadGlobalVariables, 0x9)
{
	GET(CCINIClass* const, pINI, EBP);

	ScenarioExt::Global()->ReadVariables(true, pINI);

	// Stupid inline
	R->ESI(GameMode::Campaign);

	return 0x68773F;
}