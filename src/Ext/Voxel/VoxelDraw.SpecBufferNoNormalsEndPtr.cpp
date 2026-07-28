// ===========================================================================
// Voxel_Draw_Function_Tbl entries "11 / 15 / 27 / 31, spec buffer, endptr"
//   vanilla 0x758430 .. 0x758660
//   (IDA names it _11_15_27_31_, i.e. FOUR table slots point here)
//
// The backward-walking twin of 0x7581F0. fn 19 with a one-byte-per-voxel span
// stream. No new mechanisms - this closes the table-function group.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:00758558  88 98 78 FF B2 00  -> disp32 at 0x75855A, your list: 0x75855A      OK
//   .text:0075855E  88 98 79 FF B2 00  -> disp32 at 0x758560, your list: 0x758560 (+1) OK
//
// FINAL TALLY across all 13 ported functions: 4 wrong out of 26.
//   0x756EDF -> want 0x756EDE
//   0x757063 -> want 0x757062
//   0x75728B -> want 0x75728A
//   0x757291 -> want 0x757290
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:00758540  66 0F B6 98 [E0 D5 B1 00]  READ  depth,   disp32 at 0x758544  <- prefix+2-byte opcode, +4
//   .text:00758552  88 90       [E0 D5 B1 00]  write depth,   disp32 at 0x758554
//   .text:00758564  88 90       [E1 D5 B1 00]  write depth+1, disp32 at 0x758566
//
// SPAN WALK - BACKWARD, ONE BYTE PER VOXEL
// ----------------------------------------
//   .text:00758504  mov  al, [esi]     ; runCount at the block terminator E
//   .text:00758506  dec  esi           ; -> P
//   .text:0075851D  mov  al, [esi]     ; colour at P
//   .text:00758548  dec  esi           ; -> step -1, matching fn 25/29
//   .text:0075858F  mov  dl, [esi-1]   ; skipCount
//   .text:00758592  dec  esi
//   .text:00758597  dec  esi           ; -> previous block's terminator
//
// Block layout skip | run | colour*run | run-dup, size 3 + run.
//
// Colour is fetched EAGERLY, before the depth test (as in fn 18 / 19 / and
// 0x7581F0), not lazily as in fn 22 / 23. The `dec esi` at 0x758548 also happens
// before the compare, so both paths advance identically.
//
// SUSPECT: as in every endptr variant, the accumulators still step by +AxisZ
// despite blocks arriving in reverse Z order, so the caller must supply a
// pre-negated AxisZ and a far-end Start. Preserved verbatim.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:0075851F  mov  edx, edi        ; X accumulator
//   .text:00758527  and  edx, 0FFFFh
//   .text:00758532  shr  edx, 8          ; Xint (0..255)
//   .text:00758525  mov  eax, ebp        ; Y accumulator
//   .text:0075852D  and  eax, 0FF00h     ; Yint << 8
//   .text:00758535  or   eax, edx
//
// SCOPE - WHAT IS LEFT
// --------------------
// All twelve Voxel_Draw_Function_Tbl rasterizers plus Draw_Shadow are now ported.
// Remaining:
//
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) replacement
//   OPEN:    the dispatch table itself - see the slot-coverage note below
//
// SLOT COVERAGE GAP
// -----------------
// Collecting every IDA slot number across the family gives
//   2 3 5 6 6 7 9 10 10 11 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
// Slots 0, 1, 4, 8, 12 and 13 never appear, and 6 and 10 each appear twice. So
// either some numbers are hand-assigned and wrong, or up to six slots point at
// functions not yet analysed - quite possibly the Asm_Voxel_Normals_Function_Old_*
// family. Dumping Voxel_Draw_Function_Tbl would settle it.
//
// Replacer::BufferSize MUST stay 256 until the remaining items are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_7585D8` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_SpecBufferNoNormals_EndPtr(
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

			// endptr variant -> the END table (mov edx,[ecx+4] @ 0x7584D5).
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
							// ONE byte per voxel - no normal byte in this stream
							// (dec esi @ 0x758548, not `sub esi, 2`).
							const std::uint8_t colour = pSpan[0];
							pSpan -= 1;

							PutPixelDepthPair(ToWhole(accX), ToWhole(accY),
								colour, ToDepth(accZ));

							accX += stepZX;
							accY += stepZY;
							accZ += stepZZ;
							--runCount;
						}
						while (runCount != 0);
					}

					const int skipCount = pSpan[-1];
					pSpan -= 2;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					// 16-bit signed multiply, truncated - matches
					// `movzx dx, dl` / `imul dx, [ecx+2Eh]` @ 0x75859C.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					zRemaining -= skipCount;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_7585D8.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_758618.
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
// NOTE: four table slots point at this function, so replacing the entry point
// covers all four. If you switch to overwriting table pointers, write all four.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x758430, VoxelDraw_SpecBufferNoNormals_EndPtr)
