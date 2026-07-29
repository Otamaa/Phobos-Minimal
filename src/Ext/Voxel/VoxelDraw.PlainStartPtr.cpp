// ===========================================================================
// Voxel_Draw_Function_Tbl entry "16, plain, startptr"
//   vanilla 0x756DD0 .. 0x756F70
//   pixel write at 0x756EDC  (displacement operand at 0x756EDE)
//
// ###########################################################################
// # PATCH LIST BUG - READ THIS FIRST                                        #
// ###########################################################################
//
// VoxelBufferReplace.cpp currently contains:
//
//     DEFINE_PATCH_TYPED(DWORD, 0x756EDF, DWORD(&Replacer::VoxelPixelBuffer))
//
// That address is OFF BY ONE. The instruction is:
//
//   .text:00756EDC  88 93 78 FF B2 00   mov  VoxelPixelBuffer[ebx], dl
//                   ^^ ^^ \__________/
//                   |  |   disp32 at 0x756EDE .. 0x756EE1
//                   |  ModRM 0x93 = mod 10, reg DL, rm EBX (no SIB byte)
//                   opcode 88 (mov r/m8, r8)
//
// so the displacement starts at 0x756EDE, i.e. instruction + 2. Writing a DWORD
// at 0x756EDF instead:
//
//   - leaves 0x756EDE = 0x78, so the effective displacement becomes
//     (P2 << 24) | (P1 << 16) | (P0 << 8) | 0x78  -> a wild pointer
//   - overwrites 0x756EE2, which is the 0x66 operand-size prefix of the NEXT
//     instruction (66 8B 51 2A -> mov dx, [ecx+2Ah]), corrupting the decode
//
// Cross-check against the sites that ARE correct, all of which are
// instruction + 2:
//
//   0x756A79: 88 8E 78 FF B2 00  -> patched at 0x756A7B  OK
//   0x756A86: 88 96 79 FF B2 00  -> patched at 0x756A88  OK
//   0x756B4A: 88 91 78 FF B2 00  -> patched at 0x756B4C  OK
//   0x756B50: 88 91 79 FF B2 00  -> patched at 0x756B52  OK
//   0x756EDC: 88 93 78 FF B2 00  -> patched at 0x756EDF  WRONG, want 0x756EDE
//
// ACTION: verify the byte at 0x756EDE is 0x78 in your IDB, then either fix that
// one entry to 0x756EDE or delete it entirely (this file replaces the whole
// function, so the patch is redundant once the hook is active). Re-check every
// remaining entry in the list the same way - instruction + 2 for a plain
// ModRM+disp32, instruction + 3 when a SIB byte is present.
//
// ###########################################################################
//
// WHY THE VANILLA VERSION IS LOCKED TO 256x256
// --------------------------------------------
// Same two limits as Draw_Shadow, confirmed again here:
//
// 1) The write offset is an OR, never a multiply:
//
//      .text:00756EC4  mov  ebp, esi          ; esi = X accumulator
//      .text:00756EC8  and  ebp, 0FFFFh
//      .text:00756ED7  shr  ebp, 8            ; Xint  (0..255)
//      .text:00756EC6  mov  ebx, edi          ; edi = Y accumulator
//      .text:00756ECE  and  ebx, 0FF00h       ; Yint << 8
//      .text:00756EDA  or   ebx, ebp
//      .text:00756EDC  mov  VoxelPixelBuffer[ebx], dl
//
// 2) Every coordinate and step is a 16-bit load into the low half of a 32-bit
//    register, leaving stale garbage in the upper half that the masks discard:
//
//      .text:00756E23  mov  si, [ecx+18h]     ; Start.X   (esi upper half stale)
//      .text:00756E27  mov  di, [ecx+1Ah]     ; Start.Y
//      .text:00756E9B  mov  bx, VoxelDistanceLut[edx*4]   ; ebx upper half stale
//      .text:00756EE2  mov  dx, [ecx+2Ah]     ; AxisZ.X
//      .text:00756F10  add  si, word ptr [esp+xx]
//
//    A third 16-bit limit hides in VoxelDistanceLut itself: its entries are
//    int16 accumulations of AxisZ, so they wrap on a tall voxel.
//
// SCOPE: all 21 rasterizers, all four clear helpers and both surface
// initialisers are ported. BufferSize is free.

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

namespace VoxelRaster
{
	std::array<LutEntry, LutSize> DistanceLut {};
}

