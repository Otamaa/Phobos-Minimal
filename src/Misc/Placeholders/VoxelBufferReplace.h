#pragma once

#include <RectangleStruct.h>
#include <Helpers/CompileTime.h>
#include <Surface.h>

struct Replacer
{
	static constexpr int BufferSize = 512;
	static char VoxelPixelBuffer[BufferSize][BufferSize];
	static constexpr reference<RectangleStruct, 0xB2FB60> VoxelClippingRect {};
	static constexpr reference<BSurface, 0xB2D7F0> VoxelSurface {};

	static void _Apply() {
		VoxelSurface->Width = BufferSize;
		VoxelSurface->Height = BufferSize;
		VoxelSurface->BufferPtr = MemoryBuffer(VoxelPixelBuffer, sizeof(VoxelPixelBuffer));
		//VoxelClippingRect = RectangleStruct(0, 0, Replacer::BufferSize - 1, Replacer::BufferSize - 1);
	}
};