#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class AircraftPutDataRules
{
public :

	Valueable<Point3D> PosOffset;
	Valueable<bool> ForceOffset;
	Valueable<bool> RemoveIfNoDocks;

	AircraftPutDataRules():
		PosOffset { {0,0,0} }
		, RemoveIfNoDocks { false  }
		, ForceOffset { false }
	{ }

	~AircraftPutDataRules() = default;

	void Read(INI_EX & parser, const char* pSection)
	{
		PosOffset.Read(parser, pSection, "AircraftNoHelipadPutOffset");
		ForceOffset.Read(parser, pSection, "AircraftForcePutOffset");
		RemoveIfNoDocks.Read(parser, pSection, "RemoveIfNoDocks");
	}

	template <typename T>
	void Serialize(T & Stm)
	{
		Debug::Log("Loading Element From AircraftPutDataRules ! \n");
		Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;
	}
};
#endif