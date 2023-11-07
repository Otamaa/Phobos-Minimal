#include "AresTrajectoryHelper.h"

#include <Ext/BuildingType/Body.h>

bool  AresTrajectoryHelper::SubjectToAnything(
	BulletTypeClass const* pType, BulletTypeExtData const* pTypeExt)
{
	return pType->SubjectToCliffs
		|| pType->SubjectToWalls
		|| pTypeExt->SubjectToSolid;
}

bool AresTrajectoryHelper::IsCliffHit(
	CellClass const* const pSource, CellClass const* const pBefore,
	CellClass const* const pAfter)
{
	return pAfter->GetLevelFrom(pBefore) >= Unsorted::BridgeLevels && pAfter->GetLevelFrom(pSource) > 0;
}

bool AresTrajectoryHelper::IsWallHit(
	CellClass const* const pSource, CellClass const* const pCheck,
	CellClass const* const pTarget, HouseClass const* const pOwner)
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

bool AresTrajectoryHelper::IsBuildingHit(
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CoordStruct const& crdCur, HouseClass const* const pOwner)
{
	auto const pCellBullet = MapClass::Instance->GetCellAt(crdCur);

	if (auto const pBld = pCellBullet->GetBuilding())
	{
		// source building and target buildings are always traversable.
		// this should fix the issue of aircraft being unable to hit buildings
		if (pBld == pSource || pBld == pTarget)
		{
			return false;
		}

		// does the building let allies through?
		auto const isTransparent = RulesExtData::Instance()->AlliedSolidTransparency
			&& pBld->Owner->IsAlliedWith(pOwner);

		if (isTransparent)
		{
			return false;
		}

		auto const pBldType = pBld->Type;

		// open gates are not hit: let it pass through
		if (pBldType->Gate && pBld->IsTraversable())
		{
			return false;
		}

		auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		if (int solidHeight = pBldTypeExt->Solid_Height)
		{
			if (solidHeight < 0)
			{
				solidHeight = pBldType->Height;
			}

			auto const floor = MapClass::Instance->GetCellFloorHeight(crdCur);
			return crdCur.Z <= floor + (solidHeight << 8);
		}
	}

	return false;
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CoordStruct& coords)
{
	return{ std::abs(coords.X), std::abs(coords.Y) };
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CellStruct& cell)
{
	return{ std::abs(cell.X), std::abs(cell.Y) };
}

CellClass* AresTrajectoryHelper::GetObstacle(
	CellClass const* const pCellSource, CellClass const* const pCellTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CellClass const* const pCellBullet, CoordStruct const& crdCur,
	BulletTypeClass const* const pType, BulletTypeExtData const* pTypeExt,
	HouseClass const* const pOwner)
{
	auto const cellCur = CellClass::Coord2Cell(crdCur);
	auto const pCellCur = MapClass::Instance->GetCellAt(cellCur);
	auto const isHit = pType->SubjectToCliffs
		&& AresTrajectoryHelper::IsCliffHit(pCellSource, pCellBullet, pCellCur)

		|| pType->SubjectToWalls
		&& AresTrajectoryHelper::IsWallHit(pCellSource, pCellCur, pCellTarget, pOwner)

		|| pTypeExt && pTypeExt->SubjectToSolid
		&& AresTrajectoryHelper::IsBuildingHit(pSource, pTarget, crdCur, pOwner)

		;

	return isHit ? pCellCur : nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	BulletTypeClass const* const pType,
	BulletTypeExtData const* const pTypeExt,
	HouseClass const* const pOwner)
{
	if (AresTrajectoryHelper::SubjectToAnything(pType, pTypeExt))
	{
		auto const cellTarget = CellClass::Coord2Cell(crdTarget);
		auto const pCellTarget = MapClass::Instance->GetCellAt(cellTarget);

		auto const cellSrc = CellClass::Coord2Cell(crdSrc);
		auto const pCellSrc = MapClass::Instance->GetCellAt(cellSrc);

		auto const delta = AresTrajectoryHelper::AbsoluteDifference(cellSrc - cellTarget);
		auto const maxDelta = static_cast<size_t>(MaxImpl(delta.X, delta.Y));

		auto const step = !maxDelta ? CoordStruct::Empty
			: (crdTarget - crdSrc) * (1.0 / maxDelta);

		CoordStruct crdCur = crdSrc;
		auto pCellCur = pCellSrc;
		for (size_t i = 0; i < maxDelta; ++i)
		{
			if (auto const pCell = AresTrajectoryHelper::GetObstacle(pCellSrc, pCellTarget, pSource,
				pTarget, pCellCur, crdCur, pType, pTypeExt, pOwner))
			{
				return pCell;
			}

			pCellCur = MapClass::Instance->GetCellAt(crdCur);
			crdCur += step;
		}
	}

	return nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstImpenetrableObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	WeaponTypeClass const* const pWeapon, HouseClass const* const pOwner)
{
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjectileExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	if (auto const pCell = AresTrajectoryHelper::FindFirstObstacle(
		crdSrc, crdTarget, pSource, pTarget, pProjectile, pProjectileExt,
		pOwner))
	{
		if (pCell->ConnectsToOverlay(-1, -1))
		{
			if (pWeapon->Warhead->Wall)
			{
				return nullptr;
			}
		}
		else if (auto const pBld = pCell->GetBuilding())
		{
			// only willingfully fire through enemy buildings
			if (!pOwner->IsAlliedWith(pBld))
			{
				auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				// penetrable if warhead level is at least equal to building level
				if (pProjectileExt->Solid_Level >= pBldTypeExt->Solid_Level)
				{
					return nullptr;
				}
			}
		}

		return pCell;
	}

	return nullptr;
}