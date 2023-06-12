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
	TechnoTypeExt::ExtData *pExt = TechnoTypeExt::ExtMap.Find(pType);

DEFINE_HOOK(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	TechnoExt::PlayAnim(pExt->WarpOut.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);

	if (const auto pWeapon = pExt->WarpOutWeapon.Get(pOwner))
		WeaponTypeExt::DetonateAt(pWeapon, pOwner, pOwner , true);

	return 0x719447;
}

DEFINE_HOOK(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x6)
{
	GET_LOCO(ESI);

	//WarpIn is unused , maybe a type on WW side
	TechnoExt::PlayAnim(pExt->WarpIn.GetOrDefault(pOwner ,RulesClass::Instance->WarpOut), pOwner);

	const auto pTechnoExt = TechnoExt::ExtMap.Find(pOwner);

	const auto& weaponType =
		pTechnoExt->LastWarpDistance < pExt->ChronoRangeMinimum.GetOrDefault(pOwner ,RulesClass::Instance->ChronoRangeMinimum)
		? pExt->WarpInMinRangeWeapon : pExt->WarpInWeapon ;

	if (const auto pWeapon = weaponType.Get(pOwner)) {
		const int damage = pExt->WarpInWeapon_UseDistanceAsDamage.Get(pOwner) ?
			(pTechnoExt->LastWarpDistance / Unsorted::LeptonsPerCell) : pWeapon->Damage;
		WeaponTypeExt::DetonateAt(pWeapon, pOwner, pOwner, damage, true);
	}

	return 0x719796;
}

DEFINE_HOOK(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x6)
{
	GET_LOCO(ESI);

	TechnoExt::PlayAnim(pExt->WarpAway.GetOrDefault(pOwner , RulesClass::Instance->WarpOut), pOwner);
	return 0x719878;
}

DEFINE_HOOK(0x7194D0, TeleportLocomotionClass_ILocomotion_Process_ChronoTrigger, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);

	R->AL(pExt->ChronoTrigger.GetOrDefault(pOwner, pRules->ChronoTrigger));

	return 0x7194D6;
}

DEFINE_HOOK(0x7194E3, TeleportLocomotionClass_ILocomotion_Process_ChronoDistanceFactor, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);
	GET(int, val, EAX);

	const auto nDecided = pExt->ChronoDistanceFactor.GetOrDefault(pOwner, pRules->ChronoDistanceFactor);
	const auto factor = MaxImpl(nDecided, 1);// fix factor 0 crash by force it to 1 (Vanilla bug)

	//IDIV
	R->EAX(val / factor);
	R->EDX(val % factor);

	return 0x7194E9;
}

DEFINE_HOOK(0x719519, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EBX);

	R->EBX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));

	return 0x71951F;
}

DEFINE_HOOK(0x719562, TeleportLocomotionClass_ILocomotion_Process_ChronoMinimumDelay2, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, ECX);

	R->ECX(pExt->ChronoMinimumDelay.GetOrDefault(pOwner, pRules->ChronoMinimumDelay));

	return 0x719568;
}

DEFINE_HOOK(0x719555, TeleportLocomotionClass_ILocomotion_Process_ChronoRangeMinimum, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, ECX);
	GET(int, comparator, EDX);

	TechnoExt::ExtMap.Find(pOwner)->LastWarpDistance = comparator;
	const auto factor = pExt->ChronoRangeMinimum.GetOrDefault(pOwner, pRules->ChronoRangeMinimum);
	return comparator < factor ? 0x71955D : 0x719576;
}

DEFINE_HOOK(0x71997B, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x6)
{
	GET_LOCO(ESI);
	GET(RulesClass*, pRules, EAX);

	R->ECX(pExt->ChronoDelay.GetOrDefault(pOwner, pRules->ChronoDelay));

	return 0x719981;
}

#undef GET_LOCO