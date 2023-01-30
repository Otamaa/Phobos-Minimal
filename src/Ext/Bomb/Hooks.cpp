#include "Body.h"

DEFINE_HOOK(0x438764, BombClass_Detonate_fetch, 0x7)
{
	GET(BombClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, ECX);

	BombExt::BombTemp = pThis;

	pTarget->BombVisible = false;
	// Also adjust detonation coordinate.
	const CoordStruct coords = pTarget->GetCenterCoords();

	R->EDX(&coords);
	return 0x438771;
}

DEFINE_JUMP(CALL, 0x4387A3, GET_OFFSET(BombExt::DamageArea));

// skip the Explosion Anim block
DEFINE_HOOK(0x4387A8, BombClass_Detonate_ExplosionAnimHandled, 0x5)
{
	BombExt::BombTemp = nullptr;
	return 0x438857;
}

DEFINE_JUMP(VTABLE,0x7E3D4C,GET_OFFSET(BombExt::GetOwningHouse));

DEFINE_HOOK(0x6F51F8, TechnoClass_DrawExtras_IvanBombImage, 0x9)
{
	GET(TechnoClass*, pThis, EBP);
	GET(CoordStruct*, pCoordBuffA, ECX);
	R->EAX(pThis->GetCenterCoords(pCoordBuffA));
	return 0x6F5201;
}