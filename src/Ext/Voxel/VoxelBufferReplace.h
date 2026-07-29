#pragma once

#include <YRPP.h>
#include <Utilities/Macro.h>

#include <cstdint>
#include <cstddef>

// ============================================================================
// VOXEL PIXEL BUFFER REPLACEMENT
//
// STATUS: the 256x256 limit is gone. All 21 rasterizers, all four clear helpers,
// both surface initialisers and the buffer-diagonal initialiser are replaced by
// C++ that computes `y * BufferSize + x` explicitly. Nothing in the pipeline
// still uses the vanilla `(Yint << 8) | Xint` packing.
//
// The original problem, for reference: every vanilla rasterizer built its write
// offset by merging bit fields of a packed 8.8/8.8 accumulator -
//
//     v16 = HIWORD(acc);            // 0x00YYyy
//     LOBYTE(v16) = BYTE1(acc);     // 0x00YYXX
//     VoxelPixelBuffer[0][v16] = c; // == buffer[Yint * 256 + Xint]
//
// so the stride AND the 0..255 coordinate range were structural, not immediates.
// Raising BufferSize alone changed only the readers, never the writers.
// ============================================================================
// ============================================================================

struct Replacer
{
	// --- buffer geometry -----------------------------------------------------
	// FREE NOW. RasterStride / RasterMaxCoord and the `BufferSize == 256` assert
	// are gone: every rasterizer, clear helper and initialiser is ported, so the
	// vanilla (Yint << 8) | Xint packing no longer exists in the pipeline.
	static constexpr int BufferSize = 1024;

	static constexpr std::size_t BufferBytes =
		static_cast<std::size_t>(BufferSize) * static_cast<std::size_t>(BufferSize);

	// Where world origin lands inside the buffer, in whole pixels.
	//
	// This has to be changed in TWO places at once or the voxels disappear:
	//
	//   1. THE PROJECTION - VoxelLibraryClass::Draw computes
	//        Start = (world + 128.0 - camera) * 256.0
	//      so the integer pixel is world + 128 - camera.
	//
	//   2. THE CLIP RECT - Voxel_Sort_Calc_And_Draw_Clipped_0/_1 compute
	//        VoxelClippingRect.X = 128 - extentX/2 - 4
	//        VoxelClippingRect.Y = 128 - extentY/2 - 4
	//      and that rect is the source region the blit copies out of the surface.
	//
	// Moving only the projection puts the voxel at pixel 256 while the blit still
	// reads the box around pixel 128 - a blank screen with no crash. That was the
	// second disappearance.
	//
	// The * 256.0 sitting next to the 128.0 in the projection is the 8.8 FIXED
	// POINT SCALE, not a dimension. It must NOT change, ever.
	static constexpr int   BufferCenterInt = BufferSize / 2;
	static constexpr float BufferCenter    = static_cast<float>(BufferCenterInt);

	// Storage for the above, so its address can be patched into the two `fadd`
	// operands in VoxelLibraryClass::Draw. X and Y only - Z stays at 128.
	static const float BufferCenterXY;

	// -----------------------------------------------------------------------
	// OWNED BUFFERS.
	//
	// Every consumer is now ported, so there are NO surviving
	// DEFINE_PATCH_TYPED entries for either buffer's address. The only patches
	// left in the .cpp are the two `fadd` operands that recentre the projection.
	//
	// uint8_t, not char: the clear paths emit dword stores and every consumer
	// treats these as palette indices, never as signed values.
	// alignas(64) so the memset paths stay aligned.
	// -----------------------------------------------------------------------

	// The PHOBOS_VOXEL_ALIAS_VANILLA_BUFFER debug switch has been removed. It
	// aliased 0xB2FF78, which is only 64 KB - at BufferSize 512 that would have
	// been an out-of-bounds alias, i.e. dangerous rather than diagnostic. Its
	// purpose was to separate "ports broken" from "redirect incomplete", and no
	// patch anywhere references either buffer's address any more.
	alignas(64) static std::uint8_t VoxelPixelBuffer[BufferSize][BufferSize];

	// Depth buffer, formerly VoxelBufferedPixelBuffer at 0xB1D5E0. Must always
	// have exactly the same geometry as VoxelPixelBuffer - every spec-buffer
	// rasterizer indexes both with the same x/y.
	//
	// WIDENED TO 16 BITS. Vanilla was ONE BYTE per pixel, read as
	// `mov dx, word[..] ; shr dx, 8` - so 256 distinct depth levels total.
	//
	// A large model projects to a bigger depth extent. Once that extent passes
	// 255 units the depth values wrap, a near fragment ends up with a LOWER
	// stored depth than a far one, the test rejects it, and whole chunks of the
	// model simply do not appear. That is the "missing segments" symptom, and it
	// is independent of how big the colour buffer is - which is why enlarging the
	// buffer never addressed it.
	//
	// Safe to change because every consumer is ported: the six spec-buffer
	// rasterizers, all three clear paths, and _ApplyDepth. Nothing blits the
	// depth surface - 0xB2D7F0's only xrefs are its own init and deinit.
	alignas(64) static std::uint16_t VoxelDepthBuffer[BufferSize][BufferSize];

