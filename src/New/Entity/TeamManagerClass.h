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
		return Stm.Process(Array);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm.Process(Array);
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