#pragma once

#include "Enum.h"
#include <vector>

class TechnoTypeClass;
class HouseClass;
class TechnoClass;
class AnimTypeClass;
/*
*	WARNING : not direcly compatible with Phobos Develop one !
*/
class PhobosStreamReader;
class PhobosStreamWriter;
class INI_EX;
struct TechnoTypeConvertData
{
	std::vector<TechnoTypeClass*> From { };
	TechnoTypeClass* To { nullptr };
	AffectedHouse Eligible { AffectedHouse::All };

public:

	static void ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass* pHouse, TechnoClass* pTarget , AnimTypeClass* SucceededAnim);
	static void Parse(bool useDevelopversion, std::vector<TechnoTypeConvertData>& list, INI_EX& exINI, const char* pSection, const char* pKey);

public:

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
};