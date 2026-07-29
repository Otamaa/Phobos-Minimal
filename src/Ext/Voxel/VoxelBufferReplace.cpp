#include "VoxelBufferReplace.h"

#include <Utilities/Macro.h>

#include <cstdint>

alignas(64) std::uint8_t Replacer::VoxelPixelBuffer[Replacer::BufferSize][Replacer::BufferSize];
alignas(64) std::uint16_t Replacer::VoxelDepthBuffer[Replacer::BufferSize][Replacer::BufferSize];

// ===========================================================================
// THIS IS WHY THE VOXELS WERE INVISIBLE
//
// The previous version was:
//
//     ::new (VoxelSurface.operator->()) BSurface(BufferSize, BufferSize, 1);
//
// BSurface(width, height, bpp) ALLOCATES ITS OWN BUFFER. Vanilla does not:
//
//     .text:007539DA  push offset VoxelPixelBuffer      ; buffer
//     .text:007539DF  mov  ecx, offset VoxelSurface.buff
//     .text:00753A0C  call Buffer::Buffer(char *, long)
//
// Buffer::Buffer(ptr, size) ATTACHES an existing block. So the ported _Apply()
// left VoxelSurface pointing at a freshly allocated, permanently blank 64 KB
// heap block, while every rasterizer wrote into Replacer::VoxelPixelBuffer.
// The blit read the heap block. Nothing on screen, no crash, no corruption -
// exactly the symptom.
//
// It also leaked 64 KB per call.
//
// The fix below writes the fields directly, mirroring 0x7539D0 instruction for
// instruction but with our buffer. Raw offsets are used deliberately: they come
// straight from the disassembly's absolute stores, so they cannot drift with
// YRpp's BSurface field naming.
//
//   .text:007539E4  mov VoxelSurface.Width,         eax   -> 0xB2D92C
//   .text:007539E9  mov VoxelSurface.Height,        eax   -> 0xB2D930
//   .text:007539EE  mov VoxelSurface.LockLevel,     0     -> 0xB2D934
//   .text:00753A02  mov VoxelSurface.BytesPerPixel, 1     -> 0xB2D938
//   .text:007539DF  mov ecx, offset VoxelSurface.buff     -> 0xB2D93C
//   .text:00753A16  mov VoxelSurface.vftble, BSurface     -> 0xB2D928
//
// and .data:00B2D928 confirms the shape:
//   BSurface <<<vftable, 100h, 100h>, 0, 1>, <0B2FF78h, 10000h, 0>>
// ===========================================================================

void __fastcall Replacer::_Apply()
{
	// VERIFY: BSurface base 0xB2D928, 8 dwords.
	//   +00 vftable  +04 Width  +08 Height  +0C LockLevel  +10 BytesPerPixel
	//   +14 Buffer.Pointer  +18 Buffer.Size  +1C Buffer.Allocated
	auto* const surface = reinterpret_cast<std::uint32_t*>(0xB2D928);

	surface[0] = 0x7E2070;                    // BSurface vftable
	surface[1] = BufferSize;                  // Width
	surface[2] = BufferSize;                  // Height
	surface[3] = 0;                           // LockLevel
	surface[4] = 1;                           // BytesPerPixel
	surface[5] = reinterpret_cast<std::uint32_t>(&VoxelPixelBuffer[0][0]);
	surface[6] = sizeof(VoxelPixelBuffer);

	// MUST be 0. Anything that treats this as "the surface owns its buffer"
	// would try to free a static array.
	surface[7] = 0;
}

// No destructor and no atexit. Vanilla registers deinit_VoxelShadowSurface, but
// there is nothing to tear down once the buffer is a static array - and calling
// ~BSurface() on a surface whose Allocated flag is 0 either does nothing or,
// worse, frees our array.
DEFINE_FUNCTION_JUMP(LJMP, 0x7539D0, Replacer::_Apply)

// ===========================================================================
// Init_spec_VoxelSurfaceBuffer2 @ 0x753A50 - the depth buffer's twin of the
// above. Byte-for-byte the same shape, so the same treatment applies.
//
//   .text:00753A64  mov VoxelBufferedSurface.Width,         eax   -> 0xB2D7F4
//   .text:00753A69  mov VoxelBufferedSurface.Height,        eax   -> 0xB2D7F8
//   .text:00753A6E  mov VoxelBufferedSurface.LockLevel,     0     -> 0xB2D7FC
//   .text:00753A82  mov VoxelBufferedSurface.BytesPerPixel, 1     -> 0xB2D800
//   .text:00753A5F  mov ecx, offset VoxelBufferedSurface.buff     -> 0xB2D804
//   .text:00753A96  mov VoxelBufferedSurface.vftble, BSurface     -> 0xB2D7F0
//
// Same trap as _Apply: do NOT construct a BSurface here, it would allocate.
// ===========================================================================

void __fastcall Replacer::_ApplyDepth()
{
	// VERIFY: BSurface base 0xB2D7F0, same 8-dword layout as 0xB2D928.
	auto* const surface = reinterpret_cast<std::uint32_t*>(0xB2D7F0);

	surface[0] = 0x7E2070;                    // BSurface vftable
	surface[1] = BufferSize;                  // Width
	surface[2] = BufferSize;                  // Height
	surface[3] = 0;                           // LockLevel
	surface[4] = sizeof(VoxelDepthBuffer[0][0]);   // BytesPerPixel - now 2
	surface[5] = reinterpret_cast<std::uint32_t>(&VoxelDepthBuffer[0][0]);
	surface[6] = sizeof(VoxelDepthBuffer);
	surface[7] = 0;                           // Allocated - must stay 0
}

