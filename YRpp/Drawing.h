/*
	Drawing related static class.
	This provides access to the game's Surfaces, color value conversion
	and text aligning helpers.
*/

#pragma once

#include <ColorScheme.h>
#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <Surface.h>

struct DirtyAreaStruct
{
	RectangleStruct Rect;
	bool alphabool10;

	bool operator==(const DirtyAreaStruct& another) const
	{
		return
			Rect.X == another.Rect.X &&
			Rect.Y == another.Rect.Y &&
			Rect.Width == another.Rect.Width &&
			Rect.Height == another.Rect.Height &&
			alphabool10 == another.alphabool10;
	};
};

namespace Drawing
{
	constexpr static reference<DynamicVectorClass<DirtyAreaStruct>, 0xB0CE78> DirtyAreas {};
	constexpr static reference<RectangleStruct, 0x886FA0u> const SurfaceDimensions_Hidden {};
	constexpr static reference<ColorStruct, 0xB0FA1Cu> const TooltipColor {};

	//TextBox dimensions for tooltip-style boxes
	static RectangleStruct GetTextBox(const wchar_t* pText, int nX, int nY, DWORD flags, int nMarginX, int nMarginY)
		{
			RectangleStruct box;
			RectangleStruct* p_box=&box;

			PUSH_VAR32(nMarginY);		//X Margin
			PUSH_VAR32(nMarginX);		//Y Margin - should add 2, because X margin adds to 2 internally!
			PUSH_VAR32(flags);
			PUSH_VAR32(nY);
			PUSH_VAR32(nX);
			SET_REG32(edx,pText);
			SET_REG32(ecx,p_box);
			CALL(0x4A59E0);

			return box;
		}

	static RectangleStruct* __fastcall GetTextDimensions(
		RectangleStruct* pOutBuffer, wchar_t const* pText, Point2D location,
		WORD flags, int marginX = 0, int marginY = 0)
			{ JMP_STD(0x4A59E0); }

	static RectangleStruct* __fastcall GetTextDimensions(
		RectangleStruct* pOutBuffer, wchar_t const* pText, Point2D location,
		TextPrintType flags, int marginX = 0, int marginY = 0)
		{ JMP_STD(0x4A59E0); }

	static RectangleStruct GetTextBox(const wchar_t* pText, int nX, int nY, int nMargin)
		{ return GetTextBox(pText, nX, nY, 0, nMargin + 2, nMargin); }

	static RectangleStruct GetTextBox(const wchar_t* pText, int nX, int nY)
		{ return GetTextBox(pText, nX, nY, 2); }

	static RectangleStruct GetTextBox(const wchar_t* pText, Point2D* pPoint)
		{ return GetTextBox(pText, pPoint->X, pPoint->Y, 2); }

	static RectangleStruct GetTextBox(const wchar_t* pText, Point2D* pPoint, int nMargin)
		{ return GetTextBox(pText, pPoint->X, pPoint->Y, nMargin); }

	//TextDimensions for text aligning
	static RectangleStruct GetTextDimensions(const wchar_t* pText)
		{
			RectangleStruct dim=GetTextBox(pText,0,0,0);

			dim.X=0;
			dim.Y=0;
			dim.Width-=4;
			dim.Height-=2;

			return dim;
		}

	static RectangleStruct __fastcall GetTextDimensions(
		wchar_t const* pText, Point2D location, WORD flags, int marginX = 0,
		int marginY = 0)
	{
		RectangleStruct buffer;
		GetTextDimensions(&buffer, pText, location, flags, marginX, marginY);
		return buffer;
	}

	// Rectangles
	static RectangleStruct Intersect(RectangleStruct* rect1, RectangleStruct* rect2, int* delta_left, int* delta_top)
	{
		RectangleStruct box;
		RectangleStruct* p_box = &box;

		PUSH_VAR32(delta_top);
		PUSH_VAR32(delta_left);
		PUSH_VAR32(rect2);
		SET_REG32(edx, rect1);
		SET_REG32(ecx, p_box);
		CALL(0x421B60);

		return box;
	}

	static RectangleStruct* __fastcall Intersect(
		RectangleStruct* pOutBuffer, RectangleStruct const& rect1,
		RectangleStruct const& rect2, int* delta_left = nullptr,
		int* delta_top = nullptr)
			{ JMP_STD(0x421B60); }

	static RectangleStruct __fastcall Intersect(
		RectangleStruct const& rect1, RectangleStruct const& rect2,
		int* delta_left = nullptr, int* delta_top = nullptr)
	{
		RectangleStruct buffer;
		Intersect(&buffer, rect1, rect2, delta_left, delta_top);
		return buffer;
	}

