#pragma once


struct SavedGames
{
	static int HowManyTimesISavedForThisScenario;
	static bool CreateSubdir();
	static char* FormatPath(const char* pFileName);
};