#pragma once

#include <array>
#include <Base/Always.h>

class PhobosStreamReader;
class PhobosStreamWriter;
class INI_EX;

// Use fixed array instead of vectors , to save some size
template<int Amount>
struct MultiBoolFixedArray
{
	using Storage = std::uint32_t;
	static constexpr int BitsPerWord = sizeof(Storage) * CHAR_BIT;
	static constexpr int WordCount =
		(Amount + BitsPerWord - 1) / BitsPerWord;

	void Read(
		INI_EX& parser,
		const char* const pSection,
		const char* const pKey,
		std::array<const char*, Amount>& nKeysArray);
	
	OPTIONALINLINE COMPILETIMEEVAL int size() const { return Amount; }

	OPTIONALINLINE COMPILETIMEEVAL bool at(int Index) const
	{
		return Get(Index);
	}

	OPTIONALINLINE COMPILETIMEEVAL bool Get(int Index) const
	{
		if (Index < 0 || Index >= Amount)
			return false;

		const int word = Index / BitsPerWord;
		const int bit = Index % BitsPerWord;
		return (Bits[word] >> bit) & 1u;
	}

	OPTIONALINLINE COMPILETIMEEVAL void Set(int Index, bool value)
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

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	OPTIONALINLINE void Reset() {
		std::memset(Bits, 0, sizeof(Bits));
	}

protected:
	Storage Bits[WordCount] {};
};