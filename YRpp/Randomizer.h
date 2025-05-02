#pragma once

#include <Helpers/CompileTime.h>

#include <Point2D.h>
#include <RandomStruct.h>

#include <bit>
#include <array>

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
	static COMPILETIMEEVAL reference<Random2Class, 0x886B88u> const NonCriticalRandomNumber{};
	static COMPILETIMEEVAL reference<Random2Class, 0x886B88u> const Global{}; // For backward compatibility
	static COMPILETIMEEVAL reference<double, 0x7E3570u> const INT_MAX_GAME {};
	static COMPILETIMEEVAL reference<DWORD, 0xA8ED94u> const Seed {};
	static COMPILETIMEEVAL reference<DWORD, 0x839644u, 19u> FirstTable {};
	static COMPILETIMEEVAL reference<DWORD, 0x839690u, 22u> SecondTable {};

public:

	//COMPILETIMEEVAL explicit Random2Class(std::uint32_t seed) noexcept {
	//	Index1 = 0;
	//	Index2 = 103;

	//	for (size_t i = 0; i < Table.size(); ++i) {
	//		int v8 = 0;
	//		for (int a = 0; a < 4; ++a) {
	//			const int v7 = static_cast<int>(i) ^ FirstTable[a];

	//			v8 = seed ^ ((static_cast<std::uint16_t>(v7) * (v7 >> 16))
	//			+ (SecondTable[a] ^ ((((static_cast<std::uint16_t>(v7) * static_cast<std::uint16_t>(v7)
	//			+ ~(v7 >> 16) * (v7 >> 16)) << 16) |
	//			((static_cast<std::uint16_t>(v7) * static_cast<std::uint16_t>(v7)
	//			+ ~(v7 >> 16) * (v7 >> 16)) >> 16)))));
	//		}
	//		this->Table[i] = v8;
	//	}
	//}

	Random2Class(std::uint32_t seed) noexcept {
		JMP_THIS(0x65C6D0);
	}

	int Random() noexcept
	{
		JMP_THIS(0x65C780);
	}

	int RandomRanged(int min, int max) noexcept
	{
		JMP_THIS(0x65C7E0);
	}

	//[[nodiscard]] COMPILETIMEEVAL int Random() noexcept {
	//	if (unknownBool_00) return 0;

	//	Table[Index1] ^= Table[Index2];
	//	int result = Table[Index1++];

	//	if (++Index2 >= (int)Table.size()) Index2 = 0;
	//	if (Index1 >= (int)Table.size()) Index1 = 0;

	//	return result;
	//}

	//[[nodiscard]] COMPILETIMEEVAL int RandomRanged(int min, int max) noexcept {
 //       if (min == max) return min;

 //       if (min > max) std::swap(min, max);
	//	const int range = max - min;
	//	const unsigned int unsignedRange = static_cast<unsigned int>(range);

 //       int bitmask = (1 << (std::bit_width(unsignedRange) - 1)) - 1;
 //       int randomValue;

 //       do {
 //           randomValue = Table[Index1] ^= Table[Index2];

	//		if (++Index2 >= (int)Table.size()) Index2 = 0;
 //           if (++Index1 >= (int)Table.size()) Index1 = 0;

 //           randomValue &= bitmask;
 //       } while (randomValue > range);

 //       return min + randomValue;
 //   }

	[[nodiscard]] FORCEDINLINE int RandomRanged(const Point2D& nMinMax)
	{ return RandomRanged(nMinMax.X, nMinMax.Y); }

	[[nodiscard]] FORCEDINLINE int operator()(const Point2D& nMinMax)
	{ return RandomRanged(nMinMax); }

	[[nodiscard]] FORCEDINLINE int RandomRanged(const RandomStruct& nMinMax)
	{ return RandomRanged(nMinMax.Min, nMinMax.Max); }

	[[nodiscard]] FORCEDINLINE int operator()(const RandomStruct& nMinMax)
	{ return RandomRanged(nMinMax); }

	[[nodiscard]] FORCEDINLINE int operator()()
	{ return Random(); }

	[[nodiscard]] FORCEDINLINE int operator()(int nMin, int nMax)
	{ return RandomRanged(nMin, nMax); }

	/*
	*	Param :
	*	int Percent
	*	Return :
	*	True = if percent less than random 0 - 99
	*	False = if percent more than random 0 - 99
	*/
	[[nodiscard]] FORCEDINLINE bool PercentChance(int percent)
	{ return RandomRanged(0,99) < percent; }

	/*
	*	Param :
	*	double chance
	*	Return :
	*	True = if chanche less than RandomDouble() result
	*	False = if chance more than RandomDouble() result
	*/
	[[nodiscard]] FORCEDINLINE bool PercentChance(double dChance)
	{ return RandomDouble() < dChance; }

	[[nodiscard]] FORCEDINLINE double RandomDouble()
	{ return RandomRanged(1, INT_MAX) * 4.656612873077393e-10; }

	[[nodiscard]] FORCEDINLINE double RandomDouble_Closest()
	{ return RandomRanged(1, INT_MAX) * 4.656612873077393e-10 - 0.5; }

	[[nodiscard]] FORCEDINLINE double GameRandomDouble()
	{ return RandomRanged(1, INT_MAX) * INT_MAX_GAME(); }

	[[nodiscard]] FORCEDINLINE double GameRandomDouble_Closest()
	{ return RandomRanged(1, INT_MAX) * INT_MAX_GAME() - 0.5; }

	[[nodiscard]] FORCEDINLINE bool ProbabilityOf(double probability) {
		return ((Math::abs(this->Random()) % 1000000) / 1000000.0) < probability;
	}

	[[nodiscard]] FORCEDINLINE bool ProbabilityOf2(double probability) {
		return (((RandomRanged(0, INT_MAX - 1) / (double)(INT_MAX - 1))) < probability);
	}

	[[nodiscard]] FORCEDINLINE bool RandomBool()
	{ return static_cast<bool>(RandomRanged(0, 1)); }

	template<typename T> requires std::is_integral<std::underlying_type_t<T>>::value
	[[nodiscard]] FORCEDINLINE T RandomRangedSpecific(T nMin, T nMax) {
		return static_cast<T>(RandomRanged(static_cast<int>(nMin), static_cast<int>(nMax)));
	}

	template<typename T> requires std::is_integral<T>::value
	[[nodiscard]] FORCEDINLINE T RandomRangedSpecific(T nMin, T nMax) {
		return static_cast<T>(RandomRanged(static_cast<int>(nMin), static_cast<int>(nMax)));
	}
	[[nodiscard]] FORCEDINLINE int RandomFromMax(int nMax) {
		return RandomRanged(0, nMax);
	}

