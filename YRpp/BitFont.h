#pragma once

#include <YRPP.h>

class NOVTABLE BitFont
{
public:
	static COMPILETIMEEVAL reference<BitFont*, 0x89C4D0> Instance {};

private:
	BitFont(const char* pFileName) { JMP_THIS(0x433880); }
public:
	virtual ~BitFont() RX;

	bool GetTextDimension(const wchar_t* pText, int* pWidth, int* pHeight, int nMaxWidth) { JMP_THIS(0x433CF0); }
	int Blit(wchar_t wch, int X, int Y, int nColor) { JMP_THIS(0x434120); }

	bool Lock(Surface* pSurface) { JMP_THIS(0x4348F0); }
	bool UnLock(Surface* pSurface) { JMP_THIS(0x434990); }
	unsigned char* GetCharacterBitmap(wchar_t wch) { JMP_THIS(0x4346C0); }

	COMPILETIMEEVAL FORCEDINLINE void SetBounds(LTRBStruct* pBound)
	{
		if (pBound)
			this->Bounds = *pBound;
		else
			this->Bounds = { 0,0,0,0 };
	}

	COMPILETIMEEVAL FORCEDINLINE void SetBounds_Rect(RectangleStruct* pBound)
	{
		if (pBound)
		{
			auto nTemp = *pBound;
			this->Bounds = { nTemp.X , nTemp.Y ,nTemp.Width ,nTemp.Height };
		}
		else
			this->Bounds = { 0,0,0,0 };
	}

	COMPILETIMEEVAL FORCEDINLINE void SetColor(WORD nColor)
	{
		this->Color = nColor;
	}

	COMPILETIMEEVAL FORCEDINLINE void SetField20(int x)
	{
		this->field_20 = x;
	}

	COMPILETIMEEVAL FORCEDINLINE void SetField41(char flag)
	{
		this->field_41 = flag;
	}

	/// Properties
	struct InternalData
	{
		int FontWidth;
		int Stride;
		int FontHeight;
		int Lines;
		int Count;
		int SymbolDataSize;
		short* SymbolTable;
		char* Bitmaps;
		int ValidSymbolCount;
	};

	static InternalData* __fastcall LoadInternalData(const char* pFileName)
		{ JMP_STD(0x433990); }

	int CharPixelWidth(wchar_t wChar)
		{ JMP_THIS(0x4349B0); }

	int StringPixelWidth(wchar_t wChar, int nMaximum)
		{ JMP_THIS(0x433ED0); }

	void SetSpacing(int nSpacing)
		{ JMP_THIS(0x434100); }

	int GetWidth(bool bHalf = false)
		{ return bHalf ? InternalPTR->FontWidth / 2 : InternalPTR->FontWidth; }

	int GetHeight(bool bHalf = false)
		{ return bHalf ? InternalPTR->FontHeight / 2 : InternalPTR->FontHeight; }

	static BitFont* __fastcall BitFontPtr(TextPrintType nType)
		{ JMP_STD(0x4A60D0); }

	static void __fastcall Print(void* pThis, BitFont* font, Surface* surface, wchar_t* string, int xLeft, int yTop, int a6, int a7)
		{ JMP_STD(0x434B90); }

	InternalData* InternalPTR;
	void* Pointer_8;
	short* pGraphBuffer;
	int PitchDiv2;
	int Unknown_14;
	wchar_t* field_18;
	int field_1C;
	int field_20;
	WORD Color;
	short DefaultColor2;
	int Unknown_28;
	int State_2C;
	LTRBStruct Bounds;
	bool Bool_40;
	bool field_41;
	bool field_42;
	bool field_43;
};

static_assert(sizeof(BitFont) == 0x44, "Invalid size.");
