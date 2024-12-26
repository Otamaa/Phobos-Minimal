
#include "Header.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Ext/WeaponType/Body.h>

#include <CaptureManagerClass.h>

#include <EBolt.h>
#include <SpawnManagerClass.h>

void WeaponTypeExtData::FireRadBeam(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target)
{
	if (auto const supportRadBeam = RadBeam::Allocate(RadBeamType::RadBeam))
	{
		const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		supportRadBeam->SetCoordsSource(source);
		supportRadBeam->SetCoordsTarget(target);

		if (pExt->Beam_IsHouseColor) {
			supportRadBeam->Color = pFirer->Owner->LaserColor;
		} else {
			supportRadBeam->Color = pExt->GetBeamColor();
		}

		supportRadBeam->Period = pExt->Beam_Duration;
		supportRadBeam->Amplitude = pExt->Beam_Amplitude;
	}
}

void WeaponTypeExtData::FireEbolt(TechnoClass* pFirer, WeaponTypeClass* pWeapon, CoordStruct& source, CoordStruct& target, int idx)
{
	if (auto const supportEBolt = WeaponTypeExtData::CreateBolt(pWeapon))
	{
		const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
		supportEBolt->Owner = pFirer;
		supportEBolt->WeaponSlot = idx;
		supportEBolt->AlternateColor = pWeapon->IsAlternateColor;
		supportEBolt->Fire(source, target, 0); //messing with 3rd arg seems to make bolts more jumpy, and parts of them disappear
	}

}

DEFINE_HOOK(0x44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	//GET(int, idxWeapon, EBP); //which weapon was chosen to attack the target with
	R->EAX(pThis->Type);

	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6 };

	auto const pMasterType = pThis->Type;
	auto const pMasterTypeData = BuildingTypeExtContainer::Instance.Find(pMasterType);

	if (pMasterTypeData->PrismForwarding.CanAttack())
	{
		auto const pMasterData = BuildingExtContainer::Instance.Find(pThis);

		if (pThis->PrismStage == PrismChargeState::Idle)
		{
			pThis->PrismStage = PrismChargeState::Master;
			pThis->DelayBeforeFiring = pThis->Type->DelayedFireDelay;

			pThis->PrismTargetCoords.X = 0;
			if (pMasterType->Weapon[1].WeaponType)
			{
				if (pThis->IsOverpowered)
				{
					pThis->PrismTargetCoords.X = 1;
				}
			}

			pThis->PrismTargetCoords.Y = pThis->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;

			int LongestChain = 0;

			//set up slaves
			int NetworkSize = 0;
			int stage = 0;

			//when it reaches zero we can't acquire any more slaves
			while (pMasterData->PrismForwarding.AcquireSlaves_MultiStage(&pMasterData->PrismForwarding, stage++, 0, NetworkSize, LongestChain) != 0) { }

			//now we have all the towers we know the longest chain, and can set all the towers' charge delays
			pMasterData->PrismForwarding.SetChargeDelay(LongestChain);

		}
		else if (pThis->PrismStage == PrismChargeState::Slave)
		{
			//a slave tower is changing into a master tower at the last second
			pThis->PrismStage = PrismChargeState::Master;
			pThis->PrismTargetCoords.X = 0;
			if (pMasterType->Weapon[1].WeaponType)
			{
				if (pThis->IsOverpowered)
				{
					pThis->PrismTargetCoords.X = 1;
				}
			}
			pThis->PrismTargetCoords.Y = pThis->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;
			pMasterData->PrismForwarding.SetSupportTarget(nullptr);
		}

		return IsCustomPrism; //always custom, the new code is a complete rewrite of the old code
	}

	return IsNotPrism;
}

DEFINE_HOOK(0x447FAE, BuildingClass_GetFireError_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	enum { BusyCharging = 0x447FB8, NotBusyCharging = 0x447FC3 };

	if (pThis->DelayBeforeFiring > 0)
	{
		//if this is a slave prism tower, then it might still be able to become a master tower at this time
		auto const pType = pThis->Type;
		auto const pTypeData = BuildingTypeExtContainer::Instance.Find(pType);
		if (pTypeData->PrismForwarding.CanAttack())
		{
			//is a prism tower
			if (pThis->PrismStage == PrismChargeState::Slave && pTypeData->PrismForwarding.BreakSupport)
			{
				return NotBusyCharging;
			}
		}
		return BusyCharging;
	}
	return NotBusyCharging;
}

