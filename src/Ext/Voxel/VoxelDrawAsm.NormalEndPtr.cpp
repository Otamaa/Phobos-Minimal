// ===========================================================================
// Voxel_Draw_Function_Tbl slot 5 - Asm_Voxel_Draw_Function_Old_5_normal
//   vanilla 0x7DFAE0 .. 0x7DFBE9
//
// The backward-walking twin of slot 4. Lit, endptr, single pixel.
// See VoxelDrawAsm.PlainStartPtr.cpp for the full write-up of the five ways the
// asm family deviates from the Cpp variants; only the deltas appear here.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DFBC8  88 90 78 FF B2 00  -> disp32 at 0x7DFBCA, your list: 0x7DFBCA  OK
//
// Running tally: 4 wrong out of 30.
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:007DFBB2  8A B2 [90 59 B4 00]  NormalToLut, disp32 at 0x7DFBB4
//   .text:007DFBC2  8A 92 [78 11 B4 00]  VPLLookup,   disp32 at 0x7DFBC4
//
// PAIR ORDER - THIRD INDEPENDENT CONFIRMATION
// -------------------------------------------
// Reading backwards from P, the normal sits above the colour:
//
//   .text:007DFBAE  mov  dl, [edi]        ; normal at P
//   .text:007DFBB2  mov  dh, VoxelNormalToLut[edx]
//   .text:007DFBB8  mov  dl, [edi-1]      ; colour at P-1
//   .text:007DFBBD  sub  edi, 2
//
// Same as fn 21 (Cpp normal endptr) and consistent with every forward variant's
// { colour, normal } layout. The shading index is built in DX exactly as in
// slot 4 - `mov dh, NormalToLut[edx]` reads EDX before overwriting DH, then DL
// is replaced by the colour, giving edx == (section << 8) | colour.
//
// SKIP - identical to slots 1 and 5's siblings
//   .text:007DFBD0  dec  edi
//   .text:007DFBD3  mov  al, [edi]        ; skipCount
//   .text:007DFBD5  dec  edi              ; -> previous block's terminator
//
// Column offsets come from [esi+4], the END table (mov ebx,[esi+4] @ 0x7DFB46).
//
// SINGLE PIXEL, unlike fn 21 which writes a pair (0x7578AF / 0x7578B5). Same
// asm-vs-Cpp 1x/2x divergence noted for slot 4 vs slot 20.
//
// UNCHANGED FROM SLOT 0
//   * packed 16:16 accumulator, `add ebx, ebp` carries X into Y
//   * 8-bit zRemaining (sub cl,al @ 0x7DFBD6, dec cl @ 0x7DFBA7)
//   * empty-column test is `jns`, not `== -1` (.text:007DFB4C)
//   * current column position written back to Start (.text:007DFB3E)
//   * BUG: LUT build has no SizeZ == 0 guard (.text:007DFAF9 / loc_7DFB0A)
//   * `mov esi, 4` @ 0x7DFB05 is dead
//   * index derivation identical: shr eax,10h then mov al,bh
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x7DFC00 (slots 8/12, plain), 0x7DFD00 (slots 9/13, plain)
//   MISSING: two asm functions around 0x7DFE00 and 0x7DFF00 that are NOT in the
//            dispatch table - sites 0x7DFEE5 / 0x7DFEEB and 0x7DFFD7 / 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) relocation - recipe known, see
//            VoxelBufferReplace.h
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_Normal_EndPtr(
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

			// Sign test, not `== -1` (jns @ 0x7DFB4C).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFB99).
				while (zRemaining != 0)
				{
					int runCount = pSpan[0];
					--pSpan;

					while (runCount != 0)
					{
						// Reading backwards: normal above, colour below.
						const std::uint8_t normal = pSpan[0];
						const std::uint8_t colour = pSpan[-1];
						pSpan -= 2;

						// SINGLE pixel - not the pair fn 21 writes.
						PutPixel(ToWhole(accX), ToWhole(accY),
							Shade(colour, normal));

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

			// Column advance - vanilla loc_7DFB4E.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFB5E.
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
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DFAE0, VoxelDrawAsm_Normal_EndPtr)
