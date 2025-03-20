#pragma once

#include <ASMMacros.h>
#include <Helpers/CompileTime.h>

class BitFont;
class Surface;
class BitText
{
public:
	static COMPILETIMEEVAL reference<BitText*, 0x89C4B8> Instance {};

private:
	BitText() { JMP_THIS(0x434AD0); }
public:
	virtual ~BitText() RX;

	// Seems like draw in a single line
	void Print(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int X, int Y, int W, int H)
	{ JMP_THIS(0x434B90); }

	void DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int X, int Y, int W, int H, char a8, int a9, int nColorAdjust)
	{ JMP_THIS(0x434CD0); }
};

static_assert(sizeof(BitText) == 0x4, "Invalid size.");