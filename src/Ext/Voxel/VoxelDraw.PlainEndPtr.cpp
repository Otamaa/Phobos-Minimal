// ===========================================================================
// Voxel_Draw_Function_Tbl entry "17, plain, endptr"
//   vanilla 0x756F80 .. 0x75711A
//   pixel write at 0x757060  (displacement operand at 0x757062)
//
// ###########################################################################
// # PATCH LIST BUG #2 - SAME OFF-BY-ONE AS 0x756EDF                         #
// ###########################################################################
//
// VoxelBufferReplace.cpp currently contains:
//
//     DEFINE_PATCH_TYPED(DWORD, 0x757063, DWORD(&Replacer::VoxelPixelBuffer))
//
// Wrong by one. The instruction is:
//
//   .text:00757060  88 93 78 FF B2 00   mov  VoxelPixelBuffer[ebx], dl
//                   ^^ ^^ \__________/
//                   |  |   disp32 at 0x757062 .. 0x757065
//                   |  ModRM 0x93 = mod 10, reg DL, rm EBX (no SIB byte)
//                   opcode 88
//
// Writing a DWORD at 0x757063 leaves 0x757062 = 0x78 (garbage displacement) and
// clobbers 0x757066, which is the 0x66 operand-size prefix of the next
// instruction (66 8B 51 2A -> mov dx, [ecx+2Ah]).
//
// THE PATTERN: every PAIRED entry in your list is correct (instruction + 2), and
// both SINGLETON entries are +3. Confirmed so far:
//
//   0x756A79 -> 0x756A7B  OK      (paired)
//   0x756A86 -> 0x756A88  OK      (paired, +1)
//   0x756B4A -> 0x756B4C  OK      (paired)
//   0x756B50 -> 0x756B52  OK      (paired, +1)
//   0x756EDC -> 0x756EDF  WRONG, want 0x756EDE   (singleton)
//   0x757060 -> 0x757063  WRONG, want 0x757062   (singleton)
//
// ACTION: re-derive every entry from the instruction address rather than by
// hand. Rule: disp32 sits at instruction + 2 for a plain ModRM+disp32, and at
// instruction + 3 when a SIB byte is present (ModRM rm == 100b).
// Both singletons become redundant anyway once this file and
// VoxelDraw.PlainStartPtr.cpp replace their functions outright.
//
// ###########################################################################
//
// STRUCT CORRECTION
// -----------------
// +0x00 and +0x04 are TWO separate column-offset tables, not one pointer plus a
// spare field:
//
//   fn 16 (startptr) @ 0x756E62:  mov eax, [ecx]     -> ColumnOffsetsStart
//   fn 17 (endptr)   @ 0x75700C:  mov eax, [ecx+4]   -> ColumnOffsetsEnd
//
// VoxelRaster.h has been updated. Anything that was reading the old
// `ColumnOffsets` / `Unknown04` names needs renaming.
//
// HOW THE BACKWARD WALK WORKS
// ---------------------------
// This variant enters at the LAST byte of a span (the duplicate runCount
// terminator) and walks down, so the block is consumed in the opposite order to
// fn 16 - run first, then skip:
//
//   .text:00757035  mov  dl, [eax]      ; runCount (the duplicate terminator)
//   .text:00757037  dec  eax
//   .text:00757046  mov  dl, [eax-1]    ; colour, with normal at [eax]
//   .text:00757053  dec  eax
//   .text:0075705F  dec  eax            ; -> 2 bytes per voxel
//   .text:00757081  mov  dl, [eax-1]    ; skipCount
//   .text:00757084  dec  eax
//   .text:00757089  dec  eax            ; -> lands on the previous block's tail
//
// SUSPECT: the accumulator is still stepped by +AxisZ per voxel, exactly as in
// the forward variant, even though the blocks arrive in reverse Z order. The
// caller must therefore hand this variant a pre-negated AxisZ and a Start at the
// far end. Preserved verbatim - do not "fix" the sign here.
//
// WHY THE VANILLA VERSION IS LOCKED TO 256x256 (third confirmation)
// -----------------------------------------------------------------
//   .text:00757049  mov  ebp, esi       ; X accumulator
//   .text:0075704D  and  ebp, 0FFFFh
//   .text:0075705A  shr  ebp, 8         ; Xint  (0..255)
//   .text:0075704B  mov  ebx, edi       ; Y accumulator
//   .text:00757054  and  ebx, 0FF00h    ; Yint << 8
//   .text:0075705D  or   ebx, ebp
//   .text:00757060  mov  VoxelPixelBuffer[ebx], dl
//
// plus the usual 16-bit loads into register halves (0x756FD2, 0x756FD6,
// 0x757066, 0x75706C, 0x757094, 0x75709E, 0x7570BA, 0x7570EE).
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x75728B, 0x75748C, 0x7576EE, 0x7578B1, 0x757B1B, 0x757D4F,
//            0x757F81, 0x758118, 0x758358, 0x75855A
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (second 256x256 array) not replaced
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The vanilla `jz loc_7570AC` skip paths become an early
// exit from the span block onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_Plain_EndPtr(VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	// Identical build to fn 16; both tables are refreshed.
	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla keeps these in the low 16 bits of ESI/EDI with stale garbage
	// above (mov si,[ecx+18h] @ 0x756FD2). Widened to a real int32 8.8 value.
	int rowX = pDraw->Start.X;
	int rowY = pDraw->Start.Y;

	// All three sizes are unsigned bytes; the `cmp al, dl` / `jbe` guards at
	// 0x756FDA and 0x756FF9 only ever mean "== 0", which the for-loops cover.
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

			// NOTE: the END table, not the start table.
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			if (spanOffset != -1 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// BUG (vanilla, preserved): terminator is `!= 0`, not `> 0`.
				// Malformed span data whose counts overshoot drives this negative
				// and the loop walks backwards off the front of the voxel data.
				// No guard added, so behaviour stays bit-identical.
				do
				{
					int runCount = pSpan[0];
					--pSpan;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							// Pair is { colour, normal }; reading backwards the
							// normal sits at pSpan[0] and is unused here.
							const std::uint8_t colour = pSpan[-1];
							pSpan -= 2;

							PutPixel(ToWhole(accX), ToWhole(accY), colour);

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

			// Shared column advance - vanilla loc_7570AC, reached from both the
			// drawn and the skipped path.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_7570DF.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		accX = rowX;
		accY = rowY;

		// DataPos is written back into the caller's struct. Side effect preserved.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x756F80, VoxelDraw_Plain_EndPtr)
