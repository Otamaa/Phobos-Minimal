#include "ImageSwapModules.h"

#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>

#include <Phobos.h>

#include <CCINIClass.h>

void TechnoImageReplacer::Replace(InfantryTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		char filename[0x105];
		_makepath_s(filename, 0, 0, nameBuffer, GameStrings::dot_SHP());
		pType->Image = GameCreate<SHPReference>(filename);
	}
}

void TechnoImageReplacer::Replace(UnitTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		if (pType->Voxel)
		{
			char savedName[0x19];
			strcpy_s(savedName, pType->ImageFile);
			strcpy_s(pType->ImageFile, nameBuffer);
			pType->LoadVoxel__();
			strcpy_s(pType->ImageFile, savedName);
		}
		else
		{
			char filename[0x105];
			_makepath_s(filename, 0, 0, nameBuffer, GameStrings::dot_SHP());
			pType->Image = GameCreate<SHPReference>(filename);
		}
	}
}

void TechnoImageReplacer::Replace(AircraftTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		if (pType->Voxel)
		{
			char savedName[0x19];
			strcpy_s(savedName, pType->ImageFile);
			strcpy_s(pType->ImageFile, nameBuffer);
			pType->LoadVoxel__();
			strcpy_s(pType->ImageFile, savedName);
		}
	}
}


