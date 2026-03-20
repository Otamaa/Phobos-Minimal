#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Common/Components/Component.h>
#include <Misc/Kratos/Common/Components/ComponentPool.h>
#include <Misc/Kratos/Common/EventSystems/EventSystem.h>
#include <Misc/Kratos/Common/INI/INI.h>
#include <Misc/Kratos/Common/INI/INIConstant.h>

#include <Misc/Kratos/Ext/Helper/MathEx.h>

#include <Misc/Kratos/Ext/Common/FireSuperManager.h>
#include <Misc/Kratos/Ext/Common/PaintballSyncManager.h>
#include <Misc/Kratos/Ext/Common/PrintTextManager.h>

class GeneraHook
{
public:
	GeneraHook()
	{
		EventSystems::General.AddHandler(Events::CmdLineParse, KratosCommon::CmdLineParse);
		EventSystems::General.AddHandler(Events::ExeRun, KratosCommon::ExeRun);
		EventSystems::General.AddHandler(Events::ExeTerminate, KratosCommon::ExeTerminate);
		EventSystems::General.AddHandler(Events::ScenarioStartEvent, ComponentPool::Clear);
		EventSystems::General.AddHandler(Events::ScenarioStartEvent, INIConstant::SetGameModeName);
		EventSystems::General.AddHandler(Events::ScenarioStartEvent, FireSuperManager::Clear);
		EventSystems::General.AddHandler(Events::ScenarioStartEvent, PaintballSyncManager::Clear);
		EventSystems::General.AddHandler(Events::ScenarioStartEvent, PrintTextManager::Clear);
		EventSystems::General.AddHandler(Events::ScenarioClearClassesEvent, INI::ClearBuffer);
		EventSystems::General.AddHandler(Events::ScenarioClearClassesEvent, ExtTypeRegistryClear);
		EventSystems::Logic.AddHandler(Events::LogicUpdateEvent, FireSuperManager::Update);
	}
};

static GeneraHook _GeneraHook;

#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x52F639, Game_CmdLineParse, 0x5)
{
	GET(char **, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	void *args[]{ppArgs, (void *)nNumArgs};
	EventSystems::General.Broadcast(Events::CmdLineParse, args);
	return 0;
}


ASMJIT_PATCH(0x6851F0, Scenario_ClearClasses_Start, 0x6)
{
	Kratos::ScenarioClearFlagSet();
	return 0;
}

ASMJIT_PATCH(0x685659, Scenario_ClearClasses_End, 0xA)
{
	Kratos::ScenarioClearFlagUnset();
	return 0;
}
#endif