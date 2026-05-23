/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <Base/Always.h>

#include <ColorScheme.h>
#include <CoordStruct.h>
#include <Utilities/Enum.h>
#include <Utilities/VectorHelper.h>
#include <Unsorted.h>
#include <string>

#include <Utilities/ClassInterfaces.h>
#include <Lib/fmt/format.h>

class PhobosStreamReader;
class PhobosStreamWriter;
class WarheadTypeClass;
class TechnoClass;
class HouseClass;
class CellClass;
class FlyingStrings : public GlobalSaveable
{
private:
	const char* ClassName = CLASS_NAME(FlyingStrings);

public:

	struct Item
	{
		CoordStruct Location;
		Point2D PixelOffset;
		int CreationFrame;
		COLORREF Color;
		COLORREF Back_Color;
		int TPrintType;
		std::wstring Text;

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
		bool Save(PhobosStreamWriter& Stm) const;
	};

	static COMPILETIMEEVAL int Duration = 75;
	HelperedVector<Item> Data;

	static bool DrawAllowed(CoordStruct const& nCoords , Point2D& outPoint);

public:

	static FlyingStrings Instance;
	static COMPILETIMEEVAL size_t ItemSize = sizeof(Item);

	void FORCEDINLINE Add(std::wstring text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, int(TextPrintType::Center | TextPrintType::NoShadow), std::move(text));
	}

	void FORCEDINLINE Add(const wchar_t* text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, int(TextPrintType::Center | TextPrintType::NoShadow), text);
	}

	void FORCEDINLINE Add(std::wstring_view text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, int(TextPrintType::Center | TextPrintType::NoShadow), std::move(std::wstring(text)));
	}

	void FORCEDINLINE Add(const fmt::basic_memory_buffer<wchar_t>& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, int(TextPrintType::Center | TextPrintType::NoShadow), std::wstring(buffer.data(), buffer.size()));
	}

	void FORCEDINLINE Add(fmt::basic_memory_buffer<wchar_t>&& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, int(TextPrintType::Center | TextPrintType::NoShadow), std::wstring(buffer.data(), buffer.size()));
	}

	void AddMoneyString(bool Display, int amount, TechnoClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty , ColorStruct nOverrideColor = ColorStruct::Empty);
	void AddMoneyString(bool Display, int amount, HouseClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty, ColorStruct nOverrideColor = ColorStruct::Empty);
	void AddString(const std::wstring& text, bool Display, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor);
	void AddNumberString(int amount, HouseClass* owner, AffectedHouse const&  displayToHouses, ColorStruct const& color, CoordStruct const& coords, Point2D pixelOffset, bool sign, const wchar_t* prefix);
	void UpdateAll();

	void DisplayDamageNumberString(int damage, DamageDisplayType type, const CoordStruct coords, int& offset, DrawDamageMode mode, WarheadTypeClass* pWH = nullptr);

public:

	FlyingStrings() = default;
	virtual ~FlyingStrings() = default;

	virtual bool SaveGlobal(PhobosStreamWriter& root);
	virtual bool LoadGlobal(PhobosStreamReader& root);
	virtual void Clear();
};