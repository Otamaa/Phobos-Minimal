#include "Body.h"
#include <Utilities/Cast.h>

DEFINE_HOOK_AGAIN(0x522790, TechnoClass_DefaultDisguise, 0x6) // InfantryClass_SetDisguise_DefaultDisguise
DEFINE_HOOK(0x6F421C, TechnoClass_DefaultDisguise, 0x6) // TechnoClass_Init_DefaultDisguise
{
	GET(TechnoClass*, pThis, ESI);

	enum { SetDisguise = 0x5227BF, DefaultDisguise = 0x6F4277 };

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		//ToDo:
#ifdef TANK_DISGUISE
		if ((R->Origin() == 0x6F421C) && pThis->WhatAmI() == AbstractType::Unit && pExt->TankDisguiseAsTank.Get())
		{
			pThis->Disguised = false;
			return DefaultDisguise;
		}
#endif
		if (pExt->DefaultDisguise.isset())
		{
			pThis->Disguise = pExt->DefaultDisguise;
			pThis->Disguised = true;
			return R->Origin() == 0x522790 ? SetDisguise : DefaultDisguise;
		}
	}

	pThis->Disguised = false;

	return 0;
}

#ifdef ENABLE_NEWHOOKS
#define CAN_BLINK_DISGUISE(pTechno) \
RulesExt::Global()->ShowAllyDisguiseBlinking && (HouseClass::IsPlayerObserver() || (pTechno->Owner ? pTechno->Owner->IsAlliedWith(HouseClass::Player):true))

DEFINE_HOOK(0x70EE53, TechnoClass_IsClearlyVisibleTo_BlinkAllyDisguise1, 0xA)
{
	enum { SkipGameCode = 0x70EE6A, Return = 0x70EEEC };

	GET(TechnoClass*, pThis, ESI);
	GET(int, accum, EAX);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipGameCode;
	else if (accum && (pThis->Owner  ? !pThis->Owner->ControlledByPlayer():false))
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
#endif

DEFINE_HOOK(0x7060A9, TechnoClass_TechnoClass_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	TechnoTypeClass* pType = nullptr;
	const auto pDisguise = type_cast<TechnoTypeClass*>(pThis->Disguise);
	LightConvertClass* pConvert = nullptr;
	int nColorIdx = -1;

	if (pThis->IsDisguised() && pDisguise) {
		if (const auto pHouse = pThis->GetDisguiseHouse(true))
			nColorIdx = pHouse->ColorSchemeIndex;

		pType = pDisguise;
	} else {
		nColorIdx = pThis->GetOwningHouse()->ColorSchemeIndex;
		pType = pThis->GetTechnoType();
	}

	if (pType && pType->Palette && pType->Palette->Count > 0)
		pConvert = pType->Palette->GetItem(nColorIdx)->LightConvert;
	else
		pConvert = ColorScheme::Array->GetItem(nColorIdx)->LightConvert;

	R->EBX(pConvert);

	return SkipGameCode;
}

#pragma region Otamaa
#ifdef Advanched_DISGUISE
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

#include <TerrainTypeClass.h>

DEFINE_HOOK(0x7466D8, UnitClass_DesguiseAs_AsAnotherUnit, 0xA)
{
	GET(UnitClass*, pTarget, ESI);
	GET(UnitClass*, pThis, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const& nData = pTypeExt->AnotherData;

	if (pTarget->IsDisguised() || !nData.TankDisguiseAsTank.Get())
		return 0x0;

	pThis->Disguise = pTarget->Type;
	pThis->DisguisedAsHouse = pTarget->GetOwningHouse();

	return 0x746712;
}

static bool Allowed(const Otamaa::TTyExt::ExtDataB& pThis, TechnoTypeClass* pThat)
{
	if (pThis.DisguiseDisAllowed.empty())
		return true;

	if (!pThis.DisguiseDisAllowed.Contains(pThat))
		return true;

	return false;
}

DEFINE_HOOK(0x746670, UnitClass_DisguiseAs_Override, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);

	if (!pTarget || pTarget->WhatAmI() == AbstractType::Infantry)
		return 0x746714;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const& nData = pExt->AnotherData;

	if (!pTarget->IsDisguised())
	{
		if (!nData.TankDisguiseAsTank.Get())
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
				if (Allowed(nData, pUnit->Type))
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
			if (Allowed(nData, pUnit->Type))
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

static void __fastcall UnitClass_DisguiseAI_(UnitClass* pThis, void* _)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const& nData = pExt->AnotherData;

	if (!nData.TankDisguiseAsTank.Get())
		pThis->UpdateDisguise();
}

DEFINE_JUMP(CALL,0x73649C, GET_OFFSET(UnitClass_DisguiseAI_));


DEFINE_HOOK(0x746A30, UnitClass_Disguise_AI_UnitAsUnit, 0xB)
{
	GET(UnitClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const& nData = pTypeExt->AnotherData;

	if (!nData.TankDisguiseAsTank.Get())
		return 0x0;

	R->EAX(pThis->Type);
	return 0x746A6C;
}

//522700
DEFINE_HOOK(0x522718, InfantryClass_DisguiseAs_Allowed, 0x5)
{
	GET(InfantryClass*, pThis, EDI);
	GET(InfantryClass*, pThat, ESI);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const& nData = pExt->AnotherData;

	return Allowed(nData, pThat->Type) ? 0x0 : 0x522772;
}
#endif
#pragma endregion