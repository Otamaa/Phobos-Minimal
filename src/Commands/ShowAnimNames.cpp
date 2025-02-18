#include "ShowAnimNames.h"

#include <Drawing.h>
#include <BitFont.h>
#include <ObjectClass.h>

#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>

#include <format>

const char* ShowAnimNameCommandClass::GetName() const
{
	return "Show Anim Names";
}

const wchar_t* ShowAnimNameCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ANIMNAME", L"Show Anim Names");
}

const wchar_t* ShowAnimNameCommandClass::GetUICategory() const
{
	return CATEGORY_GUIDEBUG;
}

const wchar_t* ShowAnimNameCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ANIMNAME_DESC", L"Show Anim Names.");
}

bool ShowAnimNameIsActive;

void ShowAnimNameCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	ShowAnimNameIsActive = !ShowAnimNameIsActive;
}

bool ShowAnimNameCommandClass::IsActivated()
{
	return ShowAnimNameIsActive;
}

void ShowAnimNameCommandClass::AI()
{
	if (IsActivated())
	{
		auto const pCiv = HouseExtData::FindFirstCivilianHouse();

		for (auto pTech : *AnimClass::Array())
		{
			if (!pTech->Type)
				continue;

			if (auto pCell = pTech->GetCell())
			{
				if (pCell->IsFogged() || pCell->IsShrouded())
					continue;
			}

			//if (!IS_SAME_STR_("ROTEEMP4", pTech->Type->ID) /*&& !IS_SAME_STR_("INVISO", pTech->Type->ID)*/)
			//	continue;

			//if (auto const pTechnoOwnerObject = pTech->OwnerObject) {
			//	if (!pTechnoOwnerObject->IsAlive || !pTechnoOwnerObject->IsOnMap)
			//		Debug::LogInfo("Anim [%s - %x] Attahed to DeadObject[%s - %x] , WTF ?", pTech->Type->ID , pTech , pTechnoOwnerObject->get_ID() , pTechnoOwnerObject);
			//}

			std::wstring pText(sizeof(pTech->Type->ID) + 0x1, L'#');
			mbstowcs(&pText[0], pTech->Type->ID, 0x18 );
			Point2D pixelOffset = Point2D::Empty;
			int width = 0, height = 0;
			BitFont::Instance->GetTextDimension(pText.c_str(), &width, &height, 120);
			pixelOffset.X -= (width / 2);

			auto pos = TacticalClass::Instance->CoordsToView(pTech->Location);
			pos += pixelOffset;
			auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();

			auto const pResultOwner = pTech->Owner ? pTech->Owner : pCiv;

			if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height))
			{
				Point2D tmp { 0,0 };
				ColorStruct _color{};
				ColorScheme::Array->Items[pResultOwner->ColorSchemeIndex]->BaseColor.ToColorStruct(&_color);

				Simple_Text_Print_Wide(&tmp, pText.c_str(), DSurface::Temp(), &bound, &pos, _color.ToInit() , 0, TextPrintType::Center, 1);
			}
		}
	}
}
