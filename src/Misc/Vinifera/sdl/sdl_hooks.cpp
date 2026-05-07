/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "sdl_functions.h"
#include "syringe.h"

#include "winuser.h"

#include <algorithm>
#include <VQAClass.h>
#include <Surface.h>

/**
 *  Update the window after updating the visible surface.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F4B85, _Update_Visible_Surface_SDL_Update_Window_Patch, 5)
{
    SDL_Update_Screen(DSurface::Primary());

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing a movie frame.
 * 
 *  @author: tomsons26, ZivDero
 */

void SDL_Movie_Blit_To_Screen()
{
    if (!VQA_Get_Option(VQAAnimControlFlags::VQAAnimControlFlags_15)) {
        DSurface::Primary()->Copy_From(VQHandle::Instance()->StretchedRect, VQHandle::Instance()->Surface, VQHandle::Instance()->InitialRect);
        SDL_Update_Screen(DSurface::Primary());
    }
}

void SDL_Movie_Update_Visible_Surface()
{
    if (VQHandle::Instance() != nullptr) {
        DSurface::Alternate->Fill(0);
        GameDefinitions::Update_Visible_Surface(false, DSurface::Alternate(), nullptr);
        SDL_Movie_Blit_To_Screen();
    }
}


/**
 *  Update the window after updating the visible surface when drawing in MSEngine.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005D230C, _MSEngine_BlitAll_SDL_Update_Window_Patch, 7)
{
    SDL_Update_Screen(DSurface::Primary());

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing in MSEngine.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005D23E8, _MSEngine_BlitRect_SDL_Update_Window_Patch, 6)
{
    SDL_Update_Screen(DSurface::Primary());

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing in ScoreClass.
 *
 *  @author: ZivDero
 */
//DEFINE_HOOK(0x005E6468, _ScoreClass_Call_Back_Delay_SDL_Update_Window_Patch, 6)
//{
//    SDL_Update_Screen(DSurface::Primary());
//
//    return 0;
//}


/**
 *  Dummy replacement for ClientToScreen. SDL uses client coordinates directly,
 *  so most of these calls are now not necessary.
 *
 *  @author: ZivDero
 */
BOOL WINAPI Fake_ClientToScreen(HWND hwnd, LPPOINT point)
{
    return TRUE;
}


/**
 *  CtrlProc is the main window procedure for all Tiberian Sun window controls.
 *  We substitute it with a proxy so that after it is done, can update the screen.
 *
 *  @author: ZivDero
 */
LRESULT CALLBACK CtrlProcProxy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = CtrlProc(window, message, wparam, lparam);
    if (message == WM_PAINT) {
        SDL_Update_Screen(DSurface::Primary());
    }
    return result;
}


/**
 *  Windows have a sliding opening animation that happens within a single WM_PAINT call,
 *  so we need to hook multiple locations within CtrlProc to ensure the screen is updated.
 *
 *  @author: ZivDero
 */
//DEFINE_HOOK(0x00593F8D, _CtrlProc_SDL_Update_Screen_Patch, 6)
//{
//    SDL_Update_Screen(DSurface::Primary());
//    return 0;
//}
//DEFINE_HOOK_AGAIN(0x00594101, _CtrlProc_SDL_Update_Screen_Patch, 6);
//DEFINE_HOOK_AGAIN(0x0059437C, _CtrlProc_SDL_Update_Screen_Patch, 6);
//DEFINE_HOOK_AGAIN(0x0059449F, _CtrlProc_SDL_Update_Screen_Patch, 6);


