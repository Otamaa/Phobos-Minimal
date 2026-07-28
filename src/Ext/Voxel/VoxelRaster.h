#pragma once

// ===========================================================================
// Shared plumbing for the stride-aware voxel rasterizer backports.
//
// Every vanilla voxel rasterizer computes its write offset the same way:
//
//     ebp = accX & 0xFFFF ; ebp >>= 8      -> Xint  (0..255)
//     ebx = accY & 0xFF00                  -> Yint << 8
//     ebx |= ebp                           -> idx = Yint * 256 + Xint
//
// and holds every coordinate and step as int16 in 8.8 fixed point, so the whole
// projection lives in a 256x256 space that WRAPS instead of clipping.
//
// This header replaces both halves: int32 accumulators (same 8 fraction bits,
// no 16-bit truncation) and an explicit y * Stride + x with a real bounds check.
// ===========================================================================

#include "VoxelBufferReplace.h"

#include <YRPP.h>

#include <array>
#include <cstdint>

namespace VoxelRaster
{
	// -----------------------------------------------------------------------
	// 8.8 fixed point. The *256.0 the setup code applies before __ftol is what
	// produces the 8 fraction bits; we keep that scale and only widen the type.
	// -----------------------------------------------------------------------
	inline constexpr int FixedShift = 8;

	// SUSPECT: vanilla masks rather than shifts, so it never observes a negative
	// coordinate - it wraps into the buffer instead. An arithmetic shift keeps the
	// sign, and the bounds check below then rejects it. That is the intended DIFF.
	inline constexpr int ToWhole(int fixedValue) noexcept
	{
		return fixedValue >> FixedShift;
	}

	// EXTENSION: explicit clip. Vanilla had none; the 8-bit masks made every
	// out-of-range coordinate wrap silently into the buffer, which is the
	// stripe/garbage artefact seen on oversized voxels.
	inline void PutPixel(int x, int y, std::uint8_t value) noexcept
	{
		if (static_cast<unsigned>(x) < static_cast<unsigned>(Replacer::BufferSize)
			&& static_cast<unsigned>(y) < static_cast<unsigned>(Replacer::BufferSize))
		{
			Replacer::VoxelPixelBuffer[y][x] = value;
		}
	}

	// -----------------------------------------------------------------------
	// Per-component 8.8 vector. VERIFY: 6 bytes, confirmed by the field offsets
	// 0x18/0x1A, 0x1E/0x20, 0x24/0x26, 0x2A/0x2C in the rasterizer disassembly.
	// The Z component is never read by the 2D rasterizers.
	// -----------------------------------------------------------------------
	struct FixedVector
	{
		std::int16_t X;
		std::int16_t Y;
		std::int16_t Z;
	};
	static_assert(sizeof(FixedVector) == 6, "FixedVector must be 3 x int16.");

	// -----------------------------------------------------------------------
	// The struct the Voxel_Draw_Function_Tbl entries receive.
	// VERIFY every offset; these are derived from addressing modes only.
	// -----------------------------------------------------------------------
	struct DrawStruct
	{
		// TWO separate column-offset tables, both indexed [DataPos], -1 == empty.
		// The "startptr" draw variants read +0x00, the "endptr" variants read
		// +0x04. Confirmed: fn 16 @ 0x756E62 does `mov eax,[ecx]`, fn 17 @
		// 0x75700C does `mov eax,[ecx+4]`.
		const int*    ColumnOffsetsStart;  // +0x00  offset to the FIRST byte of a span
		const int*    ColumnOffsetsEnd;    // +0x04  offset to the LAST  byte of a span
		std::uint8_t* SpanData;        // +0x08  mov eax,[ecx+8] ; add eax, edx
		int           DataPos;         // +0x0C  read AND written back in place
		int           XSteps;          // +0x10  column advance for DataPos
		int           YSteps;          // +0x14  row advance for DataPos
		FixedVector   Start;           // +0x18
		FixedVector   AxisX;           // +0x1E  per-column screen step
		FixedVector   AxisY;           // +0x24  per-row screen step
		FixedVector   AxisZ;           // +0x2A  per-Z-voxel screen step
		std::uint8_t  SizeX;           // +0x30
		std::uint8_t  SizeY;           // +0x31
		std::uint8_t  SizeZ;           // +0x32
	};
	static_assert(offsetof(DrawStruct, ColumnOffsetsEnd) == 0x04, "EndPtr @ +0x04");
	static_assert(offsetof(DrawStruct, SpanData) == 0x08, "SpanData @ +0x08");
	static_assert(offsetof(DrawStruct, DataPos)  == 0x0C, "DataPos @ +0x0C");
	static_assert(offsetof(DrawStruct, Start)    == 0x18, "Start @ +0x18");
	static_assert(offsetof(DrawStruct, AxisX)    == 0x1E, "AxisX @ +0x1E");
	static_assert(offsetof(DrawStruct, AxisY)    == 0x24, "AxisY @ +0x24");
	static_assert(offsetof(DrawStruct, AxisZ)    == 0x2A, "AxisZ @ +0x2A");
	static_assert(offsetof(DrawStruct, SizeX)    == 0x30, "SizeX @ +0x30");
	static_assert(offsetof(DrawStruct, SizeZ)    == 0x32, "SizeZ @ +0x32");

