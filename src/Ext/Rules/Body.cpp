#include "Body.h"

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExtData::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExtData::Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	if (R->Origin() == 0x675210){
		pItem->BarrelDebris.Clear();
		pItem->DeadBodies.Clear();
		pItem->DropPod.Clear();
		pItem->MetallicDebris.Clear();
		pItem->BridgeExplosions.Clear();
		pItem->DamageFireTypes.Clear();
		pItem->WeatherConClouds.Clear();
		pItem->WeatherConBolts.Clear();
	}

	RulesExtData::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{

	return 0;
}