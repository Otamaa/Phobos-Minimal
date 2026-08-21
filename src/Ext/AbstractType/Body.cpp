#include "Body.h"


bool AbstractTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return false;

	INI_EX exINI(pINI);

	auto pSection = this->Name.data();
	this->CameoPriority_Houses = pINI->ReadHouseTypesList(pSection, "CameoPriority.Houses", this->CameoPriority_Houses);
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	return true;
}