	// -----------------------------------------------------------------------
	// DEPTH BUFFER  (vanilla symbol: VoxelBufferedPixelBuffer)
	//
	// VERIFY: 0xB1D5E0, one BYTE per pixel, same geometry as VoxelPixelBuffer.
	// Confirmed from fn 18:
	//   .text:0075726E  66 0F B6 98 [E0 D5 B1 00]  movzx bx, VoxelBufferedPixelBuffer[eax]
	//   .text:00757282  88 90       [E0 D5 B1 00]  mov   VoxelBufferedPixelBuffer[eax], dl
	//   .text:00757294  88 90       [E1 D5 B1 00]  mov   (VoxelBufferedPixelBuffer+1)[eax], dl
	//
	// This is the buffer VoxelBufferReplace.cpp never replaced. It is still the
	// vanilla 256x256 array, and it is indexed with the SAME (Yint << 8) | Xint
	// packing, so it has to move in lockstep with VoxelPixelBuffer.
	//
	// While BufferSize == 256 we simply alias the vanilla array, which keeps the
	// ported and unported rasterizers agreeing. The assert below is what forces
	// you to deal with it before raising BufferSize.
	//
	// CONFIRMED LAYOUT - overflowing either buffer lands on live data:
	//
	//   0xB1D5E0  VoxelBufferedPixelBuffer  10000h bytes
	//   0xB2D5E0  VoxelClippingMax          <- immediately after the depth buffer
	//   0xB2D7F0  VoxelBufferedSurface      (BSurface wrapping 0xB1D5E0)
	//   0xB2D928  VoxelSurface              (BSurface wrapping 0xB2FF78)
	//   0xB2D958  VoxelQueue                64 x VoxelDrawStruct
	//   0xB2FB60  VoxelClippingRect         initialised to <0, 0, 255, 255>
	//   0xB2FF78  VoxelPixelBuffer          10000h bytes
	//   0xB3FF78  VoxelShadowQueue          <- immediately after the colour buffer
	//
	// So the out-of-bounds clear I flagged earlier - raising the clip-rect bounds
	// past 255 while the depth buffer is still 256 wide - writes straight into
	// VoxelClippingMax, VoxelClippingMin, both surfaces and the voxel queue.
	// -----------------------------------------------------------------------
	static_assert(Replacer::BufferSize == 256,
		"VoxelBufferedPixelBuffer (0xB1D5E0) is still the vanilla 256x256 array. "
		"Replace it before raising BufferSize, or the depth test reads garbage.");

	inline std::uint8_t (&DepthBuffer)[256][256] =
		*reinterpret_cast<std::uint8_t (*)[256][256]>(0xB1D5E0);

	// Depth-tested paired write, as used by the "spec buffer" variants.
	//
	// BUG (vanilla, preserved): the depth test is performed ONCE, against the LEFT
	// pixel only, and then BOTH pixels are written. A voxel can therefore overwrite
	// a nearer neighbour at x+1. See 0x757279 (single cmp) vs 0x757282..0x757299
	// (four stores).
	//
	// BUGFIX: x and x+1 are bounds-checked independently, so the vanilla row-bleed
	// at Xint == 255 - where (buffer+1)[idx] landed in column 0 of the NEXT row -
	// no longer happens.
	inline void PutPixelDepthPair(int x, int y, std::uint8_t colour,
		std::uint8_t depth) noexcept
	{
		if (static_cast<unsigned>(x) >= static_cast<unsigned>(Replacer::BufferSize)
			|| static_cast<unsigned>(y) >= static_cast<unsigned>(Replacer::BufferSize))
		{
			return;
		}

		// jbe at 0x75727C: strictly greater wins, ties lose.
		if (depth <= DepthBuffer[y][x])
		{
			return;
		}

		DepthBuffer[y][x] = depth;
		Replacer::VoxelPixelBuffer[y][x] = colour;

		if (static_cast<unsigned>(x + 1) < static_cast<unsigned>(Replacer::BufferSize))
		{
			Replacer::VoxelPixelBuffer[y][x + 1] = colour;
			DepthBuffer[y][x + 1] = depth;
		}
	}

