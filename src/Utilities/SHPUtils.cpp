#include "SHPUtils.h"
#include "GeneralUtils.h"
#include <Utilities/Debug.h>

SHPStruct* SHPUtils::LoadSHPFileWithFallback(const char* pFileName, SHPStruct* pFallback)
{
	if (!pFileName || !*pFileName)
		return pFallback;

	if (auto pSHP = FileSystem::LoadSHPFile(pFileName))
	{
		if (IsValidSHP(pSHP))
			return pSHP;
		
		Debug::LogInfo("Loaded SHP file '{}' is invalid, using fallback", pFileName);
	}
	else
	{
		Debug::LogInfo("Failed to load SHP file '{}', using fallback", pFileName);
	}

	return pFallback;
}

SHPStruct* SHPUtils::LoadTheaterSHPWithFallback(const char* pFileName, SHPStruct* pFallback)
{
	if (!pFileName || !*pFileName)
		return pFallback;

	// Try theater-specific version first
	std::string theaterFile = pFileName;
	GeneralUtils::ApplyTheaterSuffixToString(theaterFile);
	
	if (theaterFile.find(".shp") == std::string::npos)
		theaterFile += ".shp";

	if (auto pSHP = FileSystem::LoadSHPFile(theaterFile.c_str()))
	{
		if (IsValidSHP(pSHP))
			return pSHP;
		
		Debug::LogInfo("Loaded theater SHP file '{}' is invalid", theaterFile.c_str());
	}

	// Try generic version as fallback
	std::string genericFile = pFileName;
	if (genericFile.find(".shp") == std::string::npos)
		genericFile += ".shp";

	if (auto pSHP = FileSystem::LoadSHPFile(genericFile.c_str()))
	{
		if (IsValidSHP(pSHP))
		{
			Debug::LogInfo("Using generic SHP '{}' as fallback for theater-specific '{}'", genericFile.c_str(), theaterFile.c_str());
			return pSHP;
		}
		
		Debug::LogInfo("Loaded generic SHP file '{}' is invalid", genericFile.c_str());
	}

	Debug::LogInfo("Failed to load both theater-specific '{}' and generic '{}' SHP files, using fallback", theaterFile.c_str(), genericFile.c_str());
	return pFallback;
}

bool SHPUtils::IsValidSHP(SHPStruct* pSHP)
{
	if (!pSHP)
		return false;

	// Basic sanity checks
	if (pSHP->Width <= 0 || pSHP->Height <= 0 || pSHP->Frames <= 0)
		return false;

	// Check for reasonable limits (prevent memory issues)
	if (pSHP->Width > 2048 || pSHP->Height > 2048 || pSHP->Frames > 1000)
	{
		Debug::LogInfo("SHP has suspicious dimensions: {}x{} with {} frames", pSHP->Width, pSHP->Height, pSHP->Frames);
		return false;
	}

	return true;
}

int SHPUtils::GetSafeFrameCount(SHPStruct* pSHP)
{
	if (!IsValidSHP(pSHP))
		return 0;
	
	return pSHP->Frames;
}

int SHPUtils::GetSafeFrameIndex(SHPStruct* pSHP, int requestedFrame)
{
	if (!IsValidSHP(pSHP))
		return 0;

	if (requestedFrame < 0)
		return 0;
	
	if (requestedFrame >= pSHP->Frames)
		return pSHP->Frames - 1;
	
	return requestedFrame;
}

UniqueGamePtrC<SHPStruct> SHPUtils::MakeUniqueSHP(SHPStruct* pSHP)
{
	return UniqueGamePtrC<SHPStruct>(pSHP);
} 