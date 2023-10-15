#include "AttachStatus.h"
#include <Ext/Techno/Body.h>

void AttachStatus::OnEnable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker)
{
	Active = true;
	if (auto pTechno = generic_cast<TechnoClass*>(pObject))
	{
		if (auto ext = TechnoExtContainer::Instance.Find(pTechno))
		{
			ExtData = ext;
			ext->RecalculateStatus();
		}

	}
}

void AttachStatus::Disable(CoordStruct location)
{
	Active = false;
	if(ExtData)
		((TechnoExtData*)ExtData)->RecalculateStatus();
}