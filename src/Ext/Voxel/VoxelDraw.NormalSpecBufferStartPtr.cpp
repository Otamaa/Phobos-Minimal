// ===========================================================================
// Voxel_Draw_Function_Tbl entry "22, normal + spec buffer, startptr"
//   vanilla 0x757980 .. 0x757BE4
//
// Forward span walk, depth test AND shading. First variant to combine both.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:00757B19  88 98 78 FF B2 00  -> disp32 at 0x757B1B, your list: 0x757B1B  OK
//   .text:00757B1F  88 98 79 FF B2 00  -> disp32 at 0x757B21, your list: 0x757B21  OK
//
// Running tally: 4 wrong out of 16 (0x756EDF, 0x757063, 0x75728B, 0x757291).
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:00757ABC  66 0F B6 98 [E0 D5 B1 00]  READ  depth,  disp32 at 0x757AC0
//   .text:00757ADF  88 90       [E0 D5 B1 00]  write depth,  disp32 at 0x757AE1
//   .text:00757AEB  88 90       [E1 D5 B1 00]  write depth+1,disp32 at 0x757AED
//   .text:00757AF1  8A 9B       [90 59 B4 00]  NormalToLut,  disp32 at 0x757AF3
//   .text:00757B0E  8A 9C 2B    [78 11 B4 00]  VPLLookup,    disp32 at 0x757B11  <- SIB, +3
//
// STRUCTURAL QUIRK: LAZY BYTE FETCH
// ---------------------------------
// Unlike fn 18, this variant does NOT read the colour/normal bytes before the
// depth test. It tests first and only fetches on success:
//
//   .text:00757AC4  cmp  dx, bx
//   .text:00757AC7  jbe  loc_757B27       ; FAIL path
//   .text:00757AC9  mov  bl, [esi] ; inc esi    ; colour   (pass only)
//   .text:00757AD0  mov  bl, [esi] ; inc esi    ; normal   (pass only)
//   ...
//   loc_757B27:
//   .text:00757B27  add  esi, 2           ; FAIL path advances the same 2 bytes
//
// Both paths advance the span pointer by exactly 2, so the net effect is
// identical to fn 18 - but the port keeps the lazy fetch anyway, because on the
// fail path vanilla never performs the VPLLookup indexing. Given the VPL table
// is probably smaller than the byte index range NormalToLut can produce (see
// VoxelRaster.h), doing the lookup eagerly could touch memory vanilla never
// touches. Not worth the risk for zero benefit.
//
// THE Z ACCUMULATOR IS DESTROYED IN PLACE EACH ITERATION
// ------------------------------------------------------
//   .text:00757AB8  shr  dx, 8            ; EDX holds the Z accumulator - this
//                                         ; overwrites its low word with the
//                                         ; 8-bit depth value
//   .text:00757B2E  mov  edx, [esp+48h+var_30]   ; reloaded from the stack copy
//
// var_30 is the live Z accumulator and var_2C the live Y accumulator; EDX and
// EBP are only scratch inside the run loop. This is the main reason the IDA
// pseudocode for this function aliases v1/v2/v16 so confusingly.
//
// SUSPECT: zRemaining lives in the function's own incoming argument slot
// (`mov [esp+48h+a1], ebx` @ 0x757A90), so vanilla clobbers its argument here
// too. DIFF: this port leaves the caller's pushed argument intact.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:00757AA4  mov  ebx, edi        ; X accumulator
//   .text:00757AA8  and  ebx, 0FFFFh
//   .text:00757AB3  shr  ebx, 8          ; Xint (0..255)
//   .text:00757AA6  mov  eax, ebp        ; Y accumulator
//   .text:00757AAE  and  eax, 0FF00h     ; Yint << 8
//   .text:00757AB6  or   eax, ebx
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x757D4F, 0x757F81, 0x758118, 0x758358, 0x75855A
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The vanilla jbe/jmp pair around loc_757B27 becomes a
// plain if/else with the pointer advance hoisted out of both arms.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_NormalSpecBuffer_StartPtr(VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;
	const int stepZZ = pDraw->AxisZ.Z;

	// DIFF: X and Y widened to real int32 8.8 values. Z left alone - only its low
	// 16 bits are ever read and the depth buffer is one byte per pixel.
	int rowX = pDraw->Start.X;
	int rowY = pDraw->Start.Y;
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

			// startptr variant -> the START table (mov esi,[ecx] @ 0x757A1A).
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

					// 16-bit signed multiply, truncated - matches
					// `movzx bx, byte[var_38]` / `imul bx, [ecx+2Eh]` @ 0x757A6E.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					int runCount = *pSpan++;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							const int pixelX = ToWhole(accX);
							const int pixelY = ToWhole(accY);
							const std::uint8_t depth = ToDepth(accZ);

							// Lazy fetch: vanilla only reads these on a passing
							// test. Kept that way deliberately - see header note.
							if (DepthTest(pixelX, pixelY, depth))
							{
								const std::uint8_t colour = pSpan[0];
								const std::uint8_t normal = pSpan[1];

								// Vanilla write order: both depth bytes, then
								// both pixel bytes.
								PutDepthPair(pixelX, pixelY, depth);
								PutPixelPair(pixelX, pixelY, Shade(colour, normal));
							}

							// Both arms advance by 2 (inc esi twice on the pass
							// path, `add esi, 2` @ 0x757B27 on the fail path).
							pSpan += 2;

							accX += stepZX;
							accY += stepZY;
							accZ += stepZZ;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc esi @ 0x757B5B).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_757B64.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_757BA0.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x757980, VoxelDraw_NormalSpecBuffer_StartPtr)