namespace
{
	// VERIFY: 0xB45590, 255 written entries of { int16 X, int16 Y }.
	VoxelRaster::VanillaLutEntry* const VanillaDistanceLut =
		reinterpret_cast<VoxelRaster::VanillaLutEntry*>(0xB45590);
}

void VoxelRaster::BuildDistanceLut(const FixedVector& axisZ, int count) noexcept
{
	// Vanilla zeroes entry 0 unconditionally, then only runs the loop when
	// SizeZ > 1 (cmp al,1 / jbe @ 0x756DED). Entries at or beyond `count` are
	// deliberately left holding the previous voxel's values.
	DistanceLut[0].X = 0;
	DistanceLut[0].Y = 0;
	VanillaDistanceLut[0].X = 0;
	VanillaDistanceLut[0].Y = 0;

	const int stepX = axisZ.X;
	const int stepY = axisZ.Y;

	for (int i = 1; i < count && i < LutSize; ++i)
	{
		// EXTENSION: int32 accumulation. Vanilla stores int16 here, so a tall
		// voxel with a large AxisZ silently wraps.
		DistanceLut[i].X = DistanceLut[i - 1].X + stepX;
		DistanceLut[i].Y = DistanceLut[i - 1].Y + stepY;

		// Kept in sync so any UNPORTED rasterizer running afterwards still finds
		// the values it expects. Remove once the whole family is ported.
		VanillaDistanceLut[i].X =
			static_cast<std::int16_t>(VanillaDistanceLut[i - 1].X + stepX);
		VanillaDistanceLut[i].Y =
			static_cast<std::int16_t>(VanillaDistanceLut[i - 1].Y + stepY);
	}
}

// ---------------------------------------------------------------------------
// The backport itself. Goto-free; the vanilla `jz loc_756F02` skip paths become
// an early `continue` onto the shared column-advance tail.
// ---------------------------------------------------------------------------
static void __cdecl VoxelDraw_Plain_StartPtr(VoxelRaster::DrawStruct* pDraw) noexcept
{
	using namespace VoxelRaster;

	const int sizeX = pDraw->SizeX;
	const int sizeY = pDraw->SizeY;
	const int sizeZ = pDraw->SizeZ;

	BuildDistanceLut(pDraw->AxisZ, sizeZ);

	const int stepZX = pDraw->AxisZ.X;
	const int stepZY = pDraw->AxisZ.Y;

	// DIFF: vanilla keeps these in the low 16 bits of ESI/EDI with stale garbage
	// above. Widened to a real int32 8.8 value.
	int rowX = pDraw->StartX;
	int rowY = pDraw->StartY;

	// All three sizes are unsigned bytes, hence the `jbe` guards at 0x756E35 and
	// 0x756E59 - they only ever mean "== 0". The for-loops cover that.
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

			const int spanOffset = pDraw->ColumnOffsetsStart[pDraw->DataPos];

			// -1 == no voxels in this column.
			if (spanOffset != -1 && sizeZ != 0)
			{
				const std::uint8_t* pSpan = pDraw->SpanData + spanOffset;
				int zRemaining = sizeZ;

				// BUG (vanilla, preserved): the terminator is `zRemaining != 0`,
				// not `> 0`. Malformed span data whose skip/run counts overshoot
				// drives this negative and the loop never exits, walking off the
				// end of the voxel data. No guard is added here so the behaviour
				// stays bit-identical; add one if you ever load untrusted .vxl.
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
							// pSpan[1] is the normal index, unused by this variant.
							pSpan += 2;

							PutPixel(ToWhole(accX), ToWhole(accY), colour);

							accX += stepZX;
							accY += stepZY;
							--runCount;
						}
						while (runCount != 0);
					}

					// Trailing duplicate run-count byte (inc eax @ 0x756EFD).
					++pSpan;
				}
				while (zRemaining != 0);
			}

			// Shared column advance - vanilla loc_756F02, reached from both the
			// drawn and the skipped path.
			accX = columnX + pDraw->AxisX.X;
			accY = columnY + pDraw->AxisX.Y;
			pDraw->DataPos += pDraw->XSteps;
		}

		// Row advance - vanilla loc_756F35.
		rowX = savedRowX + pDraw->AxisY.X;
		rowY = savedRowY + pDraw->AxisY.Y;
		accX = rowX;
		accY = rowY;

		// DataPos is written back into the caller's struct, not just tracked
		// locally. Side effect preserved.
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
DEFINE_FUNCTION_JUMP(LJMP, 0x756DD0, VoxelDraw_Plain_StartPtr)
