#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Anchor.h>

class PaletteManager;
class DigitalDisplayTypeClass final : public Enumerable<DigitalDisplayTypeClass>
{
public:
	Damageable<ColorStruct> Text_Color;
	Valueable<bool> Text_Background;
	Valueable<Point2D> Offset;
	Nullable<Point2D> Offset_ShieldDelta;
	Valueable<TextAlign> Align;
	Anchor AnchorType;
	Valueable<BuildingSelectBracketPosition> AnchorType_Building;
	Valueable<SHPStruct*> Shape;
	Valueable<PaletteManager*> Palette;
	Nullable<Point2D> Shape_Spacing;
	Valueable<bool> Shape_PercentageFrame;
	Valueable<bool> Percentage;
	Nullable<bool> HideMaxValue;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<bool> VisibleInSpecialState;
	Valueable<DisplayInfoType> InfoType;
	Valueable<int> InfoIndex;
	Nullable<int> ValueScaleDivisor;
	Valueable<bool> ValueAsTimer;

	DigitalDisplayTypeClass(const char* pTitle) : Enumerable<DigitalDisplayTypeClass>(pTitle)
		, Text_Color({ 0, 255, 0 }, { 255,255,0 }, { 255,0,0 })
		, Text_Background(false)
		, Offset({ 0, 0 })
		, Offset_ShieldDelta()
		, Align(TextAlign::Right)
		, AnchorType(HorizontalPosition::Right, VerticalPosition::Top)
		, AnchorType_Building(BuildingSelectBracketPosition::Top)
		, Shape(nullptr)
		, Palette()
		, Shape_Spacing()
		, Shape_PercentageFrame()
		, Percentage(false)
		, HideMaxValue()
		, VisibleToHouses_Observer(true)
		, VisibleToHouses(AffectedHouse::All)
		, VisibleInSpecialState(true)
		, InfoType(DisplayInfoType::Health)
		, InfoIndex()
		, ValueScaleDivisor()
		, ValueAsTimer()
	{ }


	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

private:

	void DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);
	void DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};