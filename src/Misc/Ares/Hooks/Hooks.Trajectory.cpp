#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/BuildingType/Body.h>

class AresTrajectoryHelper
{
private:
	static bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter);

	static bool IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass const* pOwner);

	static bool IsBuildingHit(
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CoordStruct const& crdCur, HouseClass const* pOwner);

	static Vector2D<int> AbsoluteDifference(const CoordStruct& coords);

	static Vector2D<int> AbsoluteDifference(const CellStruct& cell);

public:
	// gets whether collision checks are needed
	static bool SubjectToAnything(
		BulletTypeClass const* pType, BulletTypeExt::ExtData const* pTypeExt)
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
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};

bool AresTrajectoryHelper::IsCliffHit(
	CellClass const* const pSource, CellClass const* const pBefore,
	CellClass const* const pAfter)
{
	return pAfter->GetLevelFrom(pBefore) >= CellClass::BridgeLevels && pAfter->GetLevelFrom(pSource) > 0;
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
					|| !HouseClass::Array->Items[pCheck->WallOwnerIndex]->IsAlliedWith_(pOwner);
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
		auto const isTransparent = RulesExt::Global()->AlliedSolidTransparency
			&& pBld->Owner->IsAlliedWith_(pOwner);

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

		auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldType);
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
	BulletTypeClass const* const pType, BulletTypeExt::ExtData const* pTypeExt,
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
	BulletTypeExt::ExtData const* const pTypeExt,
	HouseClass const* const pOwner)
{
	if (AresTrajectoryHelper::SubjectToAnything(pType, pTypeExt))
	{
		auto const cellTarget = CellClass::Coord2Cell(crdTarget);
		auto const pCellTarget = MapClass::Instance->GetCellAt(cellTarget);

		auto const cellSrc = CellClass::Coord2Cell(crdSrc);
		auto const pCellSrc = MapClass::Instance->GetCellAt(cellSrc);

		auto const delta = AbsoluteDifference(cellSrc - cellTarget);
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
	auto const pProjectileExt = BulletTypeExt::ExtMap.Find(pProjectile);

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
			if (!pOwner->IsAlliedWith_(pBld))
			{
				auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

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

DEFINE_OVERRIDE_HOOK(0x468BE2, BulletClass_ShouldDetonate_Obstacle, 6)
{
	GET(BulletClass* const, pThis, ESI);
	GET(CoordStruct* const, pOutCoords, EDI);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (AresTrajectoryHelper::SubjectToAnything(pThis->Type, pTypeExt))
	{
		auto const Map = MapClass::Instance();
		auto const pCellSource = Map->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = Map->GetCellAt(pThis->TargetCoords);
		auto const pCellLast = Map->GetCellAt(pThis->LastMapCoords);

		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->Owner;

		if (AresTrajectoryHelper::GetObstacle(
			pCellSource, pCellTarget, pThis->Owner, pThis->Target, pCellLast,
			*pOutCoords, pThis->Type, pTypeExt, pOwner))
		{
			return 0x468C76;
		}
	}

	return 0x468C86;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F7631, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
DEFINE_OVERRIDE_HOOK(0x6F7511, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
{
	GET_BASE(WeaponTypeClass* const, pWeapon, 0x10);
	GET_BASE(AbstractClass* const, pTarget, 0xC);
	GET(CoordStruct const* const, pSource, ESI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x3C, 0x1C));

	auto const pThis = (R->Origin() == 0x6F7631)
		? R->EDI<TechnoClass*>()
		: R->EAX<TechnoClass*>();

	R->EAX(AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, dest, pThis, pTarget, pWeapon, pThis->Owner));

	return 0x6F7647;
}

DEFINE_OVERRIDE_HOOK(0x4CC360, TrajectoryHelper_GetObstacle, 5)
{
	GET(CellClass* const, pCellSource, ECX);
	GET(CellClass* const, pCellTarget, EDX);
	GET_STACK(CellClass* const, pCellBullet, 0x4);
	REF_STACK(CoordStruct const, crdCur, 0x8);
	GET_STACK(BulletTypeClass* const, pType, 0x14);
	GET_STACK(HouseClass const* const, pOwner, 0x18);

	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	const auto ret = AresTrajectoryHelper::GetObstacle(
		pCellSource, pCellTarget, nullptr, nullptr, pCellBullet, crdCur, pType,
		pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC671;
}

DEFINE_OVERRIDE_HOOK(0x4CC100, TrajectoryHelper_FindFirstObstacle, 7)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(BulletTypeClass* const, pType, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	const auto ret = AresTrajectoryHelper::FindFirstObstacle(
		*pSource, *pTarget, nullptr, nullptr, pType, pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC30B;
}

DEFINE_OVERRIDE_HOOK(0x4CC310, TrajectoryHelper_FindFirstImpenetrableObstacle, 5)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(WeaponTypeClass const* const, pWeapon, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	const auto ret = AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, *pTarget, nullptr, nullptr, pWeapon, pOwner);

	R->EAX(ret);
	return 0x4CC357;
}
