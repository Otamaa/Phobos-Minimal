#pragma once

#include <CoordStruct.h>
#include <BuildingClass.h>
#include <MapClass.h>

enum class PassError : size_t
{
	NONE = 0,
	PASS = 1, // 可通行
	UNDERGROUND = 2, // 潜地
	ONWATER = 3, // 掉水上
	HITWALL = 4, // 不可通行
	HITBUILDING = 5, // 撞建筑
	DOWNBRIDGE = 6, // 从上方撞桥
	UPBRIDEG = 7 // 从下方撞桥
};

struct PhysicsHelper
{
	static PassError CanMoveTo(const CoordStruct& sourcePos, CoordStruct nextPos, bool passBuilding, CoordStruct& nextCellPos, bool& onBridge)
	{
		PassError canPass = PassError::PASS;

		nextCellPos = sourcePos;
		onBridge = false;
		int deltaZ = sourcePos.Z - nextPos.Z;

		if (auto pTargetCell = MapClass::Instance->TryGetCellAt(nextPos))
		{
			nextCellPos = pTargetCell->GetCoordsWithBridge();
			onBridge = pTargetCell->ContainsBridge();
			if (nextCellPos.Z >= nextPos.Z)
			{
				nextPos.Z = nextCellPos.Z;
				canPass = PassError::UNDERGROUND;

				if (pTargetCell->TileIs(TileType::Cliff) || pTargetCell->TileIs(TileType::DestroyableCliff))
				{
					if (deltaZ <= 0)
					{
						canPass = PassError::HITWALL;
					}
				}
			}

			if (canPass == PassError::UNDERGROUND && onBridge)
			{
				int bridgeHeight = nextCellPos.Z;
				if (sourcePos.Z > bridgeHeight && nextPos.Z <= bridgeHeight)
				{
					canPass = PassError::DOWNBRIDGE;
				}
				else if (sourcePos.Z < bridgeHeight && nextPos.Z >= bridgeHeight)
				{
					canPass = PassError::UPBRIDEG;
				}
			}


			if (!passBuilding)
			{
				if (BuildingClass* pBuilding = pTargetCell->GetBuilding())
				{
					if (CanHit(pBuilding, nextPos.Z))
					{
						canPass = PassError::HITBUILDING;
					}
				}
			}
		}

		return canPass;
	}

	static bool CanHit(BuildingClass* pBuilding, int targetZ, bool blade = false, int zOffset = 0)
	{
		if (!blade)
		{
			const int height = pBuilding->GetHeight();
			const int sourceZ = pBuilding->GetCoords().Z;
			return targetZ <= (sourceZ + height * Unsorted::LevelHeight + zOffset);
		}
		return blade;
	}
};
