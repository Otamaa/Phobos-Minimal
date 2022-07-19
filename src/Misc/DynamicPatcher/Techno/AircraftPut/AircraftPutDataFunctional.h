#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct AircraftPutDataFunctional
{
	static void OnPut(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt , CoordStruct* pCoord);
	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static CoordStruct GetOffset(AircraftPutData& nData);
	static bool IsForceOffset(AircraftPutData& nData);
	static bool RemoveIfNoDock(AircraftPutData& nData);
};
#endif