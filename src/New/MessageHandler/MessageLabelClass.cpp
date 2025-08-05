#include "MessageLabelClass.h"
#include "MessageColumnClass.h"

#include <BitFont.h>
#include <Surface.h>

#include <Ext/Scenario/Body.h>

const wchar_t* MessageLabelClass::GetText() const
{
	return
		ScenarioExtData::Instance()->RecordMessages
		[this->ID].c_str();
}

MessageLabelClass::MessageLabelClass(int x, int y, size_t id, int deleteTime, bool animate, int drawDelay)
	: GadgetClass(x, y, 1, 1, GadgetFlag(0), false)
	, ID(id)
	, DeleteTime(deleteTime)
	, Animate(animate)
	, DrawDelay(drawDelay)
{ }

bool MessageLabelClass::Draw(bool forced)
{
	if (this->DrawDelay > MessageColumnClass::GetSystemTime())
		return false;

	if (!GadgetClass::Draw(forced))
		return false;

	if (!ColorScheme::Array->Count)
		return false;

	const auto pBit = BitFont::Instance();
	const int surfaceWidth = DSurface::Temp->Get_Width();
	RectangleStruct rect { this->Rect.X, this->Rect.Y, MessageColumnClass::Instance.GetWidth(), pBit->field_1C };
	const int remainWidth = surfaceWidth - rect.X;

	if (rect.Width > remainWidth)
	{
		rect.Width = remainWidth;

		if (rect.Width <= 0 || rect.Height <= 0)
			return false;
	}

	const wchar_t* text = this->GetText();
	size_t textLen = wcslen(text);

	if (this->Animate)
	{
#pragma warning(suppress: 4996)
		const auto time = Imports::TimeGetTime.invoke()();
		const size_t animPos = this->AnimPos;
		this->AnimPos = animPos ? (animPos + ((time - this->AnimTiming) >> 4u)) : 1u;

		if (this->AnimPos != animPos)
			this->AnimTiming = time;

		if (this->AnimPos <= textLen)
			VocClass::PlayGlobal(RulesClass::Instance->MessageCharTyped, Panning::Center, 1.0f);
	}

	Animated_Text_Print_623880(DSurface::Temp(),  & rect, text, textLen, pBit, MessageColumnClass::Instance.GetTextColor(), &this->DrawPos, this->IsFocused(), false, true, this->AnimPos);

	return true;
}