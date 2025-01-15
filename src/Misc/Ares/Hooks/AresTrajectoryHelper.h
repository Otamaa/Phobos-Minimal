#pragma once

#include <Ext/BulletType/Body.h>

class AbstractClass;
class HouseClass;
class CellClass;
class WeaponTypeClass;
class AresTrajectoryHelper
{
private:


	static COMPILETIMEEVAL bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter)
	{
		return pAfter->GetLevelFrom(pBefore) >= Unsorted::BridgeLevels && pAfter->GetLevelFrom(pSource) > 0;
	}

	static COMPILETIMEEVAL bool IsWallHit(
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

	static COMPILETIMEEVAL bool SubjectToObstacles(BulletTypeClass* pBulletType)
	{
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);
		const bool subjectToTerrain = pBulletTypeExt->SubjectToLand.isset() || pBulletTypeExt->SubjectToWater.isset();

		if (!subjectToTerrain)
			return pBulletType->Level;

		return true;;
	}

	static COMPILETIMEEVAL bool SubjectToTerrain(CellClass* pCurrentCell, BulletTypeClass* pBulletType)
	{
		const bool isCellWater = (pCurrentCell->LandType == LandType::Water || pCurrentCell->LandType == LandType::Beach) && pCurrentCell->ContainsBridge();
		const bool isLevel = pBulletType->Level ? pCurrentCell->IsOnFloor() : false;
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);

		if (isLevel && !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset())
			return true;
		else if (!isCellWater && pBulletTypeExt->SubjectToLand.Get(false))
			return pBulletTypeExt->SubjectToLand_Detonate;
		else if (isCellWater && pBulletTypeExt->SubjectToWater.Get(false))
			return pBulletTypeExt->SubjectToWater_Detonate;

		return false;
	}

	// gets whether collision checks are needed
	static COMPILETIMEEVAL bool SubjectToAnything(
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
		BulletTypeExtData const* pTypeExt, HouseClass const* pOwner , bool SubjectToObstacles);

	static CellClass* GetObstacle(
	CellClass const* pCellSource, CellClass const* pCellTarget,
	AbstractClass const* pSource, AbstractClass const* pTarget,
	CellClass const* pCellBullet, CoordStruct const& crdCur,
	BulletTypeClass const* pType, HouseClass const* pOwner, bool SubjectToObstacles);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		BulletTypeClass const* pType,
		BulletTypeExtData const* pTypeExt, HouseClass const* pOwner);

	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		BulletTypeClass* pType, HouseClass const* pOwner)
	{
		const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pType);

		return AresTrajectoryHelper::FindFirstObstacle(
			crdSrc, crdTarget, nullptr, nullptr, pType, pTypeExt, pOwner);
	}

	static CellClass* FindFirstObstacle(
	CoordStruct const& crdSrc,
	CoordStruct const& crdTarget,
	BulletTypeClass* pType,
	AbstractClass const* pSource,
	AbstractClass const* pTarget,
	HouseClass const* pOwner)
	{
		const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pType);

		return AresTrajectoryHelper::FindFirstObstacle(
			crdSrc, crdTarget, pSource, pTarget, pType, pTypeExt, pOwner);
	}

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};