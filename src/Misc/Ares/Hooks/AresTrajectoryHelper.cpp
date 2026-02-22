#include "AresTrajectoryHelper.h"

#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>

#pragma region AresBulletObstacleHelper

bool AresBulletObstacleHelper::IsWallHit(
	CellClass const* pSource,
	CellClass const* pCheck,
	CellClass const* pTarget,
	 HouseClass* pOwner) {
	if (pCheck != pTarget && pCheck->OverlayTypeIndex != -1) {
		if (OverlayTypeClass::Array->Items[pCheck->OverlayTypeIndex]->Wall) {
			if (pSource->Level <= pTarget->Level) {
				return !RulesClass::Instance->AlliedWallTransparency
				|| (pCheck->WallOwnerIndex != -1 && !HouseClass::Array->Items[pCheck->WallOwnerIndex]->IsAlliedWith(pOwner));
			}
		}
	}

	return false;
}

bool AresBulletObstacleHelper::IsBuildingHit(
	AbstractClass* pSource,
	AbstractClass* pTarget,
	CoordStruct const& crdCur,
	HouseClass* pOwner)
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

CellClass* AresBulletObstacleHelper::GetObstacle(
	CellClass const* const pCellSource,
	CellClass const* const pCellTarget,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	CellClass const* const pCellBullet,
	CoordStruct const& crdCur,
	BulletTypeClass* pType,
	HouseClass* pOwner)
{
	auto const cellCur = CellClass::Coord2Cell(crdCur);
	auto const pCellCur = MapClass::Instance->GetCellAt(cellCur);

	auto const IsCliffHit = [=]() {
		return pType->SubjectToCliffs
			&& BulletObstacleHelper::IsCliffHit(pCellSource, pCellBullet, pCellCur);
	};

	auto const IsWallHit = [=]() {
		return pType->SubjectToWalls
			&& AresBulletObstacleHelper::IsWallHit(pCellSource, pCellCur, pCellTarget, pOwner);
	};

	auto const IsBuildingHit = [=]() {
		return BulletTypeExtContainer::Instance.Find(pType)->SubjectToSolid
			&& AresBulletObstacleHelper::IsBuildingHit(pSource, pTarget, crdCur, pOwner);
	};

	auto const isHit = IsCliffHit() || IsWallHit() || IsBuildingHit();

	return isHit ? pCellCur : nullptr;
}

bool AresBulletObstacleHelper::SubjectToAnything(
	BulletTypeClass* pType)
{
	auto pTypeExt = BulletTypeExtContainer::Instance.Find(pType);
	return pType->SubjectToCliffs
		|| pType->SubjectToWalls
		|| pTypeExt->SubjectToSolid;
}

CellClass* AresBulletObstacleHelper::FindFirstObstacle(
	CoordStruct const& crdSrc,
	CoordStruct const& crdTarget,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	BulletTypeClass* pType,
	HouseClass* pOwner)
{
	if(AresBulletObstacleHelper::SubjectToAnything(pType)) {
		auto const cellTarget = CellClass::Coord2Cell(crdTarget);
		auto const pCellTarget = MapClass::Instance->GetCellAt(cellTarget);

		auto const cellSrc = CellClass::Coord2Cell(crdSrc);
		auto const pCellSrc = MapClass::Instance->GetCellAt(cellSrc);

		auto const delta = BulletObstacleHelper::AbsoluteDifference(cellSrc - cellTarget);
		auto const maxDelta = static_cast<size_t>(MaxImpl(delta.X, delta.Y));

		auto const step = !maxDelta ? CoordStruct::Empty
			: (crdTarget - crdSrc) * (1.0 / maxDelta);

		auto crdCur = crdSrc;
		auto pCellCur = pCellSrc;
		for(size_t i = 0; i < maxDelta; ++i) {
			if(auto const pCell = AresBulletObstacleHelper::GetObstacle(pCellSrc, pCellTarget, pSource,
				pTarget, pCellCur, crdCur, pType, pOwner))
			{
				return pCell;
			}

			pCellCur = MapClass::Instance->GetCellAt(crdCur);
			crdCur += step;
		}
	}

	return nullptr;
}

