#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <CRT.h>
#include <YRPPCore.h>

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4324)

//__declspec(align(4)) class Fixed
ALIGN(4)
class Fixed
{
	static constexpr unsigned int PRECISION = 1 << 8;
	typedef unsigned char data_type;

public:
	Fixed() {}
	Fixed(const Fixed & rvalue) { Data.Raw = rvalue.Data.Raw; }
	Fixed(int numerator, int denominator);
	Fixed(int value) { Data.Composite.Fraction = 0U; Data.Composite.Whole = (unsigned char)value; }
	Fixed(unsigned int value) { Data.Composite.Fraction = 0U; Data.Composite.Whole = (unsigned char)value; }
	Fixed(float value) { value += 1.0f / (PRECISION << 1); Data.Composite.Fraction = (unsigned char)((value - (unsigned char)value) * PRECISION); Data.Composite.Whole = (unsigned char)value; }
	Fixed(const char *ascii);

	operator unsigned() const { return(unsigned)(((unsigned __int64)Data.Raw + (PRECISION >> 1)) / PRECISION); }

	Fixed & operator *= (const Fixed & rvalue) { Data.Raw = (unsigned int)(((unsigned __int64)Data.Raw * rvalue.Data.Raw) / PRECISION); return(*this); }
	Fixed & operator *= (int rvalue) { Data.Raw *= (unsigned int)rvalue; return(*this); }
	Fixed & operator /= (const Fixed & rvalue) { if (rvalue.Data.Raw != 0U && rvalue.Data.Raw != PRECISION) Data.Raw = (unsigned int)((((unsigned __int64)Data.Raw * PRECISION) + (PRECISION >> 1)) / rvalue.Data.Raw); return(*this); }
	Fixed & operator /= (int rvalue) { if (rvalue) Data.Raw /= (unsigned int)rvalue; return(*this); }
	Fixed & operator += (const Fixed & rvalue) { Data.Raw += rvalue.Data.Raw; return(*this); }
	Fixed & operator -= (const Fixed & rvalue) { Data.Raw -= rvalue.Data.Raw; return(*this); }

	const Fixed operator * (const Fixed & rvalue) const { return Fixed(*this) *= rvalue; }
	const int operator * (int rvalue) const { return Fixed(*this) *= rvalue; }
	const Fixed operator / (const Fixed & rvalue) const { return Fixed(*this) /= rvalue; }
	const int operator / (int rvalue) const { return Fixed(*this) /= rvalue; }
	const Fixed operator + (const Fixed & rvalue) const { return Fixed(*this) += rvalue; }
	const int operator + (int rvalue) const { return Fixed(*this) += rvalue; }
	const Fixed operator - (const Fixed & rvalue) const { return Fixed(*this) -= rvalue; }
	const int operator - (int rvalue) const { return Fixed(*this) -= rvalue; }

	Fixed & operator >>= (unsigned rvalue) { Data.Raw >>= rvalue; return (*this); }
	Fixed & operator <<= (unsigned rvalue) { Data.Raw <<= rvalue; return (*this); }
	const Fixed operator >> (unsigned rvalue) const { return Fixed(*this) >>= rvalue; }
	const Fixed operator << (unsigned rvalue) const { return Fixed(*this) <<= rvalue; }

	bool operator == (const Fixed & rvalue) const { return Data.Raw == rvalue.Data.Raw; }
	bool operator != (const Fixed & rvalue) const { return Data.Raw != rvalue.Data.Raw; }
	bool operator < (const Fixed & rvalue) const { return Data.Raw < rvalue.Data.Raw; }
	bool operator > (const Fixed & rvalue) const { return Data.Raw > rvalue.Data.Raw; }
	bool operator <= (const Fixed & rvalue) const { return Data.Raw <= rvalue.Data.Raw; }
	bool operator >= (const Fixed & rvalue) const { return Data.Raw >= rvalue.Data.Raw; }
	bool operator ! () const { return Data.Raw == 0U; }

	bool operator < (int rvalue) const { return Data.Raw < ((unsigned int)rvalue*PRECISION); }
	bool operator > (int rvalue) const { return Data.Raw > ((unsigned int)rvalue*PRECISION); }
	bool operator <= (int rvalue) const { return Data.Raw <= ((unsigned int)rvalue*PRECISION); }
	bool operator >= (int rvalue) const { return Data.Raw >= ((unsigned int)rvalue*PRECISION); }
	bool operator == (int rvalue) const { return Data.Raw == ((unsigned int)rvalue*PRECISION); }
	bool operator != (int rvalue) const { return Data.Raw != ((unsigned int)rvalue*PRECISION); }

