// ===========================================================================
// VoxelLibraryClass::Draw_Shadow  -  vanilla 0x756860 .. 0x756BA2
//
// Goto-free C++ backport with stride-aware addressing, so the voxel pixel
// buffer is no longer locked to 256x256.
//
// WHY THE VANILLA VERSION IS LOCKED TO 256x256
// --------------------------------------------
// Two independent hard limits, both confirmed in the disassembly:
//
// 1) The write offset is a bit merge, never a multiply:
//
//      .text:00756A04  mov  esi, ebx          ; ebx = X accumulator
//      .text:00756A0A  and  esi, 0FFFFh
//      .text:00756A12  shr  esi, 8            ; esi = Xint          (0..255)
//      .text:00756A10  mov  edx, eax          ; eax = Y accumulator
//      .text:00756A15  and  edx, 0FF00h       ; edx = Yint << 8     (0..255)
//      .text:00756A1B  or   esi, edx          ; idx = (Yint << 8) | Xint
//      .text:00756A79  mov  VoxelPixelBuffer[esi], cl
//      .text:00756A86  mov  (VoxelPixelBuffer+1)[esi], dl
//
//    idx == Yint * 256 + Xint. The 256 is produced by the OR, so there is no
//    immediate to patch. Base symbol is 0xB2FF78, +1 at 0xB2FF79.
//
// 2) The accumulators and all four step values are stored as int16:
//
//      .text:007568DA  mov  [esp+anonymous_0], ax     ; startX = ...*256.0
//      .text:007568F0  mov  [esp+anonymous_1], ax     ; startY
//      .text:0075692A  mov  [esp+anonymous_2], ax     ; stepColX
//      .text:0075697F  mov  [esp+anonymous_3], ax     ; stepColY
//      .text:00756965  mov  word ptr [esp+var_10],   ax   ; stepRowX
//      .text:0075699A  mov  word ptr [esp+var_10+2], ax   ; stepRowY
//
//    Everything is 8.8 fixed point (the *256.0 before __ftol) held in 16 bits,
//    so the integer coordinate range is 0..255 WITH WRAPAROUND. On an oversized
//    voxel the wrap is what produces the stripes/garbage, not a clip.
//
// Setting VoxelSurface->Width = 512 only changes the pitch that READERS use.
// The writer above still steps 256. That mismatch is the reported bug.
//
// WHAT THIS FILE CHANGES
// ----------------------
// - accumulators and steps widened to int32 (still 8.8, so identical fractional
//   precision; only the 16-bit truncation is gone)
// - idx = y * Replacer::BufferSize + x, an explicit multiply
// - explicit bounds check replaces the implicit 8-bit wrap
//
// SCOPE WARNING - THIS IS ONE FUNCTION OF THE FAMILY
// --------------------------------------------------
// Draw_Shadow shares VoxelPixelBuffer with the colour rasterizers. Those are
// NOT ported here and still use the (Yint << 8) | Xint packing:
//
//   MISSING: 0x756EDF, 0x757063, 0x75728B, 0x75748C, 0x7576EE, 0x7578B1,
//            0x757B1B, 0x757D4F, 0x757F81, 0x758118, 0x758358, 0x75855A
//   MISSING: Asm_Voxel_Normals_Function_Old_* family, 0x7DF8A7 .. 0x7DFFDD
//   MISSING: VoxelBufferedPixelBuffer (second 256x256 array) is not replaced
//
// Until all of the above are ported, Replacer::BufferSize MUST stay 256.
// ===========================================================================

#include "VoxelBufferReplace.h"

#include <YRPP.h>
#include <Utilities/Macro.h>

#include <cstdint>

// ---------------------------------------------------------------------------
// Layout mirrors. VERIFY every offset against the IDB before trusting these;
// they are derived from the addressing modes in Draw_Shadow only.
// ---------------------------------------------------------------------------
namespace VoxelShadow
{
	// VERIFY: 12 bytes per element.
	//   lea eax,[eax+eax*2] ; mov eax,[edx+eax*4]   -> stride 12, Index at +0
	struct LayerHeader
	{
		int Index;      // +0x00
		int Unknown04;  // +0x04  VERIFY
		int Unknown08;  // +0x08  VERIFY
	};
	static_assert(sizeof(LayerHeader) == 12, "LayerHeader stride is 12 bytes.");

	// VERIFY: 164 bytes per element.
	//   lea edx,[eax+eax*4] ; lea eax,[eax+edx*8] -> eax*41 ; [ecx+eax*4] -> 164
	struct LayerInfo
	{
		int            Unknown00;        // +0x00  VERIFY
		const int*     ColumnOffsets;    // +0x04  IDA called this DataEndPtr.
		                                 //        Indexed [y * SizeX + x];
		                                 //        -1 means "no voxel in column".
		std::uint8_t   Padding08[0x98];  // +0x08 .. +0x9F  VERIFY
		std::uint8_t   SizeX;            // +0xA0
		std::uint8_t   SizeY;            // +0xA1
		std::uint8_t   PaddingA2[0x02];  // +0xA2 .. +0xA3  VERIFY
	};
	static_assert(offsetof(LayerInfo, ColumnOffsets) == 0x04, "ColumnOffsets @ +4");
	static_assert(offsetof(LayerInfo, SizeX) == 0xA0, "SizeX @ +0xA0");
	static_assert(offsetof(LayerInfo, SizeY) == 0xA1, "SizeY @ +0xA1");
	static_assert(sizeof(LayerInfo) == 164, "LayerInfo stride is 164 bytes.");

