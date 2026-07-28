// ===========================================================================
// Voxel_Draw_Function_Tbl entry "18, spec buffer, startptr"
//   vanilla 0x757120 .. 0x757354
//
// This is the depth-buffered variant. It introduces two things the previous two
// functions did not have: the Z accumulator, and VoxelBufferedPixelBuffer.
//
// ###########################################################################
// # CORRECTION TO MY PREVIOUS MESSAGE                                       #
// ###########################################################################
//
// I told you the paired patch entries were all correct and only the singletons
// were off by one. That was wrong - I generalised from two functions. This one
// is a PAIR and both entries are off by one:
//
//   .text:00757288  88 98 78 FF B2 00   mov  VoxelPixelBuffer[eax], bl
//                   ^^ ^^ \__________/   disp32 at 0x75728A
//   .text:0075728E  88 98 79 FF B2 00   mov  (VoxelPixelBuffer+1)[eax], bl
//                                        disp32 at 0x757290
//
//   your list has 0x75728B and 0x757291  ->  both WRONG, want 0x75728A / 0x757290
//
// Verified tally so far, by instruction address:
//
//   0x756A79 -> 0x756A7B  OK       (Draw_Shadow)
//   0x756A86 -> 0x756A88  OK       (Draw_Shadow)
//   0x756B4A -> 0x756B4C  OK       (Draw_Shadow)
//   0x756B50 -> 0x756B52  OK       (Draw_Shadow)
//   0x756EDC -> 0x756EDF  WRONG    (fn 16)
//   0x757060 -> 0x757063  WRONG    (fn 17)
//   0x757288 -> 0x75728B  WRONG    (fn 18)
//   0x75728E -> 0x757291  WRONG    (fn 18)
//
// The real split is by ADDRESS, not by pairing: everything in Draw_Shadow is
// right, everything from 0x756EDC onward is +1 too far. Treat the rest of the
// list as unverified and re-derive each entry from its instruction address.
// Rule: disp32 sits at instruction + 2 for a plain ModRM+disp32, instruction + 3
// when a SIB byte is present (ModRM rm == 100b).
//
// ###########################################################################
// # VoxelBufferedPixelBuffer FOUND - 0xB1D5E0                               #
// ###########################################################################
//
// The second 256x256 buffer your patch list never touches:
//
//   .text:0075726E  66 0F B6 98 [E0 D5 B1 00]  movzx bx, VoxelBufferedPixelBuffer[eax]
//   .text:00757282  88 90       [E0 D5 B1 00]  mov   VoxelBufferedPixelBuffer[eax], dl
//   .text:00757294  88 90       [E1 D5 B1 00]  mov   (VoxelBufferedPixelBuffer+1)[eax], dl
//
// One BYTE per pixel, indexed with the same (Yint << 8) | Xint packing, and it
// MUST have the same geometry as VoxelPixelBuffer. VoxelRaster.h aliases the
// vanilla array and carries a static_assert that fires the moment BufferSize
// leaves 256. Note the movzx site at 0x757272 is a READ - if you ever relocate
// the buffer, that displacement needs patching too, not just the three writes.
//
// ###########################################################################
//
// A FOURTH 16-BIT LIMIT: THE Z / DEPTH AXIS
// -----------------------------------------
// Depth lives in the function's own incoming argument slot, which it overwrites:
//
//   .text:00757171  mov  ax, [ecx+1Ch]                ; Start.Z
//   .text:0075717D  mov  word ptr [esp+3Ch+a1], ax    ; low WORD only
//
// so the accumulator's upper half starts out as the high half of the caller's
// pointer. It is read back as
//
//   .text:00757265  mov  dx, word ptr [esp+40h+a1]
//   .text:0075726A  shr  dx, 8                        ; LOGICAL, 16-bit
//
// giving an 8-bit depth (0..255). A negative Z wraps to a large positive depth
// rather than clamping. And the skip step is a 16-bit multiply:
//
//   .text:00757202  movzx dx, dl                      ; skipCount
//   .text:00757206  imul  dx, [ecx+2Eh]               ; * AxisZ.Z, truncated to 16
//
// All of that is preserved verbatim. The depth buffer is one byte per pixel and
// is shared with the unported rasterizers, so widening depth is out of scope
// here - only X and Y are widened.
//
// SIDE EFFECT NOT REPRODUCED
// --------------------------
// DIFF: vanilla clobbers its own stack argument slot (it reuses it as the Z
// accumulator). This port leaves the caller's pushed argument intact. Harmless
// under __cdecl - the caller pops with `add esp, 4` and never re-reads it - but
// noted in case some call site is doing something unusual.
//
// WHY THE VANILLA VERSION IS LOCKED TO 256x256 (fourth confirmation)
// ------------------------------------------------------------------
//   .text:0075724D  mov  edx, edi        ; X accumulator
//   .text:00757255  and  edx, 0FFFFh
//   .text:00757260  shr  edx, 8          ; Xint  (0..255)
//   .text:00757253  mov  eax, ebp        ; Y accumulator
//   .text:0075725B  and  eax, 0FF00h     ; Yint << 8
//   .text:00757263  or   eax, edx
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x75748C, 0x7576EE, 0x7578B1, 0x757B1B, 0x757D4F, 0x757F81,
//            0x758118, 0x758358, 0x75855A
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_7572CC` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_SpecBuffer_StartPtr(VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;
	const int stepZZ = pDraw->AxisZ.Z;

	// DIFF: X and Y widened to real int32 8.8 values.
	int rowX = pDraw->Start.X;
	int rowY = pDraw->Start.Y;
	// Z deliberately NOT widened - only its low 16 bits are ever read, and the
	// depth buffer is one byte per pixel. See ToDepth() in VoxelRaster.h.
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

			// startptr variant -> the START table.
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

					// 16-bit signed multiply, truncated - matches `imul dx, [ecx+2Eh]`.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					int runCount = *pSpan++;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							const std::uint8_t colour = pSpan[0];
							// pSpan[1] is the normal index, unused by this variant.
							pSpan += 2;

							PutPixelDepthPair(ToWhole(accX), ToWhole(accY),
								colour, ToDepth(accZ));

							accX += stepZX;
							accY += stepZY;
							accZ += stepZZ;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc esi @ 0x7572C3).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_7572CC.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_75730C.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x757120, VoxelDraw_SpecBuffer_StartPtr)
