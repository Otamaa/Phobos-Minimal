#pragma once

#include <StringTable.h>
#include <MessageListClass.h>
#include <Helpers/String.h>

#include <Utilities/SavegameDef.h>

// provides storage for a csf label with automatic lookup.
class CSFText
{
	static COMPILETIMEEVAL const size_t Capacity = 0x20;
public:
	CSFText() noexcept { }
	explicit CSFText(nullptr_t) noexcept { }

	explicit CSFText(const char* label) noexcept
	{
		*this = label;
	}

	~CSFText() noexcept = default;

	CSFText& operator = (CSFText const& rhs) = default;
	CSFText(const CSFText& other) = default;

	const CSFText& operator = (const char* label)
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

	template<bool check = true>
	void FORCEDINLINE PrintAsMessage(int colorScheme) const
	{

		if COMPILETIMEEVAL (check)
		{
			if (this->empty())
				return;
		}

		MessageListClass::Instance->PrintMessage(this->Text, RulesClass::Instance->MessageDelay, colorScheme);
	}

	COMPILETIMEEVAL operator const wchar_t* () const
	{
		return this->Text;
	}

	COMPILETIMEEVAL bool empty() const
	{
		return !this->Text || !*this->Text;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->Text = nullptr;
		if (Stm.Load(this->Label.data()))
		{
			if (this->Label)
			{
				this->Text = StringTable::FetchString(this->Label);
			}
			return true;
		}
		return false;
	}
	bool save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->Label.data());
		return true;
	}

public:

	FixedString<0x20> Label;
	const wchar_t* Text { nullptr };

};