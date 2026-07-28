#pragma once

#include <YRPP.h>
#include <Helpers/Macro.h>

#include <cstdint>
#include <cstddef>

// ============================================================================
// VOXEL PIXEL BUFFER REPLACEMENT
//
// THE CORE PROBLEM WITH BufferSize = 512
// --------------------------------------
// Setting VoxelSurface->Width = 512 changes the pitch that EVERY CONSUMER of the
// surface uses (blits, Lock, Get_Buffer arithmetic). It does NOT change the pitch
// the rasterizer uses.
//
// The rasterizers (Asm_Voxel_Normals_Function_Old_*, ~0x756A00-0x758600 and
// ~0x7DF800-0x7E0000) never multiply. They merge bit fields of a packed
// 8.8 / 8.8 fixed point accumulator:
//
//     acc bits  0.. 7 = X fraction
//         bits  8..15 = X integer
//         bits 16..23 = Y fraction
//         bits 24..31 = Y integer
//
//     v16 = HIWORD(acc);            // 0x00YYyy
//     LOBYTE(v16) = BYTE1(acc);     // 0x00YYXX
//     VoxelPixelBuffer[0][v16] = c; // == buffer[Yint * 256 + Xint]
//
// VoxelDistanceLut entries are packed identically and are added straight onto the
// accumulator, so the stride AND the 0..255 coordinate range are structural.
//
// Net effect of BufferSize = 512:
//   - rasterizer writes rows 256 bytes apart
//   - surface / blit reads rows 512 bytes apart
//   => the voxel lands in the top-left quarter, every second engine-row folded
//      into the same physical row. Sheared / half-width / stale garbage.
//
// This is why the change "seems not to work properly". It is not a missing patch
// site; the two halves of the pipeline now disagree about pitch.
// ============================================================================

struct Replacer
{
	// --- structural, DO NOT change without porting the rasterizers -----------
	// Row stride the vanilla rasterizer bit-packing produces.
	static constexpr int RasterStride = 256;
	// Max addressable coordinate in the packed accumulator (8 bits per axis).
	static constexpr int RasterMaxCoord = 255;

	// --- buffer geometry -----------------------------------------------------
	// EXTENSION: intended to become > 256 once the rasterizers are ported.
	static constexpr int BufferSize = 256;

	static_assert(BufferSize == RasterStride,
		"BufferSize must equal RasterStride. The rasterizer computes its write "
		"offset as (Yint << 8) | Xint, which cannot express any other pitch. "
		"Port Asm_Voxel_Normals_Function_Old_* to explicit y * Stride + x first.");

	static constexpr std::size_t BufferBytes =
		static_cast<std::size_t>(BufferSize) * static_cast<std::size_t>(BufferSize);

	// uint8_t, not char: the clear loops emit dword stores and every consumer
	// treats these as palette indices, never as signed values.
	// alignas(64) so the 4 * (count >> 2) memset path stays aligned.
	alignas(64) static std::uint8_t VoxelPixelBuffer[BufferSize][BufferSize];

	// =======================================================================
	// ADDRESS BUG IN THE ORIGINAL HEADER - 0xB2D7F0 IS THE WRONG SURFACE
	//
	// Your version had:
	//     static constexpr reference<BSurface, 0xB2D7F0> VoxelSurface {};
	//
	// 0xB2D7F0 is VoxelBufferedSurface - the DEPTH buffer's surface. From .data:
	//
	//   .data:00B2D7F0  VoxelBufferedSurface  BSurface <<<vftable,100h,100h>,0,1>,
	//                                                   <0B1D5E0h, 10000h, 0>>
	//   .data:00B2D928  VoxelSurface          BSurface <<<vftable,100h,100h>,0,1>,
	//                                                   <0B2FF78h, 10000h, 0>>
	//
	// i.e. 0xB2D7F0 wraps VoxelBufferedPixelBuffer (0xB1D5E0) and 0xB2D928 wraps
	// VoxelPixelBuffer (0xB2FF78).
	//
	// So _Apply() was pointing the DEPTH surface at the new colour buffer and
	// resizing it to 512x512, while the real colour surface at 0xB2D928 kept its
	// vanilla 256x256 geometry and its vanilla buffer pointer. That alone is
	// enough to produce garbage, independently of the stride problem.
	//
	// BSurface is 32 bytes: { vftable, Width, Height, ?, BytesPerPixel } followed
	// by MemoryBuffer { Pointer, Size, Allocated }. Confirmed by
	// 0xB2D810 - 0xB2D7F0 == 0x20.
	// =======================================================================

	static constexpr reference<RectangleStruct, 0xB2FB60> VoxelClippingRect {};

	// Colour surface, wraps VoxelPixelBuffer (0xB2FF78).
	static constexpr reference<BSurface, 0xB2D928> VoxelSurface {};

	// Depth surface, wraps VoxelBufferedPixelBuffer (0xB1D5E0).
	// MISSING: not resized or repointed anywhere yet. It has its own initialiser,
	// Init_spec_VoxelSurfaceBuffer2, so replacing the depth buffer means giving
	// this the same Width / Height / BufferPtr treatment - not just a pointer swap.
	static constexpr reference<BSurface, 0xB2D7F0> VoxelBufferedSurface {};

