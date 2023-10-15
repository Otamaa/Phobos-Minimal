#pragma once
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct AircraftPutDataFunctional
{
	static void OnPut(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt , CoordStruct* pCoord);
	static void AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	static CoordStruct GetOffset(AircraftPutData& nData);
	static bool IsForceOffset(AircraftPutData& nData);
	static bool RemoveIfNoDock(AircraftPutData& nData);
};
