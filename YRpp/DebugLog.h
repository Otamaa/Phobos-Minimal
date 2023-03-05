#pragma once

#include <ASMMacros.h>

struct GameDebugLog
{
	static void Log(const char* pFormat, ...) 	JMP_STD(0x4068E0);
};