	struct Library
	{
		std::uint8_t Padding00[0x10];  // +0x00 .. +0x0F  VERIFY
		LayerHeader* LayerHeaders;     // +0x10   mov edx,[ecx+10h]
		LayerInfo*   LayerInfos;       // +0x14   mov ecx,[ecx+14h]
	};
	static_assert(offsetof(Library, LayerHeaders) == 0x10, "LayerHeaders @ +0x10");
	static_assert(offsetof(Library, LayerInfos) == 0x14, "LayerInfos @ +0x14");

	// Only the fields Draw_Shadow touches are named. Everything else is padding.
	struct DrawStruct
	{
		int      Unknown00;      // +0x00  VERIFY
		int      HeaderIndex;    // +0x04
		int      InfoIndex;      // +0x08
		int      Unknown0C;      // +0x0C  VERIFY
		int      Unknown10;      // +0x10  VERIFY
		int      Unknown14;      // +0x14  VERIFY
		float    V1X;            // +0x18
		float    V1Y;            // +0x1C
		float    Unknown20;      // +0x20  VERIFY (V1Z?)
		float    V2X;            // +0x24
		float    V2Y;            // +0x28
		float    Unknown2C;      // +0x2C  VERIFY (V2Z?)
		float    V3X;            // +0x30
		float    V3Y;            // +0x34
		float    Unknown38;      // +0x38  VERIFY (V3Z?)
		Surface* SurfacePtr;     // +0x3C
		int      ShadowPointX;   // +0x40
		int      ShadowPointY;   // +0x44
	};
	static_assert(offsetof(DrawStruct, V2X) == 0x24, "V2X @ +0x24");
	static_assert(offsetof(DrawStruct, SurfacePtr) == 0x3C, "SurfacePtr @ +0x3C");
	static_assert(offsetof(DrawStruct, ShadowPointY) == 0x44, "ShadowPointY @ +0x44");
}

namespace
{
	// VERIFY: __ftol lives at 0x7C5F00 (call target of E8 2F F6 06 00 @ 0x7568CC).
	// It is C-runtime truncation toward zero, which is what static_cast<int> does.
	// SUSPECT: vanilla evaluates the divides on the x87 stack at 80-bit precision;
	// MSVC will use SSE at 64-bit here. A 1-ULP difference can shift the truncated
	// result by one 1/256th of a pixel in rare cases. DIFF: accepted.
	inline int F2I(double value) noexcept
	{
		return static_cast<int>(value);
	}

	// 8.8 fixed point, matching the *256.0 the vanilla code applies before __ftol.
	inline constexpr int FixedShift = 8;

	inline constexpr int ToWhole(int fixedValue) noexcept
	{
		// SUSPECT: vanilla masks rather than shifts, so it never sees a negative
		// coordinate. An arithmetic shift right keeps negatives negative, which the
		// bounds check below then rejects. That is the intended DIFF.
		return fixedValue >> FixedShift;
	}

	// EXTENSION: explicit bounds check. Vanilla had none - the 8-bit masks made
	// out-of-range coordinates wrap silently into the buffer, which is exactly the
	// stripe/garbage artefact seen on oversized voxels.
	// BUGFIX: also fixes the vanilla row-bleed at Xint == 255, where the second
	// write (VoxelPixelBuffer+1)[esi] landed in column 0 of the NEXT row.
	inline void PutShadowPixel(int x, int y, std::uint8_t value) noexcept
	{
		if (static_cast<unsigned>(x) < static_cast<unsigned>(Replacer::BufferSize)
			&& static_cast<unsigned>(y) < static_cast<unsigned>(Replacer::BufferSize))
		{
			Replacer::VoxelPixelBuffer[y][x] = value;
		}
	}

	// Vanilla reads the shadow source surface with both coordinates masked to
	// 16 bits (and edi,0FFFFh / and ebp,0FFFFh @ 0x756A2F, 0x756A35), then
	// increments X for the second sample WITHOUT re-masking.
	// SUSPECT: preserved verbatim. A negative ShadowPoint therefore wraps to a
	// large positive and Get_Pixel rejects it, which vanilla treats as "empty".
	inline int SampleSurface(Surface* pSurface, int x, int y) noexcept
	{
		Point2D point { x, y };
		return pSurface->Get_Pixel(point);
	}
}

