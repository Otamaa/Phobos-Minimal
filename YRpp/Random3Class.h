#pragma once

#include <Base/Macros.h>

#include <array>
#include <Straws.h>

// Random3 — Random3Straw (RandomStraw<Random3Class>)
//   Pool of 32 Random3Class instances — high-quality PRNG pool.
//   Used exclusively in PKStraw ENCODE path for Blowfish session key generation.
//   Template: RandomStraw<T> — generic Straw adapter for any RNG class.
//   Dead code in retail binary — internal MIX packer remnant.
// ---------------------------------------------------------------------------
class Random3Class
{
protected:

	explicit Random3Class(unsigned seed1, unsigned seed2) noexcept
		: Seed(seed1), Index(seed2)
	{}

public:

	operator int() { return operator()(); }

	int operator()()
	{
		int index = this->Index++;
		int seed = this->Seed;

		for (int i = 0; i < 4; ++i)
		{
			const int previous = index;

			int value = index ^ Mix1[i];
			const uint16_t low = static_cast<uint16_t>(value);
			const int high = value >> 16;

			const int squareMix = low * low + ~(high * high);

			value = seed ^ (
				low * high +
				(
					Mix2[i + 1] ^
					((squareMix << 16) | (squareMix >> 16))
					)
			);

			seed = previous;
			index = value;
		}

		return index;
	}

	int operator()(int min, int max)
	{
		if (min == max)
		{
			return min;
		}

		if (min > max)
		{
			std::swap(min, max);
		}

		const int range = max - min;

		int highestBit = 31;
		while (highestBit > 0 && ((1 << highestBit) & range) == 0)
		{
			--highestBit;
		}

		const int mask = ~(-1 << (highestBit + 1));
		int value = range + 1;

		// Preserve the original overflow guard.
		if ((range + 1) > range && (range + 1) != range)
		{
			const int seed = this->Seed;

			do
			{
				int index = this->Index++;
				int previous = seed;
				int random = 0;

				for (int i = 0; i < 4; ++i)
				{
					const int oldIndex = index;

					int mixed = index ^ Mix1[i];
					const int high = mixed >> 16;

					random = previous ^ (
						mixed * high +
						(
							Mix2[i + 1] ^
							(
								(((mixed * mixed) + ~(high * high)) << 16) |
								(((mixed * mixed) + ~(high * high)) >> 16)
								)
							)
					);

					previous = oldIndex;
					index = random;
				}

				value = random & mask;

			}
			while (value > range);
		}

		return min + value;
	}

	template<typename T> T operator()(T minval, T maxval) { return (T)(*this)(int(minval), int(maxval)); }

protected:
	int Seed;
	int Index;

private:
	static COMPILETIMEEVAL reference<unsigned, 0x839644u, 20> Mix1 {};
	static COMPILETIMEEVAL reference<unsigned, 0x839690u, 20> Mix2 {};
};
static_assert(sizeof(Random3Class) == 0x8);

class Random3Straw : public Straw
{
public:
	static COMPILETIMEEVAL reference<Random3Straw, 0xA8E7B0u> const Instance {};

	virtual ~Random3Straw() override { JMP_THIS(0x661C70); }
	virtual int Get(void* source, int slen) override { JMP_THIS(0x661C10); }

private:
	DWORD m_currentSeedBits;
	DWORD m_currentRandoms;
	Random3Class m_randomArray[32];

private:
	Random3Straw(const Random3Straw&) = delete;
	Random3Straw& operator=(const Random3Straw&) = delete;
};
