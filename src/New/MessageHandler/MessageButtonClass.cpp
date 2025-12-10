#include "MessageButtonClass.h"
#include "MessageColumnClass.h"

#include <Surface.h>
#include <Drawing.h>

MessageButtonClass::MessageButtonClass(int id, int x, int y, int width, int height)
	: MessageToggleClass(id, x, y, width, height)
{
	this->Disabled = true;
	this->Flags |= GadgetFlag::LeftHeld;
}

bool MessageButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if (!this->Clicking)
	{
		if ((flags & GadgetFlag::LeftPress) && this->Hovering)
		{
			this->CheckTime = MessageColumnClass::GetSystemTime() + MessageButtonClass::HoldInitialDelay;
			this->Clicking = true;

			if (this->ID == 3)
				MessageColumnClass::Instance.ScrollDown();
			else
				MessageColumnClass::Instance.ScrollUp();
		}
	}
	else if (flags & GadgetFlag::LeftRelease)
	{
		this->Clicking = false;
	}
	else if (flags & GadgetFlag::LeftHeld)
	{
		const int timeExpired = MessageColumnClass::GetSystemTime() - this->CheckTime;

		if (timeExpired > 0)
		{
			this->CheckTime += MessageButtonClass::HoldTriggerDelay;

			if (this->ID == 3)
				MessageColumnClass::Instance.ScrollDown();
			else
				MessageColumnClass::Instance.ScrollUp();
		}
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void MessageButtonClass::DrawShape() const
{
	if (this->Disabled)
		return;

	constexpr int intervalX = 5;
	constexpr int intervalY = 1;
	RectangleStruct drawRect = this->Rect;
	ColorStruct color = MessageColumnClass::Instance.GetColor();
	MessageColumnClass::Instance.IncreaseBrightness(color, 3);
	const bool can = this->ID == 3 ? MessageColumnClass::Instance.CanScrollDown() : MessageColumnClass::Instance.CanScrollUp();
	const bool highLight = can && this->Hovering;
	const int opacity = highLight ? MessageColumnClass::MediumOpacity : MessageColumnClass::LowOpacity;

	DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, opacity);

	color = can ? MessageColumnClass::Instance.GetColor() : ColorStruct { 0, 0, 0 };

	if (can && this->Clicking)
		MessageColumnClass::Instance.IncreaseBrightness(color, 2);
	else if (highLight)
		MessageColumnClass::Instance.IncreaseBrightness(color);

	drawRect.X += intervalX;
	drawRect.Y += intervalY;
	drawRect.Width -= (intervalX * 2);
	drawRect.Height = MessageToggleClass::ButtonIconWidth;

	DSurface::Composite->Fill_Rect(drawRect, color.ToInit());
}