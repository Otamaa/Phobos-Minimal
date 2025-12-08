#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ScenarioClass.h>

#include <Dimensions.h>

#include <HouseClass.h>
#include <CellClass.h>

#include <TextDrawing.h>

#include <Utilities/EnumFunctions.h>
HelperedVector<FlyingStrings::Item> FlyingStrings::Data;

bool FlyingStrings::DrawAllowed(CoordStruct const& nCoords, Point2D& outPoint)
{
	if (!nCoords.IsValid())
		return false;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords)) {
		if (!pCell->IsFogged() && !pCell->IsShrouded()){
			auto[_ret, _cond] = TacticalClass::Instance->GetCoordsToClientSituation(nCoords);
			outPoint = _ret;
			return _cond;
		}
	}

	return false;
}

void FlyingStrings::Add(std::wstring text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset) {
	Data.emplace_back(Item {
		.Location = coords,
		.PixelOffset = pixelOffset,
		.CreationFrame = Unsorted::CurrentFrame,
		.Color = Drawing::ColorStructToWordRGB(color),
		.TextPrintType = TextPrintType::Center | TextPrintType::NoShadow,
		.Text = std::move(text),
	});
}

void FlyingStrings::Add(const wchar_t* text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset)
{
	Data.emplace_back(Item {
		.Location = coords,
		.PixelOffset = pixelOffset,
		.CreationFrame = Unsorted::CurrentFrame,
		.Color = Drawing::ColorStructToWordRGB(color),
		.TextPrintType = TextPrintType::Center | TextPrintType::NoShadow,
		.Text = text,
	});
}

void FlyingStrings::Add(std::wstring_view text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset)
{
	Data.emplace_back(Item {
		.Location = coords,
		.PixelOffset = pixelOffset,
		.CreationFrame = Unsorted::CurrentFrame,
		.Color = Drawing::ColorStructToWordRGB(color),
		.TextPrintType = TextPrintType::Center | TextPrintType::NoShadow,
		.Text = std::wstring(text),
	});
}

void FlyingStrings::Add(const fmt::basic_memory_buffer<wchar_t>& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset)
{
	Data.emplace_back(Item {
		.Location = coords,
		.PixelOffset = pixelOffset,
		.CreationFrame = Unsorted::CurrentFrame,
		.Color = Drawing::ColorStructToWordRGB(color),
		.TextPrintType = TextPrintType::Center | TextPrintType::NoShadow,
		.Text = std::wstring(buffer.data(), buffer.size()),
	});
}

void FlyingStrings::Add(fmt::basic_memory_buffer<wchar_t>&& buffer, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset)
{
	Data.emplace_back(Item {
		.Location = coords,
		.PixelOffset = pixelOffset,
		.CreationFrame = Unsorted::CurrentFrame,
		.Color = Drawing::ColorStructToWordRGB(color),
		.TextPrintType = TextPrintType::Center | TextPrintType::NoShadow,
		.Text = std::wstring(buffer.data(), buffer.size()),
	});
}

void FlyingStrings::AddMoneyString(bool Display, int amount, TechnoClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset, ColorStruct nOverrideColor)
{
	if (!coords.IsValid() || !Display || !owner)
		return;

	static fmt::basic_memory_buffer<wchar_t> moneyStr;
	if (EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::CurrentPlayer()))
	{
		if (owner->VisualCharacter(0, HouseClass::CurrentPlayer()) == VisualType::Hidden)
			return;

		moneyStr.clear();
		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty) {
			bool isPositive = amount > 0;
			color = isPositive ? Drawing::DefaultColors[(int)DefaultColorList::Green] : Drawing::DefaultColors[(int)DefaultColorList::Red];
			fmt::format_to(std::back_inserter(moneyStr), L"{}{}{}", amount > 0 ? L"+" : L"-", Phobos::UI::CostLabel, Math::abs(amount));
		} else {
			fmt::format_to(std::back_inserter(moneyStr), L"+{}{}", Phobos::UI::CostLabel, Math::abs(amount));
		}

		moneyStr.push_back(L'\0');
		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(moneyStr.data(), &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);

		if (const auto pBuilding = cast_to<BuildingClass* , false>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;

		FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
	}
}

