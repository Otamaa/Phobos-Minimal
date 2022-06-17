#include "HoverTypeClass.h"

Enumerable<HoverTypeClass>::container_t Enumerable<HoverTypeClass>::Array;

const char* Enumerable<HoverTypeClass>::GetMainSection()
{
	return "HoverCharacteristic";
}

void HoverTypeClass::LoadFromINI(CCINIClass * pINI)
{
	const char* pSection = this->Name;

	INI_EX exINI(pINI);

	this->HoverHeight.Read(exINI, pSection, "Hover.Height");
	this->HoverBob.Read(exINI, pSection, "Hover.Bob");
	this->HoverDampen.Read(exINI, pSection, "Hover.Dampen");
	this->HoverAcceleration.Read(exINI, pSection, "Hover.Acceleration");
	this->HoverBrake.Read(exINI, pSection, "Hover.Brake");
	this->HoverBoost.Read(exINI, pSection, "Hover.Boost");
	this->AboveWaterAnim.Read(exINI, pSection, "Hover.AboveWaterAnim",true);
	this->ScoldSound.Read(exINI, pSection, "Hover.ScoldSound");

}

template <typename T>
void HoverTypeClass::Serialize(T & Stm)
{
	Stm
		.Process(this->HoverHeight)
		.Process(this->HoverBob)
		.Process(this->HoverDampen)
		.Process(this->HoverAcceleration)
		.Process(this->HoverBrake)
		.Process(this->HoverBoost)
		.Process(this->AboveWaterAnim)
		.Process(this->ScoldSound)
		;
};

void HoverTypeClass::LoadFromStream(PhobosStreamReader & Stm){this->Serialize(Stm);}
void HoverTypeClass::SaveToStream(PhobosStreamWriter & Stm){this->Serialize(Stm);}