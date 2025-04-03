#pragma once

#include <Utilities/TemplateDefB.h>

class CCINIClass;
struct MultipleFactoryCaps
{
	Valueable<int> AbsUnit { -1 };
	Valueable<int> AbsUnitNaval { -1 };
	Valueable<int> AbsInf { -1 };
	Valueable<int> AbsAircraft { -1 };
	Valueable<int> AbsBuilding { -1 };

//
	void Read(INI_EX& exINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	int Get(AbstractType abs , bool naval) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
