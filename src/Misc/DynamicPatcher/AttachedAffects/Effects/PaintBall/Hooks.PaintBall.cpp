#include "PaintBall.h"

#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x73C15F, UnitClass_DrawVXL_Colour, 0x7)
{
	GET(UnitClass* const, pOwnerObject, EBP);

	auto pExt = TechnoExt::ExtMap.Find(pOwnerObject);
	if (pExt->PaintBallState.get())
		pExt->PaintBallState->DrawVXL_Paintball(pOwnerObject, R, false);

	return 0;
}

//#826
//DEFINE_HOOK(0x0423420, AnimClass_Draw_ParentBuildingCheck, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	GET(BuildingClass*, pBuilding, EAX);
//
//	if (!pBuilding) {		
//		R->EAX(AnimExt::ExtMap.Find(pThis)->ParentBuilding);
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x423508, AnimClass_Draw_ForceShieldICColor, 0xB)
//{
//	enum { SkipGameCode = 0x423525 };
//	GET(AnimClass*, pThis, ESI);
//
//	auto const pBuilding = AnimExt::ExtMap.Find(pThis)->ParentBuilding;
//
//	const RulesClass* rules = RulesClass::Instance;
//
//	R->ECX(rules);
//	R->EAX(pBuilding->ForceShielded ? rules->ForceShieldColor : rules->IronCurtainColor);
//
//	return SkipGameCode;
//}
//
//DEFINE_HOOK(0x4235D3, AnimClass_Draw_TintColor, 0x6)
//{
//	GET(int, tintColor, EBP);
//	GET(AnimClass*, pThis, ESI);
//
//	auto const pBuilding = AnimExt::ExtMap.Find(pThis)->ParentBuilding;
//
//	if (!pBuilding)
//		return 0;
//
//	tintColor |= TechnoExt::GetCustomTintColor(pBuilding);
//
//	R->EBP(tintColor);
//
//	return 0;
//}

DEFINE_HOOK(0x423630, AnimClass_Draw_It, 0xC)
{
	GET(AnimClass*, pAnim, ESI);

	if (pAnim && pAnim->IsBuildingAnim)
	{
		auto const pCell = pAnim->GetCell();
		auto const pBuilding = pCell->GetBuilding();

		if (pBuilding && pBuilding->IsAlive && !pBuilding->Type->Invisible)
		{
			const auto pExt = TechnoExt::ExtMap.Find(pBuilding);
			if (pExt->PaintBallState)
			{
				pExt->PaintBallState->DrawSHP_Paintball_BuildAnim(pBuilding, R);
			}
		}
	}

	return 0;
}

//case VISUAL_NORMAL
DEFINE_HOOK(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass* const, pOwnerObject, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pOwnerObject);

	if (pExt->PaintBallState.get())
		pExt->PaintBallState->DrawSHP_Paintball(pOwnerObject, R);

	return 0;
}

DEFINE_HOOK(0x706640, TechnoClass_DrawVXL_Colour, 0x5)
{
	GET(TechnoClass* const, pOwnerObject, ECX);

	if (pOwnerObject->WhatAmI() == AbstractType::Building)
	{
		auto pExt = TechnoExt::ExtMap.Find(pOwnerObject);

		if (pExt->PaintBallState.get())
			pExt->PaintBallState->DrawVXL_Paintball(pOwnerObject, R, true);

	}

	return 0;
}