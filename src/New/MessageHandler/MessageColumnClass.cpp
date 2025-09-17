#include "MessageColumnClass.h"
#include "MessageLabelClass.h"
#include "MessageToggleClass.h"
#include "MessageScrollClass.h"
#include "MessageButtonClass.h"

#include <GScreenClass.h>
#include <Memory.h>
#include <BitFont.h>

#include <Phobos.h>

#include <Ext/Scenario/Body.h>
#include <Ext/Side/Body.h>

MessageColumnClass MessageColumnClass::Instance;
MessageColumnClass::~MessageColumnClass()
{
	this->Initialize();
}

void MessageColumnClass::InitClear()
{
	this->Initialize();

	if (this->Button_Main)
	{
		GScreenClass::Instance->RemoveButton(this->Button_Main);
		GameDelete(this->Button_Main);
		this->Button_Main = nullptr;
	}

	if (this->Button_Toggle)
	{
		GScreenClass::Instance->RemoveButton(this->Button_Toggle);
		GameDelete<true,false>(this->Button_Toggle);
		this->Button_Toggle = nullptr;
	}

	if (this->Button_Up)
	{
		GScreenClass::Instance->RemoveButton(this->Button_Up);
		GameDelete<true,false>(this->Button_Up);
		this->Button_Up = nullptr;
	}

	if (this->Button_Down)
	{
		GScreenClass::Instance->RemoveButton(this->Button_Down);
		GameDelete<true,false>(this->Button_Down);
		this->Button_Down = nullptr;
	}

	if (this->Scroll_Bar)
	{
		GScreenClass::Instance->RemoveButton(this->Scroll_Bar);
		GameDelete<true,false>(this->Scroll_Bar);
		this->Scroll_Bar = nullptr;
	}

	if (this->Scroll_Board)
	{
		GScreenClass::Instance->RemoveButton(this->Scroll_Board);
		GameDelete<true,false>(this->Scroll_Board);
		this->Scroll_Board = nullptr;
	}
}

void MessageColumnClass::InitIO()
{
	if (Unsorted::ArmageddonMode || !Phobos::Config::MessageDisplayInCenter)
		return;

	const auto& rect = DSurface::ViewBounds();
	const int sideWidth = rect.Width / 6;
	int width = rect.Width - (sideWidth * 2);
	int posX = rect.X + sideWidth;
	int posY = rect.Height - rect.Height / 8;
	const int maxLines = posY / MessageToggleClass::ButtonSide - 1;
	constexpr int maxChars = 112;
	constexpr int minRecord = 4;
	constexpr int minCount = 1;
	const int maxRecord = std::clamp(Phobos::Config::MessageDisplayInCenter_RecordsCount, minRecord, maxLines);
	const int maxCount = std::clamp(Phobos::Config::MessageDisplayInCenter_LabelsCount, minCount, maxLines);

	this->Initialize(posX, posY, maxCount, maxRecord, maxChars, width);

	posX -= 1;
	width += 2;

	// Button_Main
	{
		const int locX = rect.Width - MessageToggleClass::ButtonSide;
		const int locY = 0;
		const auto pButton = GameCreate<MessageToggleClass>(0, locX, locY, MessageToggleClass::ButtonSide, MessageToggleClass::ButtonSide);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Button_Main = pButton;
	}

	// Button_Toggle
	{
		const int locX = posX + width - MessageToggleClass::ButtonSide;
		const int locY = posY - MessageToggleClass::ButtonSide;
		const auto pButton = GameCreate<MessageToggleClass>(1, locX, locY, MessageToggleClass::ButtonSide, MessageToggleClass::ButtonSide);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Button_Toggle = pButton;
	}

	// Button_Up
	{
		const int locX = rect.Width * 5 / 12;
		const int locY = posY - (MessageToggleClass::ButtonSide * this->MaxRecord) - 1 - MessageToggleClass::ButtonHeight;
		const auto pButton = GameCreate<MessageButtonClass>(2, locX, locY, sideWidth, MessageToggleClass::ButtonHeight);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Button_Up = pButton;
	}

	// Button_Down
	{
		const int locX = rect.Width * 5 / 12;
		const int locY = posY;
		const auto pButton = GameCreate<MessageButtonClass>(3, locX, locY, sideWidth, MessageToggleClass::ButtonHeight);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Button_Down = pButton;
	}

	// Scroll_Bar
	{
		constexpr int buttonInterval = 3;
		const int locX = posX + width - ((MessageToggleClass::ButtonSide - MessageToggleClass::ButtonHeight) / 2 + MessageToggleClass::ButtonHeight);
		const int locY = posY - (MessageToggleClass::ButtonSide * this->MaxRecord) + (buttonInterval - 1);
		const int barHeight = posY - (MessageToggleClass::ButtonSide + buttonInterval) - locY;
		const auto pButton = GameCreate<MessageScrollClass>(0, locX, locY, MessageToggleClass::ButtonHeight, barHeight);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Scroll_Bar = pButton;
	}

	// Scroll_Board
	{
		const auto pButton = GameCreate<MessageScrollClass>(1, posX, posY, width, 1);
		pButton->Zap();
		GScreenClass::Instance->AddButton(pButton);
		this->Scroll_Board = pButton;
	}

	const int color = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex])->MessageTextColorIndex;

	// 0x72A4C5
	if (const auto pScheme = ColorScheme::Array->Items[(color < 0 || color >= ColorScheme::Array->Count) ? 0 : color])
		pScheme->BaseColor.ToColorStruct(&this->Color);

	this->Update();
}

