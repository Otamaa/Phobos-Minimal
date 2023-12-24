#include "Body.h"

#include <Locomotor/DropPodLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NewSWType.h>

DEFINE_HOOK(0x4B5BCD, DroppodLoco_Process_Speed, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	R->EAX(pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod ?
		SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Speed : pRules->DropPodSpeed);

	return 0x4B5BD3;
}

DEFINE_HOOK(0x4B5BEC, DroppodLoco_Process_Angle1, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	R->EDX(pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod ?
		SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5BF2;
}

DEFINE_HOOK(0x4B5C14, DroppodLoco_Process_Angle2, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EDX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	R->ECX(pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod ?
		SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5C1A;
}

DEFINE_HOOK(0x4B5C50, DroppodLoco_Process_Angle3, 0x6)
{
	GET(DropPodLocomotionClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

	R->EAX(pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod ?
		SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Angle : pRules->DropPodAngle);

	return 0x4B5C56;
}

DEFINE_HOOK(0x519168, InfantryClass_DrawIt_DroppodLinked, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if(pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod) {
		R->EAX(SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_PodImage_Infantry.Get());
		return 0x519172;
	}

	return 0x0;
}

DEFINE_HOOK(0x4B641D, DroppodLocomotionClass_IPiggy_EndPiggyback, 7)
{
	GET(ILocomotion*, pIloco, EAX);
	const auto pLoco = static_cast<LocomotionClass*>(pIloco);

	if (pLoco->Owner) {
		const auto pExt = TechnoExtContainer::Instance.Find(pLoco->Owner);
		if (pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod)
			pExt->LinkedSW = nullptr;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	GET(DropPodLocomotionClass*, pDroppod, EDI);
	GET(FootClass*, pFoot, ESI);
	LEA_STACK(CoordStruct*, pCoords, 0x40 - 0xC);

	const auto pExt = TechnoExtContainer::Instance.Find(pFoot);

	// create trailer even without weapon, but only if it is set
	if (!(Unsorted::CurrentFrame % 6))
	{
		if (AnimTypeClass* pType = pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod
			? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Trailer :  RulesExtData::Instance()->DropPodTrailer)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoords , 0 ,1,(AnimFlag)0x600, 0 , false),
				pFoot->Owner,
				nullptr,
				pFoot,
				false
			);
		}
	}

	if (const auto pWeapon = pExt->LinkedSW ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Weapon : RulesClass::Instance->DropPodWeapon)
	{
		if (!(Unsorted::CurrentFrame % 3))
		{
			// Please dont ask , see the binary yourself ,.. (-Otamaa)
			CoordStruct nDest = *reinterpret_cast<CoordStruct*>(((uintptr_t)(pDroppod)) + 0x1C);

			const auto pCell = MapClass::Instance->GetCellAt(nDest);
			const auto pCellTechno = pCell->FindTechnoNearestTo(Point2D::Empty, false, nullptr);

			if(!pCellTechno || !pFoot->Owner->IsAlliedWith(pCellTechno))
			{
				auto coordDest = MapClass::GetRandomCoordsNear(nDest, 85, false);

				if (pWeapon->Report.Count > 0) {
					VocClass::PlayIndexAtPos(ScenarioClass::Instance->Random.RandomFromMax(pWeapon->Report.Count - 1), pCoords , nullptr);
				}

				if(pWeapon->Warhead) {
					MapClass::DamageArea(coordDest, 2 * pWeapon->Damage, pFoot, pWeapon->Warhead, pWeapon->Warhead->Tiberium, pFoot->Owner);

					if (auto pWeaponAnimType = MapClass::SelectDamageAnimation(2 * pWeapon->Damage, pWeapon->Warhead, LandType::Clear, coordDest))
					{
						auto zAdjust = Game::AdjustHeight(coordDest.Z);

						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pWeaponAnimType, coordDest, 0, 1, (AnimFlag)0x2600, zAdjust, false),
							pFoot->Owner,
							pCellTechno ? pCellTechno->Owner : nullptr,
							pFoot,
							false
						);
					}
				}
			}
		}
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

	const auto pExt = TechnoExtContainer::Instance.Find(pFoot);
	const auto IsLinkedSWEligible = pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod;

	if (auto pAnimType = IsLinkedSWEligible ? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_Puff : RulesClass::Instance->DropPodPuff)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pFoot->Owner,
			nullptr,
			pFoot,
			false
		);
	}

	const auto nDroppod = IsLinkedSWEligible ?
		make_iterator((SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_GroundPodAnim)) : make_iterator(RulesClass::Instance->DropPod);

	if (!nDroppod)
		return 0x4B5E4C;

	//TS random it with the lpvtable ? idk
	if (auto pAnimType = nDroppod[ScenarioClass::Instance->Random.RandomFromMax(nDroppod.size() - 1)])
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pFoot->Owner,
			nullptr,
			pFoot,
			false
		);
	}

	return 0x4B5E4C;
}

DEFINE_HOOK(0x4B619F, DropPodLocomotionClass_MoveTo_AtmosphereEntry, 0x5)
{
	GET(DropPodLocomotionClass*, pLoco, EDI);
	LEA_STACK(CoordStruct*, pCoord, 0x1C - 0xC);

	const auto pExt = TechnoExtContainer::Instance.Find(pLoco->Owner);

	if (auto pAnimType = pExt->LinkedSW && (int)pExt->LinkedSW->Type->Type == (int)AresNewSuperType::DropPod 
		? SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Droppod_AtmosphereEntry : RulesClass::Instance->AtmosphereEntry)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0),
			pLoco->Owner->Owner,
			nullptr,
			pLoco->Owner,
			false);
	}

	return 0x4B61D6;
}
