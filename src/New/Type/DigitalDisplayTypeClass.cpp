#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/ShapeTextPrinter.h>

#include <Ext/Techno/Body.h>

#include "PaletteManager.h"
#include <string>

Enumerable<DigitalDisplayTypeClass>::container_t Enumerable<DigitalDisplayTypeClass>::Array;

const char* Enumerable<DigitalDisplayTypeClass>::GetMainSection()
{
	return "DigitalDisplayTypes";
}

void DigitalDisplayTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name.c_str();

	INI_EX exINI(pINI);

	this->Text_Color.Read(exINI, section, "Text.Color.%s");
	this->Text_Background.Read(exINI, section, "Text.Background");
	this->Offset.Read(exINI, section, "Offset");
	this->Offset_ShieldDelta.Read(exINI, section, "Offset.ShieldDelta");
	this->Align.Read(exINI, section, "Align");
	this->AnchorType.Read(exINI, section, "Anchor.%s");
	this->AnchorType_Building.Read(exINI, section, "Anchor.Building");
	this->Shape.Read(exINI, section, "Shape");
	this->Palette.Read(exINI, section, "Palette");
	this->Shape_PercentageFrame.Read(exINI, section, "Shape.PercentageFrame");
	this->Shape_Spacing.Read(exINI, section, "Shape.Spacing");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->VisibleToHouses_Observer.Read(exINI, section, "VisibleToHouses.Observer");
	this->VisibleInSpecialState.Read(exINI, section, "VisibleInSpecialState");
	this->VisibleToHouses.Read(exINI, section, "VisibleToHouses");
	this->InfoType.Read(exINI, section, "InfoType");
	this->InfoIndex.Read(exINI, section, "InfoIndex");
	this->ValueScaleDivisor.Read(exINI, section, "ValueScaleDivisor");
	this->ValueAsTimer.Read(exINI, section, "ValueAsTimer");
	this->ShowType.Read(exINI, section, "ShowType");
}

bool DigitalDisplayTypeClass::CanShow(TechnoClass* pThis)
{

	if (HouseClass::IsCurrentPlayerObserver() && !this->VisibleToHouses_Observer)
	{
		return false;
	}
	else if (!EnumFunctions::CanTargetHouse(this->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer))
	{
		return false;
	}

	if (!this->VisibleInSpecialState && (pThis->TemporalTargetingMe || pThis->IsIronCurtained()))
		return false;

	const DisplayShowType flags = this->ShowType;

	if (flags == DisplayShowType::All)
		return true;

	DisplayShowType current = pThis->IsMouseHovering ? DisplayShowType::CursorHover : DisplayShowType::None;

	if (pThis->IsSelected)
		current |= DisplayShowType::Selected;

	if (current != DisplayShowType::None) // is hovering | is selected
		return (current & flags) != DisplayShowType::None;

	return (flags & DisplayShowType::Idle) != DisplayShowType::None; // not hovering & not selected

}

void DigitalDisplayTypeClass::Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	position.X += this->Offset->X;
	position.Y += this->Offset->Y;

	if (hasShield)
	{
		if (this->Offset_ShieldDelta.isset())
		{
			position.X += this->Offset_ShieldDelta->X;
			position.Y += this->Offset_ShieldDelta->Y;
		}
		else if (this->InfoType == DisplayInfoType::Shield)
		{
			position.Y -= 10;	//default
		}
	}

	if (isBuilding)
	{
		if (this->AnchorType_Building == BuildingSelectBracketPosition::Top)
			position.Y -= 4; //Building's pips height
		else if (this->AnchorType_Building == BuildingSelectBracketPosition::LeftTop || this->AnchorType_Building == BuildingSelectBracketPosition::LeftBottom)
			position.X -= 8; //anchor to the left border of pips
	}

	if (this->Shape != nullptr)
		this->DisplayShape(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
	else
		this->DisplayText(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
}

void DigitalDisplayTypeClass::DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	fmt::basic_memory_buffer<wchar_t> wbuf;

	if (!ValueAsTimer) {
		const int minute = value / 60;

		if (const int hour = minute / 60)
			fmt::format_to(std::back_inserter(wbuf), L"{:02}:{:02}:{:02}", hour, minute % 60, value % 60);
		else
			fmt::format_to(std::back_inserter(wbuf), L"{:02}:{:02}", minute % 60, value % 60);
	}
	else
	{
		if (Percentage){
			fmt::format_to(std::back_inserter(wbuf), L"%{}", static_cast<int>(static_cast<double>(value) / maxValue * 100));
		} else if (!HideMaxValue.Get(isInfantry)) {
			fmt::format_to(std::back_inserter(wbuf), L"{}/{}", value, maxValue);
		} else {
			fmt::format_to(std::back_inserter(wbuf), L"{}", value);
		}
	}

	wbuf.push_back(L'\0');
	double ratio = static_cast<double>(value) / maxValue;
	COLORREF color = Drawing::RGB_To_Int(this->Text_Color.Get(ratio , RulesClass::Instance->ConditionYellow, RulesClass::Instance->ConditionRed));
	RectangleStruct rect = DSurface::Composite->Get_Rect_WithoutBottomBar();
	const int textHeight = 12;
	const int pipsHeight = hasShield ? 4 : 0;

	if (this->AnchorType.Vertical == VerticalPosition::Top)
		position.Y -= textHeight + pipsHeight; // upper of healthbar and shieldbar

	TextPrintType printType = static_cast<TextPrintType>(this->Align.Get())
		| TextPrintType::FullShadow
		| (this->Text_Background ? TextPrintType::Background : TextPrintType::LASTPOINT);

	DSurface::Composite->DSurfaceDrawText(wbuf.data(), &rect, &position, color, 0, printType);
}

