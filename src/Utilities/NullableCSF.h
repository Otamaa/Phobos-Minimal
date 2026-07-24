#pragma once

#include <string>
#include <Base/Always.h>

class CCINIClass;
struct NullableCSF
{
	NullableCSF(const wchar_t* defaultText) noexcept
		: Text(defaultText)
	{}

	void Read(CCINIClass* pINI, const char* pSection, const char* pKey);
	void Set(const wchar_t* _to) { this->Text = _to; };

	COMPILETIMEEVAL const wchar_t* c_str() const noexcept
	{
		return this->Text;
	}

private:
	const wchar_t* Text;
};