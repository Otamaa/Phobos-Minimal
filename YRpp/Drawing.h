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

class Blitter;
struct Drawing
{
	constexpr static reference<DynamicVectorClass<DirtyAreaStruct>, 0xB0CE78> DirtyAreas {};
	constexpr static reference<RectangleStruct, 0x886FA0u> const SurfaceDimensions_Hidden {};
	static constexpr reference<ColorStruct, 0xB0FA1Cu> const TooltipColor {};

	static constexpr reference<RGBMode, 0x8205D0> ColorMode {};
	static constexpr reference<int, 0x8A0DD0> const RedShiftLeft {};
	static constexpr reference<int, 0x8A0DD4> const RedShiftRight {};
	static constexpr reference<int, 0x8A0DE0> const GreenShiftLeft {};
	static constexpr reference<int, 0x8A0DE4> const GreenShiftRight {};
	static constexpr reference<int, 0x8A0DD8> const BlueShiftLeft {};
	static constexpr reference<int, 0x8A0DDC> const BlueShiftRight {};

	static bool __fastcall DrawSurfaces(Surface* surface1, ConvertClass* convert, Surface* surface2, RectangleStruct* a2, Point2D* point, RectangleStruct* a6, int drawerval, Blitter* a7, int z_val, int somearrayindex, int alpha_val, int Blit_Move_2_arg, int wrap_value)
	{
		JMP_STD(0x4AF2A0);
	}

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
		return static_cast<WORD>(
			(color.B >> BlueShiftRight) |
			((color.G >> 2) << GreenShiftLeft) |
			((color.R >> RedShiftRight) << 11));
	}

	static int __fastcall RGB_To_Int(BYTE red, BYTE green, BYTE blue)
	{
		// JMP_STD(0x4355D0);
		return (red >> RedShiftRight << RedShiftLeft) |
			(green >> GreenShiftRight << GreenShiftLeft) |
			(blue >> BlueShiftRight << BlueShiftLeft);
	}
	
	static int RGB_To_Int(int red, int green, int blue)
	{
		return (red >> RedShiftRight << RedShiftLeft) | (green >> GreenShiftRight << GreenShiftLeft) | (blue >> BlueShiftRight << BlueShiftLeft);
	}

	static int RGB_To_Int(const ColorStruct& Color)
	{
		return RGB_To_Int(Color.R, Color.G, Color.B);
	}

	static int RGB_To_Int(ColorStruct&& Color)
	{
		return RGB_To_Int(Color.R, Color.G, Color.B);
	}

	static void Int_To_RGB(int color, BYTE& red, BYTE& green, BYTE& blue)
	{
		red = static_cast<BYTE>(color >> RedShiftLeft << RedShiftRight);
		green = static_cast<BYTE>(color >> GreenShiftLeft << GreenShiftRight);
		blue = static_cast<BYTE>(color >> BlueShiftLeft << BlueShiftRight);
	}

	static void Int_To_RGB(int color, ColorStruct& buffer)
	{
		Int_To_RGB(color, buffer.R, buffer.G, buffer.B);
	}

	static ColorStruct Int_To_RGB(int color)
	{
		ColorStruct ret;
		Int_To_RGB(color, ret);
		return ret;
	}
	static DWORD __fastcall RGB2DWORD(int red, int green, int blue)
	{ JMP_STD(0x4355D0); }

	static DWORD RGB2DWORD(const ColorStruct Color) {
		return RGB2DWORD(Color.R, Color.G, Color.B);
	}

	static DWORD RGB2DWORD(const Color16Struct Color) {
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

	static ColorStruct RGB888_HEX(const char* pHEX)
	{
		ColorStruct res = ColorStruct();

		if (strlen(pHEX) != 6U)
			return res;

		char hexRGB[7];
		strcpy_s(hexRGB, pHEX);

		for (char& c : hexRGB)
		{
			c = static_cast<char>(tolower(c));
		}

		auto toDecimal = [](char high, char low)
		{
			return static_cast<BYTE>
				(
					(isdigit(high) ? static_cast<int>(high) - '0' : 10 + high - 'a') * 16
					+ (isdigit(low) ? static_cast<int>(low) - '0' : 10 + low - 'a')
				);
		};

		res.R = toDecimal(hexRGB[0], hexRGB[1]);
		res.G = toDecimal(hexRGB[2], hexRGB[3]);
		res.B = toDecimal(hexRGB[4], hexRGB[5]);

		return res;
	}

	static DWORD RGB888_HEX_DWORD(const char* pHEX)
	{
		return RGB2DWORD(RGB888_HEX(pHEX));
	}

	static inline constexpr ColorStruct ColorWhite = { 255 , 255 , 255 };
	static inline constexpr ColorStruct ColorRed = { 255,0,0 };
	static inline constexpr ColorStruct ColorGreen = { 0,255,0 };
	static inline constexpr ColorStruct ColorBlue = { 0,0,255 };
	static inline constexpr ColorStruct ColorGrey = { 128,128,128 };
	static inline constexpr ColorStruct ColorYellow = { 255,255,0 };

	static void __stdcall DrawLinesTo(CoordStruct nFrom, CoordStruct nTo, ColorStruct color)
	{
		JMP_STD(0x704E40);
	}

	static bool __stdcall Draw_action_lines_7049C0(CoordStruct nFrom, CoordStruct nTo, DWORD DWORD_clr, bool bDashedLine, bool a10)
	{
		JMP_STD(0x7049C0);
	}

	static bool __stdcall Draw_action_lines_7049C0(int nFrom_x , int nFrom_y , int nFrom_z, int nTo_x , int nTo_y , int nTo_z, DWORD DWORD_clr, bool bDashedLine, bool a10)
	{
		JMP_STD(0x7049C0);
	}

	static void DrawSquare(Point2D center, double range, COLORREF nColor)
	{
		int semiWidth = static_cast<int>(range * Unsorted::CellWidthInPixels);
		int semiHeight = static_cast<int>(range * Unsorted::CellHeightInPixels);

		Point2D points[4] = {
			center + Point2D{ semiWidth, 0 },
			center + Point2D{ 0, semiHeight },
			center + Point2D{ -semiWidth, 0 },
			center + Point2D{ 0, -semiHeight }
		};

		for (int i = 0; i < 4; i++) {
			DSurface::Composite->Draw_Line_Rect(DSurface::ViewBounds(), points[i], points[(i + 1) % 4], nColor);
		}
	}
};

