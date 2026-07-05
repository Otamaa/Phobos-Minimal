#include "GameConfig.h"

#include <Utilities/Debug.h>

bool GameConfig::OpenINI(FileAccessMode mode) noexcept
{
	if (!File->IsAvaible() || !File->Open1(mode)) {
		Debug::LogInfo("Failed to Open file [{} - {}] ", this->RequestedFile , this->File->Filename);
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
	if (!File->IsAvaible() || !File->Create() || !File->Open1(mode)) {
		Debug::LogInfo("Failed to Open file [{} - {}] ", this->RequestedFile , this->File->Filename);
		return false;
	}

	Ini.reset(GameCreate<CCINIClass>());
	Ini->ReadCCFile(this->File.get());
	Ini->CurrentSection = nullptr;
	Ini->CurrentSectionName = nullptr;

	return true;
}