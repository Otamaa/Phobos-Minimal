#include "SelectBoxTypeClass.h"
#include <Phobos.Defines.h>

Enumerable<SelectBoxTypeClass>::container_t Enumerable<SelectBoxTypeClass>::Array;

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

#ifndef _Print
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
#else
template <typename T>
void SelectBoxTypeClass::Serialize(T& Stm)
{
	auto debugProcess = [&Stm](auto& field, const char* fieldName) -> auto&
		{
			if constexpr (std::is_same_v<T, PhobosStreamWriter>)
			{
				size_t beforeSize = Stm.Getstream()->Size();
				auto& result = Stm.Process(field);
				size_t afterSize = Stm.Getstream()->Size();
				GameDebugLog::Log("[SelectBoxTypeClass] SAVE %s: size %zu -> %zu (+%zu)\n",
					fieldName, beforeSize, afterSize, afterSize - beforeSize);
				return result;
			}
			else
			{
				size_t beforeOffset = Stm.Getstream()->Offset();
				bool beforeSuccess = Stm.Success();
				auto& result = Stm.Process(field);
				size_t afterOffset = Stm.Getstream()->Offset();
				bool afterSuccess = Stm.Success();

				GameDebugLog::Log("[SelectBoxTypeClass] LOAD %s: offset %zu -> %zu (+%zu), success: %s -> %s\n",
					fieldName, beforeOffset, afterOffset, afterOffset - beforeOffset,
					beforeSuccess ? "true" : "false", afterSuccess ? "true" : "false");

				if (!afterSuccess && beforeSuccess)
				{
					GameDebugLog::Log("[SelectBoxTypeClass] ERROR: %s caused stream failure!\n", fieldName);
				}
				return result;
			}

		};

	debugProcess(this->Shape, "Shape");
	debugProcess(this->Palette, "Palette");
	debugProcess(this->Frames, "Frames");
	debugProcess(this->Grounded, "Grounded");
	debugProcess(this->Offset, "Offset");
	debugProcess(this->Translucency, "Translucency");
	debugProcess(this->VisibleToHouses, "VisibleToHouses");
	debugProcess(this->VisibleToHouses_Observer, "VisibleToHouses_Observer");
	debugProcess(this->DrawAboveTechno, "DrawAboveTechno");

	debugProcess(this->GroundShape, "GroundShape");
	debugProcess(this->GroundPalette, "GroundPalette");
	debugProcess(this->GroundFrames, "GroundFrames");
	debugProcess(this->GroundOffset, "GroundOffset");
	debugProcess(this->Ground_AlwaysDraw, "Ground_AlwaysDraw");
	debugProcess(this->GroundLine, "GroundLine");
	debugProcess(this->GroundLineColor, "GroundLineColor");
	debugProcess(this->GroundLine_Dashed, "GroundLine_Dashed");
}
#endif

void SelectBoxTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void SelectBoxTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}