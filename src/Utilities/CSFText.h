#pragma once

#include <Helpers/String.h>

class PhobosStreamReader;
class PhobosStreamWriter;
// provides storage for a csf label with automatic lookup.
class CSFText
{
	static COMPILETIMEEVAL const size_t Capacity = 0x20;

public:

	CSFText() noexcept { }
	explicit CSFText(nullptr_t) noexcept { }
	explicit CSFText(const char* label) noexcept;

	~CSFText() noexcept = default;

	CSFText& operator = (CSFText const& rhs) = default;
	CSFText(const CSFText& other) = default;

	const CSFText& operator = (const char* label);

	template<bool check = true>
	void PrintAsMessage(int colorScheme) const;

	COMPILETIMEEVAL operator const wchar_t* () const
	{
		return this->Text;
	}

	COMPILETIMEEVAL bool empty() const
	{
		return !this->Text || !*this->Text;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

public:

	FixedString<0x20> Label;
	const wchar_t* Text { nullptr };

};