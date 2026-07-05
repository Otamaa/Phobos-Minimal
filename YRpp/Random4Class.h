#pragma once

#include <array>
#include <Base/Always.h>

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
	COMPILETIMEEVAL Random4Class(unsigned int seed = 4357)
	{
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

	template<typename T> T operator()(T minval, T maxval) { return (T)(*this)(int(minval), int(maxval)); }

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
	static COMPILETIMEEVAL int Pick_Random_Number(T& generator, int minval, int maxval)
	{
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