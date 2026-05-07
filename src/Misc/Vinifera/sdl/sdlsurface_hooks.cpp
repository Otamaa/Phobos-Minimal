/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the SDLSurface class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include "sdlsurface.h"


 /**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or destructor.
  *
  *  @note: All functions must not be virtual and must also be prefixed
  *         with "_" to prevent accidental virtualization.
  */
class DSurfaceExt : public DSurface
{
public:
	DSurface* CTOR_Proxy(int width, int height, bool system_memory);
};


/**
 *  A function imitating a constructor because we can't take the address of a constructor.
 *
 *  @author: ZivDero
 */
DSurface* DSurfaceExt::CTOR_Proxy(int width, int height, bool system_memory)
{
	return new (this) SDLSurface(width, height);
}


/**
 *  Main function for patching the hooks.
 */
#define PATCH_SURFACE(addrsize, addrProx ,name)\
DEFINE_PATCH_ADDR_OFFSET(BYTE, addrsize , 1, sizeof(SDLSurface));\
DEFINE_FUNCTION_JUMP(CALL, addrProx, DSurfaceExt::CTOR_Proxy);

DEFINE_FUNCTION_JUMP(CALL , 0x55CE44, DSurfaceExt::CTOR_Proxy)

DEFINE_FUNCTION_JUMP(CALL, 0x00560D2D, SDLSurface::Create_Primary)
DEFINE_FUNCTION_JUMP(CALL, 0x00560F13, SDLSurface::Create_Primary)
DEFINE_FUNCTION_JUMP(CALL, 0x006BDCA2, SDLSurface::Create_Primary)

DEFINE_FUNCTION_JUMP(CALL, 0x004A691F, SDLSurface::GetDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005BC56A, SDLSurface::GetDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005BD4B7, SDLSurface::GetDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005E286E, SDLSurface::GetDC);
DEFINE_FUNCTION_JUMP(CALL, 0x006191C5, SDLSurface::GetDC);
DEFINE_FUNCTION_JUMP(CALL, 0x006194D3, SDLSurface::GetDC);

DEFINE_FUNCTION_JUMP(CALL, 0x004A6942, SDLSurface::ReleaseDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005BC6CC, SDLSurface::ReleaseDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005BD552, SDLSurface::ReleaseDC);
DEFINE_FUNCTION_JUMP(CALL, 0x005E2990, SDLSurface::ReleaseDC);
DEFINE_FUNCTION_JUMP(CALL, 0x006191E5, SDLSurface::ReleaseDC);
DEFINE_FUNCTION_JUMP(CALL, 0x00619A25, SDLSurface::ReleaseDC);

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

DEFINE_JUMP(LJMP, 0x00777450, 0x00777575)// Skip Restore_Check calls in Focus_Restore

