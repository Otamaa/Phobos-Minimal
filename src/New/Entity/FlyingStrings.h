/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <vector>
#include <ColorScheme.h>
#include <CoordStruct.h>
#include <Utilities/Enum.h>

class TechnoClass;
class FlyingStrings
{
private:

	struct Item
	{
		CoordStruct Location;
		Point2D PixelOffset;
		int Duration;
		COLORREF Color;
		std::wstring Text;

	};

	static std::vector<Item> Data;

	static bool DrawAllowed(CoordStruct const& nCoords);

public:
	static void Clear() { Data.clear(); }
	static void Add(const std::wstring_view text, CoordStruct const& coords, ColorStruct const& color, Point2D const& pixelOffset);
	static void AddMoneyString(bool Display, int const amount, TechnoClass * owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty);
	static void UpdateAll();
};