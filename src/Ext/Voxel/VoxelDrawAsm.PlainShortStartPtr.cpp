// ===========================================================================
// Voxel_Draw_Function_Tbl slots 8 / 12 - Asm_Voxel_Draw_Function_Old_8_12_plain
//   vanilla 0x7DFC00 .. 0x7DFCF6
//
// Plain, startptr, ONE byte per voxel - confirming that slots 8-15 are the
// short-stream group, mirroring 24-31 on the Cpp side.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DFCE3  88 90 78 FF B2 00  -> disp32 at 0x7DFCE5, your list: 0x7DFCE5  OK
//
// Running tally: 4 wrong out of 31.
//
// ###########################################################################
// # BUG: THE LUT BUILD COUNT IS INVERTED - 255 - SizeZ INSTEAD OF SizeZ      #
// ###########################################################################
//
// Every other rasterizer in the family loads the loop count straight from SizeZ:
//
//   slot 0 @ 0x7DF7D7:  xor  ecx, ecx
//                       mov  cl, [esi+32h]        ; count = SizeZ
//
// This one does something else entirely:
//
//   .text:007DFC17  B9 FF 00 00 00   mov  ecx, 0FFh
//   .text:007DFC1C  2A 4E 32         sub  cl, [esi+32h]    ; count = 255 - SizeZ
//
// The loop writes entries 0 .. count-1, so it fills 254 - SizeZ entries. The skip
// byte pulled from the span data can be as large as SizeZ, so the table is only
// large enough when
//
//     254 - SizeZ >= SizeZ    i.e.    SizeZ <= 127
//
// Consequences:
//
//   SizeZ <= 127   fine - over-fills, harmless
//   SizeZ >  127   entries above (254 - SizeZ) are STALE from whatever voxel was
//                  drawn previously, so large Z-skips land at the wrong screen
//                  position. Progressively worse the taller the voxel gets.
//   SizeZ == 255   count is 0, `dec ecx` wraps to 0xFFFFFFFF, and it writes two
//                  words per iteration for 4 billion iterations.
//
// RELEVANT TO THE ORIGINAL COMPLAINT: this misdraws exactly the case you set out
// to fix - tall voxel models. It is independent of the 256x256 buffer limit, so
// enlarging the buffer would never have helped here.
//
// PRESERVED VERBATIM (the `255 - sizeZ` below), because it is observable vanilla
// behaviour and your convention is to keep those. The one-line fix, if you ever
// decide to take it, is to pass `sizeZ` instead - mark it BUGFIX and expect tall
// voxels drawn through slots 8/12 to shift position.
//
// BUGFIX applied only to the SizeZ == 255 crash: BuildDistanceLut clamps the
// count, so it writes entry 0 and stops rather than running off the section.
//
// ###########################################################################
//
// ONE BYTE PER VOXEL
//   .text:007DFCDA  mov  dl, [edi]     ; colour
//   .text:007DFCDE  inc  edi           ; not `add edi, 2`
//   .text:007DFCEB  inc  edi           ; terminator
//
// Block layout skip | run | colour*run | run-dup, size 3 + run.
//
// UNCHANGED FROM SLOT 0
//   * packed 16:16 accumulator, `add ebx, ebp` carries X into Y
//   * 8-bit zRemaining (sub cl,al @ 0x7DFCC3, dec cl @ 0x7DFCD5)
//   * empty-column test is `jns`, not `== -1` (.text:007DFC6E)
//   * current column position written back to Start (.text:007DFC61)
//   * `mov esi, 4` @ 0x7DFC28 is dead
//   * single pixel write, forward walk, START table (mov ebx,[esi] @ 0x7DFC69)
//   * index derivation identical: shr eax,10h then mov al,bh
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_PlainShort_StartPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUG (vanilla, preserved): the count is 255 - SizeZ, not SizeZ. See the
	// block comment above - this is why tall voxels drawn through slots 8/12
	// misplace their Z-skips.
	// BUGFIX: the count is clamped inside BuildDistanceLut, so SizeZ == 255
	// no longer wraps the counter and runs off the end of the section.
	BuildDistanceLut(pDraw->AxisZ, 255 - sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla holds these as one packed dword, so the X half carries into
	// the Y half. Split into independent int32 8.8 values.
	int rowX = pDraw->StartX;
	int rowY = pDraw->StartY;

	for (int y = 0; y < sizeY; ++y)
	{
		const int savedRowX = rowX;
		const int savedRowY = rowY;
		const int savedDataPos = pDraw->DataPos;

		int columnX = rowX;
		int columnY = rowY;

		for (int x = 0; x < sizeX; ++x)
		{
			// Vanilla wrote the running position back into Start here because
			// its inner block read it out again. This port keeps it in locals,
			// and Start is the narrow 16-bit field we are moving away from, so
			// the writeback is dropped.

			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DFC6E).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				// SEPARATE ACCUMULATOR. Vanilla keeps the column position in
				// [ebp-0Ch] and loads a FRESH packed accumulator into EBX from
				// Start at the top of the draw block (mov bx,[esi+1Ah] / shl /
				// mov bx,[esi+18h]). The draw block never touches [ebp-0Ch], and
				// EBX is discarded on the way out. Conflating the two makes every
				// column after the first start from the previous column's final
				// span position.
				int accX = columnX;
				int accY = columnY;

				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFCBB).
				while (zRemaining != 0)
				{
					const int skipCount = *pSpan++;
					zRemaining = (zRemaining - skipCount) & 0xFF;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					int runCount = *pSpan++;

					while (runCount != 0)
					{
						// ONE byte per voxel - no normal byte in this stream
						// (inc edi @ 0x7DFCDE).
						const std::uint8_t colour = pSpan[0];
						pSpan += 1;

						PutPixel(ToWhole(accX), ToWhole(accY), colour);

						accX += stepZX;
						accY += stepZY;

						zRemaining = (zRemaining - 1) & 0xFF;
						--runCount;
					}

					// Terminator byte, consumed even when runCount was 0
					// (inc edi @ 0x7DFCEB is on the shared path).
					++pSpan;
				}
			}

			// Column advance - vanilla loc_7DFC70.
			columnX += pDraw->AxisX.X;
			columnY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFC80.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		columnX = rowX;
		columnY = rowY;

		pDraw->DataPos = savedDataPos + pDraw->YSteps;
	}
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
//
// NOTE: two table slots (8 and 12) point here, so replacing the entry point
// covers both.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DFC00, VoxelDrawAsm_PlainShort_StartPtr)
