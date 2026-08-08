// ===========================================================================
// Voxel_Init_Surface_Stuff  -  vanilla 0x753E00 .. 0x753F82
//   __cdecl, no arguments. Called from BulletClass::Draw_Voxel,
//   TechnoClass_Draw_Voxel, TechnoClass_Voxel_Shadow, VoxelAnimClass::Draw.
//
// PATCH AUDIT
//   0x753E5F  cmp eax,0FFh          your list  OK
//   0x753E6F  cmp eax,0FFh          your list  OK
//   0x753E84  lea esi,[ecx+esi+d32] your list  OK   (SIB, +3)
//   0x753E93  add esi,100h          your list  OK
//   0x753E26  mov edi,offset        your list  DEAD (inside your 0x753E1E hook)
//   0x753EBF  mov edi,offset        your list  DEAD (inside your 0x753EB7 hook)
//   0x753E7F  shl ecx,8             MISSING    imm8, log2 of the row stride
//   0x753E1F / 0x753E33 / 0x753EB8  MISSING    mov ecx,4000h dword counts
//   0x753E38 / 0x753EE9 / 0x753EEE / 0x753F05  MISSING  the depth-buffer half
//
// Everything above collapses into Replacer::BufferSize once ported. Your two
// ASMJIT memset hooks at 0x753E1E and 0x753EB7 become redundant - delete them.
//
// BUG (vanilla, preserved): THE DEPTH RECT CLEAR IGNORES THE BOUNDS TEST.
// The four `> 255` checks guard only the COLOUR clear; failing them jumps to
// loc_753EB7, which clears colour fully and then FALLS THROUGH into the depth
// block at loc_753EC5. That block only checks `Y > Y + Height`, so a clip rect
// wider or taller than the buffer still runs a rect clear on the depth buffer
// with out-of-range coordinates.
//
// This is the out-of-bounds clear flagged at the very start of this work, now
// confirmed. It is dormant while everything is 256 and the rect is in range -
// but it is exactly what fires if you raise the bounds to BufferSize-1 while
// VoxelBufferedPixelBuffer is still 256 wide. Past that buffer sit
// VoxelClippingMax, both BSurface objects and VoxelQueue.
//
// Left as-is because it is observable vanilla behaviour; the ported version is
// safe only because DepthBuffer and VoxelPixelBuffer are the same size.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstring>

// ===========================================================================
// DIAGNOSTIC: set to a palette index (e.g. 15 = white) to FILL the whole colour
// buffer instead of clearing it. Whatever the blit copies then shows up on
// screen as a solid block, and its size tells you exactly how large a region is
// actually being transferred out of the surface.
//
//   block is BufferSize x BufferSize   -> the blit is fine, the problem is
//                                         upstream in the rasterizers
//   block is ~256 x 256                -> something in the blit path is still
//                                         clamped to 256 and that is the bug
//   block matches the clip rect        -> the rect is correct and the limit is
//                                         somewhere else again
//
// Set back to 0 afterwards.
// ===========================================================================
#define PHOBOS_VOXEL_FILL_BUFFER_DIAGNOSTIC 0

// ===========================================================================
// ALWAYS FULL-CLEAR.  This is the fix for the stale-pixel glitch.
//
// The vanilla order per unit is:
//
//   1. Voxel_Init_Surface_Stuff()          clears using VoxelClippingRect
//   2. Voxel_Set_Queue_Values() x N        accumulates ClippingMax/Min
//   3. Voxel_Sort_Calc_And_Draw_Clipped()  DRAWS, then WRITES VoxelClippingRect
//   4. blit using the rect from step 3
//
// So the clear at step 1 uses the PREVIOUS unit's rect, while the blit at step 4
// copies THIS unit's rect. Wherever this unit's rect reaches past the previous
// one, the buffer still holds whatever an earlier unit left there - and the blit
// copies it out as if it were part of the model. That is the "missing segment"
// look: not geometry that failed to draw, but stale geometry drawn over it.
//
// It is a vanilla design flaw, not something this port introduced. Vanilla mostly
// got away with it because its rect could not exceed 256 and consecutive units
// overlapped heavily; the bigger the buffer, the further apart successive rects
// can sit and the more stale area is exposed.
//
// COST: a BufferSize^2 memset per unit per frame - 1 MB at 1024, versus 64 KB in
// vanilla. Measure before worrying: it is a linear memset, and correctness first.
//
// THE CHEAPER FIX, once this is confirmed: the rect derives purely from
// ClippingMax/Min, which are fully accumulated by the end of step 2. Porting
// Voxel_Sort_Calc_And_Draw_Clipped_0/_1 to compute the rect at the TOP of the
// function and clear exactly that before drawing removes the ordering problem
// entirely, and clears only what is about to be used.
// ===========================================================================
#define PHOBOS_VOXEL_ALWAYS_FULL_CLEAR 1

