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

//TODO : use better stuffs
/*
*	Husk.Type
*
*/
DEFINE_HOOK(0x737F6D, UnitClass_ReceiveDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x44, -0x4));

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	R->ECX(R->ESI());
	pExt->ReceiveDamage = true;
	AnimTypeExtData::ProcessDestroyAnims(pThis, args.Attacker , args.WH);
	pThis->Destroy();
	return 0x737F74;
}

DEFINE_HOOK(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExtContainer::Instance.Find(pThis);

	if (!Extension->ReceiveDamage) {
		AnimTypeExtData::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_MarkCell(pThis);
	return 0;
}

DEFINE_HOOK(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_Spawn(pThis);
	return 0;
}