#pragma once

#include <YRPP.h>
#include <Helpers/CompileTime.h>

class NOVTABLE EvadeClass
{
public:
	static COMPILETIMEEVAL reference<EvadeClass, 0x8A38E0> Instance {};

	void Do() { JMP_THIS(0x4C6210); }
	bool Save(IStream* pStm) JMP_THIS(0x4C6340);
	bool Load(IStream* pStm) JMP_THIS(0x4C6320);

	//Properties
public:
	char  CarryOverGlobals[50];
	int   CarryOverMoney;
	int   CarryOverTimer;
	int   CarryOverDifficulty;
	short CarryOverStage;
};
static_assert(sizeof(EvadeClass) == 0x44);