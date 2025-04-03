#include "Body.h"
#include <Ext/TechnoType/Body.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>

ASMJIT_PATCH(0x6F421C, TechnoClass_Init_PermaDisguise_DefaultDisguise, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pThis->WhatAmI() == UnitClass::AbsID && pExt->TankDisguiseAsTank.Get())
	{
		pThis->Disguised = false;
		pThis->DisguisedAsHouse = nullptr;
		pThis->Disguise = nullptr;
		return 0x6F424B;
	}

	return 0;
}

ASMJIT_PATCH(0x7466D8, UnitClass_DesguiseAs_AsAnotherUnit, 0xA)
{
	GET(AbstractClass*, pTarget, ESI);
	GET(UnitClass*, pThis, EDI);

	auto const pObjectT = flag_cast_to<ObjectClass*>(pTarget);

	if (!pObjectT || pObjectT->IsDisguised())
		return 0x0;

	if (pObjectT->WhatAmI() != UnitClass::AbsID || !TechnoTypeExtContainer::Instance.Find(pThis->Type)
		->TankDisguiseAsTank.Get())
		return 0x0;

	pThis->Disguise = static_cast<UnitClass*>(pTarget)->Type;
	pThis->DisguisedAsHouse = pTarget->GetOwningHouse();

	return 0x746712;
}

