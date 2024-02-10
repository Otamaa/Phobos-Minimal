/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"

#include <GameStrings.h>
#include <CCFileClass.h>
#include <CCINIClass.h>
#include <ScenarioClass.h>
#include <Unsorted.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x6849C9, ReadScenario_RandomMap, 0x5)
{
	// Vanilla Instructions
	GET(CCINIClass*, pINI, ECX);
	R->EAX(ScenarioClass::ReadScenarioINI(pINI));

	if (SpawnerMain::Configs::Enabled && ScenarioClass::Instance->IsRandom)
		return 0x68496B;

	return 0x6849C9 + 0x5;
}

DEFINE_HOOK(0x686C48, ReadScenarioINI_RandomMap, 0x8)
{
	if (SpawnerMain::Configs::Enabled && !ScenarioClass::Instance->IsRandom)
	{
		GET(CCINIClass*, pINI, EBP);
		ScenarioClass::Instance->IsRandom = pINI->ReadBool(GameStrings::Basic, "RandomMap", ScenarioClass::Instance->IsRandom);

		if (ScenarioClass::Instance->IsRandom)
			return 0x687917;
	}

	return 0;
}

// Hack Get_Starting_locations, it seems to be broken in random maps
// This can be skipped because the starting location in random maps is stored somewhere else
DEFINE_HOOK(0x688564, ScenStruct_ScenStruct_RandomMap, 0x6)
{
	if (SpawnerMain::Configs::Enabled && ScenarioClass::Instance->IsRandom)
		return 0x6885D7;

	return 0;
}

// Hack rmp to read the scenario for unit mods and stuff
DEFINE_HOOK(0x5997AB, MapGeneratorClass_Init_RandomMap, 0x9)
{
	if (SpawnerMain::Configs::Enabled)
	{
		LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0xB0, -0x64));

		const char* pName = &Game::ScenarioName();
		CCFileClass file { pName };

		if (file.Exists())
			pINI->ReadCCFile(&file);
	}

	return 0;
}

// Use Seed from spawn.ini rather than from the map.
DEFINE_HOOK(0x597B76, MapSeedClass_LoadMap_RandomMap, 0x6)
{
	if (SpawnerMain::Configs::Enabled)
		R->EAX(SpawnerMain::GameConfigs::GetGameConfigs()->Seed);

	return 0;
}
