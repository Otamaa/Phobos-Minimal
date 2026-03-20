#include <exception>
#include <Windows.h>

#include <HouseClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Extension/HouseExt.h>

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>

#ifndef _ENABLE_HOOKS
// ----------------
// Extension
// ----------------

ASMJIT_PATCH(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	if (RulesExtData::Instance()->EnablePowerSurplus)
		pItem->PowerSurplus = RulesClass::Instance->PowerSurplus;

	HouseExtContainer::Instance.Allocate(pItem);

	HouseExt::ExtMap.TryAllocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExtContainer::Instance.Remove(pItem);
	HouseExt::ExtMap.Remove(pItem);

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