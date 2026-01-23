#include "Body.h"

#include "NewSuperWeaponType/Dominator.h"

#include <Ext/Techno/Body.h>

#include <Utilities/Helpers.h>
#include <Misc/DamageArea.h>

// completely replace the PsyDom::Fire() method.
ASMJIT_PATCH(0x53B080, PsyDom_Fire, 5)
{
	if (SuperClass* pSuper = SW_PsychicDominator::CurrentPsyDom)
	{
		const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		HouseClass* pFirer = PsyDom::Owner;
		CellStruct cell = PsyDom::Coords;
		auto pNewData = pData->GetNewSWType();
		CellClass* pTarget = MapClass::Instance->GetCellAt(cell);
		CoordStruct coords = pTarget->GetCoords();

		// blast!
		if (pData->Dominator_Ripple)
		{
			auto pBlast = GameCreate<IonBlastClass>(coords);
			pBlast->DisableIonBeam = TRUE;
		}

		// tell!
		if (pData->SW_RadarEvent)
		{
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, cell);
		}

		// anim
		PsyDom::Anim = nullptr;
		if (AnimTypeClass* pAnimType = pData->Dominator_SecondAnim.Get(RulesClass::Instance->DominatorSecondAnim))
		{
			CoordStruct animCoords = coords;
			animCoords.Z += pData->Dominator_SecondAnimHeight;
			auto pCreated = GameCreate<AnimClass>(pAnimType, animCoords);
			pCreated->SetHouse(PsyDom::Owner);
			PsyDom::Anim = pCreated;
		}

		// kill
		auto damage = pNewData->GetDamage(pData);
		auto pWarhead = pNewData->GetWarhead(pData);

		if (pWarhead && damage != 0)
		{

			//this update every frame , so getting the firer here , seems degreading the performance ,..
			DamageArea::Apply(&coords, damage, pNewData->GetFirer(pSuper, cell, false), pWarhead, true, pFirer);
		}

		// capture
		if (pData->Dominator_Capture)
		{
			// every techno in this area shall be one with Yuri.
			auto const [widthORange, Height] = pNewData->GetRange(pData);
			Helpers::Alex::DistinctCollector<TechnoClass*> items;
			Helpers::Alex::for_each_in_rect_or_spread<TechnoClass>(cell, widthORange, Height, items);
			items.apply_function_for_each([pData, pFirer](TechnoClass* pTechno)
			{
				TechnoTypeClass* pType = GET_TECHNOTYPE(pTechno);

				// don't even try.
				if (pTechno->IsIronCurtained())
				{
					return true;
				}

				// ignore BalloonHover and inair units.
				if (pType->BalloonHover || pTechno->IsInAir())
				{
					return true;
				}

				// ignore units with no drivers
				if (TechnoExtContainer::Instance.Find(pTechno)->Is_DriverKilled)
				{
					return true;
				}

				// SW dependent stuff
				if (!pData->IsHouseAffected(pFirer, pTechno->Owner))
				{
					return true;
				}

				if (!pData->IsTechnoAffected(pTechno))
				{
					return true;
				}

				// ignore mind-controlled
				if (pTechno->MindControlledBy && !pData->Dominator_CaptureMindControlled)
				{
					return true;
				}

				// ignore permanently mind-controlled
				if (pTechno->MindControlledByAUnit && !pTechno->MindControlledBy
					&& !pData->Dominator_CapturePermaMindControlled)
				{
					return true;
				}

				// ignore ImmuneToPsionics, if wished
				if (TechnoExtData::IsPsionicsImmune(pTechno) && !pData->Dominator_CaptureImmuneToPsionics)
				{
					return true;
				}

				// free this unit
				if (pTechno->MindControlledBy)
				{
					pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
				}

				// capture this unit, maybe permanently
				pTechno->SetOwningHouse(pFirer);
				pTechno->MindControlledByAUnit = pData->Dominator_PermanentCapture;

				// remove old permanent mind control anim
				if (pTechno->MindControlRingAnim)
				{
					pTechno->MindControlRingAnim->TimeToDie = true;
					pTechno->MindControlRingAnim->UnInit();
					pTechno->MindControlRingAnim = nullptr;
				}

				// create a permanent capture anim
				if (AnimTypeClass* pAnimType = pData->Dominator_ControlAnim.Get(RulesClass::Instance->PermaControlledAnimationType))
				{
					CoordStruct animCoords = pTechno->GetCoords();
					bool Isbuilding = false;

					if (pTechno->WhatAmI() != BuildingClass::AbsID)
						animCoords.Z += pType->MindControlRingOffset;
					else
					{
						Isbuilding = true;
						animCoords.Z += ((BuildingClass*)pTechno)->Type->Height;
					}

					pTechno->MindControlRingAnim = GameCreate<AnimClass>(pAnimType, animCoords);
					pTechno->MindControlRingAnim->SetOwnerObject(pTechno);

					if (Isbuilding)
						pTechno->MindControlRingAnim->ZAdjust = -1024;
				}

				// add to the other newly captured minions.
				if (FootClass* pFoot = flag_cast_to<FootClass*, false>(pTechno))
				{
					// the AI sends all new minions to hunt
					const auto nMission = GET_TECHNOTYPE(pFoot)->ResourceGatherer ? Mission::Harvest :
						!PsyDom::Owner->IsControlledByHuman() ? Mission::Hunt : Mission::Guard;

					pFoot->QueueMission(nMission, false);
				}

				return true;
			});
		}

		// skip everything
		return 0x53B3EC;
	}

	return 0;
}
