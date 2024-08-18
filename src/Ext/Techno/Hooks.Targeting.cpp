#include <Ext/TechnoType/Body.h>

#include "Body.h"

DEFINE_HOOK(0x6F7E24, TechnoClass_EvaluateObject_MapZone, 0x6)
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

DEFINE_HOOK(0x6FA697, TechnoClass_Update_DontScanIfUnarmed, 0x6)
{
	enum { SkipTargeting = 0x6FA6F5, DoTargeting = 0 };

	GET(TechnoClass*, pThis, ESI);

	return pThis->IsArmed() ? DoTargeting : SkipTargeting;
}

DEFINE_HOOK(0x70982C, TechnoClass_TargetAndEstimateDamage_ScanDelay, 0x8)
{
	GET(TechnoClass*, pThis, ESI);

	pThis->__creationframe_4FC = R->EAX();
	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);
	const bool IsHuman = pThis->Owner->IsHumanPlayer || pThis->Owner->IsControlledByHuman();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const auto pRules = RulesClass::Instance();

	if (pThis->CurrentMission == Mission::Area_Guard) {
		if (IsHuman) {
			delay += pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExtData::Instance()->PlayerGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));
		} else {
			delay += pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExtData::Instance()->AIGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));
		}
	} else {
		if (IsHuman) {
			delay += pTypeExt->PlayerNormalTargetingDelay.Get(RulesExtData::Instance()->PlayerNormalTargetingDelay.Get(pRules->NormalTargetingDelay));
		} else {
			delay += pTypeExt->AINormalTargetingDelay.Get(RulesExtData::Instance()->AINormalTargetingDelay.Get(pRules->NormalTargetingDelay));
		}
	}

	pThis->TargetingTimer.Start(delay);

	if (pTypeExt->AutoFire) {
		pThis->SetTarget(pTypeExt->AutoFire_TargetSelf ? pThis :
		static_cast<AbstractClass*>(pThis->GetCell()));
		return 0x7099B8;
	}

	return 0x7098B9;
}