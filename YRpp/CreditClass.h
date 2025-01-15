#pragma once

#include <CRT.h>

class CreditClass
{
	static COMPILETIMEEVAL reference<int, 0x89F95Cu> const LastUpdateFrame {};

	void DrawIt(bool bForced) const {
		JMP_THIS(0x4A2370);
	}

	void Update(bool bForced) const {
		JMP_THIS(0x4A2600);
	}

public :
	int Credits { 0 };
	int Current { 0 };
	bool IsToRedraw { false };
	bool IsUp { false };
	bool IsAudible { false };
	int Countdown { 0 };
};

static_assert(sizeof(CreditClass) == 0x10, "Invalid Size !");
typedef CreditClass TabDataClass ;