// ---------------------------------------------------------------------------
// The backport itself.
// ---------------------------------------------------------------------------
// __fastcall + an unused second parameter is the standard way to express
// __thiscall here: `this` arrives in ECX, the two remaining arguments stay on
// the stack, and the callee cleans them - i.e. `ret 8`, exactly like vanilla.
static void __fastcall VoxelLibrary_Draw_Shadow(
	VoxelShadow::Library* pThis,
	void* /* unused, occupies the EDX slot */,
	VoxelShadow::DrawStruct* pDraw,
	Vector3D<float>* pPos)
{
	const VoxelShadow::LayerHeader* pHeaders = pThis->LayerHeaders;
	VoxelShadow::LayerInfo* pInfos = pThis->LayerInfos;

	const int infoIndex = pDraw->InfoIndex + pHeaders[pDraw->HeaderIndex].Index;
	const VoxelShadow::LayerInfo& info = pInfos[infoIndex];

	// Both are unsigned bytes in vanilla, hence the jbe guards at 0x7569C3 and
	// 0x7569EC - they only ever mean "== 0".
	const int sizeX = info.SizeX;
	const int sizeY = info.SizeY;
	const int* const pColumnOffsets = info.ColumnOffsets;

	// flt_7F695C == 128.0, flt_7E2224 == 256.0.
	// DIFF: vanilla truncated these to int16 (mov [esp+..], ax). Kept full width.
	int rowX = F2I((static_cast<double>(pDraw->V2X) + 128.0 - pPos->X) * 256.0);
	int rowY = F2I((static_cast<double>(pDraw->V2Y) + 128.0 - pPos->Y) * 256.0);

	// Vanilla writes these back into the struct before using them (0x7568FF,
	// 0x756910). Side effect preserved.
	pDraw->ShadowPointX = F2I(static_cast<double>(pDraw->ShadowPointX) + pPos->X);
	pDraw->ShadowPointY = F2I(static_cast<double>(pDraw->ShadowPointY) + pPos->Y);

	const int shadowBaseX = pDraw->ShadowPointX;
	const int shadowBaseY = pDraw->ShadowPointY;

	const double fSizeX = static_cast<double>(sizeX);
	const double fSizeY = static_cast<double>(sizeY);

	// BUG (vanilla, preserved as harmless): the inner loop reloads these as DWORDs
	// from overlapping stack slots (mov esi,[esp+anonymous_2] @ 0x756A90), so the
	// upper 16 bits are neighbouring variables / uninitialised stack. Vanilla masks
	// them off at every use, so the garbage never mattered. Here the values are
	// genuine 32-bit quantities, which is the whole point of the port.
	// DIFF: vanilla truncated all four to int16.
	const int stepColX = F2I((static_cast<double>(pDraw->V1X) - pDraw->V2X) / fSizeX * 256.0);
	const int stepColY = F2I((static_cast<double>(pDraw->V1Y) - pDraw->V2Y) / fSizeX * 256.0);
	const int stepRowX = F2I((static_cast<double>(pDraw->V3X) - pDraw->V2X) / fSizeY * 256.0);
	const int stepRowY = F2I((static_cast<double>(pDraw->V3Y) - pDraw->V2Y) / fSizeY * 256.0);

	Surface* const pSurface = pDraw->SurfacePtr;

	int rowColumnBase = 0;

	for (int y = 0; y < sizeY; ++y)
	{
		int accX = rowX;
		int accY = rowY;
		int columnIndex = rowColumnBase;

		for (int x = 0; x < sizeX; ++x)
		{
			// -1 == empty column in this layer's XY footprint.
			if (pColumnOffsets[columnIndex] != -1)
			{
				const int pixelX = ToWhole(accX);
				const int pixelY = ToWhole(accY);

				if (pSurface != nullptr)
				{
					// Branch at 0x7569F6: sample the source surface and write
					// "true where the source pixel is empty" (setz cl / setz dl).
					const int sampleX = (shadowBaseX + pixelX) & 0xFFFF;
					const int sampleY = (shadowBaseY + pixelY) & 0xFFFF;

					const int left  = SampleSurface(pSurface, sampleX, sampleY);
					const int right = SampleSurface(pSurface, sampleX + 1, sampleY);

					PutShadowPixel(pixelX,     pixelY, left  == 0 ? 1 : 0);
					PutShadowPixel(pixelX + 1, pixelY, right == 0 ? 1 : 0);
				}
				else
				{
					// Branch at 0x756B29: no source surface, mark both unconditionally.
					PutShadowPixel(pixelX,     pixelY, 1);
					PutShadowPixel(pixelX + 1, pixelY, 1);
				}
			}

			accX += stepColX;
			accY += stepColY;
			++columnIndex;
		}

		rowX += stepRowX;
		rowY += stepRowY;
		rowColumnBase += sizeX;
	}
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
//
// Vanilla is __thiscall with two stack arguments and `retn 8`. The __fastcall
// declaration above produces a byte-compatible frame and cleanup, so the jump
// target inherits ESP -> return address, arg1 at [ESP+4], arg2 at [ESP+8], and
// returns correctly.
//
// Instruction alignment does NOT matter here (unlike ASMJIT_PATCH): the 5 bytes
// LJMP overwrites are never executed again, and nothing jumps into the middle of
// the function from outside.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x756860, VoxelLibrary_Draw_Shadow)
