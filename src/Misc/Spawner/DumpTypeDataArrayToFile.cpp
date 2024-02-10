#include "DumpTypeDataArrayToFile.h"

#include <CCFileClass.h>

#include <SmudgeTypeClass.h>
#include <TerrainTypeClass.h>
#include <ParticleSystemTypeClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>

#include <Utilities/GeneralUtils.h>
#include <Unsorted.h>

void DumpTypeDataArrayToFile::writeLog(CCFileClass* file, const char* pSectionName)
{
	file->WriteBytes(const_cast<char*>(pSectionName), strlen(pSectionName));
}

template <typename T>
static void LogType(const char* pSectionName, CCFileClass* file)
{
	DumpTypeDataArrayToFile::writeLog(file, pSectionName);
	DumpTypeDataArrayToFile::writeLog(file, "\n");

	int index = 1;
	char indexStr[10];;

	for (auto pItem : *T::Array)
	{
		sprintf(indexStr, "%d", index++);
		DumpTypeDataArrayToFile::writeLog(file, indexStr);

		DumpTypeDataArrayToFile::writeLog(file, "=");
		char* id = (char*)pItem->get_ID();
		DumpTypeDataArrayToFile::writeLog(file, id);

		if (strcmp(id, pItem->Name) != 0)
		{
			DumpTypeDataArrayToFile::writeLog(file, " ;");
			DumpTypeDataArrayToFile::writeLog(file, pItem->Name);
		}
		DumpTypeDataArrayToFile::writeLog(file, "\n");
	}

	DumpTypeDataArrayToFile::writeLog(file, "\n");
};

void DumpTypeDataArrayToFile::Dump()
{
	CCFileClass file = CCFileClass("DumpRulesTypes.ini");

	if (!file.Exists()) {
		if (!file.CreateFileA()) {
			return;
		}
	}

	if (file.Open(FileAccessMode::Write))
	{
		LogType<HouseTypeClass>("[Countries]", &file);

		LogType<InfantryTypeClass>("[InfantryTypes]", &file);
		LogType<UnitTypeClass>("[VehicleTypes]", &file);
		LogType<AircraftTypeClass>("[AircraftTypes]", &file);
		LogType<BuildingTypeClass>("[BuildingTypes]", &file);

		LogType<TerrainTypeClass>("[TerrainTypes]", &file);
		LogType<SmudgeTypeClass>("[SmudgeTypes]", &file);
		LogType<OverlayTypeClass>("[OverlayTypes]", &file);

		LogType<AnimTypeClass>("[Animations]", &file);
		LogType<VoxelAnimTypeClass>("[VoxelAnims]", &file);
		LogType<ParticleTypeClass>("[Particles]", &file);
		LogType<ParticleSystemTypeClass>("[ParticleSystems]", &file);

		LogType<WeaponTypeClass>("[WeaponTypes]", &file);
		LogType<SuperWeaponTypeClass>("[SuperWeaponTypes]", &file);
		LogType<WarheadTypeClass>("[Warheads]", &file);

		LogType<BulletTypeClass>("[Projectiles]", &file);
	}
	else
	{
		Debug::Log(" %s Failed to Open file %s for\n", __FUNCTION__, file.FileName);
	}
}