#include "Base.h"
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>

void EffectsBase::SetAEManager(ObjectClass* pOwner)
{
	if (auto pTechno = generic_cast<TechnoClass*>(pOwner))
	{
		OwnerAEM = TechnoExt::ExtMap.Find(pTechno)->AnotherData.MyManager.get();
	}
	else if (auto pBullet = specific_cast<BulletClass*>(pOwner))
	{
		OwnerAEM = BulletExt::ExtMap.Find(pBullet)->AnotherData.MyManager.get();
	}
}
