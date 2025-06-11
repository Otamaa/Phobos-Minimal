#pragma once

#include <Utilities/TemplateDef.h>
#include "PaletteManager.h"

class HealthBarTypeClass final : public Enumerable<HealthBarTypeClass>
{
public:

	Nullable<Point3D> Pips;
	Nullable<Point3D> Pips_Building;
	Nullable<int> PipsEmpty;
	Valueable<Point2D> PipsInterval;
	Valueable<Point2D> PipsInterval_Building;
	Nullable<int> PipsLength;
	Valueable<SHPStruct*> PipsShape;
	CustomPalette PipsPalette;

	Nullable<int> PipBrd;
	Nullable<SHPStruct*> PipBrdShape;
	CustomPalette PipBrdPalette;
	Valueable<int> PipBrdXOffset;

	Valueable<int> XOffset;

	HealthBarTypeClass(const char* pTitle) : Enumerable<HealthBarTypeClass>(pTitle)
	, Pips {}
	, Pips_Building {}
	, PipsEmpty {}
	, PipsInterval { { 2,0 } }
	, PipsInterval_Building { { -4,2 } }
	, PipsLength {}
	, PipsShape { FileSystem::PIPS_SHP }
	, PipsPalette {}

	, PipBrd {}
	, PipBrdShape {}
	, PipBrdPalette {}
	, PipBrdXOffset { 0 }
	, XOffset { 0 }

	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};