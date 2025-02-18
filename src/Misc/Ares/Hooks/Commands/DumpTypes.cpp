#include "DumpTypes.h"

#include <SmudgeTypeClass.h>
#include <TerrainTypeClass.h>
#include <ParticleSystemTypeClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>

#include <Utilities/GeneralUtils.h>
#include <Unsorted.h>

#include <AITriggerTypeClass.h>
#include <ScriptTypeClass.h>

template <typename T>
void DumperTypesCommandClass::LogType(const char* pSection) const
{
	Debug::LogInfo("[{}]", pSection);

	int i = 0;
	for (auto pItem : *T::Array)
	{
		Debug::LogInfo("{} ={}", i++, pItem->get_ID());
	}
}

const char* DumperTypesCommandClass::GetName() const
{
	return "Dump Data Types";
}

const wchar_t* DumperTypesCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_TYPES", L"Dump Types");
}

const wchar_t* DumperTypesCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* DumperTypesCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_TYPES_DESC", L"Dumps the current type list to the log");
}

void DumperTypesCommandClass::Execute(WWKey dwUnk) const
{
	Debug::LogInfo("Dumping all Types");

	Debug::LogInfo("Dumping Rules Types");

	LogType<AnimTypeClass>("Animations");
	LogType<WeaponTypeClass>("WeaponTypes");
	LogType<WarheadTypeClass>("Warheads");
	LogType<BulletTypeClass>("Projectiles");

	LogType<HouseTypeClass>("Countries");

	LogType<InfantryTypeClass>("InfantryTypes");
	LogType<UnitTypeClass>("VehicleTypes");
	LogType<AircraftTypeClass>("AircraftTypes");
	LogType<BuildingTypeClass>("BuildingTypes");

	LogType<SuperWeaponTypeClass>("SuperWeaponTypes");
	LogType<SmudgeTypeClass>("SmudgeTypes");
	LogType<OverlayTypeClass>("OverlayTypes");
	LogType<TerrainTypeClass>("TerrainTypes"); // needs class map in YRPP
	LogType<ParticleTypeClass>("Particles");
	LogType<ParticleSystemTypeClass>("ParticleSystems");

	Debug::LogInfo("Dumping Art Types");
	Debug::LogInfo("[Movies]");

	for (int i = 0; i < MovieInfo<GameDeleter>::Array->Count; ++i) {
		Debug::LogInfo("{} = {}", i, MovieInfo<GameDeleter>::Array->Items[i].Name);
	}

	Debug::LogInfo("Dumping AI Types");
	LogType<ScriptTypeClass>("ScriptTypes");
	LogType<TeamTypeClass>("TeamTypes");
	LogType<TaskForceClass>("TaskForces");

	Debug::LogInfo("[AITriggerTypes]");
	for (auto const pItem : *AITriggerTypeClass::Array)
	{
		char Buffer[1024];
		pItem->FormatForSaving(Buffer, sizeof(Buffer));
		Debug::LogInfo("{}", Buffer);
	}

	Debug::LogInfo("[AITriggerTypesEnable]");
	for (auto const pItem : *AITriggerTypeClass::Array)
	{
		Debug::LogInfo("{} = {}", pItem->get_ID(), pItem->IsEnabled ? "yes" : "no");
	}

	// yes , iam sorry
	// sometime i need the rules that you lock behind the mix
	// so i can validate and fix the stuffs , enable it with your own risk ! -Otamaa
	if(Phobos::Otamaa::IsAdmin) {
		GenericNode* sectionNode = CCINIClass::INI_Rules()->Sections.First();
		while (sectionNode)
		{
			if (*((unsigned int*)sectionNode) == 0x7EB73C)
			{
				INIClass::INISection* section = (INIClass::INISection*)sectionNode;
				Debug::LogInfo("[{}]", section->Name);

				GenericNode* entryNode = section->Entries.GenericList::First();
				while (entryNode)
				{
					if (*((unsigned int*)entryNode) == 0x7EB734)
					{
						INIClass::INIEntry* entry = (INIClass::INIEntry*)entryNode;
						unsigned int checksum = 0xAFFEAFFE;

						for (int i = 0; i < section->EntryIndex.Count(); i++)
						{
							const auto data = section->EntryIndex.begin() + i;
							if (data->Data == entry)
							{
								checksum = data->ID;
								break;
							}
						}
						Debug::LogInfo("\t{} = {} (Checksum: {})", entry->Key, entry->Value, checksum);
					}
					entryNode = entryNode->Next();
				}
			}
			sectionNode = sectionNode->Next();
		}
	}

	MessageListClass::Instance->PrintMessage(L"Type data dumped");
}