#include "SelectedCameoClass.h"
#include "SelectedInfoClass.h"

#include "SpawnManagerClass.h"

#include <Phobos.h>
#include <GScreenClass.h>
#include <MouseClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

SelectedCameoClass::SelectedCameoClass(int id, int x, int y)
	: GadgetClass(x, y, 60, 48, (GadgetFlag::LeftPress | GadgetFlag::RightPress), false)
	, ID(id)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || SelectedInfoClass::Instance.SingleSelect;
}

bool SelectedCameoClass::Draw(bool forced)
{
	return false;
}

void SelectedCameoClass::OnMouseEnter()
{
	this->Rect.Y -= 10;
	this->Rect.Height += 10;
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedCameoClass::OnMouseLeave()
{
	this->Rect.Y += 10;
	this->Rect.Height -= 10;
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SelectedCameoClass::Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier)
{
	if (this->Disabled)
		return false;

	auto& seIns = SelectedInfoClass::Instance;

	if (seIns.ShouldUpdate)
		seIns.UpdateSelected();

	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
	const auto& seCST = seIns.CurrentSelectTechno;

	if (seIns.CurrentSelectCameo.size() == 1 || Phobos::Config::SelectedDisplay_Expand)
	{
		const auto pSelect = seCST[this->ID + seIns.Current]->This();

		if (flags & GadgetFlag::LeftPress)
		{
			for (const auto& pCurrent : seCST)
			{
				if (pCurrent->This() != pSelect)
					pCurrent->This()->Deselect();
			}
		}
		else if (flags & GadgetFlag::RightPress)
		{
			pSelect->Deselect();
		}
	}
	else
	{
		const auto pTypeExt = seIns.CurrentSelectCameo[this->ID + seIns.Current].TypeExt;
		const auto groupID = pTypeExt->GetSelectionGroupID();

		if (flags & GadgetFlag::LeftPress)
		{
			if (static_cast<int>(modifier) & static_cast<int>(KeyModifier::Shift))
			{
				for (const auto& pCurrent : seCST)
				{
					if (pCurrent->TypeExtData->GetSelectionGroupID() != groupID)
						pCurrent->This()->Deselect();
				}
			}
			else
			{
				std::vector<TechnoClass*> selects;

				for (const auto& pCurrent : seCST)
				{
					if (pCurrent->TypeExtData->GetSelectionGroupID() == groupID)
					{
						selects.push_back(pCurrent->This());
						continue;
					}

					pCurrent->This()->Deselect();
				}

				const int size = selects.size();
				const int random = Unsorted::CurrentFrame() % size;

				for (int i = 0; i < size; ++i)
				{
					if (i != random)
						selects[i]->Deselect();
				}
			}
		}
		else if (flags & GadgetFlag::RightPress)
		{
			if (static_cast<int>(modifier) & static_cast<int>(KeyModifier::Shift))
			{
				for (const auto& pCurrent : seCST)
				{
					if (pCurrent->TypeExtData->GetSelectionGroupID() == groupID)
						pCurrent->This()->Deselect();
				}
			}
			else
			{
				std::vector<TechnoClass*> selects;

				for (const auto& pCurrent : seCST)
				{
					if (pCurrent->TypeExtData->GetSelectionGroupID() == groupID)
						selects.push_back(pCurrent->This());
				}

				selects[Unsorted::CurrentFrame() % selects.size()]->Deselect();
			}
		}
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void SelectedCameoClass::DrawInfo() const
{
	if (this->Disabled)
		return;

	auto drawCameo = [this](TechnoTypeExtData* pTypeExt)
	{
		if (const auto CameoPCX = pTypeExt->CameoPCX.GetSurface())
		{
			RectangleStruct drawRect { this->Rect.X, this->Rect.Y, 60, 48 };
			PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
		}
		else if (const auto pSHP = pTypeExt->This()->GetCameo())
		{
			if (const auto MissingCameoPCX = SelectedInfoClass::SearchMissingCameo(pTypeExt->This()->WhatAmI(), pSHP))
			{
				RectangleStruct drawRect { this->Rect.X, this->Rect.Y, 60, 48 };
				PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, MissingCameoPCX);
			}
			else
			{
				Point2D position { this->Rect.X, this->Rect.Y };
				RectangleStruct rect { 0, 0, this->Rect.X + 60, this->Rect.Y + 48 };
				DSurface::Composite->DrawSHP(pTypeExt->CameoPal.GetOrDefaultConvert(FileSystem::CAMEO_PAL), pSHP, 0, &position, &rect,
					BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
			}
		}

		ColorStruct fillColor { 0, 0, 0 };
		RectangleStruct fillRect { this->Rect.X, this->Rect.Y + 48, this->Rect.Width, 21 };

		if (this->Hovering)
			fillRect.Height += 10;

		DSurface::Composite->Fill_Rect_Trans(&fillRect, &fillColor, 30);
	};

	const auto& seIns = SelectedInfoClass::Instance;

	if (seIns.CurrentSelectCameo.size() == 1 || Phobos::Config::SelectedDisplay_Expand)
	{
		const auto pExt = seIns.CurrentSelectTechno[this->ID + seIns.Current];
		const auto pTechno = pExt->This();
		const auto pType = pTechno->GetTechnoType();
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		drawCameo(pTypeExt);

		const auto pRules = RulesClass::Instance();
		const auto ratio = static_cast<double>(pTechno->Health) / pType->Strength;
		auto rect = RectangleStruct { this->Rect.X + 4, this->Rect.Y + 2, 52, 8 };
		DSurface::Composite->Fill_Rect(rect, COLOR_BLACK);

		rect = RectangleStruct { rect.X + 1, rect.Y + 1, static_cast<int>(50 * ratio + 0.5), 3 };
		const auto color = (ratio > pRules->ConditionYellow) ? 0x67EC : (ratio > pRules->ConditionRed ? 0xFFEC : 0xF986);
		DSurface::Composite->Fill_Rect(rect, color);
		const auto pShield = pExt->ShieldEntity.get();

		if (pShield && !pShield->IsBrokenAndNonRespawning())
		{
			rect.Width = static_cast<int>(50 * (static_cast<double>(pShield->GetHP()) / pShield->GetType()->Strength.Get()) + 0.5);
			ColorStruct fillColor { 153, 153, 255 };
			DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, 80);
		}

		if (pTechno->IsIronCurtained())
		{
			const auto& timer = pTechno->IronCurtainTimer;
			rect.Width = static_cast<int>(50 * (static_cast<double>(timer.GetTimeLeft()) / timer.TimeLeft) + 0.5);
			ColorStruct fillColor { 200, 50, 50 };
			DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, 80);
		}

		int value = -1, maxValue = 0;
		TechnoExtData::GetValuesForDisplay(pTechno, pTypeExt->SelectedInfo_CameoType.Get(), value, maxValue, pTypeExt->SelectedInfo_CameoIndex, pShield);

		rect.Y += 4;
		rect.Width = static_cast<int>(50 * ((value <= -1 || maxValue <= 0) ? 1.0 : (static_cast<double>(value) / maxValue)) + 0.5);
		--rect.Height;
		DSurface::Composite->Fill_Rect(rect, COLOR_WHITE);
	}
	else
	{
		const auto pSelect = seIns.CurrentSelectCameo[this->ID + seIns.Current];
		drawCameo(pSelect.TypeExt);
		const int count = pSelect.Count;

		if (count > 1)
		{
			wchar_t text[0x20];
			swprintf_s(text, L"%d", count);
			TextPrintType printType = TextPrintType::Background | TextPrintType::FullShadow | TextPrintType::Point8;
			const COLORREF color = Drawing::TooltipColor->ToInit();
			Point2D textPosition { this->Rect.X + 1, this->Rect.Y + 1 };
			RectangleStruct surfaceRect { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
			DSurface::Composite->DSurfaceDrawText(text, &surfaceRect, &textPosition, color, 0, printType);
		}
	}
}

// ----------------------------------------

SelectedMainCameoClass::SelectedMainCameoClass(int x, int y)
	: GadgetClass(x, y, 60, 48, static_cast<GadgetFlag>(0), false)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect;
}

bool SelectedMainCameoClass::Draw(bool forced)
{
	return false;
}

void SelectedMainCameoClass::OnMouseEnter()
{
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedMainCameoClass::OnMouseLeave()
{
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}
