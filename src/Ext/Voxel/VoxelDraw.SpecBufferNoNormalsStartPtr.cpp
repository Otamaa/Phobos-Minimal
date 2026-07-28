// ===========================================================================
// Voxel_Draw_Function_Tbl entries "10 / 14 / 26 / 30, spec buffer, startptr"
//   vanilla 0x7581F0 .. 0x758422
//   (IDA names it _10_14_26_30_, i.e. FOUR table slots point here)
//
// This is fn 18 (0x757120) almost line for line, with one difference that
// changes how the span stream is consumed.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:00758356  88 98 78 FF B2 00  -> disp32 at 0x758358, your list: 0x758358      OK
//   .text:0075835C  88 98 79 FF B2 00  -> disp32 at 0x75835E, your list: 0x75835E (+1) OK
//
// Running tally: 4 wrong out of 24 (0x756EDF, 0x757063, 0x75728B, 0x757291).
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:0075833E  66 0F B6 98 [E0 D5 B1 00]  READ  depth,   disp32 at 0x758342  <- prefix+2-byte opcode, +4
//   .text:00758350  88 90       [E0 D5 B1 00]  write depth,   disp32 at 0x758352
//   .text:00758362  88 90       [E1 D5 B1 00]  write depth+1, disp32 at 0x758364
//
// THE ONE DIFFERENCE FROM fn 18: ONE BYTE PER VOXEL
// -------------------------------------------------
//   fn 18  @ 0x757276:  add  esi, 2      ; { colour, normal } pair
//   fn 10  @ 0x758346:  inc  esi         ; colour only
//
// So this variant consumes the same short span stream as fn 24/25/28/29 - block
// layout skip | run | colour*run | run-dup, size 3 + run - but unlike those it
// uses the `(buffer+1)[idx]` displacement form for the second pixel rather than
// the `(x | 1)` index form, so it DOES have the row-bleed bug at Xint == 255.
// That is fixed here by the independent bounds check in PutPixelDepthPair.
//
// Everything else matches fn 18 exactly: eager colour fetch before the depth
// test (not the lazy form fn 22 uses), Z accumulator in the incoming argument
// slot, 16-bit truncating `imul dx, [ecx+2Eh]` for the skip, depth read as
// `mov dx, word[a1] ; shr dx, 8`.
//
// FOUR SLOTS, AND THE IDA SLOT NUMBERS ARE NOT RELIABLE
// -----------------------------------------------------
// Across the family the IDA names claim slots 2/18, 3/19, 5/20, 6/21, 6/22,
// 7/23, 9/24/28, 10/25/29 and now 10/14/26/30. Slot 6 and slot 10 each appear
// twice, so at least some of those numbers are hand-assigned rather than derived
// from the table. Worth dumping Voxel_Draw_Function_Tbl itself rather than
// trusting them - it would also settle how many slots exist and whether any
// still point at an unanalysed function.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:0075831D  mov  edx, edi        ; X accumulator
//   .text:00758325  and  edx, 0FFFFh
//   .text:00758330  shr  edx, 8          ; Xint (0..255)
//   .text:00758323  mov  eax, ebp        ; Y accumulator
//   .text:0075832B  and  eax, 0FF00h     ; Yint << 8
//   .text:00758333  or   eax, edx
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: the endptr twin of this function, containing 0x75855A / 0x758560.
//            Expect it to start around 0x758430.
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) replacement
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_75839A` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_SpecBufferNoNormals_StartPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;
	const int stepZZ = pDraw->AxisZ.Z;

	// DIFF: X and Y widened to real int32 8.8 values. Z left alone - only its low
	// 16 bits are ever read and the depth buffer is one byte per pixel.
	int rowX = pDraw->Start.X;
	int rowY = pDraw->Start.Y;
	int rowZ = pDraw->Start.Z;

	for (int y = 0; y < sizeY; ++y)
	{
		const int savedRowX = rowX;
		const int savedRowY = rowY;
		const int savedRowZ = rowZ;
		const int savedDataPos = pDraw->DataPos;

		int accX = rowX;
		int accY = rowY;
		int accZ = rowZ;

		for (int x = 0; x < sizeX; ++x)
		{
			const int columnX = accX;
			const int columnY = accY;
			const int columnZ = accZ;

			// startptr variant -> the START table (mov edx,[ecx] @ 0x75829B).
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

					// 16-bit signed multiply, truncated - matches
					// `movzx dx, dl` / `imul dx, [ecx+2Eh]` @ 0x7582D2.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					int runCount = *pSpan++;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							// ONE byte per voxel - no normal byte in this stream
							// (inc esi @ 0x758346, not `add esi, 2`).
							const std::uint8_t colour = pSpan[0];
							pSpan += 1;

							PutPixelDepthPair(ToWhole(accX), ToWhole(accY),
								colour, ToDepth(accZ));

							accX += stepZX;
							accY += stepZY;
							accZ += stepZZ;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc esi @ 0x758391).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_75839A.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_7583DA.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		rowZ = savedRowZ + pDraw->AxisY.Z;
		accX = rowX;
		accY = rowY;
		accZ = rowZ;

		pDraw->DataPos = savedDataPos + pDraw->YSteps;
	}
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
//
// __cdecl with a single stack argument, so the jump target inherits exactly the
// frame the original expected: ESP -> return address, argument at [ESP+4].
//
// NOTE: because four table slots point at this function, replacing the entry
// point covers all four at once. If you switch to overwriting table pointers
// instead, remember to write all four slots.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7581F0, VoxelDraw_SpecBufferNoNormals_StartPtr)
