#pragma once
#include <Utilities/TemplateDef.h>

class AircraftPutDataRules
{
public :

	Valueable<Point3D> PosOffset { {0,0,0} };
	Valueable<bool> RemoveIfNoDocks { false };
	Valueable<bool> ForceOffset { false };

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<AircraftPutDataRules*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;
	}
};