//NB: PrismTargetCoords is not just a coord struct, it's a union whose first dword is the used weapon index and two others are undefined...
DEFINE_HOOK(0x4503F0, BuildingClass_Update_Prism, 9)
{
	GET(BuildingClass* const, pThis, ECX);
	auto const pType = pThis->Type;

	auto const PrismStage = pThis->PrismStage;
	if (PrismStage != PrismChargeState::Idle)
	{
		auto const pData = BuildingExtContainer::Instance.Find(pThis);
		if (pData->PrismForwarding.PrismChargeDelay <= 0)
		{
			--pThis->DelayBeforeFiring;
			if (pThis->DelayBeforeFiring <= 0)
			{
				if (PrismStage == PrismChargeState::Slave)
				{
					if (auto pTarget = pData->PrismForwarding.SupportTarget)
					{
						auto const pTargetData = BuildingExtContainer::Instance.Find(pTarget->Owner);
						auto const pTypeData = BuildingTypeExtContainer::Instance.Find(pType);
						//slave firing
						pTargetData->PrismForwarding.ModifierReserve +=
							(pTypeData->PrismForwarding.GetSupportModifier() + pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.DamageReserve +=
							(pTypeData->PrismForwarding.DamageAdd + pData->PrismForwarding.DamageReserve);
						pThis->FireLaser(pThis->PrismTargetCoords);
					}
				}
				if (PrismStage == PrismChargeState::Master)
				{
					if (auto const Target = pThis->Target)
					{
						if (pThis->GetFireError(Target, pThis->PrismTargetCoords.X, true) == FireError::OK)
						{
							if (auto const LaserBeam = pThis->Fire(Target, pThis->PrismTargetCoords.X))
							{
								auto const pTypeData = BuildingTypeExtContainer::Instance.Find(pType);

								//apparently this is divided by 256 elsewhere
								LaserBeam->DamageMultiplier = static_cast<int>((pData->PrismForwarding.ModifierReserve + 100) * 256) / 100;
								LaserBeam->Health += pTypeData->PrismForwarding.DamageAdd + pData->PrismForwarding.DamageReserve;
							}
						}
					}
				}
				//This tower's job is done. Go idle.
				pData->PrismForwarding.ModifierReserve = 0.0;
				pData->PrismForwarding.DamageReserve = 0;
				pData->PrismForwarding.RemoveAllSenders();
				pThis->SupportingPrisms = 0; //Ares sets this to the longest backward chain
				pData->PrismForwarding.SetSupportTarget(nullptr);
				pThis->PrismStage = PrismChargeState::Idle;
			}
		}
		else
		{
			//still in delayed charge so not actually charging yet
			--pData->PrismForwarding.PrismChargeDelay;
			if (pData->PrismForwarding.PrismChargeDelay <= 0)
			{
				//now it's time to start charging
				if (pType->GetBuildingAnim(BuildingAnimSlot::Special).Anim[0])
				{ //only if it actually has a special anim
					pThis->DestroyNthAnim(BuildingAnimSlot::Active);
					pThis->Game_PlayNthAnim(BuildingAnimSlot::Special ,!pThis->IsGreenHP(),pThis->GetOccupantCount() > 0 ,0);
				}

			}
		}
	}
	return 0x4504E2;
}

DEFINE_HOOK(0x44ABD0, BuildingClass_FireLaser, 5)
{
	GET(BuildingClass* const, pThis, ECX);
	REF_STACK(CoordStruct const, targetXYZ, 0x4);

	auto const pType = pThis->Type;
	auto const pTypeData = BuildingTypeExtContainer::Instance.Find(pType);

	CoordStruct sourceXYZ;
	pThis->GetFLH(&sourceXYZ ,0, CoordStruct::Empty);

	const int idxSupport = pThis->Veterancy.IsElite()  ?
		pTypeData->PrismForwarding.EliteSupportWeaponIndex : pTypeData->PrismForwarding.SupportWeaponIndex ;

	auto const supportWeapon = (idxSupport != -1)
		? pType->GetWeapon(idxSupport)->WeaponType : nullptr;

	LaserDrawClass* LaserBeam = nullptr;
	if (supportWeapon)
	{
		auto const supportWeaponData = WeaponTypeExtContainer::Instance.Find(supportWeapon);
		//IsLaser
		if (supportWeapon->IsLaser)
		{
			if (supportWeapon->IsHouseColor)
			{
				LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ,
					targetXYZ,
					pThis->Owner->LaserColor,
					ColorStruct::Empty,
					ColorStruct::Empty,
					supportWeapon->LaserDuration
				);
			}
			else
			{
				LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ,
					targetXYZ,
					supportWeapon->LaserInnerColor,
					supportWeapon->LaserOuterColor,
					supportWeapon->LaserOuterSpread,
					supportWeapon->LaserDuration
				);
			}

			if (LaserBeam)
			{
				LaserBeam->IsHouseColor = supportWeapon->IsHouseColor;
				//basic thickness (intensity additions are added later)
				if (supportWeaponData->Laser_Thickness == -1)
				{
					LaserBeam->Thickness = 3; //original default
				}
				else
				{
					LaserBeam->Thickness = supportWeaponData->Laser_Thickness;
				}
			}
		}

		//IsRadBeam
		if (supportWeapon->IsRadBeam) {
			CoordStruct target = targetXYZ;
			WeaponTypeExtData::FireRadBeam(pThis, supportWeapon, sourceXYZ, target);
		}

		//IsElectricBolt
		if (supportWeapon->IsElectricBolt)
		{
			CoordStruct target = targetXYZ;
			WeaponTypeExtData::FireEbolt(pThis, supportWeapon, sourceXYZ, target, idxSupport);
		}

		//Report
		if (supportWeapon->Report.Count > 0)
		{
			auto const ReportIndex = ScenarioClass::Instance->Random.RandomFromMax(supportWeapon->Report.Count - 1);
			auto const SoundArrayIndex = supportWeapon->Report.Items[ReportIndex];
			if (SoundArrayIndex != -1)
			{
				VocClass::PlayAt(SoundArrayIndex, sourceXYZ, nullptr);
			}
		}

		//ROF
		pThis->ReloadTimer.Start(supportWeapon->ROF);
	}
	else
	{
		//just the default support beam
		LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ,
			targetXYZ,
			pThis->Owner->LaserColor,
			ColorStruct::Empty,
			ColorStruct::Empty,
			RulesClass::Instance->PrismSupportDuration
		);

		if (LaserBeam)
		{
			LaserBeam->IsHouseColor = true;
			LaserBeam->Thickness = 3;
		}
		pThis->ReloadTimer.Start(RulesClass::Instance->PrismSupportDelay);
	}

	//Intensity adjustment for LaserBeam
	if (LaserBeam)
	{
		if (pThis->SupportingPrisms)
		{
			if (pTypeData->PrismForwarding.Intensity < 0)
			{
				LaserBeam->Thickness -= pTypeData->PrismForwarding.Intensity; //add on absolute intensity
			}
			else if (pTypeData->PrismForwarding.Intensity > 0)
			{
				LaserBeam->Thickness += (pTypeData->PrismForwarding.Intensity * pThis->SupportingPrisms);
			}

			LaserBeam->IsSupported = (LaserBeam->Thickness > 5);
		}
	}

	pThis->SupportingPrisms = 0; //not sure why Westwood set this here. We're setting it in BuildingClass_Update_Prism

	return 0x44ACE2;
}