	// Rect1 will be changed, notice that - secsome
	static RectangleStruct* __fastcall Union(
		RectangleStruct* pOutBuffer,
		const RectangleStruct& rect1,
		const RectangleStruct& rect2)
			{ JMP_STD(0x487F40); }

	// Rect1 will be changed, notice that - secsome
	static RectangleStruct __fastcall Union(
		const RectangleStruct& rect1,
		const RectangleStruct& rect2)
	{
		RectangleStruct buffer;
		Union(&buffer, rect1, rect2);
		return buffer;
	}

	// Converts an RGB color to a 16bit color value.
	static WORD Color16bit(const ColorStruct& color)
	{
		return static_cast<WORD>((color.B >> 3) | ((color.G >> 2) << 5) | ((color.R >> 3) << 11));
	}

	static DWORD __fastcall RGB2DWORD(int red, int green, int blue)
	{ JMP_STD(0x4355D0); }

	static DWORD RGB2DWORD(const ColorStruct Color)
	{
		return RGB2DWORD(Color.R, Color.G, Color.B);
	}

	// Converts a 16bit color to an RGB color.
	static ColorStruct WordColor(WORD bits) {
		ColorStruct color;
		color.R = static_cast<BYTE>(((bits & 0xF800) >> 11) << 3);
		color.G = static_cast<BYTE>(((bits & 0x07E0) >> 5) << 2);
		color.B = static_cast<BYTE>((bits & 0x001F) << 3);
		return color;
	}

	/** Message is a vswprintf format specifier, ... is for any arguments needed */
	static Point2D * __cdecl PrintUnicode(Point2D *Position1, wchar_t *Message, Surface *a3, RectangleStruct *Rect, Point2D *Position2,
			ColorScheme *a6, int a7, int a8, ...)
		{ JMP_STD(0x4A61C0); };
}

//A few preset 16bit colors.
class ABuffer
{
public:
	static constexpr reference<ABuffer*, 0x87E8A4u> const Instance {};

	ABuffer(RectangleStruct rect)
	{
		JMP_THIS(0x410CE0);
	}

	int Unoffset(int pos) const { return BufferOffset - pos; }
	void Copy_To(BSurface* surface, int x, int y, int w, int h) JMP_THIS(0x410DC0);
	void Set(unsigned int dst, int size, unsigned value) JMP_THIS(0x410E70);
	void Pan(int x_speed, int y_speed, unsigned value) JMP_THIS(0x410ED0);
	bool Fill(unsigned value)  JMP_THIS(0x4112D0);
	bool Fill(unsigned value, RectangleStruct a2) JMP_THIS(0x411310);
	void Update(int x, int y, int w, int h) JMP_THIS(0x411330);
	unsigned int Get_Buffer_Offset(Point2D pos) JMP_THIS(0x4114B0);
	BSurface* Get_Surface() const { return SurfacePtr; }
	const RectangleStruct& Get_Area() const { return Area; }
	unsigned int Get_Buffer_Width() const { return BufferWidth; }

public:
	RectangleStruct Area;
	unsigned int SurfaceOffset;
	BSurface* SurfacePtr;
	unsigned int BufferStart;
	unsigned int BufferEnd;
	unsigned int BufferSize;
	unsigned int BufferOffset;
	unsigned int BufferWidth;
	unsigned int BufferHeight;
};

class ZBuffer
{
public:

	static constexpr reference<ZBuffer*, 0x887644u> const Instance {};

	ZBuffer(RectangleStruct rect) JMP_THIS(0x7BC970);;
	~ZBuffer() JMP_THIS(0x7BCAE0);

	int Unoffset(int pos) const { return BufferOffset - pos; }
	void Copy_To(BSurface* surface, int x, int y, int w, int h) JMP_THIS(0x7BCA50);
	void Set(unsigned int dst, int size, unsigned value) JMP_THIS(0x7BCAF0);
	void Pan(int x_speed, int y_speed, unsigned value) JMP_THIS(0x7BCB50);
	bool Fill(unsigned value) JMP_THIS(0x7BCF50);
	bool Fill(unsigned value, RectangleStruct a2) JMP_THIS(0x7BCF90);
	void Update(int x, int y, int w, int h) JMP_THIS(0x7BCFB0);
	unsigned int Get_Buffer_Offset(Point2D pos) JMP_THIS(0x7BD130);
	BSurface* Get_Surface() const { return SurfacePtr; }
	const RectangleStruct& Get_Area() const { return Area; }
	unsigned int Get_Buffer_Width() const { return BufferWidth; }

public:
	RectangleStruct Area;
	unsigned int SurfaceOffset;
	BSurface* SurfacePtr;
	unsigned int BufferStart;
	unsigned int BufferEnd;
	unsigned int BufferSize;
	unsigned int BufferOffset;
	unsigned int BufferWidth;
	unsigned int BufferHeight;
};