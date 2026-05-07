/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SDL Surface class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include <Base/Always.h>
#include <Utilities/Debug.h>

#include "sdlsurface.h"

/**
 *  The pixel format of the SDL surfaces created.
 */
const SDL_PixelFormatDetails* SDLSurface::PixelFormat = nullptr;


/**
 *  Struct used to create GDI DIB sections.
 */
struct BitmapInfo
{
    BITMAPINFOHEADER Header;
    DWORD Masks[3];
};


/**
 *  SDLSurface constructor.
 *
 *  @author: ZivDero
 */
SDLSurface::SDLSurface(int width, int height) :
    DSurface(), // use the default constructor so that we don't initialize the DDraw portions of the surface
    SDLSurfacePtr(nullptr),
    GDIDC(nullptr),
    GDIBitmap(nullptr),
    GDIBuffer(nullptr),
    Pitch(0)
{
    /**
     *  If this is our first surface, fetch the pixel format.
     */
    if (!PixelFormat) {
        PixelFormat = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGB565);
        if (!PixelFormat) {
            Debug::Log("Failed to get pixel format details for RGB565.\n");
            return;
        }
    }

    /**
     *  Create persistent memory DC and DIB section.
     */
    GDIDC = CreateCompatibleDC(nullptr);
    if (!GDIDC) {
		Debug::Log("CreateCompatibleDC failed\n");
        return;
    }

    BitmapInfo bmi = {};
    bmi.Header.biSize = sizeof(BITMAPINFOHEADER);
    bmi.Header.biWidth = width;
    bmi.Header.biHeight = -height;
    bmi.Header.biPlanes = 1;
    bmi.Header.biBitCount = PixelFormat->bits_per_pixel;
    bmi.Header.biCompression = BI_BITFIELDS;
    bmi.Masks[0] = PixelFormat->Rmask;
    bmi.Masks[1] = PixelFormat->Gmask;
    bmi.Masks[2] = PixelFormat->Bmask;

    /**
     *  Create DIB section (let GDI allocate memory).
     */
    GDIBitmap = CreateDIBSection(GDIDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &GDIBuffer, nullptr, 0);
    if (!GDIBitmap || !GDIBuffer) {
        Debug::Log("CreateDIBSection failed! Error = %lu\n", GetLastError());
        DeleteDC(GDIDC);
        GDIDC = nullptr;
        return;
    }

    SelectObject(GDIDC, GDIBitmap);

    DIBSECTION ds = {};
    GetObject(GDIBitmap, sizeof(ds), &ds);
    Pitch = ds.dsBm.bmWidthBytes;

    /**
     *  Create an SDL surface wrapping GDIBuffer.
     */
    SDLSurfacePtr = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGB565, GDIBuffer, Pitch);
    if (SDLSurfacePtr == nullptr) {
        Debug::Log("SurfacePtr could not be created! SDL Error: %s\n", SDL_GetError());
        return;
    }

    /**
     *  Set surface properties.
     */
    BytesPerPixel = PixelFormat->bytes_per_pixel;
    Width = SDLSurfacePtr->w;
    Height = SDLSurfacePtr->h;
}


/**
 *  SDLSurface destructor.
 *
 *  @author: ZivDero
 */
SDLSurface::~SDLSurface()
{
    if (SDLSurfacePtr) {
        SDL_DestroySurface(SDLSurfacePtr);
        SDLSurfacePtr = nullptr;
    }
    if (GDIBitmap) {
        DeleteObject(GDIBitmap);
        GDIBitmap = nullptr;
    }
    if (GDIDC) {
        DeleteDC(GDIDC);
        GDIDC = nullptr;
    }
}


/**
 *  Calculate bit shifts to properly extract channel data.
 *
 *  @author: ZivDero, tomsons26
 */
static void Calculate_Mask_Info(unsigned int mask, unsigned int& right, unsigned int& left)
{
    right = 0;
    left = 0;

    /**
     *  Figure out how far to shift bits to the left.
     */
    for (int index = 0; index < 16; index++) {
        if (mask & 0x01) break;
        mask >>= 1;
        right++;
    }

    /**
     *  Figure out how far to shift bits to the right.
     */
    for (int index = 0; index < 8; index++) {
        if (mask & 0x80) break;
        mask <<= 1;
        left++;
    }
}


