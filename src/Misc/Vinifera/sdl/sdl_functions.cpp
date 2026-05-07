/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains functions for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include <Utilities/Debug.h>

#include "sdl_functions.h"

#include <Lib/SDL3/SDL_hints.h>
#include <Lib/SDL3/SDL_init.h>
#include <Lib/SDL3/SDL_oldnames.h>
#include <Lib/SDL3/SDL_render.h>
#include <Lib/SDL3/SDL_video.h>

#include <GameOptionsClass.h>

#include "sdlmouse.h"
#include "sdlsurface.h"

#include <windowsx.h>
#include <CCToolTip.h>
#include <WWKeyboardClass.h>

#include <CD.h>

enum class RendererDriverType
{
	RENDERER_DRIVER_AUTO = -1,
	RENDERER_DRIVER_DIRECT3D,
	RENDERER_DRIVER_DIRECT3D11,
	RENDERER_DRIVER_DIRECT3D12,
	RENDERER_DRIVER_OPENGL,
	RENDERER_DRIVER_VULKAN
};

struct RendererDriverInfo
{
	const char* ConfigName;
	const char* SDLName;
	RendererDriverType Type;
};

constexpr RendererDriverInfo RendererDrivers[] = {
	{"Direct3D", "direct3d", RendererDriverType::RENDERER_DRIVER_DIRECT3D},
	{"Direct3D11", "direct3d11", RendererDriverType::RENDERER_DRIVER_DIRECT3D11},
	{"Direct3D12", "direct3d12", RendererDriverType::RENDERER_DRIVER_DIRECT3D12},
	{"OpenGL", "opengl", RendererDriverType::RENDERER_DRIVER_OPENGL},
	{"Vulkan", "vulkan", RendererDriverType::RENDERER_DRIVER_VULKAN}
};


namespace
{
    /**
     *  Applies the SDL renderer driver hint for the selected backend.
     *
     *  @author: ZivDero
     */
    void SDL_Apply_Renderer_Driver_Hint()
    {
        const char* requested_driver_name = RendererDrivers[4].SDLName;
        const char* requested_driver_config_name = RendererDrivers[4].ConfigName;

        Debug::Log("Requested renderer driver: %s\n", requested_driver_config_name);

        if (requested_driver_name != nullptr) {
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, requested_driver_name);
        } else {
            SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        }
    }

    /**
     *  Computes the tactical display rectangle for the given visible area.
     *
     *  @author: ZivDero
     */
    RectangleStruct SDL_Get_Display_View_Rect(const RectangleStruct& visible_rect)
    {
		RectangleStruct temp = visible_rect;
        temp.X = GameOptionsClass::Instance->SidebarMode || Unsorted::MAP_DEBUG_MODE() ? 0 : 168;
        temp.Y = 16;
        temp.Width -= 168;
        temp.Height -= 16;
        return temp;
    }

    /**
     *  Recalculates the SDL mouse cursor image if a cursor exists.
     *
     *  @author: ZivDero
     */
    void SDL_Recalc_Mouse_Cursor_Image()
    {
        if (WWMouseClass::Instance() != nullptr) {
           ((SDLMouseClass*)WWMouseClass::Instance())->Recalc_Cursor_Image();
        }
    }

    /**
     *  Rebuilds the software surfaces and UI state for the current display mode.
     *
     *  @author: ZivDero
     */
    void SDL_Rebuild_Display_State(const RectangleStruct& visible_rect)
    {
		RectangleStruct temp = SDL_Get_Display_View_Rect(visible_rect);

        DSurface::WindowBounds = visible_rect;
        GameDefinitions::VideoWidth() = visible_rect.Width;
        GameDefinitions::VideoHeight() = visible_rect.Height;

        DSurface::Primary = SDLSurface::Create_Primary();
		auto composite_rect = RectangleStruct { 0, 0, temp.Width,  DSurface::WindowBounds->Height };
		auto composite_rectb = RectangleStruct { 0, 0, 168,  DSurface::WindowBounds->Height };
		
        Allocate_Surfaces(
            DSurface::WindowBounds(),
			composite_rect,
			composite_rect,
			composite_rectb,
			false);

        DSurface::Temp = DSurface::Hidden();

        WWMouseClass::Instance->HideCursor();
        SDL_Recalc_Mouse_Cursor_Image();
         WWMouseClass::Instance->ShowCursor();

		DisplayClass::Instance->Set_View_Dimensions(temp);
		MapClass::Instance->Init_IO();
		SidebarClass::Instance->Activate(1);
		SidebarClass::Instance->CloseWindow();
		DisplayClass::Instance->RedrawSidebar(2);
         WWMouseClass::Instance->ShowCursor();
    }
}


