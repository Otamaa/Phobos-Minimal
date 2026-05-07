/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the SDLMouse class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "sdlmouse.h"
#include "sdlsurface.h"
#include <WWMouseClass.h>
#include <Surface.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class WWMouseClassExt : public WWMouseClass
{
public:
    SDLMouseClass* CTOR_Proxy(Surface*, HWND);
};


/**
 *  A function imitating a constructor because we can't take the address of a constructor.
 *
 *  @author: ZivDero
 */
SDLMouseClass* WWMouseClassExt::CTOR_Proxy(Surface*, HWND)
{
    return new (reinterpret_cast<SDLMouseClass*>(this)) SDLMouseClass;
}

#define PATCH_SURFACE(addrsize, addrProx ,name)\
DEFINE_PATCH_ADDR_OFFSET(BYTE, addrsize , 1, sizeof(SDLSurface));\
DEFINE_FUNCTION_JUMP(CALL, addrProx, WWMouseClassExt::CTOR_Proxy);

PATCH_SURFACE(0x560D41, 0x560D62, MainOptions_Video_Mode_Change)
PATCH_SURFACE(0x561059, 0x56107A, MainOptions_Video_Mode_Change2)
PATCH_SURFACE(0x6BDEF9, 0x6BDF1C, MainOptions_Video_Mode_Change3)