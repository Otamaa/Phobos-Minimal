#pragma once

#include "Template.h"

// Use fixed array instead of vectors , to save some size
template<int Amount>
struct MultiBoolFixedArray
{
	inline void Read(INI_EX& parser, const char* const pSection, const char* const pKey, const std::array<const char*, Amount>& nKeysArray)
	{
		if (parser.ReadString(pSection, pKey) > 0)
		{
			char* context = nullptr;
			for (char* cur = strtok_s(parser.value(), ",", &context); cur; cur = strtok_s(nullptr, ",", &context))
			{
				for (int i = 0; i < Amount; ++i)
				{
					if (!_strcmpi(cur, nKeysArray[i]))
					{
						this->Datas[i] = true;
						break;
					}
				}
			}
		}
	}

	constexpr int size() const { return Amount;	}
	
	bool at(int Index) const { return Get(Index); }

	bool Get(int Index) const
	{
		Index = std::clamp(Index, 0, Amount);
		return Datas[Index];
	}

	constexpr Iterator<bool> GetElements() const noexcept
	{
		return Iterator<bool>(&this->Datas[0], Amount);
	}

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{

		for (int i = 0; i < Amount; ++i)
		{
			if (!Savegame::ReadPhobosStream(Stm, this->Datas[i]))
				return false;
		}

		return true;
	}

	inline bool Save(PhobosStreamWriter& Stm) const
	{

		for (auto nData : this->Datas)
		{
			if (!Savegame::WritePhobosStream(Stm, nData))
				return false;
		}

		return true;
	}

protected:
	bool Datas[Amount] { false };
};
