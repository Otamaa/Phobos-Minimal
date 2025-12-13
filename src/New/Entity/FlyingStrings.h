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
#include <Utilities/VectorHelper.h>
#include <Unsorted.h>
#include <string>

class WarheadTypeClass;
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
		COLORREF Back_Color;
		TextPrintType TextPrintType;
		std::wstring Text;

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return
				Stm
				.Process(this->Location, RegisterForChange)
				.Process(this->PixelOffset, RegisterForChange)
				.Process(this->CreationFrame, RegisterForChange)
				.Process(this->Color, RegisterForChange)
				.Process(this->Back_Color, RegisterForChange)
				.Process(this->TextPrintType, RegisterForChange)
				.Process(this->Text, RegisterForChange)
				;
		}

		bool Save(PhobosStreamWriter& Stm) const
		{
			return Stm
				.Process(this->Location)
				.Process(this->PixelOffset)
				.Process(this->CreationFrame)
				.Process(this->Color)
				.Process(this->Back_Color)
				.Process(this->TextPrintType)
				.Process(this->Text)
				;
		}
	};

	static COMPILETIMEEVAL int Duration = 75;
	static HelperedVector<Item> Data;

	static bool DrawAllowed(CoordStruct const& nCoords , Point2D& outPoint);

public:

	static COMPILETIMEEVAL size_t ItemSize = sizeof(Item);
	static OPTIONALINLINE COMPILETIMEEVAL void Clear() { Data.clear(); };

	static void FORCEDINLINE Add(std::wstring text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, TextPrintType::Center | TextPrintType::NoShadow, std::move(text));
	}

	static void FORCEDINLINE Add(const wchar_t* text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, TextPrintType::Center | TextPrintType::NoShadow, text);
	}

	static void FORCEDINLINE Add(std::wstring_view text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, TextPrintType::Center | TextPrintType::NoShadow, std::move(std::wstring(text)));
	}

	static void FORCEDINLINE Add(const fmt::basic_memory_buffer<wchar_t>& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, TextPrintType::Center | TextPrintType::NoShadow, std::wstring(buffer.data(), buffer.size()));
	}

	static void FORCEDINLINE Add(fmt::basic_memory_buffer<wchar_t>&& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty) {
		Data.emplace_back(coords, pixelOffset, Unsorted::CurrentFrame(), color.ToInit(), 0, TextPrintType::Center | TextPrintType::NoShadow, std::wstring(buffer.data(), buffer.size()));
	}

	static void AddMoneyString(bool Display, int amount, TechnoClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty , ColorStruct nOverrideColor = ColorStruct::Empty);
	static void AddMoneyString(bool Display, int amount, HouseClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty, ColorStruct nOverrideColor = ColorStruct::Empty);
	static void AddString(const std::wstring& text, bool Display, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor);
	static void AddNumberString(int amount, HouseClass* owner, AffectedHouse const&  displayToHouses, ColorStruct const& color, CoordStruct const& coords, Point2D pixelOffset, bool sign, const wchar_t* prefix);
	static void UpdateAll();

	static void DisplayDamageNumberString(int damage, DamageDisplayType type, const CoordStruct coords, int& offset, DrawDamageMode mode, WarheadTypeClass* pWH = nullptr);

public:

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(Data)
			.Success()
			;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(Data)
			.Success()
			;
	}
};