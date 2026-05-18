#include "Body.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Anim/Body.h>

#include <Locomotor/Cast.h>

#include <SuperClass.h>
#include <TechnoClass.h>

ASMJIT_PATCH(0x4148F4, AircraftClass_DrawIt_LevelIntensity, 0x5)
{

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

	return 0x414925;
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
		R->EBP(TechnoExtData::GetJumpjetIntensity(pThis));
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x423420, AnimClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x4235D3 };

	GET(AnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);
	REF_STACK(int, color, STACK_OFFSET(0x110, -0xF4));
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	TechnoClass* pTechno = pBuilding;
	auto const pUnit = cast_to<UnitClass*>(pThis->OwnerObject);
	bool allowBerserkTint = false;

	if (pUnit && pUnit->DeployAnim == pThis) {
		pTechno = pUnit;
		allowBerserkTint = true;

		if (!pThis->Type->UseNormalLight)
			intensity = TechnoExtData::GetDeployingAnimIntensity(pUnit);
	} else if (!pTechno) {
		pTechno = AnimExtContainer::Instance.Find(pThis)->ParentBuilding;
	}

	if (pTechno) {
		color |= TechnoExtData::ApplyTintColor(pTechno, true, true, allowBerserkTint);
		TechnoExtData::ApplyCustomTint(pTechno, &color, (pThis->Type->UseNormalLight ? nullptr : &intensity));
	}

	R->EBP(color);
	return SkipGameCode;
}

ASMJIT_PATCH(0x43DC1C, BuildingClass_Draw_Fogged_TintColor, 0x6)
{
	enum { SkipGameCode = 0x43DD8E };

	GET(BuildingClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));

	color = TechnoExtData::ApplyTintColor(pThis, true, true, false);
	TechnoExtData::ApplyCustomTint(pThis,&color , nullptr);

	return SkipGameCode;
}

ASMJIT_PATCH(0x73BF95, UnitClass_DrawAsVoxel_Tint, 0x7)
{
	enum { SkipGameCode = 0x73C141 };

	GET(UnitClass*, pThis, EBP);
	GET(const int, flashIntensity, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x1D0, 0x10));

	intensity = flashIntensity;

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	int color = TechnoExtData::ApplyTintColor(pThis, true, true, true);
	TechnoExtData::ApplyCustomTint(pThis, &color, &intensity);

	R->ESI(color);
	return SkipGameCode;
}

ASMJIT_PATCH(0x518FC8, InfantryClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x519082 };

	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color = 0;
	color |= TechnoExtData::ApplyTintColor(pThis, true, true, true);
	TechnoExtData::ApplyCustomTint(pThis, &color, nullptr);

	return SkipGameCode;
}

ASMJIT_PATCH(0x51946D, InfantryClass_Draw_TintIntensity, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, intensity, ESI);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	TechnoExtData::ApplyCustomTint(pThis, nullptr, &intensity);
	R->ESI(intensity);

	return 0;
}