	// Depth test alone, for the variants that only fetch the colour/normal bytes
	// after the test passes (fn 22 and friends). Returns false when off-buffer.
	inline bool DepthTest(int x, int y, std::uint8_t depth) noexcept
	{
		if (static_cast<unsigned>(x) >= static_cast<unsigned>(Replacer::BufferSize)
			|| static_cast<unsigned>(y) >= static_cast<unsigned>(Replacer::BufferSize))
		{
			return false;
		}

		// jbe: strictly greater wins, ties lose.
		return depth > DepthBuffer[y][x];
	}

	// Paired depth write. Same independent bounds checks as PutPixelPair.
	inline void PutDepthPair(int x, int y, std::uint8_t depth) noexcept
	{
		if (static_cast<unsigned>(x) < static_cast<unsigned>(Replacer::BufferSize)
			&& static_cast<unsigned>(y) < static_cast<unsigned>(Replacer::BufferSize))
		{
			DepthBuffer[y][x] = depth;
		}

		if (static_cast<unsigned>(x + 1) < static_cast<unsigned>(Replacer::BufferSize)
			&& static_cast<unsigned>(y) < static_cast<unsigned>(Replacer::BufferSize))
		{
			DepthBuffer[y][x + 1] = depth;
		}
	}

	// Paired write where the SECOND pixel is at (x | 1) rather than (x + 1).
	//
	// Vanilla folds the "+1" into the index register instead of the displacement:
	//
	//   .text:00757F6E  and  edi, 0FF00h     ; edi = Yint << 8
	//   .text:00757F7A  or   ebx, edi        ; ebx = (Yint << 8) | Xint
	//   .text:00757F7C  inc  edi             ; edi = (Yint << 8) | 1
	//   .text:00757F7D  or   edi, eax        ; edi = (Yint << 8) | (Xint | 1)
	//   .text:00757F7F  mov  VoxelPixelBuffer[ebx], dl
	//   .text:00757F85  mov  VoxelPixelBuffer[edi], dl
	//
	// so BOTH instructions carry the plain base displacement with no `+ 1`. That
	// is why 0x757F81 / 0x757F87 legitimately differ from every other pair in the
	// DEFINE_PATCH_TYPED list - they are correct as written.
	//
	// Behaviourally this is NOT the same as (x + 1):
	//   even x -> writes x and x+1   (a real pair)
	//   odd  x -> writes x twice     (a single pixel)
	//
	// Side effect: because Xint stays within 0..255, this form can never carry
	// into the next row, so the variants using it are free of the row-bleed bug
	// that (buffer+1)[idx] has in the other rasterizers.
	//
	// SUSPECT: preserved verbatim. Do not "normalise" this to x + 1 - the odd-x
	// single-pixel case is observable.
	inline void PutPixelPairSetLsb(int x, int y, std::uint8_t colour) noexcept
	{
		PutPixel(x, y, colour);
		PutPixel(x | 1, y, colour);
	}

	// Vanilla reads the depth accumulator as `mov dx, word[..] ; shr dx, 8`, i.e.
	// a LOGICAL shift on the low 16 bits only. A negative Z therefore wraps to a
	// large positive depth rather than clamping. Preserved exactly.
	//
	// SUSPECT: depth stays 8-bit because the buffer is one byte per pixel and the
	// unported rasterizers share it. Only X and Y are widened by this port; deep
	// voxels can still z-fight. Revisit once the whole family is ported.
	inline constexpr std::uint8_t ToDepth(int fixedZ) noexcept
	{
		return static_cast<std::uint8_t>((static_cast<unsigned>(fixedZ) >> 8) & 0xFF);
	}

	// -----------------------------------------------------------------------
	// VOXEL LIGHTING TABLES  (used by the "normal" draw variants)
	//
	// Per voxel the shaded colour is
	//
	//     shade = VPLLookup[ VoxelNormalToLut[normal] * 256 + colour ]
	//
	// straight from fn 20:
	//
	//   .text:007576B9  8A 8A    [90 59 B4 00]  mov cl, VoxelNormalToLut[edx]
	//   .text:007576D8  mov edx, [a1] ; and edx, 0FFh ; shl edx, 8
	//   .text:007576E5  8A 94 1A [78 11 B4 00]  mov dl, VPLLookup[edx+ebx]
	//
	// VERIFY: VoxelNormalToLut = 0xB45990, VPLLookup = 0xB41178.
	//
	// NOTE the VPLLookup encoding - ModRM 0x94 has rm == 100b, so there IS a SIB
	// byte (0x1A) and the disp32 starts at instruction + 3, not + 2. This is the
	// exact case to watch for when auditing the DEFINE_PATCH_TYPED list.
	//
	// Useful confirmation from the address gaps: 0xB45990 - 0xB45590 == 0x400,
	// i.e. VoxelDistanceLut really is 256 entries of 4 bytes and is fully
	// allocated, so the index-255 read noted below stays in bounds.
	// -----------------------------------------------------------------------

