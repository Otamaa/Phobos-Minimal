#pragma once
#include <Utilities/TemplateDef.h>

class AircraftPutData
{
public:

	Nullable<Point3D> PosOffset {};
	Nullable<bool> ForceOffset {};
	Nullable<bool> RemoveIfNoDocks {};

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<AircraftPutData*>(this)->Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::LogInfo("Loading Element From AircraftPutData ! ");
		return Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;
	}
};
