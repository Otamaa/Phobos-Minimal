#pragma once

#include <Utilities/Template.h>

class TechnoTypeClass;
struct BuildSpeedBonus
{
	bool Enabled { false };
	double SpeedBonus_Aircraft { 0.000 };
	double SpeedBonus_Building { 0.000 };
	double SpeedBonus_Infantry { 0.000 };
	double SpeedBonus_Unit { 0.000 };
	ValueableVector<TechnoTypeClass*> AffectedType { };

public:

	void Read(INI_EX& parser, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
