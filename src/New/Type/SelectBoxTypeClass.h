#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Enum.h>
#include <New/Type/PaletteManager.h>

class PaletteManager;
CREATEENUMTYPECLASS(SelectBox)
{
public:
	Valueable<SHPStruct*> Shape;
	Valueable<PaletteManager*> Palette;
	Nullable<Point3D> Frames;
	Valueable<bool> Grounded;
	Valueable<Point2D> Offset;
	TranslucencyLevel Translucency;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<bool> DrawAboveTechno;

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
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		auto pDefault=	FindOrAllocate(DEFAULT_STR2);

		if (!pDefault->Shape)
			pDefault->Shape = FileSystem::LoadSHPFile("select.shp");
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};