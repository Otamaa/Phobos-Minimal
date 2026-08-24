#include "UniqueTechnoButtonClass.h"
#include "UniqueTechnoColumnClass.h"

#include <FactoryClass.h>
#include <TacticalClass.h>
#include <PCX.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Infantry/Body.h>

UniqueTechnoButtonClass::UniqueTechnoButtonClass(int id, int x, int y)
	: GadgetClass(x, y, 60, 48, GadgetFlag::LeftPress, false)
	, ID(id)
{
	this->Disabled = !UniqueTechnoColumnClass::Instance.Visible;
}

bool UniqueTechnoButtonClass::Draw(bool forced)
{
	if (!UniqueTechnoColumnClass::Instance.Visible)
		return false;

	auto& vec = ScenarioExtData::Instance()->OwnedUniqueTechnos;

	if (vec.empty())
		return false;

	const int index = this->ID;

	if (index >= static_cast<int>(vec.size()))
		return false;

	const auto pExt = vec[index];
	const auto pTechno = pExt->This();
	
	if (!pTechno->IsAlive)
		return false;

	const auto pTypeExt = pExt->TypeExtData;
	const auto pType = pTypeExt->This();

	Point2D position { this->Rect.X, this->Rect.Y };
	RectangleStruct drawRect { this->Rect.X, this->Rect.Y, 60, 48 };

	if (const auto CameoPCX = pTypeExt->CameoPCX.GetSurface())
	{
		PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, CameoPCX);
	}
	else if (const auto pSHP = pType->GetCameo())
	{
		auto getMissingCameo = [pSHP]() -> BSurface*
		{
			char pFilename[0x20];
			strcpy_s(pFilename, FakeRulesClass::Instance->MissingCameo.data());
			_strlwr_s(pFilename);

			if (!_stricmp(pSHP->Filename, GameStrings::XXICON_SHP) && strstr(pFilename, ".pcx"))
			{
				PCXImages::Instance->LoadFile(pFilename);

				if (const auto MissingCameoPCX = PCXImages::Instance->GetSurface(pFilename))
					return MissingCameoPCX;
			}

			return nullptr;
		};

		if (const auto MissingCameoPCX = getMissingCameo())
		{
			PCXImages::Instance->BlitToSurface(&drawRect, DSurface::Composite, MissingCameoPCX);
		}
		else
		{
			RectangleStruct rect { 0, 0, position.X + 60, position.Y + 48 };
			DSurface::Composite->DrawSHP(pTypeExt->CameoPal.GetOrDefaultConvert(FileSystem::CAMEO_PAL), pSHP, 0, &position, &rect,
				BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	auto GetSelectedTechno = [pTechno, pExt]() -> TechnoClass* {
		TechnoClass* pSelect = nullptr;

		if (!pTechno->InLimbo)
			pSelect = pTechno;
		else if (auto pTrans = pTechno->Transporter){
			for (; pTrans; pSelect = pTrans, pTrans = pTrans->Transporter);

			pSelect = pSelect;
		}else if (pExt->AbsType == AbstractType::Infantry && ((InfantryExtData*)pExt)->GarrisonedIn)
			pSelect = ((InfantryExtData*)pExt)->GarrisonedIn;

		if (pSelect && pSelect->IsAlive)
			return pSelect;

		return nullptr;
	};

	TechnoClass* pSelect = GetSelectedTechno();

	if (pSelect)
	{
		const auto pSelectExt = TechnoExtContainer::Instance.Find(pSelect);
		const auto pRules = FakeRulesClass::Instance();

		// pushed bar Width past the 50/54px allotment and overshot the cameo edge.
		auto ratio = std::clamp(pTechno->GetHealthRatio(), 0.0, 1.0);

		if (pSelect->IsIronCurtained())
		{
			ColorStruct fillColor { 50, 50, 50 };
			DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 20);

			int time = Unsorted::CurrentFrame() - pSelect->LastFireBulletFrame;

			if (time < 20)
			{
				fillColor = ColorStruct { 255, 255, 0 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 20 - time);
			}
		}
		else
		{
			int time = Unsorted::CurrentFrame()-pSelectExt->LastHurtFrame;

			if (ratio < pRules->ConditionRed)
			{
				ColorStruct fillColor { 255, 0, 0 };
				int trans = 40 - time;

				if (trans < 0)
				{
					const int round = time % 60;
					trans = ((round <= 20) ? 0 : ((round <= 40) ? (round - 20) : (60 - round)));
				}

				if (trans > 0)
					DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, trans);
			}
			else if (ratio < pRules->ConditionYellow)
			{
				ColorStruct fillColor { 255, 0, 0 };
				int trans = 30 - time;

				if (trans < 0)
				{
					const int round = time % 160;
					trans = ((round <= 140) ? 0 : ((round <= 150) ? (round - 140) : (160 - round)));
				}

				if (trans > 0)
					DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, trans);
			}
			else if (time < 20)
			{
				ColorStruct fillColor { 255, 0, 0 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, (20 - time));
			}

			time = Unsorted::CurrentFrame() - pSelect->LastFireBulletFrame;

			if (time < 20)
			{
				ColorStruct fillColor { 255, 255, 0 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 20 - time);
			}

			if (pSelect->TemporalTargetingMe)
			{
				ColorStruct fillColor { 100, 100, 255 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 25);
			}
			else if (pSelect->AirstrikeTintStage)
			{
				ColorStruct fillColor { 255, 50, 0 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 25);
			}
			else if (pSelect->DrainingMe || pSelect->LocomotorSource || (pSelect->AbstractFlags & AbstractFlags::Foot) && static_cast<FootClass*>(pSelect)->ParasiteEatingMe)
			{
				ColorStruct fillColor { 200, 0, 255 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 25);
			}
			else if (pSelect->IsUnderEMP() || pSelect->Deactivated || pSelect->IsParalyzed())
			{
				ColorStruct fillColor { 128, 128, 128 };
				DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 25);
			}
		}

		RectangleStruct rect { (position.X + 3), (position.Y + 1), 0, 7 };

		if (pSelect->BunkerLinkedItem && pSelect->WhatAmI() != AbstractType::Building)
		{
			const auto bunkerRatio = std::clamp(pSelect->BunkerLinkedItem->GetHealthRatio(), 0.0, 1.0);
			rect.Width = static_cast<int>(54 * bunkerRatio + 0.5);
			DSurface::Composite->Draw_Rect(rect, 0x781F);
		}
		else if (pSelect != pTechno)
		{
			const auto selectRatio = std::clamp(pSelect->GetHealthRatio(), 0.0, 1.0);
			rect.Width = static_cast<int>(54 * selectRatio + 0.5);
			DSurface::Composite->Draw_Rect(rect, 0xFB20);
		}

		++rect.X;
		++rect.Y;
		rect.Width = 52;
		rect.Height = 5;

		DSurface::Composite->Fill_Rect(rect, 0);

		++rect.X;
		++rect.Y;
		rect.Width = static_cast<int>(50 * ratio + 0.5);
		rect.Height = 3;

		const int color = (ratio > pRules->ConditionYellow) ? 0x67EC : (ratio > pRules->ConditionRed ? 0xFFEC : 0xF986);
		DSurface::Composite->Fill_Rect(rect, color);

		const auto pShield = pSelectExt->ShieldEntity.get();

		if (pShield && !pShield->IsBrokenAndNonRespawning())
		{
			// shield ratio (e.g. from overcharge) is what was overshooting the healthbar
			// rect into neighboring cameos.
			const auto shieldStrength = pShield->GetType()->Strength.Get();
			ratio = (shieldStrength > 0)
				? std::clamp(static_cast<double>(pShield->GetHP()) / shieldStrength, 0.0, 1.0)
				: 0.0;
			rect.Width = static_cast<int>(50 * ratio + 0.5);
			ColorStruct fillColor { 153, 153, 255 };
			DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, 70);
		}

		if (pSelect->IsIronCurtained())
		{
			const auto& timer = pSelect->IronCurtainTimer;

			ratio = (timer.TimeLeft > 0)
				? std::clamp(static_cast<double>(timer.GetTimeLeft()) / timer.TimeLeft, 0.0, 1.0)
				: 0.0;
			rect.Width = static_cast<int>(50 * ratio + 0.5);
			ColorStruct fillColor { 200, 50, 50 };
			DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, 70);
		}
	}
	else
	{
		const auto absType = pTechno->WhatAmI();
		const auto buildCat = (absType == AbstractType::Building) ? static_cast<BuildingClass*>(pTechno)->Type->BuildCat : BuildCat::DontCare;
		const auto pFactory = pTechno->Owner->GetPrimaryFactory(absType, pType->Naval, buildCat);

		if (pFactory && pFactory->Object == pTechno)
		{
			ColorStruct fillColor { 0, 0, 0 };
			DSurface::Composite->Fill_Rect_Trans(&drawRect, &fillColor, 30);

			RectangleStruct rect { (position.X + 4), (position.Y + 2), 52, 5 };
			DSurface::Composite->Fill_Rect(rect, 0);

			// overshot the progress bar past the black backing rect.
			const auto ratio = std::clamp(static_cast<double>(pFactory->GetProgress()) / 54, 0.0, 1.0);
			rect = RectangleStruct { (position.X + 5), (position.Y + 3), static_cast<int>(50 * ratio), 3 };
			DSurface::Composite->Fill_Rect(rect, 0xFFFF);
		}
		else
		{
			RectangleStruct rect { (position.X + 3), (position.Y + 1), 54, 7 };
			DSurface::Composite->Draw_Rect(rect, 0xFFFF);

			++rect.X;
			++rect.Y;
			rect.Width = 52;
			rect.Height = 5;

			DSurface::Composite->Fill_Rect(rect, 0);

			const auto ratio = std::clamp(pTechno->GetHealthRatio(), 0.0, 1.0);

			++rect.X;
			++rect.Y;
			rect.Width = static_cast<int>(50 * ratio + 0.5);
			rect.Height = 3;

			const auto pRules = FakeRulesClass::Instance();
			const int color = (ratio > pRules->ConditionYellow) ? 0x67EC : (ratio > pRules->ConditionRed ? 0xFFEC : 0xF986);
			DSurface::Composite->Fill_Rect(rect, color);

			rect.Width = 50;
			ColorStruct fillColor { 255, 255, 255 };
			DSurface::Composite->Fill_Rect_Trans(&rect, &fillColor, 70);
		}
	}

	if (this->Hovering)
	{
		RectangleStruct rect { 0, 0, position.X + 60, position.Y + 48 };
		DSurface::Composite->Draw_Rect(rect, drawRect, Drawing::TooltipColor->ToInit());
	}

	return true;
}

