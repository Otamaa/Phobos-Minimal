#pragma once

#include <Utilities/TemplateDef.h>
#include <Utilities/Anchor.h>
#include <Utilities/Constructs.h>
#include <New/Type/PaletteManager.h>

class HugeBar
{
private:

	static std::vector<TechnoClass*> Technos;

public:

	static void InitializeHugeBar(TechnoClass* pTechno);
	static void ProcessHugeBar();

public:

	Valueable<double> HugeBar_RectWidthPercentage { 0.82 };
	Valueable<Point2D> HugeBar_RectWH { { -1, 30 } };
	Damageable<ColorStruct> HugeBar_Pips_Color1 {};
	Damageable<ColorStruct> HugeBar_Pips_Color2 {};

	Valueable<SHPCaches*> HugeBar_Shape {};
	Valueable<SHPCaches*> HugeBar_Pips_Shape {};
	CustomPalette HugeBar_Palette {};
	CustomPalette HugeBar_Pips_Palette {};
	Damageable<int> HugeBar_Frame { -1 };
	Damageable<int> HugeBar_Pips_Frame { -1 };
	Valueable<int> HugeBar_Pips_Spacing {};

	Valueable<Point2D> HugeBar_Offset {};
	Nullable<Point2D> HugeBar_Pips_Offset {};
	Valueable<int> HugeBar_Pips_Num {};

	Damageable<ColorStruct> Value_Text_Color {};

	Valueable<SHPCaches*> Value_Shape {};
	CustomPalette Value_Palette {};
	Valueable<int> Value_Num_BaseFrame {};
	Valueable<int> Value_Sign_BaseFrame { 30 };
	Valueable<int> Value_Shape_Spacing { 8 };

	Valueable<bool> DisplayValue { true };
	Valueable<bool> Value_Percentage {};
	Valueable<Point2D> Value_Offset {};
	Anchor Anchor { HorizontalPosition::Center, VerticalPosition::Top };
	DisplayInfoType InfoType {};

	Valueable<bool> VisibleToHouses_Observer { true };
	Valueable<AffectedHouse> VisibleToHouses { AffectedHouse::All };

public:

	HugeBar() = default;
	HugeBar(DisplayInfoType infoType);
	MOVEABLE_ONLY(HugeBar);
	~HugeBar() = default;

	void LoadFromINI(CCINIClass* pINI);

	void DrawHugeBar(int iCurrent, int iMax);
	void HugeBar_DrawValue(Point2D& posDraw, int iCurrent, int iMax);

	static void InvalidatePointer(void* ptr, bool removed);


	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static void Clear();

private:

	template <typename T>
	bool Serialize(T& stm);
};

template <>
struct Savegame::ObjectFactory<HugeBar>
{
	std::unique_ptr<HugeBar> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<HugeBar>();
	}
};