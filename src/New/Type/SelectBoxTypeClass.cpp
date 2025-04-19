#include "SelectBoxTypeClass.h"
#include <Phobos.Defines.h>

Enumerable<SelectBoxTypeClass>::container_t Enumerable<SelectBoxTypeClass>::Array;

const char* Enumerable<SelectBoxTypeClass>::GetMainSection()
{
	return "SelectBoxTypes";
}

void SelectBoxTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (!_stricmp(pSection, DEFAULT_STR2))
		return;

	INI_EX exINI(pINI);

	this->Shape.Read(exINI, pSection, "Shape");
	this->Palette.Read(exINI, pSection, "Palette");
	this->Frame.Read(exINI, pSection, "Frame");
	this->Grounded.Read(exINI, pSection, "Grounded");
	this->Offset.Read(exINI, pSection, "Offset");
	this->Translucency.Read(exINI, pSection, "Translucency");
	this->Show.Read(exINI, pSection, "Show");
	this->ShowObserver.Read(exINI, pSection, "ShowObserver");

	if (!this->Shape.Get())
		this->Shape = FileSystem::LoadSHPFile("select.shp");
}

template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Frame)
		.Process(this->Grounded)
		.Process(this->Offset)
		.Process(this->Translucency)
		.Process(this->Show)
		.Process(this->ShowObserver)
		;
}

void SelectBoxTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void SelectBoxTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}