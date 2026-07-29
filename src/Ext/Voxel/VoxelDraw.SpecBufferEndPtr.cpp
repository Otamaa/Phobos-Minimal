// ===========================================================================
// Voxel_Draw_Function_Tbl entry "19, spec buffer, endptr"
//   vanilla 0x757360 .. 0x757592
//
// The last of the four "plain/spec x start/end" combinations: fn 17's backward
// span walk plus fn 18's depth test. No new mechanisms.
//
// ###########################################################################
// # SECOND CORRECTION - THERE IS NO PATTERN IN THE PATCH LIST ERRORS        #
// ###########################################################################
//
// I have now been wrong twice about this. First I said "singletons wrong, pairs
// right"; fn 18 disproved that. Then I said "everything from 0x756EDC onward is
// wrong"; this function disproves that too. Both of fn 19's entries are CORRECT:
//
//   .text:0075748A  88 98 78 FF B2 00  -> disp32 at 0x75748C, your list: 0x75748C  OK
//   .text:00757490  88 98 79 FF B2 00  -> disp32 at 0x757492, your list: 0x757492  OK
//
// Running tally, verified byte-by-byte against the disassembly:
//
//   0x756A79 -> 0x756A7B  OK     Draw_Shadow
//   0x756A86 -> 0x756A88  OK     Draw_Shadow
//   0x756B4A -> 0x756B4C  OK     Draw_Shadow
//   0x756B50 -> 0x756B52  OK     Draw_Shadow
//   0x756EDC -> 0x756EDF  WRONG  fn 16, want 0x756EDE
//   0x757060 -> 0x757063  WRONG  fn 17, want 0x757062
//   0x757288 -> 0x75728B  WRONG  fn 18, want 0x75728A
//   0x75728E -> 0x757291  WRONG  fn 18, want 0x757290
//   0x75748A -> 0x75748C  OK     fn 19
//   0x757490 -> 0x757492  OK     fn 19
//
// 4 wrong out of 10. Scattered, no rule. The remaining ~30 entries are simply
// unverified - each one has to be checked against its own instruction. Rule:
// disp32 at instruction + 2 for a plain ModRM+disp32, instruction + 3 when a SIB
// byte is present (ModRM rm == 100b). A quick IDB script over every xref to
// 0xB2FF78 / 0xB2FF79 / 0xB1D5E0 / 0xB1D5E1 would settle the whole list faster
// than doing it by eye.
//
// ###########################################################################
//
// DEPTH BUFFER SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:00757473  66 0F B6 98 [E0 D5 B1 00]  READ  disp32 at 0x757477
//   .text:00757484  88 90       [E0 D5 B1 00]  write disp32 at 0x757486
//   .text:00757496  88 90       [E1 D5 B1 00]  write disp32 at 0x757498
//
// SPAN WALK
// ---------
// Backward, identical pointer arithmetic to fn 17 - enter at the block's
// duplicate runCount terminator, run first, then skip:
//
//   .text:00757434  mov  al, [esi]      ; runCount
//   .text:00757436  dec  esi
//   .text:0075744D  mov  al, [esi-1]    ; colour, normal at [esi]
//   .text:00757450  dec  esi
//   .text:00757464  dec  esi            ; 2 bytes per voxel
//   .text:007574C1  mov  dl, [esi-1]    ; skipCount
//   .text:007574C4  dec  esi
//   .text:007574C9  dec  esi
//
// SUSPECT: as in fn 17, the accumulators still step by +AxisZ even though blocks
// arrive in reverse Z order, so the caller must supply a pre-negated AxisZ and a
// far-end Start. Preserved verbatim - do not "fix" the sign.
//
// The 256x256 lock and the four 16-bit limits are the same as the previous
// three functions; see the block comment at the top of VoxelRaster.h.
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_75750A` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_SpecBuffer_EndPtr(VoxelRaster::DrawStruct* pDraw) noexcept
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
	int rowX = pDraw->StartX;
	int rowY = pDraw->StartY;
	int rowZ = pDraw->StartZ;

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

			// endptr variant -> the END table (mov edx,[ecx+4] @ 0x757405).
			const int spanOffset = pDraw->ColumnOffsetsEnd[pDraw->DataPos];

			if (spanOffset != -1 && sizeZ != 0)
			{
				// Points at the block's duplicate runCount terminator.
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// BUG (vanilla, preserved): terminator is `!= 0`, not `> 0`.
				// Malformed span data walks backwards off the front of the data.
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

							PutPixelDepthPair(ToWhole(accX), ToWhole(accY),
								colour, ToDepth(accZ));

							accX += stepZX;
							accY += stepZY;
							accZ += stepZZ;
							--runCount;
						}
						while (runCount != 0);
					}

					const int skipCount = pSpan[-1];
					pSpan -= 2;

					accX += DistanceLut[skipCount].X;
					accY += DistanceLut[skipCount].Y;

					// 16-bit signed multiply, truncated - matches
					// `movzx dx, dl` / `imul dx, [ecx+2Eh]` @ 0x7574CE.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					zRemaining -= skipCount;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_75750A.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_75754A.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x757360, VoxelDraw_SpecBuffer_EndPtr)
