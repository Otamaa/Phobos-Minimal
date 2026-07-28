#include "VoxelBufferReplace.h"
#include <Utilities/Macro.h>
alignas(64) std::uint8_t Replacer::VoxelPixelBuffer[Replacer::BufferSize][Replacer::BufferSize];

void __cdecl Replacer::_De_Apply()
{
	VoxelSurface->~BSurface();
}

void __fastcall Replacer::_Apply()
{
	// VERIFY: BytesPerPixel must remain 1, otherwise Get_Pitch() diverges from
	// the array stride even when Width is correct.
	VoxelSurface->BytesPerPixel = 1;
	VoxelSurface->Width = BufferSize;
	VoxelSurface->Height = BufferSize;

	// VERIFY: the vanilla BufferPtr points at the static VoxelPixelBuffer and
	// is therefore NOT owned by the surface. Confirm MemoryBuffer's assignment
	// operator does not try to free the previous pointer, or this leaks/faults.
	VoxelSurface->BufferPtr = MemoryBuffer(VoxelPixelBuffer, sizeof(VoxelPixelBuffer));

	// Left commented in the original. Correct to leave it that way: the clip
	// rect is recomputed per-voxel from the projected bounds by
	// Voxel_Init_Surface_Stuff, and the rasterizer cannot address past 255
	// on either axis regardless of what is written here.
	// VoxelClippingRect = RectangleStruct(0, 0, RasterMaxCoord, RasterMaxCoord);

	atexit(_De_Apply);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7539D0 , Replacer::_Apply)