/**
 *  With DSurface, this would create the primary (visible) surface.
 *  There is no such thing with SDL, but we take this opportunity to
 *  initialize some static variables used for color conversions.
 *
 *  @author: ZivDero, tomsons26
 */
SDLSurface* SDLSurface::Create_Primary(void*)
{
    Debug::Log("SDLSurface::Create_Primary\n");

    AllowStretchBlits = true;
    AllowHardwareBlitFills = false;

    Debug::Log("SDLSurface::Create_Primary - Creating surface\n");
    SDLSurface* surface = new SDLSurface(GameDefinitions::VideoWidth(), GameDefinitions::VideoHeight());

    /**
     *  If this is a hicolor surface, then build the shift values for
     *  building and extracting the colors from the hicolor pixel.
     */
    if (DSurface::RGBPixelFormat() == COLORMODE_INVALID) {
        Calculate_Mask_Info(PixelFormat->Rmask, RedRight, RedLeft);
        Calculate_Mask_Info(PixelFormat->Gmask, GreenRight, GreenLeft);
        Calculate_Mask_Info(PixelFormat->Bmask, BlueRight, BlueLeft);

        /**
         *  Create the halfbright mask.
         */
        HalfbrightMask = static_cast<unsigned short>(RGB_To_Pixel(127, 127, 127));
        QuarterbrightMask = static_cast<unsigned short>(RGB_To_Pixel(63, 63, 63));
        EighthbrightMask = static_cast<unsigned short>(RGB_To_Pixel(31, 31, 31));

        if (BlueRight() == 0 && BlueLeft() == 3 && GreenRight() == 5 && GreenLeft() == 3 && RedRight() == 10 && RedLeft() == 3) {
			DSurface::RGBPixelFormat = COLORMODE_555;
        } else if (BlueRight() == 0 && BlueLeft() == 2 && GreenRight() == 6 && GreenLeft() == 3 && RedRight() == 11 && RedLeft() == 3) {
			DSurface::RGBPixelFormat = COLORMODE_556;
        } else if (BlueRight() == 0 && BlueLeft() == 3 && GreenRight() == 5 && GreenLeft() == 2 && RedRight() == 11 && RedLeft() == 3) {
			DSurface::RGBPixelFormat = COLORMODE_565;
        } else if (BlueRight() == 0 && BlueLeft() == 3 && GreenRight() == 5 && GreenLeft() == 3 && RedRight() == 11 && RedLeft() == 2) {
			DSurface::RGBPixelFormat = COLORMODE_655;
        }
    }
    Debug::Log("SDLSurface::Create_Primary done\n");

    return surface;
}


/**
 *  Blit from one surface to this one.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Copy_From(RectangleStruct& dcliprect, RectangleStruct& destrect, Surface* ssource, RectangleStruct& scliprect, RectangleStruct& sourcerect, bool trans, bool)
{
    if (!dcliprect.IsValid() || !scliprect.IsValid() || !destrect.IsValid() || !sourcerect.IsValid()) return false;

    bool use_xsurface = false;

    /**
     *  For non-SDL surfaces, or if a trans blit is requested, let vanilla
     *  blitters handle the blit.
     */
    if (!ssource->IsDSurface() == true || trans == true) {
        use_xsurface = true;
    }

    if (use_xsurface == true) {
        return XSurface::Copy_From(destrect, ssource, sourcerect, trans, true);
    }

	RectangleStruct drect = destrect;
	RectangleStruct srect = sourcerect;

	RectangleStruct swindow = scliprect.IntersectWith(ssource->Get_Rect());
	RectangleStruct dwindow = dcliprect.IntersectWith(this->Get_Rect());

    if (!Game::func_007BBE20(&drect, &dwindow, &srect, &swindow)) {
        return false;
    }

    SDL_Surface* src_surf = static_cast<SDLSurface const*>(ssource)->Get_SDL_Surface();
    SDL_Surface* dst_surf = this->Get_SDL_Surface();

    if (!src_surf || !dst_surf) {
        return false;
    }

    SDL_Rect src {srect.X + swindow.X, srect.Y + swindow.Y, srect.Width, srect.Height};
    SDL_Rect dst {drect.X + dwindow.X, drect.Y + dwindow.Y, drect.Width, drect.Height};

    SDL_SetSurfaceBlendMode(src_surf, SDL_BLENDMODE_NONE);
    return SDL_BlitSurfaceScaled(src_surf, &src, dst_surf, &dst, SDL_SCALEMODE_LINEAR);
}


