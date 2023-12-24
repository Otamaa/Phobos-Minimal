#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Locomotor/Cast.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion*, Loco, reg_Loco); \
	TeleportLocomotionClass* pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	TechnoClass* pOwner =  pLocomotor->LinkedTo ? pLocomotor->LinkedTo : pLocomotor->Owner; \
	TechnoTypeClass* pType = pOwner->GetTechnoType(); \
	TechnoTypeExtData *pExt = TechnoTypeExtContainer::Instance.Find(pType);

DEFINE_HOOK(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpOut.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);

	if (const auto pWeapon = pExt->WarpOutWeapon.Get(pOwner))
		WeaponTypeExtData::DetonateAt(pWeapon, pOwner, pOwner , true , nullptr);

	return 0x719447;
}

DEFINE_HOOK(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	//WarpIn is unused , maybe a type on WW side
	TechnoExtData::PlayAnim(pExt->WarpIn.GetOrDefault(pOwner ,RulesClass::Instance->WarpOut), pOwner);

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pOwner);

	const auto Rank = pOwner->CurrentRanking;
	const auto pWarpInWeapon = pExt->WarpInWeapon.GetFromSpecificRank(Rank);

	const auto pWeapon = pTechnoExt->LastWarpDistance < pExt->ChronoRangeMinimum.GetOrDefault(pOwner ,RulesClass::Instance->ChronoRangeMinimum)
		? pExt->WarpInMinRangeWeapon.GetFromSpecificRank(Rank)->Get(pWarpInWeapon) : pWarpInWeapon;

	if (pWeapon) {
		const int damage = pExt->WarpInWeapon_UseDistanceAsDamage.Get(pOwner) ?
			(pTechnoExt->LastWarpDistance / Unsorted::LeptonsPerCell) : pWeapon->Damage;

		WeaponTypeExtData::DetonateAt(pWeapon, pOwner, pOwner, damage, true, nullptr);
	}

	return 0x719796;
}

DEFINE_HOOK(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	TechnoExtData::PlayAnim(pExt->WarpAway.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);
	return 0x719878;
}

DEFINE_HOOK(0x7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);
	GET(int, val, EDX);
	enum { SetTimer = 0x7194E9, CheckTheTimer = 0x7194FD };

	if (pExt->ChronoTrigger.GetOrDefault(pOwner, pRules->ChronoTrigger)) {

		R->ECX(Unsorted::CurrentFrame());

		const auto nDecided = pExt->ChronoDistanceFactor.GetOrDefault(pOwner, pRules->ChronoDistanceFactor);
		// fix factor 0 crash by force it to 1 (Vanilla bug)
		R->EAX(val / MaxImpl(nDecided, 1));
		return SetTimer;
	}

	return CheckTheTimer;
}

DEFINE_HOOK(0x719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);

	R->EBX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));

	return 0x71951F;
}

DEFINE_HOOK(0x719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 0x6)
{
	enum { SetTimer = 0x719568, SetWarpingOut = 0x719576 };

	GET_LOCO(ESI);
	GET(RulesClass*, pRules, ECX);
	GET(int, comparator, EDX);

	TechnoExtContainer::Instance.Find(pOwner)->LastWarpDistance = comparator;
	const auto factor = pExt->ChronoRangeMinimum.GetOrDefault(pOwner, pRules->ChronoRangeMinimum);

	if(comparator < factor) {
		R->EAX(Unsorted::CurrentFrame());
		R->ECX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));
		return SetTimer;
	}

	return SetWarpingOut;
}

DEFINE_HOOK(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EAX);

	R->ECX(pExt->ChronoDelay.GetOrDefault(pOwner, pRules->ChronoDelay));

	return 0x719981;
}

#undef GET_LOCO