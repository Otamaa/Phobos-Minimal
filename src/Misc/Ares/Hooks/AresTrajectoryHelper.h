#pragma once

#include <Ext/BulletType/Body.h>

class AbstractClass;
class HouseClass;
class CellClass;
class WeaponTypeClass;
class AresTrajectoryHelper
{
private:
	static constexpr bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter)
	{
		return pAfter->GetLevelFrom(pBefore) >= Unsorted::BridgeLevels && pAfter->GetLevelFrom(pSource) > 0;
	}

	static constexpr bool IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass const* pOwner)
	{
		if (pCheck != pTarget && pCheck->OverlayTypeIndex != -1)
		{
			if (OverlayTypeClass::Array->Items[pCheck->OverlayTypeIndex]->Wall)
			{
				if (pSource->Level <= pTarget->Level)
				{
					return !RulesClass::Instance->AlliedWallTransparency
						|| (pCheck->WallOwnerIndex != -1 && !HouseClass::Array->Items[pCheck->WallOwnerIndex]->IsAlliedWith(pOwner));
				}
			}
		}

		return false;
	}

	static bool IsBuildingHit(
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CoordStruct const& crdCur, HouseClass const* pOwner);

	static Vector2D<int> AbsoluteDifference(const CoordStruct& coords);

	static Vector2D<int> AbsoluteDifference(const CellStruct& cell);

public:
	// gets whether collision checks are needed
	static constexpr bool SubjectToAnything(
		BulletTypeClass const* pType, BulletTypeExtData const* pTypeExt)
	{
		return pType->SubjectToCliffs
			|| pType->SubjectToWalls
			|| pTypeExt->SubjectToSolid;
	}

	// gets the obstacle when moving from pCellBullet to crdCur
	static CellClass* GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CellClass const* pCellBullet, CoordStruct const& crdCur,
		BulletTypeClass const* pType,
		BulletTypeExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		BulletTypeClass const* pType,
		BulletTypeExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};