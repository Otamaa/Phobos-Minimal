#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TechnoTypeExt.h>

#ifndef _ENABLE_HOOKS

ASMJIT_PATCH(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(TechnoTypeClass *, pItem, ESI);

		TechnoTypeExt::ExtMap.TryAllocate(pItem);
	}
	return 0;
}

ASMJIT_PATCH(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass *, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5)
ASMJIT_PATCH(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass *, pItem, EBP);
	GET_STACK(CCINIClass *, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
#endif