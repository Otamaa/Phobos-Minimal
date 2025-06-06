#include "SWButtonClass.h"

#include <Ext/SWType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>

#include <ControlClass.h>
#include <EventClass.h>
#include <CCToolTip.h>

#include "SWSidebarClass.h"
#include "SWColumnClass.h"

#include <Misc/PhobosToolTip.h>

SWButtonClass::SWButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height)
	: ControlClass(id, x, y, width, height, (GadgetFlag::LeftPress | GadgetFlag::RightPress), true)
	, SuperIndex(superIdx)
{
	if (const auto backColumn = SWSidebarClass::Global()->Columns.back())
		backColumn->Buttons.emplace_back(this);

	this->Disabled = !SWSidebarClass::IsEnabled();
}

bool SWButtonClass::Draw(bool forced)
{
	if (!forced)
		return false;

	const auto pSurface = DSurface::Composite();
	auto bounds = pSurface->Get_Rect();
	Point2D location = { this->Rect.X, this->Rect.Y };
	RectangleStruct destRect = { location.X, location.Y, this->Rect.Width, this->Rect.Height };

	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto pSuper = pCurrent->Supers[this->SuperIndex];
	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	//const auto pHouseExt = HouseExtContainer::Instance.Find(pCurrent);

	// support for pcx cameos
	if (const auto pPCXCameo = pSWExt->SidebarPCX.GetSurface())
	{
		PCX::Instance->BlitToSurface(&destRect, pSurface, pPCXCameo);
	}
	else if (const auto pCameo = pSuper->Type->SidebarImage) // old shp cameos, fixed palette
	{
		const auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP) && strstr(pFilename, ".pcx"))
		{
			PCX::Instance->LoadFile(pFilename);

			if (const auto CameoPCX = PCX::Instance->GetSurface(pFilename))
				PCX::Instance->BlitToSurface(&destRect, pSurface, CameoPCX);
		}
		else
		{
			const auto pConvert = pSWExt->SidebarPalette ? pSWExt->SidebarPalette->GetOrDefaultConvert<PaletteManager::Mode::Default>(FileSystem::CAMEO_PAL) : FileSystem::CAMEO_PAL;
			pSurface->DrawSHP(pConvert, pCameo, 0, &location, &bounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}
	}

	if (this->IsHovering)
	{
		RectangleStruct cameoRect = { location.X, location.Y, this->Rect.Width, this->Rect.Height };
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
		pSurface->Draw_Rect(cameoRect, tooltipColor);
	}

	if (SWTypeExtData::DrawDarken(pSuper))
	{
		RectangleStruct darkenBounds { 0, 0, location.X + this->Rect.Width, location.Y + this->Rect.Height };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::DARKEN_SHP, 0, &location, &darkenBounds, BlitterFlags::bf_400 | BlitterFlags::Darken, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	const bool ready = !pSuper->IsOnHold && (pSuper->Type->UseChargeDrain ? pSuper->ChargeDrainState == ChargeDrainState::Ready : pSuper->IsCharged);
	bool drawReadiness = true;

	if (ready && this->ColumnIndex == 0)
	{
		auto& buttons = SWSidebarClass::Global()->Columns[this->ColumnIndex]->Buttons;
		const int buttonId = std::distance(buttons.begin(), std::find(buttons.begin(), buttons.end(), this));

		if (buttonId < 10)
		{
			unsigned short hotkey = 0;
			for (int i = 0; i < CommandClass::Hotkeys->IndexCount; i++)
			{
				if (CommandClass::Hotkeys->IndexTable[i].Data == SWSidebarClass::Commands[buttonId])
					hotkey = CommandClass::Hotkeys->IndexTable[i].ID;
			}

			Point2D textLoc = { location.X + this->Rect.Width / 2, location.Y };
			const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
			COMPILETIMEEVAL TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

			wchar_t buffer[64];
			Game::GetKeyboardKeyString(hotkey, buffer);

			if (std::wcslen(buffer))
			{
				pSurface->DSurfaceDrawText(buffer, &bounds, &textLoc, foreColor, 0, printType);
				drawReadiness = false;
			}
		}
	}

	if (drawReadiness)
	{
		if (const auto buffer = pSuper->NameReadiness())
		{
			Point2D textLoc = { location.X + this->Rect.Width / 2, location.Y };
			const COLORREF foreColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
			COMPILETIMEEVAL TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;

			pSurface->DrawText_Old(buffer, &bounds, &textLoc, (DWORD)foreColor, 0, (DWORD)printType);
		}
	}

	if (pSuper->ShouldDrawProgress())
	{
		Point2D loc = { location.X, location.Y };
		pSurface->DrawSHP(FileSystem::SIDEBAR_PAL, FileSystem::GCLOCK2_SHP, pSuper->GetCameoChargeState() + 1, &loc, &bounds, BlitterFlags::bf_400 | BlitterFlags::TransLucent50, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	return true;
}

void SWButtonClass::OnMouseEnter()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = true;
	SWSidebarClass::Global()->CurrentButton = this;
	SWSidebarClass::Global()->Columns[this->ColumnIndex]->OnMouseEnter();
	CCToolTip::Instance->SaveTimerDelay();
	CCToolTip::Instance->SetTimerDelay(0);
}

void SWButtonClass::OnMouseLeave()
{
	if (!SWSidebarClass::IsEnabled())
		return;

	this->IsHovering = false;
	SWSidebarClass::Global()->CurrentButton = nullptr;
	SWSidebarClass::Global()->Columns[this->ColumnIndex]->OnMouseLeave();
	CCToolTip::Instance->RestoreTimeDelay();
}

bool SWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if (!SWSidebarClass::IsEnabled())
		return false;

	if (flags & GadgetFlag::RightPress)
		DisplayClass::Instance->CurrentSWTypeIndex = -1;

	if (flags & GadgetFlag::LeftPress)
	{
		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
		VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0);
		SWTypeExtData::LauchSuper(HouseClass::CurrentPlayer->Supers.Items[this->SuperIndex]);
	}

	return this->ControlClass::Action(flags, pKey, KeyModifier::None);
}

bool SWButtonClass::LaunchSuper() const
{
	return SWTypeExtData::LauchSuper(HouseClass::CurrentPlayer->Supers.Items[this->SuperIndex]);

}