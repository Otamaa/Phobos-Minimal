#include "StraightBulletFunctional.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/Otamaa/Misc/DynamicPatcher/Helpers/Helpers.h>
#include "StraightBullet.h"
#include "../Proximity/Proximity.h"

void StraightBulletFunctional::Put(BulletClass* pBullet)
{
	if (!pBullet || !pBullet->Type)
		return;

	auto pBulletExt = BulletExt::GetExtData(pBullet);
	auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (!pBulletExt || !pBulletTypeExt)
		return;

	if (pBulletTypeExt->AnotherData.StraightBulletDatas.Enabled ?
		pBullet->Type->ROT == 1 : pBulletTypeExt->AnotherData.StraightBulletDatas.IsStraight())
	{
		CoordStruct sourcePos = pBullet->SourceCoords;
		CoordStruct targetPos = pBullet->TargetCoords;

		if (pBulletTypeExt->AnotherData.StraightBulletDatas.AbsolutelyStraight)
		{
			double distance = targetPos.DistanceFrom(sourcePos);
			DirStruct facing = pBullet->Owner->GetRealFacing().current();
			auto ntargetTestPosBuff = CoordStruct { (int)distance, 0, 0 };
			targetPos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, ntargetTestPosBuff, facing);
			pBullet->TargetCoords = targetPos;
		}

		BulletVelocity velocity = RecalculateBulletVelocity(pBullet, sourcePos, targetPos);
		pBulletExt->AnotherData.StraightBulletD.reset(GameCreate<StraightBullet>(true, sourcePos, targetPos, velocity));
	}

	if (pBullet->Type->Proximity)
	{
		// this.Proximity = new Proximity(pBullet.Ref.Owner, pBullet.Ref.Type.Ref.CourseLockDuration);
		pBulletExt->AnotherData.BulletProximity.reset(
			GameCreate<Proximity>(pBulletTypeExt->AnotherData.BulletProximityData, pBullet->Owner, pBullet->Type->CourseLockDuration));

	}
}

void StraightBulletFunctional::AI(BulletClass* pBullet)
{
	if (!pBullet)
		return;

	auto pBulletExt = BulletExt::GetExtData(pBullet);

	if (!pBulletExt)
		return;

	if (auto const pStraight = pBulletExt->AnotherData.StraightBulletD.get())
	{
		if (pStraight->Enable)
		{
			pBullet->Velocity = pStraight->Velocity;
		}
	}
}

BulletVelocity StraightBulletFunctional::RecalculateBulletVelocity(BulletClass* pBullet, CoordStruct sourcePos, CoordStruct targetPos)
{
	BulletVelocity velocity = BulletVelocity{ (double)(targetPos.X - sourcePos.X), (double)(targetPos.Y - sourcePos.Y), (double)(targetPos.Z - sourcePos.Z) };
	velocity *= pBullet->Speed / targetPos.DistanceFrom(sourcePos);
	pBullet->Velocity = velocity;
	return velocity;
}