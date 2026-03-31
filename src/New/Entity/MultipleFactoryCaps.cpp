#include "MultipleFactoryCaps.h"


void MultipleFactoryCaps::Read(INI_EX& exINI, const char* pSection)
{
	this->AbsUnit.Read(exINI, pSection, "MultipleFactoryCap.UnitType");
	this->AbsUnitNaval.Read(exINI, pSection, "MultipleFactoryCap.UnitTypeNaval");
	this->AbsInf.Read(exINI, pSection, "MultipleFactoryCap.InfantryType");
	this->AbsAircraft.Read(exINI, pSection, "MultipleFactoryCap.AircraftType");
	this->AbsBuilding.Read(exINI, pSection, "MultipleFactoryCap.BuildingType");
}

int MultipleFactoryCaps::Get(AbstractType abs, bool naval) const
{
	switch (abs)
	{
	case AbstractType::Unit:
	case AbstractType::UnitType:
		return naval ? this->AbsUnitNaval : this->AbsUnit;
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		return this->AbsAircraft;

	case AbstractType::Building:
	case AbstractType::BuildingType:
		return this->AbsBuilding;
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		return this->AbsInf;
	default:
		return 0;
	}
}

#pragma region(save/load)

template <class T>
bool MultipleFactoryCaps::Serialize(T& stm)
{
	return stm
		.Process(this->AbsUnit)
		.Process(this->AbsUnitNaval)
		.Process(this->AbsInf)
		.Process(this->AbsAircraft)
		.Process(this->AbsBuilding)
		.Success()
		//&& stm.RegisterChange(this)
		; // announce this type
}

bool MultipleFactoryCaps::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool MultipleFactoryCaps::Save(PhobosStreamWriter& stm) const
{
	return const_cast<MultipleFactoryCaps*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