void UniqueTechnoButtonClass::OnMouseEnter()
{
	if (!UniqueTechnoColumnClass::Instance.Visible)
		return;

	auto& vec = ScenarioExtData::Instance()->OwnedUniqueTechnos;
	const int index = this->ID;

	if (index >= static_cast<int>(vec.size()))
		return;

	this->Hovering = true;
	UniqueTechnoColumnClass::Instance.Hovering = index;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void UniqueTechnoButtonClass::OnMouseLeave()
{
	this->Hovering = false;
	UniqueTechnoColumnClass::Instance.Hovering = -1;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool UniqueTechnoButtonClass::Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier)
{
	if (!UniqueTechnoColumnClass::Instance.Visible || !(flags & GadgetFlag::LeftPress))
		return false;

	auto& vec = ScenarioExtData::Instance()->OwnedUniqueTechnos;
	const int index = this->ID;

	if (index >= static_cast<int>(vec.size()))
		return false;

	VocClass::PlayGlobal(FakeRulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
	auto pSelect = vec[index]->This();

	for (auto pTrans = pSelect->Transporter; pTrans; pTrans = pTrans->Transporter)
		pSelect = pTrans;

	if (ObjectClass::CurrentObjects->Count != 1 || !pSelect->IsSelected)
		MapClass::UnselectAll();

	if (!pSelect->InLimbo && !pSelect->Select())
		TacticalClass::Instance->SetTacticalPosition(&pSelect->Location);

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}
