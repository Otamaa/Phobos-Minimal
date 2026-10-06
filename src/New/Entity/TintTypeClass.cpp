#include "TintTypeClass.h"

void TintTypeClass::Read(INI_EX& exINI, const char* pSection)
{
	this->Color.Read(exINI, pSection, "Tint.Color");
	this->Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");
	this->Cumulative.Read(exINI, pSection, "Tint.Cumulative");
	this->Enabled = this->Color.Get() != ColorStruct::Empty || this->Intensity != 0.0;
}

#pragma region(save/load)

template <class T>
bool TintTypeClass::Serialize(T& stm)
{
	stm.RegisterChange(this);
	return stm

		.Process(this->Color)
		.Process(this->Intensity)
		.Process(this->VisibleToHouses)
		.Process(this->Cumulative)
		.Process(this->Enabled)
		.Success();
}

bool TintTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TintTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TintTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)