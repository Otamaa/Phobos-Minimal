#include "Body.h"
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/Side/Body.h>

DEFINE_HOOK(0x6870D7, ReadScenario_LoadingScreens, 0x5)
{
	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0x174, -0x158));

	auto const pScenario = ScenarioClass::Instance.get();
	auto const scenarioName = pScenario->FileName;
	auto const defaultsSection = "Defaults";

	pScenario->LS640BriefLocX = pINI->ReadInteger(scenarioName, "LS640BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocX", 0));
	pScenario->LS640BriefLocY = pINI->ReadInteger(scenarioName, "LS640BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocY", 0));
	pScenario->LS800BriefLocX = pINI->ReadInteger(scenarioName, "LS800BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocX", 0));
	pScenario->LS800BriefLocY = pINI->ReadInteger(scenarioName, "LS800BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocY", 0));

	pINI->ReadString(defaultsSection, "DefaultLS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(scenarioName, "LS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);
	pINI->ReadString(scenarioName, "LS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);


	return 0x0;
}

DEFINE_HOOK(0x6873AB, INIClass_ReadScenario_EarlyLoadRules, 5)
{

	if (SessionClass::Instance->GameMode == GameMode::Campaign) {
		RulesClass::Instance()->Read_Sides(CCINIClass::INI_Rules);
		for (auto pSide : *SideClass::Array)
			SideExtContainer::Instance.LoadFromINI(pSide, CCINIClass::INI_Rules, false);
	}

	R->EAX(0x1180);
	return 0x6873B0;
}

DEFINE_HOOK(0x55DBF5, MainLoop_SaveGame, 0xA)
{
	return Phobos::Config::SaveGameOnScenarioStart ? 0 : 0x55DC99;
}