/**
 *  Allocates all game surfaces with the given sizes.
 *
 *  @author: ZivDero, tomsons26
 */
bool __fastcall SDL_Allocate_Surfaces(const RectangleStruct& hidden_rect, const RectangleStruct& composite_rect, const RectangleStruct& tile_rect, const RectangleStruct& sidebar_rect, bool hidden_first)
{
    Debug::Log("Allocating new surfaces\n");

    if (DSurface::Alternate() != nullptr) {
        Debug::Log("Deleting Alternate\n");
        delete DSurface::Alternate();
        DSurface::Alternate = nullptr;
    }

    if (DSurface::Hidden() != nullptr) {
        Debug::Log("Deleting HiddenSurface\n");
        delete DSurface::Hidden();
		DSurface::Hidden = nullptr;
    }

    if (DSurface::Composite() != nullptr) {
        Debug::Log("Deleting CompositeSurface\n");
        delete DSurface::Composite();
		DSurface::Composite() = nullptr;
    }

    if (DSurface::Tile() != nullptr) {
        Debug::Log("Deleting TileSurface\n");
        delete DSurface::Tile();
		DSurface::Tile = nullptr;
    }

    if (DSurface::Sidebar() != nullptr) {
        Debug::Log("Deleting SidebarSurface\n");
        delete DSurface::Sidebar();
		DSurface::Sidebar = nullptr;
    }

    if (hidden_first && hidden_rect.IsValid()) {
		DSurface::Hidden = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
		DSurface::Hidden->Fill(0);
        Debug::Log("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (composite_rect.IsValid()) {
		DSurface::Composite = new SDLSurface(composite_rect.Width, composite_rect.Height);
		DSurface::Composite->Fill(0);
        Debug::Log("CompositeSurface (%dx%d)\n", composite_rect.Width, composite_rect.Height);
    }

    if (tile_rect.IsValid()) {
		DSurface::Tile = new SDLSurface(tile_rect.Width, tile_rect.Height);
		DSurface::Tile->Fill(0);
        Debug::Log("TileSurface (%dx%d)\n", tile_rect.Width, tile_rect.Height);
    }

    if (sidebar_rect.IsValid()) {
		DSurface::Sidebar = new SDLSurface(sidebar_rect.Width, sidebar_rect.Height);
		DSurface::Sidebar->Fill(0);
        Debug::Log("SidebarSurface (%dx%d)\n", sidebar_rect.Width, sidebar_rect.Height);
    }

    if (!hidden_first && hidden_rect.IsValid()) {
		DSurface::Hidden = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
		DSurface::Hidden->Fill(0);
        Debug::Log("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (hidden_rect.IsValid()) {
        DSurface::Alternate = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        DSurface::Alternate->Fill(0);
        Debug::Log("Alternate (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    return true;
}

void __fastcall Activate_Command_From_Name(const char* a1) {
	JMP_FAST(0x533F50);
}

bool __stdcall On_Window_Moving(HWND, WPARAM, LPARAM) {
	JMP_STD(0x776D80);
}

bool __fastcall Is_Movie_Playing() {
	JMP_FAST(0x5C0980);
}

void __fastcall Blit_Movie() {
	JMP_FAST(0x5C09A0);
}

/**
 *  Initializes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
bool __fastcall SDL_Set_Video_Mode(HWND, int width, int height, int bits_per_pixel)
{
    if (Phobos::Misc::SDLWindow == nullptr) {
        Debug::Log("SDLWindow is null!\n");
        return false;
    }
    
    /**
     *  We need to delete the existing presentation layer first.
     */
    SDL_Reset_Video_Mode();
    
    /**
     *  Query the window's pixel format.
     */
    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(Phobos::Misc::SDLWindow);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        Debug::Log("SDL3 window pixel format unsupported: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));
        return false;
    }

    Debug::Log("Pixel format: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));

    /**
     *  Apply the renderer backend selection before creating the renderer.
     */
    SDL_Apply_Renderer_Driver_Hint();

    /**
     *  Create the renderer for window.
     */
	Phobos::Misc::SDLWindowRenderer = SDL_CreateRenderer(Phobos::Misc::SDLWindow, nullptr);
    if (Phobos::Misc::SDLWindowRenderer == nullptr) {
        Debug::Log("SDLWindowRenderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    Debug::Log("SDLWindowRenderer created.\n");

    const char* driver_name = SDL_GetRendererName(Phobos::Misc::SDLWindowRenderer);
    Debug::Log("Renderer driver: %s\n", driver_name != nullptr ? driver_name : "<unknown>");

    /**
     *  Toggle VSync.
     */
	const bool vSync = false; // OptionsExtension->IsVSync ? 1 : 0
    SDL_SetRenderVSync(Phobos::Misc::SDLWindowRenderer, vSync);

    /**
     *  Set the scaling mode if specified.
     */
	SDL_ScaleMode scalemode = SDL_ScaleMode::SDL_SCALEMODE_NEAREST;//OptionsExtension->ScaleMode
    if (scalemode != SDL_SCALEMODE_INVALID) {

        SDL_SetDefaultTextureScaleMode(Phobos::Misc::SDLWindowRenderer, scalemode);
    }

    /**
     *  Create the window texture.
     */
	Phobos::Misc::SDLWindowTexture = SDL_CreateTexture(Phobos::Misc::SDLWindowRenderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (Phobos::Misc::SDLWindowTexture == nullptr) {
        Debug::Log("SDLWindowTexture could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    Debug::Log("SDLWindowTexture created.\n");

    /**
     *  Save video mode information.
     */
    GameDefinitions::VideoWidth() = width;
    GameDefinitions::VideoHeight() = height;
	GameDefinitions::VideoBitsPerPixel = bits_per_pixel;

    return true;
}


/**
 *  Resets video mode and deletes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
void SDL_Reset_Video_Mode()
{
    /**
     *  Destroy the renderer.
     */
    SDL_DestroyRenderer(Phobos::Misc::SDLWindowRenderer);
	Phobos::Misc::SDLWindowRenderer = nullptr;

    /**
     *  Deallocate the texture.
     */
    SDL_DestroyTexture(Phobos::Misc::SDLWindowTexture);
	Phobos::Misc::SDLWindowTexture = nullptr;

    /**
     *  Clear video mode information.
     */
    GameDefinitions::VideoWidth() = 0;
    GameDefinitions::VideoHeight() = 0;
	GameDefinitions::VideoBitsPerPixel = 0;
}


/**
 *  Pointer to the window procedure set by SDL.
 */
static WNDPROC SDL_Proc = nullptr;

/**
 *  Replacement window procedure for the main window.
 *
 *  @author: tomsons26, ZivDero
 */
#include <UDPInterfaceClass.h>

LRESULT CALLBACK SDL_Windows_Procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /*
    **  Scale mouse inputs before they are processed by SDL or the game.
    */
    switch (message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (SDL_Should_Scale()) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            x = static_cast<int>(x * SDL_XScale());
            y = static_cast<int>(y * SDL_YScale());

            lParam = MAKELPARAM(x, y);
        }
        break;
    default:
        break;
    }

    /*
    **  Pass on any messages intended for the winsock message handler.
    */
    if (UDPInterfaceClass::Instance()) {
        if (message == (UINT)UDPInterfaceClass::Instance->Protocol_Event_Message()) {
            if (UDPInterfaceClass::Instance->Message_Handler(hwnd, message, wParam, lParam)) {
                return DefWindowProc(hwnd, message, wParam, lParam);
            } else {
                return 0;
            }
        }
    }

    ScrollClass::Instance->Process_Mouse(hwnd, message, wParam, lParam);

    switch (message) {

        /*
        **  Refresh the window.
        */
    case WM_PAINT:
		if(!GameDefinitions::SpecialDialogTypeSet() && (Game::IsFocused() && GameOptionsClass::WindowedMode())){
			if (WWMouseClass::Instance() != nullptr && DSurface::Primary() != nullptr && DSurface::Hidden() != nullptr && DSurface::Composite() != nullptr) {
				if (Game::InScenario2() == true) {
					GameDefinitions::Update_Visible_Surface(WWMouseClass::Instance->GetField10(), DSurface::Composite(), nullptr);
					SidebarClass::Instance->Do_Blit(true);
				} else if (Is_Movie_Playing() == true) {
					Blit_Movie();
				} else {
					GameDefinitions::Update_Visible_Surface(WWMouseClass::Instance()->GetField10(), DSurface::Hidden(), nullptr);
				}
			}
		}
        /*
        **  Tell SDL that the window needs refreshing to simulate what it does itself.
        */
        SDL_Event event;
        event.type = SDL_EVENT_WINDOW_EXPOSED;
        event.window.windowID = SDL_GetWindowID(Phobos::Misc::SDLWindow);
        event.window.data1 = 0;
        event.window.data2 = 0;
        SDL_PushEvent(&event);

        /*
        **  But don't let SDL handle this event, or it will break Win32 controls' drawing.
        */
        return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_CLOSE:
        //CDControl.Unlock_All_CD_Trays();
        break;

        /*
        **  Windoze message says we have to shut down. Try and do it cleanly.
        */
    case WM_DESTROY:
        if (CCToolTip::Instance() != nullptr) {
            delete CCToolTip::Instance();
			CCToolTip::Instance = nullptr;
        }
       // CDControl.Unlock_All_CD_Trays();
        Game::hWnd() = nullptr;

        /*
        **  If we are shutting down gracefully than flag that the message loop has finished.
        **  If this is a forced shutdown (ReadyToQuit == 0) then try and close down everything
        **  before we exit.
        */
		switch (GameDefinitions::ReadyToQuitP())
		{
        default:
        case 1:
			GameDefinitions::ReadyToQuitP = 2;
            break;

        case 0:
            break;
        }
        return 0;

    case WM_ACTIVATEAPP:
        if (hwnd == Game::hWnd() && Game::IsFocused() != (wParam != 0)) {
            Game::IsFocused = wParam != 0;
            if (Game::IsFocused()) {
                Focus_Restore();

                /*
                **  Force all child controls to redraw when regaining focus.
                */
                EnumChildWindows(
                    hwnd,
                    [](HWND child, LPARAM) -> BOOL {
                        RedrawWindow(child, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
                        return TRUE;
                    },
                    0);

                RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
            } else {
                Focus_Loss();
            }
        }
        return 0;

    case WM_RBUTTONUP:

        /*
        **  Set some kind of scolling flag, perhaps "CanScroll".
        */
        ScrollClass::Instance->unknown_byte_554C = false;
        break;

    case WM_MOVING:
		On_Window_Moving(hwnd, wParam, lParam);
        return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);

    case WM_MOUSEWHEEL:
        if (!GameDefinitions::_MouseWheel()) {
			GameDefinitions::_MouseWheel = true;

            /**
             *  If we are not currently playing a scenario, no need to execute this command.
             */
            if (Game::InScenario2()) {
                if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
					Activate_Command_From_Name("SidebarDown");
                } else {
					Activate_Command_From_Name("SidebarUp");
                }
            }
			GameDefinitions::_MouseWheel = false;
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam) {

        case SC_CLOSE:
           // CDControl.Unlock_All_CD_Trays();

            /*
            **  Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
            **  pretending to handle the message and returning true;
            */
            return 0;

        case SC_SCREENSAVE:

            /*
            **  Windoze is about to start the screen saver. If we just return without passing
            **  this message to DefWindowProc then the screen saver will not be allowed to start.
            */
            return 0;

        default:
            break;
        }
        break;

    default:
        break;
    }

    /*
    **  Pass this message through to the keyboard handler.
    */
	WWKeyboardClass::Instance->MessageHandler(hwnd, message, wParam, lParam);

    return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);
}


const char *Vinifera_Get_Window_Title(DWORD dwPid)
{
    return "PhobosMinimal With SDL3 !";
}

/**
 *  Creates the main game window.
 *
 *  @author: ZivDero, CCHyper
 */
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Debug::Log("SDL_Init failed! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_PropertiesID props = SDL_CreateProperties();

    if (GameOptionsClass::WindowedMode()) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    } else {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
    }

    DWORD dwPid = GetProcessId(GetCurrentProcess());
    if (!dwPid) {
        Debug::Log("Create_Main_Window() - Failed to get the process id!\n");
        return false;
    }

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, Vinifera_Get_Window_Title(dwPid));

    /**
     *  OpenGL and Vulkan renderers need a matching graphics-capable window from the start on Windows.
     */
    //if (OptionsExtension->RendererDriver == OptionsClassExtension::RENDERER_DRIVER_OPENGL) {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    //} else
	//if (OptionsExtension->RendererDriver == OptionsClassExtension::RENDERER_DRIVER_VULKAN) {
        //SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
    //}

    /**
     *  Create the window.
     */
	Phobos::Misc::SDLWindow = SDL_CreateWindowWithProperties(props);
    if (Phobos::Misc::SDLWindow == nullptr) {
        Debug::FatalError("SDLWindow could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    Debug::Log("SDLWindow created.\n");
    
    /**
     *  Record the size that the window has been created at.
     */
    SDL_GetWindowSize(Phobos::Misc::SDLWindow, &Phobos::Misc::SDLWindowWidth, &Phobos::Misc::SDLWindowHeight);
    Debug::Log("SDLWindow size: %d X %d.\n", Phobos::Misc::SDLWindowWidth, Phobos::Misc::SDLWindowHeight);

    /**
     *  Save the window handle for the game to use.
     */
    props = SDL_GetWindowProperties(Phobos::Misc::SDLWindow);
    Game::hWnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    /**
     *  We draw Win32 child windows as part of the main window, so we need to disable clipping.
     *  Otherwise, we will see black boxes where child windows are.
     */
    LONG_PTR style = GetWindowLongPtr(Game::hWnd(), GWL_STYLE);
    style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    SetWindowLongPtr(Game::hWnd(), GWL_STYLE, style);

    /**
     *  Set the window to use our window procedure, save the one SDL set.
     */
    SDL_Proc = (WNDPROC)SetWindowLongPtr(Game::hWnd(), GWLP_WNDPROC, (LONG_PTR)SDL_Windows_Procedure);

    /**
     *  Explicitly set input focus to the window.
     */
    SDL_RaiseWindow(Phobos::Misc::SDLWindow);
    Game::IsFocused = true; // The SDL window needs this initially otherwise we need to alt-tab to gain focus.

    /**
     *  This used to happen on WM_CREATE but our proc is no longer the proc that's used when
     *  the window is created, so it never happens.
     */
    if (!CCToolTip::Instance()) {
		CCToolTip::Instance = new CCToolTip(Game::hWnd());
        if (CCToolTip::Instance()) {
			CCToolTip::Instance->SetTimerDelay(500);
        }
    }

    return true;
}

/**
 *  Creates the main window for Tiberian Sun at 480p resolution.
 *
 *  @author: ZivDero
 */
void __fastcall Create_Main_Window_480p(HINSTANCE hInstance, int command_show, int width, int height)
{
	GameDefinitions::IconPointer = LoadIconA(hInstance, (LPCSTR)93);
	SDL_Create_Main_Window(hInstance, width, height);
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Phobos::Misc::SDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)GameDefinitions::IconPointer());
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)GameDefinitions::IconPointer());
	GameDefinitions::CommandShow = command_show;
}