struct BufferData
{
	int BufferPosition;
	BSurface* Surface;
	int BufferHead;
	int BufferTail;
	int BufferSize;
	int MaxValue;
	int Width;
	int Height;
};

//A few preset 16bit colors.
class NOVTABLE ABuffer
{
public:
	static constexpr reference<ABuffer*, 0x87E8A4u> const Instance {};

	ABuffer(RectangleStruct rect) { JMP_THIS(0x410CE0); }
	~ABuffer() { JMP_THIS(0x410E50); }

	int Unoffset(int pos) const { return BufferPosition - pos; }
	bool BlitTo(Surface* pSurface, int X, int Y, int Offset, int Size) { JMP_THIS(0x410DC0); }
	void ReleaseSurface() JMP_THIS(0x410E50);
	void Blitter(unsigned short* Data, int Length, unsigned short Value) { JMP_THIS(0x410E70); }
	void BlitAt(int X, int Y, COLORREF Color) { JMP_THIS(0x410ED0); }
	bool Fill(unsigned short Color) { JMP_THIS(0x4112D0); }
	bool FillRect(unsigned short Color, RectangleStruct Rect) { JMP_THIS(0x411310); }
	void BlitRect(RectangleStruct Rect) { JMP_THIS(0x411330); }
	void* GetBuffer(int X, int Y) { JMP_THIS(0x4114B0); }
	template<typename T>
	void AdjustPointer(T*& ptr) {
		if (ptr >= BufferTail)
			reinterpret_cast<char*&>(ptr) -= BufferSize;
	}
	
	BSurface* GetSurface() const { return Surface; }
	const RectangleStruct& GetArea() const { return Area; }
	unsigned int GetBufferWidth() const { return Width; }

public:
	RectangleStruct Area;
	int BufferPosition;
	BSurface* Surface;
	void* BufferHead;
	void* BufferTail;
	int BufferSize;
	int MaxValue;
	int Width;
	int Height;
};

class NOVTABLE ZBuffer
{
public:

	static constexpr reference<ZBuffer*, 0x887644u> const Instance {};

	ZBuffer(RectangleStruct rect) JMP_THIS(0x7BC970);;
	~ZBuffer() JMP_THIS(0x7BCAE0);

	int Unoffset(int pos) const { return  BufferPosition - pos; }
	bool BlitTo(Surface* pSurface, int X, int Y, int Offset, int Size) { JMP_THIS(0x7BCA50); }
	void ReleaseSurface() { JMP_THIS(0x7BCAE0); }
	void Blitter(unsigned short* Data, int Length, unsigned short Value) { JMP_THIS(0x7BCAF0); }
	void BlitAt(int X, int Y, COLORREF Color) { JMP_THIS(0x7BCB50); }
	bool Fill(unsigned short Color) { JMP_THIS(0x7BCF50); }
	bool FillRect(unsigned short Color, RectangleStruct Rect) { JMP_THIS(0x7BCF90); }
	void BlitRect(RectangleStruct Rect) { JMP_THIS(0x7BCFB0); }
	void* GetBuffer(int X, int Y) { JMP_THIS(0x7BD130); }
	template<typename T>
	void AdjustPointer(T*& ptr) {
		if (ptr >= BufferTail)
			reinterpret_cast<char*&>(ptr) -= BufferSize;
	}
	
	BSurface* GetSurface() const { return Surface; }
	const RectangleStruct& GetArea() const { return Area; }
	unsigned int GetBuffer_Width() const { return Width; }

public:
	RectangleStruct Area;
	int BufferPosition;
	BSurface* Surface;
	void* BufferHead;
	void* BufferTail;
	int BufferSize;
	int MaxValue;
	int Width;
	int Height;
};