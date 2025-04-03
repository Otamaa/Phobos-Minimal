#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosFixedString.h>

class ThemeTypeClass final : public Enumerable<ThemeTypeClass>
{
public:

	int DefaultTo;
	PhobosFixedString<64U> NextText;
	PhobosFixedString<100U> HousesText;

	Valueable<CSFText> UIName;
	Valueable<bool> Normal;
	Valueable<bool> Repeat;
	Valueable<int> Side;

	ThemeTypeClass(const char* const pTitle) : Enumerable<ThemeTypeClass> { pTitle }
		, NextText {}
		, HousesText {}
		, UIName {}
		, Normal { true }
		, Repeat { false }
		, Side { -1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);

};
