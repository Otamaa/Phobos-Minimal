#include "PaintBall.h"

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>
#include <Misc/PhobosGlobal.h>

#include <AirstrikeClass.h>
#include <InfantryClass.h>

ASMJIT_PATCH(0x43FA19, BuildingClass_Mark_TintIntensity, 0x7)
{
	GET(BuildingClass*, pThis, EDI);
	GET(int, intensity, ESI);

	TechnoExtData::ApplyCustomTint(pThis , nullptr , &intensity);
	R->ESI(intensity);

	return 0;
}

// ASMJIT_PATCH(0x43DC1C, BuildingClass_Draw2_TintColor, 0x6)
// {
// 	enum { SkipGameCode = 0x43DD8E };
//
// 	GET(BuildingClass*, pThis, EBP);
// 	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));
//
// 	color = TechnoExtData::ApplyTintColor(pThis, true, true, false);
// 	TechnoExtData::ApplyCustomTint(pThis , &color , nullptr);
//
// 	return SkipGameCode;
// }

ASMJIT_PATCH(0x706786, TechnoClass_DrawVoxel_TintColor, 0x5)
{
	enum { SkipTint = 0x7067E4 };

	GET(TechnoClass*, pThis, EBP);

	auto const rtti = pThis->WhatAmI();

	// Vehicles already have had tint intensity as well as custom tints applied, no need to do it twice.
	if (rtti == AbstractType::Unit)
		return SkipTint;

	GET(int, intensity, EAX);
	REF_STACK(int, color, STACK_OFFSET(0x50, 0x24));

	if (rtti == AbstractType::Aircraft)
		color = TechnoExtData::ApplyTintColor(pThis, true, false, false);

	// Non-aircraft voxels do not need custom tint color applied again, discard that component for them.
	TechnoExtData::ApplyCustomTint(pThis, rtti == AbstractType::Aircraft ? &color : nullptr, &intensity);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AirstrikeTargetingMe)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	R->EDI(intensity);
	return SkipTint;
}

namespace TechnoClass_DrawShapeTemp
{
	bool DisableTint = false;
}

ASMJIT_PATCH(0x7060D6, TechnoClass_DrawShape_DisguiseTint_SetContext, 0x5)
{
	TechnoClass_DrawShapeTemp::DisableTint = true;
	return 0;
}

ASMJIT_PATCH(0x70632E, TechnoClass_DrawShape_GetTintIntensity, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EAX);

	if (!TechnoClass_DrawShapeTemp::DisableTint)
	{
		if (pThis->IsIronCurtained())
			intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

		const auto pExt = TechnoExtContainer::Instance.Find(pThis);

		if (pExt->AirstrikeTargetingMe)
			intensity = pThis->GetAirstrikeTintIntensity(intensity);
	}

	R->EBP(intensity);
	return 0x706389;
}

ASMJIT_PATCH(0x706389, TechnoClass_DrawObject_TintColor, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, intensity, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, 0x2C));

	if (TechnoClass_DrawShapeTemp::DisableTint)
	{
		TechnoClass_DrawShapeTemp::DisableTint = false;
		return 0x7063E0;
	}

	const auto what = pThis->WhatAmI();
	const bool isVehicle = what == AbstractType::Unit;
	const bool isAircraft = what == AbstractType::Aircraft;

	if (isVehicle || isAircraft)
	{
		color |= TechnoExtData::ApplyTintColor(pThis, true, true, !isAircraft);
	}

	TechnoExtData::ApplyCustomTint(pThis, &color, &intensity);

	R->EBP(intensity);

	return 0;
}

ASMJIT_PATCH(0x423420, AnimClass_Draw_ParentBuildingCheck, 0x6)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);
	REF_STACK(int, color, STACK_OFFSET(0x110, -0xF4));
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	enum { SkipGameCode = 0x4235D3 };

	TechnoClass* pTechno = pBuilding;
	auto const pUnit = cast_to<UnitClass*>(pThis->OwnerObject);
	bool allowBerserkTint = false;

	if (pUnit && pUnit->DeployAnim == pThis)
	{
		pTechno = pUnit;
		allowBerserkTint = true;

		if (!pThis->Type->UseNormalLight)
			intensity = TechnoExtData::GetDeployingAnimIntensity(pUnit);
	}
	else if (!pTechno)
	{
		pTechno = pThis->_GetExtData()->ParentBuilding;
	}

	if (pTechno)
	{
		bool UseNormalLight = pThis->Type->UseNormalLight;

		if(auto pBld = cast_to<BuildingClass*>(pTechno)) {
			if ((pBld->CurrentMission == Mission::Construction)
				&& pBld->BState == BStateType::Construction && pBld->Type->Buildup)
			if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->BuildUp_UseNormalLIght.Get())
				UseNormalLight = true;
		}

		TechnoExtData::ApplyTintColor(pTechno,true , true , allowBerserkTint);
		TechnoExtData::ApplyCustomTint(pTechno, &color, !UseNormalLight ? &intensity : nullptr);
	}

	R->EBP(color);
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

ASMJIT_PATCH(0x518FC8, InfantryClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x519082 };

	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color = TechnoExtData::ApplyTintColor(pThis, true, true, true);
	TechnoExtData::ApplyCustomTint(pThis, &color, nullptr);

	return SkipGameCode;
}

