#pragma once

#include "Template.h"

// Use fixed array instead of vectors , to save some size
template<int Amount>
struct MultiBoolFixedArray
{
	using Storage = std::uint32_t;
	static constexpr int BitsPerWord = sizeof(Storage) * CHAR_BIT;
	static constexpr int WordCount =
		(Amount + BitsPerWord - 1) / BitsPerWord;

	OPTIONALINLINE void Read(
		INI_EX& parser,
		const char* const pSection,
		const char* const pKey,
		std::array<const char*, Amount>& nKeysArray)
	{
		if (parser.ReadString(pSection, pKey) > 0)
		{
			Reset();
			char* context = nullptr;

			for (char* cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				for (int i = 0; i < Amount; ++i)
				{
					if (IS_SAME_STR_(cur, nKeysArray[i]))
					{
						Set(i, true);
						break;
					}
				}
			}
		}
	}

	COMPILETIMEEVAL int size() const { return Amount; }

	COMPILETIMEEVAL bool at(int Index) const
	{
		return Get(Index);
	}

	bool Get(int Index) const
	{
		if (Index < 0 || Index >= Amount)
			return false;

		const int word = Index / BitsPerWord;
		const int bit = Index % BitsPerWord;
		return (Bits[word] >> bit) & 1u;
	}

	void Set(int Index, bool value)
	{
		if (Index < 0 || Index >= Amount)
			return;

		const int word = Index / BitsPerWord;
		const int bit = Index % BitsPerWord;
		const Storage mask = Storage(1) << bit;

		if (value)
			Bits[word] |= mask;
		else
			Bits[word] &= ~mask;
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Reset();

		for (int i = 0; i < Amount; ++i)
		{
			bool value = false;
			if (!Stm.Process(value, RegisterForChange))
				return false;
			Set(i, value);
		}

		return true;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		for (int i = 0; i < Amount; ++i)
		{
			bool value = Get(i);
			if (!Stm.Process(value))
				return false;
		}

		return true;
	}

	void Reset() {
		std::memset(Bits, 0, sizeof(Bits));
	}

protected:
	Storage Bits[WordCount] {};
};