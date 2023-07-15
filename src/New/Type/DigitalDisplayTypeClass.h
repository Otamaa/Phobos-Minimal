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
	Valueable<bool> Percentage;
	Nullable<bool> HideMaxValue;
	Valueable<bool> VisibleToHouses_Observer;
	Valueable<AffectedHouse> VisibleToHouses;
	Valueable<DisplayInfoType> InfoType;

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
		, Percentage(false)
		, HideMaxValue()
		, VisibleToHouses_Observer(true)
		, VisibleToHouses(AffectedHouse::All)
		, InfoType(DisplayInfoType::Health)
	{
	}

	virtual ~DigitalDisplayTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	void Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

private:

	void DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);
	void DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield);

	template <typename T>
	void Serialize(T& Stm);
};