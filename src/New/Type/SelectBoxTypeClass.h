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
	Valueable<SHPStruct*> Shape {};
	CustomPalette Palette {};
	Nullable<Point3D> Frames {};
	Valueable<bool> Grounded { false };
	Valueable<Point2D> Offset { Point2D::Empty };
	TranslucencyLevel Translucency { 0 };           // or TranslucencyLevel::SomeValue if it's an enum
	Valueable<AffectedHouse> VisibleToHouses { AffectedHouse::All };
	Valueable<bool> VisibleToHouses_Observer { true };
	Valueable<bool> DrawAboveTechno { true };

	Valueable<SHPStruct*> GroundShape { nullptr };
	CustomPalette GroundPalette {};
	Nullable<Point3D> GroundFrames {};
	Valueable<Point2D> GroundOffset { Point2D::Empty };
	Valueable<bool> Ground_AlwaysDraw { true };
	Valueable<bool> GroundLine { false };
	Damageable<ColorStruct> GroundLineColor { {0, 255, 0} };   // nested braces for Damageable<ColorStruct>
	Valueable<bool> GroundLine_Dashed { false };

	SelectBoxTypeClass(const char* pTitle) : Enumerable<SelectBoxTypeClass>(pTitle) {}
	virtual ~SelectBoxTypeClass() = default;

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