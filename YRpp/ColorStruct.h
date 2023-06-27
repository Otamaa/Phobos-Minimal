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
	static inline constexpr BYTE Max = 255;

	static constexpr reference<ColorStruct, 0xA80220> White {};
	static constexpr reference<int, 0x8A0DD0> RedShiftLeft {};
	static constexpr reference<int, 0x8A0DD4> RedShiftRight {};
	static constexpr reference<int, 0x8A0DE0> GreenShiftLeft {};
	static constexpr reference<int, 0x8A0DE4> GreenShiftRight {};
	static constexpr reference<int, 0x8A0DD8> BlueShiftLeft {};
	static constexpr reference<int, 0x8A0DDC> BlueShiftRight {};

	ColorStruct() = default;

	ColorStruct(BYTE const r, BYTE const g, BYTE const b)
		: R(r), G(g), B(b)
	{ }

	constexpr ColorStruct(int const r, int const g, int const b) noexcept
		: R(0) , G(0) , B(0)
	{
		R = std::clamp<BYTE>((BYTE)r, (BYTE)0, Max);
		G = std::clamp<BYTE>((BYTE)g, (BYTE)0, Max);
		B = std::clamp<BYTE>((BYTE)b, (BYTE)0, Max);
	}

	ColorStruct(const ColorStruct& c)
		: R(c.R), G(c.G), B(c.B)
	{ }

	template<bool WordColor = false >
	ColorStruct(const int& rgb)
	{
		if constexpr (!WordColor)
		{
			R = GetRValue(rgb);
			G = GetGValue(rgb);
			B = GetBValue(rgb);
		}
		else
		{
			R = static_cast<BYTE>((static_cast<WORD>(rgb) >> RedShiftLeft.get()) << RedShiftRight.get());
			G = static_cast<BYTE>((static_cast<WORD>(rgb) >> GreenShiftLeft.get()) << GreenShiftRight.get());
			B = static_cast<BYTE>((static_cast<WORD>(rgb) >> BlueShiftLeft.get()) << BlueShiftRight.get());
		}
	}

	inline explicit ColorStruct(Color16Struct const color);
	static const ColorStruct Empty;

	explicit ColorStruct(DWORD const color)
	{ memcpy(this, &color, sizeof(ColorStruct)); }

	inline explicit ColorStruct(WORD const color);

	bool operator == (ColorStruct const rhs) const
	{ return R == rhs.R && G == rhs.G && B == rhs.B; }

	bool operator != (ColorStruct const rhs) const
	{ return !(*this == rhs); }

	__forceinline bool operator!() const
	{
		return (*this == ColorStruct::Empty);
	}

	__forceinline operator bool() const
	{
		return !(*this == ColorStruct::Empty);
	}

	explicit operator DWORD() const
	{

		DWORD ret = 0;
		memcpy(&ret, this, sizeof(ColorStruct));
		return ret;
	}

	inline DWORD Pack() const noexcept {
		return (DWORD)(*this);
	}

	inline explicit operator WORD() const;

	ColorStruct* Adjust_Brightness(ColorStruct& color, float adjust)
	{ JMP_THIS(0x661190); }

	ColorStruct* Adjust_Brightness(ColorStruct* color, float adjust)
	{ JMP_THIS(0x661190); }

	static inline ColorStruct Interpolate(const ColorStruct& from, const ColorStruct& towards, float amount)
	{
		ColorStruct tmp;
		tmp.R = (BYTE)std::clamp(from.R * (1.0f - amount) + towards.R * amount, 0.0f, 255.0f);
		tmp.G = (BYTE)std::clamp(from.G * (1.0f - amount) + towards.G * amount, 0.0f, 255.0f);
		tmp.B = (BYTE)std::clamp(from.B * (1.0f - amount) + towards.B * amount, 0.0f, 255.0f);
		return tmp;
	}

	ColorStruct* AdjustBrightness(const ColorStruct* towards, float amount)
	{
		this->R = (BYTE)std::clamp((towards->R * amount), 0.0f, 255.0f);
		this->G = (BYTE)std::clamp((towards->G * amount), 0.0f, 255.0f);
		this->B = (BYTE)std::clamp((towards->B * amount), 0.0f, 255.0f);
		return this;
	}

	ColorStruct* Lerp(ColorStruct& lower, ColorStruct& upper, float adjust) const
	{ JMP_THIS(0x661020); }

	uintptr_t ToInit() const
	{ JMP_THIS(0x63DAD0); }

	void Adjust(int adjust, const ColorStruct& that)
	{ JMP_THIS(0x6612C0); }

	int Difference(const ColorStruct& that) const
	{ JMP_THIS(0x661350); }

	HSVClass* ConstructHSV(HSVClass* ret) const
	{ JMP_THIS(0x6613A0); }

	BYTE R, G, B;
};

struct HSVClass
{
	char Hue;
	char Sat;
	char Val;

	ColorStruct* ToColorStruct(ColorStruct* pResult)
	{
		JMP_THIS(0x517440);
	}
};

