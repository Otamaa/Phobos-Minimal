#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/BuildingType/Body.h>

#include "AresTrajectoryHelper.h"

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
