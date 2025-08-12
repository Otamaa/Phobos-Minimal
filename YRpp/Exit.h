#pragma once
#include <ASMMacros.h>

void __cdecl  Fatal(const char* why, ...)
{
	JMP(0x54A8C0);
}

void __fastcall Emergency_Exit(int code)
{
	JMP_FAST(0x6BEC50);
}