/**
 *  The kick player dialog needs separate hooks to update the screen after drawing.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x64AE95, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 5)
{
    SDL_Update_Screen(DSurface::Primary());
    return 0;
}
DEFINE_HOOK_AGAIN(0x64AEC8, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 5);
DEFINE_HOOK_AGAIN(0x64AEA6, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 6);

/**
 *  This function moves a dialog window to a specified position.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
int __fastcall _ODMoveDialog(HWND window, int x, int y)
{
    int xpos;
    int ypos;

    RECT rect1;
    rect1.left = 0;
    rect1.top = 0;
    rect1.right = GameDefinitions::VideoWidth();
    rect1.bottom = GameDefinitions::VideoHeight();

    ClientToScreen(Game::hWnd(), (LPPOINT)&rect1);
    ClientToScreen(Game::hWnd(), (LPPOINT)&rect1.right);

    RECT rect2;
    GetWindowRect(window, &rect2);

    rect2.right -= rect2.left;
    rect2.bottom -= rect2.top;

    if (x == -1) {
        xpos = rect2.left - rect1.left;
    } else {
        xpos = x;
    }
    rect2.left = xpos;

    if (y == -1) {
        ypos = rect2.top - rect1.top;
    } else {
        ypos = y;
    }
    rect2.top = ypos;

    return MoveWindow(window, rect2.left, rect2.top, rect2.right, rect2.bottom, FALSE);
}


/**
 *  This function centers a window within a parent window.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
void __fastcall _Center_Window_Within_Window(HWND window, HWND parent)
{
    RECT rcl;
    GetClientRect(parent, &rcl);

    if (parent == Game::hWnd()) {
        rcl.right = GameDefinitions::VideoWidth();
        rcl.bottom = GameDefinitions::VideoHeight();
    }

    ClientToScreen(parent, (LPPOINT)&rcl);
    ClientToScreen(parent, (LPPOINT)&rcl.right);
    rcl.right -= rcl.left;
    rcl.bottom -= rcl.top;

    RECT rect;
    GetClientRect(window, &rect);
    ClientToScreen(window, (LPPOINT)&rect);
    ClientToScreen(window, (LPPOINT)&rect.right);
    rect.right -= rect.left;
    rect.bottom -= rect.top;
    int x = (rcl.right - rect.right + 1) / 2;
    int y = (rcl.bottom - rect.bottom + 1) / 2;

    x = std::max(x, 0);
    y = std::max(y, 0);

    SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}


/**
 *  This function gets the display rectangle of a window.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
BOOL __fastcall _GetDisplayRect(HWND window, LPRECT rect)
{
    RECT c;
    BOOL res = GetWindowRect(window, rect);
    if (!res) {
        return res;
    }
    GetClientRect(Game::hWnd(), &c);
    ClientToScreen(Game::hWnd(), (LPPOINT)&c);
    rect->left -= c.left;
    rect->right -= c.left;
    rect->top -= c.top;
    rect->bottom -= c.top;
    return res;
}


/**
 *  Proxy for GetWindowRect because we cannot take the address of GetWindowRect directly.
 *
 *  @author: ZivDero
 */
BOOL __fastcall _GetWindowRect(HWND window, LPRECT rect)
{
    return GetWindowRect(window, rect);
}

#include <ToolTipManager.h>
#include <WWMouseClass.h>

/**
 *  Patch to make tooltips use MouseCursor for mouse position instead of querying Windows.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00724359, _ToolTipManager_Message_Handler_Mouse_Pos_Patch_, 0)
{
    GET(ToolTipManager*, tooltips, ESI);

    tooltips->CurrentMousePosition = WWMouseClass::Instance->GetCoords_();

    return 0x0072436E;
}


/**
 *  Replacement for EnumDisplayModes that uses Windows API to enumerate display modes
 *  instead of relying on DirectDraw.
 *
 *  @author: ZivDero
 */
