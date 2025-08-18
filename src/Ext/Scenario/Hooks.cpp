#include "Body.h"
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/Side/Body.h>

#include <LoadProgressManager.h>

ASMJIT_PATCH(0x6870D7, ReadScenario_LoadingScreens, 0x5)
{
	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0x174, -0x158));

	auto const pScenario = ScenarioClass::Instance.get();
	auto const pScenarioExt = ScenarioExtData::Instance();
	auto const scenarioName = pScenario->FileName;
	auto const defaultsSection = "Defaults";

	pScenarioExt->DefaultLS640BkgdName.Read(pINI, defaultsSection, "DefaultLS640BkgdName");
	pScenarioExt->DefaultLS800BkgdName.Read(pINI, defaultsSection, "DefaultLS800BkgdName");
	pScenarioExt->DefaultLS800BkgdPal.Read(pINI, defaultsSection, "DefaultLS800BkgdPal");

	pScenarioExt->ShowBriefing = pINI->ReadBool(scenarioName, "ShowBriefing", pScenarioExt->ShowBriefing);
	pScenarioExt->BriefingTheme = pINI->ReadTheme(scenarioName, "BriefingTheme", pScenarioExt->BriefingTheme);

	pScenario->LS640BriefLocX = pINI->ReadInteger(scenarioName, "LS640BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocX", 0));
	pScenario->LS640BriefLocY = pINI->ReadInteger(scenarioName, "LS640BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocY", 0));
	pScenario->LS800BriefLocX = pINI->ReadInteger(scenarioName, "LS800BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocX", 0));
	pScenario->LS800BriefLocY = pINI->ReadInteger(scenarioName, "LS800BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocY", 0));

	pINI->ReadString(scenarioName, "LS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);

	return 0x0;
}

ASMJIT_PATCH(0x6873AB, INIClass_ReadScenario_EarlyLoadRules, 5)
{

	if (SessionClass::Instance->GameMode == GameMode::Campaign) {
		RulesClass::Instance()->Read_Sides(CCINIClass::INI_Rules);
		for (auto pSide : *SideClass::Array)
			SideExtContainer::Instance.LoadFromINI(pSide, CCINIClass::INI_Rules, false);
	}

	R->EAX(0x1180);
	return 0x6873B0;
}

ASMJIT_PATCH(0x552F79, LoadProgressManager_Draw_MissingLoadingScreenDefaults, 0x6)
{
	GET(LoadProgressManager*, pThis, EBP);
	GET(ConvertClass*, pDrawer, EBX);
	GET_STACK(bool, isLowRes, STACK_OFFSET(0x1268, -0x1235));

	auto const pScenarioExt = ScenarioExtData::Instance();

	if (!pThis->LoadScreenSHP)
		pThis->LoadScreenSHP = FileSystem::LoadSHPFile(isLowRes ? pScenarioExt->DefaultLS640BkgdName : pScenarioExt->DefaultLS800BkgdName);

	if (!pDrawer) {
		// Uncertain how necessary this is but is what game does...
		if (LoadProgressManager::LoadScreenPal) {
			GameDelete<true,false>(LoadProgressManager::LoadScreenPal());
			LoadProgressManager::LoadScreenPal = nullptr;
		}

		if (LoadProgressManager::LoadScreenBytePal) {
			GameDelete<true,false>(LoadProgressManager::LoadScreenBytePal());
			LoadProgressManager::LoadScreenBytePal = nullptr;
		}

		ConvertClass::CreateFromFile(pScenarioExt->DefaultLS800BkgdPal, LoadProgressManager::LoadScreenBytePal, LoadProgressManager::LoadScreenPal);

		R->EBX(LoadProgressManager::LoadScreenPal());
	}

	return 0;
}
