/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SDL Mouse class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <Lib/SDL3/SDL_mouse.h>
#include <Surface.h>

#include <mmsystem.h>
#include <vector>

#include <FileFormats/SHP.h>

class SDLSurface;
/*
**  This class manages the "mouse cursor". It presumes the mouse behaves in the traditional
**  manner, but requires more manual management than a traditional mouse.
**
**  The mouse interface is designed with the following requirements:
**
**  1> The interface (coordinate system) must be consistent with respect to the game user.
**     This means that coordinate 0,0 is the upper left pixel of the drawable client area.
**
**  2> It must support arbitrary mouse cursor artwork size and hotspot positioning. Mouse shape
**     animation should be a simple process of just changing the mouse shape.
**
**  3> The mouse must be able to break free of the game constraints where necessary in order
**     to interface with the operating system. The transition should be easy to manage.
**
**  4> The game mouse "active" region may be a subset rectangle of the normal visible surface.
**     This bounding requirement should be transparent to system's functionality.
**
**  The system assumes that the sub-rectangle that binds the mouse to the visible surface will
**  exactly match the dimensions of any hidden surface that the mouse may have occasion to be
**  drawn upon.
*/
class Mouse
{
public:
	virtual ~Mouse() {}

	/*
	**  Sets the game-drawn mouse imagery.
	*/
	virtual void Set_Cursor(Point2D const& hotspot, SHPStruct* cursor, int shape) = 0;

	/*
	**  Controls visibility of the game-drawn mouse.
	*/
	virtual bool Is_Hidden() const = 0;
	virtual void Hide_Mouse() = 0;
	virtual void Show_Mouse() = 0;

	/*
	**  Takes control of and releases control of the mouse with
	**  respect to the operating system. The mouse must be released
	**  during operations with the operating system. When the mouse is
	**  relased, it may move outside of the confining rectangle and its
	**  shape is controlled by the operating sytem.
	*/
	virtual void Release_Mouse() = 0;
	virtual void Capture_Mouse() = 0;
	virtual bool Is_Captured() const = 0;

	/*
	**  Hide the mouse if it falls within this game screen region.
	*/
	virtual void Conditional_Hide_Mouse(RectangleStruct region) = 0;
	virtual void Conditional_Show_Mouse() = 0;

	/*
	**  Query about the mouse visiblity state and location. If the mouse
	**  state is zero or greater, then the mouse is visible.
	*/
	virtual int Get_Mouse_State() const = 0;
	virtual int Get_Mouse_X() const = 0;
	virtual int Get_Mouse_Y() const = 0;
	virtual Point2D Get_Mouse_Point() const = 0;
	virtual void Set_Mouse_Point(Point2D XY) = 0;
	/*
	** The following two routines can be used to render the mouse onto an alternate
	**  surface.
	*/
	virtual void Draw_Mouse(Surface* scr, bool issidebarsurface = false) = 0;
	virtual void Erase_Mouse(Surface* scr, bool issidebarsurface = false) = 0;

	/*
	**  Converts O/S screen coordinates into game coordinates.
	*/
	virtual void Convert_Coordinate(int& x, int& y) const = 0;
};

/*
**  Handles the mouse as it relates to the C&C game engine. It is expected that only
**  one object of this type will be created during the lifetime of the game.
*/
class SDLMouseClass : public Mouse
{
public:
    /*
    **  Constructor/destructor.
    */
    SDLMouseClass();
    virtual ~SDLMouseClass() override;

    /*
    **  Maintenance callback routine.
    */
    void Process_Mouse();

    /*
    **  Sets the game-drawn mouse imagery.
    */
    void Set_Cursor(Point2D const& hotspot, SHPStruct* cursor, int shape) override;

    /*
    **  Controls visibility of the game-drawn mouse.
    */
    bool Is_Hidden() const override { return Get_Mouse_State() < 0; }
    void Hide_Mouse() override;
    void Show_Mouse() override;

    /*
    **  Confines or releases the mouse cursor from the window rect.
    **  Only operates in full screen mode.
    */
    void Release_Mouse() override;
    void Capture_Mouse() override;
    bool Is_Captured() const override { return IsCaptured; }

    /*
    **  Hide the mouse if it falls within this game screen region.
    */
    void Conditional_Hide_Mouse(RectangleStruct region) override;
    void Conditional_Show_Mouse() override;

    /*
    **  Query about the mouse visiblity state and location.
    */
    int Get_Mouse_State() const override;
    int Get_Mouse_X() const override { return MouseX; }
    int Get_Mouse_Y() const override { return MouseY; }
    Point2D Get_Mouse_Point() const override { return Point2D(MouseX, MouseY); }
	void Set_Mouse_Point(Point2D XY) override{
		this->MouseX = XY.X;
		this->MouseY = XY.Y;
	}
    /*
    **  The following two routines would render the mouse onto a surface.
    **  However, we now use a hardware cursor, so these are no-ops.
    */
    void Draw_Mouse(Surface* = nullptr, bool = false) override {}
    void Erase_Mouse(Surface* = nullptr, bool = false) override {}

    /*
    **  Would convert O/S coordinates to game coordinates.
    **  However, SDL uses client coordinates directly, so this is a no-op.
    */
    void Convert_Coordinate(int& x, int& y) const override {}

    /*
    **  Recalculates the cursor's image using the same shape.
    */
    void Recalc_Cursor_Image();

private:
    /*
    **  This specifies the mouse shape data. It records the shape set
    **  data as well as the particular image contained within.
    */
	SHPStruct* MouseShape;
    int ShapeNumber;

    /*
    **  This vector contains pointers to SDL_Surfaces that contain
    **  the converted shape frames.
    */
    std::vector<SDL_Surface*> CursorSurfaces;

    /*
    **  The hotspot for the currently used cursor image.
    */
    Point2D Hotspot;

    /*
    **  The currently used cursor.
    */
    SDL_Cursor* Cursor;

    /*
    **  If the mouse is being managed by this class (for the game), then this flag
    **  will be true. When the mouse has been released to be managed by the operating
    **  system, this flag will be false. However, this class will still track the mouse
    **  position.
    */
    bool IsCaptured;

    /*
    **  This is the last recorded mouse position that it was drawn to.
    */
    int MouseX;
    int MouseY;

    /*
    **  Maintenance timer handle.
    */
    MMRESULT TimerHandle;

    /*
    **  Various private utility routines.
    */
    void Update_Mouse_Position(int x, int y);
    void Delete_Cursor_Image();
    void Convert_Cursor_Image(SHPStruct* shapes);
    void Replace_Cursor(SDL_Cursor* cursor);
    void Set_System_Cursor();

    static int Get_Cursor_Scale();
};

void CALLBACK SDL_Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD);
