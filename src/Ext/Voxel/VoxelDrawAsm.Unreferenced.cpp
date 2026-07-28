// ===========================================================================
// Asm_Voxel_Normals_Function_Old_7  -  vanilla 0x7DFE00 .. 0x7DFEFC
// Asm_Voxel_Normals_Function_Old_8  -  vanilla 0x7DFF00 .. 0x7DFFFC
//
// ###########################################################################
// # THESE TWO ARE DEAD CODE                                                 #
// ###########################################################################
//
// Neither appears in Voxel_Draw_Function_Tbl and neither has a single xref.
// Compare the IDA proc headers:
//
//   slot 0 @ 0x7DF7C0:   ; VoxelLibraryClass__Draw+2B3^p
//                        ; .data:Voxel_Draw_Function_Tbl_o
//   Old_7  @ 0x7DFE00:   (nothing)
//   Old_8  @ 0x7DFF00:   (nothing)
//
// The 32-slot table is fully enumerated and accounts for six distinct asm
// functions - 0x7DF7C0, 0x7DF8C0, 0x7DF9C0, 0x7DFAE0, 0x7DFC00, 0x7DFD00.
// These two are not among them.
//
// CONSEQUENCE FOR THE PATCH LIST: the four entries covering them
//
//     DEFINE_PATCH_TYPED(DWORD, 0x7DFEE5, ...)
//     DEFINE_PATCH_TYPED(DWORD, 0x7DFEEB, ...)
//     DEFINE_PATCH_TYPED(DWORD, 0x7DFFD7, ...)
//     DEFINE_PATCH_TYPED(DWORD, 0x7DFFDD, ...)
//
// patch code that never executes. They are harmless, but they can go.
//
// This whole file can simply be excluded from the build. It exists as cheap
// insurance: if a caller ever turns up - a computed reference, or a table IDA
// did not type - the hooks are already correct and inert until then.
//
// VERIFY the "no xrefs" claim in your own IDB before deleting anything. IDA only
// reports references it can see statically.
//
// ###########################################################################
//
// PATCH LIST STATUS: ALL FOUR CORRECT
//
//   .text:007DFEE3  88 90 78 FF B2 00  -> disp32 at 0x7DFEE5, your list: 0x7DFEE5      OK
//   .text:007DFEE9  88 90 79 FF B2 00  -> disp32 at 0x7DFEEB, your list: 0x7DFEEB (+1) OK
//   .text:007DFFD5  88 90 78 FF B2 00  -> disp32 at 0x7DFFD7, your list: 0x7DFFD7      OK
//   .text:007DFFDB  88 90 79 FF B2 00  -> disp32 at 0x7DFFDD, your list: 0x7DFFDD (+1) OK
//
// FINAL TALLY: 4 wrong out of 36, across 21 functions.
//
// ---------------------------------------------------------------------------
// WHAT THEY ARE: THE BUGGY ANCESTORS OF fn 24/25
// ---------------------------------------------------------------------------
// Both are plain, one byte per voxel, PAIRED write - the same feature set as the
// Cpp functions at slots 24/25 (0x757E70 / 0x758030). The difference is the pair
// addressing:
//
//   Old_7 / Old_8   mov VoxelPixelBuffer[eax], dl
//                   mov (VoxelPixelBuffer+1)[eax], dl      -> idx and idx+1
//
//   fn 24 / fn 25   or  ebx, edi / inc edi / or edi, eax
//                   mov VoxelPixelBuffer[ebx], dl
//                   mov VoxelPixelBuffer[edi], dl          -> x and (x | 1)
//
// The `+1` form bleeds into column 0 of the next row when Xint == 255; the
// `x | 1` form cannot. So when Westwood rewrote these in C++ they appear to have
// changed the addressing deliberately to fix that, and left the originals in the
// binary unreferenced. That is decent supporting evidence for the reading in
// VoxelDraw.PlainLsbStartPtr.cpp - the odd-x single-pixel behaviour of `x | 1`
// is intentional, not an accident.
//
// Everything else matches the rest of the asm family: packed 16:16 accumulator
// with X-into-Y carry, 8-bit zRemaining, `jns` empty-column test, position
// written back into Start, dead `mov esi, 4`, and the inverted
// `255 - SizeZ` LUT count that slots 8/12 and 9/13 also have.
//
// Old_7 walks forward from the START table (mov ebx,[esi] @ 0x7DFE69);
// Old_8 walks backward from the END table (mov ebx,[esi+4] @ 0x7DFF69).
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Old_7 - forward, one byte per voxel, paired write.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_PlainShortPair_StartPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUG (vanilla, preserved): count is 255 - SizeZ, not SizeZ.
	// BUGFIX: clamped inside BuildDistanceLut so SizeZ == 255 cannot wrap.
	BuildDistanceLut(pDraw->AxisZ, 255 - sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla packs X and Y into one dword, so X carries into Y.
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
			// of the struct.
			pDraw->Start.X = static_cast<std::int16_t>(accX);
			pDraw->Start.Y = static_cast<std::int16_t>(accY);

			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DFE6E).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFEBB).
				while (zRemaining != 0)
				{
					const int skipCount = *pSpan++;
					zRemaining = (zRemaining - skipCount) & 0xFF;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					int runCount = *pSpan++;

					while (runCount != 0)
					{
						// ONE byte per voxel (inc edi @ 0x7DFEDE).
						const std::uint8_t colour = pSpan[0];
						pSpan += 1;

						// Pair at idx and idx+1. BUGFIX: bounds-checked
						// independently, so no row bleed at Xint == 255.
						PutPixelPair(ToWhole(accX), ToWhole(accY), colour);

						accX += stepZX;
						accY += stepZY;

						zRemaining = (zRemaining - 1) & 0xFF;
						--runCount;
					}

					// Terminator byte (inc edi @ 0x7DFEF1).
					++pSpan;
				}
			}

			// Column advance - vanilla loc_7DFE70.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFE80.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		accX = rowX;
		accY = rowY;

		pDraw->DataPos = savedDataPos + pDraw->YSteps;
	}
}

// ---------------------------------------------------------------------------
// Old_8 - backward, one byte per voxel, paired write.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_PlainShortPair_EndPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUG (vanilla, preserved): count is 255 - SizeZ, not SizeZ.
	BuildDistanceLut(pDraw->AxisZ, 255 - sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

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
			pDraw->Start.X = static_cast<std::int16_t>(accX);
			pDraw->Start.Y = static_cast<std::int16_t>(accY);

			// endptr variant -> the END table (mov ebx,[esi+4] @ 0x7DFF69).
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DFF6F).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP (cmp ecx,0 @ 0x7DFFB9).
				while (zRemaining != 0)
				{
					int runCount = pSpan[0];
					--pSpan;

					while (runCount != 0)
					{
						// ONE byte per voxel (dec edi @ 0x7DFFD0).
						const std::uint8_t colour = pSpan[0];
						pSpan -= 1;

						PutPixelPair(ToWhole(accX), ToWhole(accY), colour);

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

			// Column advance - vanilla loc_7DFF71.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DFF81.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		accX = rowX;
		accY = rowY;

		pDraw->DataPos = savedDataPos + pDraw->YSteps;
	}
}

// ---------------------------------------------------------------------------
// Hooks. Inert unless a caller exists - see the header note.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DFE00, VoxelDrawAsm_PlainShortPair_StartPtr)
DEFINE_FUNCTION_JUMP(LJMP, 0x7DFF00, VoxelDrawAsm_PlainShortPair_EndPtr)
