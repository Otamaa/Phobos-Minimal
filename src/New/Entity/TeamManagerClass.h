#pragma once

#include <Utilities/SavegameDef.h>

class TeamManagerClass
{
	static std::vector<TeamManagerClass> Array;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<TeamManagerClass*>(this)->Serialize(Stm); }

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved) { };

public:

	static void Clear()
	{
		Array.clear();
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Process((int)Array.size());

		for (auto& item : Array) {
			item.Save(Stm);
		}

		return true;
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Clear();

		int Count = 0;
		if (!Stm.Load(Count))
			return false;

		Array.resize(Count);

		for (size_t i = 0; i < Count; ++i) {
			if (!Array[i].Load(Stm,true)) {
				return false;
			}
		}

		return true;
	}
private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Success()
			&& Stm.RegisterChange(this)
			;
		;
	}
};