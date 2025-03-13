#ifdef VoxelBufferReplace

#define VoxelBufferSize 256
static char VoxelPixelBuffer[VoxelBufferSize][VoxelBufferSize];
//static char VoxelShadowPixelBuffer[256][256];
//static BSurface NewVoxelBuffer { 256 , 256 , 1 , VoxelPixelBuffer };
//static BSurface NewVoxelShadowBuffer { 256 , 256 , 1 , VoxelPixelBuffer };

DEFINE_HOOK(0x754720, Voxel_Clear_Voxel_Surface_Buffer, 0x6)
{
	std::memset(VoxelPixelBuffer, 0, sizeof(VoxelPixelBuffer));
	return 0x754730;
}

DEFINE_HOOK(0x7547A0, Voxel_Init_Surface_Stuff_Memset3, 0x5)
{
	std::memset(VoxelPixelBuffer, 0, sizeof(VoxelPixelBuffer));
	return 0x7547AE;
}

DEFINE_HOOK(0x753EB7, Voxel_Init_Surface_Stuff_Memset2, 0x5)
{
	std::memset(VoxelPixelBuffer, 0, sizeof(VoxelPixelBuffer));
	return 0x753EC5;
}

DEFINE_HOOK(0x753E1E, Voxel_Init_Surface_Stuff_Memset1, 0x5)
{
	std::memset(VoxelPixelBuffer, 0, sizeof(VoxelPixelBuffer));
	*reinterpret_cast<bool*>(0x8467E0) = R->EBX();
	return 0x753E32;
}


//size max
//DEFINE_PATCH_TYPED(DWORD, 0x754752, VoxelBufferSize)
//DEFINE_PATCH_TYPED(DWORD, 0x75475F, VoxelBufferSize)
DEFINE_PATCH_TYPED(DWORD, 0x753E5F, VoxelBufferSize - 1)
DEFINE_PATCH_TYPED(DWORD, 0x753E6F, VoxelBufferSize - 1)
//DEFINE_PATCH_TYPED(DWORD, 0x7547D8, VoxelBufferSize)
//DEFINE_PATCH_TYPED(DWORD, 0x7547E4, VoxelBufferSize)
DEFINE_PATCH_TYPED(DWORD, 0x753E93, VoxelBufferSize)
//surface size
DEFINE_PATCH_TYPED(DWORD, 0x7539D1, VoxelBufferSize)

// some pointer shifting
//DEFINE_PATCH_TYPED(BYTE, 0x754786, 0x8)
//7547F2

DEFINE_PATCH_TYPED(DWORD, 0x7539D6, sizeof(VoxelPixelBuffer))
DEFINE_PATCH_TYPED(DWORD, 0x753C61, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7539DB, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753E26, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753E84, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x753EBF, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754729, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754776, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7547A8, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754803, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x754832, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756A7B, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756A88, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x756B4C, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x756B52, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x756EDF, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757063, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75728B, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757291, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x75748C, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757492, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7576EE, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7576F4, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7578B1, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7578B7, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757B1B, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757B21, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757D4F, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757D55, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x757F81, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x757F87, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758118, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75811E, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758358, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x75835E, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x75855A, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x758560, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7DF8A7, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DF998, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFAB8, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFBCA, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFCE5, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFDD7, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFEE5, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFEEB, DWORD(&VoxelPixelBuffer) + 1)//
DEFINE_PATCH_TYPED(DWORD, 0x7DFFD7, DWORD(&VoxelPixelBuffer))//
DEFINE_PATCH_TYPED(DWORD, 0x7DFFDD, DWORD(&VoxelPixelBuffer) + 1)//
#undef VoxelBufferSize
#endif