/**
 *  This routine will fill the specified rectangle.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Fill_Rect(RectangleStruct& fillrect, unsigned color)
{
	auto rect = this->Get_Rect();
    return SDLSurface::Fill_Rect(rect, fillrect, color);
}


/**
 *  Fills a rectangle with clipping control.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Fill_Rect(RectangleStruct& cliprect, RectangleStruct& fillrect, unsigned color)
{
    if (SDLSurfacePtr == nullptr || !fillrect.IsValid()) return false;

    /**
     *  Ensure that the clipping rectangle is legal.
     */
    RectangleStruct crect = cliprect.IntersectWith(this->Get_Rect());

    /**
     *  Bias the fill rect to the clipping rectangle.
     */
	RectangleStruct frect = fillrect.Bias_To(cliprect);

    /**
     *  Find the region that should be filled after being clipped by the
     *  clipping rectangle. This could result in no fill operation being performed
     *  if the desired fill rectangle has been completely clipped away.
     */
    frect = frect.IntersectWith(crect);
    if (!frect.IsValid()) return false;

    SDL_Rect rect;
    rect.x = frect.X;
    rect.y = frect.Y;
    rect.w = frect.Width;
    rect.h = frect.Height;

    return SDL_FillSurfaceRect(SDLSurfacePtr, &rect, color);
}


/**
 *  Get the windows device context from our surface.
 *
 *  @author: ZivDero
 */
HDC SDLSurface::GetDC()
{
    if (GDIDC == nullptr) {
        return nullptr;
    }

    LockLevel++;
    return GDIDC;
}


/**
 *  Release the windows device context from our surface.
 *
 *  @author: ZivDero
 */
int SDLSurface::ReleaseDC(HDC hdc)
{
    if (!GDIDC || hdc != GDIDC) {
        return 0;
    }

    if (LockLevel > 0) {
        LockLevel--;
    }

    return 1;
}


/**
 *  Fetches the bytes between rows.
 *
 *  @author: ZivDero
 */
int SDLSurface::Get_Pitch() const
{
    return Pitch;
}


/**
 *  Fetches a working pointer into surface memory.
 *
 *  @author: ZivDero, tomsons26
 */
void* SDLSurface::Lock(int x , int y)
{
    if (x < 0 || y < 0) return nullptr;

    if (LockLevel == 0) {
        if (SDL_MUSTLOCK(SDLSurfacePtr)) {
            if (!SDL_LockSurface(SDLSurfacePtr)) {
                return nullptr; // failed to lock
            }
        }
        BufferPtr = SDLSurfacePtr->pixels;
    }
    XSurface::Lock();
    return static_cast<char*>(BufferPtr) + y * Get_Pitch() + x * Get_Bytes_Per_Pixel();
}


/**
 *  Returns if the surface can be locked.
 *
 *  @author: ZivDero
 */
bool SDLSurface::Can_Lock(int x, int y) const
{
    return SDLSurfacePtr != nullptr;
}


/**
 *  Returns if the surface can be blitted to.
 *
 *  @author: ZivDero
 */
bool SDLSurface::Can_Blit() const
{
    return SDLSurfacePtr != nullptr;
}


/**
 *  Unlock a previously locked surface.
 *
 *  @author: ZivDero
 */
bool SDLSurface::Unlock()
{
    if (LockLevel > 0) {
        XSurface::Unlock();
        if (LockLevel == 0) {
            if (SDL_MUSTLOCK(SDLSurfacePtr)) {
                SDL_UnlockSurface(SDLSurfacePtr);
            }
			BufferPtr = nullptr;
        }
        return true;
    }
    return false;
}
