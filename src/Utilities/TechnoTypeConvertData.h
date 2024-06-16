#pragma once

#include "EnumFunctions.h"
#include "SavegameDef.h"

class TechnoTypeClass;
class HouseClass;
class TechnoClass;
/*
*	WARNING : not direcly compatible with Phobos Develop one !
*/
struct TechnoTypeConvertData
{
	std::vector<TechnoTypeClass*> From { };
	TechnoTypeClass* To { nullptr };
	AffectedHouse Eligible { AffectedHouse::All };

	static void ApplyConvert(const std::vector<TechnoTypeConvertData>& nPairs , HouseClass* pHouse, TechnoClass* pTarget , AnimTypeClass* SucceededAnim);
	static void Parse(bool useDevelopversion, std::vector<TechnoTypeConvertData>& list, INI_EX& exINI, const char* pSection, const char* pKey);

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<TechnoTypeConvertData*>(this)->Serialize(stm);
	}

	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->From)
			.Process(this->To)
			.Process(this->Eligible)
			.Success()
			//&& stm.RegisterChange(this)
			; // announce this type
	}
};