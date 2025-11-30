#pragma once

struct CRTHooks{
	static void Apply();
	static void Print_FPUMode();
	static void __cdecl _set_fp_mode();
protected:
	static void ApplyMallocHooks();
	static void ApplyFreeHooks();
	static void ApplyMathHooks();
	static void ApplyftolHooks();
};