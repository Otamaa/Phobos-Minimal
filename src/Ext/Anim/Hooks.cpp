#include "Body.h"
#include <Utilities/Macro.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EBP);

	auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
	pAnimExt->ParentBuilding = pThis;
	pAnimExt->Invoker = pThis;

	return 0;
}

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));

	if (pThis->Type)
	{
		pCoord->X += AnimTypeExt::ExtMap.Find(pThis->Type)->XDrawOffset;
	}

	return 0;
}

DEFINE_HOOK(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pType, EDX);

	if (pType && pType->HideIfNoOre)
	{
		int nThreshold = abs(AnimTypeExt::ExtMap.Find(pType)->HideIfNoOre_Threshold.Get());
		auto const pCell = pThis->GetCell();

		pThis->Invisible = !pCell || pCell->GetContainedTiberiumValue() <= nThreshold;

		return 0x423BBF;
	}

	return 0x0;

} //was 8

//DEFINE_JUMP(VTABLE, 0x7E33CC, GET_OFFSET(AnimExt::GetLayer_patch));

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_Override, 0x6) //was 5
{
	GET(AnimClass*, pThis, ECX);

	enum
	{
		RetLayerGround = 0x424CBA,
		RetLayerAir = 0x0424CD1,
		RetTypeLayer = 0x424CCA,
		ReturnSetManualResult = 0x424CD6
	};

	if (pThis->Type)
	{
		if (pThis->OwnerObject)
		{
			const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

			if (!pExt->Layer_UseObjectLayer.isset())
			{
				return RetLayerGround;
			}

			if (pExt->Layer_UseObjectLayer.Get())
			{
				Layer nRes = Layer::Ground;

				if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject))
				{
					if (auto const pLocomotor = pFoot->Locomotor.get())
						nRes = pLocomotor->In_Which_Layer();
				}
				else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
					nRes = pBullet->InWhichLayer();
				else
					nRes = pThis->OwnerObject->ObjectClass::InWhichLayer();

				R->EAX(nRes);
				return ReturnSetManualResult;
			}
		}
		else
		{
			R->EAX(pThis->Type->Layer);
			return ReturnSetManualResult;
		}
	}

	return RetLayerAir;
}

DEFINE_HOOK(0x424C3D, AnimClass_AttachTo_BuildingCoords, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);
	LEA_STACK(CoordStruct*, pCoords, STACK_OFFS(0x34, 0xC));

	if (pThis->Type)
	{
		if (AnimTypeExt::ExtMap.Find(pThis->Type)
			->UseCenterCoordsIfAttached)
		{
			pObject->GetRenderCoords(pCoords);
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
		const auto pExt = AnimExt::ExtMap.Find(pThis);
		const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

		if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
			pExt->DeleteAttachedSystem();

		if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
			pExt->CreateAttachedSystem();
	}

	return 0x0;
}

#ifdef ENABLE_PHOBOS_DAMAGEDELAYANIM

// Goes before and replaces Ares animation damage / weapon hook at 0x424538.
//DEFINE_HOOK(0x42450D, AnimClass_AI_Damage, 0x6)
//{
//	enum
//	{
//		SkipDamage = 0x424665,
//		CheckIsActive = 0x42464C,
//		SkipDamage2 = 0x42466B,
//		ReturnFinished = 0x424B42,
//		Continue = 0x0
//	};
//
//	GET(AnimClass*, pThis, ESI);
//
//	if (pThis->Type)
//	{
//		R->EBX(pThis->Animation.Value);
//
//		if (!AnimExt::DealDamageDelay(pThis))
//		{
//			R->EAX(pThis->Type);
//			R->EDI(0);
//			R->ECX(pThis->Type->MiddleFrameIndex);
//			return SkipDamage2;
//		}
//
//		return CheckIsActive;
//	}
//
//	return Continue;
//}
#endif