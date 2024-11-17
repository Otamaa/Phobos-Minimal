#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/ShapeTextPrinter.h>

#include <Ext/Techno/Body.h>

#include "PaletteManager.h"
#include <string>

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
	this->Shape_Spacing.Read(exINI, section, "Shape.Spacing");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->VisibleToHouses_Observer.Read(exINI, section, "VisibleToHouses.Observer");
	this->VisibleToHouses.Read(exINI, section, "VisibleToHouses");
	this->InfoType.Read(exINI, section, "InfoType");
	this->ValueScaleDivisor.Read(exINI, section, "ValueScaleDivisor");
}

void DigitalDisplayTypeClass::Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	position.X += this->Offset->X;
	position.Y -= this->Offset->Y;

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
	std::wstring valueString(std::move(Percentage ?
		std::to_wstring(static_cast<int>(static_cast<double>(value) / maxValue * 100)) :
		std::to_wstring(value)
	));

	if (Percentage)
		valueString.push_back('%%');
	else if (!HideMaxValue.Get(isInfantry))
	{
		valueString += '/';
		valueString += std::to_wstring(maxValue);
	}

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

	DSurface::Composite->DSurfaceDrawText(valueString.c_str(), &rect, &position, color, 0, printType);
}

constexpr Point2D GetSpacing(const Nullable<Point2D>& shapeSpace, bool isBuilding)
{
	if (shapeSpace.isset())
		return shapeSpace.Get();

	if (isBuilding)
		return { 4, -2 };

	return { 4 , 0 };
}

struct FrameData
{
	int Base, Extra;
};

void DigitalDisplayTypeClass::DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	std::string valueString(std::move(Percentage ?
		std::to_string(static_cast<int>(static_cast<double>(value) / maxValue * 100)) :
		std::to_string(value)
	));

	Point2D spacing = GetSpacing(this->Shape_Spacing,isBuilding);
	const int pipsHeight = hasShield ? 4 : 0;

	if (Percentage)
		valueString.push_back('%');
	else if (!HideMaxValue.Get(isInfantry))
	{
		valueString += '/';
		valueString += std::to_string(maxValue);
	}

	if (AnchorType.Vertical == VerticalPosition::Top)
		position.Y -= Shape->Height + pipsHeight; // upper of healthbar and shieldbar

	switch (Align)
	{
	case TextAlign::Center:
	{
		position.X -= (int)valueString.length() * spacing.X / 2;
		position.Y += (int)valueString.length() * spacing.Y / 2;
		break;
	}
	case TextAlign::Right:
	{
		std::reverse(valueString.begin(), valueString.end());
		position.X -= spacing.X;
		spacing.X = -spacing.X;
		break;
	}
	default:
		break;
	}

	const auto ExtraFrame = GeneralUtils::GetItemByHealthRatio<FrameData>(static_cast<double>(value) / maxValue, { 0, 30 }, { 10 , 32 }, { 20 , 34 });
	ConvertClass* pPal = Palette.Get() ? Palette->GetOrDefaultConvert<PaletteManager::Mode::Temperate>(FileSystem::PALETTE_PAL) : FileSystem::PALETTE_PAL;

	ShapeTextPrintData shapeTextPrintData
	(
		Shape.Get(),
		pPal,
		ExtraFrame.Base,
		ExtraFrame.Extra,
		spacing
	);

	RectangleStruct rect = DSurface::Composite->Get_Rect_WithoutBottomBar();
	ShapeTextPrinter::PrintShape(valueString.c_str(), shapeTextPrintData, &position, &rect, DSurface::Composite);
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
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->VisibleToHouses)
		.Process(this->InfoType)
		.Process(this->ValueScaleDivisor)
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