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