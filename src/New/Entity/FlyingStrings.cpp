#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>

#include <string_view>

#include <Utilities/EnumFunctions.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;

void FlyingStrings::Clear() { Data.clear(); }

bool FlyingStrings::DrawAllowed(CoordStruct const& nCoords)
{
	if (!nCoords)
		return false;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords))
	{
		Point2D screenCoords { 0,0 };
		return (!pCell->IsFogged() && !pCell->IsShrouded()) && TacticalClass::Instance->CoordsToClient(nCoords, &screenCoords);
	}

	return false;
}

void FlyingStrings::Add(const std::wstring& text, CoordStruct const& coords, ColorStruct const& color, Point2D const& pixelOffset)
{
	Item nItem { coords,pixelOffset,75, Drawing::RGB2DWORD(color) , L"" };
	nItem.Text = text;
	Data.push_back(std::move(nItem));
}

void FlyingStrings::AddMoneyString(bool Display, int const amount, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor)
{
	if (!coords || !Display || !owner)
		return;

	if (EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::CurrentPlayer()))
	{
		if (owner->VisualCharacter(0, HouseClass::CurrentPlayer()) == VisualType::Hidden)
			return;

		wchar_t moneyStr[0x20];
		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty)
		{
			bool isPositive = amount > 0;
			color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
			swprintf_s(moneyStr, L"%ls%ls%d", amount > 0 ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));
		}
		else
		{
			swprintf_s(moneyStr, L"%ls%ls%d", L"+", Phobos::UI::CostLabel, std::abs(amount));
		}

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		if (const auto pBuilding = specific_cast<BuildingClass*>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;

		if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
		{
			if (!pCell->IsFogged() && !pCell->IsShrouded())
			{
				FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
			}
		}
	}
}

void FlyingStrings::AddString(const std::wstring& text, bool Display, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor)
{
	if (text.empty() || !coords || !Display || !owner)
		return;

	if (EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::CurrentPlayer()))
	{
		if (owner->VisualCharacter(0, HouseClass::CurrentPlayer()) == VisualType::Hidden)
			return;

		wchar_t moneyStr[0x250];
		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty) {
			color = Drawing::ColorRed;
		}

		swprintf_s(moneyStr, L"%ls", text.c_str());

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		if (const auto pBuilding = specific_cast<BuildingClass*>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;


		if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
		{
			if (!pCell->IsFogged() && !pCell->IsShrouded())
			{
				FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
			}
		}
	}
}

void FlyingStrings::AddNumberString(int amount, HouseClass* owner, AffectedHouse const& displayToHouses,
	ColorStruct const& color, CoordStruct const& coords, Point2D pixelOffset, bool sign, const wchar_t* prefix)
{
	if (displayToHouses == AffectedHouse::All ||
		owner && EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer))
	{
		bool isPositive = amount > 0;
		const wchar_t* sign_symbol = (sign && amount != 0) ? (isPositive ? L"+" : L"-") : L"";
		wchar_t displayStr[0x20];
		swprintf_s(displayStr, L"%ls%ls%d", sign_symbol, prefix, std::abs(amount));

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(displayStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		if (auto const pCell = MapClass::Instance->TryGetCellAt(coords)) {
			if (!pCell->IsFogged() && !pCell->IsShrouded()) {
				FlyingStrings::Add(displayStr, coords, color, pixelOffset);
			}
		}
	}
}

void FlyingStrings::UpdateAll()
{
	if (Data.empty())
		return;

	for (int i = Data.size() - 1; i >= 0; --i) {
		auto& dataItem = Data[i];
		if (auto const pCell = MapClass::Instance->TryGetCellAt(dataItem.Location)) {
			if (!pCell->IsFogged() && !pCell->IsShrouded()) {
				auto pos = TacticalClass::Instance->CoordsToView(dataItem.Location);
				pos += dataItem.PixelOffset;
				auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

				if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height)) {
					Point2D tmp { 0,0 };
					if (dataItem.Duration < 70)
						pos.Y = dataItem.Duration + pos.Y - 70;

					Fancy_Text_Print_Wide_REF(&tmp, dataItem.Text.c_str(), DSurface::Temp(), &bound, &pos, dataItem.Color, 0, TextPrintType::Center, 1);
				}
			}
		}

		if (--dataItem.Duration <= 0) {
			Data.erase(Data.begin() + i);
		}
	}
}