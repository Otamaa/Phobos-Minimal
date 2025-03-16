#include "Body.h"
#include <Ext/Infantry/Body.h>

#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x4148F4, AircraftClass_DrawIt_LevelIntensity, 0x5)
{
	enum { SkipGameCode = 0x41493E };

	GET(AircraftClass*, pThis, EBP);
	GET(int, level, EDI);

	if (PsyDom::IsActive())
		level = ScenarioClass::Instance->DominatorLighting.Level;
	else if (NukeFlash::IsFadingIn())
		level = ScenarioClass::Instance->NukeLighting.Level;

	auto const pRulesExt = RulesExtData::Instance();
	int levelIntensity = 0;
	int cellIntensity = 1000;
	TechnoExtData::GetLevelIntensity(pThis, level, levelIntensity, cellIntensity, pRulesExt->AircraftLevelLightMultiplier, pRulesExt->AircraftCellLightLevelMultiplier);

	R->ESI(levelIntensity);
	R->EBX(cellIntensity);
	R->EAX(pThis->Locomotor.GetInterfacePtr()); // Restore overridden instruction

	return SkipGameCode;
}

ASMJIT_PATCH(0x51933B, InfantryClass_DrawIt_LevelIntensity, 0x6)
{
	enum { SkipGameCode = 0x51944D };

	GET(InfantryClass*, pThis, EBP);
	GET(int, level, EBX);

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		auto const pRulesExt = RulesExtData::Instance();
		int levelIntensity = 0;
		int cellIntensity = 1000;
		bool applyBridgeBonus = pRulesExt->JumpjetCellLightApplyBridgeHeight ? TechnoExtData::IsOnBridge(pThis) : false;
		TechnoExtData::GetLevelIntensity(pThis, level, levelIntensity, cellIntensity, pRulesExt->JumpjetLevelLightMultiplier, pRulesExt->JumpjetCellLightLevelMultiplier, applyBridgeBonus);

		R->ESI(levelIntensity + cellIntensity);
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x73CFA7, UnitClass_DrawIt_LevelIntensity, 0x6)
{
	enum { SkipGameCode = 0x73D0C3 };

	GET(UnitClass*, pThis, ESI);

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		int level = ScenarioClass::Instance->NormalLighting.Level;

		if (LightningStorm::IsActive())
			level = ScenarioClass::Instance->IonLighting.Level;
		else if (PsyDom::IsActive())
			level = ScenarioClass::Instance->DominatorLighting.Level;
		else if (NukeFlash::IsFadingIn())
			level = ScenarioClass::Instance->NukeLighting.Level;

		auto const pRulesExt = RulesExtData::Instance();
		int levelIntensity = 0;
		int cellIntensity = 1000;
		bool applyBridgeHeightBonus = pRulesExt->JumpjetCellLightApplyBridgeHeight ? TechnoExtData::IsOnBridge(pThis) : false;
		TechnoExtData::GetLevelIntensity(pThis, level, levelIntensity, cellIntensity, pRulesExt->JumpjetLevelLightMultiplier, pRulesExt->JumpjetCellLightLevelMultiplier, applyBridgeHeightBonus);

		R->EBP(levelIntensity + cellIntensity);
		return SkipGameCode;
	}

	return 0;
}