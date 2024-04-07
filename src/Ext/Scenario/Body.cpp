#include "Body.h"

DEFINE_HOOK(0x683549, ScenarioClass_CTOR, 0x9)
{
	GET(ScenarioClass*, pItem, EAX);
	ScenarioExtData::Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6BEB7D, ScenarioClass_DTOR, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);
	ScenarioExtData::Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x689470, ScenarioClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x689310, ScenarioClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);
	ScenarioExtData::g_pStm = pStm;
	return 0;
}

DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	return 0;
}

DEFINE_HOOK(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	return 0;
}