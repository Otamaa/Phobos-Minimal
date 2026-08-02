#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosFixedString.h>

class ThemeTypeClass final : public Enumerable<ThemeTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "Themes";
	static COMPILETIMEEVAL const char* ClassName = "ThemeTypeClass";

public:

	PhobosFixedString<64U> NextText {};
	PhobosFixedString<100U> HousesText {};

	Valueable<CSFText> UIName {};
	Valueable<bool> Normal { true };
	Valueable<bool> Repeat {};
	Valueable<int> Side { -1 };

	ThemeTypeClass(const char* const pTitle) : Enumerable<ThemeTypeClass> { pTitle } { }
	virtual ~ThemeTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};