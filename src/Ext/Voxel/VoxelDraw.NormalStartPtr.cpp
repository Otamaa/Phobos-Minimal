// ===========================================================================
// Voxel_Draw_Function_Tbl entry "20, normal, startptr (MAIN)"
//   vanilla 0x7575A0 .. 0x75778A
//
// The lit variant. Span walk is identical to fn 16 (forward, skip-then-run);
// the new mechanism is per-voxel shading from the normal byte, which the plain
// variants throw away.
//
// PATCH LIST STATUS FOR THIS FUNCTION: BOTH CORRECT
//
//   .text:007576EC  88 91 78 FF B2 00  -> disp32 at 0x7576EE, your list: 0x7576EE  OK
//   .text:007576F2  88 91 79 FF B2 00  -> disp32 at 0x7576F4, your list: 0x7576F4  OK
//
// Running tally now 4 wrong out of 12 (0x756EDF, 0x757063, 0x75728B, 0x757291).
// Still no pattern - the remaining entries need individual verification.
//
// A CONCRETE SIB EXAMPLE FOR THE AUDIT
// ------------------------------------
// This function contains the one encoding shape that breaks the "+2" rule:
//
//   .text:007576E5  8A 94 1A 78 11 B4 00   mov dl, VPLLookup[edx+ebx]
//                   ^^ ^^ ^^ \__________/
//                   |  |  |   disp32 at instruction + 3
//                   |  |  SIB 0x1A = scale 1, index EBX, base EDX
//                   |  ModRM 0x94 -> mod 10, reg DL, rm 100b = SIB FOLLOWS
//                   opcode 8A
//
// Compare with the pixel writes just below it, ModRM 0x91 (rm == 001b = ECX,
// no SIB), disp32 at instruction + 2. Any audit script has to branch on
// rm == 100b or it will be off by one on every SIB-form instruction.
//
// LIGHTING
// --------
//   .text:00757696  mov  dl, [eax]        ; colour
//   .text:00757698  mov  cl, [eax+1]      ; normal
//   .text:007576B9  mov  cl, VoxelNormalToLut[edx]
//   .text:007576E2  shl  edx, 8
//   .text:007576E5  mov  dl, VPLLookup[edx+ebx]
//
//     shade = VPLLookup[ VoxelNormalToLut[normal] * 256 + colour ]
//
// VoxelNormalToLut = 0xB45990, VPLLookup = 0xB41178. Both wired up in
// VoxelRaster.h as VoxelRaster::Shade().
//
// No depth test in this variant, and both pixels get the same shade.
//
// The 256x256 lock and the 16-bit limits are unchanged:
//
//   .text:007576BF  mov  edx, edi        ; X accumulator
//   .text:007576C7  and  edx, 0FFFFh
//   .text:007576D3  shr  edx, 8          ; Xint (0..255)
//   .text:007576C5  mov  ecx, ebp        ; Y accumulator
//   .text:007576CD  and  ecx, 0FF00h     ; Yint << 8
//   .text:007576D6  or   ecx, edx
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Goto-free backport. The two `jz loc_75771C` skip paths become an early exit
// from the span walk onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_Normal_StartPtr(VoxelRaster::DrawStruct* pDraw) noexcept
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

			// startptr variant -> the START table (mov edx,[esi] @ 0x757630).
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

					int runCount = *pSpan++;

					if (runCount != 0)
					{
						zRemaining -= runCount;

						do
						{
							const std::uint8_t colour = pSpan[0];
							const std::uint8_t normal = pSpan[1];
							pSpan += 2;

							PutPixelPair(ToWhole(accX), ToWhole(accY),
								Shade(colour, normal));

							accX += stepZX;
							accY += stepZY;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc eax @ 0x757713).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_75771C.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_75774F.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x7575A0, VoxelDraw_Normal_StartPtr)
