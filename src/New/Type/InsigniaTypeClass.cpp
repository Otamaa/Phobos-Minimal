#include "InsigniaTypeClass.h"

Enumerable<InsigniaTypeClass>::container_t Enumerable<InsigniaTypeClass>::Array;

template<>
const char* Enumerable<InsigniaTypeClass>::GetMainSection() { return "InsigniaTypes"; }

void InsigniaTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name.data();

	INI_EX exINI(pINI);

	this->Insignia.Read(exINI, section, "Insignia.%s");
	this->InsigniaFrame.Read(exINI, section, "InsigniaFrame.%s");
}

template <typename T>
void InsigniaTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Insignia)
		.Process(this->InsigniaFrame)
		;
};

void InsigniaTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void InsigniaTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
