#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Ext/AnimType/Body.h>

//DEFINE_HOOK(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(AnimClass*, pAnim, EBP);
//
//	auto const pAnimExt = AnimExtContainer::Instance.Find(pAnim);
//	pAnimExt->ParentBuilding = pThis;
//	pAnimExt->Invoker = pThis;
//
//	return 0;
//}

DEFINE_HOOK(0x4232E2, AnimClass_DrawIt_AltPalette, 0x6)
{
	enum { SkipGameCode = 0x4232EA  , SetAltPaletteLightConvert = 0x4232F0 };

	GET(AnimClass*, pThis, ESI);

	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	int schemeIndex = RulesExtData::Instance()->AnimRemapDefaultColorScheme;

	if (((pTypeExt->CreateUnit && pTypeExt->CreateUnit_RemapAnim.Get(pTypeExt->RemapAnim)) || pTypeExt->RemapAnim) && pThis->Owner) {
		schemeIndex = pThis->Owner->ColorSchemeIndex - 1;
	}

	schemeIndex += pTypeExt->AltPalette_ApplyLighting ? 1 : 0;
	R->ECX(ColorScheme::Array->Items[schemeIndex]);

	return SkipGameCode;
}

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));

	if (pThis->Type)
	{
		pCoord->X += AnimTypeExtContainer::Instance.Find(pThis->Type)->XDrawOffset;
	}

	return 0;
}

DEFINE_HOOK(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pType, EDX);

	if (pType && pType->HideIfNoOre)
	{
		int nThreshold = abs(AnimTypeExtContainer::Instance.Find(pType)->HideIfNoOre_Threshold.Get());
		auto const pCell = pThis->GetCell();

		pThis->Invisible = !pCell || pCell->GetContainedTiberiumValue() <= nThreshold;

		return 0x423BBF;
	}

	return 0x0;

} //was 8

//DEFINE_JUMP(VTABLE, 0x7E33CC, GET_OFFSET(AnimExtData::GetLayer_patch));

//DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_Override, 0x6) //was 5
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

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(AnimClass*, pThis, ECX);

	auto pExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

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

DEFINE_HOOK(0x424C3D, AnimClass_AttachTo_BuildingCoords, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);
	LEA_STACK(CoordStruct*, pCoords, STACK_OFFS(0x34, 0xC));

	if (pThis->Type)
	{
		if (AnimTypeExtContainer::Instance.Find(pThis->Type)
			->UseCenterCoordsIfAttached)
		{
			pObject->GetRenderCoords(pCoords);

			//save original coords because centering it broke damage
			AnimExtContainer::Instance.Find(pThis)->BackupCoords = pObject->Location;

			pCoords->X += 128;
			pCoords->Y += 128;
			R->EAX(pCoords);
			return 0x424C49;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x424807, AnimClass_AI_Next, 0x6) //was 8
{
	GET(AnimClass*, pThis, ESI);

	if (pThis->Type)
	{
		const auto pExt = AnimExtContainer::Instance.Find(pThis);
		const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

		if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
			pExt->AttachedSystem.clear();

		if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
			pExt->CreateAttachedSystem();
	}

	return 0x0;
}

DEFINE_HOOK(0x424AEC, AnimClass_AI_SetMission, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	const Mission nMission = pTypeExt->MakeInfantry_Mission.Get(Mission::Hunt);
	Debug::Log("Anim[%s] with MakeInf , setting Mission[%s] ! \n", pThis->Type->ID , MissionClass::MissionToString(nMission));
	pInf->QueueMission(nMission, false);
	return 0x0;
}

//the stack is change , so i need to replace everything if i want just use normal hook
//this make it unnessesary
//replace the vtable call
void __fastcall Dummy(DWORD t, DWORD , Mission m, bool e){ }
DEFINE_JUMP(CALL6, 0x424B04, GET_OFFSET(Dummy));