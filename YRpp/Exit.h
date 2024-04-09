#pragma once
#include <ASMMacros.h>

void Fatal(const char* why, ...)
{
	JMP_STD(0x54A8C0);
}

void __fastcall Emergency_Exit(int code)
{
	JMP_STD(0x6BEC50);
}