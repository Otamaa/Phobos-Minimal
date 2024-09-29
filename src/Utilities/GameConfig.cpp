#include "GameConfig.h"

#include <Utilities/Debug.h>

bool GameConfig::OpenINI(FileAccessMode mode) noexcept
{
	if (!File->Exists() || !File->Open(mode))
	{
		Debug::Log("Failed to Open file %s \n", this->File->FileName);
		return false;
	}

	Ini.reset(GameCreate<CCINIClass>());
	Ini->ReadCCFile(this->File.get());
	Ini->CurrentSection = nullptr;
	Ini->CurrentSectionName = nullptr;

	return true;
}

bool GameConfig::OpenOrCreate(FileAccessMode mode) noexcept
{
	if (!File->Exists() || !File->CreateFileA() || !File->Open(mode))
	{
		Debug::Log("Failed to Open file %s \n", this->File->FileName);
		return false;
	}

	Ini.reset(GameCreate<CCINIClass>());
	Ini->ReadCCFile(this->File.get());
	Ini->CurrentSection = nullptr;
	Ini->CurrentSectionName = nullptr;

	return true;
}