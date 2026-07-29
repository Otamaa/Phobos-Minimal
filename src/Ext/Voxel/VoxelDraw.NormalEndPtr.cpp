// ===========================================================================
// Voxel_Draw_Function_Tbl entry "21, normal, endptr"
//   vanilla 0x757790 .. 0x757972
//
// fn 17's backward span walk plus fn 20's shading. No depth test.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:007578AF  88 90 78 FF B2 00  -> disp32 at 0x7578B1, your list: 0x7578B1  OK
//   .text:007578B5  88 90 79 FF B2 00  -> disp32 at 0x7578B7, your list: 0x7578B7  OK
//
// Running tally: 4 wrong out of 14 (0x756EDF, 0x757063, 0x75728B, 0x757291).
//
// PAIR ORDER CONFIRMED FROM BOTH DIRECTIONS
// -----------------------------------------
// This is the first backward variant that actually reads the normal byte, so it
// pins down the { colour, normal } pair order independently:
//
//   .text:0075785A  mov  al, [ecx]        ; normal   <- at P
//   .text:0075785C  dec  ecx
//   .text:0075786B  dec  ecx              ; ecx now P-2
//   .text:00757872  mov  dl, [ecx+1]      ; colour   <- at P-1
//
// So walking down from P: normal at P, colour at P-1, step -2. That matches the
// forward variants exactly (colour at pSpan[0], normal at pSpan[1]) and confirms
// the plain endptr functions (fn 17, fn 19) were right to read their colour from
// pSpan[-1] and ignore pSpan[0].
//
// The lighting is identical to fn 20:
//
//   .text:0075786C  mov  al, VoxelNormalToLut[edx]
//   .text:007578A5  shl  edx, 8
//   .text:007578A8  mov  dl, VPLLookup[edx+ebx]     ; SIB form, disp32 at +3
//
//     shade = VPLLookup[ VoxelNormalToLut[normal] * 256 + colour ]
//
// SUSPECT: as in every endptr variant, the accumulators still step by +AxisZ
// even though blocks arrive in reverse Z order, so the caller must supply a
// pre-negated AxisZ and a far-end Start. Preserved verbatim.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:00757879  mov  edx, edi        ; X accumulator
//   .text:00757885  and  edx, 0FFFFh
//   .text:00757896  shr  edx, 8          ; Xint (0..255)
//   .text:00757883  mov  eax, ebp        ; Y accumulator
//   .text:0075788B  and  eax, 0FF00h     ; Yint << 8
//   .text:00757899  or   eax, edx
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_757904` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_Normal_EndPtr(VoxelRaster::DrawStruct* pDraw) noexcept
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

			// endptr variant -> the END table (mov edx,[esi+4] @ 0x75781A).
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			if (spanOffset != -1 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// BUG (vanilla, preserved): terminator is `!= 0`, not `> 0`.
				do
				{
					int runCount = pSpan[0];
					--pSpan;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							// Reading backwards: normal above, colour below.
							const std::uint8_t normal = pSpan[0];
							const std::uint8_t colour = pSpan[-1];
							pSpan -= 2;

							PutPixelPair(ToWhole(accX), ToWhole(accY),
								Shade(colour, normal));

							accX += stepZX;
							accY += stepZY;
							--runCount;
						}
						while (runCount != 0);
					}

					const int skipCount = pSpan[-1];
					pSpan -= 2;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;
					zRemaining -= skipCount;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_757904.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_757937.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x757790, VoxelDraw_Normal_EndPtr)
