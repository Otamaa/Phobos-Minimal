// ===========================================================================
// Voxel_Conditional_Clear_VoxelSurfaceBuffer1_0
//   vanilla 0x7547C0 .. 0x75483A   __fastcall, one argument in ECX
//
// The colour buffer's rect-bounded clear. Not a rasterizer, so it was outside
// everything analysed so far - but it carries SEVEN hardcoded geometry
// constants, which is more than any rasterizer.
//
// ###########################################################################
// # PATCH AUDIT FOR THIS FUNCTION: 2 right, 1 WRONG, 4 MISSING               #
// ###########################################################################
//
//   0x7547D6  81 F9 FF 00 00 00   cmp ecx, 0FFh
//                                 imm32 at 0x7547D8   your list: 0x7547D8   OK
//
//   0x7547E3  3D FF 00 00 00      cmp eax, 0FFh
//                                 imm32 at 0x7547E4   your list: 0x7547E4   OK
//
//   0x7547F2  C1 E3 08            shl ebx, 8
//                                 imm8  at 0x7547F4   MISSING
//                                 log2(stride). Patching this constrains the
//                                 buffer to a power-of-two width.
//
//   0x7547FF  8D BC 3B 78 FF B2 00  lea edi, VoxelPixelBuffer[ebx+edi]
//                                 ^^ ^^ ^^ \__________/
//                                 |  |  SIB 0x3B
//                                 |  ModRM 0xBC -> mod 10, reg EDI, rm 100b
//                                 disp32 at 0x754802  your list: 0x754803  WRONG
//
//   0x75480D  81 C3 00 01 00 00   add ebx, 100h
//                                 imm32 at 0x75480F   MISSING  (row stride)
//
//   0x75482A  B9 00 40 00 00      mov ecx, 4000h
//                                 imm32 at 0x75482B   MISSING  (dword count
//                                 for the full clear: 0x4000 * 4 == 0x10000)
//
//   0x754831  BF 78 FF B2 00      mov edi, offset VoxelPixelBuffer
//                                 imm32 at 0x754832   your list: 0x754832   OK
//
// That makes 0x754803 the FIFTH wrong entry found. Updated tally: 5 wrong.
//
// Note your sibling patch 0x75478D patches a row stride in the OTHER clear
// helper (around 0x754740), so the equivalent was caught there and missed here.
// Worth re-checking that function for its own `shl` and dword-count constants.
//
// ###########################################################################
//
// WHY PORTING BEATS PATCHING HERE
// -------------------------------
// Seven constants, one of which is a shift exponent rather than a value. Get any
// one wrong and the clear either misses rows or runs off the buffer - and the
// buffer is immediately followed by VoxelShadowQueue at 0xB3FF78. In C++ they
// all collapse into Replacer::BufferSize.
//
// THE ARGUMENT IS A RECT, NOT A VoxelDrawStruct
// ---------------------------------------------
// IDA typed it `SomeVoxelStruct*`, which is why the pseudocode reads as nonsense
// (`a1->DataStartPtr < 0`). The accesses are [edx+0], [+4], [+8], [+0xC] as
// signed ints, and the values are compared against 255 - it is the same layout
// as VoxelClippingRect at 0xB2FB60, initialised to <0, 0, 255, 255>:
//
//     +0x00 int X    +0x04 int Y    +0x08 int Width    +0x0C int Height
//
// Width and Height are INCLUSIVE maxima, hence the `Width + 1` byte count and
// the `<=` loop bound.
//
// VERIFY: confirm the caller passes VoxelClippingRect (or another rect) rather
// than something else that merely happens to fit.
//
// VANILLA BEHAVIOUR PRESERVED
//   * bounds test is `> 255` on X+Width and Y+Height, and `< 0` on X and Y;
//     failing any of them clears the WHOLE buffer instead of the rect
//   * the `cmp esi, eax ; jg` guard skips the loop entirely when Y > Y+Height,
//     which can only happen if Height is negative
//   * BUG: a negative Width makes `Width + 1` non-positive, and vanilla then does
//     `shr ecx, 2 ; rep stosd` on a huge unsigned count. None of the guards catch
//     it. Reproduced as-is - a negative Width in a clipping rect would already
//     have been catastrophic.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstring>

// ---------------------------------------------------------------------------
// Goto-free backport. __fastcall with a single register argument, so vanilla's
// bare `retn` and MSVC's __fastcall epilogue agree.
// ---------------------------------------------------------------------------
static void __fastcall VoxelClear_ConditionalColour_0(RectangleStruct* pRect) noexcept
{
	const int rectX = pRect->X;
	const int rectY = pRect->Y;
	const int rectW = pRect->Width;
	const int rectH = pRect->Height;

	// Vanilla: jl / jl / jg / jg, all falling through to the full clear.
	// The two 255s were the only constants here you had patched.
	if (rectX < 0
		|| rectY < 0
		|| rectX + rectW > Replacer::BufferSize - 1
		|| rectY + rectH > Replacer::BufferSize - 1)
	{
		// loc_75482A: mov ecx, 4000h / mov edi, offset / rep stosd
		std::memset(Replacer::VoxelPixelBuffer, 0,
			sizeof(Replacer::VoxelPixelBuffer));
		return;
	}

	const int lastRow = rectY + rectH;

	// cmp esi, eax ; jg loc_754838 - only reachable with a negative Height.
	if (rectY > lastRow)
	{
		return;
	}

	// BUG (vanilla, preserved): a negative Width makes this count non-positive
	// and the vanilla `shr ecx,2 ; rep stosd` walks off the end. No guard added.
	const std::size_t rowBytes = static_cast<std::size_t>(rectW) + 1;

	// Vanilla tracks a byte offset (`shl ebx, 8` then `add ebx, 100h`); indexing
	// by row does the same thing without hardcoding the stride or requiring it to
	// be a power of two.
	for (int row = rectY; row <= lastRow; ++row)
	{
		std::memset(&Replacer::VoxelPixelBuffer[row][rectX], 0, rowBytes);
	}
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
//
// Vanilla is __fastcall with one argument in ECX and a bare `retn`, which is
// exactly what MSVC emits for a single-pointer __fastcall - no dummy EDX
// parameter needed here, unlike the __thiscall case in Draw_Shadow.
//
// Once this is active, DELETE all seven patch sites listed above, including the
// two that were correct.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7547C0, VoxelClear_ConditionalColour_0)
