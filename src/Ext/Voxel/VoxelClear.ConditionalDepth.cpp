// ===========================================================================
// Voxel_Conditional_Clear_spec_VoxelBufferedPixelBuffer     0x754860 .. 0x7548A6
// Voxel_Conditional_Clear_spec_VoxelBufferedPixelBuffer_0   0x7548B0 .. 0x7548FB
//
// The last two clear helpers. Same rect clear as the colour pair, on the depth
// buffer - one taking four arguments, one taking a rect pointer.
//
// PATCH AUDIT - none of these were in your list
//   0x754877  shl esi,8              MISSING
//   0x75487D  lea esi,[esi+ecx+d32]  MISSING  (SIB, +3)
//   0x754894  add esi,100h           MISSING
//   0x7548C6  shl ebx,8              MISSING
//   0x7548D4  lea edi,[ebx+edi+d32]  MISSING  (SIB, +3)
//   0x7548E1  add ebx,100h           MISSING
//
// ###########################################################################
// # NEITHER ONE VALIDATES ITS BOUNDS                                        #
// ###########################################################################
//
// The colour versions (0x754740, 0x7547C0) both open with four checks - X < 0,
// Y < 0, X+Width > 255, Y+Height > 255 - and fall back to a full clear if any
// fail. These two have NOTHING of the sort. The only guard is
//
//     cmp edx, eax ; jg   ->  return if Y > Y + Height
//
// which only catches a negative Height. So the depth buffer is cleared entirely
// on the caller's word.
//
// That is the third and last piece of the out-of-bounds clear traced through
// this work, and it is the reason the original 512 attempt was corrupting
// memory: raising the colour bounds to BufferSize-1 lets a bigger rect through,
// and these two then write it straight into a 256-wide depth buffer. Immediately
// past that buffer sit VoxelClippingMax, both BSurface objects and VoxelQueue.
//
// Preserved as-is - adding a guard would be a behaviour change, and once both
// buffers are the same size the caller's rect is valid by construction.
//
// SUSPECT: IDA types the second one's argument as SomeVoxelStruct*, same
// mistake as 0x7547C0. The accesses are [+0], [+4], [+8], [+0xC] as ints - it is
// a rect.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstring>

namespace
{
	// Shared with the colour helpers in spirit; kept local to avoid coupling the
	// two translation units.
	void ClearDepthRect(int x, int y, int width, int height) noexcept
	{
		const int lastRow = y + height;

		// cmp edx, eax ; jg - the only guard vanilla has.
		if (y > lastRow)
		{
			return;
		}

		// BUG (vanilla, preserved): non-positive with a negative Width, which the
		// vanilla `shr ecx,2 ; rep stosd` then turns into a huge unsigned count.
		// * element size: the depth buffer is 16-bit now.
		const std::size_t rowBytes = (static_cast<std::size_t>(width) + 1)
			* sizeof(VoxelRaster::DepthBuffer[0][0]);

		for (int row = y; row <= lastRow; ++row)
		{
			std::memset(&VoxelRaster::DepthBuffer[row][x], 0, rowBytes);
		}
	}
}

// ---------------------------------------------------------------------------
// 0x754860 - __fastcall(x /*ecx*/, y /*edx*/, width, height), retn 8.
// ---------------------------------------------------------------------------
static void __fastcall VoxelClear_ConditionalDepth(
	int rectX, int rectY, int rectW, int rectH) noexcept
{
	ClearDepthRect(rectX, rectY, rectW, rectH);
}

// ---------------------------------------------------------------------------
// 0x7548B0 - __fastcall(RectangleStruct* /*ecx*/), plain retn.
// ---------------------------------------------------------------------------
static void __fastcall VoxelClear_ConditionalDepth_0(
	RectangleStruct* pRect) noexcept
{
	ClearDepthRect(pRect->X, pRect->Y, pRect->Width, pRect->Height);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x754860, VoxelClear_ConditionalDepth)
DEFINE_FUNCTION_JUMP(LJMP, 0x7548B0, VoxelClear_ConditionalDepth_0)
