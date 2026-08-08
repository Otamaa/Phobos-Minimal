/*
	Converts are palettes... AFAIK
*/

#pragma once

#include <AbstractClass.h>
#include <ArrayClasses.h>
#include <Surface.h>

#include <FileFormats/SHP.h>
#include <Helpers/CompileTime.h>

class Blitter;
class RLEBlitter;
struct ColorStruct;
class DSurface;

class NOVTABLE ConvertClass
{
public:
	enum BPP : int {
		One = 1, Two
	};

	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E5358;
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<ConvertClass*>, 0x89ECF8u> const Array {};
	static COMPILETIMEEVAL reference<ConvertClass*, 0x7B93F0u> const MouseConvert {};

	static void __fastcall CreateFromFile(const char* pal_filename, BytePalette*& pPalette, ConvertClass*& pDestination)
	  { JMP_FAST(0x72ADE0); }

	static ConvertClass* CreateFromFile(const char* pal_filename);

	virtual ~ConvertClass() JMP_THIS(0x491210);

	void Alloc_Blitters() JMP_THIS(0x48EBF0);
	void Dealloc_Blitters() JMP_THIS(0x490490);

	Blitter* Select_Blitter(BlitterFlags flags) JMP_THIS(0x490B90);

	RLEBlitter* Select_RLE_Blitter(BlitterFlags  flags) JMP_THIS(0x490E50);

	static void Recalc_Color_Remap_Tables(int a1, int a2, int a3, int a4) JMP_THIS(0x491100);

	OPTIONALINLINE unsigned inline_01(unsigned index)
	{
		if (this->BytesPerPixel == BPP::One)
			return reinterpret_cast<uint8_t*>(ShadeTables)[index];

		return reinterpret_cast<uint16_t*>(ShadeTables)[index];
	}

	OPTIONALINLINE unsigned inline_02(unsigned index)
	{
		if  (this->BytesPerPixel == BPP::One)
			return reinterpret_cast<uint8_t*>(NormalShadeTable)[index];

		return reinterpret_cast<uint16_t*>(NormalShadeTable)[index];
	}

	void* SelectProperBlitter(SHPCaches* SHP, int FrameIndex, BlitterFlags flags) {
		return (SHP->HasCompression(FrameIndex))
			? static_cast<void*>(this->Select_Blitter(flags))
			: static_cast<void*>(this->Select_RLE_Blitter(flags))
			;
	}

	ConvertClass(
	BytePalette const& palette,
	BytePalette const& eightbitpalette, //???
	DSurface* pSurface,
	size_t shadeCount,
	bool skipBlitters) : ConvertClass(noinit_t()) {
		JMP_THIS(0x48E740);
	}

	ConvertClass(
	BytePalette const* palette,
	BytePalette const* eightbitpalette, //???
	DSurface* pSurface,
	size_t shadeCount,
	bool skipBlitters) : ConvertClass(noinit_t())
	{
		JMP_THIS(0x48E740);
	}

	void* GetPptrFromPad()
	{
		return reinterpret_cast<void*>((static_cast<uint32_t>(_HalfColorMask) << 16) | _QuarterColorMask);
	}

	void CleanPad()
	{
		_HalfColorMask = 0u;
		_QuarterColorMask = 0u;
	}

	void SetPadToPtr(void* ptr)
	{
		// Cleanup first
		CleanPad();

		uint32_t full_address = reinterpret_cast<uint32_t>(ptr);
		// Extract the top 16 bits
		_HalfColorMask = static_cast<uint16_t>(full_address >> 16);
		// Extract the bottom 16 bits
		_QuarterColorMask = static_cast<uint16_t>(full_address & 0xFFFF);
	}

protected:
	explicit __forceinline ConvertClass(noinit_t) {
		//VTable::Set(this,0x7E5358);
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================
public:
	union{
		BPP BytesPerPixel;
		int _BytesPerPixel;
	};
	Blitter* Blitters[50];
	RLEBlitter* RLEBlitters[39];
	int ShadeCount; //16C
	void* ShadeTables; //170, new(ShadeCount * 8 * BytesPerPixel) - gets filled with palette values on CTOR
	void* NormalShadeTable; //174, points to the middle of BufferA above, ??
	char* DarkenTable; //178, if(BytesPerPixel == 1) { BufferB = new byte[0x100]; }
	DWORD CurrentZRemap; //17C, set right before drawing
	uint16_t HalfColorMask; //180, for masking colors right-shifted by 1
	uint16_t _HalfColorMask;
	uint16_t QuarterColorMask; //184, for masking colors right-shifted by 2
	uint16_t _QuarterColorMask;
};

static_assert(sizeof(ConvertClass) == 0x188);

class NOVTABLE LightConvertClass : public ConvertClass
{
public:

	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<LightConvertClass*>, 0x87F698u> const Array{};

	//Destructor
	virtual ~LightConvertClass() RX;

	virtual void UpdateColors(int red, int green, int blue, bool tinted) final
		{ JMP_THIS(0x556090); }

	static LightConvertClass* __fastcall InitLightConvert(int red, int green, int blue)
		{ JMP_FAST(0x544E70); }

	//Constructor
	LightConvertClass(
		BytePalette* palette1,
		BytePalette* palette2,
		Surface* pSurface,
		int color_R,
		int color_G,
		int color_B,
		bool skipBlitters,
		char* pBuffer, // allowed to be null
		int shadeCount) : ConvertClass(noinit_t())
	{ JMP_THIS(0x555DA0); }

public:

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================
	BytePalette const* UsedPalette1;
	BytePalette const* UsedPalette2;
	BYTE* IndexesToIgnore;
	int RefCount;
	DECLARE_PROPERTY(TintStruct , Color1);
	DECLARE_PROPERTY(TintStruct , Color2);
	bool Tinted;
	PROTECTED_PROPERTY(BYTE, align_1B1[3]);
};

static_assert(sizeof(LightConvertClass) == 0x1B4);

struct UninitConvert
{
	void operator() (ConvertClass* pConvert) const
	{
		GameDelete<true, true>(pConvert);
		pConvert = nullptr;
	}
};
