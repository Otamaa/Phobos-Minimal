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

bool FORCEDINLINE CanBePlacedHere(DisplayClass* pThis, BuildingTypeClass* pBld, int nHouse, CellStruct* pPlace, CellStruct* pTry)
{
	if (nHouse != HouseClass::CurrentPlayer->ArrayIndex
	  || Unsorted::ArmageddonMode
	  || !pPlace
	  || !pTry->IsValid()
	  || !pBld
	  || pBld->WhatAmI() != BuildingTypeClass::AbsID)
	{
		return true;
	}

	auto height = pBld->GetFoundationHeight(false);
	auto width = pBld->GetFoundationWidth();
	auto cellx = pTry->X;
	auto celly = pTry->Y;
	auto width_1 = width;
	auto adjplus1 = pBld->Adjacent + 1;
	auto width_2 = width_1;
	auto celly_1 = celly;
	auto xmax = cellx - adjplus1 + width_1 + 2 * adjplus1;
	auto cellyminadj = celly - adjplus1;
	auto retval = 0;
	auto xpos = cellx - adjplus1;
	auto xmax_org = xmax;
	if (cellx - adjplus1 < xmax)
	{
		auto ymax1 = (char*)height + 2 * adjplus1 + cellyminadj;
		auto ymax = ymax1;
		do
		{
			auto ypos = cellyminadj;
			if (cellyminadj < (int)ymax1)
			{
				do
				{
					CellStruct coord { (short)xpos  , (short)ypos };

					if ((short)xpos < cellx
					  || (short)xpos >= cellx + width_2
					  || (short)ypos < celly_1
					  || (short)ypos >= (int)height + celly_1)
					{

						auto cellofcoord = MapClass::Instance->GetCellAt(coord);
						if (auto base = cellofcoord->GetBuilding())
						{
							auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(base->Type);

							if (pTypeExt->NoBuildAreaOnBuildup && base->CurrentMission == Mission::Construction)
								goto continue_bld;

							auto const& pBuildingsAllowed = BuildingTypeExtContainer::Instance.Find(pBld)->Adjacent_Allowed;

							if (!pBuildingsAllowed.empty() && !pBuildingsAllowed.Contains(base->Type))
								goto continue_bld;

							auto const& pBuildingsDisallowed = BuildingTypeExtContainer::Instance.Find(pBld)->Adjacent_Disallowed;

							if (!pBuildingsDisallowed.empty() && pBuildingsDisallowed.Contains(base->Type))
								goto continue_bld;

							auto owner = base->Owner;
							if (owner->ArrayIndex == nHouse && base->Type->BaseNormal && !BuildingExtContainer::Instance.Find(base)->IsFromSW) {
								retval = 1;
							}

							if (GameModeOptionsClass::Instance->BuildOffAlly && owner->IsAlliedWith(HouseClass::Array->Items[nHouse])) {
								if (base->Type->EligibileForAllyBuilding) {
									retval = 1;
								}
							}
						}
					}
				continue_bld:
					ymax1 = ymax;
					++ypos;
				}
				while (ypos < (int)ymax);
				xmax = xmax_org;
			}
			++xpos;
		}
		while (xpos < xmax);
	}
	return retval;
}
#ifndef Original

 ASMJIT_PATCH(0x4A8EB0, DisplayClass_BuildingProximityCheck_Override, 0x5) {
 	GET(DisplayClass*, pThis, ECX);
 	GET_STACK(BuildingTypeClass*, pObj, 0x4);
 	GET_STACK(int , house, 0x8);
 	GET_STACK(CellStruct*, pList, 0xC);
 	GET_STACK(CellStruct*, pTry, 0x10);
 	R->EAX(CanBePlacedHere(pThis , pObj , house , pList , pTry));
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

ASMJIT_PATCH(0x508D8D, HouseClass_UpdatePower_Techno, 0x6)
{
	GET(HouseClass*, pThis, ESI);

	if(Phobos::Config::UnitPowerDrain){
		CountPowerOf<AircraftTypeClass>(pThis, pThis->ActiveAircraftTypes);
		CountPowerOf<InfantryTypeClass>(pThis, pThis->ActiveInfantryTypes);
		CountPowerOf<UnitTypeClass>(pThis, pThis->ActiveUnitTypes);
	}

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