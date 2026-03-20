#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/BulletTypeExt.h>
#include <Ext/BulletType/Body.h>

#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(BulletTypeClass *, pItem, EAX);
		BulletTypeExtContainer::Instance.Allocate(pItem);
		BulletTypeExt::ExtMap.TryAllocate(pItem);
	}
	return 0;
}

ASMJIT_PATCH(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass *, pItem, ESI);
	BulletTypeExtContainer::Instance.Remove(pItem);
	BulletTypeExt::ExtMap.Remove(pItem);

	return 0;
}

bool FakeBulletTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->BulletTypeClass::LoadFromINI(pINI);
	BulletTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	BulletTypeExt::ExtMap.LoadFromINI(this, pINI);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E49AC, FakeBulletTypeClass::_ReadFromINI)
#endif