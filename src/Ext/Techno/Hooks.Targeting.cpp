#include <Ext/TechnoType/Body.h>

#include "Body.h"
#include <InfantryClass.h>

ASMJIT_PATCH(0x6F7E24, TechnoClass_EvaluateObject_MapZone, 0x6)
{
	enum { AllowedObject = 0x6F7EA2, DisallowedObject = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(ZoneType, zone, STACK_OFFSET(0x3C ,0x18));

	auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!TechnoExtData::AllowedTargetByZone(pThis, pTarget, pThisTypeExt->TargetZoneScanType, pWeapon, zone))
		return DisallowedObject;

	return AllowedObject;
}

ASMJIT_PATCH(0x6FA67D, TechnoClass_Update_DistributeTargetingFrame, 0xA)
{
	enum { Targeting = 0x6FA687, SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);

	auto const pRulesExt = RulesExtData::Instance();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if ((!pThis->Owner->IsControlledByHuman() || !pRulesExt->DistributeTargetingFrame_AIOnly) && pTypeExt->DistributeTargetingFrame.Get(pRulesExt->DistributeTargetingFrame)) {
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (Unsorted::CurrentFrame % 16 != pExt->MyTargetingFrame) {
			return SkipTargeting;
		}
	}

	if(pThis->MegaMissionIsAttackMove()){
		if(!RulesExtData::Instance()->ExpandAircraftMission && pThis->WhatAmI() == AbstractType::Aircraft && (!pThis->Ammo || pThis->GetHeight() < Unsorted::CellHeight))
			return SkipTargeting;

		pThis->UpdateAttackMove();
		return SkipTargeting;
	}

	if (auto pInf = cast_to<InfantryClass*, false>(pThis)) {
		if (pInf->Type->Slaved && pInf->SlaveOwner) {
			return SkipTargeting;
		}
	}

	if(!pThis->IsArmed())
		return SkipTargeting;

	return 0x6FA697;
}