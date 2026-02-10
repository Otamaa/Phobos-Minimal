#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

ASMJIT_PATCH(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

//ASMJIT_PATCH(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Type->MaxNumberOccupants > 10) {
//		GET(int, nFiringIndex, EBX);
//		R->EAX(BuildingTypeExtData::GetOccupyMuzzleFlash(pThis,nFiringIndex));
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0x6) // A
{
	GET(FakeBuildingClass*, pThis, ESI);

	if (pThis->Type->MaxNumberOccupants > 10) {
		R->EDX(pThis->_GetTypeExtData()->OccupierMuzzleFlashes.data() + pThis->FiringOccupantIndex);
	}

	return 0;
}

ASMJIT_PATCH(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(FakeBuildingClass*, pThis, ESI);

	if (pThis->Type->MaxNumberOccupants > 10) {
		GET(int, nFiringIndex, EDI);
		R->ECX(pThis->_GetTypeExtData()->OccupierMuzzleFlashes.data() + nFiringIndex);
	}

	return 0;
}

ASMJIT_PATCH(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	BuildingTypeExtData::DisplayPlacementPreview();
	return 0x0;
}

ASMJIT_PATCH(0x47EFAE, CellClass_Draw_It_MakePlacementGridTranparent, 0x6)
{
	LEA_STACK(BlitterFlags*, blitFlags, STACK_OFFS(0x68, 0x58));

	const auto trans = RulesExtData::Instance()->Building_PlacementPreview ?
		RulesExtData::Instance() ->PlacementGrid_TranslucencyWithPreview .Get(RulesExtData::Instance()->PlacementGrid_TranslucentLevel) : RulesExtData::Instance()->PlacementGrid_TranslucentLevel;

	*blitFlags |= EnumFunctions::GetTranslucentLevel(trans);
	return 0;
}

