#include "MissileBulletFunctional.h"

#include <Ext/BulletType/Body.h>

void MissileBulletFunctional::Put(BulletClass* pThis)
{
	if (!pThis || !pThis->Type)
		return;

	auto pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt)
		return;

	if (pThis->Type->ROT > 1)
	{
		if (pThis->WeaponType && pThis->WeaponType->Lobber)
		{
			if (pThis->Velocity.Z < 0)
			{
				pThis->Velocity.Z *= -1;
			}

			pThis->Velocity.Z += BulletTypeExtData::GetAdjustedGravity(pThis->Type);
		}

		if (pTypeExt->AnotherData.MissileData.ReverseVelocity)
		{
			BulletVelocity &velocity = pThis->Velocity;
			pThis->Velocity *= -1;

			if (!pTypeExt->AnotherData.MissileData.ReverseVelocityZ)
				pThis->Velocity.Z = velocity.Z;

		}

		if (pTypeExt->AnotherData.MissileData.ShakeVelocity != 0)
		{
			auto &nRandom = ScenarioClass::Instance->Random;
			double shakeX = nRandom.RandomDouble() * pTypeExt->AnotherData.MissileData.ShakeVelocity;
			double shakeY = nRandom.RandomDouble() * pTypeExt->AnotherData.MissileData.ShakeVelocity;
			double shakeZ = nRandom.RandomDouble();
			pThis->Velocity.X *= shakeX;
			pThis->Velocity.Y *= shakeY;
			pThis->Velocity.Z *= shakeZ;
		}
	}
}
