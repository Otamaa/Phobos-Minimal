#include "Body.h"
#include <Ext/TechnoType/Body.h>

#include <Utilities/Cast.h>

DEFINE_HOOK_AGAIN(0x522790, TechnoClass_DefaultDisguise, 0x6) // InfantryClass_SetDisguise_DefaultDisguise
DEFINE_HOOK(0x6F421C, TechnoClass_DefaultDisguise, 0x6) // TechnoClass_Init_DefaultDisguise
{
	enum { InfInit_SetDisguise = 0x5227BF, TechnoInit_SetDisguise = 0x6F4277 };

	GET(TechnoTypeClass*, pType, EAX);
	GET(TechnoClass*, pThis, ESI);

	if(R->Origin() == 0x6F421C && pThis->WhatAmI() != AbstractType::Infantry)
		return 0x0;

	if (!pType)
		return 0x0;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	//ToDo:
#ifdef TANK_DISGUISE
	if ((R->Origin() == 0x6F421C) && pThis->WhatAmI() == AbstractType::Unit && pExt->TankDisguiseAsTank.Get())
	{
		pThis->Disguised = false;
		return TechnoInit_SetDisguise;
	}
#endif

	if (auto const pDisguise = pExt->DefaultDisguise.Get(nullptr))
	{
		pThis->Disguise = pDisguise;
		pThis->Disguised = true;
		return R->Origin() == 0x522790 ? InfInit_SetDisguise : TechnoInit_SetDisguise;
	}

	return 0x0;
}

#ifdef ENABLE_NEWHOOKS
//TODO : rework , and desync test
#define CAN_BLINK_DISGUISE(pTechno) \
RulesExt::Global()->ShowAllyDisguiseBlinking && (HouseExt::IsObserverPlayer() || (pTechno->Owner ? pTechno->Owner->IsAlliedWith(HouseClass::CurrentPlayer):true))

DEFINE_HOOK(0x70EE53, TechnoClass_IsClearlyVisibleTo_BlinkAllyDisguise1, 0xA)
{
	enum { SkipGameCode = 0x70EE6A, Return = 0x70EEEC };

	GET(TechnoClass*, pThis, ESI);
	GET(int, accum, EAX);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipGameCode;
	else if (accum && (pThis->Owner ? !pThis->Owner->ControlledByPlayer() : false))
		return Return;

	return SkipGameCode;
}

