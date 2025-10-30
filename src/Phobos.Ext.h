#pragma once

#include <Utilities/SavegameDef.h>

struct PhobosExt
{
	static bool LoadGlobal(LPSTREAM pStm);
	static bool SaveGlobal(LPSTREAM pStm);

};