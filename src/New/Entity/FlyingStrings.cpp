#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>

#include <Utilities/EnumFunctions.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;

bool FlyingStrings::DrawAllowed(CoordStruct& nCoords)
{
	if (!nCoords)
		return false;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords))
	{
		Point2D screenCoords;
		return (!pCell->IsFogged() && !pCell->IsShrouded()) && TacticalClass::Instance->CoordsToClient(nCoords, &screenCoords);
	}

	return false;
}

void FlyingStrings::Add(const wchar_t* text, CoordStruct const& coords, ColorStruct const& color, Point2D const& pixelOffset)
{
	Item item {};
	item.Location = coords;
	item.PixelOffset = pixelOffset;
	item.Duration = 75;
	item.Color = Drawing::RGB2DWORD(color);
	PhobosCRT::wstrCopy(item.Text, text, 0x20);
	Data.push_back(std::move(item));
}

void FlyingStrings::AddMoneyString(bool Display , int amount, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset)
{
	if (!coords || !Display || !owner)
		return;

	if (displayToHouses == AffectedHouse::All || EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::Player))
	{
		bool isPositive = amount > 0;
		auto color = isPositive ? ColorStruct{ 0, 255, 0 } : ColorStruct{ 255, 0, 0 };
		wchar_t moneyStr[0x20];
		swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		if (owner->VisualCharacter(0, HouseClass::Player()) != VisualType::Hidden) {
			if (auto pBuilding = specific_cast<BuildingClass*>(owner))
				coords.Z += 104 * pBuilding->Type->Height;
			else
				coords.Z += 256;

			if (auto const pCell = MapClass::Instance->TryGetCellAt(coords)) {
				if (!pCell->IsFogged() && !pCell->IsShrouded()) {
					FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
				}
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
		auto pos = TacticalClass::Instance->CoordsToView(dataItem.Location);
		pos += dataItem.PixelOffset;
		auto bound = DSurface::Temp->Get_Rect();
		bound.Height -= 32;

		if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height)) {
			Point2D tmp { 0,0 };
			if (dataItem.Duration < 70)
				pos.Y = dataItem.Duration + pos.Y - 70;

			Simple_Text_Print_Wide(&tmp, dataItem.Text, DSurface::Temp(), &bound, &pos, dataItem.Color, 0, TextPrintType::Center, 1);
		}

		if (--dataItem.Duration <= 0) {
			Data.erase(Data.begin() + i);
		}
	}
}