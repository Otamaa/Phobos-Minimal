#pragma once

#include <FileFormats/SHP.h>
#include <Utilities/GameUniquePointers.h>

class SHPUtils
{
public:
	// Load SHP with fallback support
	static SHPStruct* LoadSHPFileWithFallback(const char* pFileName, SHPStruct* pFallback = nullptr);
	
	// Load SHP with theater-specific fallback
	static SHPStruct* LoadTheaterSHPWithFallback(const char* pFileName, SHPStruct* pFallback = nullptr);
	
	// Validate SHP structure
	static bool IsValidSHP(SHPStruct* pSHP);
	
	// Get safe frame count (prevents out-of-bounds access)
	static int GetSafeFrameCount(SHPStruct* pSHP);
	
	// Get safe frame index (clamps to valid range)
	static int GetSafeFrameIndex(SHPStruct* pSHP, int requestedFrame);
	
	// Create smart pointer wrapper for SHP
	static UniqueGamePtrC<SHPStruct> MakeUniqueSHP(SHPStruct* pSHP);
}; 