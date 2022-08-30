#include "Body.h"

DEFINE_HOOK(0x438771, BombClass_Detonate_fetch, 0x6)
{
	GET(BombClass*, pThis, ESI);
	BombExt::BombTemp = pThis;
	return 0x0;
}

DEFINE_JUMP(CALL, 0x4387A3, GET_OFFSET(BombExt::DamageArea));
// skip the Explosion Anim block
DEFINE_HOOK(0x4387A8, BombClass_Detonate_ExplosionAnimHandled, 0x5)
{
	BombExt::BombTemp = nullptr;
	return 0x438857;
}

DEFINE_JUMP(VTABLE,0x7E3D4C,GET_OFFSET(BombExt::GetOwningHouse));