#pragma once
#include <Base/Always.h>

struct ReshadeContainer
{
	static BOOL Attach(HMODULE hModule);
	static void Detach();
};