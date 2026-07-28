// ===========================================================================
// Voxel_Draw_Function_Tbl entry "23, normal + spec buffer, endptr"
//   vanilla 0x757BF0 .. 0x757E6A
//
// fn 22 with a backward span walk. Completes the plain/normal x spec/no-spec x
// start/end matrix - eight of twelve rasterizers done.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:00757D4D  88 98 78 FF B2 00  -> disp32 at 0x757D4F, your list: 0x757D4F  OK
//   .text:00757D53  88 98 79 FF B2 00  -> disp32 at 0x757D55, your list: 0x757D55  OK
//
// Running tally: 4 wrong out of 18 (0x756EDF, 0x757063, 0x75728B, 0x757291).
//
// OTHER TABLE SITES IN THIS FUNCTION (none are in your patch list)
//
//   .text:00757CF3  66 0F B6 A8 [E0 D5 B1 00]  READ  depth,   disp32 at 0x757CF7  <- 4-byte prefix+opcode
//   .text:00757D12  8A 9B       [90 59 B4 00]  NormalToLut,   disp32 at 0x757D14
//   .text:00757D3A  88 90       [E0 D5 B1 00]  write depth,   disp32 at 0x757D3C
//   .text:00757D40  88 90       [E1 D5 B1 00]  write depth+1, disp32 at 0x757D42
//   .text:00757D46  8A 9C 2B    [78 11 B4 00]  VPLLookup,     disp32 at 0x757D49  <- SIB, +3
//
// NOTE the depth READ here is a THIRD offset shape: `66 0F B6` is a two-byte
// opcode with an operand-size prefix, so ModRM sits at +3 and the disp32 at +4.
// An audit script keyed only on "+2 or +3" will get this one wrong as well.
//
// REGISTER MAP (differs from fn 22 - worth having when reading the pseudocode)
//   EDI = X accumulator, register-live
//   EBX = Y accumulator, mirrored in var_38
//   EBP = Z accumulator, mirrored in the incoming argument slot
//
// Both EBX and EBP are used as scratch inside the run loop and reloaded from
// their mirrors afterwards. In particular:
//
//   .text:00757CF3  movzx bp, VoxelBufferedPixelBuffer[eax]   ; clobbers Z acc
//   .text:00757D66  mov   ebp, [esp+48h+a1]                   ; reloaded
//
// and the clobber happens on the FAIL path too, which is why the reload sits
// after the jbe target rather than inside the pass arm.
//
// LAZY BYTE FETCH, same as fn 22
//
//   .text:00757CFE  jbe  loc_757D5F        ; fail
//   .text:00757D00  mov  bl, [esi]         ; normal at P   (pass only)
//   .text:00757D1C  mov  bl, [esi+1]       ; colour at P-1 (pass only)
//   loc_757D5F:
//   .text:00757D5F  sub  esi, 2            ; fail advances the same 2
//
// Pair order matches fn 21 exactly: walking down from P, normal at P, colour at
// P-1, step -2.
//
// SUSPECT: the accumulators still step by +AxisZ despite blocks arriving in
// reverse Z order, so the caller must supply a pre-negated AxisZ and a far-end
// Start. Preserved verbatim.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:00757CD8  mov  edx, edi        ; X accumulator
//   .text:00757CDC  and  edx, 0FFFFh
//   .text:00757CE7  shr  edx, 8          ; Xint (0..255)
//   .text:00757CDA  mov  eax, ebx        ; Y accumulator
//   .text:00757CE2  and  eax, 0FF00h     ; Yint << 8
//   .text:00757CEA  or   eax, edx
//
// SCOPE - STILL MISSING
// ---------------------
//   MISSING: 0x757F81, 0x758118, 0x758358, 0x75855A
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//
// HEADS UP for the next two: in your original patch list, 0x757F81/0x757F87 and
// 0x758118/0x75811E are the only pairs where the SECOND entry does not carry the
// `+ 1`, unlike every other pair. Either those two functions genuinely use the
// base twice, or the `+ 1` was dropped. Worth watching for.
//
// Replacer::BufferSize MUST stay 256 until all of the above are done.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The vanilla jbe/jmp pair around loc_757D5F becomes a
// plain if with the pointer advance hoisted out of both arms.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_NormalSpecBuffer_EndPtr(VoxelRaster::DrawStruct* pDraw) noexcept
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

			// endptr variant -> the END table (mov eax,[ecx+4] @ 0x757C8C).
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
							const int pixelX = ToWhole(accX);
							const int pixelY = ToWhole(accY);
							const std::uint8_t depth = ToDepth(accZ);

							// Lazy fetch, as in fn 22 - vanilla only reads these
							// and only indexes VPLLookup on a passing test.
							if (DepthTest(pixelX, pixelY, depth))
							{
								// Reading backwards: normal above, colour below.
								const std::uint8_t normal = pSpan[0];
								const std::uint8_t colour = pSpan[-1];

								// Vanilla write order: both depth bytes, then
								// both pixel bytes.
								PutDepthPair(pixelX, pixelY, depth);
								PutPixelPair(pixelX, pixelY, Shade(colour, normal));
							}

							// Both arms retreat by 2 (two `dec esi` on the pass
							// path, `sub esi, 2` @ 0x757D5F on the fail path).
							pSpan -= 2;

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
					// `movzx dx, byte[var_34]` / `imul dx, [ecx+2Eh]` @ 0x757DB5.
					accZ += static_cast<std::int16_t>(skipCount * stepZZ);

					zRemaining -= skipCount;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_757DDA.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			accZ = columnZ + pDraw->AxisX.Z;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_757E1E.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x757BF0, VoxelDraw_NormalSpecBuffer_EndPtr)
