#pragma once

#include "Template.h"

// Use fixed array instead of vectors , to save some size
template<int Amount>
struct MultiBoolFixedArray
{
	OPTIONALINLINE void Read(INI_EX& parser, const char* const pSection, const char* const pKey, const std::array<const char* const, Amount>& nKeysArray) {
		if (parser.ReadString(pSection, pKey) > 0) {
			Reset();
			char* context = nullptr;
			for (char* cur = strtok_s(parser.value(), Phobos::readDelims, &context);
				cur; cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
				for (int i = 0; i < Amount; ++i) {
					if (IS_SAME_STR_(cur, nKeysArray[i])) {
						this->Datas[i] = true;
						break;
					}
				}
			}
		}
	}

	COMPILETIMEEVAL int size() const { return Amount;	}

	// no index validation
	COMPILETIMEEVAL bool at(int Index) const { 	return Datas[Index]; }

	// index validation manually done
	bool Get(int Index) const {
		const size_t compare = size_t(Index);
		return Datas[compare >= Amount ? Amount - 1 : Index];
	}

	COMPILETIMEEVAL Iterator<bool> GetElements() const noexcept
	{
		return Iterator<bool>(&this->Datas[0], Amount);
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Reset();

		for (int i = 0; i < Amount; ++i) {
			if (!Stm.Process(this->Datas[i], RegisterForChange))
				return false;
		}

		return true;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		for (auto& nData : this->Datas) {
			if (!Stm.Process(nData))
				return false;
		}

		return true;
	}

	void Reset()
	{
		if constexpr(sizeof(bool) == 1)
			__stosb(reinterpret_cast<unsigned char*>(Datas), 0, sizeof(bool) * Amount);
		else
			std::memset(Datas, 0, sizeof(bool) * Amount);
	}

protected:
	bool Datas[Amount] { false };

};
static_assert(sizeof(bool) == 1, "This code assumes bool is 1 byte!");