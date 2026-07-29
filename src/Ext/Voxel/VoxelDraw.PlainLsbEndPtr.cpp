// ===========================================================================
// Voxel_Draw_Function_Tbl entries "25 / 29, plain, endptr"
//   vanilla 0x758030 .. 0x7581ED
//   (IDA names it _10_25_29_, i.e. referenced from two table slots)
//
// The backward-walking twin of fn 24/28. Same (x | 1) second-pixel address,
// same 1-byte-per-voxel span stream.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:00758116  88 93 78 FF B2 00  mov VoxelPixelBuffer[ebx], dl
//                                      disp32 at 0x758118, value = base       OK
//   .text:0075811C  88 96 78 FF B2 00  mov VoxelPixelBuffer[esi], dl
//                                      disp32 at 0x75811E, value = base       OK
//
// This confirms the prediction made in VoxelDraw.PlainLsbStartPtr.cpp: the
// second entry legitimately carries no `+ 1` because the increment lives in the
// index register:
//
//   .text:00758105  and  esi, 0FF00h     ; esi = Yint << 8
//   .text:0075810B  shr  eax, 8          ; eax = Xint
//   .text:0075810E  mov  ebx, eax
//   .text:00758111  or   ebx, esi        ; idx1 = (Yint << 8) | Xint
//   .text:00758113  inc  esi             ; esi  = (Yint << 8) | 1
//   .text:00758114  or   esi, eax        ; idx2 = (Yint << 8) | (Xint | 1)
//
// FINAL TALLY for the 12 entries verified across these 10 functions:
//   4 wrong: 0x756EDF, 0x757063, 0x75728B, 0x757291
//   8 right: 0x756A7B, 0x756A88, 0x756B4C, 0x756B52, 0x75748C, 0x757492,
//            0x7576EE, 0x7576F4, 0x757B1B, 0x757B21, 0x757D4F, 0x757D55,
//            0x7578B1, 0x7578B7, 0x757F81, 0x757F87, 0x758118, 0x75811E
//
// SPAN POINTER WALK
// -----------------
// Enter at the block's duplicate runCount terminator E, then:
//
//   .text:007580EB  mov  al, [edi]     ; runCount at E
//   .text:007580ED  dec  edi           ; -> E-1
//   .text:007580FC  mov  dl, [edi]     ; colour, ONE byte per voxel
//   .text:00758110  dec  edi           ; -> step -1, not -2
//   .text:00758145  mov  al, [edi-1]   ; skipCount
//   .text:00758148  dec  edi
//   .text:0075814D  dec  edi           ; -> previous block's terminator
//
// Block layout forward is skip | run | colour*run | run-dup, size 3 + run, and
// the pointer moves exactly 3 + run bytes per block. Consistent.
//
// REGISTER MAP
//   EBP = X accumulator, register-live
//   ESI = Y accumulator, mirrored in the incoming argument slot
//   EDX = zRemaining, mirrored in var_1C because EDX doubles as the colour byte
//
//   .text:00758105  and  esi, 0FF00h          ; destroys the Y accumulator
//   .text:00758126  mov  esi, [esp+38h+a1]    ; reloaded from the mirror
//
// SUSPECT: as in every endptr variant, the accumulators still step by +AxisZ
// despite blocks arriving in reverse Z order, so the caller must supply a
// pre-negated AxisZ and a far-end Start. Preserved verbatim.
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_758177` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_PlainLsb_EndPtr(VoxelRaster::DrawStruct* pDraw) noexcept
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

			// endptr variant -> the END table (mov eax,[ecx+4] @ 0x7580C0).
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
							// One byte per voxel - no normal byte in this stream.
							const std::uint8_t colour = pSpan[0];
							pSpan -= 1;

							// Second pixel at (x | 1), not (x + 1).
							PutPixelPairSetLsb(ToWhole(accX), ToWhole(accY), colour);

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

			// Shared column advance - vanilla loc_758177.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_7581AE.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x758030, VoxelDraw_PlainLsb_EndPtr)
