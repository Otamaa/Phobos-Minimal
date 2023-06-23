#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

void PlayChronoSparkleAnim(TechnoClass* pTechno, CoordStruct* pLoc, int X_Offs = 120, int nDelay = 24 , bool bHidden = false , int ZAdjust = 0)
{
	if (bHidden || (Unsorted::CurrentFrame % nDelay))
		return;

	const auto pSparkle = RulesClass::Instance->ChronoSparkle1;
	if (!pSparkle)
		return;
	
	CoordStruct nLoc { pLoc->X + X_Offs , pLoc->Y + X_Offs  , pLoc->Z };

	if (auto pAnim = GameCreate<AnimClass>(pSparkle, nLoc, 0, 1, AnimFlag(0x600), false, false))
	{
		pAnim->ZAdjust = ZAdjust;
		HouseClass* pOwner = nullptr;
		HouseClass* pVictim = pTechno->GetOwningHouse();
		TechnoClass* pTInvoker = nullptr;
		if (const auto pInvoker = pTechno->TemporalTargetingMe) {
			if (auto pOwnerOfTemp = pInvoker->Owner) {
				pOwner = pOwnerOfTemp->GetOwningHouse();
				pTInvoker = pOwnerOfTemp;
			}
		}

		AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, pVictim, pTInvoker, false);
	}
}

DEFINE_HOOK(0x73622F, UnitClass_AI_ChronoSparkle, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	PlayChronoSparkleAnim(pThis, &pThis->Location, 120 , RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x7362A7;
}

DEFINE_HOOK(0x51BAF6, InfantryClass_AI_ChronoSparkle, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	PlayChronoSparkleAnim(pThis, &pThis->Location, 120, RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x51BB6E;

}

DEFINE_HOOK(0x414C06, AircraftClass_AI_ChronoSparkle, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	PlayChronoSparkleAnim(pThis, &pThis->Location, 0, RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x414C78;
}

DEFINE_HOOK(0x4403D4, BuildingClass_AI_ChronoSparkle, 0x6)
{
	enum { SkipGameCode = 0x44055D };

	GET(BuildingClass*, pThis, ESI);

	if (RulesClass::Instance->ChronoSparkle1)
	{
		auto const displayPositions = RulesExt::Global()->ChronoSparkleBuildingDisplayPositions;
		auto const pType = pThis->Type;
		const bool displayOnBuilding = (displayPositions & ChronoSparkleDisplayPosition::Building) != ChronoSparkleDisplayPosition::None;
		const bool displayOnSlots = (displayPositions & ChronoSparkleDisplayPosition::OccupantSlots) != ChronoSparkleDisplayPosition::None;
		const bool displayOnOccupants = (displayPositions & ChronoSparkleDisplayPosition::Occupants) != ChronoSparkleDisplayPosition::None;
		const int occupantCount = displayOnSlots ? pType->MaxNumberOccupants : pThis->GetOccupantCount();
		bool showOccupy = occupantCount && (displayOnOccupants || displayOnSlots);

		if (showOccupy)
		{
			for (int i = 0; i < occupantCount; i++)
			{
				if (!((Unsorted::CurrentFrame + i) % RulesExt::Global()->ChronoSparkleDisplayDelay))
				{
					auto offset =  TacticalClass::Instance->ApplyMatrix_Pixel(
						(pType->MaxNumberOccupants <= 10 ? 
						pType->MuzzleFlash[i] : 
						BuildingTypeExt::ExtMap.Find(pType)->OccupierMuzzleFlashes.at(i))
					);

					auto coords = pThis->GetRenderCoords();
					coords.X += offset.X;
					coords.Y += offset.Y;

					if (auto const pAnim = GameCreate<AnimClass>(RulesClass::Instance->ChronoSparkle1, coords))
					{
						pAnim->ZAdjust = -200;

						HouseClass* pOwner = nullptr;
						HouseClass* pVictim = pThis->GetOwningHouse();
						TechnoClass* pTInvoker = nullptr;
						if (const auto pInvoker = pThis->TemporalTargetingMe) {
							if (auto pOwnerOfTemp = pInvoker->Owner) {
								pOwner = pOwnerOfTemp->GetOwningHouse();
								pTInvoker = pOwnerOfTemp;
							}
						}

						AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, pVictim, pTInvoker, false);
					}
				}
			}
		}

		if ((!showOccupy || displayOnBuilding)) {
			auto nLoc = pThis->GetCenterCoords();
			PlayChronoSparkleAnim(pThis, &nLoc, 0);
		}
	}

	return SkipGameCode;
}