bool NOINLINE Passes_Proximity_Check(
    BuildingTypeClass *object,
    int house,
    CellStruct *list,
    CellStruct *trycell,
    int distanceOverride = 0)  // default = 0 in header
{
	if (house != HouseClass::CurrentPlayer->ArrayIndex
	  || Unsorted::ArmageddonMode
	  || !list
	  || !trycell->IsValid()
	  || !object
	  || object->WhatAmI() != BuildingTypeClass::AbsID)
	{
		return true;
	}

    auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(object);

    bool skipDisallowed = false;

    if (pTypeExt->Adjacent_Disallowed_Prohibit
        && pTypeExt->Adjacent_Disallowed_ProhibitDistance > 0
        && distanceOverride == 0)
    {
        bool result = Passes_Proximity_Check(
            object, house, list, trycell,
            pTypeExt->Adjacent_Disallowed_ProhibitDistance);

        if (!result)
        {
            return false;
        }

        skipDisallowed = true;
    }

    int adj = (distanceOverride > 0) ? distanceOverride : object->Adjacent;
    	adj += 1;

   const int height = object->GetFoundationHeight(false);
   const int width = object->GetFoundationWidth();
   const int cellx = trycell->X;
   const int celly = trycell->Y;

	const int xmin = cellx - adj;
    const int xmax = cellx - adj + width + 2 * adj;
    const int ymin = celly - adj;
    const int ymax = celly - adj + height + 2 * adj;

    bool passed = false;

    for (int x = xmin; x < xmax; ++x)
    {
        for (int y = ymin; y < ymax; ++y)
        {
            CellStruct coord { (short)x  , (short)y };

            if (x >= cellx && x < cellx + width && y >= celly && y < celly + height)
            {
                continue;
            }

            CellClass* cell = MapClass::Instance->GetCellAt(coord);
            BuildingClass* building = cell->GetBuilding();

            if (building == nullptr)
            {
                continue;
            }

            auto const pCellBuildingExt =  BuildingTypeExtContainer::Instance.Find(building->Type);

            if (pCellBuildingExt->NoBuildAreaOnBuildup && building->CurrentMission == Mission::Construction)
            {
                continue;
            }

            auto const &buildingsAllowed = pTypeExt->Adjacent_Allowed;

            if (buildingsAllowed.size() > 0 && !buildingsAllowed.Contains(building->Type))
            {
                continue;
            }

            if (!skipDisallowed && building->Owner->ArrayIndex == house)
            {
                auto const &buildingsDisallowed = pTypeExt->Adjacent_Disallowed;

                if (buildingsDisallowed.size() > 0 && buildingsDisallowed.Contains(building->Type))
                {
                    if (pTypeExt->Adjacent_Disallowed_Prohibit)
                    {
                        return false;
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            HouseClass *owner = building->Owner;

            if (owner->ArrayIndex == house && building->Type->BaseNormal && !BuildingExtContainer::Instance.Find(building)->IsFromSW)
            {
                passed = true;
            }

            if (GameModeOptionsClass::Instance->BuildOffAlly
                && owner->IsAlliedWith(HouseClass::Array->Items[house])
                && building->Type->EligibileForAllyBuilding)
            {
                passed = true;
            }
        }
    }

    return passed;
}


#ifndef Original

 ASMJIT_PATCH(0x4A8EB0, DisplayClass_BuildingProximityCheck_Override, 0x5) {
 	//GET(DisplayClass*, pThis, ECX);
 	GET_STACK(BuildingTypeClass*, pObj, 0x4);
 	GET_STACK(int , house, 0x8);
 	GET_STACK(CellStruct*, pList, 0xC);
 	GET_STACK(CellStruct*, pTry, 0x10);
 	R->EAX(Passes_Proximity_Check(pObj , house , pList , pTry));
 	return 0x4A9063;
 }

template<typename Arr>
void CountPowerOf(HouseClass*pHouse ,CounterClass& counter){
	
	 if(counter.IsAllocated) {
		 for (int a = 0; a < counter.Capacity; ++a)  {
			 if (counter.Items[a] > 0) {
				 const auto pExt = TechnoTypeExtContainer::Instance.Find(Arr::Array->Items[a]);

				if(pExt->Power.isset()) {

				 if (pExt->Power > 0)
					 pHouse->PowerOutput += pExt->Power * counter.Items[a];
				 else
					 pHouse->PowerDrain -= pExt->Power * counter.Items[a];

				}
			 }
		 }
	 }
}

 // Trigger power recalculation on gain / loss of any techno, not just buildings.

ASMJIT_PATCH(0x502A80, HouseClass_RegisterGain, 0x8)
{
	 GET(HouseClass*, pThis, ECX);

	 pThis->RecheckPower = true;

	 return 0;
 }ASMJIT_PATCH_AGAIN(0x5025F0, HouseClass_RegisterGain, 0x5) // RegisterLoss

 void CalculatePowerSurplus(HouseClass* pThis)
{
	auto const pRulesExt = RulesExtData::Instance();

	if (!pRulesExt->EnablePowerSurplus)
		return;

	int scaleToDrainAmount = pRulesExt->PowerSurplus_ScaleToDrainAmount;

	if (scaleToDrainAmount <= 0)
	{
		pThis->PowerSurplus = RulesClass::Instance->PowerSurplus;
	}
	else
	{
		double factor = pThis->PowerDrain / static_cast<double>(scaleToDrainAmount);
		pThis->PowerSurplus = static_cast<int>(RulesClass::Instance->PowerSurplus * factor);
	}
}

ASMJIT_PATCH(0x508D8D, HouseClass_UpdatePower_Techno, 0x6)
{
	GET(HouseClass*, pThis, ESI);

	if(Phobos::Config::UnitPowerDrain){
		CountPowerOf<AircraftTypeClass>(pThis, pThis->ActiveAircraftTypes);
		CountPowerOf<InfantryTypeClass>(pThis, pThis->ActiveInfantryTypes);
		CountPowerOf<UnitTypeClass>(pThis, pThis->ActiveUnitTypes);
	}


	CalculatePowerSurplus(pThis);

	return 0x0;
}
#else
ASMJIT_PATCH(0x4A8FF5, MapClass_CanBuildingTypeBePlacedHere_Ignore, 5)
{
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->IsFromSW ? 0x4A8FFA : 0x0;
}

//ASMJIT_PATCH(0x4A8FD7, DisplayClass_BuildingProximityCheck_BuildArea, 0x6)
//{
//	enum { SkipBuilding = 0x4A902C , Continue = 0x0 };
//
//	GET(BuildingClass*, pCellBuilding, ESI);
//	GET_STACK(BuildingTypeClass*, pType, 0x4 + 0x28);
//
//	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pCellBuilding->Type);
//
//	if (pTypeExt->NoBuildAreaOnBuildup && pCellBuilding->CurrentMission == Mission::Construction)
//		return SkipBuilding;
//
//	auto const& pBuildingsAllowed = BuildingTypeExtContainer::Instance.Find(pType)->Adjacent_Allowed;
//
//	if (!pBuildingsAllowed.empty() && !pBuildingsAllowed.Contains(pCellBuilding->Type))
//		return SkipBuilding;
//
//	auto const& pBuildingsDisallowed = BuildingTypeExtContainer::Instance.Find(pType)->Adjacent_Disallowed;
//
//	if (!pBuildingsDisallowed.empty() && pBuildingsDisallowed.Contains(pCellBuilding->Type))
//		return SkipBuilding;
//
//	return Continue;
//}
#endif