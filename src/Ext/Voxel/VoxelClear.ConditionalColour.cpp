// ===========================================================================
// Voxel_Conditional_Clear_VoxelSurfaceBuffer1  -  vanilla 0x754740 .. 0x7547B3
//   __fastcall(int x /*ecx*/, int y /*edx*/, int width, int height), retn 8
//
// The sibling of 0x7547C0. Same job, but the rect arrives as four arguments
// instead of behind a pointer.
//
// PATCH AUDIT - your five entries here are ALL CORRECT
//   0x754752  cmp eax,0FFh           OK
//   0x75475F  cmp eax,0FFh           OK
//   0x754776  lea esi,[esi+ecx+d32]  OK   (SIB, +3)
//   0x75478D  add esi,100h           OK
//   0x7547A8  mov edi,offset         DEAD (inside your 0x7547A0 hook)
//
//   0x754771  shl esi,8              MISSING  imm8, log2 of the row stride
//   0x7547A1  mov ecx,4000h          MISSING  (dead anyway, in your hook range)
//
// So the only real gap was the shift exponent - the same one missed in 0x7547C0.
// Both disappear here.
//
// IDA's parameter names (a1, bufferentry, count, entryid) are misleading; the
// comparisons against 255 and the `Width + 1` byte count make it the same
// X / Y / Width / Height rect as VoxelClippingRect, with Width and Height as
// INCLUSIVE maxima.
//
// SUSPECT: the return value. Vanilla sets EAX to 0 on both clearing paths, but
// the early-out at `cmp edx,eax ; jg loc_7547AE` leaves EAX holding Y + Height.
// Reproduced exactly in case a caller reads it - though nothing seen so far does.
//
// BUG (vanilla, preserved): a negative Width makes `Width + 1` non-positive and
// the vanilla `shr ecx,2 ; rep stosd` walks off the end. None of the guards catch
// it. Same as 0x7547C0.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstring>

// ---------------------------------------------------------------------------
// __fastcall puts x in ECX and y in EDX, leaves width and height on the stack,
// and cleans 8 bytes on return - byte-compatible with vanilla's `retn 8`.
// ---------------------------------------------------------------------------
static int __fastcall VoxelClear_ConditionalColour(
	int rectX, int rectY, int rectW, int rectH) noexcept
{
	if (rectX < 0
		|| rectY < 0
		|| rectX + rectW > Replacer::BufferSize - 1
		|| rectY + rectH > Replacer::BufferSize - 1)
	{
		// loc_7547A0
		std::memset(Replacer::VoxelPixelBuffer, 0,
			sizeof(Replacer::VoxelPixelBuffer));
		return 0;
	}

	const int lastRow = rectY + rectH;

	// cmp edx, eax ; jg loc_7547AE - only reachable with a negative Height.
	// SUSPECT: EAX still holds Y + Height on this path.
	if (rectY > lastRow)
	{
		return lastRow;
	}

	// BUG (vanilla, preserved): non-positive with a negative Width.
	const std::size_t rowBytes = static_cast<std::size_t>(rectW) + 1;

	for (int row = rectY; row <= lastRow; ++row)
	{
		std::memset(&Replacer::VoxelPixelBuffer[row][rectX], 0, rowBytes);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// DELETE your ASMJIT_PATCH hook at 0x7547A0 - this replaces it.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x754740, VoxelClear_ConditionalColour)
