#include "Surface.h"

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x4BA770, DXSurface::CreatePrimary);
DEFINE_FUNCTION_JUMP(CALL, 0x00560D2D, DXSurface::CreatePrimary)
DEFINE_FUNCTION_JUMP(CALL, 0x00560F13, DXSurface::CreatePrimary)
DEFINE_FUNCTION_JUMP(CALL, 0x006BDCA2, DXSurface::CreatePrimary)

DEFINE_JUMP(LJMP, 0x77747A, 0x777575); // Skip Restore_Check

static DXSurface* __fastcall _DXSurface_CTOR(DXSurface* surface, discard_t, int width, int height, bool system_mem, bool enable_3d) {
	return new(surface) DXSurface(width, height);
}

#ifdef _DSurfaceSize_nEEDfIX
#define PATCH_SURFACE(addrsize, addrProx ,name)\
DEFINE_PATCH_ADDR_OFFSET(BYTE, addrsize , 1, sizeof(DXSurface));\
DEFINE_FUNCTION_JUMP(CALL, addrProx, _DXSurface_CTOR);
#else
#define PATCH_SURFACE(addrsize, addrProx ,name)\
DEFINE_FUNCTION_JUMP(CALL, addrProx, _DXSurface_CTOR);
#endif

DEFINE_FUNCTION_JUMP(LJMP, 0x004BA5A0, _DXSurface_CTOR);
DEFINE_FUNCTION_JUMP(CALL, 0x0055CE44, _DXSurface_CTOR)

PATCH_SURFACE(0x004C425D, 0x004C427D, Show_Who_Was_Responsible)
PATCH_SURFACE(0x0053B486, 0x0053B4B7, IonStormClass_ChronoScreenEffect_Start)
PATCH_SURFACE(0x0053BB0B, 0x0053BB3C, IonStormClass_ChronoScreenEffect_Reset)
PATCH_SURFACE(0x00640DA5, 0x00640E0E, PreviewClass_Draw_Map)
PATCH_SURFACE(0x00641276, 0x0064128C, PreviewClass_Generate_Preview_Image)
PATCH_SURFACE(0x00641B43, 0x00641B69, PreviewClass_Read_Preview_INI)
PATCH_SURFACE(0x00641E25, 0x00641E4E, PreviewClass_Read_PCX_Preview)
PATCH_SURFACE(0x00654760, 0x00654794, RadarClass_Init_Radar_Surfaces)
PATCH_SURFACE(0x0068CEBB, 0x0068CEEA, ScoreClass_Presentation)
PATCH_SURFACE(0x00642627, 0x00642641, PreviewClass_Create_Preview_Surface)