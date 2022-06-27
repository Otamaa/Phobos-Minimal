#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Utilities/LambdaPatcher.h>

namespace FetchBomb {
	BombClass* pThisBomb;
}

DEFINE_HOOK(0x438771, BombClass_Detonate_fetch, 0x6)
{
	GET(BombClass*, pThis, ESI);
	FetchBomb::pThisBomb = pThis;
	return 0x0;
}

static DamageAreaResult __fastcall BombClass_Detonate_DamageArea(const args_DamageArea& nArgs)
{
	auto pThisBomb = FetchBomb::pThisBomb;
	auto OwningHouse = pThisBomb->GetOwningHouse();

	if (auto const pExt = BombExt::GetExtData(pThisBomb)) {
		const char* pHouse = OwningHouse ? OwningHouse->get_ID() : NONE_STR;
		Debug::Log("Extension for Bomb[%x] found House = %s ! \n", pExt , pHouse);
	}

	auto nCoord = *nArgs.Coord;
	auto nResult = Map.DamageArea(nCoord, nArgs.Damage, nArgs.Source, nArgs.Warhead, nArgs.Warhead->Tiberium, OwningHouse);
				   Map.FlashbangWarheadAt(nArgs.Damage, nArgs.Warhead, nCoord);

	if (auto pAnimType = Map.SelectDamageAnimation(nArgs.Damage, nArgs.Warhead, Map[nCoord]->LandType, nCoord)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false)) {
			if (AnimExt::SetAnimOwnerHouseKind(pAnim, OwningHouse, pThisBomb->Target ? pThisBomb->Target->GetOwningHouse() : nullptr, false))
				if (auto const pAnimExt = AnimExt::GetExtData(pAnim))
					pAnimExt->Invoker = pThisBomb->Owner;
		}
	}

	FetchBomb::pThisBomb = nullptr;
	return nResult;
}

// skip the Explosion Anim block
DEFINE_LJMP(0x4387A8, 0x438857);
DEFINE_POINTER_CALL(0x4387A3, &BombClass_Detonate_DamageArea);

HouseClass* __fastcall BombClass_GetOwningHouse_Wrapper(BombClass* pThis, void* _)
{ return pThis->Owner ? pThis->Owner->Owner : pThis->OwnerHouse; }

DEFINE_VTABLE_PATCH(0x7E3D4C, &BombClass_GetOwningHouse_Wrapper);