void MessageColumnClass::Initialize(int x, int y, int maxCount, int maxRecord, int maxChars, int width)
{
	if (Phobos::Otamaa::ExeTerminated)
		return;

	this->LabelList = nullptr;
	this->LabelsPos = Point2D { x, y };
	this->MaxCount = maxCount;
	this->MaxRecord = maxRecord;
	this->MaxChars = maxChars;
	this->Height = MessageToggleClass::ButtonSide;
	this->Width = width - MessageColumnClass::TextReservedSpace;
	this->Color = ColorStruct { 0, 0, 0 };
	this->PackUp(true);
	this->Hovering = false;
	this->Drawing = false;
	this->Blocked = false;
}

MessageLabelClass* MessageColumnClass::AddMessage(const wchar_t* name, const wchar_t* message, int timeout, bool silent, int delay)
{
	if (!message)
		return nullptr;

	const auto pBit = BitFont::Instance();

	if (!pBit)
		return nullptr;

	std::wstring buffer(name ? std::wstring(name) + L":" : L"");

	int prefixWidth = 0;
	pBit->GetTextDimension(buffer.c_str(), &prefixWidth, nullptr, 0);
	const int availableWidth = this->Width - prefixWidth - MessageColumnClass::TextReservedSpace;

	if (availableWidth <= 0)
		return nullptr;

	const int messageLen = static_cast<int>(wcslen(message));
	// As vanilla
	const int charsToCopy = pBit->Func_433F50(message, availableWidth, 111, 1);

	if (charsToCopy < 0)
		return nullptr;

	buffer.append(message, charsToCopy);

	if (this->MaxCount > 0 && (this->GetLabelCount() + 1) > this->MaxCount)
	{
		if (auto pLabel = this->LabelList)
			this->RemoveTextLabel(pLabel);
		else
			return nullptr;
	}

	const size_t newID = ScenarioExtData::Instance()->RecordMessages.size();

	if (!MessageColumnClass::AddRecordString(buffer))
		return nullptr;

	const int currentTime = MessageColumnClass::GetSystemTime();

	if (!silent)
		VocClass::PlayGlobal(RulesClass::Instance->IncomingMessage, Panning::Center, 1.0f);

	const auto pLabel = GameCreate<MessageLabelClass>
	(
		this->LabelsPos.X,
		this->LabelsPos.Y,
		newID,
		(timeout == -1) ? 0 : (timeout + currentTime),
		!silent,
		delay + currentTime
	);

	if (this->LabelList)
		pLabel->AddTail(*this->LabelList);
	else
		this->LabelList = pLabel;

	this->Update();

	if (charsToCopy < messageLen)
	{
		const wchar_t* remainingText = &message[charsToCopy];

		while (*remainingText && *remainingText < 0x20)
			++remainingText;

		if (*remainingText)
		{
			int nextDelay = delay;

			if (!silent)
				nextDelay += (charsToCopy * 2 - 1);

			this->AddMessage(name, remainingText, timeout, silent, nextDelay);
		}
	}

	return pLabel;
}

void MessageColumnClass::MouseEnter(bool block)
{
	this->Hovering = true;

	if (block)
		this->Blocked = true;

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	if (this->Button_Main && this->Button_Main->Hovering)
		return;

	if (const auto pButton = this->Button_Toggle)
		pButton->Disabled = false;

}

void MessageColumnClass::MouseLeave(bool block)
{
	this->Hovering = false;

	if (block)
		this->Blocked = false;

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	if (!this->IsExpanded())
	{
		if (const auto pButton = this->Button_Toggle)
			pButton->Disabled = true;
	}
}

