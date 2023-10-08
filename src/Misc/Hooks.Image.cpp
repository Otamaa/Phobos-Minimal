#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <CCINIClass.h>
#include <RulesClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>

void ReplaceImageInfantry(InfantryTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		//Debug::Log("[Phobos] Replacing image for %s with %s.\n", pType->ImageFile, nameBuffer);
		char filename[0x105];
		_makepath_s(filename, 0, 0, nameBuffer, GameStrings::dot_SHP());
		pType->Image = GameCreate<SHPReference>(filename);
	}
}

void ReplaceImageUnit(UnitTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		//Debug::Log("[Phobos] Replacing image for %s with %s.\n", pType->ImageFile, nameBuffer);
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

void ReplaceImageAircraft(AircraftTypeClass* pType)
{
	char nameBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(pType->ImageFile, GameStrings::Image(), 0, nameBuffer, 0x19) > 0)
	{
		if (pType->Voxel)
		{
			//Debug::Log("[Phobos] Replacing image for %s with %s.\n", pType->ImageFile, nameBuffer);
			char savedName[0x19];
			strcpy_s(savedName, pType->ImageFile);
			strcpy_s(pType->ImageFile, nameBuffer);
			pType->LoadVoxel__();
			strcpy_s(pType->ImageFile, savedName);
		}
	}
}

DEFINE_HOOK(0x524734, InfantryTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(InfantryTypeClass*, pType, ESI);
		ReplaceImageInfantry(pType);
	}

	return 0;
}

DEFINE_HOOK(0x524B53, InfantryTypeClass_Load, 0x5)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(BYTE*, poisonedVal, EDI);
		poisonedVal -= 0xE20;
		//const auto TypeResult = R->EDI() - 0xE20;
		//const auto prevEdi = R->EDI();
		//R->EDI(TypeResult);	// 0x524B42 permanently adds 0x520 to the pointer, we want to revert it
		//GET(TechnoTypeClass*, pType, EDI);
		ReplaceImageInfantry(reinterpret_cast<InfantryTypeClass*>(poisonedVal));
		//R->EDI(prevEdi);
	}

	return 0;
}

DEFINE_HOOK(0x747B49, UnitTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(UnitTypeClass*, pType, EDI);
		ReplaceImageUnit(pType);
	}

	return 0;
}

DEFINE_HOOK(0x74809E, UnitTypeClass_Load, 0x9)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(UnitTypeClass*, pType, ESI);
		ReplaceImageUnit(pType);
	}

	return 0;
}

DEFINE_HOOK(0x41CD54, AircraftTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(AircraftTypeClass*, pType, ESI);
		ReplaceImageAircraft(pType);
	}

	return 0;
}

DEFINE_HOOK(0x41CE7E, AircraftTypeClass_Load, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(AircraftTypeClass*, pType, ESI);
		ReplaceImageAircraft(pType);
	}

	return 0;
}