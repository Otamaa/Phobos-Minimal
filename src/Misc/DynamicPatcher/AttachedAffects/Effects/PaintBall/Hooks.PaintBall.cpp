#include "PaintBall.h"

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>


// Applies custom tint color and intensity from TechnoTypes & Warheads on provided values.
void ApplyCustomIntensity(TechnoClass* pThis,int& intensity)
{
	BuildingClass* pBld = specific_cast<BuildingClass*>(pThis);

	if (pBld) {
		if ((pBld->CurrentMission == Mission::Construction)
			&& pBld->BState == BStateType::Construction && pBld->Type->Buildup) {
			if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->BuildUp_UseNormalLIght.Get())
			{
				intensity = 1000;
			}
		}
	}
	const bool bInf = pThis->WhatAmI() == InfantryClass::AbsID;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	// EMP
	if (pThis->IsUnderEMP())
	{
		if (!bInf || pTypeExt->Infantry_DimWhenEMPEd.Get(((InfantryTypeClass*)(pTypeExt->AttachedToObject))->Cyborg))
		{
			intensity /= 2 ;
		}
	}
	else if (pThis->IsDeactivated())
	{
		if (!bInf || pTypeExt->Infantry_DimWhenDisabled.Get(((InfantryTypeClass*)(pTypeExt->AttachedToObject))->Cyborg))
		{
			intensity /= 2;
		}
	}
}

// Gets tint colors for invulnerability, airstrike laser target and berserk, depending on parameters.
int ApplyTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk)
{
	int tintColor = 0;

	if (invulnerability && pThis->IsIronCurtained())
			tintColor |= GeneralUtils::GetColorFromColorAdd(pThis->ProtectType == ProtectTypes::ForceShield ? RulesClass::Instance->ForceShieldColor : RulesClass::Instance->IronCurtainColor);
	if (airstrike && pThis->Airstrike && pThis->Airstrike->Target == pThis)
			tintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->LaserTargetColor);
	if (berserk && pThis->Berzerk)
			tintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->BerserkColor);

	return tintColor;
}

void ApplyCustomTintcolor(TechnoClass* pThis, int& tintColor) {
	for (auto& paint : TechnoExtContainer::Instance.Find(pThis)->PaintBallStates) {
		if (paint.second.timer.GetTimeLeft() && paint.second.AllowDraw(pThis)) {
			tintColor |= paint.second.Color;
		}
	}
}

DEFINE_HOOK(0x706389, TechnoClass_DrawObject_TintColor, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, 0x2C));

	const auto what = pThis->WhatAmI();
	const bool isVehicle = what == AbstractType::Unit;
	const bool isAircraft = what == AbstractType::Aircraft;

	ApplyCustomIntensity(pThis, intensity);

	// SHP vehicles and aircraft
	if (isVehicle || isAircraft) {
		color |= ApplyTintColor(pThis, true, false, !isAircraft);

		ApplyCustomTintcolor(pThis, color);
	}

	R->EBP(intensity);

	return 0;
}

DEFINE_HOOK(0x7067E4, TechnoClass_DrawVoxel_TintColor, 0x8)
{
	GET(TechnoClass*, pThis, EBP);
	GET(int, intensity, EDI);
	REF_STACK(int, color, STACK_OFFSET(0x50, 0x24));

	color |= ApplyTintColor(pThis, true, false, true);
	R->EDI(intensity);

	return 0;
}

DEFINE_HOOK(0x43D442, BuildingClass_Draw_ForceShieldICColor, 0x7)
{
	enum { SkipGameCode = 0x43D45B };

	GET(BuildingClass*, pThis, ESI);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x43DCE1, BuildingClass_Draw2_ForceShieldICColor, 0x7)
{
	enum { SkipGameCode = 0x43DCFA };

	GET(BuildingClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x43D4EB, BuildingClass_Draw_TintColor, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, color, EDI);

	ApplyCustomTintcolor(pThis , color);

	R->EDI(color);

	return 0;
}