	// VERIFY: this is still the vanilla 256x256 array in the game's .data.
	// Voxel_Init_Surface_Stuff clears it with the SAME clip rect whose bounds you
	// patched at 0x753E5F / 0x753E6F / 0x7547D8 / 0x7547E4. If those bounds ever
	// exceed 255 while this stays 256 wide, that clear loop writes out of bounds.
	// MISSING: replacement for VoxelBufferedPixelBuffer is not implemented.
	// static constexpr reference<...> VoxelBufferedPixelBuffer { 0x??????? };

	static void __fastcall _Apply();
	static void __cdecl _De_Apply();
	// ORDERING: _Apply() must run AFTER the vanilla static initializer
	// init_VoxelShadowSurface (~0x7539C0), which sets Width / Height / BufferPtr
	// itself. That initializer runs during the game's CRT init, which happens
	// after Syringe has loaded this DLL. Calling _Apply() at DLL attach time gets
	// silently overwritten.
	// VERIFY: hook _Apply() at the end of init_VoxelShadowSurface, or at a known
	// post-CRT point, not from DllMain.
};

// ============================================================================
// SURFACE INITIALISERS - RESOLVED
//
// Both initialisers are now fully disassembled, and the earlier VERIFY about a
// "missing Height immediate" was a false alarm. There is only ONE dimension
// immediate, loaded into EAX and stored to both fields:
//
//   .text:007539D0  B8 00 01 00 00   mov  eax, 100h                 <- 0x7539D1
//   .text:007539D5  68 00 00 01 00   push 10000h            ; size  <- 0x7539D6
//   .text:007539DA  68 78 FF B2 00   push offset VoxelPixelBuffer   <- 0x7539DB
//   .text:007539DF  B9 3C D9 B2 00   mov  ecx, offset VoxelSurface.buff
//   .text:007539E4  A3 2C D9 B2 00   mov  VoxelSurface.Width,  eax
//   .text:007539E9  A3 30 D9 B2 00   mov  VoxelSurface.Height, eax
//
// So your three existing patches are CORRECT and COMPLETE:
//
//   DEFINE_PATCH_TYPED(DWORD, 0x7539D1, Replacer::BufferSize)                  // Width AND Height
//   DEFINE_PATCH_TYPED(DWORD, 0x7539D6, sizeof(Replacer::VoxelPixelBuffer))    // Buffer size
//   DEFINE_PATCH_TYPED(DWORD, 0x7539DB, DWORD(&Replacer::VoxelPixelBuffer))    // Buffer pointer
//
// RECOMMENDATION: delete _Apply() entirely. It duplicates all three of these,
// it targeted the wrong surface, and it has an initialisation-order hazard the
// static patches do not.
//
// Confirmed BSurface layout from the store addresses (base 0xB2D928):
//   +0x00 vftable   +0x04 Width   +0x08 Height   +0x0C LockLevel
//   +0x10 BytesPerPixel          +0x14 Buffer { Pointer, Size, Allocated }
//   sizeof == 0x20
// VERIFY that YRpp's BSurface names map onto those offsets before using
// ->Width / ->Height / ->BufferPtr anywhere.
//
// ----------------------------------------------------------------------------
// DEPTH BUFFER RELOCATION - RECIPE (NOT YET APPLIED)
//
// Init_spec_VoxelSurfaceBuffer2 @ 0x753A50 has exactly the same shape:
//
//   .text:00753A50  B8 00 01 00 00   mov  eax, 100h                      <- 0x753A51
//   .text:00753A55  68 00 00 01 00   push 10000h                 ; size  <- 0x753A56
//   .text:00753A5A  68 E0 D5 B1 00   push offset VoxelBufferedPixelBuffer <- 0x753A5B
//
// so relocating the depth buffer needs, at minimum:
//
//   alignas(64) static std::uint8_t VoxelDepthBuffer[BufferSize][BufferSize];
//
//   DEFINE_PATCH_TYPED(DWORD, 0x753A51, Replacer::BufferSize)
//   DEFINE_PATCH_TYPED(DWORD, 0x753A56, sizeof(Replacer::VoxelDepthBuffer))
//   DEFINE_PATCH_TYPED(DWORD, 0x753A5B, DWORD(&Replacer::VoxelDepthBuffer))
//
// plus every 0xB1D5E0 / 0xB1D5E1 displacement in the rasterizers and in the
// clear paths (Voxel_Init_Surface_Stuff, Voxel_Clear_Voxel_Surface_spec_Buffer_2,
// Voxel_Conditional_Clear_spec_VoxelBufferedPixelBuffer and its _0 sibling).
//
// DO NOT APPLY YET. Several asm rasterizers still index the depth buffer with the
// hardcoded (Yint << 8) | Xint packing, so moving it while BufferSize is 256
// gains nothing and moving it when BufferSize is larger desynchronises those.
// ============================================================================
