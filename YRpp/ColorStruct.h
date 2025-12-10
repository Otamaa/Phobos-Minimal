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

struct HSVClass;
#pragma pack(push, 1)
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

	//template<bool WordColor = false >
	//COMPILETIMEEVAL ColorStruct(const int& rgb)
	//{
	//	if COMPILETIMEEVAL(!WordColor)
	//	{
	//		R = GetRValue(rgb);
	//		G = GetGValue(rgb);
	//		B = GetBValue(rgb);
	//	}
	//	else
	//	{
	//		R = static_cast<BYTE>((static_cast<WORD>(rgb) >> RedShiftLeft()) << RedShiftRight());
	//		G = static_cast<BYTE>((static_cast<WORD>(rgb) >> GreenShiftLeft()) << GreenShiftRight());
	//		B = static_cast<BYTE>((static_cast<WORD>(rgb) >> BlueShiftLeft()) << BlueShiftRight());
	//	}
	//}

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

	//explicit operator DWORD() const
	//{
	//
	//	DWORD ret = 0;
	//	memcpy(&ret, this, sizeof(ColorStruct));
	//	return ret;
	//}

	FORCEDINLINE DWORD Pack() const noexcept {
		return (DWORD)(*this);
	}

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

	//RGB
	FORCEDINLINE COMPILETIMEEVAL uintptr_t ToInit() const
	{
		return
			((uint32_t(this->R) >> RedShiftRight) << RedShiftLeft) |
			((uint32_t(this->G) >> GreenShiftRight) << GreenShiftLeft) |
			((uint32_t(this->B) >> BlueShiftRight) << BlueShiftLeft);
	}

	//GBR
	FORCEDINLINE COMPILETIMEEVAL uintptr_t ToInitGBR() const
	{
		return
			((uint32_t(this->G) >> GreenShiftRight) << GreenShiftLeft) |
			((uint32_t(this->B) >> BlueShiftRight) << BlueShiftLeft) |
			((uint32_t(this->R) >> RedShiftRight) << RedShiftLeft);
	}

	//GRB
	FORCEDINLINE COMPILETIMEEVAL uintptr_t ToInitGRB() const
	{
		return
			((uint32_t(this->G) >> GreenShiftRight) << GreenShiftLeft) |
			((uint32_t(this->R) >> RedShiftRight) << RedShiftLeft) |
			((uint32_t(this->B) >> BlueShiftRight) << BlueShiftLeft);
	}

	//BGR 16bit
	FORCEDINLINE COMPILETIMEEVAL uint16_t ToInitBGR() const
	{
		return
			((uint16_t(this->B) >> BlueShiftRight) << 0) |      // B: 5 bits
			((uint16_t(this->G) >> GreenShiftLeft) << 5) |      // G: 6 bits
			((uint16_t(this->R) >> RedShiftRight) << 11);       // R: 5 bits
	}

	//Color16Struct int
	FORCEDINLINE COMPILETIMEEVAL uintptr_t ToRGB16_Quantized() const {
		const ColorStruct rr(
			(static_cast<BYTE>(R << 3u | R >> 2u)),
			(static_cast<BYTE>(G << 2u | G >> 4u)),
			(static_cast<BYTE>(B << 3u | B >> 2u)));

		return rr.ToInit();
	}

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

public:

	BYTE R, G, B;
};
#pragma pack(pop)

struct HSVClass
{
	enum { MAX_VALUE=255 };
	static const HSVClass BlackColor;

	unsigned char Hue;
	unsigned char Saturation;
	unsigned char Value;

	// H (Hue): 0-255 maps to 0-360 degrees (multiplied by 6/255 internally)
	// S (Saturation): 0-255
	// V (Value/Brightness): 0-255
	operator ColorStruct (void) const
	{
		int hue = this->Hue;
		int sat = this->Saturation;
		int val = this->Value;

		// Scale hue: hue * 6 (to get 6 sectors of the color wheel)
		// Original uses hue * 3 * 2 = hue * 6
		int scaledHue = hue * 6;

		// Fractional part of hue within sector (0-255 range)
		int hueFraction = scaledHue % 255;

		// Sector index (0-5, but can exceed due to scaling)
		int sector = scaledHue / 255;

		// Pre-calculate common color components:
		// p = val * (255 - sat) / 255              -- minimum brightness
		// q = val * (255 - sat * fraction) / 255   -- descending
		// t = val * (255 - sat * (255 - fraction)) / 255  -- ascending

		int p = (val * (255 - sat)) / 255;
		int q = (val * (255 - (sat * hueFraction) / 255)) / 255;
		int t = (val * (255 - (sat * (255 - hueFraction)) / 255)) / 255;

		// Build lookup table for RGB values based on sector
		// values[1] = val (V)
		// values[2] = val (V)  
		// values[3] = q (descending)
		// values[4] = p (minimum)
		// values[5] = p (minimum)
		// values[6] = t (ascending)
		int values[7];
		values[1] = val;  // V
		values[2] = val;  // V
		values[3] = q;    // Descending brightness
		values[4] = p;    // Minimum (desaturated)
		values[5] = p;    // Minimum (desaturated)
		values[6] = t;    // Ascending brightness

		// Calculate indices for R, G, B based on sector
		// The pattern cycles through the 6 sectors with wraparound at 4
		// Sector 0: R=V, G=t, B=p  (Red to Yellow)
		// Sector 1: R=q, G=V, B=p  (Yellow to Green)
		// Sector 2: R=p, G=V, B=t  (Green to Cyan)
		// Sector 3: R=p, G=q, B=V  (Cyan to Blue)
		// Sector 4: R=t, G=p, B=V  (Blue to Magenta)
		// Sector 5: R=V, G=p, B=q  (Magenta to Red)

		// Wraparound logic: if index > 4, subtract 6 (add -4 then +2 = -4, or just +2)
		// This implements circular indexing through the values array

		unsigned int redIndex = sector;
		if (redIndex > 4)
			redIndex -= 4;  // Wrap around (equivalent to: sector > 4 ? sector - 4 : sector + 2, but simplified)
		else
			redIndex += 2;

		unsigned int greenIndex = redIndex;
		if (greenIndex > 4)
			greenIndex -= 4;
		else
			greenIndex += 2;

		unsigned int blueIndex = greenIndex;
		if (blueIndex > 4)
			blueIndex -= 4;
		else
			blueIndex += 2;

		return ColorStruct{ (unsigned char)values[redIndex]
			, (unsigned char)values[blueIndex]
			, (unsigned char)values[greenIndex] 
		} ;
	}

	uintptr_t ToColorStructInt() {
		return this->operator ColorStruct().ToInit();
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
