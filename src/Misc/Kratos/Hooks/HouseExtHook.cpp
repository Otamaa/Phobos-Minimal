#include <exception>
#include <Windows.h>

#include <HouseClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Extension/HouseExt.h>

#ifdef _ENABLE_HOOKS
// ----------------
// Extension
// ----------------

ASMJIT_PATCH(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.TryAllocate(pItem);

	if (AIConfig::Data()->EnablePowerSurplus)
		pItem->PowerSurplus = RulesClass::Instance->PowerSurplus;

	return 0;
}

ASMJIT_PATCH(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
ASMJIT_PATCH(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();

	return 0;
}

ASMJIT_PATCH(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}
#endif