DEFINE_FUNCTION_JUMP(CALL , 0x6BD9CF, Create_Main_Window_480p)
/**
 *  Creates the main window for Tiberian Sun at a custom resolution.
 *
 *  @author: ZivDero
 */
void __fastcall Create_Main_Window_Custom(HINSTANCE hInstance, int command_show, int width, int height)
{
	GameDefinitions::IconPointer = LoadIconA(hInstance, (LPCSTR)93);
	SDL_Create_Main_Window(hInstance, width, height);
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Phobos::Misc::SDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)GameDefinitions::IconPointer());
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)GameDefinitions::IconPointer());
	GameDefinitions::CommandShow = command_show;
}


DEFINE_FUNCTION_JUMP(CALL, 0x6BDB0C, Create_Main_Window_Custom)
/**
 *  Destroys the main game window.
 *
 *  @author: CCHyper
 */
void SDL_Destroy_Main_Window()
{
    /**
     *  Destroy window.
     */
    SDL_DestroyWindow(Phobos::Misc::SDLWindow);
	Phobos::Misc::SDLWindow = nullptr;
}


/**
 *  Update the screen with any rendering performed since the previous call.
 *
 *  @author: ZivDero, CCHyper, tomsons26
 */
bool SDL_Update_Screen(Surface* surface)
{
    SDL_RenderClear(Phobos::Misc::SDLWindowRenderer);

    /**
     *  Blit game's surface to SDL's window surface.
     */
    if (surface) {

        /**
         *  First, update the texture with the pixels from the game's surface.
         */
        if (void* pixels = surface->Lock()) {
            SDL_UpdateTexture(Phobos::Misc::SDLWindowTexture, nullptr, pixels, surface->Get_Pitch());

            surface->Unlock();
        }

        static bool scaled = SDL_Should_Scale();

        /**
         *  Then, copy the texture to the renderer.
         */
        if (!SDL_Should_Scale()) {
			RectangleStruct src_rect = surface->Get_Rect();
            SDL_FRect dst_rect = {static_cast<float>(src_rect.X), static_cast<float>(src_rect.Y), static_cast<float>(src_rect.Width), static_cast<float>(src_rect.Height)};
            SDL_RenderTexture(Phobos::Misc::SDLWindowRenderer, Phobos::Misc::SDLWindowTexture, nullptr, &dst_rect);
        } else {
            SDL_RenderTexture(Phobos::Misc::SDLWindowRenderer, Phobos::Misc::SDLWindowTexture, nullptr, nullptr);
        }

        /**
         *  If the scale has changed, recalculate the mouse cursor image.
         */
        if (scaled != SDL_Should_Scale()) {
            scaled = SDL_Should_Scale();
            ((SDLMouseClass*)WWMouseClass::Instance())->Recalc_Cursor_Image();
        }
    }

    /**
     *  Present the image to the window.
     */
    SDL_RenderPresent(Phobos::Misc::SDLWindowRenderer);

    return true;
}


