#include "Header.h"

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>

ASMJIT_PATCH(0x732d47, TacticalClass_CollectSelectedIDs, 5)
{
	auto pNames = R->EBX<DynamicVectorClass<const char*>*>();

	auto Add = [pNames](TechnoTypeClass* pType)
		{
			const char* id = TechnoTypeExtContainer::Instance.Find(pType)->GetSelectionGroupID();

			if (pNames->none_of([id](const char* pID)
				{
					return !CRT::strcmpi(pID, id);
				}))
			{
				pNames->push_back(id);
			}
		};

	bool useDeploy = RulesExtData::Instance()->TypeSelectUseDeploy;

	for (auto pObject : ObjectClass::CurrentObjects())
	{
		// add this object's id used for grouping
		if (TechnoTypeClass* pType = pObject->GetTechnoType())
		{
			Add(pType);

			// optionally do the same the original game does, but support the new grouping feature.
			if (useDeploy)
			{
				if (pType->DeploysInto)
				{
					Add(pType->DeploysInto);
				}
				if (pType->UndeploysInto)
				{
					Add(pType->UndeploysInto);
				}
			}
		}
	}

	R->EAX(pNames);
	return 0x732FD9;
}


ASMJIT_PATCH(0x4ABD6C, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}ASMJIT_PATCH_AGAIN(0x4ABD9D, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
ASMJIT_PATCH_AGAIN(0x4ABE58, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