public:
	bool unknownBool_00;
	int Index1;
	int Index2;
	std::array<int, 250> Table;
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

class Random4Class
{
	// Period parameters
	static COMPILETIMEEVAL OPTIONALINLINE int N = 624;
	static COMPILETIMEEVAL OPTIONALINLINE int M = 397;
	static COMPILETIMEEVAL OPTIONALINLINE unsigned int MATRIX_A = 0x9908b0df; // constant vector a
	static COMPILETIMEEVAL OPTIONALINLINE unsigned int UPPER_MASK = 0x80000000; // most significant w-r bits
	static COMPILETIMEEVAL OPTIONALINLINE unsigned int LOWER_MASK = 0x7fffffff;// least significant r bits

	// Tempering parameters
	static COMPILETIMEEVAL OPTIONALINLINE unsigned int  TEMPERING_MASK_B = 0x9d2c5680;
	static COMPILETIMEEVAL OPTIONALINLINE unsigned int  TEMPERING_MASK_C = 0xefc60000;

	static COMPILETIMEEVAL unsigned int mag01[2] = { 0x0, MATRIX_A };

	template<typename T>
	static COMPILETIMEEVAL auto TEMPERING_SHIFT_U(T y) { return y >> 11; }

	template<typename T>
	static COMPILETIMEEVAL auto TEMPERING_SHIFT_S(T y) { return y << 7; }

	template<typename T>
	static COMPILETIMEEVAL auto TEMPERING_SHIFT_T(T y) { return y << 15; }

	template<typename T>
	static COMPILETIMEEVAL auto TEMPERING_SHIFT_L(T y) { return y >> 18; }

public:
	COMPILETIMEEVAL Random4Class(unsigned int seed = 4357) {
		if (!seed)
		{
			seed = 4375;
		}

		mt[0] = seed & 0xffffffff;

		for (mti = 1; mti < N; mti++)
		{
			mt[mti] = (69069 * mt[mti - 1]) & 0xffffffff;
		}
	};

	COMPILETIMEEVAL operator int() { return operator()(); }
	COMPILETIMEEVAL int operator()()
	{
		unsigned int y;

		if (mti >= N)
		{
			int kk;

			for (kk = 0; kk < N - M; kk++)
			{
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
			}
			for (; kk < N - 1; kk++)
			{
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
			}
			y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
			mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

			mti = 0;
		}

		y = mt[mti++];
		y ^= TEMPERING_SHIFT_U(y);
		y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
		y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
		y ^= TEMPERING_SHIFT_L(y);

		int* x = (int*)&y;

		return *x;
	}

	COMPILETIMEEVAL int operator()(int minval, int maxval)
	{
		return Pick_Random_Number(*this, minval, maxval);
	}

	template<typename T> T operator()(T minval, T maxval) { return T(*this)(int(minval), int(maxval)); }

	COMPILETIMEEVAL float Get_Float()
	{
		int x = (*this)();
		unsigned int* y = (unsigned int*)&x;

		return (*y) * 2.3283064370807973754314699618685e-10f;
	}

	enum class Bits
	{
		SIGNIFICANT = 32 // Random number bit significance.
	};

	template<class T>
	static COMPILETIMEEVAL int Pick_Random_Number(T& generator, int minval, int maxval) {
		if (minval == maxval) return minval;

		if (minval > maxval)
		{
			int temp = minval;
			minval = maxval;
			maxval = temp;
		}

		int magnitude = maxval - minval;
		int highbit = (int)T::Bits::SIGNIFICANT - 1;
		while ((magnitude & (1 << highbit)) == 0 && highbit > 0)
		{
			highbit--;
		}

		int mask = ~((~0L) << (highbit + 1));

		int pick = magnitude + 1;
		while (pick > magnitude)
		{
			pick = generator() & mask;
		}

		return pick + minval;
	}

protected:
	unsigned int mt[624]; // state vector
	int mti; // index
};