	// VERIFY: 0xB45990. Maps a raw .vxl normal index to a VPL section index.
	inline const std::uint8_t* const NormalToLut =
		reinterpret_cast<const std::uint8_t*>(0xB45990);

	// VERIFY: 0xB41178. 32 sections of 256 entries = 8192 bytes.
	//
	// The .data dump settles the earlier SUSPECT note:
	//   .data:00B2FB58  VPL_SectionCount  dd 20h        ; 32
	//   .data:00B41178  VPLLookup         ... 0x2000 bytes, ends at 0xB43178
	//
	// 0x2000 == 32 * 256, so the table is 32 x 256 and the indexing
	// `VPLLookup[section * 256 + colour]` is section-major.
	//
	// NOTE the IDB declares it `unsigned __int8 VPLLookup[256][32]` - the
	// dimensions are the wrong way round. It is [32][256].
	//
	// BUG (vanilla, preserved): nothing bounds-checks the section index. If
	// NormalToLut[normal] ever exceeds 31 the read runs past the end into
	// VPL_Unused / VPL_RemapEnd / VoxelUseBuffer_NighthawkNeedsThis. The port
	// reproduces this rather than clamping.
	inline constexpr int VplSectionCount = 32;
	inline constexpr int VplSectionSize = 256;

	inline const std::uint8_t* const VplLookup =
		reinterpret_cast<const std::uint8_t*>(0xB41178);

	// Runtime section count, in case a mod's .vpl differs from the default 32.
	// VERIFY: 0xB2FB58, written by Init_Voxel_Palette+44.
	inline const int& VplSectionCountLive =
		*reinterpret_cast<const int*>(0xB2FB58);

	inline std::uint8_t Shade(std::uint8_t colour, std::uint8_t normal) noexcept
	{
		return VplLookup[(NormalToLut[normal] << 8) + colour];
	}

	// Undepth-tested paired write, as used by the "normal" variants.
	//
	// BUGFIX: x and x+1 are bounds-checked independently, so the vanilla row-bleed
	// at Xint == 255 - where (VoxelPixelBuffer+1)[idx] landed in column 0 of the
	// NEXT row - no longer happens.
	inline void PutPixelPair(int x, int y, std::uint8_t colour) noexcept
	{
		PutPixel(x, y, colour);
		PutPixel(x + 1, y, colour);
	}

	// -----------------------------------------------------------------------
	// SPAN DATA BLOCK LAYOUT
	//
	// Reconstructed from fn 16 (forward walk) and confirmed against fn 17
	// (backward walk). One column is a chain of blocks:
	//
	//     +0                  skipCount   (voxels to skip along Z)
	//     +1                  runCount    (voxels present)
	//     +2 .. +2+2*run      { colour, normal } pairs
	//     +2+2*run            runCount    (duplicate, block terminator)
	//
	// Block size = 3 + 2 * runCount. The chain ends when the running
	// zRemaining counter reaches zero, NOT on a sentinel byte.
	//
	// A "startptr" variant enters at +0 and walks up; an "endptr" variant enters
	// at the duplicate terminator and walks down, which reverses the order of the
	// skip and run handling within each block.
	// -----------------------------------------------------------------------

	// -----------------------------------------------------------------------
	// Z-skip distance lookup.
	//
	// Vanilla lives at 0xB45590 as 255 entries of { int16 dx, int16 dy }, and
	// EVERY rasterizer rebuilds it at its own entry point. Because the entries
	// are int16 and hold an accumulated step, they overflow on a tall voxel with
	// a large AxisZ - a second, separate 16-bit limit.
	//
	// EXTENSION: we keep an int32 mirror for the ported rasterizers.
	// The vanilla table is rebuilt alongside it so any UNPORTED rasterizer that
	// runs afterwards still sees the values it expects.
	// -----------------------------------------------------------------------
	struct VanillaLutEntry
	{
		std::int16_t X;
		std::int16_t Y;
	};
	static_assert(sizeof(VanillaLutEntry) == 4, "Vanilla LUT stride is 4 bytes.");

	struct LutEntry
	{
		int X;
		int Y;
	};

	// VERIFY: 0xB45590, from `mov bx, VoxelDistanceLut[edx*4]` @ 0x756E9B
	// (66 8B 1C 95 [90 55 B4 00]).
	// SUSPECT: only 255 entries are ever written, but the skip byte pulled from
	// the span data is a full uint8 (0..255), so index 255 is reachable and reads
	// one past the written range. Sized 256 here to keep that read in bounds.
	inline constexpr int LutSize = 256;

	extern std::array<LutEntry, LutSize> DistanceLut;

	// Rebuilds both tables. `count` is SizeZ; entry 0 is always zero and entries
	// >= count keep whatever the previous voxel left there, exactly as in vanilla.
	void BuildDistanceLut(const FixedVector& axisZ, int count) noexcept;
}
