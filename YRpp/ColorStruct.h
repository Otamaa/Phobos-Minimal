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

struct HSVClass
{
	char Hue;
	char Sat;
	char Val;

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(Hue, Sat, Val);
	//}

};

//static_assert(sizeof(HSVClass) == 0x3);

struct Color16Struct;
struct ColorStruct
{
	static inline constexpr int Max = 255;

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
		: R(static_cast<BYTE>(std::clamp(r, 0, Max)))
		, G(static_cast<BYTE>(std::clamp(g, 0, Max)))
		, B(static_cast<BYTE>(std::clamp(b, 0, Max)))
	{ }

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

	explicit operator DWORD() const
	{
		DWORD ret = 0;
		memcpy(&ret, this, sizeof(ColorStruct));
		return ret;
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

typedef ColorStruct RGBClass;

struct BytePalette
{
	static inline constexpr int EntryMax = 256;

	BytePalette(const ColorStruct& rgb = ColorStruct(0, 0, 0)) {
		JMP_THIS(0x626020);
	}

	BytePalette(const BytePalette& that) {
		JMP_THIS(0x626070);
	}

	~BytePalette() = default;

	operator const unsigned char* () const { return (const unsigned char*)&Entries[0]; }
	operator unsigned char* () { return (unsigned char*)&Entries[0]; }

	ColorStruct& operator [](int const idx)
	{ return this->Entries[idx]; }

	ColorStruct const& operator [](int const idx) const
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

	auto begin() const { return std::begin(Entries); }
	auto end() const { return std::end(Entries); }
	auto begin() { return std::begin(Entries); }
	auto end() { return std::end(Entries); }

public :
	ColorStruct Entries[EntryMax];
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