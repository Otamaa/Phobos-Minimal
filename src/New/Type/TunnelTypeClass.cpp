#include "TunnelTypeClass.h"

#include <Utilities/INIParser.h>

const char* Enumerable<TunnelTypeClass>::GetMainSection()
{
	return "TunnelTypes";
}

void TunnelTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.data();
	INI_EX exINI(pINI);

	this->Passengers.Read(exINI, pSection, "Passengers");
	this->MaxSize.Read(exINI, pSection, "MaxSize");
}

template <typename T>
void TunnelTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Passengers)
		.Process(this->MaxSize)
		;
}

void TunnelTypeClass::LoadFromStream(PhobosStreamReader& Stm) {
	this->Serialize(Stm);
}

void TunnelTypeClass::SaveToStream(PhobosStreamWriter& Stm) {
	this->Serialize(Stm);
}