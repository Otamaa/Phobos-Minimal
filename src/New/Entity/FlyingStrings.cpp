#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>
#include <Dimensions.h>

#include <HouseClass.h>
#include <CellClass.h>

#include <Utilities/EnumFunctions.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;

bool FlyingStrings::DrawAllowed(CoordStruct const& nCoords, Point2D& outPoint)
{
	if (!nCoords.IsValid())
		return false;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords)) {
		return (!pCell->IsFogged() && !pCell->IsShrouded()) && TacticalClass::Instance->CoordsToClient(nCoords, &outPoint);
	}

	return false;
}

void FlyingStrings::Add(const wchar_t* text, CoordStruct const& coords, ColorStruct const& color, Point2D const& pixelOffset)
{
	Item item { coords, pixelOffset, Unsorted::CurrentFrame, Drawing::RGB2DWORD(color), TextPrintType::Center | TextPrintType::NoShadow, L"" };
	PhobosCRT::wstrCopy(item.Text, text, 0x20);
	Data.push_back(item);
}

void FlyingStrings::AddMoneyString(bool Display, int const amount, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor)
{
	if (!coords.IsValid() || !Display || !owner)
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
			color = isPositive ? Drawing::DefaultColors[(int)DefaultColorList::Green] : Drawing::DefaultColors[(int)DefaultColorList::Red];
			swprintf_s(moneyStr, L"%ls%ls%d", amount > 0 ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));
		}
		else
		{
			swprintf_s(moneyStr, L"%ls%ls%d", L"+", Phobos::UI::CostLabel, std::abs(amount));
		}

		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(moneyStr, &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);

		if (const auto pBuilding = specific_cast<BuildingClass*>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;

		FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
	}
}

void FlyingStrings::AddMoneyString(bool Display, int const amount, HouseClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor)
{
	if (!coords.IsValid() || !Display || !owner)
		return;

	if (EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer()))
	{
		wchar_t moneyStr[0x20];
		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty)
		{
			bool isPositive = amount > 0;
			color = isPositive ? Drawing::DefaultColors[(int)DefaultColorList::Green] : Drawing::DefaultColors[(int)DefaultColorList::Red];
			swprintf_s(moneyStr, L"%ls%ls%d", amount > 0 ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));
		}
		else
		{
			swprintf_s(moneyStr, L"%ls%ls%d", L"+", Phobos::UI::CostLabel, std::abs(amount));
		}

		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(moneyStr, &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);
		coords.Z += 256;

		FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
	}
}

void FlyingStrings::AddString(const std::wstring& text, bool Display, TechnoClass* owner, AffectedHouse const& displayToHouses, CoordStruct coords, Point2D pixelOffset, const ColorStruct& nOverrideColor)
{
	if (text.empty() || !coords.IsValid() || !Display || !owner)
		return;

	if (EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::CurrentPlayer()))
	{
		if (owner->VisualCharacter(0, HouseClass::CurrentPlayer()) == VisualType::Hidden)
			return;

		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty) {
			color = Drawing::DefaultColors[(int)DefaultColorList::Red];
		}

		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(text.c_str(), &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);

		if (const auto pBuilding = specific_cast<BuildingClass*>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;

		FlyingStrings::Add(text.c_str(), coords, color, pixelOffset);
	}
}

void FlyingStrings::AddNumberString(int amount, HouseClass* owner, AffectedHouse const& displayToHouses,
	ColorStruct const& color, CoordStruct const& coords, Point2D pixelOffset, bool sign, const wchar_t* prefix)
{
	if (displayToHouses == AffectedHouse::All ||
		owner && EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer))
	{
		const bool isPositive = amount > 0;
		const wchar_t* sign_symbol = (sign && amount != 0) ? (isPositive ? L"+" : L"-") : L"";
		wchar_t displayStr[0x20];
		swprintf_s(displayStr, L"%ls%ls%d", sign_symbol, prefix, std::abs(amount));
		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(displayStr, &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);
		FlyingStrings::Add(displayStr, coords, color, pixelOffset);
	}
}

static constexpr ColorStruct ShieldDamagedNumber[2] {
	{ 0, 160, 255 } , { 0, 255, 230 }
};

static constexpr ColorStruct InterceptDamageNumber[2] {
	{ 255, 128, 128 } , { 128, 255, 128 }
};

void FlyingStrings::DisplayDamageNumberString(int damage, DamageDisplayType type, CoordStruct coords, int& offset)
{
	if (damage == 0)
		return;

	ColorStruct color  = Drawing::DefaultColors[(int)DefaultColorList::White];

	switch (type)
	{
	case DamageDisplayType::Regular:
		color = damage > 0 ? Drawing::DefaultColors[(int)DefaultColorList::Red] : Drawing::DefaultColors[(int)DefaultColorList::Green];
		break;
	case DamageDisplayType::Shield:
		color = damage > 0 ? ShieldDamagedNumber[0] : ShieldDamagedNumber[1];
		break;
	case DamageDisplayType::Intercept:
		color = damage > 0 ? InterceptDamageNumber[0] : InterceptDamageNumber[1];
		break;
	default:
		break;
	}

	int maxOffset = Unsorted::CellWidthInPixels / 2;
	int width = 0, height = 0;
	const std::wstring damagestr(std::to_wstring(damage));

	BitFont::Instance->GetTextDimension(damagestr.c_str(), &width, &height, 120);

	if (offset >= maxOffset || offset == INT32_MIN)
		offset = -maxOffset;

	FlyingStrings::Add(damagestr.c_str(), coords, color, Point2D { offset - (width / 2), 0 });

	offset = offset + width;
}

void FlyingStrings::UpdateAll()
{

	for (int i = static_cast<int>(Data.size()) - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

		if (dataItem.Text[0])
		{
			Point2D pos {};
			Point2D tmp {};

			if (FlyingStrings::DrawAllowed(dataItem.Location, pos))
			{
				pos += dataItem.PixelOffset;
				auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

				if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height))
				{
					if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration - 70)
					{
						pos.Y -= (Unsorted::CurrentFrame - dataItem.CreationFrame);
						Fancy_Text_Print_Wide_REF(&tmp, dataItem.Text, DSurface::Temp(), &bound, &pos, dataItem.Color, 0, dataItem.TextPrintType, 1);
					}
					else
					{
						Fancy_Text_Print_Wide_REF(&tmp, dataItem.Text, DSurface::Temp(), &bound, &pos, dataItem.Color, 0, dataItem.TextPrintType, 1);
					}
				}
			}
		}

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration || Unsorted::CurrentFrame < dataItem.CreationFrame){
			Data.erase(Data.begin() + i);
		}
	}
}