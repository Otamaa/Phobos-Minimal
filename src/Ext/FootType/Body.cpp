#include "Body.h"

#include <Utilities/SavegameDef.h>

template <typename T>
void FootTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->FlightClimb)
		.Process(this->FlightCrash)
		.Process(this->DigStartROT)
		.Process(this->DigInSpeed)
		.Process(this->DigOutSpeed)
		.Process(this->DigEndROT)
		.Process(this->AIDefendBase_Ignore)
		.Process(this->VoiceEnterGrinder)
		;
}

bool FootTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	const char* pSection = this->Name.c_str();

	INI_EX exINI(pINI);
	//INI_EX iniEX_art(CCINIClass::INI_Art());
	//const auto pSection_art = ((TechnoTypeExtData*)this)->This()->ImageFile;

	this->FlightClimb.Read(exINI, pSection, "FlightClimb");
	this->FlightCrash.Read(exINI, pSection, "FlightCrash");
	this->DigStartROT.Read(exINI, pSection, "DigStartROT");
	this->DigInSpeed.Read(exINI, pSection, "DigInSpeed");
	this->DigOutSpeed.Read(exINI, pSection, "DigOutSpeed");
	this->DigEndROT.Read(exINI, pSection, "DigEndROT");
	this->AIDefendBase_Ignore.Read(exINI, pSection, "AIDefendBase.Ignore");
	this->VoiceEnterGrinder.Read(exINI, pSection, "VoiceEnterGrinder");

	return true;
}

FootTypeExtContainer FootTypeExtContainer::Instance;
