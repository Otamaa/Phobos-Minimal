#include "SelectedButtonClass.h"
#include "SelectedInfoClass.h"

#include <Ext/Side/Body.h>
#include <Ext/Event/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <MouseClass.h>

SelectedButtonClass::SelectedButtonClass(int id, int x, int y)
	: GadgetClass(x, y, 30, 30, GadgetFlag::LeftPress, false)
	, ID(id)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect;
}

bool SelectedButtonClass::Draw(bool forced)
{
	return false;
}

void SelectedButtonClass::OnMouseEnter()
{
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedButtonClass::OnMouseLeave()
{
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SelectedButtonClass::Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier)
{
	auto& seIns = SelectedInfoClass::Instance;

	if (seIns.ShouldUpdate)
		seIns.UpdateSelected();

	const auto& vec = seIns.CurrentSelectTechno;

	if (vec.empty())
		return false;

	if (flags & GadgetFlag::LeftPress)
	{
		if (this->ID == 0) // PushButton
		{
			const auto pExt = vec[0];
			const auto pTechno = pExt->This();

			//TODO
			if (pTechno->Owner->ControlledByCurrentPlayer() && pTechno->IsAlive && !pTechno->Berzerk)
			{
				//if (pExt->CanToggleAggressiveStance()) {
				//	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
				//	EventExt::ToggleAggressiveStance::Raise(pTechno);
				//}
			}
		}
		else // AmmoButton
		{
			const auto pExt = vec[0];
			const auto pTechno = pExt->This();

			//TODO
			if (pTechno->Owner->ControlledByCurrentPlayer() && pTechno->Ammo > 0 && pTechno->IsAlive && !pTechno->Berzerk)
			{
				//const auto pTypeExt = pExt->TypeExtData;

				//if (pTechno->Ammo != pTypeExt->This()->Ammo && pTypeExt->CanManualReload)
				//{
				//	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
				//	EventExt::ManualReloadEvent::Raise(pTechno);
				//}
			}
		}
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void SelectedButtonClass::DrawInfo() const
{
	//const auto pExt = SelectedInfoClass::Instance.CurrentSelectTechno[0];
	//const auto pTechno = pExt->This();
	//const auto pTypeExt = pExt->TypeExtData;
	//const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	//const auto pSHP = pTypeExt->SelectedInfo_Button.Get(pSideExt->SelectedInfo_Button.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Button));

	//if (!pSHP || pSHP->GetFrameCount() < 7)
	//	return;

	//const auto position = Point2D { this->Rect.X, this->Rect.Y };
	//const auto rect = RectangleStruct { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };

	//DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
	//	pSHP, 0, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	//if (this->ID == 0) // PushButton
	//{
	//	int frame = 3;

	//	//if (pExt->CanToggleAggressiveStance() && pTechno->IsAlive && !pTechno->Berzerk)
	//	//	frame = !pExt->GetAggressiveStance() ? 3 : 2;

	//	DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
	//		pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	//	if (this->Hovering)
	//	{
	//		auto location = Point2D { this->Rect.X + this->Rect.Width + 10, this->Rect.Y + 4 };
	//		const auto text = GeneralUtils::LoadStringUnlessMissing("TIP:AggressiveStance", L"AggressiveStance");
	//		RectangleStruct drawRect = Drawing::GetTextBox(text, location, { 3 ,2 });
	//		location += Point2D { 4, 1 };
	//		drawRect.Width += 8;
	//		ColorStruct color { 0, 0, 0 };
	//		DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, 40);
	//		DSurface::Composite->Draw_Rect(drawRect, COLOR_WHITE);
	//		DSurface::Composite->DSurfaceDrawText(text, &location, COLOR_WHITE);
	//	}
	//}
	//else // AmmoButton
	//{
	//	int frame = 4;

	//	if (pTypeExt->CanManualReload && pTechno->IsAlive && !pTechno->Berzerk && pTechno->Ammo != pTechno->GetTechnoType()->Ammo)
	//		frame = pTechno->Ammo ? 6 : 5;

	//	DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
	//		pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	//	if (this->Hovering)
	//	{
	//		auto location = Point2D { this->Rect.X + this->Rect.Width + 10, this->Rect.Y + 4 };
	//		const auto text = GeneralUtils::LoadStringUnlessMissing("TIP:ManualReloadAmmo", L"ManualReloadAmmo");
	//		RectangleStruct drawRect = Drawing::GetTextBox(text, location, { 3, 2 });
	//		location += Point2D { 4, 1 };
	//		drawRect.Width += 8;
	//		ColorStruct color { 0, 0, 0 };
	//		DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, 40);
	//		DSurface::Composite->Draw_Rect(drawRect, COLOR_WHITE);
	//		DSurface::Composite->DSurfaceDrawText(text, &location, COLOR_WHITE);
	//	}
	//}
}

// ----------------------------------------

SelectedNotButtonClass::SelectedNotButtonClass(int id, int x, int y)
	: GadgetClass(x, y, 14, 14, static_cast<GadgetFlag>(0), false)
	, ID(id)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect;
}

bool SelectedNotButtonClass::Draw(bool forced)
{
	return false;
}

void SelectedNotButtonClass::OnMouseEnter()
{
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedNotButtonClass::OnMouseLeave()
{
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedNotButtonClass::DrawInfo() const
{
	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pSHP = pSideExt->SelectedInfo_Buff.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Buff);

	if (!pSHP || pSHP->GetFrameCount() < 15)
		return;

	const auto position = Point2D { this->Rect.X, this->Rect.Y };
	const auto pExt = SelectedInfoClass::Instance.CurrentSelectTechno[0];
	const auto pTechno = pExt->This();
	auto getIconFrame = [](const int base, const double mult) -> int
	{
		if (mult - 1.0 > 1e-10)
			return base + (mult > 2.0 ? 4 : 3);
		else if (mult - 1.0 < -1e-10)
			return base + (mult < 0.5 ? 2 : 1);

		return base;
	};

	//TODO : add ability Type icon etc

	if (this->ID == 0) // InfoIconA
	{
		const double mult = TechnoExtData::ApplyAdditionalDamageMult(pTechno, TechnoExtData::GetDamageMult(pTechno));
		const int frame = getIconFrame(0, mult);
		RectangleStruct rect { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
		DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
			pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		if (this->Hovering)
		{
			auto location = Point2D { this->Rect.X + this->Rect.Width + 10, this->Rect.Y - 3 };
			wchar_t buffer[0x20];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("TIP:PowerMult", L"PowerMult:%5.2f"), mult);
			RectangleStruct drawRect = Drawing::GetTextBox(buffer, location, { 3, 2 });
			location += Point2D { 4, 1 };
			drawRect.Width += 8;
			ColorStruct color { 0, 0, 0 };
			DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, 40);
			DSurface::Composite->Draw_Rect(drawRect, COLOR_WHITE);
			DSurface::Composite->DSurfaceDrawText(buffer, &location, COLOR_WHITE);
		}
	}
	else if (this->ID == 1) // InfoIconD
	{
		const auto mult = TechnoExtData::GetCurrentArmorMultiplier(pTechno, pTechno->GetTechnoType(), pTechno->Owner, nullptr,false);
		const int frame = getIconFrame(5, mult);
		RectangleStruct rect { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
		DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
			pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		if (this->Hovering)
		{
			auto location = Point2D { this->Rect.X + this->Rect.Width + 10, this->Rect.Y - 3 };
			wchar_t buffer[0x20];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("TIP:ArmorMult", L"ArmorMult:%5.2f"), mult);
			RectangleStruct drawRect = Drawing::GetTextBox(buffer, location, { 3 ,2 });
			location += Point2D { 4, 1 };
			drawRect.Width += 8;
			ColorStruct color { 0, 0, 0 };
			DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, 40);
			DSurface::Composite->Draw_Rect(drawRect, COLOR_WHITE);
			DSurface::Composite->DSurfaceDrawText(buffer, &location, COLOR_WHITE);
		}
	}
	else // InfoIconS
	{
		const auto pFoot = flag_cast_to<FootClass*, true>(pTechno);
		const double mult = pFoot
			? TechnoExtData::GetCurrentSpeedMultiplier(pFoot)
			: pExt->AE.ROFMultiplier * (pTechno->HasAbility(AbilityType::ROF) ? RulesClass::Instance->VeteranROF : 1.0);

		const int frame = getIconFrame((pFoot ? 10 : 0), mult);
		RectangleStruct rect { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
		DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
			pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		if (this->Hovering)
		{
			auto location = Point2D { this->Rect.X + this->Rect.Width + 10, this->Rect.Y - 3 };
			wchar_t buffer[0x20];
			swprintf_s(buffer, (pFoot ? GeneralUtils::LoadStringUnlessMissing("TIP:SpeedMult", L"SpeedMult:%5.2f") : GeneralUtils::LoadStringUnlessMissing("TIP:ROFMult", L"ROFMult:%5.2f")), mult);
			RectangleStruct drawRect = Drawing::GetTextBox(buffer, location, { 3 ,2 });
			location += Point2D { 4, 1 };
			drawRect.Width += 8;
			ColorStruct color { 0, 0, 0 };
			DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, 40);
			DSurface::Composite->Draw_Rect(drawRect, COLOR_WHITE);
			DSurface::Composite->DSurfaceDrawText(buffer, &location, COLOR_WHITE);
		}
	}
}

// ----------------------------------------

SelectedToggleClass::SelectedToggleClass(int id, int x, int y)
	: GadgetClass(x, y, 10, 14, GadgetFlag::LeftPress, false)
	, ID(id)
{
	this->Disabled = (id == 1) && (!Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect);
}

bool SelectedToggleClass::Draw(bool forced)
{
	return false;
}

void SelectedToggleClass::OnMouseEnter()
{
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedToggleClass::OnMouseLeave()
{
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SelectedToggleClass::Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier)
{
	//if (flags & GadgetFlag::LeftPress)
	//{
	//	if (this->ID == 0) // Toggle on/off
	//		SelectedInfoClass::Instance.SwitchVisible();
	//	else // Toggle expand/storage
	//		SelectedInfoClass::Instance.SwitchExpand();
	//}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void SelectedToggleClass::DrawInfo() const
{
	//const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	//auto rect = RectangleStruct { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
	//const auto pSHP = pSideExt->SelectedInfo_Toggle.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Toggle);

	//if (!pSHP || pSHP->GetFrameCount() < 4)
	//	return;

	//const auto position = Point2D { this->Rect.X, this->Rect.Y };
	//const auto frame = this->ID == 0 ? (Phobos::Config::SelectedDisplay_Enable ? 1 : 0) : (Phobos::Config::SelectedDisplay_Expand ? 3 : 2);
	//DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
	//	pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

// ----------------------------------------

SelectedScrollClass::SelectedScrollClass(int id, int x, int y)
	: GadgetClass(x, y, 10, 14, GadgetFlag::LeftPress, false)
	, ID(id)
{
	this->Disabled = !Phobos::Config::SelectedDisplay_Enable || !SelectedInfoClass::Instance.SingleSelect || !SelectedInfoClass::Instance.ObtainSelect;
}

bool SelectedScrollClass::Draw(bool forced)
{
	return false;
}

void SelectedScrollClass::OnMouseEnter()
{
	this->Hovering = true;
	SelectedInfoClass::Instance.IsHovering = true;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

void SelectedScrollClass::OnMouseLeave()
{
	this->Hovering = false;
	SelectedInfoClass::Instance.IsHovering = false;
	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}

bool SelectedScrollClass::Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier)
{
	if (flags & GadgetFlag::LeftPress)
	{
		if (this->ID == 0) // Scroll left
		{
			if (SelectedInfoClass::Instance.ScrollLeft())
				VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
		}
		else // Scroll right
		{
			if (SelectedInfoClass::Instance.ScrollRight())
				VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, Panning::Center, 1.0);
		}
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void SelectedScrollClass::DrawInfo() const
{
	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
	auto rect = RectangleStruct { 0, 0, this->Rect.X + this->Rect.Width, this->Rect.Y + this->Rect.Height };
	const auto pSHP = pSideExt->SelectedInfo_Toggle.Get((SHPCaches*)SelectedInfoClass::SelectedInfo_Toggle);

	if (!pSHP || pSHP->GetFrameCount() < 8)
		return;

	const auto position = Point2D { this->Rect.X, this->Rect.Y };
	auto& seIns = SelectedInfoClass::Instance;
	const auto frame = this->ID == 0 ? (seIns.CanScrollLeft() ? 4 : 5) : (seIns.CanScrollRight() ? 6 : 7);
	DSurface::Composite->DrawSHP(pSideExt->SelectedInfo_Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL),
		pSHP, frame, &position, &rect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}