DEFINE_HOOK(0x70EE6A, TechnoClass_IsClearlyVisibleTo_BlinkAllyDisguise2, 0x6)
{
	enum { SkipCheck = 0x70EE79 };

	GET(TechnoClass*, pThis, ESI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}

DEFINE_HOOK(0x7062F5, TechnoClass_TechnoClass_DrawObject_BlinkAllyDisguise, 0x6)
{
	enum { SkipCheck = 0x706304 };

	GET(TechnoClass*, pThis, ESI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}

DEFINE_HOOK(0x70EDAD, TechnoClass_DisguiseBlitFlags_BlinkAllyDisguise, 0x6)
{
	enum { SkipCheck = 0x70EDBC };

	GET(TechnoClass*, pThis, EDI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}
#undef CAN_BLINK_DISGUISE


DEFINE_HOOK(0x7060A9, TechnoClass_TechnoClass_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	auto const& [pType, pOwner] = TechnoExt::GetDisguiseType<true, true>(pThis);
	LightConvertClass* pConvert = nullptr;
	int nColorIdx = pOwner->ColorSchemeIndex;

	if (pType->Palette && pType->Palette->Count > 0)
		pConvert = pType->Palette->GetItem(nColorIdx)->LightConvert;
	else
		pConvert = ColorScheme::Array->GetItem(nColorIdx)->LightConvert;

	R->EBX(pConvert);

	return SkipGameCode;
}

DEFINE_HOOK(0x705D88, TechnoClass_GetRemapColor_CheckVector, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(DynamicVectorClass<ColorScheme*>*, pTypePal, EAX);

	return pTypePal && pTypePal->Count > 0 ? 0x705D92 : 0x705D92;
}


#pragma region Otamaa

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

#include <TerrainTypeClass.h>

//DEFINE_HOOK(0x7466D8, UnitClass_DesguiseAs_AsAnotherUnit, 0xA)
//{
//	GET(UnitClass*, pTarget, ESI);
//	GET(UnitClass*, pThis, EDI);
//
//	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pTarget->IsDisguised() || !pTypeExt->TankDisguiseAsTank.Get())
//		return 0x0;
//
//	pThis->Disguise = pTarget->Type;
//	pThis->DisguisedAsHouse = pTarget->GetOwningHouse();
//
//	return 0x746712;
//}


static bool Allowed(const TechnoTypeExt::ExtData* pThis, TechnoTypeClass* pThat)
{
	if (pThis->DisguiseDisAllowed.empty())
		return true;

	if (!pThis->DisguiseDisAllowed.Contains(pThat))
		return true;

	return false;
}
//
//static void __fastcall UnitClass_DisguiseAs_(UnitClass* pThis, ObjectClass* pTarget)
//{
//	if (!pTarget || pTarget->WhatAmI() == AbstractType::Infantry)
//		return;
//
//	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
//	if (!pTarget->IsDisguised())
//	{
//		if (!pExt->TankDisguiseAsTank.Get())
//		{
//			if (pThis->CanDisguiseAs(pTarget))
//			{
//				if (const auto pOverlay = specific_cast<OverlayClass*>(pTarget))
//				{
//					pThis->Disguise = pOverlay->Type;
//					pThis->DisguisedAsHouse = nullptr;
//					pThis->Techno_70E280(pTarget);
//					return;
//				}
//
//				if (const auto pTerrain = specific_cast<TerrainClass*>(pTarget))
//				{
//					pThis->Disguise = pTerrain->Type;
//					pThis->DisguisedAsHouse = nullptr;
//					pThis->Techno_70E280(pTarget);
//					return;
//				}
//			}
//		}
//		else
//		{
//			if (const auto pUnit = specific_cast<UnitClass*>(pTarget))
//			{
//				if (Allowed(pExt, pUnit->Type))
//				{
//					pThis->Disguise = pUnit->Type;
//					pThis->DisguisedAsHouse = pUnit->GetOwningHouse();
//					pThis->Techno_70E280(pTarget);
//					return;
//				}
//			}
//		}
//	}
//	else
//	{
//		if (const auto pUnit = specific_cast<UnitClass*>(pTarget))
//		{
//			if (Allowed(pExt, pUnit->Type))
//			{
//				if (pUnit->Owner)
//				{
//					if (pUnit->Owner->IsAlliedWith(pThis->Owner))
//					{
//						pThis->Disguise = pUnit->Type;
//						pThis->DisguisedAsHouse = pUnit->GetOwningHouse();
//						pThis->Techno_70E280(pTarget);
//						return;
//					}
//				}
//
//				pThis->Disguise = pUnit->Disguise;
//				pThis->DisguisedAsHouse = pUnit->DisguisedAsHouse;
//				pThis->Techno_70E280(pTarget);
//				return;
//			}
//		}
//		else
//		{
//			pThis->Disguise = pTarget->GetDisguise(true);
//			pThis->DisguisedAsHouse = pTarget->GetDisguiseHouse(true);
//			pThis->Techno_70E280(pTarget);
//		}
//
//	}
//}
//DEFINE_JUMP(VTABLE, 0x7F60DC, GET_OFFSET(UnitClass_DisguiseAs_));

DEFINE_HOOK(0x746670, UnitClass_DisguiseAs_Override, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);

	//pThis Can become nullptr , wtf ?
	if (!pThis || !pTarget || pTarget->WhatAmI() == AbstractType::Infantry)
		return 0x746714;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTarget->IsDisguised())
	{
		if (!pExt->TankDisguiseAsTank.Get())
		{
			if (pThis->CanDisguiseAs(pTarget))
			{
				if (const auto pOverlay = specific_cast<OverlayClass*>(pTarget))
				{
					pThis->Disguise = pOverlay->Type;
					pThis->DisguisedAsHouse = nullptr;
					pThis->Techno_70E280(pTarget);
					return 0x746714;
				}

				if (const auto pTerrain = specific_cast<TerrainClass*>(pTarget))
				{
					pThis->Disguise = pTerrain->Type;
					pThis->DisguisedAsHouse = nullptr;
					pThis->Techno_70E280(pTarget);
					return 0x746714;
				}
			}
		}
		else
		{
			if (const auto pUnit = specific_cast<UnitClass*>(pTarget))
			{
				if (Allowed(pExt, pUnit->Type))
				{
					pThis->Disguise = pUnit->Type;
					pThis->DisguisedAsHouse = pUnit->GetOwningHouse();
					pThis->Techno_70E280(pTarget);
					return 0x746714;
				}
			}
		}
	}
	else
	{
		if (const auto pUnit = specific_cast<UnitClass*>(pTarget))
		{
			if (Allowed(pExt, pUnit->Type))
			{
				if (pUnit->Owner)
				{
					if (pUnit->Owner->IsAlliedWith(pThis->Owner))
					{
						pThis->Disguise = pUnit->Type;
						pThis->DisguisedAsHouse = pUnit->GetOwningHouse();
						pThis->Techno_70E280(pTarget);
						return 0x746714;
					}
				}

				pThis->Disguise = pUnit->Disguise;
				pThis->DisguisedAsHouse = pUnit->DisguisedAsHouse;
				pThis->Techno_70E280(pTarget);
				return 0x746714;
			}
		}
		else
		{
			pThis->Disguise = pTarget->GetDisguise(true);
			pThis->DisguisedAsHouse = pTarget->GetDisguiseHouse(true);
			pThis->Techno_70E280(pTarget);
		}

	}

	return 0x746714;
}

DEFINE_HOOK(0x73649A, UnitClass_AI_DisguiseAI, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt->TankDisguiseAsTank.Get())
		pThis->UpdateDisguise(); //this one updating the disguise blink and stuffs


	return 0x7364A1;
}

DEFINE_HOOK(0x746A30, UnitClass_Disguise_AI_UnitAsUnit, 0x5)
{
	GET(UnitClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TankDisguiseAsTank.Get())
		return 0x0;

	R->EAX(pThis->Type);
	return 0x746A6C;
}

DEFINE_HOOK(0x522718, InfantryClass_DisguiseAs_Allowed, 0x8)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	if (Allowed(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()), pThat->GetTechnoType()))
	{
		pThis->Techno_70E280(pThat);
		return 0x522720;
	}

	return 0x522772;
}

#pragma endregion
#endif