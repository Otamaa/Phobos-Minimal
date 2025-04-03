#include "ToggleSWButtonClass.h"
#include "SWSidebarClass.h"
#include "SWColumnClass.h"

#include <Ext/Side/Body.h>
#include <Ext/Sidebar/Body.h>

#include <Misc/PhobosToolTip.h>

#include <CCToolTip.h>
#include <GameOptionsClass.h>

ToggleSWButtonClass::ToggleSWButtonClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, (GadgetFlag::LeftPress | GadgetFlag::LeftRelease), true)
{
	SWSidebarClass::Global()->ToggleButton = this;
}

bool ToggleSWButtonClass::Draw(bool forced)
{
	auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pTogglePCX = SWSidebarClass::IsEnabled() ? pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : pSideExt->SuperWeaponSidebar_OffPCX.GetSurface();

	if (!pTogglePCX)
		return false;

	RectangleStruct destRect { this->Rect.X, this->Rect.Y, this->Rect.Width, this->Rect.Height };
	PCX::Instance->BlitToSurface(&destRect, DSurface::Composite, pTogglePCX);

	if (this->IsHovering) {
		DSurface::Composite->Draw_Rect(destRect, Drawing::RGB_To_Int(Drawing::TooltipColor()));
	}

	return true;
}

void ToggleSWButtonClass::OnMouseEnter()
{
	auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return;

	this->IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void ToggleSWButtonClass::OnMouseLeave()
{
	auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return;

	this->IsHovering = false;
	this->IsPressed = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
	CCToolTip::Instance->MarkToRedraw(CCToolTip::Instance->CurrentToolTipData);
}

bool ToggleSWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	auto& columns = SWSidebarClass::Global()->Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);

	if (SWSidebarClass::IsEnabled() ? !pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : !pSideExt->SuperWeaponSidebar_OffPCX.GetSurface())
		return false;

	if (flags & GadgetFlag::LeftPress)
		this->IsPressed = true;

	if ((flags & GadgetFlag::LeftRelease) && this->IsPressed)
	{
		this->IsPressed = false;
		VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
		ToggleSWButtonClass::SwitchSidebar();
	}

	return this->ControlClass::Action(flags, pKey, KeyModifier::None);
}

void ToggleSWButtonClass::UpdatePosition()
{
	Point2D position = Point2D::Empty;
	auto& columns = SWSidebarClass::Global()->Columns;

	if (!columns.empty())
	{
		const auto backColumn = columns.back();
		position.X = SWSidebarClass::Global()->IsEnabled() ? backColumn->Rect.X + backColumn->Rect.Width : 0;
		position.Y = backColumn->Rect.Y + (backColumn->Rect.Height - this->Rect.Height) / 2;
	}
	else
	{
		position.X = 0;
		position.Y = (GameOptionsClass::Instance->ScreenHeight - this->Rect.Height) / 2;
	}

	this->SetPosition(position.X, position.Y);
}

bool ToggleSWButtonClass::SwitchSidebar()
{
	SidebarExtData::Instance()->SWSidebar_Enable = !SidebarExtData::Instance()->SWSidebar_Enable;

	if (const auto toggleButton = SWSidebarClass::Global()->ToggleButton)
		toggleButton->UpdatePosition();

	return SWSidebarClass::Global()->IsEnabled();
}
