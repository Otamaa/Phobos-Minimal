#include "../Bullet/Proximity/ProximityFunctional.h"
#include "../Bullet/MissileBullet/MissileBulletFunctional.h"
#include "../Bullet/Straight/StraightBulletFunctional.h"

#include  <Ext/Bullet/Body.h>

ASMJIT_PATCH(0x468B5D, BulletClass_Unlimbo_DP, 0x6)
{
	GET(BulletClass* , pThis , EBX);
	//GET_STACK(CoordStruct*, pCoord, 0x20);

	StraightBulletFunctional::Put(pThis);
	ProximityFunctional::Put(pThis);
	MissileBulletFunctional::Put(pThis);

	return 0x0;
}