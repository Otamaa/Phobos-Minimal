#include "Body.h"

#include <Locomotor/DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NewSWType.h>

DEFINE_HOOK(0x4B5BCD, DroppodLoco_Process_Speed, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis->Owner);

	//auto nLinked = pThis->Owner ? pThis->Owner->get_ID() : "None";
	//Debug::Log(__FUNCTION__"DroppodLoco [%x] Owner [%s] \n",pThis, nLinked);
	R->EAX(pExt->LinkedSW ?
		SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Speed : pRules->DropPodSpeed);

	return 0x4B5BD3;
}

// Angle ,4B5BEC , EDX ,4B5BF2
DEFINE_HOOK(0x4B5BEC, DroppodLoco_Process_Angle1, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis->Owner);

	R->EDX(pExt->LinkedSW ?
		SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5BF2;
}

// Angle , 4B5C14,ECX ,4B5C1A
DEFINE_HOOK(0x4B5C14, DroppodLoco_Process_Angle2, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis->Owner);

	R->ECX(pExt->LinkedSW ?
		SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5C1A;
}

// Angle , 4B5C50, EAX ,4B5C56
DEFINE_HOOK(0x4B5C50, DroppodLoco_Process_Angle3, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis->Owner);

	R->EAX(pExt->LinkedSW ?
		SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5C56;
}

DEFINE_HOOK(0x519168, InfantryClass_DrawIt_DroppodLinked, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if(pExt->LinkedSW) {
		R->EAX(SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_PodImage_Infantry.Get());
		return 0x519172;
	}

	return 0x0;
}

DEFINE_HOOK(0x4B641D, DroppodLocomotionClass_IPiggy_EndPiggyback, 7)
{
	GET(ILocomotion*, pIloco, EAX);
	const auto pLoco = static_cast<LocomotionClass*>(pIloco);

	if (pLoco->Owner) {
		const auto pExt = TechnoExt::ExtMap.Find(pLoco->Owner);
		if (pExt->LinkedSW && (AresNewSuperType)pExt->LinkedSW->Type->Type == AresNewSuperType::DropPod)
			pExt->LinkedSW = nullptr;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	GET(FootClass*, pFoot, ESI);
	REF_STACK(const CoordStruct, Coords, 0x34);

	const auto pExt = TechnoExt::ExtMap.Find(pFoot);

	// create trailer even without weapon, but only if it is set
	if (!(Unsorted::CurrentFrame % 6))
	{
		if (AnimTypeClass* pType = pExt->LinkedSW ? SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Trailer :  RulesExt::Global()->DropPodTrailer)
		{
			AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, Coords),
				pFoot->Owner,
				nullptr,
				pFoot,
				false
			);
		}
	}

	if (const auto pWeapon = pExt->LinkedSW ? SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Weapon : RulesClass::Instance->DropPodWeapon)
	{
		R->ESI(pWeapon);
		return 0x4B5F14;
	}

	return 0x4B602D;
}

//TODO : DropPod WH explosion 4B5D8F ,4B6028
DEFINE_HOOK(0x4B5CF1, DropPodLocomotionClass_Process_DroppodPuff, 0x5)
{
	//GET(DropPodLocomotionClass*, pLoco, EDI);
	GET(FootClass*, pFoot, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x40 - 0x18);

	if (!pFoot->Unlimbo(*pCoord, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::Min, DirType::Max)))
		return 0x4B5D0A;

	const auto pExt = TechnoExt::ExtMap.Find(pFoot);

	if (auto pAnimType = pExt->LinkedSW ? SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_Puff : RulesClass::Instance->DropPodPuff)
	{
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pFoot->Owner,
			nullptr,
			pFoot,
			false
		);
	}

	const auto nDroppod = pExt->LinkedSW ?
		make_iterator((SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_GroundPodAnim)) : make_iterator(RulesClass::Instance->DropPod);

	if (!nDroppod)
		return 0x4B5E4C;

	//TS random it with the lpvtable ? idk
	if (auto pAnimType = nDroppod[ScenarioClass::Instance->Random.RandomFromMax(nDroppod.size() - 1)])
	{
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pFoot->Owner,
			nullptr,
			pFoot,
			false
		);
	}

	//original game code
	//using static_cast adding some unnessesary check !
	/*pLoco + 0x18*/
	//if (reinterpret_cast<void*>((DWORD)pLoco + 0x18)) {
	//
	//	if(RulesClass::Instance->DropPod.Count == 1)
	//		return 0x4B5E4C;
	//
	//	if (auto pAnimType = RulesClass::Instance->DropPod[1]) {
	//		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
	//			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	//	}
	//} else {
	//
	//	if (auto pAnimType = RulesClass::Instance->DropPod[0]) {
	//		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
	//			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	//	}
	//}

	return 0x4B5E4C;
}

DEFINE_HOOK(0x4B619F, DropPodLocomotionClass_MoveTo_AtmosphereEntry, 0x5)
{
	GET(DropPodLocomotionClass*, pLoco, EDI);
	LEA_STACK(CoordStruct*, pCoord, 0x1C - 0xC);

	const auto pExt = TechnoExt::ExtMap.Find(pLoco->Owner);

	if (auto pAnimType = pExt->LinkedSW ? SWTypeExt::ExtMap.Find(pExt->LinkedSW->Type)->Droppod_AtmosphereEntry : RulesClass::Instance->AtmosphereEntry)
	{
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pLoco->Owner->Owner,
			nullptr,
			pLoco->Owner,
			false);
	}

	return 0x4B61D6;
}

DEFINE_HOOK(0x4B5F7E, DropPodLocomotionClass_ILocomotion_Process_Report, 0x6)
{
	// do not divide by zero
	GET(int const, count, EBP);
	return count ? 0 : 0x4B5FAD;
}