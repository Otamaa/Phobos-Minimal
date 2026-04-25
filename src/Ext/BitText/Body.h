#pragma once
#include <BitText.h>

class FakeBitText final : public BitText
{
public:

	bool _DrawText(BitFont* font, Surface* surface,
							   const wchar_t* text,
							   int x_origin, int y_top,
							   int max_width, int max_height,
							   char align_flags,
							   int fade_end, int fade_length);
};

static_assert(sizeof(FakeBitText) == sizeof(BitText), "Size Missmatch!");