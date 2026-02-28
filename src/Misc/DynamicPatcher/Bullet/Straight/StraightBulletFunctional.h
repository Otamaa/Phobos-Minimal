#pragma once

#include <BulletClass.h>

struct StraightBulletFunctional
{

	static void Put(BulletClass* pBullet);
	static void AI(BulletClass* pBullet);
	static VelocityClass RecalculateBulletVelocity(BulletClass* pBullet,CoordStruct sourcePos, CoordStruct targetPos);
};