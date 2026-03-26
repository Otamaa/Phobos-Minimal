#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/WeaponTypeExt.h>
#include <Ext/WeaponType/Body.h>

#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x771EE0, WeaponTypeClass_CTOR, 0x6)
{
	GET(WeaponTypeClass *, pItem, ESI);

	if (!Phobos::Otamaa::DoingLoadGame) {
		WeaponTypeExtContainer::Instance.Allocate(pItem);
		WeaponTypeExtData::calculateCircuferences();
		WeaponTypeExt::ExtMap.TryAllocate(pItem);
	}

	return 0;
}

ASMJIT_PATCH(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass *, pItem, ESI);
	WeaponTypeExtContainer::Instance.Remove(pItem);
	WeaponTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
ASMJIT_PATCH(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass *, pItem, 0x4);
	GET_STACK(IStream *, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}


bool FakeWeaponTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	//WeaponTypeExtContainer::Instance.Find(this)->RadType = RadTypeClass::FindOrAllocate(GameStrings::Radiation());
	bool status = this->WeaponTypeClass::LoadFromINI(pINI);
	WeaponTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	WeaponTypeExt::ExtMap.LoadFromINI(this, pINI);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F741C, FakeWeaponTypeClass::_ReadFromINI)
#endif