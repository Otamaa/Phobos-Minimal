// ===========================================================================
// Voxel_Draw_Function_Tbl entries "24 / 28, plain, startptr"
//   vanilla 0x757E70 .. 0x758025
//   (IDA names it _9_24_28_, i.e. it is referenced from two table slots)
//
// Plain forward walk again - but with a DIFFERENT second-pixel address, which
// is the whole point of this variant.
//
// ###########################################################################
// # RETRACTION - MY FIRST ANALYSIS FLAGGED THESE TWO WRONGLY                #
// ###########################################################################
//
// In my very first pass over VoxelBufferReplace.cpp I flagged this as suspect:
//
//     DEFINE_PATCH_TYPED(DWORD, 0x757F81, DWORD(&Replacer::VoxelPixelBuffer))
//     DEFINE_PATCH_TYPED(DWORD, 0x757F87, DWORD(&Replacer::VoxelPixelBuffer))
//                                          ^ "SUSPECT: missing + 1"
//
// That was wrong. BOTH ENTRIES ARE CORRECT, address and value:
//
//   .text:00757F7F  88 93 78 FF B2 00  mov VoxelPixelBuffer[ebx], dl
//                                      disp32 at 0x757F81, value = base       OK
//   .text:00757F85  88 97 78 FF B2 00  mov VoxelPixelBuffer[edi], dl
//                                      disp32 at 0x757F87, value = base       OK
//
// The `+ 1` is folded into the INDEX REGISTER here, not the displacement:
//
//   .text:00757F6E  and  edi, 0FF00h     ; edi = Yint << 8
//   .text:00757F74  shr  eax, 8          ; eax = Xint
//   .text:00757F77  mov  ebx, eax
//   .text:00757F7A  or   ebx, edi        ; ebx = (Yint << 8) | Xint
//   .text:00757F7C  inc  edi             ; edi = (Yint << 8) | 1
//   .text:00757F7D  or   edi, eax        ; edi = (Yint << 8) | (Xint | 1)
//
// So the second address is (x | 1), NOT (x + 1). Not the same thing:
//
//   even x -> writes x and x+1   (a real pair)
//   odd  x -> writes x twice     (a single pixel)
//
// Side effect: since Xint never leaves 0..255, this form cannot carry into the
// next row, so this variant is free of the row-bleed bug that (buffer+1)[idx]
// has in fn 18 / 19 / 22 / 23 and Draw_Shadow.
//
// The other unexplained pair in your list, 0x758118 / 0x75811E, is almost
// certainly the same construct in the sibling function - check for `inc` on the
// index register before assuming a dropped `+ 1`.
//
// Running tally stays at 4 wrong out of 20 (0x756EDF, 0x757063, 0x75728B,
// 0x757291). This function's two entries are fine.
//
// ###########################################################################
//
// REGISTER MAP (swapped relative to fn 16 - watch for this in the pseudocode)
//   EBP = X accumulator, register-live
//   EDI = Y accumulator, mirrored in the incoming argument slot
//   EDX = zRemaining, mirrored in var_18 because EDX doubles as the colour byte
//
//   .text:00757F6E  and  edi, 0FF00h            ; destroys the Y accumulator
//   .text:00757F8F  mov  edi, [esp+38h+a1]      ; reloaded from the mirror
//
// The 16-bit limits are unchanged:
//
//   .text:00757EC2  mov  bp, [esi+18h]   ; Start.X into the low half of EBP
//   .text:00757EC6  mov  di, [esi+1Ah]   ; Start.Y into the low half of EDI
//   .text:00757F69  and  eax, 0FFFFh
//   .text:00757F74  shr  eax, 8
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_757FB7` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_PlainLsb_StartPtr(VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: widened to real int32 8.8 values. No Z accumulator in this variant.
	int rowX = pDraw->StartX;
	int rowY = pDraw->StartY;

	for (int y = 0; y < sizeY; ++y)
	{
		const int savedRowX = rowX;
		const int savedRowY = rowY;
		const int savedDataPos = pDraw->DataPos;

		int accX = rowX;
		int accY = rowY;

		for (int x = 0; x < sizeX; ++x)
		{
			const int columnX = accX;
			const int columnY = accY;

			// startptr variant -> the START table (mov edx,[esi] @ 0x757F00).
			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			if (spanOffset != -1 && sizeZ != 0)
			{
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// BUG (vanilla, preserved): terminator is `!= 0`, not `> 0`.
				do
				{
					const int skipCount = *pSpan++;
					zRemaining -= skipCount;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					int runCount = *pSpan++;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							const std::uint8_t colour = pSpan[0];
							// Note: this variant advances by ONE byte per voxel,
							// not two - there is no normal byte in the stream it
							// consumes (inc ecx @ 0x757F79, single increment).
							pSpan += 1;

							// Second pixel at (x | 1), not (x + 1).
							PutPixelPairSetLsb(ToWhole(accX), ToWhole(accY), colour);

							accX += stepZX;
							accY += stepZY;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc ecx @ 0x757FAE).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_757FB7.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_757FEA.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		accX = rowX;
		accY = rowY;

		pDraw->DataPos = savedDataPos + pDraw->YSteps;
	}
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
//
// __cdecl with a single stack argument, so the jump target inherits exactly the
// frame the original expected: ESP -> return address, argument at [ESP+4]. Our
// `ret` returns straight to the original caller.
//
// Instruction alignment does NOT matter here (unlike ASMJIT_PATCH): the 5 bytes
// LJMP overwrites are never executed again, and nothing jumps into the middle of
// the function from outside.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x757E70, VoxelDraw_PlainLsb_StartPtr)
