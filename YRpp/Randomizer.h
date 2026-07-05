#pragma once

#include <Point2D.h>
#include <RandomStruct.h>
#include <array>

class RandomClass
{
protected:
	explicit RandomClass(unsigned seed) noexcept
		: Seed(seed)
	{ }

public:
	operator int() { return operator()(); }

	int operator()()
	{
		constexpr uint32_t Multiplier = 0x41C64E6D;
		constexpr uint32_t Increment = 0x3039;

		this->Seed = this->Seed * Multiplier + Increment;
		return (this->Seed >> 10) & 0x7FFF;
	}

	int operator()(int min, int max)
	{
		if (min == max) {
			return min;
		}

		if (min > max) {
			std::swap(min, max);
		}

		const int range = max - min;

		int highestBit = 14;
		while (highestBit > 0 && ((1 << highestBit) & range) == 0) {
			--highestBit;
		}

		const int mask = ~(-1 << (highestBit + 1));
		int value = range + 1;
		constexpr uint32_t Multiplier = 0x41C64E6D;
		constexpr uint32_t Increment = 0x3039;

		// Preserve the original overflow guard.
		if ((range + 1) > range && (range + 1) != range)
		{
			do
			{
				this->Seed = this->Seed * Multiplier + Increment;
				value = ((this->Seed >> 10) & 0x7FFF) & mask;
			}
			while (value > range);
		}

		return min + value;
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
	static COMPILETIMEEVAL reference<std::uint32_t, 0xA8ED94u> const Seed {};
	static COMPILETIMEEVAL reference<int, 0x839644u, 19u> FirstTable {};
	static COMPILETIMEEVAL reference<int, 0x839690u, 22u> SecondTable {};

public:

	//65C6D0
	COMPILETIMEEVAL explicit Random2Class(std::uint32_t seed) noexcept {
		static constexpr int    kMixPasses = 4;    // inner loop: 4 passes (ebx 0,4,8,12 < 0x10)
		static constexpr int    kBitHalf = 16;   // 32-bit split midpoint for avalanche
		static constexpr size_t kTableSize = 250; // Random2Class::Table capacity
		static constexpr int    kInitIndex2 = 103; // Initial Index2 offset (empirical Westwood value)

		this->Index1 = 0;
		this->Index2 = kInitIndex2;

		for (size_t slotIdx = 0; slotIdx < kTableSize; ++slotIdx)
		{
			// BUGFIX: 'runningValue' threads through all 4 inner passes — it is NOT
			// reset to slotIdx each pass. IDA pseudocode showed a simple per-pass
			// recompute from slotIdx; assembly confirms esi carries the output of
			// each pass as the input to the next (0x65C70F mov eax,esi / 0x65C74B mov esi,ecx).
			int runningValue = static_cast<int>(slotIdx);

			for (int pass = 0; pass < kMixPasses; ++pass)
			{
				// XOR running state with the first mixing table constant.
				const int  mixed = runningValue ^ FirstTable[pass];

				// Split 'mixed' at the 16-bit boundary (SAR 10h in asm → signed).
				const auto low16 = static_cast<std::uint16_t>(mixed);
				const int  high16 = mixed >> kBitHalf;

				// Three multiplies: high²,  high×low (cross),  low²
				const int highSq = high16 * high16;
				const int crossProd = high16 * static_cast<int>(low16);
				const int lowSq = static_cast<int>(low16) * static_cast<int>(low16);

				// Squaring mix: (~high² + low²), then bit-swap the two halves.
				const int squareMix = (~highSq) + lowSq;
				const int folded = (squareMix << kBitHalf) | (squareMix >> kBitHalf);

				// Inject second table constant via XOR, then ADD crossProd.
				// BUGFIX: assembly uses ADD at 0x65C744 (add ecx,edi), not XOR.
				// IDA pseudocode incorrectly showed this as XOR — corrected here.
				const int withTable = folded ^ SecondTable[pass];
				const int withCross = withTable + crossProd;

				// Final seed mix; result becomes input to the next pass.
				runningValue = static_cast<int>(seed) ^ withCross;
			}

			this->Table[slotIdx] = runningValue;
		}

		this->unknownBool_00 = false;
	}

	//Random2Class(std::uint32_t seed) noexcept {
	//	JMP_THIS(0x65C6D0);
	//}

	//int Random() noexcept
	//{
	//	JMP_THIS(0x65C780);
	//}

	//int RandomRanged(int min, int max) noexcept
	//{
	//	JMP_THIS(0x65C7E0);
	//}

	[[nodiscard]] COMPILETIMEEVAL int Random() noexcept {
		if (this->unknownBool_00) return 0;

		this->Table[Index1] ^= this->Table[Index2];
		int result = this->Table[Index1++];

		if (++this->Index2 >= (int)this->Table.size()) this->Index2 = 0;
		if (this->Index1 >= (int)this->Table.size()) this->Index1 = 0;

		return result;
	}

	[[nodiscard]] COMPILETIMEEVAL int RandomRanged(int min, int max) noexcept {
        if (min == max) return min;

        if (min > max) std::swap(min, max);

		const int range = max - min;
		int highestBit = 31;
		while (highestBit > 0 && ((1 << highestBit) & range) == 0)
		{
			--highestBit;
		}

		const int mask = ~(-1 << (highestBit + 1));
		int value = range + 1;

		// Preserve the original overflow guard.
		if ((range + 1) > range && (range + 1) != range) {
			do {
				int random;

				if (this->unknownBool_00) {
					random = 0;
				} else {

					this->Table[this->Index1] ^= this->Table[this->Index2];

					random = this->Table[this->Index1];

					if (++this->Index1 >= (int)this->Table.size()) this->Index1 = 0;
					if (++this->Index2 >= (int)this->Table.size()) this->Index2 = 0;
				}

				value = random & mask;

			}
			while (value > range);
		}

		return min + value;
    }

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
	{ return RandomRanged(1, INT_MAX) * Math::INV_INT_MAX; }

	[[nodiscard]] FORCEDINLINE double RandomDoubleCentered()
	{ return RandomRanged(1, INT_MAX) * Math::INV_INT_MAX - 0.5; }

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

	template<typename T> requires std::is_integral<T>::value
	[[nodiscard]] FORCEDINLINE T RandomFromMax(T nMax) {
		return (T)RandomRanged(0, (int)nMax);
	}

public:
	bool unknownBool_00;
	int Index1;
	int Index2;
	std::array<int, 250> Table;
};

static_assert(sizeof(Random2Class) == 0x3F4);