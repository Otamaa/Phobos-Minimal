#include "ShowTechnoNames.h"

#include <Drawing.h>
#include <BitFont.h>

#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>

bool ShowTechnoNameCommandClass::IsActive = false;

const char* ShowTechnoNameCommandClass::GetName() const
{
	return "Show Techno Names ";
}

const wchar_t* ShowTechnoNameCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_MARKBLD", L"Show Techno Names ");
}

const wchar_t* ShowTechnoNameCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* ShowTechnoNameCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_MARKBLD_DESC", L"Show Techno Names .");
}

void ShowTechnoNameCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;
	IsActive = !IsActive;
}

void ShowTechnoNameCommandClass::AI()
{
	if (!IsActive)
		return;

	auto const pCiv = HouseExt::FindCivilianSide();

	for (auto pTech : *TechnoClass::Array)
	{
		if(pTech->InLimbo || pTech->Location == CoordStruct::Empty)
			continue;


		if (!pTech->IsOnMyView())
			continue;

		if (auto pBuilding = specific_cast<BuildingClass*>(pTech))
		{
			auto pExt = BuildingExt::ExtMap.Find(pBuilding);

			if (pExt->IsInLimboDelivery)
				continue;
		}

		std::wstring pText((size_t)(0x18 + 1), L'#');

		mbstowcs(&pText[0], pTech->get_ID(), 0x18);
		if (pText.empty())
			continue;

		Point2D pixelOffset = Point2D::Empty;
		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(pText.c_str(), &width, &height, 120);
		pixelOffset.X -= (width / 2);

		auto pos = TacticalClass::Instance->CoordsToView(pTech->Location);
		pos += pixelOffset;
		auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

		auto const pOWner = pTech->GetOwningHouse();
		auto const pResultOwner = pOWner ? pOWner : pCiv;

		if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height))
		{
			Point2D tmp { 0,0 };
			Fancy_Text_Print_Wide(tmp, pText.c_str(), DSurface::Temp(), bound, pos, ColorScheme::Array->GetItem(pResultOwner->ColorSchemeIndex), 0, TextPrintType::Center, 1);
		}
	}
}
