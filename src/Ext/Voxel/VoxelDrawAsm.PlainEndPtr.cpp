// ===========================================================================
// Voxel_Draw_Function_Tbl slot 1 - Asm_Voxel_Draw_Function_Old_1_plain
//   vanilla 0x7DF8C0 .. 0x7DF9B7
//
// The backward-walking twin of slot 0. Same packed accumulator, same five
// deviations from the Cpp variants - see VoxelDrawAsm.PlainStartPtr.cpp for the
// full write-up, only the deltas are repeated here.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DF996  88 90 78 FF B2 00  -> disp32 at 0x7DF998, your list: 0x7DF998  OK
//
// Running tally: 4 wrong out of 28.
//
// SPAN WALK - backward, two bytes per voxel, single pixel
// ------------------------------------------------------
//   .text:007DF97E  mov  ch, [edi]    ; runCount at the terminator E
//   .text:007DF980  dec  edi          ; -> P
//   .text:007DF989  dec  edi          ; -> P-1
//   .text:007DF98D  mov  dl, [edi]    ; colour at P-1
//   .text:007DF991  dec  edi          ; -> P-2, i.e. step -2
//   .text:007DF99E  dec  edi
//   .text:007DF9A1  mov  al, [edi]    ; skipCount
//   .text:007DF9A3  dec  edi          ; -> previous block's terminator
//
// Identical pointer semantics to the Cpp endptr variants (fn 17 etc.): colour at
// pSpan[-1] stepping -2, skipCount at pSpan[-1] stepping -2. Run is consumed
// before skip, as in every backward walk.
//
// Column offsets come from [esi+4], the END table (mov ebx,[esi+4] @ 0x7DF926).
//
// UNCHANGED FROM SLOT 0
// ---------------------
//   * packed 16:16 accumulator, `add ebx, ebp` carries X into Y
//     (.text:007DF96F shl ebx,10h / .text:007DF975 mov bx,[esi+18h])
//   * 8-bit zRemaining (sub cl,al @ 0x7DF9A4, dec cl @ 0x7DF987)
//   * empty-column test is `jns`, not `== -1` (.text:007DF92C)
//   * current column position written back to Start (.text:007DF91E)
//   * BUG: LUT build has no SizeZ == 0 guard (.text:007DF8D9 / loc_7DF8EA)
//   * `mov esi, 4` @ 0x7DF8E5 is dead
//
// The index derivation is byte-identical, so the 256 stride is the same:
//
//   .text:007DF985  mov  eax, ebx
//   .text:007DF98A  shr  eax, 10h        ; Y word
//   .text:007DF98F  mov  al, bh          ; Xint -> idx = Yint * 256 + Xint
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. Vanilla's `jmp loc_7DF979` back-edge and the
// `jmp loc_7DF92E` exit collapse into ordinary loops.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_Plain_EndPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUGFIX: vanilla has no zero guard - `dec ecx` from 0 runs 2^32 iterations.
	BuildDistanceLut(pDraw->AxisZ, sizeZ);

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

			// endptr variant -> the END table.
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DF92C).
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

				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DF979).
				while (zRemaining != 0)
				{
					int runCount = pSpan[0];
					--pSpan;

					while (runCount != 0)
					{
						// Reading backwards: normal at pSpan[0] is unused here.
						const std::uint8_t colour = pSpan[-1];
						pSpan -= 2;

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

			// Column advance - vanilla loc_7DF92E.
			columnX += pDraw->AxisX.X;
			columnY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DF93E.
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
// __cdecl with a single stack argument. Vanilla builds an EBP frame and exits
// with `leave ; retn`, but at the entry point ESP still just points at the
// return address with the argument at [ESP+4], so a plain __cdecl function is a
// drop-in.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DF8C0, VoxelDrawAsm_Plain_EndPtr)
