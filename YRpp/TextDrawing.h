#pragma once

#include <ColorScheme.h>

#include <string>

#include <BitFont.h>
#include <BitText.h>

struct TextDrawing
{

	static Point2D Simple_Text_Print_Wide(
		BitFont* pFont,
		const wchar_t* text,
		Surface* surface,
		RectangleStruct* rect,
		Point2D* xy,
		COLORREF fore,
		COLORREF back,
		TextPrintType flags
	);
	
	static FORCEDINLINE Point2D Simple_Text_Print_Wide(
		const wchar_t* text,
		Surface* surface,
		RectangleStruct* rect,
		Point2D* xy,
		COLORREF fore,
		COLORREF back,
		TextPrintType flag
	)
	{
		return Simple_Text_Print_Wide(BitFont::Instance(), text, surface, rect, xy, fore, back, flag);
	}

	static FORCEDINLINE Point2D* Simple_Text_Print_Wide(Point2D* buffer,
		const wchar_t* text,
		Surface* surface,
		RectangleStruct* rect,
		Point2D* xy,
		COLORREF fore,
		COLORREF back,
		TextPrintType flag,
		bool unused
	)
	{
		*buffer = Simple_Text_Print_Wide(BitFont::Instance(), text, surface, rect, xy, fore, back, flag);
		return buffer;
	}

	static FORCEDINLINE Point2D Simple_Text_Print_Wide(
			const std::wstring& text,
			Surface* surface,
			RectangleStruct* rect,
			Point2D* xy,
			COLORREF fore,
			COLORREF back,
			TextPrintType flag
	)
	{
		return Simple_Text_Print_Wide(BitFont::Instance(), text.c_str(), surface, rect, xy, fore, back, flag);
	}

	static Point2D Plain_Text_Print_Wide(
		const wchar_t* text,
		Surface* surface,
		RectangleStruct* rect,
		Point2D* xy,
		unsigned int fore,
		unsigned int back,
		TextPrintType flag,
		int scheme);


	static Point2D Fancy_Text_Print_Wide_NoFormat(
		const wchar_t* Text, 
		Surface* Surface, 
		RectangleStruct* Bounds,
		Point2D* Location, 
		ColorScheme* fore, 
		unsigned int BackColor, 
		TextPrintType Flag);

	static FORCEDINLINE Point2D Fancy_Text_Print_Wide_NoFormat(const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
		Point2D* Location, unsigned int fore, unsigned int BackColor, TextPrintType Flag)
	{
		if (Text)
		{
			return Simple_Text_Print_Wide(Text, Surface, Bounds, Location, fore, BackColor, Flag);
		}
		else
		{
			return Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fore, BackColor, Flag);
		}
	}

	template<typename... Args>
	static Point2D Fancy_Text_Print_Wide(const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
		Point2D* Location, ColorScheme* fore, unsigned int BackColor, TextPrintType Flag, Args... args)
	{
		Point2D ret_;
		const ColorStruct fromFore = fore->BaseColor;

		if (Text)
		{
			wchar_t buffer[512];
			swprintf(buffer, std::size(buffer), Text, args...);

			ret_ = Simple_Text_Print_Wide(buffer, Surface, Bounds, Location, fromFore.ToInitGBR(), BackColor, Flag);
		}
		else
		{
			ret_ = Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fromFore.ToInitGRB(), BackColor, Flag);
		}

		return ret_;
	}

	template<size_t N , typename... Args>
	static Point2D Fancy_Text_Print_Wide_externalBuffer(wchar_t(&buffer)[N], const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
		Point2D* Location, ColorScheme* fore, unsigned int BackColor, TextPrintType Flag, Args... args)
	{
		Point2D ret_;
		const ColorStruct fromFore = fore->BaseColor;

		if (Text) {
			swprintf_s(buffer, N, Text, std::forward<Args>(args)...);

			ret_ = Simple_Text_Print_Wide(buffer, Surface, Bounds, Location, fromFore.ToInitGBR(), BackColor, Flag);
		}
		else
		{
			ret_ = Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fromFore.ToInitGRB(), BackColor, Flag);
		}

		return ret_;
	}

	template<typename... Args>
	static Point2D Fancy_Text_Print_Wide(const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
		Point2D* Location, unsigned int fore, unsigned int BackColor, TextPrintType Flag, Args... args)
	{
		if (Text)
		{
			wchar_t buffer[512];
			swprintf(buffer, std::size(buffer), Text, args...);

			return Simple_Text_Print_Wide(buffer, Surface, Bounds, Location, fore, BackColor, Flag);
		}
		else
		{
			return Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fore, BackColor, Flag);
		}
	}

	static void __fastcall Draw_Text_On_Sidebar(const wchar_t* Text, Point2D* Location, RectangleStruct* Bounds, int a5) {
		JMP_STD(0x6AC480);
	}
};
