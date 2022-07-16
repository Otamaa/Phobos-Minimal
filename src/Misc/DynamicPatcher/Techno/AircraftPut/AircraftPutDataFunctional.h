#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

namespace AircraftPutDataFunctional
{
	void OnPut(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt , CoordStruct* pCoord);
	void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	CoordStruct GetOffset(AircraftPutData& nData);
	bool IsForceOffset(AircraftPutData& nData);
	bool RemoveIfNoDock(AircraftPutData& nData);
};
#endif