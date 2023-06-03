#pragma once
#include <Utilities/TemplateDef.h>

class AircraftPutDataRules
{
public :

	Valueable<Point3D> PosOffset { {0,0,0} };
	Valueable<bool> RemoveIfNoDocks { false };
	Valueable<bool> ForceOffset { false };

	void Read(INI_EX& parser, const char* pSection);

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From AircraftPutDataRules ! \n");
		Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;
	}
};
