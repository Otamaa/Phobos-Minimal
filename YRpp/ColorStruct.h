#pragma once

#include <YRPPCore.h>
#include <algorithm>
#include <ASMMacros.h>
#include <Helpers/CompileTime.h>
#include <tuple>
/*
		Otamaa : 18/09/2021

		Moved to separate header due to compile error
		Add More element known from Thompson YR-IDB

*/

// used for particle ColorList
//using RGBClass = ColorStruct; // <pd> wuhaha

//static_assert(sizeof(HSVClass) == 0x3);

struct Color16Struct;
struct HSVClass;
struct ColorStruct
{
	static OPTIONALINLINE COMPILETIMEEVAL BYTE Max = 255;

	static COMPILETIMEEVAL reference<ColorStruct, 0xA80220> White {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD0> RedShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD4> RedShiftRight {};
	static COMPILETIMEEVAL reference<int, 0x8A0DE0> GreenShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DE4> GreenShiftRight {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD8> BlueShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DDC> BlueShiftRight {};

	COMPILETIMEEVAL ColorStruct() noexcept = default;

	COMPILETIMEEVAL ColorStruct(BYTE const r, BYTE const g, BYTE const b) noexcept
		: R(r), G(g), B(b)
	{ }

	COMPILETIMEEVAL ColorStruct(int const r, int const g, int const b) noexcept
		: R(0) , G(0) , B(0)
	{
		R = std::clamp<BYTE>((BYTE)r, (BYTE)0, Max);
		G = std::clamp<BYTE>((BYTE)g, (BYTE)0, Max);
		B = std::clamp<BYTE>((BYTE)b, (BYTE)0, Max);
	}

	COMPILETIMEEVAL ColorStruct(const ColorStruct& c) noexcept
		: R(c.R), G(c.G), B(c.B)
	{ }

	template<bool WordColor = false >
	COMPILETIMEEVAL ColorStruct(const int& rgb)
	{
		if COMPILETIMEEVAL (!WordColor)
		{
			R = GetRValue(rgb);
			G = GetGValue(rgb);
			B = GetBValue(rgb);
		}
		else
		{
			R = static_cast<BYTE>((static_cast<WORD>(rgb) >> RedShiftLeft()) << RedShiftRight());
			G = static_cast<BYTE>((static_cast<WORD>(rgb) >> GreenShiftLeft()) << GreenShiftRight());
			B = static_cast<BYTE>((static_cast<WORD>(rgb) >> BlueShiftLeft()) << BlueShiftRight());
		}
	}

	OPTIONALINLINE explicit ColorStruct(Color16Struct const color);
	static const ColorStruct Empty;

	explicit ColorStruct(DWORD const color)
	{ memcpy(this, &color, sizeof(ColorStruct)); }

	OPTIONALINLINE explicit ColorStruct(WORD const color);

	FORCEDINLINE COMPILETIMEEVAL bool operator == (ColorStruct const rhs) const
	{ return R == rhs.R && G == rhs.G && B == rhs.B; }

	FORCEDINLINE COMPILETIMEEVAL bool operator != (ColorStruct const rhs) const
	{ return !(*this == rhs); }

	FORCEDINLINE COMPILETIMEEVAL bool operator!() const
	{
		return (*this == ColorStruct::Empty);
	}

	FORCEDINLINE COMPILETIMEEVAL operator bool() const
	{
		return !(*this == ColorStruct::Empty);
	}

	//operator HSVClass () const JMP_THIS(0x6613A0);

	explicit operator DWORD() const
	{

		DWORD ret = 0;
		memcpy(&ret, this, sizeof(ColorStruct));
		return ret;
	}

	FORCEDINLINE DWORD Pack() const noexcept {
		return (DWORD)(*this);
	}

	OPTIONALINLINE explicit operator WORD() const;

	static FORCEDINLINE COMPILETIMEEVAL ColorStruct Interpolate(const ColorStruct* from, const ColorStruct* towards, double amount)
	{
		return {
			std::clamp<BYTE>((BYTE)(from->R * (1.0 - amount) + towards->R * amount), 0u, 255u) ,
			std::clamp<BYTE>((BYTE)(from->G * (1.0 - amount) + towards->G * amount), 0u, 255u) ,
			std::clamp<BYTE>((BYTE)(from->B * (1.0 - amount) + towards->B * amount), 0u, 255u)
		};
	}

	FORCEDINLINE COMPILETIMEEVAL ColorStruct* AdjustBrightness(const ColorStruct* towards, float amount) {
		this->R = (BYTE)std::clamp((towards->R * amount), 0.0f, 255.0f);
		this->G = (BYTE)std::clamp((towards->G * amount), 0.0f, 255.0f);
		this->B = (BYTE)std::clamp((towards->B * amount), 0.0f, 255.0f);
		return this;
	}

	FORCEDINLINE COMPILETIMEEVAL ColorStruct* Lerp(ColorStruct* lower, ColorStruct* upper, float adjust) {
		auto adj = (1.0 - adjust);
		this->R = (BYTE)std::clamp((double)(upper->R * adj) + (double)(lower->R * adj), 0.0, 255.0);
		this->G = (BYTE)std::clamp((double)(upper->G * adj) + (double)(lower->G * adj), 0.0, 255.0);
		this->B = (BYTE)std::clamp((double)(upper->B * adj) + (double)(lower->B * adj), 0.0, 255.0);
		return this;
	}

	FORCEDINLINE COMPILETIMEEVAL uintptr_t ToInit() const
	{ return ((unsigned __int8)this->R >> RedShiftRight << RedShiftLeft) | ((unsigned __int8)this->G >> GreenShiftRight << GreenShiftLeft) | ((unsigned __int8)this->B >> BlueShiftRight << BlueShiftLeft); }

	FORCEDINLINE COMPILETIMEEVAL void Adjust(int adjust, ColorStruct const& rgb)
	{
		/*
		**	Ratio conversion is limited to 0 through 100%. This is
		**	the range of 0 to 255.
		*/
		adjust &= 0x00FF;


		/*
		**	Adjust the color guns by the ratio specified toward the
		**	destination color.
		*/
		int value = (int)rgb.R - (int)R;
		R = (unsigned char)((int)R + (value * adjust) / 256);

		value = (int)rgb.G - (int)G;
		G = (unsigned char)((int)G + (value * adjust) / 256);

		value = (int)rgb.B - (int)B;
		B = (unsigned char)((int)B + (value * adjust) / 256);
	}

	FORCEDINLINE COMPILETIMEEVAL int Difference(const ColorStruct* that) const
	{
		auto red = (unsigned __int8)this->R - (unsigned __int8)that->R;
		if ( red < 0 ) {
			red = (unsigned __int8)that->R - (unsigned __int8)this->R;
		}

		auto green = (unsigned __int8)this->G - (unsigned __int8)that->G;
		if ( green < 0 ) {
			green = (unsigned __int8)that->G - (unsigned __int8)this->G;
		}

		auto blue = (unsigned __int8)this->B - (unsigned __int8)that->B;
		if ( blue < 0 ) {
			blue = (unsigned __int8)that->B - (unsigned __int8)this->B;
		}

		return 3 * blue + 2 * (red + 2 * green);
	}

	HSVClass* ConstructHSV(HSVClass* ret) const
	{ JMP_THIS(0x6613A0); }

	BYTE R, G, B;
};

struct HSVClass
{
	enum { MAX_VALUE=255 };
	static const HSVClass BlackColor;

	unsigned char Hue;
	unsigned char Saturation;
	unsigned char Value;

	operator ColorStruct (void) const
	{
		unsigned int i;				// Integer part.
		unsigned int f;				// Fractional or remainder part.  f/HSV_BASE gives fraction.
		unsigned int tmp;			// Temporary variable to help with calculations.
		unsigned int values[7];	// Possible rgb values.  Don't use zero.

		int hue = Get_Hue();
		int saturation = Get_Saturation();
		int value = Get_Value();
		int red, green, blue;


		hue *= 2;
		f = hue % 255;

		// Set up possible red, green and blue values.
		values[1] =
		values[2] = value;

		//
		// The following lines of code change
		//	values[3] = (v * (255 - ( (s * f) / 255) )) / 255;
		//	values[4] = values[5] = (v * (255 - s)) / 255;
		// values[6] = (v * (255 - (s * (255 - f)) / 255)) / 255;
		// so that the are rounded divides.
		//

		tmp = (saturation * f) / 255;
		values[3] = (value * (255 - tmp)) / 255;

		values[4] =
		values[5] = (value * (255 - saturation)) / 255;

		tmp = 255 - (saturation * (255 - f)) / 255;
		values[6] = (value * tmp) / 255;


		// This should not be rounded.
		i = hue / 255;

		i += (i > 4) ? -4 : 2;
		red = values[i];

		i += (i > 4) ? -4 : 2;
		blue = values[i];

		i += (i > 4) ? -4 : 2;
		green = values[i];

		return ColorStruct{(unsigned char)red, (unsigned char)green, (unsigned char)blue } ;
	}


	uintptr_t ToColorStructInt()
	{
		return this->operator ColorStruct().ToInit();
	}

	ColorStruct* ToColorStruct(ColorStruct* ret) {
		JMP_THIS(0x517440);
	}

	void Adjust(int ratio, HSVClass const & hsv) {
		/*
		**	Ratio conversion is limited to 0 through 100%. This is
		**	the range of 0 to 255.
		*/
		ratio &= 0x00FF;

		/*
		**	Adjust the color guns by the ratio specified toward the
		**	destination color.
		*/
		int value = hsv.Get_Value() - Get_Value();
		Value = (unsigned char)(Get_Value() + (value * ratio) / 256);

		int saturation = hsv.Get_Saturation() - Get_Saturation();
		Saturation = (unsigned char)(Get_Saturation() + (saturation * ratio) / 256);

		int hue = hsv.Get_Hue() - Get_Hue();
		Hue = (unsigned char)(Get_Hue() + (hue * ratio) / 256);
	}

	int Difference(HSVClass const & hsv) const{
		int hue = (int)Hue - (int)hsv.Hue;
		if (hue < 0) hue = -hue;

		int saturation = (int)Saturation - (int)hsv.Saturation;
		if (saturation < 0) saturation = -saturation;

		int value = (int)Value - (int)hsv.Value;
		if (value < 0) value = -value;

		return(hue*hue + saturation*saturation + value*value);
	}

	int Get_Hue(void) const {return(Hue);};
	int Get_Saturation(void) const {return(Saturation);};
	int Get_Value(void) const {return(Value);};
	void Set_Hue(unsigned char value) {Hue = value;}
	void Set_Saturation(unsigned char value) {Saturation = value;}
	void Set_Value(unsigned char value) {Value = value;}

};

#pragma pack(push, 1)
class RGBClass
{
public:
	static COMPILETIMEEVAL reference<RGBClass, 0xA80220> White {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD0> const RedShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD4> const RedShiftRight {};
	static COMPILETIMEEVAL reference<int, 0x8A0DE0> const GreenShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DE4> const GreenShiftRight {};
	static COMPILETIMEEVAL reference<int, 0x8A0DD8> const BlueShiftLeft {};
	static COMPILETIMEEVAL reference<int, 0x8A0DDC> const BlueShiftRight {};

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;

	explicit RGBClass() noexcept
		: Red { 0 }
		, Green { 0 }
		, Blue { 0 }
	{
	}

	explicit RGBClass(int r, int g, int b) noexcept
		: Red { static_cast<unsigned char>(r) }
		, Green { static_cast<unsigned char>(g) }
		, Blue { static_cast<unsigned char>(b) }
	{
	}


	explicit RGBClass(int rgb, bool wordcolor = false) noexcept
	{
		if (!wordcolor)
		{
			Red = GetRValue(rgb);
			Green = GetGValue(rgb);
			Blue = GetBValue(rgb);
		}
		else
		{
			Red = (unsigned char)((unsigned short)rgb >> RedShiftLeft) << RedShiftRight;
			Green = (unsigned char)((unsigned short)rgb >> GreenShiftLeft) << GreenShiftRight;
			Blue = (unsigned char)((unsigned short)rgb >> BlueShiftLeft) << BlueShiftRight;
		}
	}

	void Adjust(int ratio, RGBClass const& rgb)
	{
		ratio &= 0x00FF;

		int value = (int)rgb.Red - (int)Red;
		Red = static_cast<unsigned char>((int)Red + (value * ratio) / 256);

		value = (int)rgb.Green - (int)Green;
		Green = static_cast<unsigned char>((int)Green + (value * ratio) / 256);

		value = (int)rgb.Blue - (int)Blue;
		Blue = static_cast<unsigned char>((int)Blue + (value * ratio) / 256);
	}

	int Difference(RGBClass const& rgb) const
	{
		int r = (int)Red - (int)rgb.Red;
		if (r < 0) r = -r;

		int g = (int)Green - (int)rgb.Green;
		if (g < 0) g = -g;

		int b = (int)Blue - (int)rgb.Blue;
		if (b < 0) b = -b;

		return(r * r + g * g + b * b);
	}

	int ToInt()
	{
		return
			(Red >> RedShiftRight << RedShiftLeft) |
			(Green >> GreenShiftRight << GreenShiftLeft) |
			(Blue >> BlueShiftRight << BlueShiftLeft);
	}
};
#pragma pack(pop)

struct BytePalette
{
	static OPTIONALINLINE COMPILETIMEEVAL int EntriesCount = 256;

	COMPILETIMEEVAL BytePalette() noexcept :
		Entries{}
	{}

	COMPILETIMEEVAL BytePalette(const ColorStruct& rgb) :
		Entries{} {
		for (int i = 0; i < EntriesCount; ++i)
			Entries[i] = rgb;
	}

	BytePalette(const BytePalette& that) :
		Entries{} {
		std::memcpy(this, &that, sizeof(BytePalette));
	}

	COMPILETIMEEVAL ~BytePalette() = default;

	COMPILETIMEEVAL FORCEDINLINE operator const unsigned char* () const { return (const unsigned char*)&Entries[0]; }
	COMPILETIMEEVAL FORCEDINLINE operator unsigned char* () { return (unsigned char*)&Entries[0]; }

	COMPILETIMEEVAL FORCEDINLINE ColorStruct& operator [](int const idx)
	{ return this->Entries[idx]; }

	COMPILETIMEEVAL FORCEDINLINE ColorStruct const& operator [](int const idx) const
	{ return this->Entries[idx]; }

	COMPILETIMEEVAL FORCEDINLINE ColorStruct& at(int const idx)
	{ return this->Entries[idx]; }

	COMPILETIMEEVAL FORCEDINLINE ColorStruct const& at(int const idx) const
	{ return this->Entries[idx]; }

	bool operator==(const BytePalette& that) const {
		if (this == &that) return(true);
		return(memcmp(&Entries[0], &that.Entries[0], sizeof(Entries)) == 0);
	}

	bool operator!=(const BytePalette& that) const {
		return !this->operator==(that);
	}

	BytePalette& operator=(const BytePalette& that) {
		if (this == &that) return(*this);

		memcpy(&Entries[0], &that.Entries[0], sizeof(Entries));
		return(*this);
	}

	void Adjust(int ratio) {
		for (int index = 0; index < EntriesCount; index++) {
			Entries[index].Adjust(ratio, ColorStruct::Empty);
		}
	}
	

	void Adjust(int ratio, const BytePalette& palette){
		for (int index = 0; index < EntriesCount; index++) {
			Entries[index].Adjust(ratio, palette[index]);
		}
	}
	void Partial_Adjust(int ratio, char* palette){
		for (int index = 0; index < EntriesCount; index++) {
			if (palette[index]) {
				Entries[index].Adjust(ratio, ColorStruct::Empty);
			}
		}
	}
	

	void Partial_Adjust(int ratio, const BytePalette& palette, char* lut) {
		for (int index = 0; index < EntriesCount; index++) {
			if (lut[index]) {
				Entries[index].Adjust(ratio, palette[index]);
			}
		}
	}

	int Closest_Color(ColorStruct& rgb) const {
		int closest = 0;
		int value = -1;

		ColorStruct const * ptr = &Entries[0];
		for (int index = 0; index < EntriesCount; index++) {
			int difference = rgb.Difference(ptr++);
			if (value == -1 || difference < value) {
				value = difference;
				closest = index;
			}
		}
		return(closest);
	}

	COMPILETIMEEVAL auto begin() const { return std::begin(Entries); }
	COMPILETIMEEVAL auto end() const { return std::end(Entries); }
	COMPILETIMEEVAL auto begin() { return std::begin(Entries); }
	COMPILETIMEEVAL auto end() { return std::end(Entries); }

public :
	ColorStruct Entries[EntriesCount];
};

//16bit colors
#pragma pack(push, 1)
struct Color16Struct
{
	COMPILETIMEEVAL Color16Struct() = default;

	COMPILETIMEEVAL explicit Color16Struct(ColorStruct const color) :
		B(static_cast<unsigned short>(color.B >> 3u)),
		R(static_cast<unsigned short>(color.R >> 3u)),
		G(static_cast<unsigned short>(color.G >> 2u))
	{ }

	explicit Color16Struct(WORD const color)
	{ memcpy(this, &color, sizeof(Color16Struct)); }

	explicit Color16Struct(DWORD const color)
		: Color16Struct(ColorStruct(color))
	{ }

	COMPILETIMEEVAL Color16Struct(WORD const r, WORD const g, WORD const b)
		:B(b) ,R(r) , G(g)
	{ }

	COMPILETIMEEVAL FORCEDINLINE bool operator == (Color16Struct const rhs) const
	{ return R == rhs.R && G == rhs.G && B == rhs.B; }

	COMPILETIMEEVAL FORCEDINLINE bool operator != (Color16Struct const rhs) const
	{ return !(*this == rhs); }

	static const Color16Struct Empty;

	explicit operator WORD() const
	{
		WORD ret;
		memcpy(&ret, this, sizeof(Color16Struct));
		return ret;
	}

	FORCEDINLINE explicit operator DWORD() const
	{ return static_cast<DWORD>(ColorStruct(*this)); }

	uintptr_t ToInit() const
	{ JMP_THIS(0x63DAD0); }

	unsigned short B : 5;
	unsigned short R : 5;
	unsigned short G : 6;
};
#pragma pack(pop)

OPTIONALINLINE ColorStruct::ColorStruct(Color16Struct const color) :
	R(static_cast<BYTE>(color.R << 3u | color.R >> 2u)),
	G(static_cast<BYTE>(color.G << 2u | color.G >> 4u)),
	B(static_cast<BYTE>(color.B << 3u | color.B >> 2u))
{ }

OPTIONALINLINE ColorStruct::ColorStruct(WORD const color) :
	ColorStruct(Color16Struct(color))
{ }

ColorStruct::operator WORD() const
{
	return static_cast<WORD>(Color16Struct(*this));
}

struct Byte16Palette
{
	Color16Struct Entries[256];

	COMPILETIMEEVAL FORCEDINLINE Color16Struct& operator [](int const idx)
	{ return this->Entries[idx]; }

	FORCEDINLINE Color16Struct const& operator [](int const idx) const
	{ return this->Entries[idx]; }
};

static OPTIONALINLINE Color16Struct ToColor16(ColorStruct const nColor)
{
	Color16Struct nRet(nColor);
	return nRet;
}