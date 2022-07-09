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

static DamageAreaResult __fastcall BombClass_Detonate_DamageArea(
	CoordStruct* pCoord,
	int Damage,
	TechnoClass* Source,
	WarheadTypeClass* Warhead,
	bool AffectTiberium,
	HouseClass* SourceHouse
)
{
	auto pThisBomb = FetchBomb::pThisBomb;
	auto OwningHouse = pThisBomb->GetOwningHouse();

	//if (auto const pExt = BombExt::GetExtData(pThisBomb)) {
	//	const char* pHouse = OwningHouse ? OwningHouse->get_ID() : NONE_STR;
	//	Debug::Log("Extension for Bomb[%x] found House = %s ! \n", pExt , pHouse);
	//}

	const auto nCoord = *pCoord;
	const auto nResult = Map.DamageArea(nCoord, Damage, Source, Warhead, Warhead->Tiberium, OwningHouse);
				   Map.FlashbangWarheadAt(Damage, Warhead, nCoord);

	const auto pCell = Map.TryGetCellAt(nCoord);
	if (auto pAnimType = Map.SelectDamageAnimation(Damage, Warhead, pCell? pCell->LandType:LandType::Clear, nCoord)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, OwningHouse, pThisBomb->Target ? pThisBomb->Target->GetOwningHouse() : nullptr, pThisBomb->Owner, false);
		}
	}

	FetchBomb::pThisBomb = nullptr;
	return nResult;
}

// skip the Explosion Anim block
DEFINE_JUMP(LJMP,0x4387A8, 0x438857);
DEFINE_JUMP(CALL,0x4387A3, GET_OFFSET(BombClass_Detonate_DamageArea));

static HouseClass* __fastcall BombClass_GetOwningHouse_Wrapper(BombClass* pThis, void* _)
{ return pThis->Owner ? pThis->Owner->Owner : pThis->OwnerHouse; }

DEFINE_JUMP(VTABLE,0x7E3D4C,GET_OFFSET(BombClass_GetOwningHouse_Wrapper));