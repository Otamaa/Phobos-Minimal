#include "MessageToggleClass.h"
#include "MessageColumnClass.h"

#include <Surface.h>
#include <Drawing.h>

MessageToggleClass::MessageToggleClass(int id, int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, GadgetFlag::LeftPress | GadgetFlag::LeftRelease, false)
	, ID(id)
{
	this->Disabled = id != 0;
}

bool MessageToggleClass::Draw(bool forced)
{
	return false;
}

void MessageToggleClass::OnMouseEnter()
{
	this->Hovering = true;
	MessageColumnClass::Instance.MouseEnter(true);
}

void MessageToggleClass::OnMouseLeave()
{
	this->Hovering = false;
	this->Clicking = false;
	MessageColumnClass::Instance.MouseLeave(true);
}

bool MessageToggleClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	if (!this->Clicking)
	{
		if ((flags & GadgetFlag::LeftPress) && this->Hovering)
		{
			this->Clicking = true;

			if (MessageColumnClass::Instance.IsExpanded())
				MessageColumnClass::Instance.PackUp(true);
			else
				MessageColumnClass::Instance.Expand();
		}
	}
	else if (flags & GadgetFlag::LeftRelease)
	{
		this->Clicking = false;
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void MessageToggleClass::DrawShape() const
{
	if (this->Disabled)
		return;

	RectangleStruct drawRect = this->Rect;
	ColorStruct color = MessageColumnClass::Instance.GetColor();
	MessageColumnClass::Instance.IncreaseBrightness(color);
	const int opacity = this->Hovering && !this->Clicking
		? MessageColumnClass::MediumOpacity : MessageColumnClass::LowOpacity;

	if (MessageColumnClass::Instance.IsExpanded())
	{
		DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, opacity);

		color = ColorStruct { 255, 0, 0 };

		if (this->Hovering && !this->Clicking)
			MessageColumnClass::Instance.IncreaseBrightness(color);

		const int drawColor = Drawing::RGB_To_Int(color);
		constexpr int offset = 4;
		constexpr int iconSide = MessageToggleClass::ButtonSide - (offset * 2);
		drawRect.X += offset;
		drawRect.Y += offset;
		drawRect.Width = iconSide;
		drawRect.Height = iconSide;

		DSurface::Composite->Fill_Rect(drawRect, drawColor);
	}
	else
	{
		DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, opacity);

		color = MessageColumnClass::Instance.GetColor();

		if (this->Hovering && !this->Clicking)
			MessageColumnClass::Instance.IncreaseBrightness(color);

		const int drawColor = Drawing::RGB_To_Int(color);
		constexpr int interval = 2;
		constexpr int drawCount = 3;
		constexpr int offset = (MessageToggleClass::ButtonSide - interval * (drawCount * 2 - 1)) / 2;
		drawRect.X += offset;
		drawRect.Y += offset;
		drawRect.Width = MessageToggleClass::ButtonSide - (2 * offset);
		drawRect.Height = interval;

		DSurface::Composite->Fill_Rect(drawRect, drawColor);

		drawRect.Y += (interval * 2);

		DSurface::Composite->Fill_Rect(drawRect, drawColor);

		drawRect.Y += (interval * 2);

		DSurface::Composite->Fill_Rect(drawRect, drawColor);
	}
}