int* __stdcall _EnumDisplayModes(DWORD minw, DWORD minh, DWORD maxw, DWORD maxh, DWORD bitdepth)
{
    std::vector<std::pair<int, int>> modes;
    DEVMODE devmode;
    DWORD mode_index = 0;

    /**
     *  Enumerate all available display modes.
     */
    while (EnumDisplaySettings(nullptr, mode_index++, &devmode)) {
        const DWORD w = devmode.dmPelsWidth;
        const DWORD h = devmode.dmPelsHeight;
        const DWORD bpp = devmode.dmBitsPerPel;

        if (w >= minw && h >= minh && w <= maxw && h <= maxh && bpp == bitdepth) {
            modes.emplace_back(static_cast<int>(w), static_cast<int>(h));
        }
    }

    if (modes.empty()) {
        return nullptr;
    }

    /**
     *  Sort and remove duplicates.
     */
    std::sort(modes.begin(), modes.end());
    modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

    /**
     *  Allocate contiguous buffer for result (two ints per mode, plus a trailing 0).
     */
    const size_t count = modes.size();
    const size_t bytes = sizeof(int) * (count * 2 + 1);

    int* list = static_cast<int*>(operator new[](bytes));
    std::memset(list, 0, bytes);

    int* ptr = list;
    for (const auto& mode : modes) {
        *ptr++ = mode.first;
        *ptr++ = mode.second;
    }

    return list;
}


/**
 *  Main function for patching the hooks.
 */

DEFINE_FUNCTION_JUMP(LJMP, 0x00533FD0, SDL_Allocate_Surfaces)

/**
 *  SDL rendering prep.
 */

DEFINE_FUNCTION_JUMP(LJMP, 0x004A42F0, SDL_Set_Video_Mode);
DEFINE_FUNCTION_JUMP(LJMP, 0x004A44F0, SDL_Reset_Video_Mode);
DEFINE_FUNCTION_JUMP(LJMP, 0x00560BF0, SDL_Change_Display_Mode);

void SDL_Hooks()
{
    /**
     *  Disable ClientToScreen.
     */
	Imports::ClientToScreen = Fake_ClientToScreen;

    /**
     *  But these 3 need to use the real ClientToScreen so that dialogs are where they should be.
     */
	Patch::Apply_LJMP(0x00623170, _ODMoveDialog);
	Patch::Apply_LJMP(0x00777080, _Center_Window_Within_Window);
	Patch::Apply_LJMP(0x00775690, _GetDisplayRect);

    /**
     *  Except for this GetDisplayRect, it's used only for drawing offset, and returning GetWindowRect makes the offset 0.
     */
	Patch::Apply_CALL(0x00610E77, _GetWindowRect);

    /**
     *  ComboDropWinCtrlProc, fix the dropdown being offset by the main window origin by patching out 2 add instructions.
     */
    //Patch_Byte_Range(0x0058FFB2, 0x90, 2);
    //Patch_Byte_Range(0x0058FFBF, 0x90, 2);

    /**
     *  Skip Wait_Blit.
     */
	Patch::Apply_LJMP(0x004A4830, 0x004A4848);



    /**
     *  Update the window surface when the game updates its DSurface::Primary().
     */
    //Patch_Jump(0x00564050, &SDL_Movie_Blit_To_Screen);
    //Patch_Jump(0x005C03D0, &SDL_Movie_Update_Visible_Surface);
	Patch::Apply_RAW(0x0060FF06, sizeof(uintptr_t), PatchType::PATCH_, reinterpret_cast<const BYTE*>((uintptr_t)&CtrlProcProxy)); // Windows controls

    /**
     *  Call Set_Video_Mode even when windowed.
     */
    //Patch_Jump(0x006016B8, 0x006016F3);
    //Patch_Jump(0x006016BF, 0x006015A9);

    /**
     *  Disable DirectDraw.
     */
	Patch::Apply_VTABLE(0x0080F1A0, (uintptr_t)&"NQXZJYVPRKMTLUGHSBDCFIEWOAQRMZNPLXTYVJHKSQGBFUACEL.DLL"); // replace DDRAW.DLL by a very unlikely library in the import table
    Patch::Apply_LJMP(0x004A3FD0, 0x004A40BD); // skip Prep_Direct_Draw
	Patch::Apply_LJMP(0x00561400, _EnumDisplayModes); // relies on DirectDraw to enumerate display modes
}
