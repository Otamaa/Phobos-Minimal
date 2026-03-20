#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/SuperWeaponTypeExt.h>

#include <Ext/SWType/Body.h>

#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x6CE6F6, SuperWeaponTypeClass_CTOR, 0x5)
{
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(SuperWeaponTypeClass *, pItem, EAX);
		SWTypeExtContainer::Instance.Allocate(pItem);
		SuperWeaponTypeExt::ExtMap.TryAllocate(pItem);
	}

	return 0;
}

ASMJIT_PATCH(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass *, pItem, ECX);
	SWTypeExtContainer::Instance.Remove(pItem);
	SuperWeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

bool FakeSuperWeaponTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	//read some properties early before
	bool status = SWTypeExtContainer::Instance.Find(this)->PreParse(pINI);
	status |= this->SuperWeaponTypeClass::LoadFromINI(pINI);
	SWTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	SuperWeaponTypeExt::ExtMap.LoadFromINI(this, pINI);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F40F4, FakeSuperWeaponTypeClass::_ReadFromINI)
#endif