bool MessageColumnClass::CanScrollUp()
{
	return this->IsExpanded() && this->GetScrollIndex() > 0;
}

bool MessageColumnClass::CanScrollDown()
{
	return this->IsExpanded() && this->GetScrollIndex() < this->GetMaxScroll();
}

void MessageColumnClass::ScrollUp()
{
	if (this->CanScrollUp())
		--this->ScrollIndex;
}

void MessageColumnClass::ScrollDown()
{
	if (this->CanScrollDown())
		++this->ScrollIndex;
}

void MessageColumnClass::SetScroll(int index)
{
	this->ScrollIndex = std::clamp(index, 0, this->GetMaxScroll());
}

void MessageColumnClass::Expand()
{
	this->Expanded = true;
	this->ScrollIndex = this->GetMaxScroll();

	if (const auto pButton = this->Button_Toggle)
		pButton->Disabled = false;

	if (const auto pButton = this->Button_Up)
		pButton->Disabled = false;

	if (const auto pButton = this->Button_Down)
		pButton->Disabled = false;

	if (const auto pButton = this->Scroll_Bar)
		pButton->Disabled = false;

	this->Refresh();
}

void MessageColumnClass::PackUp(bool clear)
{
	if (const auto pButton = this->Scroll_Bar)
	{
		if (MessageColumnClass::IsStickyButton(pButton))
			return;
	}

	if (const auto pButton = this->Scroll_Board)
	{
		if (MessageColumnClass::IsStickyButton(pButton))
			return;
	}

	this->Expanded = false;
	this->ScrollIndex = this->GetMaxScroll<true>();

	if (const auto pButton = this->Button_Up)
		pButton->Disabled = true;

	if (const auto pButton = this->Button_Down)
		pButton->Disabled = true;

	if (const auto pButton = this->Scroll_Bar)
		pButton->Disabled = true;

	if (!clear)
	{
		if (auto pLabel = this->GetLastLabel())
		{
			const int currentTime = MessageColumnClass::GetSystemTime();

			for (; pLabel; pLabel = static_cast<MessageLabelClass*>(pLabel->GetPrev()))
			{
				if (pLabel->DrawDelay > currentTime)
					pLabel->DrawDelay = currentTime;

				if (pLabel->Animate)
				{
					pLabel->Animate = false;
					pLabel->AnimPos = 0;
				}
			}

			this->Refresh();

			return;
		}
	}

	this->CleanUp();
}

void MessageColumnClass::CleanUp()
{
	for (auto pLabel = this->LabelList; pLabel; pLabel = this->LabelList)
	{
		this->LabelList = static_cast<MessageLabelClass*>(pLabel->Remove());
		GameDelete<false, false>(pLabel);
	}

	if (const auto pButton = this->Scroll_Board)
	{
		pButton->Rect.Y = this->LabelsPos.Y - 1;
		pButton->Rect.Height = 1;
		pButton->Disabled = true;
	}

	if (const auto pButton = this->Button_Toggle)
		pButton->Disabled = true;
}

void MessageColumnClass::Refresh()
{
	if (const auto pButton = this->Scroll_Board)
	{
		const int count = this->IsExpanded() ? this->MaxRecord : this->GetLabelCount();
		const int height = this->Height * count;
		pButton->Rect.Height = height + 1;
		pButton->Rect.Y = this->LabelsPos.Y - pButton->Rect.Height;
		pButton->Disabled = height == 0;
	}

	if (!this->IsHovering() && !this->IsExpanded())
	{
		if (const auto pButton = this->Button_Toggle)
			pButton->Disabled = true;
	}
}

void MessageColumnClass::Update()
{
	int y = this->LabelsPos.Y;

	for (auto pLabel = this->GetLastLabel(); pLabel; pLabel = static_cast<MessageLabelClass*>(pLabel->GetPrev()))
	{
		y -= this->Height;
		pLabel->Rect.Y = y;
	}

	this->Refresh();
}

void MessageColumnClass::Toggle()
{
	if (this->IsExpanded())
		this->PackUp();
	else
		this->Expand();
}

void MessageColumnClass::Manage()
{
	const int currentTime = MessageColumnClass::GetSystemTime();
	bool changed = false;

	for (auto pLabel = this->LabelList; pLabel; )
	{
		if (pLabel->DeleteTime && currentTime > pLabel->DeleteTime)
		{
			const auto pNextLabel = static_cast<MessageLabelClass*>(pLabel->GetNext());
			this->RemoveTextLabel(pLabel);
			pLabel = pNextLabel;
			changed = true;

			continue;
		}

		pLabel = static_cast<MessageLabelClass*>(pLabel->GetNext());
	}

	if (changed)
		this->Update();
}