namespace
{
	// VERIFY: addresses from the .data dump and your existing 0x753E1E hook.
	// DWORD, not a byte: `mov eax, ds:8467E0h` / `mov ds:8467E0h, ebx`.
	inline int& ClearVoxelSurfaces = *reinterpret_cast<int*>(0x8467E0);
	inline int& VoxelQueueCount = *reinterpret_cast<int*>(0xB2D820);
	inline int& VoxelsQueued = *reinterpret_cast<int*>(0xB2FB70);
	inline int& VoxelUseBuffer = *reinterpret_cast<int*>(0xB43180);

	inline RectangleStruct& VoxelClippingRect =
		*reinterpret_cast<RectangleStruct*>(0xB2FB60);
	inline float* const VoxelClippingMax = reinterpret_cast<float*>(0xB2D5E0);
	inline float* const VoxelClippingMin = reinterpret_cast<float*>(0xB2D948);

	// Shared row-clear. Vanilla splits it into `shr ecx,2 ; rep stosd` plus
	// `and ecx,3 ; rep stosb`, which is just a byte memset.
	template<typename TBuffer>
	void ClearRect(TBuffer& buffer, int x, int y, int width, int height) noexcept
	{
		const int lastRow = y + height;
		if (y > lastRow)
		{
			return;
		}

		// * element size: the depth buffer is 16-bit now, the colour one 8-bit.
		const std::size_t rowBytes =
			(static_cast<std::size_t>(width) + 1) * sizeof(buffer[0][0]);
		for (int row = y; row <= lastRow; ++row)
		{
			std::memset(&buffer[row][x], 0, rowBytes);
		}
	}
}

static void __cdecl VoxelInit_SurfaceStuff() noexcept
{
	VoxelQueueCount = 0;
	VoxelsQueued = 0;

#if PHOBOS_VOXEL_FILL_BUFFER_DIAGNOSTIC
	std::memset(Replacer::VoxelPixelBuffer, PHOBOS_VOXEL_FILL_BUFFER_DIAGNOSTIC,
		sizeof(Replacer::VoxelPixelBuffer));
	std::memset(VoxelRaster::DepthBuffer, 0, sizeof(VoxelRaster::DepthBuffer));
	VoxelClippingMax[0] = 10000.0f; VoxelClippingMax[1] = 10000.0f;
	VoxelClippingMax[2] = 10000.0f; VoxelClippingMin[0] = -10000.0f;
	VoxelClippingMin[1] = -10000.0f; VoxelClippingMin[2] = -10000.0f;
	VoxelQueueCount = 0;
	VoxelsQueued = 0;
	return;
#endif

	if (ClearVoxelSurfaces != 0)
	{
		ClearVoxelSurfaces = 0;
		std::memset(Replacer::VoxelPixelBuffer, 0,
			sizeof(Replacer::VoxelPixelBuffer));
		std::memset(VoxelRaster::DepthBuffer, 0,
			sizeof(VoxelRaster::DepthBuffer));
	}
#if PHOBOS_VOXEL_ALWAYS_FULL_CLEAR
	else
	{
		std::memset(Replacer::VoxelPixelBuffer, 0,
			sizeof(Replacer::VoxelPixelBuffer));

		if (VoxelUseBuffer != 0)
		{
			std::memset(VoxelRaster::DepthBuffer, 0,
				sizeof(VoxelRaster::DepthBuffer));
		}
	}
#else
	else
	{
		const int rectX = VoxelClippingRect.X;
		const int rectY = VoxelClippingRect.Y;
		const int rectW = VoxelClippingRect.Width;
		const int rectH = VoxelClippingRect.Height;

		if (rectX < 0
			|| rectY < 0
			|| rectX + rectW > Replacer::BufferSize - 1
			|| rectY + rectH > Replacer::BufferSize - 1)
		{
			// loc_753EB7 - full colour clear, then FALL THROUGH to the depth
			// block below. See the BUG note in the header.
			std::memset(Replacer::VoxelPixelBuffer, 0,
				sizeof(Replacer::VoxelPixelBuffer));
		}
		else
		{
			ClearRect(Replacer::VoxelPixelBuffer, rectX, rectY, rectW, rectH);
		}

		// loc_753EC5 - separate `if`, reached from both paths above.
		if (VoxelUseBuffer != 0)
		{
			ClearRect(VoxelRaster::DepthBuffer, rectX, rectY, rectW, rectH);
		}
	}
#endif

	// loc_753F11
	VoxelClippingMax[0] = 10000.0f;
	VoxelClippingMax[1] = 10000.0f;
	VoxelClippingMax[2] = 10000.0f;
	VoxelClippingMin[0] = -10000.0f;
	VoxelClippingMin[1] = -10000.0f;
	VoxelClippingMin[2] = -10000.0f;
}

// ---------------------------------------------------------------------------
// DELETE your ASMJIT_PATCH hooks at 0x753E1E and 0x753EB7 - this replaces both.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x753E00, VoxelInit_SurfaceStuff)