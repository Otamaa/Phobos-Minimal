#include "CSFText.h"

#include <Utilities/SavegameDef.h>

#include <StringTable.h>
#include <MessageListClass.h>
#include <RulesClass.h>

CSFText::CSFText(const char* label) noexcept
{
	*this = label;
}

const CSFText& CSFText::operator = (const char* label)
{
	if (this->Label != label)
	{
		this->Label.assign(label);

		this->Text = nullptr;

		if (this->Label)
		{
			this->Text = StringTable::FetchString(this->Label);
		}
	}

	return *this;
}

bool CSFText::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Text = nullptr;
	if (Stm.Process(this->Label))
	{
		if (this->Label)
		{
			this->Text = StringTable::FetchString(this->Label);
		}
		return true;
	}
	return false;
}

bool CSFText::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->Label);
}

template <>
void CSFText::PrintAsMessage<true>(int colorScheme) const
{
	if (this->empty())
			return;

	MessageListClass::Instance->PrintMessage(this->Text, RulesClass::Instance->MessageDelay, colorScheme);
}

template <>
void CSFText::PrintAsMessage<false>(int colorScheme) const
{
	MessageListClass::Instance->PrintMessage(this->Text, RulesClass::Instance->MessageDelay, colorScheme);
}