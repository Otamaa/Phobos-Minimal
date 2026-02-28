#include <exception>
#include <Windows.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/VoxelAnimExt.h>

#ifdef _ENABLE_HOOKS

// ----------------
// Extension
// ----------------


//ASMJIT_PATCH(0x749951, VoxelAnimClass_CTOR, 0xC)
ASMJIT_PATCH(0x74942E, VoxelAnimClass_CTOR, 0xC)
{
	GET(VoxelAnimClass*, pItem, ESI);

	VoxelAnimExt::ExtMap.TryAllocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x7499F1, VoxelAnimClass_DTOR, 0x5)
{
	GET(VoxelAnimClass*, pItem, ECX);

	VoxelAnimExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x74A970, VoxelAnimClass_SaveLoad_Prefix, 0x5)
ASMJIT_PATCH(0x74AA10, VoxelAnimClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(VoxelAnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x74A9FB, VoxelAnimClass_Load_Suffix, 0x7)
{
	VoxelAnimExt::ExtMap.LoadStatic();
	return 0;
}

ASMJIT_PATCH(0x74AA24, VoxelAnimClass_Save_Suffix, 0x5)
{
	VoxelAnimExt::ExtMap.SaveStatic();
	return 0;
}
#endif