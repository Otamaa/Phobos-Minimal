#include "SWColumnClass.h"
#include "SWSidebarClass.h"
#include "SWButtonClass.h"
#include "ToggleSWButtonClass.h"

#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>

#include <Memory.h>

SWColumnClass::SWColumnClass(int maxButtons, int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, static_cast<GadgetFlag>(0), true)
	, MaxButtons(maxButtons)
{
	SWSidebarClass::Global()->Columns.emplace_back(this);
	this->Disabled = !SWSidebarClass::IsEnabled();
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

		for (const auto &button : this->Buttons)
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

	for (const auto& button : this->Buttons)
		button->Draw(true);

	return true;
}

void SWColumnClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled() || ScenarioClass::Instance->UserInputLocked)
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
	return false;
}

bool SWColumnClass::AddButton(int superIdx)
{
	auto& buttons = this->Buttons;
	const int buttonCount = static_cast<int>(buttons.size());
	auto sidebar = SWSidebarClass::Global();

	if (buttonCount >= this->MaxButtons && !sidebar->AddColumn())
	{
		const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

		auto Compare = [ownerBits](const int left, const int right)
		{
			const auto pExtA = SWTypeExtContainer::Instance.Find(SuperWeaponTypeClass::Array->GetItemOrDefault(left));
			const auto pExtB = SWTypeExtContainer::Instance.Find(SuperWeaponTypeClass::Array->GetItemOrDefault(right));

			if (pExtB && (pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits) && (!pExtA || !(pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits)))
				return false;

			if ((!pExtB || !(pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits)) && pExtA && (pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits))
				return true;

			return BuildType::SortsBefore(AbstractType::Special, left, AbstractType::Special, right);
		};

		const int backIdx = buttons.back()->SuperIndex;

		if (!Compare(superIdx, backIdx))
			return false;

		this->RemoveButton(backIdx);
		sidebar->DisableEntry = true;
		SidebarClass::Instance->AddCameo(AbstractType::Special, backIdx);
		SidebarClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(AbstractType::Super, backIdx, false));
		sidebar->DisableEntry = false;
	}

	const int cameoWidth = 60, cameoHeight = 48;
	const auto button = GameCreate<SWButtonClass>(superIdx, 0, 0, cameoWidth, cameoHeight);

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
	auto& buttons = this->Buttons;
	auto sidebar = SWSidebarClass::Global();

	const auto it = std::ranges::find_if(buttons,
		[superIdx](SWButtonClass* const button)
		{ return button->SuperIndex == superIdx; });

	if (it == buttons.end())
		return false;

	AnnounceInvalidPointer(sidebar->CurrentButton, *it);

	ScenarioExtData::Instance()->SWSidebar_Indices.erase(superIdx);
	const auto pButton = *it;
	GScreenClass::Instance->RemoveButton(pButton);
	GameDelete<true,false>(pButton);
	buttons.erase(it);
	return true;
}

void SWColumnClass::ClearButtons(bool remove)
{
	if (remove) {
		for (const auto& button : this->Buttons) {
			GScreenClass::Instance->RemoveButton(button);
			GameDelete<true, false>(button);
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