ASMJIT_PATCH(0x73BF95, UnitClass_DrawAsVoxel_Tint, 0x7)
{
	enum { SkipGameCode = 0x73C141 };

	GET(UnitClass*, pThis, EBP);
	GET(int, flashIntensity, ESI);
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

/*
ASMJIT_PATCH(0x43D442, BuildingClass_Draw_ForceShieldICColor, 0x7)
{
	enum { SkipGameCode = 0x43D45B };

	GET(BuildingClass*, pThis, ESI);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

ASMJIT_PATCH(0x43D396, BuildingClass_Draw_LaserTargetColor, 0x6) {
	GET(BuildingClass*, pThis, ESI);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Airstrike->Owner->GetTechnoType());
	const ColorStruct clr = GeneralUtils::GetColorStructFromColorAdd(pTypeExt->LaserTargetColor.isset() ? pTypeExt->LaserTargetColor : RulesClass::Instance->LaserTargetColor);

	R->BL(clr.G);
	R->Stack8(0x11 , clr.R);
	R->Stack8(0x12, clr.B);
	return 0x43D3CA;
}

ASMJIT_PATCH(0x42343C, AnimClass_Draw_Airstrike, 0x6)
{
	GET(BuildingClass*, pBld, ECX);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Airstrike->Owner->GetTechnoType());
	const ColorStruct clr = GeneralUtils::GetColorStructFromColorAdd(pTypeExt->LaserTargetColor.isset() ? pTypeExt->LaserTargetColor : RulesClass::Instance->LaserTargetColor);

	R->Stack8(0x1B, clr.R);
	R->Stack8(0x1A, clr.G);
	R->Stack8(0x13, clr.B);
	return 0x42347B;
}

ASMJIT_PATCH(0x43DC30, BuildingClass_DrawFogged_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x43DC3C };

	GET(BuildingClass*, pThis, EBP);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Airstrike->Owner->GetTechnoType());
	const ColorStruct clr = GeneralUtils::GetColorStructFromColorAdd(pTypeExt->LaserTargetColor.isset() ? pTypeExt->LaserTargetColor : RulesClass::Instance->LaserTargetColor);

	R->BL(clr.G);
	R->Stack8(0x13, clr.R);
	R->Stack8(0x12, clr.B);
	return 0x43DC64;
}

ASMJIT_PATCH(0x43DCE1, BuildingClass_Draw2_ForceShieldICColor, 0x7)
{
	enum { SkipGameCode = 0x43DCFA };

	GET(BuildingClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

ASMJIT_PATCH(0x43D4EB, BuildingClass_Draw_TintColor, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, color, EDI);

	ApplyCustomTint(pThis, &color, nullptr);

	R->EDI(color);

	return 0;
}

ASMJIT_PATCH(0x43DD8E, BuildingClass_Draw2_TintColor, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x12C, -0x110));

	ApplyCustomTint(pThis, &color, nullptr);

	return 0;
}


ASMJIT_PATCH(0x519082, InfantryClass_Draw_TintColor, 0x7)
{
	GET(InfantryClass*, pThis, EBP);
	REF_STACK(int, color, STACK_OFFSET(0x54, -0x40));

	color |= ApplyTintColor(pThis, true, false, false);
	ApplyCustomTint(pThis, &color, nullptr);

	return 0;
}


ASMJIT_PATCH(0x73BFBF, UnitClass_DrawAsVoxel_ForceShieldICColor, 0x6)
{
	enum { SkipGameCode = 0x73BFC5 };

	GET(UnitClass*, pThis, EBP);

	RulesClass* rules = RulesClass::Instance;

	R->EAX(pThis->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}

ASMJIT_PATCH(0x73C083, UnitClass_DrawAsVoxel_TintColor, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(int, color, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x1D0, 0x10));

	ApplyCustomTint(pThis, &color, &intensity);

	R->ESI(color);

	return 0;
}

ASMJIT_PATCH(0x42350C, AnimClass_Draw_ForceShieldICColor, 0x7)
{
	enum { SkipGameCode = 0x423525 };

	GET(BuildingClass*, pBuilding, ECX);

	RulesClass* rules = RulesClass::Instance;

	R->ECX(rules);
	R->EAX(pBuilding->ProtectType == ProtectTypes::ForceShield ?
		rules->ForceShieldColor : rules->IronCurtainColor);

	return SkipGameCode;
}


ASMJIT_PATCH(0x4235D3, AnimClass_Draw_TintColor, 0x6)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET(int, color, EBP);
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));

	auto const pBuilding = pThis->_GetExtData()->ParentBuilding;

	if (!pBuilding || !pBuilding->IsAlive || pBuilding->InLimbo)
		return 0;

	ApplyCustomTint(pBuilding, &color, pThis->Type->UseNormalLight ? &intensity : nullptr);

	R->EBP(color);

	return 0;
}*/
/*
*
* ASMJIT_PATCH(0x73C15F, UnitClass_DrawVXL_Colour, 0x7)
{
	GET(UnitClass* const, pOwnerObject, EBP);

	if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
		pPaintBall->DrawVXL_Paintball(pOwnerObject, R, false);

	return 0;
}
//case VISUAL_NORMAL
ASMJIT_PATCH(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass* const, pOwnerObject, ESI);

	if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
		pPaintBall->DrawSHP_Paintball(pOwnerObject, R);

	return 0;
}

ASMJIT_PATCH(0x706640, TechnoClass_DrawVXL_Colour, 0x5)
{
	GET(TechnoClass* const, pOwnerObject, ECX);

	if (pOwnerObject->WhatAmI() == BuildingClass::AbsID)
	{
		if (auto& pPaintBall = TechnoExtContainer::Instance.Find(pOwnerObject)->PaintBallState)
			pPaintBall->DrawVXL_Paintball(pOwnerObject, R, true);
	}

	return 0;
}

ASMJIT_PATCH(0x423630, AnimClass_Draw_It, 0x6)
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