DEFINE_HOOK(0x6FF48D, TechnoClass_Fire_IsLaser, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EDI);
	GET(WeaponTypeClass* const, pFiringWeaponType, EBX);
	GET_BASE(int, idxWeapon, 0xC);// don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	auto pType = pThis->GetTechnoType();
	if(pType->TargetLaser && pThis->Owner->ControlledByCurrentPlayer()) {

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		if (pTypeExt->TargetLaser_WeaponIdx.empty()
			|| pTypeExt->TargetLaser_WeaponIdx.Contains(idxWeapon))
		{
			pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
		}
	}

	if(pFiringWeaponType->IsLaser) {
		auto const pData = WeaponTypeExtContainer::Instance.Find(pFiringWeaponType);
		int const Thickness = pData->Laser_Thickness;

		if (auto const pBld = cast_to<BuildingClass*, false>(pThis))
		{	//ToggleLaserWeaponIndex

			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = idxWeapon;
			else
				pExt->CurrentLaserWeaponIndex.clear();

			auto const pTWeapon = pBld->GetPrimaryWeapon()->WeaponType;

			if (auto const pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty))
			{

				//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
				pLaser->Thickness = Thickness == -1 ? 3 : Thickness;
				auto const pBldTypeData = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				if (pBldTypeData->PrismForwarding.CanAttack())
				{
					//is a prism tower

					if (pBld->SupportingPrisms > 0)
					{ //Ares sets this to the longest backward chain
						//is being supported... so increase beam intensity
						if (pBldTypeData->PrismForwarding.Intensity < 0)
						{
							pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
						}
						else if (pBldTypeData->PrismForwarding.Intensity > 0)
						{
							pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
						}

						// always supporting
						pLaser->IsSupported = true;
					}
				}
			}
		}
		else
		{
			if (auto const pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, CoordStruct::Empty))
			{
				if (Thickness == -1)
				{
					pLaser->Thickness = 2;
				}
				else
				{
					pLaser->Thickness = Thickness;

					// required for larger Thickness to work right
					pLaser->IsSupported = (Thickness > 3);
				}
			}
		}

		// skip all default handling
		return 0x6FF656;
	}

	//other affects
	return 0x6FF57D;
}

