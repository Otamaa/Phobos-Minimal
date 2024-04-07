#include "Body.h"


DEFINE_HOOK(0x6913F8, ScriptClass_CTOR, 0x5)
{
	GET(ScriptClass* const, pThis, ESI);
	ScriptExtData::ExtMap.FindOrAllocate(pThis);
	return 0x0;
}

DEFINE_HOOK_AGAIN(0x691F06, ScriptClass_DTOR, 0x6)
DEFINE_HOOK(0x691486, ScriptClass_DTOR, 0x6)
{
	GET(ScriptClass*, pThis, ESI);
	ScriptExtData::ExtMap.Remove(pThis);
	return 0x0;
}


DEFINE_HOOK_AGAIN(0x691690, ScriptClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x691630, ScriptClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ScriptClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	ScriptExtData::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x69166F, ScriptClass_Load_Suffix, 0x9)
{
	GET(ScriptClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Type);
	TeamExtContainer::Instance.LoadStatic();

	return 0x69167D;
}

DEFINE_HOOK(0x6916A4, ScriptClass_Save_Suffix, 0x6)
{
	GET(HRESULT const, nRes, EAX);

	if (SUCCEEDED(nRes))
	{
		TeamExtContainer::Instance.SaveStatic();
		return 0x6916A8;
	}

	return 0x6916AA;
}