#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x46A310, BulletClass_Shrapnel_Replace, 0x6)
{
	GET(BulletClass*, pThis, ECX);
	BulletExtData::ApplyShrapnel(pThis);
	return 0x46ADD4;
}
