#include "Body.h"

#include <Phobos.h>

#include <HouseClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>

ASMJIT_PATCH(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	// Remember MC ring animation.
	if (pStructure->IsMindControlled()) {
		TechnoExtContainer::Instance.Find(pStructure)->UpdateMindControlAnim();
	}

	return 0x449E34;
}

ASMJIT_PATCH(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	R->EDX<HouseClass*>(pThis->GetOriginalOwner());

	return 0x7396B3;
}
