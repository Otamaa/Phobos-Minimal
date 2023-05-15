// Anim-to--Unit
// Author: Otamaa

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_ReceiveDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x44, -0x4));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	R->ECX(R->ESI());
	pExt->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, args.Attacker);
	pThis->Destroy();
	return 0x737F74;
}

DEFINE_HOOK(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage) {
		AnimTypeExt::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExt::CreateUnit_MarkCell(pThis);
	return 0;
}

DEFINE_HOOK(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExt::CreateUnit_Spawn(pThis);
	return 0;
}

#include <Ext/Bullet/Body.h>

// this set after ares set their ownership
// DEFINE_HOOK(0x469C98, BulletClass_Logics_DamageAnimSelected, 0x9) //was 0
// {
// 	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

// 	GET(BulletClass*, pThis, ESI);
// 	GET(AnimClass*, pAnim, EAX);

// 	const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

// 	if (pAnim && pAnim->Type) {
// 		HouseClass* pInvoker =  nullptr;
// 		HouseClass* pVictim = nullptr;

// 		if(auto pTech = pThis->Owner) {
// 			pInvoker = pThis->Owner->GetOwningHouse();
// 			if(auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
// 				pAnimExt->Invoker = pTech;
// 		}
// 		else
// 		{
// 			if(auto const pBulletExt = BulletExt::ExtMap.Find(pThis))
// 			pInvoker = pBulletExt->Owner;
// 		}

// 		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
// 			pVictim = Target->Owner;

// 		AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

// 	} else if (pWarheadExt->IsNukeWarhead.Get()) {
// 		return NukeWarheadExtras;
// 	}

// 	return Continue;
// }
//
//DEFINE_HOOK(0x6E2368, ActionClass_PlayAnimAt, 0x7)
//{
//	GET(AnimClass*, pAnim, EAX);
//	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x4));
//
//	if (pAnim) {
//		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pHouse,false);
//	}
//
//	return 0;
//}