#include "MessageScrollClass.h"
#include "MessageColumnClass.h"
#include "MessageToggleClass.h"

#include <ScenarioClass.h>
#include <WWMouseClass.h>
#include <Drawing.h>

#include <Phobos.h>

MessageScrollClass::MessageScrollClass(int id, int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, GadgetFlag::LeftPress | GadgetFlag::LeftHeld | GadgetFlag::LeftRelease, true)
	, ID(id)
{
	this->Disabled = true;
}

bool MessageScrollClass::Draw(bool forced)
{
	return false;
}

void MessageScrollClass::OnMouseEnter()
{
	this->Hovering = true;
	MessageColumnClass::Instance.MouseEnter();
}

void MessageScrollClass::OnMouseLeave()
{
	this->Hovering = false;
	MessageColumnClass::Instance.MouseLeave();
}

bool MessageScrollClass::Clicked(WWKey* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier)
{
	if (!MessageColumnClass::IsStickyButton(this))
	{
		if (this->ID) // Scroll_Board
		{
			if (!(flags & GadgetFlag::LeftPress) || !MessageColumnClass::Instance.IsExpanded() || !this->Hovering)
				return false;

			this->LastY = y;
			this->LastScroll = MessageColumnClass::Instance.GetScrollIndex();
		}
		else // Scroll_Bar
		{
			if (!(flags & GadgetFlag::LeftPress) || !this->Hovering)
				return false;

			int maxScroll = 0;
			int thumbHeight = this->Rect.Height;
			int thumbPosY = this->Rect.Y;

			if (MessageColumnClass::Instance.GetThumbDimension(&maxScroll, &thumbHeight, &thumbPosY))
			{
				if (y >= thumbPosY && y <= thumbPosY + thumbHeight)
				{
					this->LastY = y;
					this->LastScroll = MessageColumnClass::Instance.GetScrollIndex();
				}
				else
				{
					const int newScroll = maxScroll * (y - this->Rect.Y - thumbHeight / 2) / (this->Rect.Height - thumbHeight);
					MessageColumnClass::Instance.SetScroll(newScroll);
					this->LastY = y;
					this->LastScroll = newScroll;
				}
			}
		}
	}
	else if (flags & GadgetFlag::LeftHeld)
	{
		if (this->ID) // Scroll_Board
		{
			const int indexOffset = (this->LastY - y) / MessageToggleClass::ButtonSide;
			MessageColumnClass::Instance.SetScroll(this->LastScroll + indexOffset);
		}
		else // Scroll_Bar
		{
			int maxScroll = 0;
			int thumbHeight = this->Rect.Height;

			if (MessageColumnClass::Instance.GetThumbDimension(&maxScroll, &thumbHeight))
			{
				const int indexOffset = maxScroll * (y - this->LastY) / (this->Rect.Height - thumbHeight);
				MessageColumnClass::Instance.SetScroll(this->LastScroll + indexOffset);
			}
		}
	}

	this->GadgetClass::Action(flags, pKey, modifier);
	return true;
}

void MessageScrollClass::DrawShape() const
{
	if (this->ID) // Scroll_Board
	{
		if ((MessageColumnClass::Instance.IsHovering() && Phobos::Config::MessageApplyHoverState)
			|| MessageColumnClass::Instance.IsExpanded()
			|| ScenarioClass::Instance->UserInputLocked)
		{
			RectangleStruct drawRect = this->Rect;
			auto color = MessageColumnClass::Instance.GetColor();
			MessageColumnClass::Instance.DecreaseBrightness(color, 3);

			DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, Phobos::Config::MessageDisplayInCenter_BoardOpacity);
		}
	}
	else // Scroll_Bar
	{
		if (!this->Disabled)
		{
			constexpr int offset = 1;
			RectangleStruct drawRect { this->Rect.X + offset, this->Rect.Y, this->Rect.Width - (offset * 2), this->Rect.Height };
			ColorStruct color = MessageColumnClass::Instance.GetColor();
			MessageColumnClass::Instance.DecreaseBrightness(color);

			int thumbHeight = this->Rect.Height;
			int thumbPos = 0;
			MessageColumnClass::Instance.GetThumbDimension(nullptr, &thumbHeight, &thumbPos);
			const int thumbY = WWMouseClass::Instance->XY1.Y - this->Rect.Y;
			const bool onThumb = thumbY >= thumbPos && thumbY <= (thumbPos + thumbHeight);
			const int opacity = this->Hovering && !onThumb
				? MessageColumnClass::HighOpacity : MessageColumnClass::MediumOpacity;

			DSurface::Composite->Fill_Rect_Trans(&drawRect, &color, opacity);

			color = MessageColumnClass::Instance.GetColor();

			if (MessageColumnClass::IsStickyButton(this))
				MessageColumnClass::Instance.IncreaseBrightness(color, 2);
			else if (this->Hovering && onThumb)
				MessageColumnClass::Instance.IncreaseBrightness(color);

			drawRect.Y += thumbPos;
			drawRect.Height = thumbHeight;

			DSurface::Composite->Fill_Rect(drawRect, color.ToInit());
		}
	}
}