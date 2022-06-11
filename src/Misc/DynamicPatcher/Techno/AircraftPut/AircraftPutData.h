#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class AircraftPutData
{
public:

	Nullable<Point3D> PosOffset;
	Nullable<bool> ForceOffset;
	Nullable<bool> RemoveIfNoDocks;

	~AircraftPutData() = default;

	void Read(INI_EX& parser, const char* pSection)
	{
		PosOffset.Read(parser, pSection, "NoHelipadPutOffset");
		ForceOffset.Read(parser, pSection, "ForcePutOffset");
		RemoveIfNoDocks.Read(parser, pSection, "RemoveIfNoDocks");
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From AircraftPutData ! \n");
		Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;
	}
};
#endif