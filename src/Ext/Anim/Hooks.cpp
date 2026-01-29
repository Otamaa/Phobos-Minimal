#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/AnimType/Body.h>

#include <Misc/Hooks.Otamaa.h>

 ASMJIT_PATCH(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(FakeAnimClass*, pAnim, EBP);
	pAnim->_GetExtData()->ParentBuilding = pThis;
	return 0;
}

ASMJIT_PATCH(0x4232E2, AnimClass_DrawIt_AltPalette, 0x6)
{
	enum { SkipGameCode = 0x4232EA  , SetAltPaletteLightConvert = 0x4232F0 };

	GET(FakeAnimClass*, pThis, ESI);

	const auto pTypeExt = pThis->_GetTypeExtData();
	int schemeIndex = RulesExtData::Instance()->AnimRemapDefaultColorScheme;

	if (((pTypeExt->CreateUnitType && pTypeExt->CreateUnitType->RemapAnim.Get(pTypeExt->RemapAnim)) || pTypeExt->RemapAnim) && pThis->Owner) {
		schemeIndex = pThis->Owner->ColorSchemeIndex - 1;
	}

	schemeIndex += pTypeExt->AltPalette_ApplyLighting ? 1 : 0;
	R->ECX(ColorScheme::Array->Items[schemeIndex]);

	return SkipGameCode;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E33CC, GET_OFFSET(AnimExtData::GetLayer_patch));

//ASMJIT_PATCH(0x424CB0, AnimClass_InWhichLayer_Override, 0x6) //was 5
//{
//	GET(AnimClass*, pThis, ECX);
//
//	enum
//	{
//		RetLayerGround = 0x424CBA,
//		RetLayerAir = 0x0424CD1,
//		RetTypeLayer = 0x424CCA,
//		ReturnSetManualResult = 0x424CD6
//	};
//
//	if (pThis->Type) {
//		if (pThis->OwnerObject) {
//
//			const auto pExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
//
//			if (!pExt->Layer_UseObjectLayer.isset() || !pThis->OwnerObject->IsAlive) {
//				return RetLayerGround;
//			}
//
//			if (pExt->Layer_UseObjectLayer.Get()) {
//
//				Layer nRes = Layer::Ground;
//
//				if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject)) {
//
//					if(pFoot->IsCrashing ||  pFoot->IsSinking)
//						return RetLayerGround;
//
//					if (auto const pLocomotor = pFoot->Locomotor.GetInterfacePtr())
//						nRes = pLocomotor->In_Which_Layer();
//				}
//				else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject)) {
//					nRes = pBullet->InWhichLayer();
//				}
//				else {
//					nRes = pThis->OwnerObject->ObjectClass::InWhichLayer();
//				}
//
//				R->EAX(nRes);
//				return ReturnSetManualResult;
//			}
//		}
//		else {
//			R->EAX(pThis->Type->Layer);
//			return ReturnSetManualResult;
//		}
//	}
//
//	return RetLayerAir;
//}

ASMJIT_PATCH(0x424C3D, AnimClass_AttachTo_AttachedAnimPosition, 0x6)
{
	enum { SkipGameCode = 0x424C76 };

	GET(FakeAnimClass*, pThis, ESI);

	auto pExt = pThis->_GetTypeExtData();

	if (pExt->AttachedAnimPosition != AttachedAnimPosition::Default)
	{
		pThis->SetLocation(CoordStruct::Empty);
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(FakeAnimClass*, pThis, ECX);

	if(!pThis->Type)
		return 0x0;

	auto pExt = pThis->_GetTypeExtData();

	if (pThis->OwnerObject && pExt->Layer_UseObjectLayer.isset())
	{
		Layer layer = pThis->Type->Layer;

		if (pExt->Layer_UseObjectLayer.Get())
			layer = pThis->OwnerObject->InWhichLayer();

		R->EAX(layer);

		return ReturnValue;
	}

	return 0;
}

//ASMJIT_PATCH(0x422BF1, AnimClass_AttachTo_AttachedCoords, 0x5)
//{
//	GET(FakeAnimClass*, pThis, ESI);
//	GET_STACK(CoordStruct*, pCoords, STACK_OFFSET(0x20, 0x4));
//
//	pThis->OwnerObject->GetRenderCoords(pCoords);
//	*pCoords = pCoords->operator+(pThis->Location);
//
//	if (pThis->Type) {
//		auto pTypeExt = pThis->_GetTypeExtData();
//
//		if (pTypeExt->AttachedAnimPosition != AttachedAnimPosition::Default) {
//
//
//			//save original coords because centering it broke damage
//			//pThis->_GetExtData()->BackupCoords = pCoords->operator+(pThis->Location);
//
//
//
//			if (pTypeExt->AttachedAnimPosition & AttachedAnimPosition::Ground){
//				pCoords->Z = MapClass::Instance->GetCellFloorHeight(pCoords);
//			}
//
//			if(pTypeExt->AttachedAnimPosition & AttachedAnimPosition::Center) {
//				pCoords->X += 128;
//				pCoords->Y += 128;
//			}
//		}
//	}
//
//	R->EAX(pCoords);
//	return 0x422C31;
//}

ASMJIT_PATCH(0x423365, AnimClass_DrawIt_ExtraShadow, 0x8)
{
	enum { DrawShadow = 0x42336D, SkipDrawShadow = 0x4233EE };

	GET(FakeAnimClass*, pThis, ESI);

	if (!pThis->Type->Shadow)
		return SkipDrawShadow;

	const bool hasExtra = R->AL();

	return hasExtra && pThis->_GetTypeExtData()->ExtraShadow ?
		DrawShadow : SkipDrawShadow;
}

//ASMJIT_PATCH(0x425060, AnimClass_Expire_ScorchFlamer, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//
//	auto const pType = pThis->Type;
//
//	if (!pType->Flamer && !pType->Scorch)
//		return 0;
//
//	AnimExtData::SpawnFireAnims(pThis);
//
//	return 0;
//}