bool __fastcall IsAlly_Wrapper(HouseClass* pThis, void* _, HouseClass* pOther)
{
	return pOther->IsObserver() || pOther->IsAlliedWith(pThis) || (RulesExtData::Instance()->DisguiseBlinkingVisibility & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool __fastcall IsControlledByCurrentPlayer_Wrapper(HouseClass* pThis)
{
	HouseClass* pCurrent = HouseClass::CurrentPlayer;
	AffectedHouse visibilityFlags = RulesExtData::Instance()->DisguiseBlinkingVisibility;

	if (SessionClass::IsCampaign() && (pThis->IsHumanPlayer || pThis->IsInPlayerControl))
	{
		if ((visibilityFlags & AffectedHouse::Allies) != AffectedHouse::None && pCurrent->IsAlliedWith(pThis))
			return true;

		return (visibilityFlags & AffectedHouse::Owner) != AffectedHouse::None;
	}

	return pCurrent->IsObserver() || EnumFunctions::CanTargetHouse(visibilityFlags, pCurrent, pThis);
}

DEFINE_FUNCTION_JUMP(CALL, 0x4DEDD2, IsAlly_Wrapper);                      // FootClass_GetImage
DEFINE_FUNCTION_JUMP(CALL, 0x70EE5D, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_ClearlyVisibleTo
DEFINE_FUNCTION_JUMP(CALL, 0x70EE70, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_ClearlyVisibleTo
DEFINE_FUNCTION_JUMP(CALL, 0x7062FB, IsControlledByCurrentPlayer_Wrapper); // TechnoClass_DrawObject

#ifdef ENABLE_OBESERVER_THRUDISGUISE
ASMJIT_PATCH(0x746750, UnitClass_CantTarget_Disguise, 0x5)
{
	if (HouseClass::IsCurrentPlayerObserver()) {
		R->AL(true);
		return 0x7467FB
	}

	return  0x0;
}

bool CanBlinkDisguise(TechnoClass* pTechno , HouseClass* pCurPlayer)
{
	if(pCurPlayer && !pCurPlayer->IsObserver()) {
		return  EnumFunctions::CanTargetHouse(
			RulesExtData::Instance()->DisguiseBlinkingVisibility,pTechno->Owner, pCurPlayer);
	}

	return true;
}

ASMJIT_PATCH(0x4DEDCB, FootClass_GetImage_DisguiseBlinking, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(HouseClass* , pCurPlayer , ECX);
	//GET(CellClass*, pCell, EDI);

	R->EAX(CanBlinkDisguise(pThis, pCurPlayer));
	return 0x4DEDD7;
}

ASMJIT_PATCH(0x70EE6A, TechnoClass_IsClearlyVisibleTo_DisguiseBlinking, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	R->EAX(CanBlinkDisguise(pThis, HouseClass::CurrentPlayer()));
	return 0x70EE75;
}

ASMJIT_PATCH(0x7062F5, TechnoClass_TechnoClass_DrawObject_DisguiseBlinking, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	R->EAX(CanBlinkDisguise(pThis, HouseClass::CurrentPlayer()));
	return 0x706300;
}

#undef CAN_BLINK_DISGUISE

ASMJIT_PATCH(0x7060A9, TechnoClass_TechnoClass_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	auto const& [pType, pOwner] = TechnoExtData::GetDisguiseType(pThis, true, true);
	LightConvertClass* pConvert = nullptr;

	if(pOwner) {
		if (pType->Palette && pType->Palette->Count > 0)
			pConvert = pType->Palette->Items[pOwner->ColorSchemeIndex]->LightConvert;
		else
			pConvert = ColorScheme::Array->Items[pOwner->ColorSchemeIndex]->LightConvert;
	}

	R->EBX(pConvert);

	return SkipGameCode;
}

// somewhat crash the game when called
// maybe already hooked by something else ?
//ASMJIT_PATCH(0x705D88, TechnoClass_GetRemapColor_CheckVector, 0x8)
//{
//	GET(DynamicVectorClass<ConvertClass*>*, pPal, EAX);
//	GET(TechnoClass*, pThis, ESI);
//
//	int nColorIdx = 0;
//	if (!pThis->Owner)
//		Debug::LogInfo("TechnoClass[%s] GetRemapColor with nullptr Owner !  ", pThis->get_ID());
//	else
//		nColorIdx = pThis->Owner->ColorSchemeIndex;
//
//	R->EDI(nColorIdx);
//	return pPal && pPal->Count > 0 ? 0x705D92 : 0x705DA1;
//}

//this check not event get tripped
//ASMJIT_PATCH(0x451A8A, BuildingClass_AnimLogic_TerrainRemap, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(LightConvertClass*, pResult, EAX);
//
//	if (!pResult)
//		Debug::LogInfo("Building[%s] , trying to get remap palette but failed ! ", pThis->get_ID());
//
//	return 0x0;
//}
#endif

#pragma region Otamaa

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

#include <TerrainTypeClass.h>

//ASMJIT_PATCH(0x73649A, UnitClass_AI_DisguiseAI, 0x7)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	if (!TechnoTypeExtContainer::Instance.Find(pThis->Type)
//		->TankDisguiseAsTank.Get())
//		pThis->UpdateDisguise(); //this one updating the disguise blink and stuffs
//
//
//	return 0x7364A1;
//}

// ASMJIT_PATCH(0x746A30, UnitClass_Disguise_AI_UnitAsUnit, 0x5)
// {
// 	GET(UnitClass*, pThis, ESI);

// 	if (!TechnoTypeExtContainer::Instance.Find(pThis->Type)
// 		->TankDisguiseAsTank.Get())
// 		return 0x0;

// 	R->EAX(pThis->Type);
// 	return 0x746A6C;
// }

#ifdef sss
ASMJIT_PATCH(0x746670, UnitClass_DisguiseAs_Override, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pTarget, 0x4);

	//pThis Can become nullptr , wtf ?
	if (!pThis || !pTarget || Is_Infantry(pTarget))
		return 0x746714;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

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

ASMJIT_PATCH(0x522718, InfantryClass_DisguiseAs_Allowed, 0x8)
{
	GET(InfantryClass*, pThis, EDI);
	GET(InfantryClass*, pThat, ESI);

	if (Allowed(TechnoTypeExtContainer::Instance.Find(pThis->Type), pThat->Type)) {
		pThis->Techno_70E280(pThat);
		return 0x522720;
	}

	return 0x522772;
}

#pragma endregion
#endif
