#include "Body.h"

#include <Phobos.h>

#include <HouseClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>

void TechnoExtData::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (!pTechnoTo || TechnoExtData::IsPsionicsImmune(pTechnoTo))
		return;

	const auto pBld = specific_cast<BuildingClass*>(pTechnoTo);

	// anim must be transfered before `Free` call , because it will get invalidated !
	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		pTechnoFrom->MindControlRingAnim = nullptr;

		// kill previous anim if any
		if (pTechnoTo->MindControlRingAnim) {
			GameDelete<true,false>(pTechnoTo->MindControlRingAnim);
			//pTechnoTo->MindControlRingAnim->TimeToDie = true;
			//pTechnoTo->MindControlRingAnim->UnInit();
		}

		CoordStruct location = pTechnoTo->GetCoords();

		if (pBld)
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		else
			location.Z += pTechnoTo->GetTechnoType()->MindControlRingOffset;

		Anim->SetLocation(location);
		Anim->SetOwnerObject(pTechnoTo);

		if (pBld)
			Anim->ZAdjust = -1024;

		pTechnoTo->MindControlRingAnim = Anim;
	}

	if (const auto MCHouse = pTechnoFrom->MindControlledByHouse) {
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	} else if(pTechnoTo->MindControlledByAUnit && !pTechnoFrom->MindControlledBy) {
			pTechnoTo->MindControlledByAUnit = pTechnoFrom->MindControlledByAUnit; //perma MC ed
	} else if (auto Controller = pTechnoFrom->MindControlledBy) {
		if (auto Manager = Controller->CaptureManager)
		{
			const bool Succeeded =
				CaptureExt::FreeUnit(Manager, pTechnoFrom, true)
				&& CaptureExt::CaptureUnit(Manager, pTechnoTo, false, true, nullptr);

			if (Succeeded)
			{
				if (pBld)
				{
					// Capturing the building after unlimbo before buildup has finished or even started appears to throw certain things off,
					// Hopefully this is enough to fix most of it like anims playing prematurely etc.
					pBld->ActuallyPlacedOnMap = false;
					pBld->DestroyNthAnim(BuildingAnimSlot::All);
					pTechnoTo->QueueMission(Mission::Construction, 0);
					pTechnoTo->Mission_Construction();
				}
			}
		}
	}
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	// Remember MC ring animation.
	if (pStructure->IsMindControlled()) {
		TechnoExtContainer::Instance.Find(pStructure)->UpdateMindControlAnim();
	}

	return 0x449E34;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	R->EDX<HouseClass*>(pThis->GetOriginalOwner());

	return 0x7396B3;
}
