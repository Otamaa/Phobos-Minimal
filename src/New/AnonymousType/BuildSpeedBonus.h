#pragma once

#include <Utilities/TemplateDefB.h>

struct BuildSpeedBonus
{
	bool Enabled { false };
	double SpeedBonus_Aircraft { 0.000 };
	double SpeedBonus_Building { 0.000 };
	double SpeedBonus_Infantry { 0.000 };
	double SpeedBonus_Unit { 0.000 };
	ValueableVector<TechnoTypeClass*> AffectedType { };

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& stm, bool registerForChange) {
		return Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const {
		return const_cast<BuildSpeedBonus*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(Enabled)
			.Process(SpeedBonus_Aircraft)
			.Process(SpeedBonus_Building)
			.Process(SpeedBonus_Infantry)
			.Process(SpeedBonus_Unit)
			.Process(AffectedType)
			.Success()
			//&& stm.RegisterChange(this)
			; // announce this type
	}
};
