#include <Helpers/Macro.h>

#include "Body.h"

#include <TagClass.h>

DEFINE_HOOK_AGAIN(0x6857EA, PhobosSaveVariables, 0x5)//Win
DEFINE_HOOK(0x685EB1, PhobosSaveVariables, 0x5)//Lose
{
	//Debug::Log("%s , Executed [%x]!\n", __FUNCTION__ , R->Origin());

	if (Phobos::Config::SaveVariablesOnScenarioEnd)
	{
		ScenarioExtData::SaveVariablesToFile(false);
		ScenarioExtData::SaveVariablesToFile(true);
	}
	else if (R->Origin() == 0x6857EA) // Only Win
	{
		ScenarioExtData::SaveVariablesToFile(true);
	}

	return 0;
}

// I'm not sure how it works, this seems to solve the problem for now.
DEFINE_HOOK(0x4C6217, ScenarioClass_LoadVariables, 0x5)
{
	//Debug::Log("%s , Executed !\n", __FUNCTION__);
	ScenarioExtData::Instance()->LoadVariablesToFile(true);

	if (!Phobos::Config::SaveVariablesOnScenarioEnd)
	{
		// Is it better not to delete the file?
		DeleteFileA("globals.ini");
	}

	return 0x4C622F;
}

// Inside the original code it forces the use of ScenarioClass::Instance->GlobalVariables[1] as a startup switch.
// This is a big problem for Phobos that rewrote Variables.
DEFINE_HOOK(0x685A38, ScenarioClass_sub_685670_SetNextScenario, 0x6)
{
	enum { AltNextScenario = 0x685A4C, NextScenario = 0x685A59, Continue = 0x685A63 };

	if (ScenarioClass::Instance->SkipMapSelect)
	{
		if (IS_SAME_STR_N(ScenarioClass::Instance->AltNextScenario, ""))
		{
			auto const LocalVariables = ScenarioExtData::GetVariables(false);
			auto const GlobalVariables = ScenarioExtData::GetVariables(true);

			if (!LocalVariables->empty())
			{
				for (auto const& itr : *LocalVariables)
				{
					// AltNextScenario can be started as long as any variable has the name <Alternate Next Scenario>.
					if (IS_SAME_STR_N(itr.second.Name, "<Alternate Next Scenario>") || itr.second.Value <= 0)
						continue;

					return AltNextScenario;
				}
			}

			if (!GlobalVariables->empty())
			{
				for (auto const& itr : *GlobalVariables)
				{
					// Same result as above.
					if (IS_SAME_STR_N(itr.second.Name, "<Alternate Next Scenario>") || itr.second.Value <= 0)
						continue;

					return AltNextScenario;
				}
			}
		}

		if (IS_SAME_STR_N(ScenarioClass::Instance->NextScenario, ""))
		{
			return NextScenario;
		}
	}

	return Continue;
}

DEFINE_HOOK(0x689910, ScenarioClass_SetLocalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExtData::Instance()->SetVariableToByID(false, nIndex, bState);

	return 0x689955;
}

DEFINE_HOOK(0x689A00, ScenarioClass_GetLocalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExtData::Instance()->GetVariableStateByID(false, nIndex, pOut);

	return 0x689A26;
}

DEFINE_HOOK(0x689B20, ScenarioClass_ReadLocalVariables, 0x6)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExtData::Instance()->ReadVariables(false, pINI);

	return 0x689C4B;
}

DEFINE_HOOK(0x689670, ScenarioClass_SetGlobalToByID, 0x5)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(const char, bState, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExtData::Instance()->SetVariableToByID(true, nIndex, bState);

	return 0x6896AF;
}

DEFINE_HOOK(0x689760, ScenarioClass_GetGlobalStateByID, 0x6)
{
	GET_STACK(const int, nIndex, 0x4);
	GET_STACK(char*, pOut, 0x8);
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	ScenarioExtData::Instance()->GetVariableStateByID(true, nIndex, pOut);

	return 0x689786;
}

// Called by MapGeneratorClass
DEFINE_HOOK(0x689880, ScenarioClass_ReadGlobalVariables, 0x6)
{
	GET_STACK(CCINIClass* const, pINI, 0x4);

	ScenarioExtData::Instance()->ReadVariables(true, pINI);

	return 0x6898FF;
}

// ScenarioClass_ReadGlobalVariables inlined in Read_Scenario_INI
DEFINE_HOOK(0x6876CE, ReadScenarioINI_Inlined_ReadGlobalVariables, 0x9)
{
	GET(CCINIClass* const, pINI, EBP);

	ScenarioExtData::Instance()->ReadVariables(true, pINI);

	// Stupid inline
	R->ESI(GameMode::Campaign);

	return 0x68773F;
}