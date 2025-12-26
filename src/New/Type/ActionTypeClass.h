#pragma once

#include "CursorTypeClass.h"

class ActionTypeClass final : public Enumerable<ActionTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "ActionTypes";
	static COMPILETIMEEVAL const char* ClassName = "ActionTypeClass";

public:

	ValueableIdx<CursorTypeClass> Cursor;
	ValueableIdx<CursorTypeClass> ShroudedCursor;
	Valueable<bool> AllowShrouded;

	ActionTypeClass(const char* pTitle) : Enumerable<ActionTypeClass>(pTitle)
		, Cursor { 21 }
		, ShroudedCursor { 22 }
		, AllowShrouded { false }
	{
		if (AllowShrouded)
			ShroudedCursor = 21;
	}

	static void AddDefaults();

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
};