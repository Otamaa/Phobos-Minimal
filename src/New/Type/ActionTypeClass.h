#pragma once

#include "CursorTypeClass.h"

class ActionTypeClass final : public Enumerable<ActionTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "ActionTypes";
	static COMPILETIMEEVAL const char* ClassName = "ActionTypeClass";

public:

	ValueableIdx<CursorTypeClass*> Cursor { 21 };
	ValueableIdx<CursorTypeClass*> ShroudedCursor { 22 };
	Valueable<bool> AllowShrouded { false };

	ActionTypeClass(const char* pTitle) : Enumerable<ActionTypeClass>(pTitle) {	}
	virtual ~ActionTypeClass() = default;

	static void AddDefaults();

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
};