	// Depth is centred here rather than at 128, so the usable range is
	// +/- 32768 units instead of +/- 128. Clear value 0 still reads as
	// "nothing drawn" because real geometry sits near the centre.
	static constexpr double DepthCenter = 32768.0;

	// -----------------------------------------------------------------------
	// SURFACES. Note which is which - 0xB2D7F0 is the DEPTH surface, not the
	// colour one. Getting these the wrong way round was the first cause of the
	// blank screen. From .data:
	//
	//   0xB2D7F0  VoxelBufferedSurface  <<<vftable,100h,100h>,0,1>, <0B1D5E0h,10000h,0>>
	//   0xB2D928  VoxelSurface          <<<vftable,100h,100h>,0,1>, <0B2FF78h,10000h,0>>
	//
	// BSurface is 32 bytes: { vftable, Width, Height, LockLevel, BytesPerPixel }
	// then MemoryBuffer { Pointer, Size, Allocated }. Confirmed by
	// 0xB2D810 - 0xB2D7F0 == 0x20.
	// -----------------------------------------------------------------------
	static constexpr reference<RectangleStruct, 0xB2FB60> VoxelClippingRect {};
	static constexpr reference<BSurface, 0xB2D928>        VoxelSurface {};
	static constexpr reference<BSurface, 0xB2D7F0>        VoxelBufferedSurface {};

	// -----------------------------------------------------------------------
	// Replacements for the two surface initialisers. Both are DEFINED IN THE
	// .cpp, not here - they write the BSurface fields at raw offsets rather than
	// constructing one, because BSurface(w, h, bpp) ALLOCATES its own buffer and
	// vanilla instead ATTACHES an existing one via Buffer::Buffer(ptr, size).
	// Constructing was the second cause of the blank screen.
	//
	// __fastcall to match the vanilla functions they replace.
	// -----------------------------------------------------------------------
	static void __fastcall _Apply();       // replaces init_VoxelShadowSurface       0x7539D0
	static void __fastcall _ApplyDepth();  // replaces Init_spec_VoxelSurfaceBuffer2 0x753A50

	// No _De_Apply. Both buffers are static arrays, so there is nothing to tear
	// down, and ~BSurface() on a surface whose Allocated flag is 0 would at best
	// do nothing and at worst try to free them.
};

// ============================================================================
// SURFACE INITIALISERS - both replaced, no patches left
//
// Earlier notes here recommended keeping 0x7539D1 / 0x7539D6 / 0x7539DB as
// DEFINE_PATCH_TYPED entries rather than porting init_VoxelShadowSurface. That
// advice is superseded: _Apply / _ApplyDepth replace both initialisers outright,
// so all six geometry immediates are gone.
//
//   .text:007539D0  B8 00 01 00 00   mov  eax, 100h        <- fed Width AND Height
//   .text:007539D5  68 00 00 01 00   push 10000h           <- buffer size
//   .text:007539DA  68 78 FF B2 00   push offset ...       <- buffer pointer
//   .text:00753A50 .. 0x753A5B       the same three for the depth surface
//
// Field offsets used by the replacements, from the absolute stores:
//   colour 0xB2D928   +04 Width  +08 Height  +0C LockLevel  +10 Bpp  +14 Buffer
//   depth  0xB2D7F0   same shape
//   0x7E2070  BSurface vftable   (the one that must be left in place)
//   0x7E2104  XSurface vftable   (vanilla writes it first, then overwrites)
//
// ============================================================================

// ----------------------------------------------------------------------------
// DEPTH BUFFER - DONE.
//
// VoxelBufferedPixelBuffer (0xB1D5E0) is no longer referenced anywhere. Its 18
// rasterizer sites are inside ported functions, the four clear paths are ported,
// and Init_spec_VoxelSurfaceBuffer2 is replaced by _ApplyDepth. The buffer now
// lives in Replacer::VoxelDepthBuffer and tracks BufferSize automatically.
//
// It MUST stay the same geometry as VoxelPixelBuffer - every spec-buffer
// rasterizer indexes both with the same x/y.
// ----------------------------------------------------------------------------
// ============================================================================
