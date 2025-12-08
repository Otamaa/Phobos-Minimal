
#include <Surface.h>

#include <ColorScheme.h>

#include <BitFont.h>
#include <BitText.h>

#include <string>
#include <GeneralDefinitions.h>

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
	)
	{
		int ypos = rect->Y + xy->Y;
		int xpos = xy->X + rect->X;
		int width {};
		int heignt {};
		pFont->GetTextDimension(text, &width, &heignt, rect->Width);

		if (flags & TextPrintType::Center)
		{
			xpos -= width / 2;
		}
		else if (flags & TextPrintType::Right)
		{
			xpos -= width;
		}

		RectangleStruct fill_rect {};
		if (flags & TextPrintType::TransparentBackgoround)
		{
			fill_rect.Width = width + 2;
			fill_rect.Y = ypos - 1;
			fill_rect.Height = heignt + 2;
			fill_rect.X = xpos - 1;

			ColorStruct trans {};
			surface->Fill_Rect_Trans(&fill_rect, &trans, 60);
		}
		else if (flags & TextPrintType::Background)
		{
			fill_rect.Width = width + 2;
			fill_rect.Height = heignt + 2;
			fill_rect.Y = ypos - 1;
			fill_rect.X = xpos - 1;
			surface->Fill_Rect(fill_rect, (unsigned int)0u);
		}

		RectangleStruct textrect {
			rect->X,
			rect->Y,
			rect->Width + rect->X,
			rect->Y + rect->Height
		};

		pFont->SetByte41(true);
		pFont->SetBounds_Rect(&textrect);
		pFont->SetColor(fore);
		BitText::Instance->Print(pFont, surface, text, xpos, ypos, 0, 0);
		return { xpos  , ypos };
	}

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
	int scheme)
	{
		if (scheme <= -1 || scheme >= ColorScheme::Array->Count)
		{
			return Simple_Text_Print_Wide(BitFont::Instance(), text, surface, rect, xy, fore, back, flag);
		}
		else
		{
			const auto fromFore = ColorScheme::Array->Items[scheme]->BaseColor.ToColorStructInt();
			return Simple_Text_Print_Wide(BitFont::Instance(), text, surface, rect, xy, fromFore, back, flag);
		}
	}


	static Point2D Fancy_Text_Print_Wide_NoFormat(const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
		Point2D* Location, ColorScheme* fore, unsigned int BackColor, TextPrintType Flag)
	{
		Point2D ret_;
		const auto fromFore = fore->BaseColor.ToColorStructInt();

		if (Text)
		{
			ret_ = Simple_Text_Print_Wide(Text, Surface, Bounds, Location, fromFore, BackColor, Flag);
		}
		else
		{
			ret_ = Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fromFore, BackColor, Flag);
		}

		return ret_;
	}

	static Point2D Fancy_Text_Print_Wide_NoFormat(const wchar_t* Text, Surface* Surface, RectangleStruct* Bounds,
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
		const auto fromFore = fore->BaseColor.ToColorStructInt();

		if (Text)
		{
			wchar_t buffer[512];
			swprintf(buffer, std::size(buffer), Text, args...);

			ret_ = Simple_Text_Print_Wide(buffer, Surface, Bounds, Location, fromFore, BackColor, Flag);
		}
		else
		{
			ret_ = Simple_Text_Print_Wide(L"", Surface, Bounds, Location, fromFore, BackColor, Flag);
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
};
