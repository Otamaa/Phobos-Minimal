#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Enum.h>
#include <New/Type/PaletteManager.h>

class SelectBoxTypeClass final : public Enumerable<SelectBoxTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "SelectBoxTypes";
	static COMPILETIMEEVAL const char* ClassName = "SelectBoxTypeClass";

public:
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;
	Nullable<Point3D> Frames;
	Valueable<bool> Grounded;
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> DrawAboveTechno;

	Valueable<SHPStruct*> GroundShape;
	CustomPalette GroundPalette;
	Nullable<Point3D> GroundFrames;
	Valueable<Point2D> GroundOffset;
	Valueable<bool> Ground_AlwaysDraw;
	Valueable<bool> GroundLine;
	Damageable<ColorStruct> GroundLineColor;
	Valueable<bool> GroundLine_Dashed;

	SelectBoxTypeClass(const char* pTitle ) : Enumerable<SelectBoxTypeClass>(pTitle)
		, Shape {}
		, Palette {}
		, Frames {}
		, Grounded { false }
		, Offset { Point2D::Empty }
		, Translucency { 0 }
		, VisibleToHouses { AffectedHouse::All }
		, VisibleToHouses_Observer { true }
		, DrawAboveTechno { true }

		, GroundShape { nullptr }
		, GroundPalette {}
		, GroundFrames {}
		, GroundOffset { Point2D::Empty }
		, Ground_AlwaysDraw { true }
		, GroundLine { false }
		, GroundLineColor { { 0,255,0 } }
		, GroundLine_Dashed { false }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		auto pDefault=	FindOrAllocate(DEFAULT_STR2);

		if (!pDefault->Shape)
			pDefault->Shape = FileSystem::LoadSHPFile("select.shp");


#ifdef _Print
//the first item usually fine , the second item is for debugging
		auto pDefault2 = FindOrAllocate(DEFAULT_STR);

		if (!pDefault2->Shape)
			pDefault2->Shape = FileSystem::LoadSHPFile("select.shp");
#endif
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};