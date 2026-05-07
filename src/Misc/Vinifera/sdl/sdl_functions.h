/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains functions for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <RectangleStruct.h>
#include <Phobos.h>
#include <Surface.h>
#include "Defs.h"

bool __fastcall SDL_Allocate_Surfaces(const RectangleStruct& hidden_rect, const RectangleStruct& composite_rect, const RectangleStruct& tile_rect, const RectangleStruct& sidebar_rect, bool hidden_first);
bool __fastcall SDL_Set_Video_Mode(HWND, int width, int height, int bits_per_pixel);
void SDL_Reset_Video_Mode();
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height);
void SDL_Destroy_Main_Window();
bool SDL_Update_Screen(Surface* surface);
bool SDL_Should_Scale();
bool __fastcall SDL_Change_Display_Mode(int width, int height);

/**
 *  Returns the current X-axis scaling factor.
 *
 *  @author: ZivDero
 */
inline float SDL_XScale()
{ 
    return static_cast<float>(GameDefinitions::VideoWidth()) / static_cast<float>(Phobos::Misc::SDLWindowWidth);
}

/**
 *  Returns the current X-axis scaling factor.
 *
 *  @author: ZivDero
 */
inline float SDL_YScale()
{
    return static_cast<float>(GameDefinitions::VideoHeight()) / static_cast<float>(Phobos::Misc::SDLWindowWidth);
}

