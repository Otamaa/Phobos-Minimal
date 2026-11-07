#include "Body.h"


bool AbstractTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return false;

	INI_EX exINI(pINI);

	auto pSection = this->Name.data();
	if(this->AbsType == AbstractType::SuperWeaponType || 
		this->AbsType == AbstractType::AircraftType || 
		this->AbsType == AbstractType::InfantryType ||
		this->AbsType == AbstractType::BuildingType || 
		this->AbsType == AbstractType::UnitType) { 
		this->CameoPriority_Houses = pINI->ReadHouseTypesList(pSection, "CameoPriority.Houses", this->CameoPriority_Houses);
		this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	}

	return true;
}