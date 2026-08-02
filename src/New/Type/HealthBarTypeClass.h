#pragma once

#include <Utilities/TemplateDef.h>
#include "PaletteManager.h"

class HealthBarTypeClass final : public Enumerable<HealthBarTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "HealthBarTypes";
	static COMPILETIMEEVAL const char* ClassName = "HealthBarTypeClass";

public:

	Nullable<Point3D> Pips {};
	Nullable<Point3D> Pips_Building {};
	Nullable<int> PipsEmpty {};
	Valueable<Point2D> PipsInterval { {2, 0} };
	Valueable<Point2D> PipsInterval_Building { {-4, 2} };
	Nullable<int> PipsLength {};
	Valueable<SHPStruct*> PipsShape { FileSystem::PIPS_SHP };
	CustomPalette PipsPalette {};

	Nullable<int> PipBrd {};
	Nullable<SHPStruct*> PipBrdShape {};
	CustomPalette PipBrdPalette {};
	Valueable<int> PipBrdXOffset { 0 };

	Valueable<int> XOffset { 0 };

	HealthBarTypeClass(const char* pTitle) : Enumerable<HealthBarTypeClass>(pTitle) { }
	virtual ~HealthBarTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};