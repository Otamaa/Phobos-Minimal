#pragma once

#include <ASMMacros.h>
#include <Helpers/CompileTime.h>

#include <RectangleStruct.h>

class BitFont;
class Surface;
class NOVTABLE BitText
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

	void Print(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, RectangleStruct InRect)
	{ this->Print(pFont, pSurface, pWideString, InRect.X, InRect.Y, InRect.Width, InRect.Height); }

	void DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int X, int Y, int W, int H, char a8, int a9, int nColorAdjust)
	{ JMP_THIS(0x434CD0); }

	void DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, RectangleStruct InRect, char a8, int a9, int nColorAdjust)
	{ this->DrawText(pFont, pSurface, pWideString, InRect.X, InRect.Y, InRect.Width, InRect.Height, a8, a9 , nColorAdjust); }
};

static_assert(sizeof(BitText) == 0x4, "Invalid size.");