#pragma once

#include <CoordStruct.h>
#include <Utilities/Iterator.h>

#include <optional>

class AnimTypeClass;
class VoxelAnimTypeClass;
class TechnoClass;
class HouseClass;
struct DebrisSpawners
{
	static void SpawnSecondary(int total , CoordStruct& coord, Iterator<AnimTypeClass*> iter,
				TechnoClass* pTechno, HouseClass* pOwner, HouseClass* pVictim);
	static void Spawn(int MinDeb, int MaxDeb, CoordStruct& coord, Iterator<VoxelAnimTypeClass*> types, Iterator<AnimTypeClass*> typeb,
				Iterator<int> maximums, Iterator<int> minimums, std::optional<bool> IsLimit,
				TechnoClass* pTechno , HouseClass* pOwner , HouseClass* pVictim
		);
};