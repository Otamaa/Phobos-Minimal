#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/BuildingType/Body.h>

#include "AresTrajectoryHelper.h"

ASMJIT_PATCH(0x468BE2, BulletClass_ShouldDetonate_Obstacle, 6)
{
	GET(BulletClass* const, pThis, ESI);
	GET(CoordStruct* const, pOutCoords, EDI);

	if (AresBulletObstacleHelper::SubjectToAnything(pThis->Type))
	{
		auto const Map = MapClass::Instance();
		auto const pCellSource = Map->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = Map->GetCellAt(pThis->TargetCoords);
		auto const pCellLast = Map->GetCellAt(pThis->LastMapCoords);

		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;

		if (AresBulletObstacleHelper::GetObstacle(
			pCellSource,
			pCellTarget,
			pThis->Owner,
			pThis->Target,
			pCellLast,
			*pOutCoords,
			pThis->Type, pOwner))
		{
			return 0x468C76;
		}
	}

	if (PhobosBulletObstacleHelper::SubjectToObstacles(pThis->Type))
	{
		auto const pCellSource = MapClass::Instance->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = MapClass::Instance->GetCellAt(pThis->TargetCoords);
		auto const pCellCurrent = MapClass::Instance->GetCellAt(pThis->LastMapCoords);
		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
		if (PhobosBulletObstacleHelper::GetObstacle(pCellSource, pCellTarget, pCellCurrent, pThis->Location, pThis->Owner, pThis->Target, pOwner, pThis->Type, false))
			return 0x468C9F;
	}

	return 0x468C86;
}

ASMJIT_PATCH(0x6F7511, TechnoClass_InRange_Obstacle, 6)
{
	GET_BASE(WeaponTypeClass* const, pWeapon, 0x10);
	GET_BASE(AbstractClass* const, pTarget, 0xC);
	GET(CoordStruct const* const, pSource, ESI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x3C, 0x1C));

	auto const pThis = (R->Origin() == 0x6F7631)
		? R->EDI<TechnoClass*>()
		: R->EAX<TechnoClass*>();

	auto pResult = AresBulletObstacleHelper::FindFirstImpenetrableObstacle(
		*pSource, dest, pThis, pTarget, pWeapon, pThis->Owner);
	if(!pResult){
		auto subjectToGround = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->SubjectToGround.Get();
		const auto newSourceCoords = subjectToGround ? PhobosBulletObstacleHelper::AddFLHToSourceCoords(*pSource, dest, pThis, pTarget, pWeapon, subjectToGround) : *pSource;
		pResult = PhobosBulletObstacleHelper::FindFirstImpenetrableObstacle(newSourceCoords, dest, pThis, pTarget, pThis->Owner, pWeapon, true, subjectToGround);
	}
	R->EAX(pResult);

	return 0x6F7647;
}ASMJIT_PATCH_AGAIN(0x6F7631, TechnoClass_InRange_Obstacle, 6)


ASMJIT_PATCH(0x4CC360, TrajectoryHelper_GetObstacle, 5)
{
	GET(CellClass* const, pCellSource, ECX);
	GET(CellClass* const, pCellTarget, EDX);
	GET_STACK(CellClass* const, pCellBullet, 0x4);
	REF_STACK(CoordStruct const, crdCur, 0x8);
	GET_STACK(BulletTypeClass*, pType, 0x14);
	GET_STACK(HouseClass*, pOwner, 0x18);

	auto const ret = AresBulletObstacleHelper::GetObstacle(
		pCellSource, pCellTarget, nullptr, nullptr, pCellBullet, crdCur, pType,
		pOwner);

	R->EAX(ret);
	return 0x4CC671;
}

ASMJIT_PATCH(0x4CC100, TrajectoryHelper_FindFirstObstacle, 7)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(BulletTypeClass*, pType, 0x4);
	GET_STACK(HouseClass*, pOwner, 0x8);

	auto const ret = AresBulletObstacleHelper::FindFirstObstacle(
		*pSource, *pTarget, nullptr, nullptr, pType, pOwner);

	R->EAX(ret);
	return 0x4CC30B;
}

ASMJIT_PATCH(0x4CC310, TrajectoryHelper_FindFirstImpenetrableObstacle, 5)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(WeaponTypeClass*, pWeapon, 0x4);
	GET_STACK(HouseClass*, pOwner, 0x8);

	auto const ret = AresBulletObstacleHelper::FindFirstImpenetrableObstacle(
		*pSource, *pTarget, nullptr, nullptr, pWeapon, pOwner);

	R->EAX(ret);
	return 0x4CC357;
}
