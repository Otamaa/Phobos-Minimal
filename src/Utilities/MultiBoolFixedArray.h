#pragma once

#include "Template.h"

// Use fixed array instead of vectors , to save some size
template<int Amount>
struct MultiBoolFixedArray
{
	inline void Read(INI_EX& parser, const char* const pSection, const char* const pKey, const std::array<const char* const, Amount>& nKeysArray) {
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

	constexpr int size() const { return Amount;	}
	
	// no index validation 
	bool at(int Index) const { 	return Datas[Index]; }

	// index validation manually done
	bool Get(int Index) const {
		const size_t compare = size_t(Index);
		return Datas[compare >= Amount ? Amount - 1 : Index];
	}

	constexpr Iterator<bool> GetElements() const noexcept
	{
		return Iterator<bool>(&this->Datas[0], Amount);
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Reset();

		for (int i = 0; i < Amount; ++i) {
			if (!Savegame::ReadPhobosStream(Stm, this->Datas[i]))
				return false;
		}

		return true;
	}

	inline bool Save(PhobosStreamWriter& Stm) const
	{
		for (auto nData : this->Datas) {
			if (!Savegame::WritePhobosStream(Stm, nData))
				return false;
		}

		return true;
	}

	void Reset()
	{
		std::memset(Datas, 0, sizeof(Datas));
	}

protected:
	bool Datas[Amount] { false };
};