COMPILETIMEEVAL Point2D GetSpacing(const Nullable<Point2D>& shapeSpace, bool isBuilding)
{
	if (shapeSpace.isset())
		return shapeSpace.Get();

	return isBuilding ? Point2D{ 4, -2 } : Point2D{ 4 , 0 };
}

struct FrameData
{
	int Base, Extra;
};

void DigitalDisplayTypeClass::DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	double ratio = static_cast<double>(value) / maxValue;
	std::string valueString("");

	if (!Shape_PercentageFrame)
	{
		if (!ValueAsTimer)
		{
			if (Percentage)

				valueString += std::move(GeneralUtils::IntToDigits(static_cast<int>(ratio * 100))) + '%';
			else if (HideMaxValue.Get(isInfantry))
				valueString += std::move(GeneralUtils::IntToDigits(value));
			else
				valueString += std::move(GeneralUtils::IntToDigits(value)) + '/' + std::move(GeneralUtils::IntToDigits(maxValue));
		}
		else
		{
			const int minute = value / 60;
			const int hour = minute / 60;

			if (hour)
				valueString += std::move(GeneralUtils::IntToDigits(hour)) + '%';

			const int min = minute % 60;

			if (!(min / 10) && hour)
				valueString += '0';

			valueString += std::move(GeneralUtils::IntToDigits(min)) + '%';

			const int sec = value % 60;

			if (!(sec / 10))
				valueString += '0';

			valueString += std::move(GeneralUtils::IntToDigits(sec));
		}
	}

	Point2D spacing = GetSpacing(this->Shape_Spacing,isBuilding);
	const int pipsHeight = hasShield ? 4 : 0;

	if (AnchorType.Vertical == VerticalPosition::Top)
		position.Y -= Shape->Height + pipsHeight; // upper of healthbar and shieldbar

	switch (Align)
	{
	case TextAlign::Center:
	{
#ifdef _old
		position.X -= (int)valueString.length() * spacing.X / 2;
		position.Y += (int)valueString.length() * spacing.Y / 2;
#else
		if (Shape_PercentageFrame) {
			position.X -= static_cast<int>(Shape->Width) / 2;
		} else {
			position.X -= static_cast<int>(valueString.length()) * spacing.X / 2;
			position.Y += static_cast<int>(valueString.length()) * spacing.Y / 2;
		}
#endif
		break;
	}
	case TextAlign::Right:
	{

#ifdef _old
		std::reverse(valueString.begin(), valueString.end());
		position.X -= spacing.X;
		spacing.X = -spacing.X;
#else
		if (Shape_PercentageFrame)
			position.X -= static_cast<int>(Shape->Width);
		else
			position.X -= spacing.X;
#endif
		break;
	}
	default:
		break;
	}

	const auto ExtraFrame = GeneralUtils::GetItemByHealthRatio<FrameData>(static_cast<double>(value) / maxValue, { 0, 30 }, { 10 , 32 }, { 20 , 34 });
	ConvertClass* pPal = Palette.GetConvert() ? Palette.GetConvert() : FileSystem::PALETTE_PAL;

	ShapeTextPrintData shapeTextPrintData
	(
		Shape.Get(),
		pPal,
		ExtraFrame.Base,
		ExtraFrame.Extra,
		spacing
	);

	RectangleStruct rect = DSurface::Composite->Get_Rect_WithoutBottomBar();
	if (Shape_PercentageFrame)
	{
		DSurface::Composite->DrawSHP
		(
			pPal,
			Shape.Get(),
			static_cast<int>(std::clamp((int)ratio, 0, 1) * (Shape->Frames - 1) + 0.5),
			&position, &rect, BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
	}
	else
	{
		ShapeTextPrinter::PrintShape(valueString.c_str(), shapeTextPrintData, &position, &rect, DSurface::Composite);
	}
}

template <typename T>
void DigitalDisplayTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Text_Color)
		.Process(this->Text_Background)
		.Process(this->Offset)
		.Process(this->Offset_ShieldDelta)
		.Process(this->Align)
		.Process(this->AnchorType)
		.Process(this->AnchorType_Building)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Shape_Spacing)
		.Process(this->Shape_PercentageFrame)
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->VisibleToHouses)
		.Process(this->VisibleInSpecialState)
		.Process(this->InfoType)
		.Process(this->InfoIndex)
		.Process(this->ValueScaleDivisor)
		.Process(this->ValueAsTimer)
		.Process(this->ShowType)
		;
}

void DigitalDisplayTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void DigitalDisplayTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}