#include "DebrisSpawners.h"

#include <ScenarioClass.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Anim/Body.h>

void DebrisSpawners::SpawnSecondary(int total, CoordStruct& coord, Iterator<AnimTypeClass*> iter,
				TechnoClass* pTechno, HouseClass* pOwner, HouseClass* pVictim)
{
	if (!iter.empty()) {
		CoordStruct _coord = coord;
		_coord.Z += 20;

		for (int i = 0; i < total; ++i) {
			if (auto const pAnimType = iter[ScenarioClass::Instance->Random.RandomFromMax(iter.size() - 1)]) {
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, _coord),
					pOwner,
					pVictim,
					pTechno,
					false, false
				);
			}
		}
	}
}

void DebrisSpawners::Spawn(int MinDeb, int MaxDeb, CoordStruct& coord, Iterator<VoxelAnimTypeClass*> types, Iterator<AnimTypeClass*> typeb,
				Iterator<int> maximums, Iterator<int> minimums, std::optional<bool> IsLimit,
				TechnoClass* pTechno, HouseClass* pOwner, HouseClass* pVictim)
{
	int totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(MinDeb, MaxDeb);

	if (totalSpawnAmount > 0)
	{
		int count = MinImpl(types.size(), maximums.size());

		if (count > 0)
		{
			const bool limit = IsLimit.value_or(count > 1);
			const int minIndex = static_cast<int>(minimums.size()) - 1;
			int currentIndex = 0;

			while (totalSpawnAmount > 0)
			{
				const int currentMaxDebris = MinImpl(1, maximums[currentIndex]);
				const int currentMinDebris = (minIndex >= 0) ? MaxImpl(0, minimums[MinImpl(currentIndex, minIndex)]) : 0;
				int amountToSpawn = MinImpl(totalSpawnAmount, ScenarioClass::Instance->Random.RandomRanged(currentMinDebris, currentMaxDebris));
				totalSpawnAmount -= amountToSpawn;

				for (; amountToSpawn > 0; --amountToSpawn)
				{
					auto pVoxAnim = GameCreate<VoxelAnimClass>(types[currentIndex], &coord, pOwner);
					VoxelAnimExtContainer::Instance.Find(pVoxAnim)->Invoker = pTechno;
				}

				if (totalSpawnAmount <= 0)
					break;

				if (++currentIndex < count)
					continue;

				if (limit)
					break;

				currentIndex = 0;
			}
		} else {

			DebrisSpawners::SpawnSecondary(totalSpawnAmount , coord, typeb, pTechno, pOwner, pVictim);
		}
	}
}
