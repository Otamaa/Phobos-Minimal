#include "HealthBarTypeClass.h"

Enumerable<HealthBarTypeClass>::container_t Enumerable<HealthBarTypeClass>::Array;

void HealthBarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	INI_EX exINI(pINI);

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->PipsEmpty.Read(exINI, pSection, "PipsEmpty");
	this->PipsInterval.Read(exINI, pSection, "PipsInterval");
	this->PipsInterval_Building.Read(exINI, pSection, "PipsInterval.Building");
	this->PipsLength.Read(exINI, pSection, "PipsLength");
	this->PipsShape.Read(exINI, pSection, "PipsShape");
	this->PipsPalette.Read(exINI, pSection, "PipsPalette");

	this->PipBrd.Read(exINI, pSection, "PipBrd");
	this->PipBrdShape.Read(exINI, pSection, "PipBrdShape");
	this->PipBrdPalette.Read(exINI, pSection, "PipBrdPalette");
	this->PipBrdXOffset.Read(exINI, pSection, "PipBrdXOffset");

	this->XOffset.Read(exINI, pSection, "XOffset");
}

template <typename T>
void HealthBarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Pips)
		.Process(this->Pips_Building)
		.Process(this->PipsEmpty)
		.Process(this->PipsInterval)
		.Process(this->PipsInterval_Building)
		.Process(this->PipsLength)
		.Process(this->PipsShape)
		.Process(this->PipsPalette)
		.Process(this->PipBrd)
		.Process(this->PipBrdShape)
		.Process(this->PipBrdPalette)
		.Process(this->PipBrdXOffset)
		.Process(this->XOffset)
		;
}

void HealthBarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}