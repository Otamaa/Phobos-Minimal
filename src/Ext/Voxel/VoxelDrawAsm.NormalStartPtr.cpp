// ===========================================================================
// Voxel_Draw_Function_Tbl slot 4 - Asm_Voxel_Draw_Function_Old_4_normal_MAIN
//   vanilla 0x7DF9C0 .. 0x7DFAC9
//
// The lit asm rasterizer. Slot 0 plus VPL shading.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DFAB6  88 90 78 FF B2 00  -> disp32 at 0x7DFAB8, your list: 0x7DFAB8  OK
//
// Running tally: 4 wrong out of 29.
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:007DFAA1  8A B2 [90 59 B4 00]  NormalToLut, disp32 at 0x7DFAA3
//   .text:007DFAB0  8A 92 [78 11 B4 00]  VPLLookup,   disp32 at 0x7DFAB2
//
// ---------------------------------------------------------------------------
// SHADING - SAME FORMULA, BUILT OUT OF REGISTER HALVES
// ---------------------------------------------------------------------------
// The Cpp variants compute the VPL index with a shift and an add. This one packs
// it into DX directly, so the VPLLookup access needs no SIB byte and no shift:
//
//   .text:007DFA9A  xor  edx, edx
//   .text:007DFA9C  mov  dl, [edi+1]                 ; dl = normal
//   .text:007DFAA1  mov  dh, VoxelNormalToLut[edx]   ; dh = section index
//   .text:007DFAA7  mov  dl, [edi]                   ; dl = colour
//   .text:007DFAB0  mov  dl, VPLLookup[edx]          ; edx == (section << 8) | colour
//
// Identical result to fn 20's `VPLLookup[(NormalToLut[normal] << 8) + colour]`,
// so VoxelRaster::Shade() covers both. Note `mov dh, VoxelNormalToLut[edx]`
// reads EDX as the index before overwriting DH, which is why the sequence works.
//
// ---------------------------------------------------------------------------
// NOT EQUIVALENT TO ITS Cpp COUNTERPART - ONE PIXEL, NOT TWO
// ---------------------------------------------------------------------------
// Slot 4 (asm) writes a SINGLE pixel:
//   .text:007DFAB6  mov  VoxelPixelBuffer[eax], dl
//
// Slot 20 (fn 20, the Cpp "normal startptr") writes a PAIR:
//   .text:007576EC  mov  VoxelPixelBuffer[ecx], dl
//   .text:007576F2  mov  (VoxelPixelBuffer+1)[ecx], dl
//
// So these two are not interchangeable implementations of one algorithm - the
// asm path draws at 1x horizontal and the Cpp path at 2x. Same is true of
// slots 0/16 and 1/17 (both single) but NOT of slots 2/18 and 3/19, which are
// literally the same function pointer. Whatever picks the slot inside
// VoxelLibraryClass::Draw is choosing between real visual variants, not just
// between old and new code.
//
// UNCHANGED FROM SLOT 0
// ---------------------
//   * packed 16:16 accumulator, `add ebx, ebp` carries X into Y
//     (.text:007DFA6F shl ebx,10h / .text:007DFA75 mov bx,[esi+18h])
//   * 8-bit zRemaining (sub cl,al @ 0x7DFA83, dec cl @ 0x7DFA95)
//   * empty-column test is `jns`, not `== -1` (.text:007DFA2B)
//   * current column position written back to Start (.text:007DFA1E)
//   * BUG: LUT build has no SizeZ == 0 guard (.text:007DF9D9 / loc_7DF9EA)
//   * `mov esi, 4` @ 0x7DF9E5 is dead
//   * index derivation identical: shr eax,10h then mov al,bh -> Yint*256 + Xint
//
// Forward walk, START table (mov ebx,[esi] @ 0x7DFA26), two bytes per voxel:
// colour at [edi], normal at [edi+1], `add edi, 2`, terminator via `inc edi`.
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x7DFAE0 (slot 5, normal endptr), 0x7DFC00 (slots 8/12, plain),
//            0x7DFD00 (slots 9/13, plain)
//   MISSING: two asm functions around 0x7DFE00 and 0x7DFF00 that are NOT in the
//            dispatch table - sites 0x7DFEE5 / 0x7DFEEB and 0x7DFFD7 / 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) relocation - the recipe is now
//            known, see VoxelBufferReplace.h
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_Normal_StartPtr(
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

			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DFA2B).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFA7B).
				while (zRemaining != 0)
				{
					const int skipCount = *pSpan++;
					zRemaining = (zRemaining - skipCount) & 0xFF;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					int runCount = *pSpan++;

					while (runCount != 0)
					{
						const std::uint8_t colour = pSpan[0];
						const std::uint8_t normal = pSpan[1];
						pSpan += 2;

						// SINGLE pixel - not the pair fn 20 writes.
						PutPixel(ToWhole(accX), ToWhole(accY),
							Shade(colour, normal));

						accX += stepZX;
						accY += stepZY;

						zRemaining = (zRemaining - 1) & 0xFF;
						--runCount;
					}

					// Terminator byte, consumed even when runCount was 0
					// (inc edi @ 0x7DFABE is on the shared path).
					++pSpan;
				}
			}

			// Column advance - vanilla loc_7DFA2D.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFA3D.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x7DF9C0, VoxelDrawAsm_Normal_StartPtr)
