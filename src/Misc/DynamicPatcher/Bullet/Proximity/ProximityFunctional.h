#pragma once

#include <CoordStruct.h>

class AbstractClass;
class BulletClass;
class TechnoClass;
struct ProximityFunctional
{

	static void Put(BulletClass* pBullet);
	static void AI(BulletClass* pBullet);

private:
	static bool ManualDetonation(BulletClass* pBullet, CoordStruct sourcePos, bool KABOOM = true, TechnoClass* pBulletOwner = nullptr, AbstractClass* pTarget = nullptr, CoordStruct detonatePos = {0,0,0});
};