CellClass* AresBulletObstacleHelper::FindFirstImpenetrableObstacle(
	CoordStruct const& crdSrc,
	CoordStruct const& crdTarget,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	HouseClass* pOwner)
{
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjectileExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	if(auto const pCell = AresBulletObstacleHelper::FindFirstObstacle(
		crdSrc, crdTarget, pSource, pTarget, pProjectile, pOwner))
	{
		if(pCell->ConnectsToOverlay(-1, -1)) {
			if(pWeapon->Warhead->Wall) {
				return nullptr;
			}
		} else if(auto const pBld = pCell->GetBuilding()) {
			// only willingfully fire through enemy buildings
			if(!pOwner->IsAlliedWith(pBld)) {
				auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				// penetrable if warhead level is at least equal to building level
				if(pProjectileExt->Solid_Level >= pBldTypeExt->Solid_Level) {
					return nullptr;
				}
			}
		}

		return pCell;
	}

	return nullptr;
}

#pragma endregion

#pragma region PhobosBulletObstacleHelper

CellClass* PhobosBulletObstacleHelper::GetObstacle(
	CellClass* pSourceCell,
	CellClass* pTargetCell,
	CellClass* pCurrentCell,
	CoordStruct currentCoords,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	HouseClass* pOwner,
	BulletTypeClass* pBulletType,
	bool isTargetingCheck)
{
	if (PhobosBulletObstacleHelper::SubjectToObstacles(pBulletType))
		if (PhobosBulletObstacleHelper::SubjectToTerrain(pCurrentCell, pBulletType, isTargetingCheck))
			return pCurrentCell;

	return nullptr;
}

CellClass* PhobosBulletObstacleHelper::FindFirstObstacle(
	CoordStruct const& pSourceCoords,
	CoordStruct const& pTargetCoords,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	HouseClass* pOwner,
	BulletTypeClass* pBulletType,
	bool isTargetingCheck,
	bool subjectToGround)
{
	if (PhobosBulletObstacleHelper::SubjectToObstacles(pBulletType) || subjectToGround) {
		auto sourceCell = CellClass::Coord2Cell(pSourceCoords);
		auto const pSourceCell = MapClass::Instance->GetCellAt(sourceCell);
		auto targetCell = CellClass::Coord2Cell(pTargetCoords);
		auto const pTargetCell = MapClass::Instance->GetCellAt(targetCell);

		auto const sub = sourceCell - targetCell;
		auto const delta = CellStruct { (short)Math::abs(sub.X), (short)Math::abs(sub.Y) };
		auto const maxDelta = static_cast<size_t>(MaxImpl(delta.X, delta.Y));
		auto const step = !maxDelta ? CoordStruct::Empty : (pTargetCoords - pSourceCoords) * (1.0 / maxDelta);
		CoordStruct crdCur = pSourceCoords;
		auto pCellCur = pSourceCell;

		for (size_t i = 0; i < maxDelta + isTargetingCheck; ++i)
		{
			if (auto const pCell = GetObstacle(pSourceCell, pTargetCell, pCellCur, crdCur, pSource, pTarget, pOwner, pBulletType, isTargetingCheck))
				return pCell;

			if (subjectToGround && crdCur.Z < MapClass::Instance->GetCellFloorHeight(crdCur))
				return pCellCur;

				pCellCur = MapClass::Instance->GetCellAt(crdCur);
				crdCur += step;
			}
		}

	return nullptr;
}

CellClass* PhobosBulletObstacleHelper::FindFirstImpenetrableObstacle(CoordStruct const& pSourceCoords,
	CoordStruct const& pTargetCoords,
	AbstractClass* pSource,
	AbstractClass* pTarget,
	HouseClass* pOwner,
	WeaponTypeClass* pWeapon,
	bool isTargetingCheck,
	bool subjectToGround)
{
	// Does not currently need further checks.
	return PhobosBulletObstacleHelper::FindFirstObstacle(pSourceCoords, pTargetCoords, pSource, pTarget, pOwner, pWeapon->Projectile, isTargetingCheck, subjectToGround);
}

