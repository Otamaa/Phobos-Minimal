#pragma once

#include <CellClass.h>

class BulletTypeClass;
class AbstractClass;
class HouseClass;
class CellClass;
class WeaponTypeClass;
struct BulletObstacleHelper {
	static COMPILETIMEEVAL bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter)
	{
		return pAfter->GetLevelFrom(pBefore) >= Unsorted::BridgeLevels && pAfter->GetLevelFrom(pSource) > 0;
	}

	static COMPILETIMEEVAL Vector2D<int> AbsoluteDifference(const CoordStruct& coords)
	{
		return{ Math::abs(coords.X), Math::abs(coords.Y) };
	}

	static COMPILETIMEEVAL Vector2D<int> AbsoluteDifference(const CellStruct& cell)
	{
		return{ Math::abs(cell.X), Math::abs(cell.Y) };
	}
};

struct AresBulletObstacleHelper {
	// gets whether collision checks are needed
	static bool SubjectToAnything(BulletTypeClass* pType);

	static bool IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass* pOwner);

	static bool IsBuildingHit(
		AbstractClass* pSource, AbstractClass* pTarget,
		CoordStruct const& crdCur, HouseClass* pOwner);

	// gets the obstacle when moving from pCellBullet to crdCur
	static CellClass* GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		AbstractClass* pSource, AbstractClass* pTarget,
		CellClass const* pCellBullet, CoordStruct const& crdCur,
		BulletTypeClass* pType, HouseClass* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass* pSource, AbstractClass* pTarget,
		BulletTypeClass* pType, HouseClass* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass* pSource, AbstractClass* pTarget,
		WeaponTypeClass* pWeapon, HouseClass* pOwner);
};

struct PhobosBulletObstacleHelper {
	static CellClass* GetObstacle(CellClass* pSourceCell,
		CellClass* pTargetCell,
		CellClass* pCurrentCell,
		CoordStruct currentCoords,
		AbstractClass* pSource,
		AbstractClass* pTarget,
		HouseClass* pOwner,
		BulletTypeClass* pBulletType, bool isTargetingCheck = false);

	static CellClass* FindFirstObstacle(CoordStruct const& pSourceCoords,
		CoordStruct const& pTargetCoords,
		AbstractClass* pSource,
		AbstractClass* pTarget,
		HouseClass* pOwner,
		BulletTypeClass* pBulletType,
		bool isTargetingCheck = false,
		bool subjectToGround = false);

	static CellClass* FindFirstImpenetrableObstacle(CoordStruct const& pSourceCoords,
		CoordStruct const& pTargetCoords,
		AbstractClass* pSource,
		AbstractClass* pTarget,
		HouseClass* pOwner,
		WeaponTypeClass* pWeapon,
		bool isTargetingCheck = false,
		bool subjectToGround = false);

	static bool SubjectToObstacles(BulletTypeClass* pBulletType);

	static bool SubjectToTerrain(CellClass* pCurrentCell, BulletTypeClass* pBulletType, bool isTargetingCheck);

	static CoordStruct AddFLHToSourceCoords(const CoordStruct& sourceCoords,
		const CoordStruct& targetCoords,
		TechnoClass* pTechno,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		bool& subjectToGround);
};