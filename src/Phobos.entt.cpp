#include "Phobos.entt.h"

#include <Ext/Techno/Body.h>

struct YrEntityComponent
{
	TechnoClass* AttachedToOj;
};

void YRentt::BindToentt(TechnoClass* pObject)
{
	entt::entity entity = YRentt::gEntt->create();
	TechnoExtContainer::Instance.Find(pObject)->myEntt = entity;
	YRentt::gEntt->emplace<YrEntityComponent>(entity, pObject);
}

void YRentt::UnbindFromentt(TechnoClass* pObject)
{
	auto pExt = TechnoExtContainer::Instance.Find(pObject);

	if (pExt->myEntt != entt::null) {

		if (YRentt::gEntt) {
			YRentt::gEntt->destroy(pExt->myEntt);
		}

		pExt->myEntt = entt::null;
	}

}