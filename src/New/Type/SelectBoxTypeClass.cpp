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

	if (IS_SAME_STR_(pSection, DEFAULT_STR2))
		return;

	INI_EX exINI(pINI);

	this->Shape.Read(exINI, pSection, "Shape");
	this->Palette.Read(exINI, pSection, "Palette");
	this->Frames.Read(exINI, pSection, "Frames");
	this->Grounded.Read(exINI, pSection, "Grounded");
	this->Offset.Read(exINI, pSection, "Offset");
	this->Translucency.Read(exINI, pSection, "Translucency");
	this->VisibleToHouses.Read(exINI, pSection, "VisibleToHouses");
	this->VisibleToHouses_Observer.Read(exINI, pSection, "VisibleToHouses.Observer");
	this->DrawAboveTechno.Read(exINI, pSection, "DrawAboveTechno");

	this->GroundShape.Read(exINI, pSection, "GroundShape");
	this->GroundPalette.Read(exINI, pSection, "GroundPalette");
	this->GroundFrames.Read(exINI, pSection, "GroundFrames");
	this->GroundOffset.Read(exINI, pSection, "GroundOffset");
	this->Ground_AlwaysDraw.Read(exINI, pSection, "Ground.AlwaysDraw");
	this->GroundLine.Read(exINI, pSection, "GroundLine");
	this->GroundLineColor.Read(exINI, pSection, "GroundLineColor.%s");
	this->GroundLine_Dashed.Read(exINI, pSection, "GroundLine.Dashed");
}

template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Frames)
		.Process(this->Grounded)
		.Process(this->Offset)
		.Process(this->Translucency)
		.Process(this->VisibleToHouses)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->DrawAboveTechno)

		.Process(this->GroundShape)
		.Process(this->GroundPalette)
		.Process(this->GroundFrames)
		.Process(this->GroundOffset)
		.Process(this->Ground_AlwaysDraw)
		.Process(this->GroundLine)
		.Process(this->GroundLineColor)
		.Process(this->GroundLine_Dashed)
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