#pragma pack(push, 1)
class RGBClass
{
public:
	static constexpr reference<RGBClass, 0xA80220> White {};
	static constexpr reference<int, 0x8A0DD0> const RedShiftLeft {};
	static constexpr reference<int, 0x8A0DD4> const RedShiftRight {};
	static constexpr reference<int, 0x8A0DE0> const GreenShiftLeft {};
	static constexpr reference<int, 0x8A0DE4> const GreenShiftRight {};
	static constexpr reference<int, 0x8A0DD8> const BlueShiftLeft {};
	static constexpr reference<int, 0x8A0DDC> const BlueShiftRight {};

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
	static inline constexpr int EntriesCount = 256;

	BytePalette() noexcept :
		Entries{}
	{}

	BytePalette(const ColorStruct& rgb) :
		Entries{} {
		for (int i = 0; i < EntriesCount; ++i)
			Entries[i] = rgb;
	}

	BytePalette(const BytePalette& that) :
		Entries{} {
		std::memcpy(this, &that, sizeof(BytePalette));
	}

	~BytePalette() = default;

	operator const unsigned char* () const { return (const unsigned char*)&Entries[0]; }
	operator unsigned char* () { return (unsigned char*)&Entries[0]; }

	ColorStruct& operator [](int const idx)
	{ return this->Entries[idx]; }

	ColorStruct const& operator [](int const idx) const
	{ return this->Entries[idx]; }

	ColorStruct& at(int const idx)
	{ return this->Entries[idx]; }

	ColorStruct const& at(int const idx) const
	{ return this->Entries[idx]; }

	bool operator==(const BytePalette& that) const { return std::memcmp(Entries, that.Entries, sizeof(Entries)) == 0; }
	bool operator!=(const BytePalette& that) const { return std::memcmp(Entries, that.Entries, sizeof(Entries)) != 0; }
	BytePalette& operator=(const BytePalette& that) {
		JMP_THIS(0x6260D0);
	}

	void Adjust(int ratio) {
		JMP_THIS(0x6260F0);
	}

	void Adjust(int ratio, const BytePalette& palette) {
		JMP_THIS(0x626120)
	}

	void Partial_Adjust(int ratio, char* palette) {
		JMP_THIS(0x626170);
	}

	void Partial_Adjust(int ratio, const BytePalette& palette, char* lut) {
		JMP_THIS(0x6261B0);
	}

	int Closest_Color(ColorStruct& rgb) const {
		JMP_THIS(0x626200);
	}

	constexpr auto begin() const { return std::begin(Entries); }
	constexpr auto end() const { return std::end(Entries); }
	constexpr auto begin() { return std::begin(Entries); }
	constexpr auto end() { return std::end(Entries); }

public :
	ColorStruct Entries[EntriesCount];
};

//16bit colors
#pragma pack(push, 1)
struct Color16Struct
{
	Color16Struct() = default;

	explicit Color16Struct(ColorStruct const color) :
		B(static_cast<unsigned short>(color.B >> 3u)),
		R(static_cast<unsigned short>(color.R >> 3u)),
		G(static_cast<unsigned short>(color.G >> 2u))
	{ }

	explicit Color16Struct(WORD const color)
	{ memcpy(this, &color, sizeof(Color16Struct)); }

	explicit Color16Struct(DWORD const color)
		: Color16Struct(ColorStruct(color))
	{ }

	Color16Struct(WORD const r, WORD const g, WORD const b)
		:B(b) ,R(r) , G(g)
	{ }

	bool operator == (Color16Struct const rhs) const
	{ return R == rhs.R && G == rhs.G && B == rhs.B; }

	bool operator != (Color16Struct const rhs) const
	{ return !(*this == rhs); }

	static const Color16Struct Empty;

	explicit operator WORD() const
	{
		WORD ret;
		memcpy(&ret, this, sizeof(Color16Struct));
		return ret;
	}

	explicit operator DWORD() const
	{ return static_cast<DWORD>(ColorStruct(*this)); }

	uintptr_t ToInit() const
	{ JMP_THIS(0x63DAD0); }

	unsigned short B : 5;
	unsigned short R : 5;
	unsigned short G : 6;
};
#pragma pack(pop)

inline ColorStruct::ColorStruct(Color16Struct const color) :
	R(static_cast<BYTE>(color.R << 3u | color.R >> 2u)),
	G(static_cast<BYTE>(color.G << 2u | color.G >> 4u)),
	B(static_cast<BYTE>(color.B << 3u | color.B >> 2u))
{ }

inline ColorStruct::ColorStruct(WORD const color) :
	ColorStruct(Color16Struct(color))
{ }

ColorStruct::operator WORD() const
{
	return static_cast<WORD>(Color16Struct(*this));
}

struct Byte16Palette
{
	Color16Struct Entries[256];

	Color16Struct& operator [](int const idx)
	{ return this->Entries[idx]; }

	Color16Struct const& operator [](int const idx) const
	{ return this->Entries[idx]; }
};

static inline Color16Struct ToColor16(ColorStruct const nColor)
{
	Color16Struct nRet(nColor);
	return nRet;
}