bool PhobosBulletObstacleHelper::SubjectToObstacles(
	BulletTypeClass* pBulletType){
	const bool subjectToTerrain = BulletTypeExtContainer::Instance.Find(pBulletType)->SubjectToLand.isset() || BulletTypeExtContainer::Instance.Find(pBulletType)->SubjectToWater.isset();
	return subjectToTerrain || pBulletType->Level;
}

bool PhobosBulletObstacleHelper::SubjectToTerrain(
	CellClass* pCurrentCell,
	BulletTypeClass* pBulletType,
	bool isTargetingCheck) {
	bool isCellWater = pCurrentCell->LandType == LandType::Water || pCurrentCell->LandType == LandType::Beach;
	bool isLevel = pBulletType->Level ? pCurrentCell->IsOnFloor() : false;

	auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);

	if (!isTargetingCheck && isLevel && !pBulletTypeExt->SubjectToLand.isset() && !pBulletTypeExt->SubjectToWater.isset())
		return true;
	else if (!isCellWater && pBulletTypeExt->SubjectToLand.Get(false))
		return !isTargetingCheck ? pBulletTypeExt->SubjectToLand_Detonate : true;
	else if (isCellWater && pBulletTypeExt->SubjectToWater.Get(false))
		return !isTargetingCheck ? pBulletTypeExt->SubjectToWater_Detonate : true;

	return false;
}

CoordStruct PhobosBulletObstacleHelper::AddFLHToSourceCoords(
	const CoordStruct& sourceCoords,
	const CoordStruct& targetCoords,
	TechnoClass* pTechno,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	bool& subjectToGround)
{
	// Buildings, air force, and passengers are not allowed, because they don't even know how to find a suitable location
	if (((pTechno->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None) || pTechno->IsInAir() || pTarget->IsInAir() || pTechno->Transporter)
	{
		// Turn off the switch for subsequent checks
		subjectToGround = false;
		return sourceCoords;
	}
	// Predicting the firing position of weapons
	// Unable to predict the degree of inclination of the techno yet
	Matrix3D mtx = Matrix3D::GetIdentity();
	// Position on the ground or bridge
	const auto pCell = MapClass::Instance->GetCellAt(sourceCoords);
	const auto source = pTechno->OnBridge ? pCell->GetCoordsWithBridge() : pCell->GetCoords();
	// Predicted orientation
	float radian = (float)(-Math::atan2(float(targetCoords.Y - source.Y), float(targetCoords.X - source.X)));
	mtx.RotateZ(radian);
	// Offset of turret, directly substitute because it is impossible to predict the orientation of the techno when it reaches this position
	// Only predict the situation when the techno is facing the target directly
	if (pTechno->HasTurret())
		GET_TECHNOTYPEEXT(pTechno)->ApplyTurretOffset(&mtx, 1.0);

	// FLH of weapon, not use independent firing positions
	// Because this will result in different results due to the current burst, causing the techno to constantly move
	const auto pWeaponStruct = pTechno->GetWeapon(pTechno->SelectWeapon(pTarget));
	const auto& flh = pWeaponStruct->FLH;
	// If the weapon's burst is greater than 1, use the center firing position for calculation
	// This can avoid constantly searching for position and pacing back and forth
	mtx.Translate((float)flh.X, ((pWeaponStruct->WeaponType == pWeapon && pWeapon->Burst != 1) ? 0 : ((float)flh.Y)), (float)flh.Z);
	const auto result = mtx.GetTranslation();
	// Starting from the center position of the cell and adding the offset value
	return source + CoordStruct { (int)result.X, -(int)result.Y, (int)result.Z };
}

#pragma endregion