// ===========================================================================
// Voxel_Clear_Voxel_Surface_spec_Buffer_2  -  vanilla 0x754840 .. 0x754851
//   __fastcall, no arguments. Full clear of the depth buffer.
//
//   .text:00754841  B9 00 40 00 00   mov ecx, 4000h    imm32 at 0x754842  MISSING
//   .text:00754848  BF E0 D5 B1 00   mov edi, offset   imm32 at 0x754849  MISSING
//
// Both were in the depth-relocation list, neither was in your patch list.
// Ported, so both can be dropped.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstring>

static void __fastcall VoxelClear_DepthFull() noexcept
{
	std::memset(VoxelRaster::DepthBuffer, 0, sizeof(VoxelRaster::DepthBuffer));
}

DEFINE_FUNCTION_JUMP(LJMP, 0x754840, VoxelClear_DepthFull)
