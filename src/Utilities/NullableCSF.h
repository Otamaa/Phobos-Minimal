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

	COMPILETIMEEVAL const wchar_t* c_str() const noexcept
	{
		return this->Text.c_str();
	}

	COMPILETIMEEVAL operator const std::wstring& () const noexcept
	{
		return this->Text;
	}

	COMPILETIMEEVAL const std::wstring& GetText() const noexcept
	{
		return this->Text;
	}

	COMPILETIMEEVAL operator std::wstring& () = delete;
	COMPILETIMEEVAL operator std::wstring() && noexcept = delete;

private:
	std::wstring Text;
};