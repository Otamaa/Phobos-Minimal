// ===========================================================================
// Voxel_Draw_Function_Tbl slot 0 - Asm_Voxel_Normals_Function_Old_0_plain
//   vanilla 0x7DF7C0 .. 0x7DF8B8
//   also called directly from VoxelLibraryClass::Draw+2B3
//
// First of the hand-written assembly rasterizers. Algorithmically this is the
// same job as fn 16 (slot 16, 0x756DD0) - plain, startptr, two-byte span stream,
// single pixel write - but the implementation differs in five observable ways,
// listed below.
//
// PATCH LIST STATUS: CORRECT
//
//   .text:007DF8A5  88 90 78 FF B2 00  -> disp32 at 0x7DF8A7, your list: 0x7DF8A7  OK
//
// Running tally: 4 wrong out of 27.
//
// ---------------------------------------------------------------------------
// THE PACKED ACCUMULATOR - SAME 256 STRIDE, DIFFERENT SHAPE
// ---------------------------------------------------------------------------
// The Cpp rasterizers keep X and Y in two registers and merge at use time. The
// asm ones keep a single packed 16:16 register, each half an 8.8 fixed point
// value:
//
//   .text:007DF86B  mov  bx, [esi+1Ah]     ; Start.Y
//   .text:007DF86F  shl  ebx, 10h          ; -> high half
//   .text:007DF875  mov  bx, [esi+18h]     ; Start.X -> low half
//   .text:007DF872  mov  ebp, [esi+2Ah]    ; AxisZ read as ONE dword {X,Y}
//
// and the index falls out of it the same way:
//
//   .text:007DF893  mov  eax, ebx
//   .text:007DF897  shr  eax, 10h          ; eax = Y word  (0x0000YYyy)
//   .text:007DF89C  mov  al, bh            ; al  = Xint    (0x0000YYXX)
//   .text:007DF8A5  mov  VoxelPixelBuffer[eax], dl
//
// idx == Yint * 256 + Xint. Identical 256 stride, identical 8-bit coordinate
// range - so the same structural limit, just reached by a different route.
//
// ---------------------------------------------------------------------------
// FIVE DIFFERENCES FROM THE Cpp TWIN (fn 16)
// ---------------------------------------------------------------------------
// 1) PACKED ADDS PROPAGATE CARRY FROM X INTO Y.
//    Every advance is a 32-bit add of a packed {X,Y} dword:
//      .text:007DF830  add  [ebp-0Ch], eax                   ; += AxisX dword
//      .text:007DF885  add  ebx, VoxelDistanceLut[eax*4]     ; += LUT dword
//      .text:007DF8A1  add  ebx, ebp                         ; += AxisZ dword
//    The Cpp versions use 16-bit adds (`add si, word ptr [...]`), which cannot
//    carry. So when the X half overflows, vanilla asm bleeds one unit into Y and
//    vanilla Cpp does not - the two "identical" rasterizers genuinely differ.
//    DIFF: this port splits X and Y into separate int32 accumulators, which
//    removes the bleed. Unavoidable: widening the coordinates and preserving a
//    16-bit carry boundary are mutually exclusive.
//
// 2) zRemaining IS 8-BIT.
//      .text:007DF883  sub  cl, al        ; -= skipCount
//      .text:007DF895  dec  cl            ; -= 1 per voxel
//    Only CL is touched, so the counter wraps at 256 instead of going negative.
//    (`cmp ecx, 0` at 0x7DF87B does test all 32 bits, but CH is zero by then and
//    the upper half was cleared at 0x7DF863.) Preserved here via an explicit
//    mask - see the `& 0xFF` in the span walk.
//
// 3) THE EMPTY-COLUMN TEST IS A SIGN TEST, NOT `== -1`.
//      .text:007DF828  or   edi, [ebx+eax*4]
//      .text:007DF82B  jns  loc_7DF860
//    Any negative offset skips the column, not just -1. Preserved as `>= 0`.
//
// 4) THE CURRENT COLUMN POSITION IS WRITTEN BACK INTO THE STRUCT.
//      .text:007DF81B  mov  eax, [ebp-0Ch]
//      .text:007DF81E  mov  [esi+18h], eax     ; clobbers Start.X AND Start.Y
//    The struct field is the channel the outer loop uses to hand the position to
//    the inner draw block, which reads it back at 0x7DF86B / 0x7DF875. The Cpp
//    versions keep it in registers and never touch Start. Side effect preserved.
//
// 5) BUG - THE LUT BUILD HAS NO SizeZ == 0 GUARD.
//      .text:007DF7D7  xor  ecx, ecx
//      .text:007DF7D9  mov  cl, [esi+32h]      ; SizeZ
//      loc_7DF7EA: ... dec ecx ; jnz loc_7DF7EA
//    With SizeZ == 0 this is a do-while entered with ecx == 0, so `dec` wraps to
//    0xFFFFFFFF and it writes two words per iteration for 4 billion iterations -
//    straight off the end of the section. Every Cpp variant guards this with
//    `cmp al, 1 ; jbe`.
//    BUGFIX: guarded here. Reproducing a wild 16 GB write is not worth fidelity;
//    if SizeZ can legitimately be 0 the vanilla build was already crashing.
//
// Also note `mov esi, 4` at 0x7DF7E5 is dead - ESI is reloaded from the argument
// at 0x7DF7FE before it is next read.
//
// ---------------------------------------------------------------------------
// SPAN WALK - forward, two bytes per voxel, single pixel
// ---------------------------------------------------------------------------
//   .text:007DF880  mov  al, [edi] ; inc edi     ; skipCount
//   .text:007DF88C  mov  ch, [edi] ; inc edi     ; runCount
//   .text:007DF89A  mov  dl, [edi]               ; colour (normal at [edi+1])
//   .text:007DF89E  add  edi, 2
//   .text:007DF8AD  inc  edi                     ; terminator, even when run == 0
//
// Block layout skip | run | {colour,normal}*run | run-dup, size 3 + 2*run -
// the same long stream fn 16-23 use.
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x7DF8C0 (slot 1, plain endptr), 0x7DF9C0 (slot 4, normal),
//            0x7DFAE0 (slot 5, normal), 0x7DFC00 (slots 8/12, plain),
//            0x7DFD00 (slots 9/13, plain)
//   MISSING: two more asm functions around 0x7DFE00 and 0x7DFF00 that are NOT in
//            the dispatch table - patch sites 0x7DFEE5 / 0x7DFEEB and
//            0x7DFFD7 / 0x7DFFDD. Find their callers.
//   MISSING: VoxelBufferedPixelBuffer (0xB1D5E0) replacement
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. Vanilla's `jmp loc_7DF82D` out of the inner block and the
// `jmp loc_7DF879` back-edge collapse into ordinary loops.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDrawAsm_Plain_StartPtr(
	VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// BUGFIX: vanilla has no zero guard here - see note 5 above.
	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla holds these as one packed dword, which lets the X half carry
	// into the Y half. Split into independent int32 8.8 values.
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
			// Side effect preserved: vanilla stores the current column position
			// back into Start, because the inner block reads it from there.
			pDraw->Start.X = static_cast<std::int16_t>(accX);
			pDraw->Start.Y = static_cast<std::int16_t>(accY);

			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			// Sign test, not `== -1` (jns @ 0x7DF82B).
			if (spanOffset >= 0 && sizeZ != 0)
			{
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;

				// 8-bit counter - wraps rather than going negative.
				int zRemaining = sizeZ;

				// Vanilla tests at the TOP of the block (cmp ecx,0 @ 0x7DF87B),
				// so this is a while, not the do-while the Cpp variants use.
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
						// pSpan[1] is the normal index, unused by this variant.
						pSpan += 2;

						PutPixel(ToWhole(accX), ToWhole(accY), colour);

						accX += stepZX;
						accY += stepZY;

						zRemaining = (zRemaining - 1) & 0xFF;
						--runCount;
					}

					// Terminator byte, consumed even when runCount was 0
					// (inc edi @ 0x7DF8AD is on the shared path).
					++pSpan;
				}
			}

			// Column advance - vanilla loc_7DF82D.
			accX += pDraw->AxisX.X;
			accY += pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla 0x7DF83D.
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
// __cdecl with a single stack argument. Vanilla builds an EBP frame
// (push ebp / mov ebp,esp / add esp,-0Ch) and exits with `leave ; retn`, but at
// the entry point ESP still just points at the return address with the argument
// at [ESP+4], so a plain __cdecl function is a drop-in.
//
// This covers both entry paths - the table slot and the direct call from
// VoxelLibraryClass::Draw+2B3.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x7DF7C0, VoxelDrawAsm_Plain_StartPtr)
