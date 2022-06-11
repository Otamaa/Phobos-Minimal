#pragma once

#include <Helpers/CompileTime.h>

class RandomClass
{
public:
	RandomClass(unsigned seed = 0)
	{
		JMP_THIS(0x65C630);
	}

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

	Random2Class(DWORD seed = *reinterpret_cast<DWORD*>(0xA8ED94))
	{ JMP_THIS(0x65C6D0); }

	int Random()
	{ JMP_THIS(0x65C780); }

	int RandomRanged(int nMin, int nMax)
	{ JMP_THIS(0x65C7E0); }

	int operator()()
	{ return Random(); }

	int operator()(int nMin, int nMax)
	{ return RandomRanged(nMin, nMax); }

	bool PercentChance(int percent)
	{ return RandomRanged(0,99) < percent; }

	bool PercentChance(double dChance)
	{ return RandomDouble() < dChance; }

	double RandomDouble()
	{ return RandomRanged(1, INT_MAX) / (double)((unsigned int)INT_MAX + 1); }

public:
	bool unknownBool_00;
	PROTECTED_PROPERTY(BYTE, padding[3]);
	int Index1;
	int Index2;
	DWORD Table[0xFA];

};

//static_assert(sizeof(Random2Class) == 0x3F4);

class Random3Class
{
public:
	Random3Class(unsigned seed1 = 0, unsigned seed2 = 0)
	{
		JMP_THIS(0x65C890);
	}

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
