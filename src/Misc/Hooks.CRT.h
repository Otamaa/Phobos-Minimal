#pragma once

struct CRTHooks{
	static void Apply();
	static void Print_FPUMode();
protected:
	static void ApplyMallocHooks();
	static void ApplyFreeHooks();
	static void ApplyMathHooks();
	static void ApplyftolHooks();

public :

	static unsigned short g_cached_game_codeword;

	static void Initializeftol();
};