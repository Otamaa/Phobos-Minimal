#pragma once

#include <Helpers/CompileTime.h>

#include <Point2D.h>

class RandomClass
{
protected:
	explicit RandomClass(unsigned seed) noexcept
	{
		JMP_THIS(0x65C630);
	}
public:
	operator int() { return operator()(); }

	int operator()()
	{
		JMP_THIS(0x65C640);
	}

	int operator()(int minval, int maxval)
	{
		JMP_THIS(0x65C660)
	}

	template<typename T> T operator()(T minval, T maxval) { return (T)(*this)(int(minval), int(maxval)); }

protected:
	unsigned long Seed;
};

class Random2Class
{
public:
	static constexpr reference<Random2Class, 0x886B88u> const NonCriticalRandomNumber{};
	static constexpr reference<Random2Class, 0x886B88u> const Global{}; // For backward compatibility
	static constexpr reference<double, 0x7E3570u> const INT_MAX_GAME {};
	static constexpr reference<DWORD, 0xA8ED94u> const Seed {};

protected:
	explicit Random2Class(DWORD seed) noexcept
	{ JMP_THIS(0x65C6D0); }

public:
	constexpr int Random()
	//{ JMP_THIS(0x65C780); }
	{
		if (this->unknownBool_00) {
			return 0;
		}

		this->Table[this->Index1] ^= this->Table[this->Index2];
		int Index1 = this->Index1;
		int result = this->Table[Index1++];
		int v3 = this->Index2 + 1;
		this->Index1 = Index1;
		this->Index2 = v3;
		if ( Index1 >= 250 ) {
			this->Index1 = 0;
		}

		if ( v3 >= 250 ) {
			this->Index2 = 0;
		}
		return result;
	}

	constexpr int RandomRanged(int nMin, int nMax)
	//{ JMP_THIS(0x65C7E0); }
	{
		int result = nMin;
		int v5 = nMax;
		int v6 = nMin;
		if ( nMin != nMax ) {
			if ( nMin > nMax ) {
				v6 = nMax;
				v5 = nMin;
			}

			int v7 = v5 - v6;
			int v9 = 31;
			if (v5 - v6 >= 0 ) {
				do {
					if ( v9 <= 0 ) {
						break;
					}
					--v9;
				}
				while ( ((1 << v9) & v7) == 0 );
			}

			int v10 = v7 + 1;
			int v11 = ~(-1 << (v9 + 1));
			if ( v7 + 1 > v7 && v7 + 1 != v7 ) {
				do {
					int v12 = 0;

					if(!this->unknownBool_00) {
						this->Table[this->Index1] ^= this->Table[this->Index2];
						int Index1 = this->Index1;
						int v14 = this->Table[Index1++];
						int v15 = this->Index2 + 1;
						this->Index1 = Index1;
						this->Index2 = v15;
						if ( Index1 >= 250 ) {
							this->Index1 = 0;
						}

						if ( v15 >= 250 ) {
							this->Index2 = 0;
						}

						v12 = v14;
					}
					v10 = v11 & v12;
				}
				while ( v10 > v7 );
			}

			return v6 + v10;
		}

		return result;
	}

	constexpr int RandomRanged(const Point2D& nMinMax)
	{ return RandomRanged(nMinMax.X, nMinMax.Y); }

	constexpr int operator()(const Point2D& nMinMax)
	{ return RandomRanged(nMinMax); }

	constexpr int operator()()
	{ return Random(); }

	constexpr int operator()(int nMin, int nMax)
	{ return RandomRanged(nMin, nMax); }

	/*
	*	Param :
	*	int Percent
	*	Return :
	*	True = if percent less than random 0 - 99
	*	False = if percent more than random 0 - 99
	*/
	constexpr bool PercentChance(int percent)
	{ return RandomRanged(0,99) < percent; }

	/*
	*	Param :
	*	double chance
	*	Return :
	*	True = if chanche less than RandomDouble() result
	*	False = if chance more than RandomDouble() result
	*/
	constexpr bool PercentChance(double dChance)
	{ return RandomDouble() < dChance; }

	constexpr double RandomDouble()
	{ return RandomRanged(1, INT_MAX) * 4.656612873077393e-10; }

	constexpr double RandomDouble_Closest()
	{ return RandomRanged(1, INT_MAX) * 4.656612873077393e-10 - 0.5; }

	constexpr double GameRandomDouble()
	{ return RandomRanged(1, INT_MAX) * INT_MAX_GAME(); }

	constexpr double GameRandomDouble_Closest()
	{ return RandomRanged(1, INT_MAX) * INT_MAX_GAME() - 0.5; }

	constexpr bool RandomBool()
	{ return static_cast<bool>(RandomRanged(0, 1)); }

	template<typename T> requires std::is_integral<std::underlying_type_t<T>>::value
	constexpr T RandomRangedSpecific(T nMin, T nMax) {
		return static_cast<T>(RandomRanged(static_cast<int>(nMin), static_cast<int>(nMax)));
	}

	template<typename T> requires std::is_integral<T>::value
	constexpr T RandomRangedSpecific(T nMin, T nMax) {
		return static_cast<T>(RandomRanged(static_cast<int>(nMin), static_cast<int>(nMax)));
	}
	constexpr int RandomFromMax(int nMax) {
		return RandomRanged(0, nMax);
	}

public:
	bool unknownBool_00;
	int Index1;
	int Index2;
	DWORD Table [0xFA];

};

static_assert(sizeof(Random2Class) == 0x3F4);

class Random3Class
{
protected:

	explicit Random3Class(unsigned seed1 , unsigned seed2 ) noexcept {
		JMP_THIS(0x65C890);
	}

public:
	operator int() { return operator()(); }

	int operator()()
	{
		JMP_THIS(0x65C8B0);
	}

	int operator()(int minval, int maxval)
	{
		JMP_THIS(0x65C910);
	}

	template<typename T> T operator()(T minval, T maxval) { return T(*this)(int(minval), int(maxval)); }

protected:
	int Seed;
	int Index;

private:
	static unsigned Mix1[20];
	static unsigned Mix2[20];
};
