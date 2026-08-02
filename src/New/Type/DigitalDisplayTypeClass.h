#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Anchor.h>
#include <Utilities/Template.h>
#include <New/Type/PaletteManager.h>

class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "DigitalDisplayTypes";
	static COMPILETIMEEVAL const char* ClassName = "DigitalDisplayTypeClass";

public:
	Damageable<ColorStruct> Text_Color {
		{0, 255, 0},
		{255, 255, 0},
		{255, 0, 0}
	};
	Valueable<bool> Text_Background { false };
	Valueable<Point2D> Offset { Point2D::Empty };
	Nullable<Point2D> Offset_ShieldDelta {};
	Valueable<TextAlign> Align { TextAlign::Right };
	Anchor AnchorType { HorizontalPosition::Right, VerticalPosition::Top };
	Valueable<BuildingSelectBracketPosition> AnchorType_Building { BuildingSelectBracketPosition::Top };
	Valueable<SHPStruct*> Shape { nullptr };
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	Nullable<Point2D> Shape_Spacing {};
	Valueable<bool> Shape_PercentageFrame {};
	Valueable<bool> Percentage { false };
	Nullable<bool> HideMaxValue {};
	Valueable<bool> VisibleToHouses_Observer { true };
	Valueable<AffectedHouse> VisibleToHouses { AffectedHouse::All };
	Valueable<bool> VisibleInSpecialState { true };
	Valueable<DisplayInfoType> InfoType { DisplayInfoType::Health };
	Valueable<int> InfoIndex {};
	Nullable<int> ValueScaleDivisor {};
	Valueable<bool> ValueAsTimer {};
	Valueable<DisplayShowType> ShowType { DisplayShowType::Select };

	DigitalDisplayTypeClass(const char* pTitle) : Enumerable<DigitalDisplayTypeClass>(pTitle) {}
	virtual ~DigitalDisplayTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
	bool CanShow(TechnoClass* pThis);
	void Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

private:

	void DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);
	void DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};