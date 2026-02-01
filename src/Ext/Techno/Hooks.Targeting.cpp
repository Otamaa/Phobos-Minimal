#include <Ext/TechnoType/Body.h>

#include "Body.h"
#include <InfantryClass.h>
#include <Ext/WeaponType/Body.h>

bool NOINLINE TechnoExtData::CanRetaliateICUnit(TechnoClass* pThis , FakeWeaponTypeClass* pWP , TechnoClass* pTarget){

	if(!pTarget->IsIronCurtained())
		return true;

	bool canAutoTarget = RulesExtData::Instance()->AutoTarget_IronCurtained;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if(!pWP) {
		auto pWPExt = pWP->_GetExtData();
		if(pThis->Owner->IsControlledByHuman()) {
			canAutoTarget = pWPExt->CanTarget_IronCurtained.Get(RulesExtData::Instance()->AutoTarget_IronCurtained);
		}
	}

	return pTypeExt->AllowFire_IroncurtainedTarget.Get(canAutoTarget);
}


ASMJIT_PATCH(0x6F7CE2, TechnoClass_CanAutoTargetObject_DisallowMoving, 0x6)
{
	enum { returnFalse = 0x6F894F , compareFireError = 0x6F7CEE , continueExcution = 0x0 };

	GET(TechnoClass* const, pThis, EDI);
	GET(AbstractClass* const, pTarget, ESI);
	GET(const int, WeaponIndex, EBX);

	if (const auto pUnit = cast_to<UnitClass*,false>(pThis)) {
		if (TechnoExtData::CannotMove(pUnit)) {
			R->EAX(pUnit->GetFireError(pTarget, WeaponIndex, true));
			return compareFireError;
		}
	}else if (const auto pInf = cast_to<InfantryClass*,false>(pThis)) {
		if (pInf->Type->Speed <= 0) {
			R->EAX(pInf->GetFireError(pTarget, WeaponIndex, true));
			return compareFireError;
		}
	}else if (auto pTechnoTarget = flag_cast_to<TechnoClass*, false>(pTarget))
		if(!TechnoExtData::CanRetaliateICUnit(pThis, (FakeWeaponTypeClass*)pThis->GetWeapon(WeaponIndex)->WeaponType, pTechnoTarget)) {
			return returnFalse;
	}

	return continueExcution;
}

ASMJIT_PATCH(0x6F7E24, TechnoClass_EvaluateObject_MapZone, 0x6)
{
	enum { AllowedObject = 0x6F7EA2, DisallowedObject = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(ZoneType, zone, STACK_OFFSET(0x3C ,0x18));

	auto pThisTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (!TechnoExtData::AllowedTargetByZone(pThis, pTarget, pThisTypeExt->TargetZoneScanType, pWeapon, zone))
		return DisallowedObject;

	return AllowedObject;
}

#pragma region PassiveAcquireMode

ASMJIT_PATCH(0x6F8E1F, TechnoClass_SelectAutoTarget_CeasefireMode, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(TechnoClass*, pThis, ESI);

	if(TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled)
		return 0x6F8E38;

	R->CL(pType->NoAutoFire || (TechnoExtContainer::Instance.Find(pThis)->GetPassiveAcquireMode()) == PassiveAcquireMode::Ceasefire);
	return R->Origin() + 0x6;
}

#pragma endregion