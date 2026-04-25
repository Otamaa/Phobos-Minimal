#include "Body.h"

#include <BitFont.h>
#include <Drawing.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

static bool BlitSegmentWithFade(BitFont* font,
								const wchar_t*& pending, const wchar_t* end,
								int& x, int y,
								unsigned short base_color,
								int& fade_counter, int fade_end, int fade_length)
{
	int ramp_index = fade_counter - fade_end + fade_length + 1;   // v21 / v32 / v41
	int until_end = fade_end - fade_counter - 1;             // v20 / v31 / v40

	while (pending < end)
	{
		++ramp_index;
		--until_end;
		++fade_counter;

		int color = base_color;
		if (fade_end != 0)
		{
			if (fade_counter >= fade_end)
				return true;   // past the end of the fade window

			if (until_end < fade_length)
			{
				ColorStruct rgb = Drawing::Int_To_RGB(base_color);
							rgb.Adjust(ramp_index * (255 / fade_length), ColorStruct::Empty);
				
				color = rgb.ToInit();
			}
		}

		if (color == -1)
			return true;  // sentinel: stop drawing

		x = font->Blit(*pending++, x, y, color);
	}

	return false;  // drew everything requested
}

bool FakeBitText::_DrawText(BitFont* font, Surface* surface,
							   const wchar_t* text,
							   int x_origin, int y_top,
							   int max_width, int max_height,
							   char align_flags,
							   int fade_end, int fade_length)
{
	if (!font || !text)
		return false;

	font->Lock(surface);
	font->field_20 = x_origin;

	const unsigned short base_color = font->Color;     // v15 / bx
	const wchar_t* pending = text;                    // v12 (ebp)
	const wchar_t* scan = text;                    // `string` / arg_8
	int            line_width = 0;                       // v11
	int            line_char_count = 0;                       // v14
	int            y_offset = 0;                       // v48
	int            fade_counter = 0;                       // v46
	const wchar_t* last_space = nullptr;                 // v49
	int            last_space_width = 0;                       // v51
	int            y_cursor = y_top;                   // a5 (mutable copy)
	wchar_t        prev_ch = 0;                       // v13
	wchar_t        ch = *scan;                   // v50 low word

	// --- alignment offset (inlined 3× in original) ---
	auto compute_line_x = [&](int width) -> int
		{
			int off = 0;
			if (max_width > width)
			{
				if (align_flags & 1) off = (max_width - width) / 2;   // center
				else if (align_flags & 2) off = max_width - width;         // right
			}
			return x_origin + off;
		};

	// --- line reset + height-limit check (inlined 2× in original) ---
	// Returns true if the height cap is hit (caller should exit).
	auto reset_line_and_check_height = [&]() -> bool
		{
			line_width = 0;
			line_char_count = 0;
			const int line_height = font->field_1C;
			y_offset += line_height;
			if (max_height != 0 && y_offset >= max_height)
				return true;
			y_cursor += line_height;
			return false;
		};

	// Main scanner. Skipped entirely on empty input (ch==0 falls through to tail flush).
	if (ch != 0)
	{
		while (true)
		{
			switch (ch)
			{
				// ---- tab: snap to next tab stop ----
			case L'\t':
			{
				const int tab_w = font->Unknown_28;
				line_width = tab_w + line_width - (tab_w + line_width) % tab_w;
				break;  // scan-advance at bottom of loop
			}

			// ---- newline(s): flush, next line. \r\n collapses to one. ----
			case L'\n':
			case L'\r':
			{
				// \r\n / \r\r / \r+anything-of-this-pair: swallow the follower.
				if (prev_ch == L'\r')
				{
					++pending;
					break;
				}

				int x = compute_line_x(line_width);
				if (pending < scan)
				{
					if (BlitSegmentWithFade(font, pending, scan, x, y_cursor,
						base_color, fade_counter,
						fade_end, fade_length))
					{
						// LABEL_73: fade window exhausted → hard exit.
						font->UnLock(surface);
						return true;
					}
				}

				if (reset_line_and_check_height())
				{
					// LABEL_57: height cap hit.
					font->UnLock(surface);
					return true;
				}
				++pending;  // step past the \r / \n itself
				break;
			}

			// ---- space: stash as wrap candidate, then measure like any glyph ----
			case L' ':
				last_space_width = line_width;
				last_space = scan;
				[[fallthrough]];

				// ---- regular glyph (and space post-stash): measure + maybe wrap ----
			default:
			{
				auto* metric = font->GetCharacterBitmap(ch);
				if (!metric)
				{
					metric = font->GetCharacterBitmap(L'?');
					if (!metric)
						break;  // totally unknown glyph — skip without width
				}

				++line_char_count;
				const int char_w =
					*reinterpret_cast<const unsigned char*>(metric) + font->State_2C;
				line_width += char_w;

				// Still fits (or no width cap)? Just keep scanning.
				if (max_width == 0 || line_width <= max_width)
					break;

				// ---- overflow: pick a wrap point ----
				const wchar_t* wrap_at_space = last_space;  // v27 snapshot
				if (last_space != nullptr)
				{
					if (line_char_count > 1)
					{
						// Break at the last space on this line.
						line_width = last_space_width;
						scan = last_space;
					}
					// If line_char_count == 1, we're on the overflow char itself
					// with a stale last_space from a prior line (SUSPECT above).
				}
				else if (line_char_count > 1)
				{
					// No space to break at: step back one glyph, break mid-word.
					// The "stepped back" char gets redrawn on the next line via
					// pending (alignment for that line will be approximate —
					// preserved from original).
					line_width -= char_w;
					--scan;
				}
				// else: first char of line overflows — let it overhang.

				int x = compute_line_x(line_width);
				if (pending < scan)
				{
					if (BlitSegmentWithFade(font, pending, scan, x, y_cursor,
						base_color, fade_counter,
						fade_end, fade_length))
					{
						// LABEL_73
						font->UnLock(surface);
						return true;
					}
				}

				// If we wrapped AT a space, consume it so the next line
				// doesn't start with a leading space.
				if (scan == wrap_at_space)
				{
					last_space = nullptr;
					++pending;
				}

				if (reset_line_and_check_height())
				{
					// LABEL_57
					font->UnLock(surface);
					return true;
				}
				break;
			}
			}

			// LABEL_55: advance scan by one wchar.
			prev_ch = ch;
			++scan;
			ch = *scan;
			if (ch == 0)
				break;
		}
	}

	// LABEL_59: tail flush — any chars between pending and scan that weren't
	// closed off by a newline or a wrap.
	{
		int x = compute_line_x(line_width);
		if (pending < scan)
		{
			// Both "exhausted" and "completed" returns lead to the same exit.
			BlitSegmentWithFade(font, pending, scan, x, y_cursor,
								base_color, fade_counter,
								fade_end, fade_length);
		}
	}

	font->UnLock(surface);
	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x434CD0, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x430568, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x43539F	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x4353D8	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x43544B	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x435479	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x4354B8	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x47903C	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x5CD491	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x5CD560	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x62102E	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x62113F	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x6AC5B2	, FakeBitText::_DrawText);
DEFINE_FUNCTION_JUMP(CALL, 0x6C9E78	, FakeBitText::_DrawText);