	friend const int operator * (int lvalue, const Fixed & rvalue) { return Fixed(lvalue) * rvalue; }
	friend const int operator / (int lvalue, const Fixed & rvalue) { return Fixed(lvalue) / rvalue; }
	friend const int operator + (int lvalue, const Fixed & rvalue) { return Fixed(lvalue) + rvalue; }
	friend const int operator - (int lvalue, const Fixed & rvalue) { return Fixed(lvalue) - rvalue; }
	friend bool operator < (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) < rvalue; }
	friend bool operator > (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) > rvalue; }
	friend bool operator <= (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) <= rvalue; }
	friend bool operator >= (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) >= rvalue; }
	friend bool operator == (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) == rvalue; }
	friend bool operator != (unsigned lvalue, const Fixed & rvalue) { return Fixed(lvalue) != rvalue; }
	friend int operator *= (int & lvalue, const Fixed & rvalue) { lvalue = lvalue * rvalue; return lvalue; }
	friend int operator /= (int & lvalue, const Fixed & rvalue) { lvalue = lvalue / rvalue; return lvalue; }
	friend int operator += (int & lvalue, const Fixed & rvalue) { lvalue = lvalue + rvalue; return lvalue; }
	friend int operator -= (int & lvalue, const Fixed & rvalue) { lvalue = lvalue - rvalue; return lvalue; }

	void Round_Up() { Data.Raw += (PRECISION - 1U); Data.Composite.Fraction = 0U; }
	void Round_Down() { Data.Composite.Fraction = 0U; }
	void Round() { if (Data.Composite.Fraction >= PRECISION >> 1) Round_Up(); Round_Down(); }
	void Saturate(unsigned capvalue) { if (Data.Raw > (capvalue*PRECISION)) Data.Raw = capvalue * PRECISION; }
	void Saturate(const Fixed & capvalue) { if (*this > capvalue) *this = capvalue; }
	void Sub_Saturate(unsigned capvalue) { if (Data.Raw >= (capvalue*PRECISION)) Data.Raw = (capvalue*PRECISION) - 1U; }
	void Sub_Saturate(const Fixed & capvalue) { if (*this >= capvalue) Data.Raw = capvalue.Data.Raw - 1U; }
	void Inverse() { *this = Fixed(1) / *this; }

	friend const Fixed Round_Up(const Fixed & value) { Fixed temp = value; temp.Round_Up(); return temp; }
	friend const Fixed Round_Down(const Fixed & value) { Fixed temp = value; temp.Round_Down(); return temp; }
	friend const Fixed Round(const Fixed & value) { Fixed temp = value; temp.Round(); return temp; }
	friend const Fixed Saturate(const Fixed & value, unsigned capvalue) { Fixed temp = value; temp.Saturate(capvalue); return temp; }
	friend const Fixed Saturate(const Fixed & value, const Fixed & capvalue) { Fixed temp = value; temp.Saturate(capvalue); return temp; }
	friend const Fixed Sub_Saturate(const Fixed & value, unsigned capvalue) { Fixed temp = value; temp.Sub_Saturate(capvalue); return temp; }
	friend const Fixed Sub_Saturate(const Fixed & value, const Fixed & capvalue) { Fixed temp = value; temp.Sub_Saturate(capvalue); return temp; }
	friend const Fixed Inverse(const Fixed & value) { Fixed temp = value; temp.Inverse(); return temp; }

	unsigned short Get_Raw() const { return Data.Raw; }
	void Set_Raw(unsigned short raw) { Data.Raw = raw; }

	int To_ASCII(char *buffer, int maxlen = -1) const;
	char const * As_ASCII() const;

	static const Fixed _1_2;
	static const Fixed _1_3;
	static const Fixed _1_4;
	static const Fixed _3_4;
	static const Fixed _2_3;

public:
#pragma pack(4) // Ensure union member is padded to 4 byte alignment.
	union {
		struct {
#ifdef SYSTEM_BIG_ENDIAN
			unsigned char Whole;
			unsigned char Fraction;
#else
			unsigned char Fraction;
			unsigned char Whole;
#endif
		} Composite;
		unsigned short Raw;
	} Data;
#pragma pack()
};

#pragma warning(pop)