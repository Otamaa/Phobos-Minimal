#pragma once
#include <BitFont.h>

class FakeBitFont final : public BitFont
{
public:

	int _Draw(const wchar_t* text, int xLeft, int yTop, int max_chars, int fade_count);
};

static_assert(sizeof(FakeBitFont) == sizeof(BitFont), "Size Missmatch!");