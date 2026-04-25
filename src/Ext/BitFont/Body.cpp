#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <Drawing.h>

int FakeBitFont::_Draw(const wchar_t* text, int xLeft, int yTop, int max_chars, int fade_count)
{
	const wchar_t* cursor = text;
	wchar_t ch = *cursor++;
	const unsigned short saved_color = this->Color;   // v10 / bx

	// Empty string — nothing to draw.
	if (ch == 0)
	{
		this->Color = saved_color;
		return xLeft;
	}

	int fade_counter = 0;                         // v18 — chars consumed in fade window
	int fade_steps_left = fade_count - 1;            // v19 — gates color adjust while < 8
	int adjust_ratio = 31 * (9 - fade_count);     // v12 — RGBClass::Adjust ratio

	while (true)
	{
		// -------- Fade step (only when fade_count > 0) --------
		if (fade_count != 0)
		{
			// Fade window exhausted → restore color, measure tail, return.
			if (fade_counter >= fade_count)
			{
				this->Color = saved_color;
				int remaining_width = 0;
				this->GetTextDimension(cursor, &remaining_width, 0, 0);
				return xLeft + remaining_width;
			}

			// Apply ramped color for the first 8 fade steps only.
			if (fade_steps_left < 8)
			{
				ColorStruct rgb = Drawing::Int_To_RGB(saved_color);
							rgb.Adjust(adjust_ratio, ColorStruct::Empty);
				this->Color = rgb.ToInit();
			}

			adjust_ratio += 31;
			++fade_counter;
			--fade_steps_left;
		}

		// -------- Draw glyph (skip CR/LF) --------
		if (ch != L'\r' && ch != L'\n')
		{
			xLeft = this->Blit(ch, xLeft, yTop, -1);
		}

		// -------- Bounded-count early out --------
		if (max_chars != 0 && --max_chars == 0)
			break;

		// -------- Advance to next char; stop at terminator --------
		ch = *cursor++;
		if (ch == 0)
			break;
	}

	this->Color = saved_color;
	return xLeft;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x434500, FakeBitFont::_Draw);
DEFINE_FUNCTION_JUMP(CALL, 0x434BCA, FakeBitFont::_Draw);
DEFINE_FUNCTION_JUMP(CALL, 0x434C40, FakeBitFont::_Draw);
DEFINE_FUNCTION_JUMP(CALL, 0x434C6E, FakeBitFont::_Draw);
DEFINE_FUNCTION_JUMP(CALL, 0x434CB9, FakeBitFont::_Draw);