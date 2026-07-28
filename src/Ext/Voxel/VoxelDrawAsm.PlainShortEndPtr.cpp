// ===========================================================================
// Voxel_Draw_Function_Tbl slots 9 / 13 - Asm_Voxel_Draw_Function_Old_9_13_plain
//   vanilla 0x7DFD00 .. 0x7DFDF6
//
// The backward-walking twin of slots 8/12. Plain, endptr, one byte per voxel.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DFDD5  88 90 78 FF B2 00  -> disp32 at 0x7DFDD7, your list: 0x7DFDD7  OK
//
// Running tally: 4 wrong out of 32.
//
// ###########################################################################
// # THE INVERTED LUT COUNT IS IN BOTH SHORT-STREAM ASM FUNCTIONS            #
// ###########################################################################
//
//   .text:007DFD17  B9 FF 00 00 00   mov  ecx, 0FFh
//   .text:007DFD1C  2A 4E 32         sub  cl, [esi+32h]     ; count = 255 - SizeZ
//
// Byte-identical to slots 8/12 at 0x7DFC17. So this is not a one-off typo in one
// function - both members of the short-stream asm pair do it, and only they.
// The four long-stream asm functions (slots 0, 1, 4, 5) all use the plain
// `xor ecx,ecx ; mov cl,[esi+32h]` form.
//
// I cannot find a reading that makes 255 - SizeZ correct. The LUT is indexed by
// the raw skip byte and entry i must hold i * AxisZ, so the values built are
// right - there are just too few of them once SizeZ passes 127:
//
//     entries built = 254 - SizeZ,  max index needed = SizeZ
//     sufficient iff SizeZ <= 127
//
// Above that, large Z-skips read whatever the previously drawn voxel left in the
// table. At SizeZ == 255 the count is 0 and the counter wraps to 4 billion.
//
// CALIBRATION: typical YR voxel sections are nowhere near 127 deep, so this is
// dormant for normal models. It only bites on unusually tall sections - worth
// checking SizeZ on the models that render badly for you before assuming it is
// the cause.
//
// PRESERVED VERBATIM below. The SizeZ == 255 wrap is guarded inside
// BuildDistanceLut, since a wild write is not behaviour worth reproducing.
//
// ###########################################################################
//
// SPAN WALK - backward, ONE byte per voxel
//   .text:007DFDBE  mov  ch, [edi]    ; runCount at terminator E
//   .text:007DFDC0  dec  edi          ; -> P
//   .text:007DFDCC  mov  dl, [edi]    ; colour at P
//   .text:007DFDD0  dec  edi          ; step -1
//   .text:007DFDDD  dec  edi
//   .text:007DFDE0  mov  al, [edi]    ; skipCount
//   .text:007DFDE2  dec  edi          ; -> previous block's terminator
//
// Identical pointer semantics to Cpp fn 25/29. Block size 3 + run.
//
// Column offsets come from [esi+4], the END table (mov ebx,[esi+4] @ 0x7DFD69).
//
// UNCHANGED FROM SLOT 0
//   * packed 16:16 accumulator, `add ebx, ebp` carries X into Y
//   * 8-bit zRemaining (sub cl,al @ 0x7DFDE3, dec cl @ 0x7DFDC7)
//   * empty-column test is `jns`, not `== -1` (.text:007DFD6F)
//   * current column position written back to Start (.text:007DFD61)
//   * `mov esi, 4` @ 0x7DFD28 is dead
//   * single pixel write
//   * index derivation identical: shr eax,10h then mov al,bh
//
// SCOPE - STILL MISSING
// ---------------------
// This completes every function reachable from Voxel_Draw_Function_Tbl.
// What remains:
//
//   MISSING: two asm functions around 0x7DFE00 and 0x7DFF00 that are NOT in the
//            dispatch table - sites 0x7DFEE5 / 0x7DFEEB and 0x7DFFD7 / 0x7DFFDD.
//            They are the only asm functions with PAIRED writes. Get their xrefs
//            first - if nothing reaches them they are dead code and can be
//            skipped entirely.
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) relocation - recipe known, see
//            VoxelBufferReplace.h
//
// Replacer::BufferSize MUST stay 256 until those are resolved.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_PlainShort_EndPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUG (vanilla, preserved): the count is 255 - SizeZ, not SizeZ - identical
	// to slots 8/12. See the block comment above.
	// BUGFIX: the count is clamped inside BuildDistanceLut, so SizeZ == 255 no
	// longer wraps the counter and runs off the end of the section.
	BuildDistanceLut(pDraw->AxisZ, 255 - sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla holds these as one packed dword, so the X half carries into
	// the Y half. Split into independent int32 8.8 values.
	int rowX = pDraw->Start.X;
	int rowY = pDraw->Start.Y;

	for (int y = 0; y < sizeY; ++y)
	{
		const int savedRowX = rowX;
		const int savedRowY = rowY;
		const int savedDataPos = pDraw->DataPos;

		int accX = rowX;
		int accY = rowY;

		for (int x = 0; x < sizeX; ++x)
		{
			// Side effect preserved: the inner block reads the position back out
			// of the struct, so the outer loop has to store it there.
			pDraw->Start.X = static_cast<std::int16_t>(accX);
			pDraw->Start.Y = static_cast<std::int16_t>(accY);

			// endptr variant -> the END table.
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DFD6F).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFDB9).
				while (zRemaining != 0)
				{
					int runCount = pSpan[0];
					--pSpan;

					while (runCount != 0)
					{
						// ONE byte per voxel - no normal byte in this stream
						// (dec edi @ 0x7DFDD0, not `sub edi, 2`).
						const std::uint8_t colour = pSpan[0];
						pSpan -= 1;

						PutPixel(ToWhole(accX), ToWhole(accY), colour);

						accX += stepZX;
						accY += stepZY;

						zRemaining = (zRemaining - 1) & 0xFF;
						--runCount;
					}

					const int skipCount = pSpan[-1];
					pSpan -= 2;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;
					zRemaining = (zRemaining - skipCount) & 0xFF;
				}
			}

			// Column advance - vanilla loc_7DFD71.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFD81.
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
// NOTE: two table slots (9 and 13) point here, so replacing the entry point
// covers both.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DFD00, VoxelDrawAsm_PlainShort_EndPtr)