void FlyingStrings::AddMoneyString(bool Display, int amount, HouseClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset, ColorStruct nOverrideColor)
{
	if (!coords.IsValid() || !Display || !owner)
		return;

	static fmt::basic_memory_buffer<wchar_t> moneyStr;
	if (EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer()))
	{
		moneyStr.clear();
		ColorStruct color = nOverrideColor;

		if (color == ColorStruct::Empty)
		{
			bool isPositive = amount > 0;
			color = isPositive ? Drawing::DefaultColors[(int)DefaultColorList::Green] : Drawing::DefaultColors[(int)DefaultColorList::Red];
			fmt::format_to(std::back_inserter(moneyStr), L"{}{}{}", amount > 0 ? L"+" : L"-", Phobos::UI::CostLabel, Math::abs(amount));
		}
		else
		{
			fmt::format_to(std::back_inserter(moneyStr), L"+{}{}", Phobos::UI::CostLabel, Math::abs(amount));
		}

		moneyStr.push_back(L'\0');
		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(moneyStr.data(), &nDim.Width, &nDim.Height, 120);
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

		if (const auto pBuilding = cast_to<BuildingClass* , false>(owner))
			coords.Z += 104 * pBuilding->Type->Height;
		else
			coords.Z += 256;

		FlyingStrings::Add(text, coords, color, pixelOffset);
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
		static fmt::basic_memory_buffer<wchar_t> buffer;
		buffer.clear();
		fmt::format_to(std::back_inserter(buffer), L"{}{}{}", sign_symbol, prefix ? prefix : Phobos::UI::CostLabel, Math::abs(amount));
		buffer.push_back(L'\0');
		Dimensions nDim {};
		BitFont::Instance->GetTextDimension(buffer.data(), &nDim.Width, &nDim.Height, 120);
		pixelOffset.X -= (nDim.Width / 2);
		FlyingStrings::Add(buffer, coords, color, pixelOffset);
	}
}

void FlyingStrings::DisplayDamageNumberString(int damage, DamageDisplayType type, CoordStruct coords, int& offset, DrawDamageMode mode , WarheadTypeClass* pWH)
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
		color = damage > 0 ? Phobos::Defines::ShieldPositiveDamageColor : Phobos::Defines::ShieldNegativeDamageColor;
		break;
	case DamageDisplayType::Intercept:
		color = damage < 0 ? Phobos::Defines::InterceptedNegativeDamageColor : Phobos::Defines::InterceptedPositiveDamageColor;
		break;
	default:
		break;
	}

	if(damage < 0)
		damage = -damage;

	int maxOffset = Unsorted::CellWidthInPixels / 2;
	int width = 0, height = 0;

	static fmt::basic_memory_buffer<wchar_t> damagestr;
	damagestr.clear();

	if(!pWH || mode != DrawDamageMode::withWH)
		fmt::format_to(std::back_inserter(damagestr), L"{}" , damage);
	else
		fmt::format_to(std::back_inserter(damagestr), L"{} [{}]", damage , PhobosCRT::StringToWideString(pWH->ID));

	damagestr.push_back(L'\0');
	BitFont::Instance->GetTextDimension(damagestr.data(), &width, &height, 120);

	if (offset >= maxOffset || offset == INT32_MIN)
		offset = -maxOffset;

	FlyingStrings::Add(damagestr, coords, color, Point2D { offset - (width / 2), 0 });

	offset = offset + width;
}

void FlyingStrings::UpdateAll()
{
	Data.remove_all_if([](FlyingStrings::Item& item) {
		if (!item.Text.empty()) {
			Point2D pos {};

			if (FlyingStrings::DrawAllowed(item.Location, pos)) {
				pos += item.PixelOffset;
				auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

				if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height)) {
					if (Unsorted::CurrentFrame > item.CreationFrame + Duration - 70) {
						pos.Y -= (Unsorted::CurrentFrame - item.CreationFrame);
					}

					TextDrawing::Simple_Text_Print_Wide(item.Text , DSurface::Temp(), &bound, &pos, item.Color, item.Back_Color, item.TextPrintType);
				}
			}

			if (!(Unsorted::CurrentFrame > item.CreationFrame + Duration || Unsorted::CurrentFrame < item.CreationFrame)) {
				return false;
			}
		}

		//always will be removed regardless
		return true;
	});

}