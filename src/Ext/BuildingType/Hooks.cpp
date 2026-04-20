#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Building/Body.h>
#include <Misc/BuildingFoundations.h>

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
	  || Unsorted::MAP_DEBUG_MODE.get()
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

ASMJIT_PATCH(0x461225, BuildingTypeClass_ReadFromINI_Foundation, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBP);

	INI_EX exINi(&CCINIClass::INI_Art.get());
	auto pBldext = BuildingTypeExtContainer::Instance.Find(pThis);

	if (pBldext->IsCustom)
	{
		//Reset
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pThis->FoundationData = pBldext->CustomData.data();
		pThis->FoundationOutside = pBldext->OutlineData.data();
	}

	const auto pSection = pThis->ImageFile && pThis->ImageFile[0] && strlen(pThis->ImageFile) ? pThis->ImageFile : pThis->ID;

	detail::read(pThis->Foundation, exINi, pSection, GameStrings::Foundation());

	if (auto pAdd = FindFoundation(Phobos::readBuffer))
	{
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pBldext->IsCustom = true;
		pBldext->CustomWidth = pAdd->Size.X;
		pBldext->CustomHeight = pAdd->Size.Y;

		pBldext->CustomData.assign(pAdd->CellCount + 1, CellStruct::Empty);
		pBldext->OutlineData.assign(pAdd->OutlineCount + 1, CellStruct::Empty);

		for (size_t i = 0; i < pAdd->CellCount; ++i)
		{
			pBldext->CustomData[i] = pAdd->Cells[i];
		}

		for (size_t i = 0; i < pAdd->OutlineCount; ++i)
		{
			pBldext->OutlineData[i] = pAdd->Outline[i];
		}

		pBldext->CustomData[pAdd->CellCount] = BuildingTypeExtData::FoundationEndMarker;
		pBldext->OutlineData[pAdd->OutlineCount] = BuildingTypeExtData::FoundationEndMarker;

	}
	else if (IS_SAME_STR_(Phobos::readBuffer, "Custom"))
	{

		char strbuff[0x80];

		if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
		{
			//Custom Foundation!
			pBldext->IsCustom = true;

			//Load Width and Height
			detail::read(pBldext->CustomWidth, exINi, pSection, "Foundation.X");
			detail::read(pBldext->CustomHeight, exINi, pSection, "Foundation.Y");

			int outlineLength = exINi->ReadInteger(pSection, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if (outlineLength < 10)
			{
				outlineLength = 10;
			}

			//Allocate CellStruct array
			const int dimension = pBldext->CustomWidth * pBldext->CustomHeight;

			pBldext->CustomData.assign(dimension + 1, CellStruct::Empty);
			pBldext->OutlineData.assign(outlineLength + 1, CellStruct::Empty);

			using Iter = std::vector<CellStruct>::iterator;

			auto ParsePoint = [](Iter& cell, const char* str) -> void
				{
					int x = 0, y = 0;
					switch (sscanf_s(str, "%d,%d", &x, &y))
					{
					case 0:
						x = 0;
						[[fallthrough]];
					case 1:
						y = 0;
					}
					*cell++ = CellStruct { static_cast<short>(x), static_cast<short>(y) };
				};

			//Load FoundationData
			auto itData = pBldext->CustomData.begin();
			//char key[0x20];

			for (int i = 0; i < dimension; ++i)
			{
				if (exINi->ReadString(pSection, (std::string("Foundation.") + std::to_string(i)).c_str(), Phobos::readDefval, strbuff))
				{
					ParsePoint(itData, strbuff);
				}
				else
				{
					break;
				}
			}

			//Sort, remove dupes, add end marker
			std::sort(pBldext->CustomData.begin(), itData,
			[](const CellStruct& lhs, const CellStruct& rhs)
			{
				if (lhs.Y != rhs.Y)
				{
					return lhs.Y < rhs.Y;
				}
				return lhs.X < lhs.X;
			});

			itData = std::unique(pBldext->CustomData.begin(), itData);
			*itData = BuildingTypeExtData::FoundationEndMarker;
			pBldext->CustomData.erase(itData + 1, pBldext->CustomData.end());

			auto itOutline = pBldext->OutlineData.begin();
			for (size_t i = 0; i < (size_t)outlineLength; ++i)
			{
				if (exINi->ReadString(pSection, (std::string("FoundationOutline.") + std::to_string(i)).c_str(), "", strbuff))
				{
					ParsePoint(itOutline, strbuff);
				}
				else
				{
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					*itOutline++ = BuildingTypeExtData::FoundationEndMarker;
				}
			}

			//Set end vector
			*itOutline = BuildingTypeExtData::FoundationEndMarker;
			bool hasOrigin = false;
			for (auto begin = pBldext->CustomData.begin(); begin < pBldext->CustomData.end(); begin++)
			{
				if (begin->X == 0 && begin->Y == 0)
				{
					hasOrigin = true;
					break;
				}
			}

			if (!hasOrigin)
			{
				Debug::LogInfo("BuildingType {} has a custom foundation which does not include cell 0,0. This breaks AI base building.", pSection);
			}
		}
	}

	return 0x46125D;
}

ASMJIT_PATCH(0x46152C, BuildingTypeClass_SetOccupy, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBP);

	auto pBldext = BuildingTypeExtContainer::Instance.Find(pThis);

	if (pBldext->IsCustom)
	{
		//Reset
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pThis->FoundationData = pBldext->CustomData.data();
		pThis->FoundationOutside = pBldext->OutlineData.data();

	}
	else
	{
		pThis->FoundationData = BuildingTypeClass::FoundationlinesData[(int)pThis->Foundation].Datas;
		pThis->FoundationOutside = BuildingTypeClass::FoundationOutlinesData[(int)pThis->Foundation].Datas;

		//pThis->FoundationData = FoundationDataStruct::Cells[(int)pThis->Foundation].Datas;
		//pThis->FoundationOutside = FoundationDataStruct::Outlines[(int)pThis->Foundation].Datas;

	}

	CCINIClass::INI_Art->ReadString(pThis->ImageFile, "Buildup", "", Phobos::readBuffer);
	if (strlen(Phobos::readBuffer))
	{
		PhobosCRT::strCopy(pThis->BuildupFile, Phobos::readBuffer);
	}

	return 0x4615B6;
}

