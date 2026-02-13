#include "Body.h"

bool FootTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	const char* pID =	((TechnoTypeExtData*)this)->This()->ID;

	INI_EX exINI(pINI);
	INI_EX iniEX_art(CCINIClass::INI_Art());
	const auto pSection_art = ((TechnoTypeExtData*)this)->This()->ImageFile;

	return true;
}


FootTypeExtContainer FootTypeExtContainer::Instance;