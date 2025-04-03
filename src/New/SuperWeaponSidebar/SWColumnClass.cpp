#include "SWColumnClass.h"
#include "SWSidebarClass.h"
#include "SWButtonClass.h"
#include "ToggleSWButtonClass.h"

#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>

#include <Memory.h>

SWColumnClass::SWColumnClass(unsigned int id, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, static_cast<GadgetFlag>(0), true)
{
	SWSidebarClass::Global()->Columns.emplace_back(this);

	this->MaxButtons = Phobos::UI::SuperWeaponSidebar_Max - (static_cast<int>(SWSidebarClass::Global()->Columns.size()) - 1);
}

bool SWColumnClass::Draw(bool forced)
{
	if (!SWSidebarClass::IsEnabled())
		return false;

	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const int cameoWidth = 60, cameoHeight = 48;
	const int cameoBackgroundWidth = Phobos::UI::SuperWeaponSidebar_Interval + cameoWidth;

	if (const auto pCenterPCX = pSideExt->SuperWeaponSidebar_CenterPCX.GetSurface())
	{
		const int cameoHarfInterval = (Phobos::UI::SuperWeaponSidebar_CameoHeight - cameoHeight) / 2;

		for (const auto button : this->Buttons)
		{
			RectangleStruct drawRect { this->Rect.X, button->Rect.Y - cameoHarfInterval, cameoBackgroundWidth, Phobos::UI::SuperWeaponSidebar_CameoHeight };
			PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pCenterPCX);
		}
	}

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
	{
		const int height = pTopPCX->Get_Height();
		RectangleStruct drawRect { this->Rect.X, this->Rect.Y, cameoBackgroundWidth, height };
		PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pTopPCX);
	}

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
	{
		const int height = pBottomPCX->Get_Height();
		RectangleStruct drawRect { this->Rect.X, this->Rect.Y + this->Rect.Height - height, cameoBackgroundWidth, height };
		PCX::Instance->BlitToSurface(&drawRect, DSurface::Composite, pBottomPCX);
	}

	for (const auto button : this->Buttons)
		button->Draw(true);

	return true;
}

void SWColumnClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	SWSidebarClass::Global()->CurrentColumn = this;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SWColumnClass::OnMouseLeave()
{
	SWSidebarClass::Global()->CurrentColumn = nullptr;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SWColumnClass::Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier)
{
	for (const auto& button : this->Buttons)
	{
		if (button->Clicked(pKey, flags, x, y, modifier))
			return true;
	}

	return false;
}

bool SWColumnClass::AddButton(int superIdx)
{
	if (const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(superIdx))
	{
		const auto pSWExt = SWTypeExtContainer::Instance.Find(pSWType);

		if (!pSWExt->SW_ShowCameo)
			return true;

		if (!Phobos::UI::SuperWeaponSidebar)
			return false;

		if (!pSWExt->SuperWeaponSidebar_Allow)
			return false;

		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		if ((pSWExt->SuperWeaponSidebar_RequiredHouses & ownerBits) == 0)
			return false;
	}
	else
	{
		return true;
	}

	auto& buttons = this->Buttons;

	if (static_cast<int>(buttons.size()) >= this->MaxButtons && !SWSidebarClass::Global()->AddColumn())
		return false;

	const int cameoWidth = 60, cameoHeight = 48;
	const auto button = GameCreate<SWButtonClass>(SWButtonClass::StartID + superIdx, superIdx, 0, 0, cameoWidth, cameoHeight);

	if (!button)
		return false;

	button->Zap();
	GScreenClass::Instance->AddButton(button);
	SWSidebarClass::Global()->SortButtons();

	if (const auto toggleButton = SWSidebarClass::Global()->ToggleButton)
		toggleButton->UpdatePosition();

	return true;
}

bool SWColumnClass::RemoveButton(int superIdx)
{
	for (auto it = this->Buttons.begin(); it != this->Buttons.end(); ++it) {
		if ((*it)->SuperIndex == superIdx) {
			AnnounceInvalidPointer(SWSidebarClass::Global()->CurrentButton, *it);
			GScreenClass::Instance->RemoveButton(*it);
			this->Buttons.erase(it);
			return true;
		}
	}

	return false;
}

void SWColumnClass::ClearButtons(bool remove)
{
	if (remove) {
		for (const auto& button : this->Buttons) {
			GScreenClass::Instance->RemoveButton(button);
		}
	}

	this->Buttons.clear();
}

void SWColumnClass::SetHeight(int height)
{
	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);

	this->Rect.Height = height;

	if (const auto pTopPCX = pSideExt->SuperWeaponSidebar_TopPCX.GetSurface())
		this->Rect.Height += pTopPCX->Get_Height();

	if (const auto pBottomPCX = pSideExt->SuperWeaponSidebar_BottomPCX.GetSurface())
		this->Rect.Height += pBottomPCX->Get_Height();
}
