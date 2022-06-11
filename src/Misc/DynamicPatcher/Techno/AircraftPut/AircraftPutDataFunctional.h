#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct AircraftPutDataFunctional
{
private:
	NO_CONSTRUCT_CLASS(AircraftPutDataFunctional)
public:
	static void OnPut(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt , CoordStruct* pCoord);
	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
private:
	static CoordStruct GetOffset(AircraftPutData& nData);
	static bool IsForceOffset(AircraftPutData& nData);
	static bool RemoveIfNoDock(AircraftPutData& nData);
};
#endif