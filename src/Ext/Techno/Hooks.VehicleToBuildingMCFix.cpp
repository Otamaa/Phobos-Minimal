#include "Body.h"

#include <Phobos.h>

#include <HouseClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>

void TechnoExt::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (!pTechnoTo)
		return;

	// anim must be transfered before `Free` call , because it will get invalidated !
	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		pTechnoFrom->MindControlRingAnim = nullptr;

		// kill previous anim if any
		if (pTechnoTo->MindControlRingAnim) {
			pTechnoTo->MindControlRingAnim->TimeToDie = true;
			pTechnoTo->MindControlRingAnim->UnInit();
		}

		const auto pWhat = (VTable::Get(pTechnoTo));
		const auto pBld = pWhat == BuildingClass::vtable ?
			static_cast<BuildingClass*>(pTechnoTo) : nullptr;
		const auto pType = pTechnoTo->GetTechnoType();

		CoordStruct location = pTechnoTo->GetCoords();

		if (pBld)
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		else
			location.Z += pType->MindControlRingOffset;

		Anim->SetLocation(location);
		Anim->SetOwnerObject(pTechnoTo);

		if (pBld)
			Anim->ZAdjust = -1024;

		pTechnoTo->MindControlRingAnim = Anim;
	}

	if (const auto MCHouse = pTechnoFrom->MindControlledByHouse) {

		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}
	else
	{
		//ares perma MC 
		pTechnoTo->MindControlledByAUnit = pTechnoFrom->MindControlledByAUnit;

		if (auto Controller = pTechnoFrom->MindControlledBy)
		{
			if (auto Manager = Controller->CaptureManager)
			{
				const bool Succeeded =
					CaptureExt::FreeUnit(Manager, pTechnoFrom, true)
					&& CaptureExt::CaptureUnit(Manager, pTechnoTo, false, true, nullptr);

				if (Succeeded)
				{
					if (Is_Building(pTechnoTo))
					{
						pTechnoTo->QueueMission(Mission::Construction, 0);
						pTechnoTo->Mission_Construction();
					}
				}
			}
		}
	}

}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_TransferMindControl, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoExt::TransferMindControlOnDeploy(pStructure, pUnit);

	if (pStructure->AttachedTag)
		pUnit->AttachTrigger(pStructure->AttachedTag);

	pUnit->QueueMission(Mission::Hunt, true);

	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	return 0x449E34;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}
