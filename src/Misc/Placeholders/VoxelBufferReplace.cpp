#ifndef VoxelBufferReplace

#include "Hooks.Otamaa.h"
#include "VoxelBufferReplace.h"

char Replacer::VoxelPixelBuffer[Replacer::BufferSize][Replacer::BufferSize];

ASMJIT_PATCH(0x754720, Voxel_Clear_Voxel_Surface_Buffer, 0x6)
{
	std::memset(Replacer::VoxelPixelBuffer, 0, sizeof(Replacer::VoxelPixelBuffer));
	return 0x754730;
}

ASMJIT_PATCH(0x7547A0, Voxel_Init_Surface_Stuff_Memset3, 0x5)
{
	std::memset(Replacer::VoxelPixelBuffer, 0, sizeof(Replacer::VoxelPixelBuffer));
	return 0x7547AE;
}

ASMJIT_PATCH(0x753EB7, Voxel_Init_Surface_Stuff_Memset2, 0x5)
{
	std::memset(Replacer::VoxelPixelBuffer, 0, sizeof(Replacer::VoxelPixelBuffer));
	return 0x753EC5;
}

ASMJIT_PATCH(0x753E1E, Voxel_Init_Surface_Stuff_Memset1, 0x5)
{
	std::memset(Replacer::VoxelPixelBuffer, 0, sizeof(Replacer::VoxelPixelBuffer));
	*reinterpret_cast<bool*>(0x8467E0) = R->EBX();
	return 0x753E32;
}

//size max
DEFINE_PATCH_TYPED(DWORD, 0x754752, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x75475F, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x753E5F, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x753E6F, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x7547D8, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x7547E4, Replacer::BufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x753E93, Replacer::BufferSize)
DEFINE_PATCH_TYPED(DWORD, 0x75478D, Replacer::BufferSize)
//surface size
DEFINE_PATCH_TYPED(DWORD, 0x7539D1, Replacer::BufferSize)

// some pointer shifting
//DEFINE_PATCH_TYPED(BYTE, 0x754786, 0x8)
//7547F2

DEFINE_PATCH_TYPED(DWORD, 0x7539D6, sizeof(Replacer::VoxelPixelBuffer))
DEFINE_PATCH_TYPED(DWORD, 0x753C61, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7539DB, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753E26, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753E84, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753EBF, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754729, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754776, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7547A8, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754803, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754832, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756A7B, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756A88, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x756B4C, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756B52, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x756EDF, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757063, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75728B, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757291, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x75748C, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757492, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7576EE, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7576F4, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7578B1, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7578B7, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757B1B, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757B21, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757D4F, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757D55, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757F81, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757F87, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758118, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75811E, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758358, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75835E, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x75855A, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758560, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7DF8A7, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DF998, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFAB8, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFBCA, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFCE5, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFDD7, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFEE5, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFEEB, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7DFFD7, DWORD(&Replacer::VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFFDD, DWORD(&Replacer::VoxelPixelBuffer) + 1)//
#endif