DEFINE_FUNCTION_JUMP(LJMP, 0x753A50, Replacer::_ApplyDepth)

const float Replacer::BufferCenterXY = Replacer::BufferCenter;

// ===========================================================================
// RECENTRING THE PROJECTION - VoxelLibraryClass::Draw @ 0x756590
//
//   data.Start.X = (vect0->X + 128.0 - pos->X) * 256.0
//   data.Start.Y = (vect0->Y + 128.0 - pos->Y) * 256.0
//   data.Start.Z = (vect0->Z + 128.0 - pos->Z) * 256.0
//
// Integer pixel = Start >> 8 = world + 128 - camera, so the +128 is the centre
// of a 256-wide buffer. At BufferSize 512 it must become 256, otherwise voxels
// still land at pixel 128 and only the +X/+Y half of the new space is reachable.
//
// The constant is a float in .rdata (flt_7F695C), shared with other code, so we
// repoint the operand instead of editing the value:
//
//   .text:00756694  D8 05 [5C 69 7F 00]  fadd ds:flt_7F695C   disp32 at 0x756696
//   .text:007566B3  D8 05 [5C 69 7F 00]  fadd ds:flt_7F695C   disp32 at 0x7566B5
//   .text:007566D3  D8 05 [5C 69 7F 00]  fadd ds:flt_7F695C   disp32 at 0x7566D5
//
// !! ONLY X AND Y. Z IS LEFT AT 128.0 ON PURPOSE. !!
//
// Start.Z feeds the depth accumulator, which every spec-buffer rasterizer reads
// as `mov dx, word[..] ; shr dx, 8` into a ONE BYTE depth buffer. Depth is 0..255
// no matter how big the buffer gets. Recentring Z to 256 would push every depth
// value to wrap through zero and invert the depth test.
//
// The `fmul ds:flt_7E2224` (256.0) sites are the 8.8 fixed point scale and are
// deliberately untouched.
// ===========================================================================
// SUPERSEDED - VoxelLibraryClass::Draw is now ported (VoxelLibraryClass.Draw.cpp)
// and applies the centre itself, into 32-bit fields. Patching the float operand
// was never going to be enough: the destination was a 16-bit field, so the
// coordinate could not exceed 255 whatever the constant said.
//
//   DEFINE_PATCH_TYPED(DWORD, 0x756696, DWORD(&Replacer::BufferCenterXY))
//   DEFINE_PATCH_TYPED(DWORD, 0x7566B5, DWORD(&Replacer::BufferCenterXY))

// ===========================================================================
// ...AND THE CLIP RECT MUST MOVE WITH IT
//
// Voxel_Sort_Calc_And_Draw_Clipped_0 @ 0x7542F0 and _1 @ 0x754510 build the rect
// the blit reads back out of the surface, centred on the same 128:
//
//     rect.X = 128 - extentX / 2 - 4
//     rect.Y = 128 - extentY / 2 - 4
//     rect.Width  = extentX + 8
//     rect.Height = extentY + 8
//
// Four plain `mov r32, imm32` immediates, so disp at instruction + 1:
//
//   .text:00754678  BB 80 00 00 00   mov ebx, 80h   -> 0x754679   _1, X
//   .text:0075468B  B8 80 00 00 00   mov eax, 80h   -> 0x75468C   _1, Y
//   .text:0075446D  BA 80 00 00 00   mov edx, 80h   -> 0x75446E   _0, X
//   .text:00754480  BA 80 00 00 00   mov edx, 80h   -> 0x754481   _0, Y
//
// _1 is called from BulletClass::Draw_Voxel, TechnoClass_Draw_Voxel and
// TechnoClass_Voxel_Shadow; _0 from VoxelAnimClass::Draw. Both paths need it.
//
// Nothing here clamps the rect to the buffer, in vanilla or now - an oversized
// voxel always produced a rect larger than the surface. Raising BufferSize just
// moves that threshold out, which is the whole point.
//
// The screen destination (`calc.rect.X = centre - (extent + 8) / 2`) is derived
// from the extent, not from the 128, so it follows automatically.
// ===========================================================================
DEFINE_PATCH_TYPED(DWORD, 0x754679, Replacer::BufferCenterInt)   // Clipped_1, X
DEFINE_PATCH_TYPED(DWORD, 0x75468C, Replacer::BufferCenterInt)   // Clipped_1, Y
DEFINE_PATCH_TYPED(DWORD, 0x75446E, Replacer::BufferCenterInt)   // Clipped_0, X
DEFINE_PATCH_TYPED(DWORD, 0x754481, Replacer::BufferCenterInt)   // Clipped_0, Y

// ===========================================================================
// REMOVED: DEFINE_PATCH_TYPED(DWORD, 0x753C61, DWORD(&Replacer::VoxelPixelBuffer))
//
// It collided with the jump in VoxelBuffer.Externals.cpp:
//
//   DEFINE_FUNCTION_JUMP(LJMP, 0x753C60, ...)   writes 0x753C60 .. 0x753C64
//   DEFINE_PATCH_TYPED  (0x753C61, ...)         writes 0x753C61 .. 0x753C64
//
// The patch lands inside the jump's rel32. Whichever is applied second wins:
// either the jump is corrupted into a branch to a wild address, or the patch is
// silently overwritten. Neither is what you want, and it is order-dependent, so
// it may behave differently between builds.
//
// Voxel_get_VoxelPixelBuffer is already ported, so the patch is redundant.
// ===========================================================================
