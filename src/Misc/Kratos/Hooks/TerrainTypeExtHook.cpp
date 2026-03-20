#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TerrainTypeExt.h>

#ifdef _ENABLE_HOOKS

ASMJIT_PATCH(0x71DBC0, TerrainTypeClass_CTOR, 0x7)
{
	if (!Phobos::Otamaa::DoingLoadGame)
	{
		GET(TerrainTypeClass *, pItem, ESI);

		// Override the default value (true) from game constructor.
		pItem->RadarInvisible = false;

		TerrainTypeExt::ExtMap.TryAllocate(pItem);
	}

	return 0;
}

ASMJIT_PATCH(0x71E364, TerrainTypeClass_SDDTOR, 0x6)
{
	GET(TerrainTypeClass *, pItem, ECX);

	TerrainTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x71E1D0, TerrainTypeClass_SaveLoad_Prefix, 0x5)
ASMJIT_PATCH(0x71E240, TerrainTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TerrainTypeClass *, pItem, 0x4);
	GET_STACK(IStream *, pStm, 0x8);

	TerrainTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x71E235, TerrainTypeClass_Load_Suffix, 0x5)
{
	TerrainTypeExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x71E25A, TerrainTypeClass_Save_Suffix, 0x5)
{
	TerrainTypeExt::ExtMap.SaveStatic();

	return 0;
}

ASMJIT_PATCH(0x71E0A6, TerrainTypeClass_LoadFromINI, 0x5)
{
	GET(TerrainTypeClass *, pItem, ESI);
	GET_STACK(CCINIClass *, pINI, STACK_OFFSET(0x210, 0x4));

	TerrainTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
#endif