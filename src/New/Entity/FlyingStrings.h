/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <Phobos.h>

#include <ColorScheme.h>
#include <CoordStruct.h>
#include <Utilities/Enum.h>

class TechnoClass;
class HouseClass;
class CellClass;
class FlyingStrings
{
private:

	struct Item
	{
		CoordStruct Location;
		Point2D PixelOffset;
		int CreationFrame;
		COLORREF Color;
		TextPrintType TextPrintType;
		wchar_t Text[0x20];
	};

	static constexpr int Duration = 75;
	inline static std::vector<Item> Data;

	static bool DrawAllowed(CoordStruct const& nCoords , Point2D& outPoint);

public:
	static inline constexpr void Clear() { Data.clear(); };
	static void Add(const wchar_t* text, CoordStruct const& coords, ColorStruct const& color, Point2D const& pixelOffset);
	static void AddMoneyString(bool Display, int const amount, TechnoClass * owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty , const ColorStruct& nOverrideColor = ColorStruct::Empty);
	static void AddMoneyString(bool Display, int const amount, HouseClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty, const ColorStruct& nOverrideColor = ColorStruct::Empty);
	static void AddString(const std::wstring& text, bool Display, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor);
	static void AddNumberString(int amount, HouseClass* owner, AffectedHouse const&  displayToHouses, ColorStruct const& color,
		CoordStruct const& coords, Point2D pixelOffset = Point2D::Empty, bool sign = true, const wchar_t* prefix = Phobos::UI::CostLabel);
	static void UpdateAll();

	static void DisplayDamageNumberString(int damage, DamageDisplayType type, const CoordStruct coords, int& offset);
};