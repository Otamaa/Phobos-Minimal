#include "Body.h"

#include <Ext/TAction/Body.h>

#include <HouseClass.h>
#include <Helpers/Macro.h>

// score options
// score music for single player missions
DEFINE_HOOK(0x6C924F, ScoreDialog_Handle_ScoreThemeA, 0x5)
{
	GET(char*, pTitle, ECX);
	GET(char*, pMessage, ESI);
	CSFText& Title = ScenarioExt::Global()->ParTitle;
	CSFText& Message = ScenarioExt::Global()->ParMessage;

	CRT::strcpy(pTitle, Title.Label);
	CRT::strcpy(pMessage, Message.Label);

	return 0;
}

DEFINE_HOOK(0x6C935C, ScoreDialog_Handle_ScoreThemeB, 0x5)
{
	REF_STACK(char*, pTheme, 0x0);

	auto& Theme = ScenarioExt::Global()->ScoreCampaignTheme;

	if (Theme.isset())
		CRT::strcpy(pTheme, Theme.Get().data());

	return 0;
}

DEFINE_HOOK(0x5AE192, SelectNextMissionScenario, 0x6)
{
	if (ScenarioExt::Global()->NextMission.isset())
		R->EAX(ScenarioExt::Global()->NextMission.Get().data());

	return 0;
}

DEFINE_HOOK(0x6851AC, LoadGame_Initialize_IonStormClass, 0x5)
{
	auto swap_data = []()
	{
		std::swap(ScenarioExt::Global()->DefaultAmbientOriginal, ScenarioClass::Instance->AmbientOriginal);
		std::swap(ScenarioExt::Global()->DefaultAmbientCurrent, ScenarioClass::Instance->AmbientCurrent);
		std::swap(ScenarioExt::Global()->DefaultAmbientTarget, ScenarioClass::Instance->AmbientTarget);
		std::swap(ScenarioExt::Global()->DefaultNormalLighting, ScenarioClass::Instance->NormalLighting);
	};

	swap_data();

	MapClass::Instance->CellIteratorReset();
	for (auto pCell = MapClass::Instance->CellIteratorNext(); pCell; pCell = MapClass::Instance->CellIteratorNext())
	{
		if (pCell->LightConvert)
			CallDTOR<false>(pCell->LightConvert);

		pCell->InitLightConvert();
	}

	swap_data();


	for (auto& pLightConvert : *LightConvertClass::Array)
		pLightConvert->UpdateColors(
			ScenarioExt::Global()->CurrentTint_Tiles.Red * 10,
			ScenarioExt::Global()->CurrentTint_Tiles.Green * 10,
			ScenarioExt::Global()->CurrentTint_Tiles.Blue * 10,
			false);

	if (ScenarioExt::Global()->CurrentTint_Schemes != TintStruct { -1,-1,-1 })
	{
		for (auto& pScheme : *ColorScheme::Array)
			pScheme->LightConvert->UpdateColors(
				ScenarioExt::Global()->CurrentTint_Schemes.Red * 10,
				ScenarioExt::Global()->CurrentTint_Schemes.Green * 10,
				ScenarioExt::Global()->CurrentTint_Schemes.Blue * 10,
				false);
	}

	if (ScenarioExt::Global()->CurrentTint_Hashes != TintStruct { -1,-1,-1 })
	{
		ScenarioClass::UpdateHashPalLighting(
			ScenarioExt::Global()->CurrentTint_Hashes.Red * 10,
			ScenarioExt::Global()->CurrentTint_Hashes.Green * 10,
			ScenarioExt::Global()->CurrentTint_Hashes.Blue * 10,
			false);
	}

	TActionExt::RecreateLightSources();
	ScenarioClass::UpdateCellLighting();

	HouseClass::CurrentPlayer->RecheckRadar = true;

	return 0x6851B1;
}

//DEFINE_HOOK(0x683A3A, ScenarioClass_Init_Bugfix, 0x6)
//{
//	GET(ScenarioClass*, pThis, EBP);
//	pThis->NumberStartingPoints = 0;
//	return 0;
//}