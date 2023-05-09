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

	if (pThis->Type) {
		if (pThis->OwnerObject) {

			const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

			if (!pExt->Layer_UseObjectLayer.isset() || !pThis->OwnerObject->IsAlive) {
				return RetLayerGround;
			}

			if (pExt->Layer_UseObjectLayer.Get()) {

				Layer nRes = Layer::Ground;

				if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject)) {
					if (auto const pLocomotor = pFoot->Locomotor.get())
						nRes = pLocomotor->In_Which_Layer();
				}
				else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject)) {
					nRes = pBullet->InWhichLayer();
				}
				else {
					nRes = pThis->OwnerObject->ObjectClass::InWhichLayer();
				}

				R->EAX(nRes);
				return ReturnSetManualResult;
			}
		}
		else {
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

			//save original coords because centering it broke damage
			AnimExt::ExtMap.Find(pThis)->BackupCoords = pObject->Location;

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
			pExt->AttachedSystem.reset(nullptr);

		if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
			pExt->CreateAttachedSystem();
	}

	return 0x0;
}

DEFINE_HOOK(0x424AEC, AnimClass_AI_SetMission, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	const auto pTypeExt = AnimTypeExt::ExtMap.TryFind(pThis->Type);

	const Mission nMission = pTypeExt && pTypeExt->MakeInfantry_Mission.isset()  ? pTypeExt->MakeInfantry_Mission : (Mission::Hunt);
	Debug::Log("Anim[%s] with MakeInf , setting Mission[%s] ! \n", pTypeExt ? pTypeExt->Get()->ID  : NONE_STR , MissionClass::MissionToString(nMission));
	pInf->QueueMission(nMission, false);
	return 0x0;
}

//the stack is change , so i need to replace everything if i want just use normal hook
//this make it unnessesary
//replace the vtable call
void __fastcall Dummy(DWORD t, DWORD , Mission m, bool e){ }
DEFINE_JUMP(CALL6, 0x424B04, GET_OFFSET(Dummy));