#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/AnimTypeExt.h>

#include <Ext/AnimType/Body.h>
#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(AnimTypeClass *, pItem, EAX);
		AnimTypeExtContainer::Instance.Allocate(pItem);
		AnimTypeExt::ExtMap.TryAllocate(pItem);
	}
	return 0;
}

ASMJIT_PATCH(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass *, pItem, ECX);
	AnimTypeExtContainer::Instance.Remove(pItem);
	AnimTypeExt::ExtMap.Remove(pItem);
	return 0;
}

bool FakeAnimTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->AnimTypeClass::LoadFromINI(pINI);
	AnimTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	AnimTypeExt::ExtMap.LoadFromINI(this, pINI);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E366C, FakeAnimTypeClass::_ReadFromINI)
#endif