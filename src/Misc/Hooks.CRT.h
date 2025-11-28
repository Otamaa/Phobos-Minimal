#pragma once

struct CRTHooks{
	static void Apply();

protected:
	static void ApplyMallocHooks();
	static void ApplyFreeHooks();
	static void ApplyMathHooks();
	static void ApplyftolHooks();
};