DEFINE_HOOK(0x43DD8E, BuildingClass_Draw2_TintColor, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));

	ApplyCustomTintcolor(pThis, color);

	return 0;
}

DEFINE_HOOK(0x43FA19, BuildingClass_Mark_TintIntensity, 0x7)
{
	GET(BuildingClass*, pThis, EDI);
	GET(int, intensity, ESI);

	ApplyCustomIntensity(pThis , intensity);
	R->ESI(intensity);

	return 0;
}

DEFINE_HOOK(0x519082, InfantryClass_Draw_TintColor, 0x7)
{
	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color |= ApplyTintColor(pThis, true, false, false);
	ApplyCustomTintcolor(pThis , color);

	return 0;
}

DEFINE_HOOK(0x51946D, InfantryClass_Draw_TintIntensity, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, intensity, ESI);

	intensity = pThis->GetEffectTintIntensity(intensity);
	ApplyCustomIntensity(pThis , intensity);
	R->ESI(intensity);

	return 0;
}

DEFINE_HOOK(0x73BFBF, UnitClass_DrawAsVoxel_ForceShieldICColor, 0x6)
{
	enum { SkipGameCode = 0x73BFC5 };

	GET(UnitClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x73C083, UnitClass_DrawAsVoxel_TintColor, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(int, color, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x1D0, 0x10));

	ApplyCustomIntensity(pThis, intensity);
	ApplyCustomTintcolor(pThis, color);

	R->ESI(color);

	return 0;
}

DEFINE_HOOK(0x423519, AnimClass_Draw_ForceShieldICColor, 0x6)
{
	enum { SkipGameCode = 0x423525 };

	GET(BuildingClass*, pBuilding, ECX);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pBuilding->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

DEFINE_HOOK(0x0423420, AnimClass_Draw_ParentBuildingCheck, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);

	if (!pBuilding)
		R->EAX(AnimExtContainer::Instance.Find(pThis)->ParentBuilding);

	return 0;
}

DEFINE_HOOK(0x4235D3, AnimClass_Draw_TintColor, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(int, color, EBP);
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	auto const pBuilding = AnimExtContainer::Instance.Find(pThis)->ParentBuilding;

	if (!pBuilding)
		return 0;

	ApplyCustomTintcolor(pBuilding, color);
	if(pThis->Type->UseNormalLight)
		ApplyCustomIntensity(pBuilding, intensity);

	R->EBP(color);

	return 0;
}
/*
*
* DEFINE_HOOK(0x73C15F, UnitClass_DrawVXL_Colour, 0x7)
{
	GET(UnitClass* const, pOwnerObject, EBP);

	if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
		pPaintBall->DrawVXL_Paintball(pOwnerObject, R, false);

	return 0;
}
//case VISUAL_NORMAL
DEFINE_HOOK(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass* const, pOwnerObject, ESI);

	if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
		pPaintBall->DrawSHP_Paintball(pOwnerObject, R);

	return 0;
}

DEFINE_HOOK(0x706640, TechnoClass_DrawVXL_Colour, 0x5)
{
	GET(TechnoClass* const, pOwnerObject, ECX);

	if (pOwnerObject->WhatAmI() == BuildingClass::AbsID)
	{
		if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
			pPaintBall->DrawVXL_Paintball(pOwnerObject, R, true);
	}

	return 0;
}

DEFINE_HOOK(0x423630, AnimClass_Draw_It, 0x6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(CellClass*, pCell, EAX);

	if (pAnim && pAnim->IsBuildingAnim)
	{
		auto const pBuilding = AnimExtContainer::Instance.Find(pAnim)->ParentBuilding;

		if (pBuilding && pBuilding->IsAlive && !pBuilding->Type->Invisible) {
			if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pBuilding)->PaintBallStates) {
				pPaintBall->DrawSHP_Paintball_BuildAnim(pBuilding, R);
			}
		}
	}

	return 0;
}
*/