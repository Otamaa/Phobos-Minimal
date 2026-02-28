#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/BulletTypeExt.h>

#ifdef _ENABLE_HOOKS

ASMJIT_PATCH(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	if (!Common::IsLoadGame)
	{
		GET(BulletTypeClass *, pItem, EAX);

		BulletTypeExt::ExtMap.TryAllocate(pItem);
	}
	return 0;
}

ASMJIT_PATCH(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass *, pItem, ESI);

	BulletTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
ASMJIT_PATCH(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeClass *, pItem, 0x4);
	GET_STACK(IStream *, pStm, 0x8);

	BulletTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x46C722, BulletTypeClass_Load_Suffix, 0x4)
{
	BulletTypeExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x46C74A, BulletTypeClass_Save_Suffix, 0x3)
{
	BulletTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
ASMJIT_PATCH(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass *, pItem, ESI);
	GET_STACK(CCINIClass *, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

#endif