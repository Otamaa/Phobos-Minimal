#include "Base.h"
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>

void EffectsBase::SetAEManager(ObjectClass* pOwner)
{
	if (auto pTechno = generic_cast<TechnoClass*>(pOwner))
	{
		OwnerAEM = TechnoExtContainer::Instance.Find(pTechno)->AnotherData.MyManager.get();
	}
	else if (auto pBullet = specific_cast<BulletClass*>(pOwner))
	{
		OwnerAEM = BulletExtContainer::Instance.Find(pBullet)->AnotherData.MyManager.get();
	}
}