void MessageColumnClass::DrawAll()
{
	if (const auto pButton = this->Scroll_Board)
		pButton->DrawShape();

	if (const auto pButton = this->Scroll_Bar)
		pButton->DrawShape();

	if (const auto pButton = this->Button_Main)
		pButton->DrawShape();

	if (const auto pButton = this->Button_Toggle)
		pButton->DrawShape();

	if (const auto pButton = this->Button_Up)
		pButton->DrawShape();

	if (const auto pButton = this->Button_Down)
		pButton->DrawShape();

	if (this->IsExpanded())
	{
		const auto& messages = ScenarioExtData::Instance()->RecordMessages;

		if (messages.empty())
			return;

		int startY = this->LabelsPos.Y;
		const int maxIndex = static_cast<int>(messages.size()) - 1;
		const int startIndex = MinImpl(this->GetScrollIndex() + (this->MaxRecord - 1), maxIndex);
		const int endIndex = MaxImpl(0, startIndex - this->MaxRecord + 1);
		const int color = Drawing::RGB_To_Int(this->GetColor());
		constexpr TextPrintType print = TextPrintType::UseGradPal | TextPrintType::FullShadow | TextPrintType::Point6Grad;

		for (int i = startIndex; i >= endIndex; --i)
		{
			startY -= this->Height;
			Point2D textLocation { this->LabelsPos.X, startY };
			RectangleStruct drawRect { 0, 0, textLocation.X + this->Width, textLocation.Y + this->Height };
			DSurface::Composite->DSurfaceDrawText(messages[i].c_str(), &drawRect, &textLocation, color, 0, print);
		}
	}
	else if (const auto pLabel = this->LabelList)
	{
		this->Drawing = true;
		pLabel->DrawAll(true);
		this->Drawing = false;
	}
}

static COMPILETIMEEVAL reference<SystemTimerClass, 0x887338> sysTimer {};

int MessageColumnClass::GetSystemTime()
{
	int currentTime = sysTimer->TimeLeft;

	if (sysTimer->StartTime != -1)
		currentTime += SystemTimer::GetTime() - sysTimer->StartTime;

	return currentTime;
}

bool MessageColumnClass::GetThumbDimension(int* pMax, int* pHeight, int* pPosY) const
{
	const int totalRecords = static_cast<int>(ScenarioExtData::Instance()->RecordMessages.size());
	const int maxScroll = totalRecords - this->MaxRecord;

	if (maxScroll <= 0)
		return false;

	if (pMax)
		*pMax = maxScroll;

	if (!pHeight)
		return false;

	const int thumbHeight = MaxImpl((*pHeight * this->MaxRecord / totalRecords), MessageToggleClass::ButtonSide);

	if (pPosY)
		*pPosY += (*pHeight - thumbHeight) * this->GetScrollIndex() / maxScroll;

	*pHeight = thumbHeight;

	return true;
}

bool MessageColumnClass::AddRecordString(const std::wstring& message, size_t copySize)
{
	if (message.empty())
		return false;

	ScenarioExtData::Instance()->RecordMessages.push_back(message.substr(0, MinImpl(copySize, message.size())));
	return true;
}

void MessageColumnClass::RemoveTextLabel(MessageLabelClass* pLabel)
{
	this->LabelList = static_cast<MessageLabelClass*>(pLabel->Remove());
	GameDelete<false, false>(pLabel);
}

int MessageColumnClass::GetLabelCount() const
{
	int num = 0;

	for (auto pLabel = this->LabelList; pLabel; pLabel = static_cast<MessageLabelClass*>(pLabel->GetNext()))
		++num;

	return num;
}

MessageLabelClass* MessageColumnClass::GetLastLabel() const
{
	auto pLabel = this->LabelList;

	if (!pLabel)
		return nullptr;

	auto pNextLabel = static_cast<MessageLabelClass*>(pLabel->GetNext());

	for (; pNextLabel; pNextLabel = static_cast<MessageLabelClass*>(pLabel->GetNext()))
		pLabel = pNextLabel;

	return pLabel;
}
template <bool check>
int MessageColumnClass::GetMaxScroll() const
{
	if constexpr (check) {
		if (!ScenarioExtData::Instance())
			return 0;
	}


	return MaxImpl(0, static_cast<int>(ScenarioExtData::Instance()->RecordMessages.size()) - this->MaxRecord);
}