//these are all for cleaning up when a prism tower becomes unavailable

DEFINE_HOOK(0x4424EF, BuildingClass_ReceiveDamage_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	BuildingExtContainer::Instance.Find(pThis)->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(0x448277, BuildingClass_ChangeOwner_PrismForwardAndLeaveBomb, 5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(HouseClass* const, newOwner, 0x58 + 0x4);
	REF_STACK(bool, announce, 0x58 + 0x8);

	enum { LeaveBomb = 0x448293 };
	pThis->NextMission();

	announce = announce && !pThis->Type->IsVehicle();

	if(announce && (pThis->Type->Powered || pThis->Type->PoweredSpecial))
		pThis->UpdatePowerDown();

	// #754 - evict Hospital/Armory contents
	TechnoExt_ExtData::KickOutHospitalArmory(pThis);

	auto pData = BuildingExtContainer::Instance.Find(pThis);
	auto const pTypeData = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	// #305: remove all jammers. will be restored with the next update.
	pData->RegisteredJammers.clear();//

	// the first and the last tower have to be allied to this
	if (pTypeData->PrismForwarding.ToAllies)
	{
		auto const FirstTarget = pData->PrismForwarding.SupportTarget;

		if (!FirstTarget)
		{
			// no first target so either this is a master tower, an idle
			// tower, or not a prism tower at all no need to remove.
			return LeaveBomb;
		}

		// the tower the new owner strives to support has to be allied, otherwise abort.
		if (newOwner->IsAlliedWith(FirstTarget->GetOwner()->Owner))
		{
			auto LastTarget = FirstTarget;
			while (LastTarget->SupportTarget)
			{
				LastTarget = LastTarget->SupportTarget;
			}

			// LastTarget is now the master (firing) tower
			if (newOwner->IsAlliedWith(LastTarget->GetOwner()->Owner))
			{
				// alliances check out so this slave tower can keep on charging.
				return LeaveBomb;
			}
		}
	}

	// if we reach this point then the alliance checks have failed. use false
	// because animation should continue / slave is busy but won't now fire
	pData->PrismForwarding.RemoveFromNetwork(false);

	return LeaveBomb;
}

DEFINE_HOOK(0x71AF76, TemporalClass_Fire_PrismForwardAndWarpable, 9)
{
	GET(TechnoClass* const, pThis, EDI);

	// bugfix #874 B: Temporal warheads affect Warpable=no units
	// it has been checked: this is warpable. free captured and destroy spawned units.
	if (pThis->SpawnManager) {
		pThis->SpawnManager->KillNodes();
	}

	if (pThis->CaptureManager) {
		pThis->CaptureManager->FreeAll();
	}

	// prism forward
	if (pThis->WhatAmI() == BuildingClass::AbsID) {
		BuildingExtContainer::Instance.Find((BuildingClass*)pThis)->PrismForwarding.RemoveFromNetwork(true);
	}

	return 0;
}

DEFINE_HOOK(0x70FD9A, TechnoClass_Drain_PrismForward, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pDrainee, EDI);
	if (pDrainee->DrainingMe != pThis)
	{ // else we're already being drained, nothing to do
		if (auto const pBld = cast_to<BuildingClass*, false>(pDrainee))
		{
			BuildingExtContainer::Instance.Find(pBld)->PrismForwarding.RemoveFromNetwork(true);
		}
	}
	return 0;
}

DEFINE_HOOK(0x454B3D, BuildingClass_UpdatePowered_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	// this building just realised it needs to go offline
	// it unregistered itself from powered unit controls but hasn't done anything else yet
	BuildingExtContainer::Instance.Find(pThis)->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(0x44EBF0, BuildingClass_Disappear_PrismForward, 5)
{
	GET(BuildingClass* const, pThis, ECX);
	BuildingExtContainer::Instance.Find(pThis)->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}