/**
 *  Returns if scaling should currently be applied.
 *  We turn off scaling when any windows dialogs are open
 *  because we cannot properly scale their input.
 *
 *  @author: ZivDero
 */
bool SDL_Should_Scale()
{
    return GameDefinitions::WSDialogCount() == 0 && GameDefinitions::SpecialDialogTypeSet() == SpecialDialogType::SDLG_NONE;
}


/**
 *  Changes the display mode to the given resolution.
 *
 *  @author: ZivDero, tomsons26
 */
bool __fastcall SDL_Change_Display_Mode(int width, int height)
{
    Debug::Log("About to set video mode\n");

	RectangleStruct old_visible_rect = DSurface::WindowBounds;
    if (!old_visible_rect.IsValid() && GameDefinitions::VideoWidth() > 0 && GameDefinitions::VideoHeight() > 0) {
        old_visible_rect = RectangleStruct(0, 0, GameDefinitions::VideoWidth(), GameDefinitions::VideoHeight());
    }

    const int old_video_width = GameDefinitions::VideoWidth();
    const int old_video_height = GameDefinitions::VideoHeight();
    const int old_video_bits_per_pixel = GameDefinitions::VideoBitsPerPixel() > 0 ? GameDefinitions::VideoBitsPerPixel() : 16;

    int old_window_x = 0;
    int old_window_y = 0;
    int old_window_width = Phobos::Misc::SDLWindowWidth;
    int old_window_height = Phobos::Misc::SDLWindowHeight;

	WWMouseClass::Instance->HideCursor();

    /**
     *  Delete the old primary surface.
     */
    if (DSurface::Primary() != nullptr) {
        Debug::Log("Deleting DSurface::Primary\n");
        delete DSurface::Primary();
        DSurface::Primary = nullptr;
    }

    /**
     *  If the window size isn't set manually, resize the window to refect the new resolution.
     */
    if (GameOptionsClass::WindowedMode()) {
        int window_width = width;
        int window_height = height;

        /**
         *  If the window size isn't set manually, resize the window to refect the new resolution.
         */
       // if (OptionsExtension->WindowWidth > 0 && OptionsExtension->WindowHeight > 0) {
        //    window_width = OptionsExtension->WindowWidth;
        //    window_height = OptionsExtension->WindowHeight;
       // }

        /**
         *  Get the current window size and position.
         */
        SDL_GetWindowPosition(Phobos::Misc::SDLWindow, &old_window_x, &old_window_y);
        SDL_GetWindowSize(Phobos::Misc::SDLWindow, &old_window_width, &old_window_height);

        /**
         *  Compute the current center point.
         */
        int center_x = old_window_x + old_window_width / 2;
        int center_y = old_window_y + old_window_height / 2;

        /**
         *  Compute new top-left corner so that the center stays the same.
         */
        int new_x = center_x - window_width / 2;
        int new_y = center_y - window_height / 2;

        /**
         *  Apply and save the new position and size.
         */
        SDL_SetWindowPosition(Phobos::Misc::SDLWindow, new_x, new_y);
        SDL_SetWindowSize(Phobos::Misc::SDLWindow, window_width, window_height);

		Phobos::Misc::SDLWindowWidth = window_width;
		Phobos::Misc::SDLWindowHeight = window_height;
        Debug::Log("SDLWindow size: %d X %d.\n", Phobos::Misc::SDLWindowWidth, Phobos::Misc::SDLWindowHeight);
    }

    /**
     *  Recreate all the SDL intermediates (texture, renderer).
     */
    if (!Set_Video_Mode(Game::hWnd(), width, height, 16)) {
        Debug::Log("Set_Video_Mode failed.\n");

        if (GameOptionsClass::WindowedMode()) {
            SDL_SetWindowPosition(Phobos::Misc::SDLWindow, old_window_x, old_window_y);
            SDL_SetWindowSize(Phobos::Misc::SDLWindow, old_window_width, old_window_height);
			Phobos::Misc::SDLWindowWidth = old_window_width;
			Phobos::Misc::SDLWindowHeight = old_window_height;
            Debug::Log("SDLWindow size restored: %d X %d.\n", Phobos::Misc::SDLWindowWidth, Phobos::Misc::SDLWindowHeight);
        }

        if (old_visible_rect.IsValid() && old_video_width > 0 && old_video_height > 0) {
            Debug::Log("Restoring previous display mode.\n");

            if (!Set_Video_Mode(Game::hWnd(), old_video_width, old_video_height, old_video_bits_per_pixel)) {
                Debug::Log("Failed to restore previous video mode.\n");
                 WWMouseClass::Instance->ShowCursor();
                return false;
            }

            SDL_Rebuild_Display_State(old_visible_rect);
        } else {
            Debug::Log("Previous display mode is invalid and cannot be restored.\n");
        }

        WWMouseClass::Instance->ShowCursor();
        return false;
    }

    /**
     *  Set the new surface resolution and reallocate the game surfaces.
     */
    SDL_Rebuild_Display_State(RectangleStruct(0, 0, width, height));
    Debug::Log("DSurface::WindowBounds: %dx%d\n", width, height);

    Debug::Log("Mode change complete.\n");

    return true;
}
