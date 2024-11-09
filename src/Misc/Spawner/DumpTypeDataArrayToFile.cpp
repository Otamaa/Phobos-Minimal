#include "DumpTypeDataArrayToFile.h"

#include <CCFileClass.h>

#include <SmudgeTypeClass.h>
#include <TerrainTypeClass.h>
#include <ParticleSystemTypeClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>
#include <BulletTypeClass.h>
#include <ParticleTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <VoxelAnimTypeClass.h>
#include <ParticleSystemTypeClass.h>

#include <Utilities/GameConfig.h>
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
	GameConfig file { "DumpRulesTypes.ini" };

	file.OpenOrCreateAction([](CCINIClass* pINI , CCFileClass* pFile){
		LogType<HouseTypeClass>("[Countries]", pFile);

		LogType<InfantryTypeClass>("[InfantryTypes]", pFile);
		LogType<UnitTypeClass>("[VehicleTypes]", pFile);
		LogType<AircraftTypeClass>("[AircraftTypes]", pFile);
		LogType<BuildingTypeClass>("[BuildingTypes]", pFile);

		LogType<TerrainTypeClass>("[TerrainTypes]", pFile);
		LogType<SmudgeTypeClass>("[SmudgeTypes]", pFile);
		LogType<OverlayTypeClass>("[OverlayTypes]", pFile);

		LogType<AnimTypeClass>("[Animations]", pFile);
		LogType<VoxelAnimTypeClass>("[VoxelAnims]", pFile);
		LogType<ParticleTypeClass>("[Particles]", pFile);
		LogType<ParticleSystemTypeClass>("[ParticleSystems]", pFile);

		LogType<WeaponTypeClass>("[WeaponTypes]", pFile);
		LogType<SuperWeaponTypeClass>("[SuperWeaponTypes]", pFile);
		LogType<WarheadTypeClass>("[Warheads]", pFile);

		LogType<BulletTypeClass>("[Projectiles]", pFile);
	});

}