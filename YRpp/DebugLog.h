#pragma once

#include <ASMMacros.h>
#include <Base/Always.h>

struct GameDebugLog
{
	static void Log(const char* pFormat, ...) 	JMP_STD(0x4068E0);

	static NOINLINE void HookLogEnd(DWORD addr, const char* pFormat, size_t size, long long time);
	static NOINLINE void HookLogStart(DWORD addr, const char* pFormat, size_t size);
};