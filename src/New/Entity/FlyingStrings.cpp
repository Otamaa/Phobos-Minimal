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
#include <Phobos.SaveGame.h>

FlyingStrings FlyingStrings::Instance;

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

void FlyingStrings::Clear()
{
	Data.clear();
}

bool FlyingStrings::LoadGlobal(const json& root)
{
	this->Clear();

	if (root.contains("FlyingStrings")) {
		size_t dataSize = root["collectionsize"].get<size_t>();
		Debug::Log("[ExtSave] Loading FlyingStrings %d\n", dataSize);

		if (dataSize > 0) {
			Data.reserve(dataSize);

			for (auto& _item : root["collection"]) {
				auto& newPtr = Data.emplace_back();
				size_t itemdatasize = _item["itemdatasize"].get<size_t>();
				std::string encoded = _item["itemdata"].get<std::string>();

				PhobosByteStream loader(itemdatasize);
				loader.data = std::move(Base64Handler::decodeBase64(encoded, itemdatasize));
				PhobosStreamReader reader(loader);

				if (!newPtr.Load(reader, false) || !reader.ExpectEndOfBlock())
					return false;
			}
		}

		return true;
	}

	return false;
}

bool FlyingStrings::SaveGlobal(json& root)
{
	Debug::Log("[ExtSave] Saving FlyingStrings: %d\n", Data.size());

	json entry_first;

	entry_first["collectionsize"] = Data.size();

	if (!Data.empty()) {

		entry_first["collection"] = json::array();
		auto& _arrayOfData = entry_first["collection"];

		for (auto& _item : Data)
		{
			json entry_sec;
			PhobosByteStream _saver(0);
			PhobosStreamWriter writer(_saver);

			if (!_item.Save(writer))
				return false;

			entry_sec["itemdatasize"] = _saver.data.size();
			entry_sec["itemdata"] = Base64Handler::encodeBase64(_saver.data);

			_arrayOfData.push_back(std::move(entry_sec));
		}
	}

	root["FlyingStrings"] = std::move(entry_first);
	return true;
}

void FlyingStrings::AddMoneyString(bool Display, int amount, TechnoClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset, ColorStruct nOverrideColor)
{
	if (!coords.IsValid() || !Display || !owner)
		return;

	static fmt::basic_memory_buffer<wchar_t> moneyStr;
	if (EnumFunctions::CanTargetHouse(displayToHouses, owner->GetOwningHouse(), HouseClass::CurrentPlayer()))
	{
		if (!owner->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
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

		this->Add(moneyStr, coords, color, pixelOffset);
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

		this->Add(moneyStr, coords, color, pixelOffset);
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

		this->Add(text, coords, color, pixelOffset);
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
		this->Add(buffer, coords, color, pixelOffset);
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

	this->Add(damagestr, coords, color, Point2D { offset - (width / 2), 0 });

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