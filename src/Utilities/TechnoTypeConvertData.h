#pragma once

#include "EnumFunctions.h"

class TechnoTypeClass;
class HouseClass;
class TechnoClass;
struct TechnoTypeConvertData
{
	TechnoTypeClass* From { nullptr };
	TechnoTypeClass* To { nullptr };
	AffectedHouse Eligible { AffectedHouse::All };

	static void ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass* pHouse, TechnoClass* pTarget , AnimTypeClass* SucceededAnim);
};