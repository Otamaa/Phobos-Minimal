#pragma once
#include <Utilities/TemplateDef.h>

class AircraftPutData
{
public:

	Nullable<Point3D> PosOffset {};
	Nullable<bool> ForceOffset {};
	Nullable<bool> RemoveIfNoDocks {};

	void Read(INI_EX& parser, const char* pSection);


	template <typename T>
	void  Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From AircraftPutData ! \n");
		Stm
			.Process(PosOffset)
			.Process(RemoveIfNoDocks)
			.Process(ForceOffset)
			;

		//Stm.RegisterChange(this);
	}
};
