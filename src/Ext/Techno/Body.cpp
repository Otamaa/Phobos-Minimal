#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>
#include <SpotlightClass.h>

#include <BitFont.h>

#include <New/Entity/FlyingStrings.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include <Commands/ShowTeamLeader.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>

#include <Locomotor/Cast.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Enum.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>

#include <Locomotor/Cast.h>


#include <memory>
#include <TerrainTypeClass.h>

#pragma region defines
UnitClass* TechnoExtData::Deployer { nullptr };
#pragma endregion

#include <Misc/PhobosGlobal.h>

#include <Drawing.h>
#include <TextDrawing.h>

void TintColors::Calculate(const int color, const int intensity, const AffectedHouse affectedHouse)
{
	if ((affectedHouse & AffectedHouse::Owner) != AffectedHouse::None)
	{
		this->ColorOwner |= color;
		this->IntensityOwner += intensity;
	}

	if ((affectedHouse & AffectedHouse::Allies) != AffectedHouse::None)
	{
		this->ColorAllies |= color;
		this->IntensityAllies += intensity;
	}

	if ((affectedHouse & AffectedHouse::Enemies) != AffectedHouse::None)
	{
		this->ColorEnemies |= color;
		this->IntensityEnemies += intensity;
	}
}

void TintColors::Update()
{
	// reset values
	//this->Reset();

	//if (!this->Owner->IsAlive)
	//	return;

	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Owner->GetTechnoType());
	//auto pOwnerExt = TechnoExtContainer::Instance.Find(this->Owner);
	//const bool hasTechnoTint = pTypeExt->Tint_Color.isset() || pTypeExt->Tint_Intensity;
	//const bool hasShieldTint = pOwnerExt->Shield && pOwnerExt->Shield->IsActive() && pOwnerExt->Shield->GetType()->HasTint();

	//// bail out early if no custom tint is applied.
	//if (!hasTechnoTint && !pOwnerExt->AE.HasTint && !hasShieldTint)
	//	return;

	//if (hasTechnoTint)
	//	this->Calculate(Drawing::RGB_To_Int(pTypeExt->Tint_Color), static_cast<int>(pTypeExt->Tint_Intensity * 1000), pTypeExt->Tint_VisibleToHouses);

	//if (pOwnerExt->AE.HasTint)
	//{
	//	for (auto const& attachEffect : pOwnerExt->PhobosAE)
	//	{
	//		if (!attachEffect)
	//			continue;

	//		auto const type = attachEffect->GetType();

	//		if (!attachEffect->IsActive() || !type->HasTint())
	//			continue;

	//		this->Calculate(Drawing::RGB_To_Int(type->Tint_Color), static_cast<int>(type->Tint_Intensity * 1000), type->Tint_VisibleToHouses);
	//	}
	//}

	//if (hasShieldTint)
	//{
	//	auto const pShieldType = pOwnerExt->Shield->GetType();
	//	this->Calculate(Drawing::RGB_To_Int(pShieldType->Tint_Color), static_cast<int>(pShieldType->Tint_Intensity * 1000), pShieldType->Tint_VisibleToHouses);
	//}
}

void TintColors::GetTints(int* tintColor, int* intensity)
{
	const bool CalculateIntensity = intensity != nullptr;
	const bool calculateTint = tintColor != nullptr;

	if (!calculateTint && !CalculateIntensity)
		return;

	auto const pOwner = this->Owner->Owner;
	auto pOwnerExt = TechnoExtContainer::Instance.Find(this->Owner);

	//for (auto& [wh, paint] : pOwnerExt->PaintBallStates) {
	//	if (paint.IsActive() && paint.AllowDraw(this->Owner)) {
	//
	//		if (calculateTint && paint.Color)
	//			*tintColor |= paint.Color;
	//
	//		if (CalculateIntensity)
	//			*intensity += int(paint.Data->BrightMultiplier * 1000);
	//	}
	//}

	auto const pTypeExt = GET_TECHNOTYPEEXT(this->Owner);
	const bool hasTechnoTint = pTypeExt->Tint_Color.isset() || pTypeExt->Tint_Intensity;
	bool hasShieldTint = false;
	auto pShield = pOwnerExt->GetShield();

	if (pShield) {
		hasShieldTint = pShield->IsActive() && pShield->GetType()->HasTint();
	}

	this->Reset();

	// bail out early if no custom tint is applied.
	if (!hasTechnoTint && !pOwnerExt->AE.flags.HasTint && !hasShieldTint)
		return;

	if (hasTechnoTint)
		this->Calculate(pTypeExt->Tint_Color->ToInit(), static_cast<int>(pTypeExt->Tint_Intensity * 1000), pTypeExt->Tint_VisibleToHouses);

	if (pOwnerExt->AE.flags.HasTint)
	{
		for (auto const& attachEffect : pOwnerExt->PhobosAE)
		{
			if (!attachEffect)
				continue;

			auto const type = attachEffect->GetType();

			if (!attachEffect->IsActive() || !type->HasTint())
				continue;

			this->Calculate(type->Tint_Color->ToInit(), static_cast<int>(type->Tint_Intensity * 1000), type->Tint_VisibleToHouses);
		}
	}

	if (hasShieldTint)
	{
		auto const pShieldType = pShield->GetType();
		this->Calculate(pShieldType->Tint_Color->ToInit(), static_cast<int>(pShieldType->Tint_Intensity * 1000), pShieldType->Tint_VisibleToHouses);
	}

	if (pOwner == HouseClass::CurrentPlayer)
	{
		if(calculateTint)
			*tintColor |= this->ColorOwner;

		if (CalculateIntensity)
			*intensity += this->IntensityOwner;
	}
	else if (pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
	{
		if (calculateTint)
			*tintColor |= this->ColorAllies;

		if (CalculateIntensity)
			*intensity += this->IntensityAllies;
	}
	else
	{
		if (calculateTint)
			*tintColor |= this->ColorEnemies;

		if (CalculateIntensity)
			*intensity += this->IntensityEnemies;
	}
}

void TechnoExtData::ShakeScreen(TechnoClass* pThis, int nValToCalc, int nRules) {
	if (pThis->IsOnMyView())
	{
		auto nFirst = GeneralUtils::GetValue(nValToCalc);
		auto nSec = nFirst - GeneralUtils::GetValue(nRules) + 4;
		GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, nSec >> 1);
		GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, nSec);
	}
}

void TechnoExtData::AddAirstrikeFactor(TechnoClass*& pKiller, double& d_factor)
{
	// before we do any other logic, check if this kill was committed by an
	// air strike and its designator shall get the experience.
	if (pKiller->Airstrike)
	{
		if (const auto pDesignator = pKiller->Airstrike->Owner)
		{
			const auto pDesignatorExt = GET_TECHNOTYPEEXT(pDesignator);

			if (pDesignatorExt->ExperienceFromAirstrike)
			{
				pKiller = pDesignator;
				d_factor *= pDesignatorExt->AirstrikeExperienceModifier;
			}
		}
	}
}

void TechnoExtData::UpdateLastTargetCrd()
{
	if (!this->TypeExtData->ExtraThreat_Enabled)
		return;

	auto const pThis = this->This();
	auto pTimer = &this->LastTargetCrdClearTimer;

	if (pThis->Target)
	{
		this->LastTargetCrd = pThis->Target->GetCoords();
		pTimer->Stop();
	}
	else
	{
		if (!pTimer->IsTicking())
			pTimer->Start(45);

		if (pTimer->Completed())
		{
			this->LastTargetCrd = CoordStruct::Empty;
			pTimer->Stop();
		}
	}
}

bool TechnoExtData::KillerInTransporterFactor(TechnoClass* pKiller, TechnoClass*& pExpReceiver, double& d_factor, bool& promoteImmediately)
{
	const auto pTransporter = pKiller->Transporter;
	if (!pTransporter)
		return false;

	const auto pTTransporterData = GET_TECHNOTYPEEXT(pTransporter);
	const auto TransporterAndKillerAllied = pTransporter->Owner->IsAlliedWith(pKiller);

	if (pKiller->InOpenToppedTransport)
	{
		// check for passenger of an open topped vehicle. transporter can get
		// experience from passengers; but only if the killer and its transporter
		// are allied. so a captured opentopped vehicle won't get experience from
		// the enemy's orders.

		// if passengers can get promoted and this transport is already elite,
		// don't promote this transport in favor of the real killer.
		const TechnoTypeClass* pTTransporter = GET_TECHNOTYPE(pTransporter);

		if ((!pTTransporter->Trainable || pTTransporterData->PassengersGainExperience) && (pTransporter->Veterancy.IsElite() || !TransporterAndKillerAllied)
				&& GET_TECHNOTYPE(pKiller)->Trainable)
		{
			// the passenger gets experience
			pExpReceiver = pKiller;
			d_factor *= pTTransporterData->PassengerExperienceModifier;
		}
		else if (pTTransporter->Trainable && pTTransporterData->ExperienceFromPassengers && TransporterAndKillerAllied)
		{
			// the transporter gets experience
			pExpReceiver = pTransporter;
		}

		return true;
	}

	return false;
}

void TechnoExtData::AddExperience(TechnoClass* pExtReceiver, TechnoClass* pVictim, int victimCost, double factor)
{
	const auto pExpReceiverType = GET_TECHNOTYPE(pExtReceiver);
	const auto pVinctimType = GET_TECHNOTYPE(pVictim);
	const auto TechnoCost = pExpReceiverType->GetActualCost(pExtReceiver->Owner);
	const auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(pVinctimType);
	const auto pKillerTypeExt = TechnoTypeExtContainer::Instance.Find(pExpReceiverType);

	const auto WeightedVictimCost = static_cast<int>(victimCost * factor *
		pKillerTypeExt->Experience_KillerMultiple * pVictimTypeExt->Experience_VictimMultiple);

	if (TechnoCost > 0 && WeightedVictimCost > 0)
	{
		pExtReceiver->Veterancy.Add(TechnoCost, WeightedVictimCost);
	}
}

void TechnoExtData::MCControllerGainExperince(TechnoClass* pExpReceiver, TechnoClass* pVictim, double& d_factor, int victimCost)
{
	// mind-controllers get experience, too.
	if (auto pController = pExpReceiver->MindControlledBy)
	{
		if (!pController->Owner->IsAlliedWith(pVictim->Owner))
		{

			// get the mind controllers extended properties
			const auto pTController = GET_TECHNOTYPE(pController);
			const auto pTControllerData = TechnoTypeExtContainer::Instance.Find(pTController);

			// promote the mind-controller
			if (pTController->Trainable)
			{
				// the mind controller gets its own factor
				AddExperience(pController, pVictim, victimCost, d_factor * pTControllerData->MindControlExperienceSelfModifier);
			}

			// modify the cost of the victim.
			d_factor *= pTControllerData->MindControlExperienceVictimModifier;
		}
	}
}

void TechnoExtData::GetSpawnerData(TechnoClass*& pSpawnOut, TechnoClass*& pExpReceiver, double& d_spawnFacor, double& d_ExpFactor)
{
	if (const auto pSpawner = pExpReceiver->SpawnOwner)
	{
		const auto pTSpawner = GET_TECHNOTYPE(pSpawner);
		if (!pTSpawner->MissileSpawn && pTSpawner->Trainable)
		{
			const auto pTSpawnerData = TechnoTypeExtContainer::Instance.Find(pTSpawner);

			// add experience to the spawn. this is done later so mind-control
			// can be factored in.
			d_spawnFacor = pTSpawnerData->SpawnExperienceSpawnModifier;
			pSpawnOut = pExpReceiver;

			// switch over to spawn owners, and factor in the spawner multiplier
			d_ExpFactor *= pTSpawnerData->SpawnExperienceOwnerModifier;
			pExpReceiver = pSpawner;
		}
	}
}

void TechnoExtData::PromoteImmedietely(TechnoClass* pExpReceiver, bool bSilent, bool Flash)
{
	auto newRank = pExpReceiver->Veterancy.GetRemainingLevel();

	if (pExpReceiver->CurrentRanking != newRank)
	{
		if (pExpReceiver->CurrentRanking != Rank::Invalid)
		{
			auto const pTypeExt = GET_TECHNOTYPEEXT(pExpReceiver);

			if (pTypeExt->Promote_IncludePassengers)
			{
				auto const& nCur = pExpReceiver->Veterancy;

				for (NextObject object(pExpReceiver->Passengers.GetFirstPassenger()); object; ++object)
				{
					if (auto const pFoot = flag_cast_to<FootClass*>(*object))
					{
						if (!GET_TECHNOTYPE(pFoot)->Trainable)
							continue;

						pFoot->Veterancy = nCur;// sync veterancy with the Transporter
						PromoteImmedietely(pFoot, true, false);
					}
				}
			}

			int sound = -1;
			int eva = -1;
			int flash = 0;
			TechnoTypeClass* pNewType = nullptr;
			double promoteExp = 0.0;
			auto const pRules = RulesClass::Instance.get();
			AnimTypeClass* Promoted_PlayAnim = nullptr;
			bool playSpotlight = false;

			if (newRank == Rank::Veteran)
			{
				flash = pTypeExt->Promote_Vet_Flash.Get(RulesExtData::Instance()->VeteranFlashTimer);
				sound = pTypeExt->Promote_Vet_Sound.Get(pRules->UpgradeVeteranSound);
				eva = pTypeExt->Promote_Vet_Eva;
				pNewType = pTypeExt->Promote_Vet_Type;
				promoteExp = pTypeExt->Promote_Vet_Exp;
				Promoted_PlayAnim = pTypeExt->Promote_Vet_Anim.Get(RulesExtData::Instance()->Promote_Vet_Anim);
				playSpotlight = pTypeExt->Promote_Vet_PlaySpotlight.Get(RulesExtData::Instance()->Promote_Vet_PlaySpotlight);
			}
			else if (newRank == Rank::Elite)
			{
				flash = pTypeExt->Promote_Elite_Flash.Get(pRules->EliteFlashTimer);
				sound = pTypeExt->Promote_Elite_Sound.Get(pRules->UpgradeEliteSound);
				eva = pTypeExt->Promote_Elite_Eva;
				pNewType = pTypeExt->Promote_Elite_Type;
				promoteExp = pTypeExt->Promote_Elite_Exp;
				Promoted_PlayAnim = pTypeExt->Promote_Elite_Anim.Get(RulesExtData::Instance()->Promote_Elite_Anim);
				playSpotlight = pTypeExt->Promote_Vet_PlaySpotlight.Get(RulesExtData::Instance()->Promote_Vet_PlaySpotlight);
			}

			if (pNewType && TechnoExtData::ConvertToType(pExpReceiver, pNewType) && promoteExp != 0.0)
			{
				newRank = pExpReceiver->Veterancy.AddAndGetRank(promoteExp);
			}

			if (!bSilent && pExpReceiver->Owner->ControlledByCurrentPlayer())
			{
				const CoordStruct loc_ = (pExpReceiver->Transporter ? pExpReceiver->Transporter : pExpReceiver)->Location;

				if (loc_.IsValid())
					VocClass::SafeImmedietelyPlayAt(sound, &loc_, nullptr);

				VoxClass::PlayIndex(eva);
			}

			if (Flash && flash > 0)
			{
				pExpReceiver->Flashing.DurationRemaining = flash;
			}

			if (Promoted_PlayAnim && !pExpReceiver->InLimbo)
			{
				auto pAnim = GameCreate<AnimClass>(Promoted_PlayAnim, pExpReceiver->Location, 0, 1, 0x600u, 0, 0);
				pAnim->SetOwnerObject(pExpReceiver);

				if (pExpReceiver->WhatAmI() == BuildingClass::AbsID)
					pAnim->ZAdjust = -1024;
			}

			if (playSpotlight)
			{
				GameCreate<SpotlightClass>(pExpReceiver->Location, 50);
			}

			AEProperties::Recalculate(pExpReceiver);
			pExpReceiver->See(0u, 0u);
		}

		pExpReceiver->CurrentRanking = newRank;
	}
}

void TechnoExtData::UpdateVeterancy(TechnoClass*& pExpReceiver, TechnoClass* pKiller, TechnoClass* pVictim, int VictimCost, double& d_factor, bool promoteImmediately)
{
	if (pExpReceiver)
	{
		// no way to get experience by proxy by an enemy unit. you cannot
		// promote your mind-controller by capturing friendly units.
		if (pExpReceiver->Owner->IsAlliedWith(pKiller))
		{

			// if this is a non-missile spawn, handle the spawn manually and switch over to the
			// owner then. this way, a mind-controlled owner is supported.
			TechnoClass* pSpawn = nullptr;
			double SpawnFactor = 1.0;

			GetSpawnerData(pSpawn, pExpReceiver, SpawnFactor, d_factor);
			MCControllerGainExperince(pExpReceiver, pVictim, d_factor, VictimCost);

			// default. promote the unit this function selected.
			AddExperience(pExpReceiver, pVictim, VictimCost, d_factor);

			// if there is a spawn, let it get its share.
			if (pSpawn)
			{
				AddExperience(pSpawn, pVictim, VictimCost, d_factor * SpawnFactor);
			}

			// gunners need to be promoted manually, or they won't only get
			// the experience until after they exited their transport once.
			if (promoteImmediately)
			{
				PromoteImmedietely(pExpReceiver, false, false);
			}
		}
	}
}

void TechnoExtData::EvaluateExtReceiverData(TechnoClass*& pExpReceiver, TechnoClass* pKiller, double& d_factor, bool& promoteImmediately)
{
	const auto pKillerTechnoType = GET_TECHNOTYPE(pKiller);
	const auto pKillerTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pKillerTechnoType);

	if (!KillerInTransporterFactor(pKiller, pExpReceiver, d_factor, promoteImmediately))
	{
		if (pKillerTechnoType->Gunner)
		{
			// an IFV can get experience, too, but we have to have an extra check
			// because the gunner is not the killer.
			FootClass* pGunner = pKiller->Passengers.GetFirstPassenger();
			const auto& nKillerVet = pKiller->Veterancy;
			if (pKillerTechnoType->Trainable && !nKillerVet.IsElite() && (!pGunner || pKillerTechnoTypeExt->ExperienceFromPassengers))
			{
				// the IFV gets credited
				pExpReceiver = pKiller;
			}
			else if (pGunner
				&& (nKillerVet.IsElite() || !pKillerTechnoTypeExt->ExperienceFromPassengers)
				&& GET_TECHNOTYPE(pGunner)->Trainable && pKillerTechnoTypeExt->PassengersGainExperience)
			{

				pExpReceiver = pGunner;
				d_factor *= pKillerTechnoTypeExt->PassengerExperienceModifier;
				promoteImmediately = true;
			}

		}
		else if (pKillerTechnoType->Trainable)
		{

			// the killer itself gets credited.
			pExpReceiver = pKiller;

		}
		else if (pKillerTechnoType->MissileSpawn)
		{

			// unchanged game logic
			if (TechnoClass* pSpawner = pKiller->SpawnOwner)
			{
				TechnoTypeClass* pTSpawner = GET_TECHNOTYPE(pSpawner);

				if (pTSpawner->Trainable)
				{
					pExpReceiver = pSpawner;
				}
			}

		}
		else if (pKiller->CanOccupyFire())
		{
			// game logic, with added check for Trainable
			if (BuildingClass* pKillerBld = cast_to<BuildingClass*, false>(pKiller))
			{
				InfantryClass* pOccupant = pKillerBld->Occupants[pKillerBld->FiringOccupantIndex];
				if (pOccupant->Type->Trainable)
				{
					pExpReceiver = pOccupant;
				}
			}
		}
	}
}


void TechnoExtData::AddPassengers(BuildingClass* const Grinder, FootClass* Vic, bool ParentReversed)
{
	auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(Grinder->Type);

	while (Vic->Passengers.FirstPassenger)
	{
		if (auto nPass = Vic->RemoveFirstPassenger())
		{
			if (auto pTeam = nPass->Team)
			{
				pTeam->RemoveMember(nPass);
			}

			if (ParentReversed && Grinder->Type->Grinding && pBldTypeExt->ReverseEngineersVictims_Passengers)
			{
				if (BuildingExtData::ReverseEngineer(Grinder, nPass))
				{
					if (nPass->Owner && nPass->Owner->ControlledByCurrentPlayer())
					{
						VoxClass::Play(nPass->WhatAmI() == InfantryClass::AbsID ? "EVA_ReverseEngineeredInfantry" : "EVA_ReverseEngineeredVehicle");
						VoxClass::Play(GameStrings::EVA_NewTechAcquired());
					}

					if (const auto FirstTag = Grinder->AttachedTag)
					{
						FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, nPass);

						if (auto pSecondTag = Grinder->AttachedTag)
						{
							pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
						}
					}
				}

			}

			// #368: refund hijackers
			if (Grinder->Type->Grinding && nPass->HijackerInfantryType != -1)
			{
				Grinder->Owner->TransactMoney(InfantryTypeClass::Array->Items[nPass->HijackerInfantryType]->GetRefund(nPass->Owner, 0));
			}

			AddPassengers(Grinder, nPass, ParentReversed);
			Grinder->Owner->TransactMoney(nPass->GetRefund());

			if (nPass->InOpenToppedTransport)
				Vic->MarkPassengersAsExited();

			if (GET_TECHNOTYPE(Vic)->Gunner)
				Vic->RemoveGunner(nPass);

			nPass->Transporter = nullptr;
			nPass->UnInit();
		}
	}
}

bool TechnoExtData::IsSabotagable(BuildingClass const* const pThis)
{
	const auto pType = pThis->Type;
	const auto pExt = BuildingTypeExtContainer::Instance.Find(pType);
	const auto civ_occupiable = pType->CanBeOccupied && pType->TechLevel == -1;
	const auto default_sabotabable = pType->CanC4 && !civ_occupiable;

	return pExt->ImmuneToSaboteurs.isset() ? !pExt->ImmuneToSaboteurs : default_sabotabable;
}

bool TechnoExtData::ApplyC4ToBuilding(InfantryClass* const pThis, BuildingClass* const pBuilding, const bool IsSaboteur)
{
	const auto pInfext = InfantryTypeExtContainer::Instance.Find(pThis->Type);

	if (pBuilding->IsIronCurtained() || pBuilding->IsBeingWarpedOut()
		|| pBuilding->GetCurrentMission() == Mission::Selling
		|| BuildingExtContainer::Instance.Find(pBuilding)->AboutToChronoshift
		)
	{
		pThis->AbortMotion();
		pThis->Uncloak(false);
		const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
		pThis->ReloadTimer.Start(Rof);
		if (!IsSaboteur)
		{
			pThis->Scatter(pBuilding->GetCoords(), true, true);
		}
		return false;
	}
	else
		if (pBuilding->IsGoingToBlow)
		{
			const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
			pThis->ReloadTimer.Start(Rof);
			if (!IsSaboteur)
			{
				pThis->AbortMotion();
				//need to set target ?
				pThis->SetDestination(nullptr, true);
				pThis->Scatter(pBuilding->GetCoords(), true, true);
			}
			return false;
		}

	// sabotage
	pBuilding->IsGoingToBlow = true;
	pBuilding->C4AppliedBy = pThis;

	const auto pData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
	const auto delay = pInfext->C4Delay.Get(RulesClass::Instance->C4Delay);

	auto duration = (int)(delay * 900.0);

	// modify good durations only
	if (duration > 0)
	{
		duration = (int)(duration * pData->C4_Modifier);
		if (duration <= 0)
			duration = 1;
	}

	//auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
	//if (pInfext->C4Damage.isset())
	//{
	//	pBldExt->C4Damage = pInfext->C4Damage;
	//}
	//
	//pBldExt->C4Warhead = pInfext->C4Warhead.Get(RulesClass::Instance->C4Warhead);
	//pBldExt->C4Owner = pThis->GetOwningHouse();
	pBuilding->Flash(duration / 2);
	pBuilding->GoingToBlowTimer.Start(duration);

	if (!IsSaboteur)
	{
		pThis->SetDestination(nullptr, true);
		pThis->Scatter(pBuilding->GetCoords(), true, true);
	}

	return true;
}

Action TechnoExtData::GetiInfiltrateActionResult(InfantryClass* pInf, BuildingClass* pBuilding)
{
	auto const pInfType = pInf->Type;
	auto const pBldType = pBuilding->Type;

	if ((pInfType->C4 || pInf->HasAbility(AbilityType::C4)) && pBldType->CanC4)
		return Action::Self_Deploy;

	const bool IsAgent = pInfType->Agent;
	if (IsAgent && pBldType->Spyable)
	{
		auto pBldOwner = pBuilding->GetOwningHouse();
		auto pInfOwner = pInf->GetOwningHouse();

		if (!pBldOwner || (pBldOwner != pInfOwner && !pBldOwner->IsAlliedWith(pInfOwner)))
			return Action::Move;
	}

	auto const bIsSaboteur = TechnoTypeExtContainer::Instance.Find(pInfType)->Saboteur.Get();

	if (bIsSaboteur && IsSabotagable(pBuilding))
		return Action::NoMove;

	return IsAgent || bIsSaboteur || !pBldType->Capturable ? Action::None : Action::Enter;
}

bool TechnoExtData::IsOperated(TechnoClass* pThis)
{
	const auto pExt = GET_TECHNOTYPEEXT(pThis);

	if (pExt->Operators.empty())
	{
		if (pExt->Operator_Any)
			return pThis->Passengers.GetFirstPassenger() != nullptr;

		TechnoExtContainer::Instance.Find(pThis)->Is_Operated = true;
		return true;
	}
	else
	{
		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (pExt->Operators.Contains((TechnoTypeClass*)object->GetType()))
			{
				// takes a specific operator and someone is present AND that someone is the operator, therefore it is operated
				return true;
			}
		}
	}

	return false;
}

bool TechnoExtData::IsOperatedB(TechnoClass* pThis)
{
	return TechnoExtContainer::Instance.Find(pThis)->Is_Operated || TechnoExtData::IsOperated(pThis);
}

bool TechnoExtData::IsPowered(TechnoClass* pThis)
{
	auto pType = GET_TECHNOTYPE(pThis);

	if (pType->PoweredUnit)
	{
		for (const auto& pBuilding : pThis->Owner->Buildings)
		{
			if (pBuilding->Type->PowersUnit == pType
				&& pBuilding->RegisteredAsPoweredUnitSource
				&& !pBuilding->IsUnderEMP()) // alternatively, HasPower, IsPowerOnline()
			{
				return true;
			}
		}
		// if we reach this, we found no building that currently powers this object
		return false;
	}
	else if (auto pPower = TechnoExtContainer::Instance.Find(pThis)->GetPoweredUnit())
	{
		// #617
		return pPower->IsPowered();
	}

	// object doesn't need a particular powering structure, therefore, for the purposes of the game, it IS powered
	return true;
}

void TechnoExtData::EvalRaidStatus(BuildingClass* pThis)
{
	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	// if the building is still marked as raided, but unoccupied, return it to its previous owner
	if (pExt->OwnerBeforeRaid && !pThis->Occupants.Count)
	{
		// Fix for #838: Only return the building to the previous owner if he hasn't been defeated
		if (!pExt->OwnerBeforeRaid->Defeated)
		{
			pThis->SetOwningHouse(pExt->OwnerBeforeRaid, false);
		}

		pExt->OwnerBeforeRaid = nullptr;
	}
}

//new
bool TechnoExtData::IsUnitAlive(UnitClass* pUnit)
{
	if (!pUnit->IsAlive)
		return false;

	if (pUnit->InLimbo)
		return false;

	if (pUnit->TemporalTargetingMe)
		return false;

	if (TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
		return false;

	if (pUnit->BerzerkDurationLeft)
		return false;

	if (pUnit->LocomotorSource)
		return false;

	if (pUnit->DeathFrameCounter > 0)
		return false;

	if (TechnoExtData::IsInWarfactory(pUnit))
		return false;

	return true;
}

//confirmed
void TechnoExtData::SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight)
{
	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		const auto pBld = (BuildingClass*)pThis;

		if (pBld->Spotlight != pSpotlight)
		{
			GameDelete<true, true>(std::exchange(pBld->Spotlight, pSpotlight));
		}
	}

	if (TechnoExtContainer::Instance.Find(pThis)->BuildingLight != pSpotlight)
	{
		GameDelete<true, true>(std::exchange(TechnoExtContainer::Instance.Find(pThis)->BuildingLight, pSpotlight));
	}
}

//confirmed
bool NOINLINE  TechnoExtData::CanSelfCloakNow(TechnoClass* pThis)
{
	// cloaked and deactivated units are hard to find otherwise
	if (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled || pThis->Deactivated)
	{
		return false;
	}

	const auto what = pThis->WhatAmI();
	auto pType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (what == BuildingClass::AbsID)
	{
		if (pExt->CloakPowered && !pThis->IsPowerOnline())
		{
			return false;
		}

	}
	else
	{
		if (what == InfantryClass::AbsID
				&& pExt->CloakDeployed
				&& !((InfantryClass*)pThis)->IsDeployed())
		{
			return false;
		}
		else if (what == UnitClass::AbsID)
		{
			if (((UnitClass*)pThis)->DeathFrameCounter > 0)
				return false;
		}
	}

	// allows cloak
	return true;
}

//confirmed
bool NOINLINE TechnoExtData::IsCloakable(TechnoClass* pThis, bool allowPassive)
{
	TechnoTypeClass* pType = GET_TECHNOTYPE(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	// object disallowed from cloaking
	if (!pTypeExt->CloakAllowed || pExt->AE.flags.ForceDecloak)
	{
		return false;
	}

	// parachuted units cannot cloak. this makes paradropping
	// units uncloakable like they were in the vanilla game
	if (pThis->Parachute)
	{
		return false;
	}

	// check for active cloak
	if (pThis->IsCloakable() || pThis->HasAbility(AbilityType::Cloak))
	{
		if (TechnoExtData::CanSelfCloakNow(pThis))
		{
			return true;
		}
	}

	// if not actively cloakable
	if (allowPassive)
	{
		// cloak generators ignore everything above ground. this
		// fixes hover units not being affected by cloak.
		if (pThis->GetHeight() > RulesExtData::Instance()->
					CloakHeight.Get(RulesClass::Instance->HoverHeight))
		{
			return false;
		}

		// search for cloak generators
		CoordStruct crd = pThis->GetCoords();
		CellClass* pCell = MapClass::Instance->GetCellAt(crd);
		return pCell->CloakGen_InclHouse(pThis->Owner->ArrayIndex);
	}

	return false;
}

//confirmed
bool TechnoExtData::CloakDisallowed(TechnoClass* pThis, bool allowPassive)
{
	if (TechnoExtData::IsCloakable(pThis, allowPassive))
	{
		auto pExt = TechnoExtContainer::Instance.Find(pThis);
		return pExt->CloakSkipTimer.InProgress()
			|| pThis->IsUnderEMP()
			|| pThis->IsParalyzed()
			|| pThis->IsBeingWarpedOut()
			|| pThis->IsWarpingIn();
	}

	return true;
}

//confirmed
bool TechnoExtData::CloakAllowed(TechnoClass* pThis)
{
	if (TechnoExtData::CloakDisallowed(pThis, true))
	{
		return false;
	}

	if (pThis->CloakState == CloakState::Cloaked)
	{
		return false;
	}

	if (!GET_TECHNOTYPEEXT(pThis)->Cloakable_IgnoreArmTimer
		 && pThis->RearmTimer.InProgress())
	{
		return false;
	}

	if (pThis->CloakDelayTimer.InProgress())
	{
		return false;
	}

	if (pThis->Target && pThis->IsCloseEnoughToAttack(pThis->Target))
	{
		//https://bugs.launchpad.net/ares/+bug/1267287
		const auto pWeaponIdx = pThis->SelectWeapon(pThis->Target);
		const auto pWeapon = pThis->GetWeapon(pWeaponIdx);

		if (pWeapon && pWeapon->WeaponType && pWeapon->WeaponType->DecloakToFire)
			return false;
	}

	if (pThis->WhatAmI() != BuildingClass::AbsID)
	{
		if (pThis->CloakProgress.Stage)
			return false;

		if (pThis->LocomotorSource && ((FootClass*)pThis)->IsAttackedByLocomotor)
			return false;
	}

	return true;
}

InfantryTypeClass* TechnoExtData::GetBuildingCrew(BuildingClass* pThis, int nChance)
{
	// with some luck, and if the building has not been captured, spawn an engineer
	if (!pThis->HasBeenCaptured
		&& nChance > 0
		&& ScenarioClass::Instance->Random.RandomFromMax(99) < nChance)
	{
		return HouseExtData::GetEngineer(pThis->Owner);
	}

	return FakeTechnoClass::__GetCrew(pThis);
}

void TechnoExtData::UpdateFactoryQueues(BuildingClass const* const pBuilding)
{
	if (pBuilding->Type->Factory != AbstractType::None)
	{
		pBuilding->Owner->Update_FactoriesQueues(
			pBuilding->Type->Factory,
			pBuilding->Type->Naval,
			BuildCat::DontCare
		);
	}
}

bool TechnoExtData::IsBaseNormal(BuildingClass* pBuilding)
{
	if (BuildingExtContainer::Instance.Find(pBuilding)->IsFromSW)
		return true;

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (pExt->AIBaseNormal.isset())
		return pExt->AIBaseNormal;

	if (pBuilding->Type->UndeploysInto && pBuilding->Type->ResourceGatherer || pBuilding->IsStrange())
		return true;

	return false;
}

int TechnoExtData::GetVictimBountyValue(TechnoClass* pVictim, TechnoClass* pKiller)
{
	int Value = 0;
	const auto pKillerTypeExt = GET_TECHNOTYPEEXT(pKiller);
	const auto pVictimTypeExt = GET_TECHNOTYPEEXT(pVictim);

	switch (pKillerTypeExt->Bounty_Value_Option.Get(RulesExtData::Instance()->Bounty_Value_Option))
	{
	case BountyValueOption::Cost:
		Value = pVictimTypeExt->This()->GetCost();
		break;
	case BountyValueOption::Soylent:
		Value = pVictim->GetRefund();
		break;
	case BountyValueOption::ValuePercentOfConst:
		Value = int(pVictimTypeExt->This()->GetCost() * pVictimTypeExt->Bounty_Value_PercentOf.Get(pVictim));
		break;
	case BountyValueOption::ValuePercentOfSoylent:
		Value = int(pVictim->GetRefund() * pVictimTypeExt->Bounty_Value_PercentOf.Get(pVictim));
		break;
	default:
		Value = pVictimTypeExt->Bounty_Value.Get(pVictim);
		break;
	}

	if (Value == 0)
		return 0;

	const double nVicMult = pVictimTypeExt->Bounty_Value_mult.Get(pVictim);
	const double nMult = pKillerTypeExt->BountyBonusmult.Get(pKiller);

	return int(Value * nVicMult * nMult);
}

bool TechnoExtData::KillerAllowedToEarnBounty(TechnoClass* pKiller, TechnoClass* pVictim)
{
	if (!pKiller || !pVictim || !pKiller->Owner || !pVictim->Owner || !TechnoExtData::IsBountyHunter(pKiller))
		return false;

	const auto pHouseTypeExt = HouseTypeExtContainer::Instance.TryFind(pVictim->Owner->Type);

	if (pHouseTypeExt && !pHouseTypeExt->GivesBounty)
		return false;

	const auto pKillerTypeExt = GET_TECHNOTYPEEXT(pKiller);
	const auto pVictimType = GET_TECHNOTYPE(pVictim);

	if (!pKillerTypeExt->BountyAllow.Eligible(pVictimType))
		return false;

	if (!pKillerTypeExt->BountyDissallow.empty() && pKillerTypeExt->BountyDissallow.Contains(pVictimType))
		return false;

	if (pKiller->Owner->IsAlliedWith(pVictim))
		return false;

	if (pKillerTypeExt->Bounty_IgnoreEnablers || RulesExtData::Instance()->Bounty_Enablers.empty())
		return true;

	for (auto const& pEnablers : RulesExtData::Instance()->Bounty_Enablers)
	{
		if (pKiller->Owner->ActiveBuildingTypes.get_count(pEnablers->ArrayIndex) > 0)
			return true;
	}

	return false;
}

void TechnoExtData::GiveBounty(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!TechnoExtData::KillerAllowedToEarnBounty(pKiller, pVictim))
		return;

	const auto pKillerTypeExt = GET_TECHNOTYPEEXT(pKiller);
	const int nValueResult = TechnoExtData::GetVictimBountyValue(pVictim, pKiller);

	if (nValueResult != 0 && pKiller->Owner->AbleToTransactMoney(nValueResult))
	{
		if (pKillerTypeExt->Bounty_Display.Get(RulesExtData::Instance()->Bounty_Display))
		{
			if (pKillerTypeExt->This()->MissileSpawn && pKiller->SpawnOwner)
				pKiller = pKiller->SpawnOwner;

			VocClass::SafeImmedietelyPlayAt(pKillerTypeExt->Bounty_ReceiveSound, &pKiller->Location);

			pKiller->Owner->TransactMoney(nValueResult);
			TechnoExtContainer::Instance.Find(pKiller)->TechnoValueAmount += nValueResult;
		}
	}
}

AresHijackActionResult TechnoExtData::GetActionHijack(InfantryClass* pThis, TechnoClass* const pTarget)
{
	if (!pThis || !pTarget || !pThis->IsAlive || !pTarget->IsAlive || pTarget->IsIronCurtained())
		return AresHijackActionResult::None;

	if (pThis->WhatAmI() != InfantryClass::AbsID)
		return AresHijackActionResult::None;

	const auto pType = pThis->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// this can't steal vehicles
	if (!pType->VehicleThief && !pTypeExt->CanDrive.Get(RulesExtData::Instance()->CanDrive))
	{
		return AresHijackActionResult::None;
	}

	const auto pTargetType = GET_TECHNOTYPE(pTarget);
	const auto absTarget = pTarget->WhatAmI();
	const auto pTargetUnit = absTarget == UnitClass::AbsID ? static_cast<UnitClass*>(pTarget) : nullptr;

	// bunkered units can't be hijacked.
	if (pTarget->BunkerLinkedItem
		// VehicleThief cannot take `NonVehicle`
		|| (pType->VehicleThief && pTargetUnit && pTargetUnit->Type->NonVehicle))
	{
		return AresHijackActionResult::None;
	}

	//no , this one bit different ?
	const bool IsNotOperated = !TechnoExtContainer::Instance.Find(pThis)->Is_Operated
		&& !TechnoExtData::IsOperated(pTarget);

	// i'm in a state that forbids capturing
	if (pThis->IsDeployed() || IsNotOperated)
	{
		return AresHijackActionResult::None;
	}

	// target type is not eligible (hijackers can also enter strange buildings)

	if (absTarget != AbstractType::Aircraft
		&& absTarget != AbstractType::Unit
		&& (!pType->VehicleThief || absTarget != AbstractType::Building)
		)
	{
		return AresHijackActionResult::None;
	}

	// target is bad
	if (pTarget->CurrentMission == Mission::Selling
		|| pTarget->IsBeingWarpedOut()
		|| pTargetType->IsTrain
		|| pTargetType->BalloonHover
		|| (absTarget != AbstractType::Unit && !pTarget->IsStrange())
		//|| (absTarget == abs_Unit && ((UnitTypeClass*)pTargetType)->NonVehicle) replaced by Hijacker.Allowed
		|| !pTarget->IsOnFloor())
	{
		return AresHijackActionResult::None;
	}

	// a thief that can't break mind control loses without trying further
	if (pType->VehicleThief && pTarget->IsMindControlled()
		&& !pTypeExt->HijackerBreakMindControl)
	{
		return AresHijackActionResult::None;
	}

	if (pTargetUnit
		&& ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& RulesClass::Instance->HarvesterUnit.contains(pTargetUnit->Type))
	{
		return AresHijackActionResult::None;
	}

	//drivers can drive, but only stuff owned by neutrals. if a driver is a vehicle thief
	//also, it can reclaim units even if they are immune to hijacking (see below)
	const auto pHouseTypeExt = HouseTypeExtContainer::Instance.Find(pTarget->Owner->Type);
	const auto specialOwned = pHouseTypeExt->CanBeDriven.Get(pTarget->Owner->Type->MultiplayPassive);
	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	if (specialOwned && pTypeExt->CanDrive.Get(RulesExtData::Instance()->CanDrive) && pTargetTypeExt->CanBeDriven)
	{
		return AresHijackActionResult::Drive;
	}

	// hijacking only affects enemies
	if (pType->VehicleThief)
	{
		// can't steal allied unit (CanDrive and special already handled)
		if (pThis->Owner->IsAlliedWith(pTarget->Owner) || specialOwned)
		{
			return AresHijackActionResult::None;
		}

		if (!pTargetTypeExt->HijackerAllowed)
		{
			return AresHijackActionResult::None;
		}

		// allowed to steal from enemy
		return AresHijackActionResult::Hijack;
	}

	// no hijacking ability
	return AresHijackActionResult::None;
}

bool TechnoExtData::PerformActionHijack(TechnoClass* pFrom, TechnoClass* const pTarget)
{
	// was the hijacker lost in the process?
	bool ret = false;

	if (const auto pThis = cast_to<InfantryClass*, false>(pFrom))
	{
		const auto pType = pThis->Type;
		//const auto pExt = TechnoExtContainer::Instance.Find(pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		const auto action = TechnoExtData::GetActionHijack(pThis, pTarget);

		// abort capturing this thing, it looked
		// better from over there...
		if (action == AresHijackActionResult::None)
		{
			pThis->SetDestination(nullptr, true);
			const auto& crd = pTarget->GetCoords();
			pThis->Scatter(crd, true, false);
			return false;
		}

		// prepare for a smooth transition. free the destination from
		// any mind control. #762
		if (pTarget->MindControlledBy)
		{
			pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
		}

		if (pTarget->CaptureManager)
		{
			pTarget->CaptureManager->FreeAll();
		}

		pTarget->MindControlledByAUnit = false;
		if (pTarget->MindControlRingAnim)
		{
			pTarget->MindControlRingAnim->UnInit();
			pTarget->MindControlRingAnim = nullptr;
		}

		bool asPassenger = false;
		const auto pDestTypeExt = GET_TECHNOTYPEEXT(pTarget);
		auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

		if (action == AresHijackActionResult::Drive && (!pDestTypeExt->Operators.empty() || pDestTypeExt->Operator_Any))
		{
			asPassenger = true;

			// raise some events in case the driver enters
			// a vehicle that needs an Operator
			if (pTarget->AttachedTag)
			{
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::EnteredBy,
					pThis, CellStruct::Empty, false, nullptr);
			}
		}
		else
		{
			// raise some events in case the hijacker/driver will be
			// swallowed by the vehicle.
			if (pTarget->AttachedTag)
			{
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::DestroyedByAnything,
					pThis, CellStruct::Empty, false, nullptr);
			}

			pTarget->Owner->HasBeenThieved = true;
			if (auto const pTag = pThis->AttachedTag)
			{
				if (pTag->ShouldReplace())
				{
					pTarget->ReplaceTag(pTag);
				}
			}
		}

		// if the hijacker is mind-controlled, free it,
		// too, and attach to the new target. #762
		const auto controller = (FakeCaptureManagerClass*)pThis->MindControlledBy;
		if (controller)
		{
			controller->__FreeUnit(pThis, true);
		}

		// let's make a steal
		pTarget->SetOwningHouse(pThis->Owner, true);
		pTarget->GotHijacked();
		VocClass::SafeImmedietelyPlayAt(pTypeExt->HijackerEnterSound, &pTarget->Location, nullptr);

		// remove the driverless-marker
		pTargetExt->Is_DriverKilled = 0;

		// save the hijacker's properties
		if (action == AresHijackActionResult::Hijack)
		{
			pTarget->HijackerInfantryType = pType->ArrayIndex;
			pTargetExt->HijackerOwner = pThis->Owner;
			pTargetExt->HijackerHealth = pThis->Health;
			pTargetExt->HijackerVeterancy = pThis->Veterancy.Veterancy;
			TechnoExtData::StoreHijackerLastDisguiseData(pThis, (FootClass*)pTarget);
		}

		// hook up the original mind-controller with the target #762
		if (controller)
		{
			controller->__CaptureUnit(pThis, true, 0);
		}

		// reboot the slave manager
		if (pTarget->SlaveManager)
		{
			pTarget->SlaveManager->ResumeWork();
		}

		// the hijacker enters and closes the door.
		ret = true;

		// only for the drive action: if the target requires an operator,
		// we add the driver to the passengers list instead of deleting it.
		// this does not check passenger count or size limits.
		if (asPassenger)
		{
			pTarget->AddPassenger(pThis);
			pThis->AbortMotion();
			ret = false;
		}

		pTarget->QueueMission(pThis->Owner->IsControlledByHuman() ? Mission::Guard : Mission::Hunt, true);

		if (auto const pTag = pTarget->AttachedTag)
		{
			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::VehicleTaken_ByHouse), pTarget, CellStruct::Empty, false, pThis);
		}

		if (pTarget->IsAlive)
		{
			if (auto const pTag2 = pTarget->AttachedTag)
			{
				pTag2->RaiseEvent(TriggerEvent(AresTriggerEvents::VehicleTaken), pTarget, CellStruct::Empty, false, nullptr);
			}
		}
	}

	return ret;
}

bool TechnoExtData::FindAndTakeVehicle(FootClass* pThis)
{
	const auto pInf = cast_to<InfantryClass*, false>(pThis);
	if (!pInf)
		return false;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pInf->Type);
	if (!pInf->Type->VehicleThief && !pExt->CanDrive.Get(RulesExtData::Instance()->CanDrive))
		return false;

	double bestDist = std::numeric_limits<double>::max();
	int bestID = INT_MAX;
	TechnoClass* best = nullptr;

	for (auto pUnit : *UnitClass::Array.get())
	{
		if (GetActionHijack(pInf, pUnit) == AresHijackActionResult::None)
			continue;

		const CoordStruct diff = pInf->Location - pUnit->Location;
		const double dist = diff.pow();

		if (dist < bestDist ||
			(dist == bestDist && (int)pUnit->UniqueID < bestID))
		{
			best = pUnit;
			bestDist = dist;
			bestID = pUnit->UniqueID;
		}
	}

	if (best)
	{
		TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode = true;
		pThis->ShouldGarrisonStructure = true;
		if (pThis->Target != best || pThis->CurrentMission != Mission::Capture)
		{
			pThis->SetDestination(best, true);
			pThis->QueueMission(Mission::Capture, true);
			return true;
		}
	}

	TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode = false;
	pThis->ShouldGarrisonStructure = false;
	return false;
}

Action TechnoExtData::GetEngineerEnterEnemyBuildingAction(BuildingClass* const pBld)
{
	//Spawner now the one control this
	if (!GameModeOptionsClass::Instance->MultiEngineer)
	{
		// single player missions are currently hardcoded to "don't do damage".
		return Action::Capture; // TODO: replace this by a new rules tag.
	}

	// damage if multi engineer is enabled and target isn't that low on health.
	// check to always capture tech structures. a structure counts
	// as tech if its initial owner is a multiplayer-passive country.
	auto const pRulesExt = RulesExtData::Instance();

	if (pBld->InitialOwner && pBld->InitialOwner->Type->MultiplayPassive && pRulesExt->EngineerAlwaysCaptureTech)
	{
		return Action::Capture;
	}

	if (pBld->GetHealthPercentage() > pRulesExt->AttachedToObject->EngineerCaptureLevel)
	{
		return (pRulesExt->EngineerDamage > 0.0)
			? Action::Damage : Action::NoEnter;
	}

	return Action::Capture;
}

bool TechnoExtData::CloneBuildingEligible(BuildingClass* pBuilding, bool requirePower)
{
	if (pBuilding->InLimbo ||
		!pBuilding->IsAlive ||
		!pBuilding->IsOnMap ||
		pBuilding->TemporalTargetingMe ||
		pBuilding->IsBeingWarpedOut() ||
		BuildingExtContainer::Instance.Find(pBuilding)->AboutToChronoshift ||
		BuildingExtContainer::Instance.Find(pBuilding)->LimboID >= 0
	)
	{
		return false;
	}

	if (pBuilding->Type->Powered && requirePower && !pBuilding->IsPowerOnline())
		return false;

	return true;
}

void TechnoExtData::KickOutClone(BuildingClass* pBuilding, TechnoTypeClass* ProductionType, HouseClass* FactoryOwner)
{
	auto Clone = static_cast<TechnoClass*>(ProductionType->CreateObject(FactoryOwner));

	const auto& nStr = TechnoTypeExtContainer::Instance.Find(pBuilding->Type)->InitialStrength_Cloning;
	if (nStr.isset())
	{
		const auto rStr = GeneralUtils::GetRangedRandomOrSingleValue(nStr);
		const int strength = std::clamp(static_cast<int>(ProductionType->Strength * rStr), 1, ProductionType->Strength);
		Clone->Health = strength;
		Clone->EstimatedHealth = strength;
	}

	if (pBuilding->KickOutUnit(Clone, CellStruct::Empty) != KickOutResult::Succeeded)
	{
		//Debug::LogInfo(__FUNCTION__" Called ");
		TechnoExtData::HandleRemove(Clone, nullptr, false, false);
	}
}

void TechnoExtData::KickOutClones(BuildingClass* pFactory, TechnoClass* const Production)
{
	const auto FactoryType = pFactory->Type;

	if (FactoryType->Cloning ||
		(FactoryType->Factory != InfantryTypeClass::AbsID &&
			FactoryType->Factory != UnitTypeClass::AbsID)
	)
	{
		return;
	}

	const auto ProductionType = GET_TECHNOTYPE(Production);
	const auto ProductionTypeData = TechnoTypeExtContainer::Instance.Find(ProductionType);

	if (!ProductionTypeData->Cloneable)
	{
		return;
	}

	const auto isPlayer = pFactory->Owner->IsControlledByHuman();

	auto ProductionTypeAs = ProductionType;
	if (!isPlayer && ProductionTypeData->AI_ClonedAs)
		ProductionTypeAs = ProductionTypeData->AI_ClonedAs;
	else if (ProductionTypeData->ClonedAs)
		ProductionTypeAs = ProductionTypeData->ClonedAs;

	if (!ProductionTypeAs || !ProductionTypeAs->Strength) // ,....
		return;

	auto const FactoryOwner = pFactory->Owner;
	auto const& CloningSources = ProductionTypeData->ClonedAt;
	auto const IsUnit = (FactoryType->Factory != InfantryTypeClass::AbsID);

	// keep cloning vats for backward compat, unless explicit sources are defined
	if (!IsUnit && CloningSources.empty())
	{
		for (auto const& CloningVat : FactoryOwner->CloningVats)
		{
			if (!CloneBuildingEligible(CloningVat, BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->Cloning_RequirePower))
				continue;

			KickOutClone(CloningVat, ProductionTypeAs, FactoryOwner);
		}

		return;
	}

	// and clone from new sources
	if (!CloningSources.empty() || IsUnit)
	{
		for (auto const& CloningVat : FactoryOwner->Buildings)
		{
			if (!CloneBuildingEligible(CloningVat, BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->Cloning_RequirePower))
				continue;

			//auto const BType = CloningVat->Type;

			auto ShouldClone = false;
			if (!CloningSources.empty())
			{
				ShouldClone = CloningSources.Contains(CloningVat->Type);
			}
			else if (IsUnit)
			{
				ShouldClone = BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->CloningFacility && (CloningVat->Type->Naval == FactoryType->Naval);
			}

			if (ShouldClone)
			{
				KickOutClone(CloningVat, ProductionTypeAs, FactoryOwner);
			}
		}
	}
}

void TechnoExtData::InitWeapon(
	TechnoClass* pThis,
	TechnoTypeClass* pType,
	WeaponTypeClass* pWeapon,
	int idxWeapon,
	CaptureManagerClass*& pCapture,
	ParasiteClass*& pParasite,
	TemporalClass*& pTemporal,
	const char* pTagName,
	bool IsFoot
)
{
	COMPILETIMEEVAL auto const Note = "Constructing an instance of [%s]:\r"
		"%s %s (slot %d) has no %s!";

	if (!pWeapon->Projectile)
	{
		Debug::FatalErrorAndExit(
			Note, pType->ID, pTagName, pWeapon->ID, idxWeapon,
			"Projectile");
	}

	auto const pWarhead = pWeapon->Warhead;

	if (!pWarhead)
	{
		Debug::FatalErrorAndExit(
			Note, pType->ID, pTagName, pWeapon->ID, idxWeapon, "Warhead");
	}

	if (pWarhead->MindControl && !pCapture)
	{
		pCapture = GameCreate<CaptureManagerClass>(
			pThis, pWeapon->Damage, pWeapon->InfiniteMindControl);
	}

	if (pWarhead->Temporal && !pTemporal)
	{
		pTemporal = GameCreate<TemporalClass>(pThis);
		pTemporal->WarpPerStep = pWeapon->Damage;
		TechnoExtContainer::Instance.Find(pThis)->idxSlot_Warp = static_cast<BYTE>(idxWeapon);
	}

	if (pWarhead->Parasite && IsFoot && !pParasite)
	{
		pParasite = GameCreate<ParasiteClass>((FootClass*)pThis);
		TechnoExtContainer::Instance.Find(pThis)->idxSlot_Parasite = static_cast<BYTE>(idxWeapon);
	}
}

InfantryClass* TechnoExtData::RecoverHijacker(FootClass* const pThis)
{
	if (auto const pType = InfantryTypeClass::Array->get_or_default(
		pThis->HijackerInfantryType))
	{
		const auto pOwner = TechnoExtContainer::Instance.Find(pThis)->HijackerOwner ?
			TechnoExtContainer::Instance.Find(pThis)->HijackerOwner : pThis->Owner;

		pThis->HijackerInfantryType = -1;

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		if (!pTypeExt->HijackerOneTime && pOwner && !pOwner->Defeated)
		{
			if (auto const pHijacker = static_cast<InfantryClass*>(pType->CreateObject(pOwner)))
			{
				TechnoExtData::RestoreStoreHijackerLastDisguiseData(pHijacker, pThis);
				pHijacker->Health = MaxImpl(TechnoExtContainer::Instance.Find(pThis)->HijackerHealth, 10) / 2;
				pHijacker->Veterancy.Veterancy = TechnoExtContainer::Instance.Find(pThis)->HijackerVeterancy;
				return pHijacker;
			}
		}
	}

	return nullptr;
}

void TechnoExtData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses, const bool PreventPassengersEscape)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pOwner = pThis->Owner;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// do not ever do this again for this unit
	if (!TechnoExtContainer::Instance.Find(pThis)->Is_SurvivorsDone)
	{
		TechnoExtContainer::Instance.Find(pThis)->Is_SurvivorsDone = true;
	}
	else
	{
		return;
	}

	// always eject passengers, but passengers only if not supressed.
	if (!TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled && !IgnoreDefenses)
	{
		// save this, because the hijacker can kill people
		auto pilotCount = pThis->GetCrewCount();

		// process the hijacker
		if (auto const pHijacker = RecoverHijacker(pThis))
		{
			auto const pHijackerTypeExt = TechnoTypeExtContainer::Instance.Find(pHijacker->Type);

			if (!TechnoExtData::EjectRandomly(pHijacker, pThis->Location, 144, Select))
			{
				pHijacker->RegisterDestruction(pKiller);
				//" Hijacker Called ");
				TechnoExtData::HandleRemove(pHijacker, pKiller, false, true);
			}
			else
			{
				// the hijacker will now be controlled instead of the unit
				if (auto const pController = (FakeCaptureManagerClass*)pThis->MindControlledBy)
				{
					pController->__FreeUnit(pThis, true);
					pController->__CaptureUnit(pHijacker, true, 0);
					pHijacker->QueueMission(Mission::Guard, true); // override the fate the AI decided upon
				}

				VocClass::SafeImmedietelyPlayAt(pHijackerTypeExt->HijackerLeaveSound, &pThis->Location, nullptr);

				// lower than 0: kill all, otherwise, kill n pilots
				pilotCount = ((pHijackerTypeExt->HijackerKillPilots < 0) ? 0 :
					(pilotCount - pHijackerTypeExt->HijackerKillPilots));
			}
		}

		// possibly eject up to pilotCount crew members
		if (pilotCount > 0 && pType->Crewed)
		{
			int pilotChance = pTypeExt->Survivors_PilotChance.Get(pThis);
			if (pilotChance < 0)
			{
				pilotChance = static_cast<int>(RulesClass::Instance->CrewEscape * 100);
			}

			if (pilotChance > 0)
			{

				for (int i = 0; i < pilotCount; ++i)
				{
					if (auto pPilotType = FakeTechnoClass::__GetCrew(pThis))
					{
						if (ScenarioClass::Instance->Random.RandomRanged(1, 100) <= pilotChance)
						{
							auto const pPilot = static_cast<InfantryClass*>(pPilotType->CreateObject(pOwner));
							pPilot->Health /= 2;
							pPilot->Veterancy.Veterancy = pThis->Veterancy.Veterancy;

							if (!TechnoExtData::EjectRandomly(pPilot, pThis->Location, 144, Select))
							{
								pPilot->RegisterDestruction(pKiller);
								//Debug::LogInfo(__FUNCTION__" Pilot Called ");
								TechnoExtData::HandleRemove(pPilot, pKiller, false, false);
							}
							else if (auto const pTag = pThis->AttachedTag)
							{
								if (pTag->ShouldReplace())
								{
									pPilot->ReplaceTag(pTag);
								}
							}
						}
					}
				}
			}
		}
	}

	if (!PreventPassengersEscape)
	{
		// passenger escape chances
		const auto passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

		//quick exit
		if (passengerChance == 0)
			return;

		const auto what = pThis->WhatAmI();

		// eject or kill all passengers
		while (pThis->Passengers.FirstPassenger)
		{
			auto const pPassenger = pThis->RemoveFirstPassenger();

			bool trySpawn = false;
			if (passengerChance > 0)
			{
				trySpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= passengerChance;
			}
			else if (passengerChance == -1 && what == UnitClass::AbsID)
			{
				const Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), FacingType::None, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}

			if (trySpawn && TechnoExtData::EjectRandomly(pPassenger, pThis->Location, 128, Select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			//Debug::LogInfo(__FUNCTION__" Passengers Called ");
			TechnoExtData::HandleRemove(pPassenger, pKiller, false, false);
		}
	}
}

int TechnoExtData::GetWarpPerStep(TemporalClass* pThis, int nStep)
{
	int totalStep = 0;

	if (!pThis)
		return 0;

	for (TemporalClass* pTemp = pThis; pTemp; pTemp = pTemp->PrevTemporal)
	{
		if (nStep > 50)
			break;

		++nStep;

		if (auto pTempOwner = pTemp->Owner)
		{

			auto const pWeapon = pTempOwner->GetWeapon(TechnoExtContainer::Instance.Find(pTempOwner)->idxSlot_Warp)
				->WeaponType;

			totalStep += pWeapon->Damage;
			pTemp->WarpPerStep = pWeapon->Damage;
		}
	}

	return totalStep;
}

bool TechnoExtData::Warpable(TemporalClass* pTemp, TechnoClass* pTarget)
{
	if (!pTarget || !pTarget->IsAlive || pTarget->IsSinking || pTarget->IsCrashing || pTarget->IsIronCurtained())
		return false;

	//the fuck
	if (pTarget == pTemp->Owner)
		return false;

	if (TechnoExtData::IsUnwarpable(pTarget))
		return false;

	if (pTarget->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find((BuildingClass*)pTarget)->AboutToChronoshift)
		{
			return false;
		}
	}
	else
	{
		if (TechnoExtData::IsInWarfactory(pTarget, true))
			return false;

		if (TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTarget)))
			return false;
	}

	return true;
}

void TechnoExtData::DepositTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, float const bonus, int const idxType)
{
	auto pTiberium = TiberiumClass::Array->Items[idxType];
	auto value = 0;

	// always put the purified money on the bank account. otherwise ore purifiers
	// would fill up storage with tiberium that doesn't exist. this is consistent with
	// the original YR, because old GiveTiberium put it on the bank anyhow, despite its name.
	if (bonus > 0.0)
	{
		value += int(bonus * pTiberium->Value * pHouse->Type->IncomeMult);
	}

	// also add the normal tiberium to the global account?
	if (amount > 0.0)
	{
		auto const pExt = GET_TECHNOTYPEEXT(pThis);
		if (!pExt->Refinery_UseStorage)
		{
			value += int(amount * pTiberium->Value * pHouse->Type->IncomeMult);
		}
		else
		{
			int decidedIndex = idxType;
			float decidedAmount = amount;
			if (pThis->WhatAmI() == BuildingClass::AbsID && RulesExtData::Instance()->Storage_TiberiumIndex >= 0)
			{
				pTiberium = TiberiumClass::Array->Items[RulesExtData::Instance()->Storage_TiberiumIndex];
				decidedIndex = RulesExtData::Instance()->Storage_TiberiumIndex;
				decidedAmount = (amount * pTiberium->Value) / pTiberium->Value;
			}

			((FakeHouseClass*)(pHouse))->_GiveTiberium(decidedAmount, decidedIndex);
		}
	}

	// deposit
	if (value > 0)
	{
		pHouse->GiveMoney(value);
	}
}

void TechnoExtData::RefineTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, int const idxType)
{
	const auto refined = BuildingTypeExtData::GetPurifierBonusses(pHouse) * amount;
	// add the tiberium to the house's credits
	TechnoExtData::DepositTiberium(pThis, pHouse, amount, refined, idxType);
}

bool TechnoExtData::FiringAllowed(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto nRulesGreen = RulesClass::Instance->ConditionGreen;
	const auto pThatTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pThatShield = pThatTechnoExt->GetShield();

	if (pThatShield && pThatShield->IsActive())
	{
		if (!pThatShield->CanBePenetrated(pWeapon->Warhead))
		{
			if (pThatShield->GetType()->CanBeHealed)
			{
				const bool IsFullHP = pThatShield->GetHealthRatio() >= nRulesGreen;
				if (IsFullHP && pThatShield->GetType()->PassthruNegativeDamage)
					return !(pTarget->GetHealthPercentage_() >= nRulesGreen);

				return true;
			}

			return false;
		}
	}

	return !(pTarget->GetHealthPercentage_() >= nRulesGreen);
}

UnitTypeClass* TechnoExtData::GetUnitTypeImage(UnitClass* const pThis)
{
	UnitTypeClass* pType = pThis->Type;
	bool isDisguised = false;

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
	{
		const auto pTargetType = pThis->GetDisguise(true);

		if (pTargetType && pTargetType->WhatAmI() == UnitTypeClass::AbsID)
		{
			pType = (UnitTypeClass*)pTargetType;
			isDisguised = true;
		}
	}

	const auto pData = TechnoTypeExtContainer::Instance.Find(pType);

	if ((pData->WaterImage || pData->WaterImage_Yellow || pData->WaterImage_Red) && !pThis->OnBridge && pThis->GetCell()->LandType == LandType::Water && !pThis->IsAttackedByLocomotor)
	{
		if (pData->WaterImage_Red && pThis->IsRedHP())
			return pData->WaterImage_Red;

		if (pData->WaterImage_Yellow && pThis->IsYellowHP())
			return pData->WaterImage_Red;

		return  pData->WaterImage;
	}

	if (pData->Image_Red && pThis->IsRedHP())
		return (UnitTypeClass*)pData->Image_Red.Get();

	if (pData->Image_Yellow && pThis->IsYellowHP())
		return (UnitTypeClass*)pData->Image_Yellow.Get();

	return isDisguised ? pType : (UnitTypeClass*)nullptr;
}

TechnoTypeClass* TechnoExtData::GetImage(FootClass* pThis)
{
	if (const auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		TechnoTypeClass* Image = pUnit->Type;

		if (UnitTypeClass* const pCustomType = TechnoExtData::GetUnitTypeImage(pUnit))
		{
			Image = pCustomType;
		}

		if (pUnit->Deployed && pUnit->Type->UnloadingClass)
		{
			Image = pUnit->Type->UnloadingClass;
		}

		// if (!pUnit->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
		// {
		// 	if (auto pDisUnit = type_cast<UnitTypeClass*>(pUnit->GetDisguise(true)))
		// 	{
		// 		Image = pDisUnit;
		// 	}
		// }

		return Image;
	}

	return GET_TECHNOTYPE(pThis);
}

void TechnoExtData::HandleTunnelLocoStuffs(FootClass* pOwner, bool DugIN, bool PlayAnim)
{
	const auto pExt = GET_TECHNOTYPEEXT(pOwner);
	const auto pRules = RulesClass::Instance();
	const auto nSound = (DugIN ? pExt->DigInSound : pExt->DigOutSound).Get(pRules->DigSound);

	VocClass::SafeImmedietelyPlayAt(nSound, &pOwner->Location);

	if (PlayAnim)
	{
		if (const auto pAnimType = (DugIN ? pExt->DigInAnim : pExt->DigOutAnim).Get(pRules->Dig))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pOwner->Location),
				pOwner->Owner,
				nullptr,
				false
			);
		}
	}
}


bool TechnoExtData::IsSameTrech(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	auto pThisTypeExt = BuildingTypeExtContainer::Instance.Find(currentBuilding->Type);
	if (pThisTypeExt->IsTrench <= 0)
	{
		return false;
	}

	return pThisTypeExt->IsTrench == BuildingTypeExtContainer::Instance.Find(targetBuilding->Type)->IsTrench;
}

bool TechnoExtData::canTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	if (targetBuilding != currentBuilding)
	{
		BuildingTypeClass* pTargetType = targetBuilding->Type;
		if (pTargetType->CanBeOccupied && targetBuilding->Occupants.Count < pTargetType->MaxNumberOccupants)
		{
			if (currentBuilding->Occupants.Count && IsSameTrech(currentBuilding, targetBuilding))
			{
				if (targetBuilding->Location.DistanceFrom(currentBuilding->Location) <= 256.0)
					return true;
			}
		}
	}

	return false;
}

#include <Ext/Infantry/Body.h>

void TechnoExtData::doTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	// depending on Westwood's handling, this could explode when Size > 1 units are involved...but don't tell the users that
	while (currentBuilding->Occupants.Count && (targetBuilding->Occupants.Count < targetBuildingType->MaxNumberOccupants))
	{
		auto item = currentBuilding->Occupants.Items[0];
		targetBuilding->Occupants.push_back(item);
		InfantryExtContainer::Instance.Find(item)->GarrisonedIn = targetBuilding;
		currentBuilding->Occupants.erase(item); // maybe switch Add/Remove if the game gets pissy about multiple of them walking around
	}

	// fix up firing index, as decrementing the source occupants can invalidate it
	if (currentBuilding->FiringOccupantIndex >= currentBuilding->GetOccupantCount())
	{
		currentBuilding->FiringOccupantIndex = 0;
	}

	//const auto oldtgt = currentBuilding->Target;
	//currentBuilding->SetTarget(nullptr);
	//targetBuilding->SetTarget(oldtgt);
	TechnoExtData::EvalRaidStatus(currentBuilding); // if the traversal emptied the current building, it'll have to be returned to its owner
}

#include <ExtraHeaders/StackVector.h>

bool TechnoExtData::AcquireHunterSeekerTarget(TechnoClass* pThis)
{

	if (!pThis->Target)
	{
		StackVector<TechnoClass*, 256> preferredTargets {};
		StackVector<TechnoClass*, 256> randomTargets {};

		// defaults if SW isn't set
		auto pOwner = pThis->GetOwningHouse();
		SWTypeExtData* pSWExt = nullptr;
		auto canPrefer = true;

		// check the hunter seeker SW
		if (auto const pSuper =
			TechnoExtContainer::Instance.Find(pThis)->LinkedSW
			)
		{
			pOwner = pSuper->Owner;
			pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
			canPrefer = !pSWExt->HunterSeeker_RandomOnly;
		}

		auto const isHumanControlled = pOwner->IsControlledByHuman();
		auto const mode = SessionClass::Instance->GameMode;

		// the AI in multiplayer games only attacks its favourite enemy
		auto const pFavouriteEnemy = HouseClass::Array->get_or_default(pOwner->EnemyHouseIndex);
		auto const favouriteEnemyOnly = (mode != GameMode::Campaign
			&& pFavouriteEnemy && !isHumanControlled);

		for (auto i : *TechnoClass::Array)
		{
			// techno ineligible
			if (i->Health < 0 || i->InLimbo || !i->IsAlive || i->IsCrashing || i->IsSinking)
				continue;

			if (!i->Location.IsValid() || !i->InlineMapCoords().IsValid())
				continue;

			if (i->IsIronCurtained())
				continue;

			const auto what = i->WhatAmI();

			if (what == BuildingClass::AbsID)
			{
				auto pBuilding = static_cast<BuildingClass*>(i);
				const auto pExt = BuildingExtContainer::Instance.Find(pBuilding);

				if (pExt->LimboID >= 0 || pBuilding->Type->InvisibleInGame)
					continue;
			}
			else
			{
				if (what == UnitClass::AbsID)
				{
					if (((UnitClass*)i)->DeathFrameCounter > 0)
						continue;

				}
			}

			// type prevents this being a target
			auto const pType = GET_TECHNOTYPE(i);

			// is type to be ignored?
			auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pType->Invisible
			|| (pExt->AI_LegalTarget.isset() && !isHumanControlled && !pExt->AI_LegalTarget.Get())
			|| !pType->LegalTarget
			|| pExt->HunterSeekerIgnore
			)
			{
				continue;
			}

			// is the house ok?
			if (favouriteEnemyOnly)
			{
				if (i->Owner != pFavouriteEnemy)
				{
					continue;
				}
			}
			else if (!pSWExt && pOwner->IsAlliedWith(i->Owner))
			{
				// default without SW
				continue;
			}
			else if (pSWExt && !pSWExt->IsHouseAffected(pOwner, i->Owner))
			{
				// use SW
				continue;
			}

			// harvester truce
			if (ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
				&& what == UnitClass::AbsID)
			{
				if (RulesClass::Instance->HarvesterUnit.contains(((UnitTypeClass*)pType)))
				{
					continue;
				}
			}

			// allow to exclude certain techno types
			if (pSWExt && !pSWExt->IsTechnoAffected(i))
			{
				continue;
			}

			// in multiplayer games, non-civilian targets are preferred
			// for human players
			auto const isPreferred = mode != GameMode::Campaign && isHumanControlled
				&& !i->Owner->Type->MultiplayPassive && canPrefer;

			// add to the right list
			if (isPreferred)
			{
				preferredTargets->push_back(i);
			}
			else
			{
				randomTargets->push_back(i);
			}
		}

		auto const targets = &(preferredTargets->size() > 0 ? preferredTargets : randomTargets);

		if (auto const count = (*targets)->size())
		{
			// that's our target
			pThis->SetTarget
			(*((*targets)->data() + (size_t(count == 1 ?
				0 : ScenarioClass::Instance->Random.RandomFromMax(count - 1)))
				));
			return true;
		}
	}

	return false;
}

void TechnoExtData::UpdateAlphaShape(ObjectClass* pSource)
{
	if (!pSource || !pSource->IsAlive)
		return;

	ObjectTypeClass* pSourceType = pSource->GetType();

	if (!pSourceType)
	{
		return;
	}

	const SHPStruct* pImage = pSourceType->AlphaImage;
	if (!pImage)
	{
		return;
	}

	const auto what = pSource->WhatAmI();
	ObjectClass* pOwner = pSource;

	if (what == AnimClass::AbsID)
	{
		const auto pAnim = (AnimClass*)pSource;
		if (pAnim->OwnerObject)
		{
			pOwner = pAnim->OwnerObject;
		}
	}

	Point2D off { (pImage->Width + 1) / -2, (pImage->Height + 1) / -2 };

	if (pOwner && (pOwner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = (FootClass*)pOwner;

		if (pFoot->CurrentMapCoords != pFoot->LastMapCoords)
		{

			CoordStruct XYZ = CellClass::Cell2Coord(pFoot->LastMapCoords);
			Point2D xyTL = TacticalClass::Instance->CoordsToClient(XYZ);

			TacticalClass::Instance->RegisterDirtyArea({
				off.X - 30 + xyTL.X ,
				xyTL.Y - 60 + off.Y ,
				pImage->Width + 60 ,
				pImage->Width + 120
			}, true);
		}
	}

	CoordStruct XYZ = pSource->GetCoords();
	Point2D xyTL = TacticalClass::Instance->CoordsToClient(XYZ);

	ObjectTypeClass* pDisguise = nullptr;

	if (pSource->InLimbo || ((pSource->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
		&& (((TechnoClass*)pSource)->Deactivated
			|| ((TechnoClass*)pSource)->CloakState == CloakState::Cloaked
			|| pSource->GetHeight() < -10
			|| pSource->IsDisguised() && (pDisguise = pSource->GetDisguise(true)) && pDisguise->WhatAmI() == AbstractType::TerrainType
			|| what == BuildingClass::AbsID && (pSource->GetCurrentMission() != Mission::Construction && !((BuildingClass*)pSource)->IsPowerOnline()
				|| BuildingExtContainer::Instance.Find(((BuildingClass*)pSource))->LimboID >= 0)
			)
	)
	{
		if (auto pAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource))
			GameDelete<true, false>(std::exchange(pAlpha, nullptr));

		return;
	}

	if (Unsorted::CurrentFrame % 2)
	{
		if (PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource)
			&& what == BuildingClass::AbsID
			&& (pImage->Frames <= 1 || !((BuildingClass*)pSource)->HasTurret()
				))
			return;

		RectangleStruct ScreenArea = TacticalClass::Instance->VisibleArea();
		++Unsorted::ScenarioInit;
		GameCreate<AlphaShapeClass>(pSource,
		(xyTL.X + off.X + ScreenArea.X),
		(xyTL.Y + off.Y + ScreenArea.Y));
		--Unsorted::ScenarioInit;
		TacticalClass::Instance->RegisterDirtyArea({
		xyTL.X + off.X,
		xyTL.Y + off.Y,
		pImage->Width,
		pImage->Height },
		true);
	}
}

int TechnoExtData::GetAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	for (int i = pExt->Ammo; i > 0; --i)
		pThis->DecreaseAmmo();

	return pExt->Ammo;
}

void TechnoExtData::DecreaseAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (GetAmmo(pThis, pWeapon) > 0)
	{
		if (pThis->WhatAmI() != AircraftClass::AbsID)
		{
			if (pTypeExt->NoAmmoWeapon > -1 && pTypeExt->NoAmmoEffectAnim)
			{
				const auto pCurWeapon = pThis->GetWeapon(pTypeExt->NoAmmoWeapon);
				if (pThis->Ammo <= pTypeExt->NoAmmoAmount && pCurWeapon->WeaponType != pWeapon)
				{
					auto pAnim = GameCreate<AnimClass>(pTypeExt->NoAmmoEffectAnim.Get(), pThis->Location);
					pAnim->SetOwnerObject(pThis);
					pAnim->SetHouse(pThis->Owner);

				}
			}
		}

		if (pThis->WhatAmI() == BuildingClass::AbsID)
		{
			const auto Ammo = reinterpret_cast<BuildingClass*>(pThis)->Type->Ammo;
			if (Ammo > 0 && pThis->Ammo < Ammo)
				pThis->StartReloading();
		}
	}
}

AnimClass* TechnoExtData::SpawnAnim(CoordStruct& crd, AnimTypeClass* pType, int dist)
{
	if (!pType)
	{
		return nullptr;
	}

	CoordStruct crdAnim = crd;

	if (dist > 0)
	{
		const auto crdNear = MapClass::GetRandomCoordsNear(crd, dist, false);
		crdAnim = MapClass::PickInfantrySublocation(crdNear, true);
	}

	const auto count = ScenarioClass::Instance->Random.RandomRanged(1, 2);
	return GameCreate<AnimClass>(pType, crdAnim, 0, count, 0x600u, 0, false);
}

void TechnoExtData::PlantBomb(TechnoClass* pSource, ObjectClass* pTarget, WeaponTypeClass* pWeapon)
{
	// ensure target isn't rigged already
	if (pTarget && !pTarget->AttachedBomb)
	{
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
		const auto pTechno = flag_cast_to <TechnoClass*, false>(pTarget);

		//https://bugs.launchpad.net/ares/+bug/1591335
		if (pTechno && !pWHExt->CanDealDamage(pTechno, false, false, false))
			return;

		BombListClass::Instance->Plant(pSource, pTarget);

		// if target has a bomb, planting was successful
		if (auto pBomb = pTarget->AttachedBomb)
		{
			const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			BombExtContainer::Instance.Find(pBomb)->Weapon = pWeaponExt;
			pBomb->DetonationFrame = Unsorted::CurrentFrame + pWeaponExt->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			pBomb->TickSound = pWeaponExt->Ivan_TickingSound.Get(RulesClass::Instance->BombTickingSound);

			const auto IsAlly = pSource->Owner && pSource->Owner->IsAlliedWith(pTarget);

			pBomb->Type = BombType((!IsAlly && pWeaponExt->Ivan_DeathBomb) || (IsAlly && pWeaponExt->Ivan_DeathBombOnAllies));

			if (pSource->Owner && pSource->Owner->ControlledByCurrentPlayer())
			{
				VocClass::SafeImmedietelyPlayAt(pWeaponExt->Ivan_AttachSound.Get(RulesClass::Instance->BombAttachSound)
				, &pBomb->Target->Location);
			}
		}
	}
}

bool TechnoExtData::CanDetonate(TechnoClass* pThis, ObjectClass* pThat)
{
	if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1)
	{
		if (const auto pBomb = pThis->AttachedBomb)
		{
			if (!pBomb->OwnerHouse)
				return false;

			if (pBomb->OwnerHouse->ControlledByCurrentPlayer())
			{
				const auto pData = BombExtContainer::Instance.Find(pBomb);
				const bool bCanDetonateDeathBomb =
					pData->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);
				const bool bCanDetonateTimeBomb =
					pData->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb);

				if (pBomb->Type == BombType::DeathBomb ?
					bCanDetonateDeathBomb : bCanDetonateTimeBomb)
					return true;
			}
		}
	}

	return false;
}

Action TechnoExtData::GetAction(TechnoClass* pThis, ObjectClass* pThat)
{
	if (!pThat)
		return Action::None;

	if (TechnoExtData::CanDetonate(pThis, pThat))
		return Action::Detonate;

	if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1)
	{
		if (pThat->AbstractFlags & AbstractFlags::Techno)
		{
			if (pThis->Owner && pThis->Owner->IsAlliedWith(pThat) && pThat->IsSelectable())
			{
				return Action::Select;
			}
		}
	}

	return Action::None;
}

int TechnoExtData::GetFirstSuperWeaponIndex(BuildingClass* pThis)
{
	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);
		if (idxSW != -1)
		{
			return idxSW;
		}
	}
	return -1;
}

void TechnoExtData::UpdateDisplayTo(BuildingClass* pThis)
{
	if (pThis->Type->Radar)
	{
		auto pHouse = pThis->Owner;
		DWORD presistData = HouseExtContainer::Instance.Find(pHouse)->RadarPersist.data;

		for (auto walk = pHouse->Buildings.begin(); walk != pHouse->Buildings.end(); ++walk)
		{
			if (!(*walk)->InLimbo)
			{
				if (BuildingTypeExtContainer::Instance.Find((*walk)->Type)->SpyEffect_RevealRadar)
				{
					presistData |= (*walk)->DisplayProductionTo.data;
				}
			}
		}

		//TODO RadarVisible
		pHouse->RadarVisibleTo.data = presistData;
		MapClass::Instance->RedrawSidebar(2);
	}
}

void TechnoExtData::InfiltratedBy(BuildingClass* EnteredBuilding, HouseClass* Enterer)
{
	auto EnteredType = EnteredBuilding->Type;
	auto Owner = EnteredBuilding->Owner;
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(EnteredBuilding->Type);
	auto pBldExt = BuildingExtContainer::Instance.Find(EnteredBuilding);
	static COMPILETIMEEVAL reference<bool, 0x884B8E> tootip_something {};

	bool raiseEva = false;
	const bool IsOwnerControlledByCurrentPlayer = Owner->ControlledByCurrentPlayer();
	const bool IsEntererControlledByCurrentPlayer = Enterer->ControlledByCurrentPlayer();

	if (IsEntererControlledByCurrentPlayer || IsOwnerControlledByCurrentPlayer)
	{
		CellStruct xy = CellClass::Coord2Cell(EnteredBuilding->GetCoords());
		if (RadarEventClass::Create(RadarEventType::BuildingInfiltrated, xy))
		{
			raiseEva = true;
		}
	}

	const bool evaForOwner = IsOwnerControlledByCurrentPlayer && raiseEva;
	const bool evaForEnterer = IsEntererControlledByCurrentPlayer && raiseEva;
	auto pEntererExt = HouseExtContainer::Instance.Find(Enterer);
	bool effectApplied = false;
	bool promotionStolen = false;
	int moneyBefore = Owner->Available_Money();

	if (!pTypeExt->SpyEffect_Custom)
	{
		if (EnteredType->Radar)
		{
			Owner->ReshroudMap();
			if (!Owner->SpySatActive && evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_RadarSabotaged);
			}
			if (!Owner->SpySatActive && evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfRadarSabotaged);
			}
			effectApplied = true;
		}
		else if (EnteredType->PowerBonus > 0)
		{
			Owner->CreatePowerOutage(RulesClass::Instance->SpyPowerBlackout);
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_PowerSabotaged);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_EnemyBasePoweredDown);
			}
			effectApplied = true;
		}
		else if (!RulesClass::Instance->BuildTech.contains(EnteredType))
		{
			if (EnteredType->SuperWeapon != -1)
			{

				if (auto pSuper = Owner->Supers[EnteredType->SuperWeapon])
				{
					pSuper->Reset();
					if (evaForOwner || evaForEnterer)
					{
						VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
					}
					effectApplied = true;
				}
			}
			else if (EnteredType->Storage > 0 && !EnteredType->Weeder)
			{

				int available = Owner->Available_Money();
				float mult = RulesClass::Instance->SpyMoneyStealPercent;
				auto const& nAIMult = RulesExtData::Instance()->AI_SpyMoneyStealPercent;

				if (!Owner->IsControlledByHuman() && nAIMult.isset())
				{
					mult = nAIMult.Get();
				}
				int bounty = int(mult * mult);

				if (bounty > 0)
				{
					bounty = MinImpl(bounty, available);
					Owner->TakeMoney(bounty);
					Enterer->GiveMoney(bounty);
					if (evaForOwner)
					{
						VoxClass::Play(GameStrings::EVA_CashStolen);
					}

					if (evaForEnterer)
					{
						VoxClass::Play(GameStrings::EVA_BuildingInfCashStolen);
					}
					effectApplied = true;
				}
			}
			else if (EnteredType->Factory != AbstractType::None)
			{
				switch (EnteredType->Factory)
				{
				case UnitTypeClass::AbsID:
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
					break;
				case InfantryTypeClass::AbsID:
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			switch (EnteredType->AIBasePlanningSide)
			{
			case 0:
				Enterer->Side0TechInfiltrated = true;
				promotionStolen = true;
				break;
			case 1:
				Enterer->Side1TechInfiltrated = true;
				promotionStolen = true;
				break;
			case 2:
				Enterer->Side2TechInfiltrated = true;
				promotionStolen = true;
				break;
			default:
				break;
			}
		}

	}
	else
	{
		if (pTypeExt->SpyEffect_ResetRadar)
		{
			Owner->ReshroudMap();
			if (!Owner->SpySatActive && evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_RadarSabotaged);
			}
			if (!Owner->SpySatActive && evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfRadarSabotaged);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_PowerOutageDuration > 0)
		{
			Owner->CreatePowerOutage(pTypeExt->SpyEffect_PowerOutageDuration);
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_PowerSabotaged);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_EnemyBasePoweredDown);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_StolenTechIndex_result.any())
		{
			HouseExtContainer::Instance.Find(Enterer)->StolenTech |= pTypeExt->SpyEffect_StolenTechIndex_result;
			Enterer->RecheckTechTree = true;
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_TechnologyStolen);
			}

			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_NewTechAcquired);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_UnReverseEngineer)
		{
			Debug::LogInfo("Undoing all Reverse Engineering achieved by house {}", Owner->Type->ID);
			HouseExtContainer::Instance.Find(Owner)->Reversed.clear();
			Owner->RecheckTechTree = true;

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_ResetSW)
		{
			bool somethingReset = false;
			for (auto types : EnteredBuilding->GetTypes())
			{
				if (auto typeExt = BuildingTypeExtContainer::Instance.TryFind(types))
				{
					for (auto i = 0; i < typeExt->GetSuperWeaponCount(); ++i)
					{
						if (auto pSuper = typeExt->GetSuperWeaponByIndex(i, Owner))
						{
							pSuper->Reset();
							somethingReset = true;
						}
					}
				}
			}

			if (somethingReset)
			{
				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}
				effectApplied = true;
			}
		}

		// Did you mean for not launching for real or not, Morton?
		auto launchTheSWHere = [EnteredBuilding, pTypeExt](int const idx, HouseClass* const pHouse, bool realLaunch = false)
			{
				if (const auto pSuper = pHouse->Supers.get_or_default(idx))
				{
					if (!realLaunch || (pSuper->Granted && pSuper->IsCharged && !pSuper->IsOnHold))
					{
						const int oldstart = pSuper->RechargeTimer.StartTime;
						const int oldleft = pSuper->RechargeTimer.TimeLeft;
						pSuper->SetReadiness(true);
						CoordStruct loc = pTypeExt->SpyEffect_SWTargetCenter.Get() ? EnteredBuilding->GetCenterCoords() : EnteredBuilding->Location;
						pSuper->Launch(CellClass::Coord2Cell(loc), pHouse->IsCurrentPlayer());
						pSuper->Reset();
						if (!realLaunch)
						{
							pSuper->RechargeTimer.StartTime = oldstart;
							pSuper->RechargeTimer.TimeLeft = oldleft;
						}
					}
				}
			};

		auto justGrantTheSW = [](int const idx, HouseClass* const pHouse)
			{
				if (const auto pSuper = pHouse->Supers.get_or_default(idx))
				{
					if (pSuper->Granted)
						pSuper->SetCharge(100);
					else
					{
						pSuper->Grant(true, false, false);
						if (pHouse->IsCurrentPlayer())
							SidebarClass::Instance->AddCameo(AbstractType::Special, idx);
					}
					SidebarClass::Instance->RepaintSidebar(1);
				}
			};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset() && !Owner->IsNeutral())
		{
			launchTheSWHere(pTypeExt->SpyEffect_VictimSuperWeapon.Get(), Owner, pTypeExt->SpyEffect_VictimSW_RealLaunch.Get());

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			const int swidx = pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get();

			if (pTypeExt->SpyEffect_InfiltratorSW_JustGrant.Get())
				justGrantTheSW(swidx, Enterer);
			else
				launchTheSWHere(swidx, Enterer);

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (auto pSuperType = pTypeExt->SpyEffect_SuperWeapon)
		{
			const auto nIdx = pSuperType->ArrayIndex;
			const auto pSuper = Enterer->Supers.Items[nIdx];
			const bool Onetime = !pTypeExt->SpyEffect_SuperWeaponPermanent;
			bool CanLauch = true;

			if (!pSuperType->IsPowered || Enterer->PowerDrain == 0 || Enterer->PowerOutput >= Enterer->PowerDrain)
				CanLauch = false;

			const bool IsCurrentPlayer = Enterer->IsCurrentPlayer();

			if (pSuper->Grant(Onetime, IsCurrentPlayer, CanLauch))
			{
				if (pTypeExt->SpyEffect_SuperWeaponPermanent)
					pSuper->CanHold = false;

				if (IsCurrentPlayer)
				{
					SidebarClass::Instance->AddCameo(AbstractType::Special, nIdx);
					const auto nTab = SidebarClass::GetObjectTabIdx(AbstractType::Special, nIdx, false);
					SidebarClass::Instance->RepaintSidebar(nTab);
				}

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		if (pTypeExt->SpyEffect_SabotageDelay > 0)
		{
			const int nDelay = int(pTypeExt->SpyEffect_SabotageDelay * 900.0);

			if (nDelay >= 0 && !EnteredBuilding->IsGoingToBlow)
			{
				EnteredBuilding->IsGoingToBlow = true;
				EnteredBuilding->GoingToBlowTimer.Start(nDelay);
				EnteredBuilding->Flash(nDelay / 2);

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		{
			int bounty = 0;
			int available = Owner->Available_Money();
			if (pTypeExt->SpyEffect_StolenMoneyAmount > 0)
			{
				bounty = pTypeExt->SpyEffect_StolenMoneyAmount;
			}
			else if (pTypeExt->SpyEffect_StolenMoneyPercentage > 0.0f)
			{
				bounty = int(available * pTypeExt->SpyEffect_StolenMoneyPercentage);
			}

			if (bounty > 0)
			{
				bounty = MinImpl(bounty, available);
				Owner->TakeMoney(bounty);
				Enterer->GiveMoney(bounty);
				if (evaForOwner)
				{
					VoxClass::Play(GameStrings::EVA_CashStolen);
				}
				if (evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfCashStolen);
				}

				effectApplied = true;
			}
		}

		{
			if (pTypeExt->SpyEffect_GainVeterancy)
			{
				switch (EnteredType->Factory)
				{
				case UnitTypeClass::AbsID:
					if (!EnteredType->Naval)
						Enterer->WarFactoryInfiltrated = true;
					else
						pEntererExt->Is_NavalYardSpied = true;

					promotionStolen = true;
					break;
				case InfantryTypeClass::AbsID:
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
					break;
				case AircraftTypeClass::AbsID:
					pEntererExt->Is_AirfieldSpied = true;
					promotionStolen = true;
					break;
				case BuildingTypeClass::AbsID:
					pEntererExt->Is_ConstructionYardSpied = true;
					promotionStolen = true;
					break;
				default:
					break;
				}
			}
			else
			{
				if (pTypeExt->SpyEffect_AircraftVeterancy)
				{
					pEntererExt->Is_AirfieldSpied = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_InfantryVeterancy)
				{
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_NavalVeterancy)
				{
					pEntererExt->Is_NavalYardSpied = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_VehicleVeterancy)
				{
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_BuildingVeterancy)
				{
					pEntererExt->Is_ConstructionYardSpied = true;
					promotionStolen = true;
				}
			}
		}

		/*	RA1-Style Spying, as requested in issue #633
			This sets the respective bit to inform the game that a particular house has spied this building.
			Knowing that, the game will reveal the current production in this building to the players who have spied it.
			In practice, this means: If a player who has spied a factory clicks on that factory,
			he will see the cameo of whatever is being built in the factory.

			Addition 04.03.10: People complained about it not being optional. Now it is.
		*/
		if (pTypeExt->SpyEffect_RevealProduction)
		{
			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_RevealRadar)
		{
			/*	Remember the new persisting radar spy effect on the victim house itself, because
				destroying the building would destroy the spy reveal info in the ExtData, too.
				2013-08-12 AlexB
			*/
			if (pTypeExt->SpyEffect_RevealRadarPersist)
			{
				HouseExtContainer::Instance.Find(Owner)->RadarPersist.Add(Enterer);
			}

			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			TechnoExtData::UpdateDisplayTo(EnteredBuilding);

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			MapClass::Instance->Map_AI();
			MapClass::Instance->RedrawSidebar(2);
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_SellDelay.isset())
		{

			if (!pBldExt->AutoSellTimer.HasStarted())
			{
				pBldExt->AutoSellTimer.Start(pTypeExt->SpyEffect_SellDelay > 0 ?
					pTypeExt->SpyEffect_SellDelay : static_cast<int>(RulesClass::Instance->C4Delay));
			}

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}
			effectApplied = true;
		}
	}
	if (pTypeExt->SpyEffect_Anim && pTypeExt->SpyEffect_Anim_Duration > 0)
	{

		pBldExt->SpyEffectAnim.reset(GameCreate<AnimClass>(pTypeExt->SpyEffect_Anim, EnteredBuilding->GetCoords()));
		pBldExt->SpyEffectAnim->SetOwnerObject(EnteredBuilding);
		pBldExt->SpyEffectAnim->RemainingIterations = 0xFFU;
		pBldExt->SpyEffectAnim->Owner = EnteredBuilding->Owner;

		pBldExt->SpyEffectAnimDuration = pTypeExt->SpyEffect_Anim_Duration;
		effectApplied = true;
	}

	if (promotionStolen)
	{
		Enterer->RecheckTechTree = true;
		if (IsEntererControlledByCurrentPlayer)
		{
			MouseClass::Instance->SidebarNeedsRepaint();
			tootip_something = true;
		}

		if (evaForOwner)
		{
			VoxClass::Play(GameStrings::EVA_TechnologyStolen);
		}

		if (evaForEnterer)
		{
			VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			VoxClass::Play(GameStrings::EVA_NewTechAcquired);
		}
		effectApplied = true;
	}

	if (effectApplied)
	{
		EnteredBuilding->Mark(MarkType::Redraw);
	}

	pBldExt->AccumulatedIncome += Owner->Available_Money() - moneyBefore;

	if (!Owner->IsControlledByHuman() && !RulesExtData::Instance()->DisplayIncome_AllowAI)
	{
		CoordStruct coord {};
		EnteredBuilding->GetRenderCoords(&coord);
		FlyingStrings::Instance.AddMoneyString(true,
				pBldExt->AccumulatedIncome,
				EnteredBuilding,
				pTypeExt->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses.Get()),
				coord,
				pTypeExt->DisplayIncome_Offset,
				ColorStruct::Empty);
		pBldExt->AccumulatedIncome = 0;
	}
}

DirStruct TechnoExtData::UnloadFacing(UnitClass* pThis)
{
	DirStruct nResult;
	nResult.Raw = 0x4000;

	if (pThis->HasAnyLink())
	{
		if (const auto pBld = cast_to<BuildingClass*>(pThis->RadioLinks.Items[0]))
		{
			auto const pBldExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
			if (pBldExt->DockUnload_Facing.isset())
				nResult.Raw = ((size_t)pBldExt->DockUnload_Facing.Get()) << 11;
		}
	}

	return nResult;
}

CellStruct TechnoExtData::UnloadCell(BuildingClass* pThis)
{
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->DockUnload_Cell;
}

BuildingClass* TechnoExtData::BuildingUnload(UnitClass* pThis)
{
	if (const auto pBld = cast_to<BuildingClass*>(pThis->RadioLinks.Items[0]))
	{
		const auto pBldCells = pBld->InlineMapCoords();
		const auto pThisCells = pThis->InlineMapCoords();

		if ((pBldCells + UnloadCell(pBld)) == pThisCells)
		{
			return pBld;
		}
	}

	return nullptr;
}

void TechnoExtData::KickOutHospitalArmory(BuildingClass* pThis)
{
	if (pThis->Type->Hospital || pThis->Type->Armory)
	{
		if (FootClass* Passenger = pThis->Passengers.RemoveFirstPassenger())
		{
			pThis->KickOutUnit(Passenger, CellStruct::Empty);
		}
	}
}


void TechnoExtData::KickOutOfRubble(BuildingClass* pBld)
{
	std::vector<std::pair<FootClass*, bool>> KickList;

	auto const location = MapClass::Instance->GetCellAt(pBld->Location)->MapCoords;
	// get the number of non-end-marker cells and a pointer to the cell data
	for (auto i = pBld->Type->FoundationData; *i != CellStruct::EOL; ++i)
	{
		// remove every techno that resides on this cell
		for (NextObject obj(MapClass::Instance->GetCellAt(location + *i)->
			GetContent()); obj; ++obj)
		{
			if (auto const pFoot = flag_cast_to<FootClass*>(*obj))
			{
				if (pFoot->Limbo())
				{
					KickList.emplace_back(pFoot, pFoot->IsSelected);
				}
			}
		}
	}

	// this part kicks out all units we found in the rubble
	for (auto const& [pFoot, bIsSelected] : KickList)
	{
		if (pBld->KickOutUnit(pFoot, location) == KickOutResult::Succeeded)
		{
			if (bIsSelected)
			{
				pFoot->Select();
			}
		}
		else
		{
			pFoot->UnInit();
		}
	}
}

void TechnoExtData::UpdateSensorArray(BuildingClass* pBld)
{
	if (pBld->Type->SensorArray)
	{
		bool isActive = !pBld->Deactivated && pBld->IsPowerOnline();
		bool wasActive = (BuildingExtContainer::Instance.Find(pBld)->SensorArrayActiveCounter > 0);

		if (isActive != wasActive)
		{
			if (isActive)
			{
				pBld->SensorArrayActivate();
			}
			else
			{
				pBld->SensorArrayDeactivate();
			}
		}
	}
}

BuildingClass* TechnoExtData::CreateBuilding(
	BuildingClass* pBuilding,
	bool remove,
	BuildingTypeClass* pNewType,
	OwnerHouseKind owner,
	int strength,
	AnimTypeClass* pAnimType
)
{
	pBuilding->Limbo(); // only takes it off the map
	pBuilding->DestroyNthAnim(BuildingAnimSlot::All);
	BuildingClass* pRet = nullptr;

	if (!remove)
	{
		HouseClass* designated =
			//pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count != 0 ? HouseExtData::FindFirstCivilianHouse() :
			pBuilding->Owner;

		auto pOwner = HouseExtData::GetHouseKind(owner, true, designated);
		pRet = static_cast<BuildingClass*>(pNewType->CreateObject(pOwner));

		if (strength <= -1 && strength >= -100)
		{
			// percentage of original health
			pRet->Health = MaxImpl((-strength * pNewType->Strength) / 100, 1);
		}
		else if (strength > 0)
		{
			pRet->Health = MinImpl(strength, pNewType->Strength);
		}

		const auto direction = pBuilding->PrimaryFacing.Current().GetDir();
		++Unsorted::ScenarioInit;
		const bool res = pRet->Unlimbo(pBuilding->Location, direction);
		--Unsorted::ScenarioInit;

		if (!res)
		{
			//Debug::LogInfo(__FUNCTION__" Called ");
			TechnoExtData::HandleRemove(pRet, nullptr, true, false);
			pRet = nullptr;
		}
	}

	if (pAnimType)
	{
		GameCreate<AnimClass>(pAnimType, pBuilding->GetCoords())->Owner = pBuilding->Owner;
	}

	return pRet;
};

void TechnoExtData::Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead)
{
	if (!pKillerHouse && pKiller)
	{
		pKillerHouse = pKiller->Owner;
	}

	if (!pWarhead)
	{
		pWarhead = RulesClass::Instance->C4Warhead;
	}

	int health = pTechno->Health;

	if (!pTechno->IsAlive || health <= 0 || pTechno->IsSinking || pTechno->IsCrashing)
		return;

	if (auto pTemp = pTechno->TemporalTargetingMe)
	{
		pTemp->JustLetGo();
	}

	pTechno->ReceiveDamage(&health, 0, pWarhead, pKiller, true, false, pKillerHouse);
}

bool TechnoExtData::IsDriverKillable(TechnoClass* pThis, double KillBelowPercent)
{
	const auto what = pThis->WhatAmI();
	if (what != UnitClass::AbsID && what != AircraftClass::AbsID)
		return false;

	if (what == AircraftClass::AbsID)
	{
		const auto pAir = (AircraftClass*)pThis;

		if (pAir->Type->AirportBound || pAir->Type->Dock.Count)
			return false;
	}

	if (pThis->BeingWarpedOut || pThis->IsIronCurtained() || TechnoExtData::IsInWarfactory(pThis, false))
		return false;

	const auto pType = GET_TECHNOTYPE(pThis);

	if (pType->Natural || pType->Organic)
		return false;

	const auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const bool protecteddriver = TechnoExtData::IsDriverKillProtected(pThis);

	const double maxKillHealth = MinImpl(
		pThisTypeExt->ProtectedDriver_MinHealth.Get(
			protecteddriver ? 0.0 : 1.0),
		KillBelowPercent);

	if (pThis->GetHealthPercentage() > maxKillHealth)
		return false;

	return true;
}

void TechnoExtData::ApplyKillDriver(TechnoClass* pTarget, TechnoClass* pKiller, HouseClass* pToOwner, bool ResetVet, Mission passiveMission)
{
	if (!pTarget || (pTarget->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
		return;

	if (pTarget->Owner == pToOwner)
	{
		return;
	}

	auto pTargetFoot = static_cast<FootClass*>(pTarget);
	auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	auto passive = pToOwner->Type->MultiplayPassive;

	pTargetExt->Is_DriverKilled = passive;

	const auto pTypeExt = GET_TECHNOTYPEEXT(pTarget);

	auto& passengers = pTarget->Passengers;

	do
	{
		if (!passengers.GetFirstPassenger())
			break;

		if (pTypeExt->Operator_Any)
		{
			const auto pOperator = pTargetFoot->RemoveFirstPassenger();
			pOperator->RegisterDestruction(pKiller);
			pOperator->UnInit();
		}
		else if (!pTypeExt->Operators.empty())
		{
			for (NextObject passenger(passengers.GetFirstPassenger()); passenger; ++passenger)
			{
				if (!pTypeExt->Operators.Contains(passenger->GetTechnoType()))
					continue;

				auto pOperator = static_cast<FootClass*>(*passenger);
				passengers.RemovePassenger(pOperator);

				if (pTypeExt->This()->Gunner && !passengers.NumPassengers)
					pTargetFoot->RemoveGunner(pOperator);

				pOperator->RegisterDestruction(pKiller);
				pOperator->UnInit();
				break;
			}
		}

		if (passive && pTypeExt->DriverKilled_KeptPassengers)
			break;

		const bool kill = pTypeExt->DriverKilled_KillPassengers.Get(RulesExtData::Instance()->DriverKilled_KillPassengers);

		while (auto pPassenger = passengers.GetFirstPassenger())
		{
			const auto pNextPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject);
			passengers.RemovePassenger(pPassenger);

			if (pTypeExt->This()->Gunner && !passengers.NumPassengers)
				pTargetFoot->RemoveGunner(pPassenger);

			if (kill || !TechnoExtData::EjectRandomly(pPassenger, pTarget->Location, 128, false))
			{
				pPassenger->RegisterDestruction(pKiller);
				pPassenger->UnInit();
			}
			else if (pTypeExt->This()->OpenTopped)
			{
				pTarget->ExitedOpenTopped(pPassenger);
			}

			pPassenger = pNextPassenger;
		}
	}
	while (false);

	if (ResetVet)
		pTarget->Veterancy.Reset();

	pTarget->HijackerInfantryType = -1;

	// If this unit is driving under influence, we have to free it first
	if (auto const pController = pTarget->MindControlledBy)
	{
		if (auto const pCaptureManager = pController->CaptureManager)
		{
			pCaptureManager->FreeUnit(pTarget);
		}
	}

	pTarget->MindControlledByAUnit = false;
	pTarget->MindControlledByHouse = nullptr;

	// remove the mind-control ring anim
	if (pTarget->MindControlRingAnim)
	{
		pTarget->MindControlRingAnim->TimeToDie = true;
		pTarget->MindControlRingAnim->UnInit();
		pTarget->MindControlRingAnim = nullptr;
	}

	// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
	if (pTarget->CaptureManager)
	{
		pTarget->CaptureManager->FreeAll();
	}

	// This unit will be freed of its duties
	if (pTargetFoot->BelongsToATeam())
	{
			pTargetFoot->Team->LiberateMember(pTargetFoot);
	}
	
	// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
	if (auto const pSpawnManager = pTarget->SpawnManager)
	{
		pSpawnManager->KillNodes();
		pSpawnManager->ResetTarget();
	}

	if (auto const pSlaveManager = pTarget->SlaveManager)
	{
		pSlaveManager->Killed(pKiller);
		pSlaveManager->ZeroOutSlaves();
		pSlaveManager->Owner = pTarget;
		if (pToOwner->Type->MultiplayPassive)
		{
			pSlaveManager->SuspendWork();
		}
		else
		{
			pSlaveManager->ResumeWork();
		}
	}

	// Hand over to a different house
	pTarget->SetOwningHouse(pToOwner);

	if (pToOwner->Type->MultiplayPassive)
	{
		pTarget->QueueMission(passiveMission, true);
	}

	pTarget->SetTarget(nullptr);
	pTarget->SetDestination(nullptr, false);

	if (auto firstTag = pTarget->AttachedTag)
		firstTag->SpringEvent((TriggerEvent)AresTriggerEvents::DriverKilled_ByHouse, pTarget, CellStruct::Empty, false, pToOwner);

	if (pTarget->IsAlive)
	{
		if (auto pSecTag = pTarget->AttachedTag)
			pSecTag->SpringEvent((TriggerEvent)AresTriggerEvents::DriverKiller, pTarget, CellStruct::Empty, false, nullptr);
	}

}

std::pair<TechnoTypeClass*, AbstractType> NOINLINE GetOriginalType(TechnoClass* pThis, TechnoTypeClass* pToType)
{
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		return { (TechnoTypeClass*)(((InfantryClass*)pThis)->Type) , AbstractType::InfantryType };
	case AbstractType::Unit:
		return { (TechnoTypeClass*)(((UnitClass*)pThis)->Type), AbstractType::UnitType };
	case AbstractType::Aircraft:
		return { (TechnoTypeClass*)(((AircraftClass*)pThis)->Type), AbstractType::AircraftType };
	default:
		Debug::LogInfo("[{}] {} is not FootClass, conversion to [{}] not allowed", (void*)pThis, pThis->get_ID(), pToType->ID);
		return { nullptr, AbstractType::None };
	}
}

void NOINLINE SetType(TechnoClass* pThis, AbstractType rtti, TechnoTypeClass* pToType)
{
	switch (rtti)
	{
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		TechnoExtContainer::Instance.Find(pThis)->TypeExtData = TechnoTypeExtContainer::Instance.Find(pToType);
		((InfantryClass*)pThis)->Type = (InfantryTypeClass*)pToType;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		TechnoExtContainer::Instance.Find(pThis)->TypeExtData = TechnoTypeExtContainer::Instance.Find(pToType);
		((UnitClass*)pThis)->Type = (UnitTypeClass*)pToType;
		break;
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		TechnoExtContainer::Instance.Find(pThis)->TypeExtData = TechnoTypeExtContainer::Instance.Find(pToType);
		((AircraftClass*)pThis)->Type = (AircraftTypeClass*)pToType;
		break;
	default:
		break;
	}
}

#include <Locomotor/Cast.h>
#include <Kamikaze.h>

//TODO jammer , powered , more stuffs that shit itself when change type
void UpdateTypeData(TechnoClass* pThis, TechnoTypeClass* pOldType, TechnoTypeClass* pCurrentType)
{
	//auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto const pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);
	auto const pNewTypeExt = TechnoTypeExtContainer::Instance.Find(pCurrentType);
	auto const pOwner = pThis->Owner;
	auto& pSlaveManager = pThis->SlaveManager;
	auto& pSpawnManager = pThis->SpawnManager;
	auto& pCaptureManager = pThis->CaptureManager;
	auto& pTemporalImUsing = pThis->TemporalImUsing;
	auto& pAirstrike = pThis->Airstrike;

	// handle AutoFire
	if (pOldTypeExt->AutoFire && !pNewTypeExt->AutoFire)
		pThis->SetTarget(nullptr);

	// Remove from harvesters list if no longer a harvester.
	if (pOldTypeExt->Harvester_Counted && !pNewTypeExt->Harvester_Counted)
	{
		HouseExtContainer::Instance.Find(pOwner)->OwnedCountedHarvesters.erase(pThis);
	}

	// Powered by ststl-s、Fly-Star
	if (pCurrentType->Enslaves && pCurrentType->SlavesNumber > 0)
	{
		// SlaveManager does not exist or they have different slaves.
		if (!pSlaveManager || pSlaveManager->SlaveType != pCurrentType->Enslaves)
		{
			if (pSlaveManager)
			{
				// Slaves are not the same, so clear out.
				pSlaveManager->Killed(nullptr);
				GameDelete<true, false>(pSlaveManager);
				pSlaveManager = nullptr;
			}

			pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pCurrentType->Enslaves, pCurrentType->SlavesNumber, pCurrentType->SlaveRegenRate, pCurrentType->SlaveReloadRate);
		}
		else if (pSlaveManager->SlaveCount != pCurrentType->SlavesNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSlaveManager->SlaveCount < pCurrentType->SlavesNumber)
			{
				// There are too few slaves here. More are needed.
				const int count = pCurrentType->SlavesNumber - pSlaveManager->SlaveCount;

				for (int index = 0; index < count; index++)
				{
					if (auto pSlaveNode = GameCreate<SlaveControl>())
					{
						pSlaveNode->Slave = nullptr;
						pSlaveNode->State = SlaveControlStatus::Dead;
						pSlaveNode->RespawnTimer.Start(pCurrentType->SlaveRegenRate);
						pSlaveManager->SlaveNodes.push_back(pSlaveNode);
					}
				}
			}
			else
			{
				// Remove excess slaves
				for (int i = pSlaveManager->SlaveCount - 1; i >= pCurrentType->SlavesNumber; --i)
				{
					if (auto pSlaveNode = pSlaveManager->SlaveNodes.operator[](i))
					{
						if (const auto pSlave = pSlaveNode->Slave)
						{
							if (pSlave->InLimbo)
							{
								// He wasn't killed, just erased.
								pSlave->RegisterDestruction(pThis);
								pSlave->UnInit();
							}
							else
							{
								// Oh, my God, he's been killed.
								pSlave->ReceiveDamage(&pSlave->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
							}
						}

						// Unlink
						pSlaveNode->Slave = nullptr;
						pSlaveNode->State = SlaveControlStatus::Dead;
						GameDelete<false, false>(pSlaveNode);
					}
				}

				pSlaveManager->SlaveNodes.clear();
			}

			pSlaveManager->SlaveCount = pCurrentType->SlavesNumber;
		}
	}
	else if (pSlaveManager)
	{
		pSlaveManager->Killed(nullptr);
		GameDelete<true, false>(pSlaveManager);
		pSlaveManager = nullptr;
	}

	if (pCurrentType->Spawns && pCurrentType->SpawnsNumber > 0)
	{
		// No SpawnManager exists, or their SpawnType is inconsistent.
		if (!pSpawnManager || pCurrentType->Spawns != pSpawnManager->SpawnType)
		{
			if (pSpawnManager)
			{
				// It may be odd that AircraftType is different, I chose to reset it.
				pSpawnManager->KillNodes();
				GameDelete<true, false>(pSpawnManager);
			}

			pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pCurrentType->Spawns, pCurrentType->SpawnsNumber, pCurrentType->SpawnRegenRate, pCurrentType->SpawnReloadRate);
		}
		else if (pSpawnManager->SpawnCount != pCurrentType->SpawnsNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSpawnManager->SpawnCount < pCurrentType->SpawnsNumber)
			{
				const int count = pCurrentType->SpawnsNumber - pSpawnManager->SpawnCount;
				struct SpawnNode_2
				{
					AircraftClass* Unit;		//ThisCan be anything Techno that not building ?
					SpawnNodeStatus Status;
					CDTimerClass NodeSpawnTimer;
					BOOL IsSpawnMissile;

					SpawnNode_2() = default;
					SpawnNode_2(AircraftClass* spwn, SpawnNodeStatus stat, int delay, bool missile) :
						Unit { spwn },
						Status { stat },
						NodeSpawnTimer { delay },
						IsSpawnMissile { missile }
					{}

					~SpawnNode_2() = default;
				};


				// Add the missing Spawns, but don't intend for them to be born right away.
				for (int index = 0; index < count; index++)
				{
					pSpawnManager->SpawnedNodes.push_back((SpawnNode*)GameCreate<SpawnNode_2>(nullptr, SpawnNodeStatus::Dead, pCurrentType->SpawnRegenRate, false));
				}
			}
			else
			{
				// Remove excess spawns
				for (int i = pSpawnManager->SpawnCount - 1; i >= pCurrentType->SpawnsNumber; --i)
				{
					if (auto pSpawnNode = pSpawnManager->SpawnedNodes.operator[](i))
					{
						auto& pStatus = pSpawnNode->Status;

						// Spawns that don't die get killed.
						if (const auto pAircraft = pSpawnNode->Unit)
						{
							pAircraft->SpawnOwner = nullptr;

							if (pAircraft->InLimbo || pStatus == SpawnNodeStatus::Idle ||
								pStatus == SpawnNodeStatus::Reloading || pStatus == SpawnNodeStatus::TakeOff)
							{
								if (pStatus == SpawnNodeStatus::TakeOff)
									Kamikaze::Instance->Remove(pAircraft);

								pAircraft->UnInit();
							}
							else if (pSpawnNode->IsSpawnMissile)
							{
								pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
							}
							else
							{
								pAircraft->Crash(nullptr);
							}
						}

						// Unlink
						pSpawnNode->Unit = nullptr;
						pStatus = SpawnNodeStatus::Dead;
						GameDelete<false, false>(pSpawnNode);
					}
				}

				pSpawnManager->SpawnedNodes.clear();
			}

			pSpawnManager->SpawnCount = pCurrentType->SpawnsNumber;
		}
	}
	else if (pSpawnManager)
	{
		// Reset the target.
		pSpawnManager->ResetTarget();

		// pSpawnManager->KillNodes() kills all Spawns, but it is not necessary to kill the parts that are not performing tasks.
		for (auto pSpawnNode : pSpawnManager->SpawnedNodes)
		{
			const auto pAircraft = pSpawnNode->Unit;
			auto& pStatus = pSpawnNode->Status;

			// A dead or idle Spawn is not killed.
			if (!pAircraft || pStatus == SpawnNodeStatus::Dead ||
				pStatus == SpawnNodeStatus::Idle || pStatus == SpawnNodeStatus::Reloading)
			{
				continue;
			}

			pAircraft->SpawnOwner = nullptr;

			if (pStatus == SpawnNodeStatus::TakeOff)
			{
				Kamikaze::Instance->Remove(pAircraft);
				pAircraft->UnInit();
			}
			else if (pSpawnNode->IsSpawnMissile)
			{
				pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
			}
			else
			{
				pAircraft->Crash(nullptr);
			}

			pSpawnNode->Unit = nullptr;
			pStatus = SpawnNodeStatus::Dead;
			pSpawnNode->IsSpawnMissile = false;
			pSpawnNode->NodeSpawnTimer.Start(pSpawnManager->RegenRate);
		}
	}

	// Prepare the variables.
	int maxCapture = 0;
	bool infiniteCapture = false;
	bool hasTemporal = false;
	bool hasAirstrike = false;
	bool hasLocomotor = false;
	bool hasParasite = false;

	auto checkWeapon = [&maxCapture, &infiniteCapture, &hasTemporal,
		&hasAirstrike, &hasLocomotor, &hasParasite](WeaponTypeClass* pWeaponType)
		{
			if (!pWeaponType)
				return;

			const auto pWH = pWeaponType->Warhead;

			if (pWH->MindControl)
			{
				if (pWeaponType->Damage > maxCapture)
					maxCapture = pWeaponType->Damage;

				if (pWeaponType->InfiniteMindControl)
					infiniteCapture = true;
			}

			if (pWH->Temporal)
				hasTemporal = true;

			if (pWH->Airstrike)
				hasAirstrike = true;

			if (pWH->IsLocomotor)
				hasLocomotor = true;

			if (pWH->Parasite)
				hasParasite = true;
		};

	for (int index = 0; index < (pCurrentType->WeaponCount > 0 ? pCurrentType->WeaponCount : 2); index++)
	{
		checkWeapon(pThis->GetWeapon(index)->WeaponType);
	}

	if (maxCapture > 0)
	{
		if (!pCaptureManager)
		{
			// Rebuild a CaptureManager
			pCaptureManager = GameCreate<CaptureManagerClass>(pThis, maxCapture, infiniteCapture);
		}
		else if (pOldTypeExt->Convert_ResetMindControl)
		{
			if (!infiniteCapture && ((FakeCaptureManagerClass*)pCaptureManager)->__GetControlledCount() > maxCapture)
			{
				// Remove excess nodes.
				for (int index = pCaptureManager->ControlNodes.Count - 1; index >= maxCapture; --index)
				{
					pCaptureManager->FreeUnit(pCaptureManager->ControlNodes.operator[](index)->Unit);
				}
			}

			pCaptureManager->MaxControlNodes = maxCapture;
			pCaptureManager->InfiniteMindControl = infiniteCapture;
		}
	}
	else if (pCaptureManager && pOldTypeExt->Convert_ResetMindControl)
	{
		// Remove CaptureManager completely
		pCaptureManager->FreeAll();
		GameDelete<true, false>(pCaptureManager);
		pCaptureManager = nullptr;
	}

	if (hasTemporal)
	{
		if (!pTemporalImUsing)
		{
			// Rebuild a TemporalClass
			pTemporalImUsing = GameCreate<TemporalClass>(pThis);
		}
	}
	else if (pTemporalImUsing)
	{
		if (pTemporalImUsing->Target)
		{
			// Free this afflicted man.
			pTemporalImUsing->LetGo();
		}

		// Delete it
		GameDelete<true, false>(pTemporalImUsing);
		pTemporalImUsing = nullptr;
	}

	if (hasAirstrike && pCurrentType->AirstrikeTeam > 0)
	{
		if (!pAirstrike)
		{
			// Rebuild a AirstrikeClass
			pAirstrike = GameCreate<AirstrikeClass>(pThis);
		}
		else
		{
			// Modify the parameters of AirstrikeClass.
			pAirstrike->AirstrikeTeam = pCurrentType->AirstrikeTeam;
			pAirstrike->EliteAirstrikeTeam = pCurrentType->EliteAirstrikeTeam;
			pAirstrike->AirstrikeTeamType = pCurrentType->AirstrikeTeamType;
			pAirstrike->EliteAirstrikeTeamType = pCurrentType->EliteAirstrikeTeamType;
			pAirstrike->AirstrikeRechargeTime = pCurrentType->AirstrikeRechargeTime;
			pAirstrike->EliteAirstrikeRechargeTime = pCurrentType->EliteAirstrikeRechargeTime;
		}
	}
	else if (pAirstrike)
	{
		pAirstrike->DetachTarget(pThis);
		GameDelete<true, false>(pAirstrike);
		pAirstrike = nullptr;
	}

	if (!hasLocomotor && pThis->LocomotorTarget)
	{
		pThis->ReleaseLocomotor(pThis->Target == pThis->LocomotorTarget);
		pThis->LocomotorTarget->LocomotorSource = nullptr;
		pThis->LocomotorTarget = nullptr;
	}

	// FireAngle
	pThis->BarrelFacing.Set_Current(DirStruct(0x4000 - (pCurrentType->FireAngle << 8)));

	// Reset recoil data
	{
		auto& turretRecoil = pThis->TurretRecoil.Turret;
		const auto& turretAnimData = pCurrentType->TurretAnimData;
		turretRecoil.Travel = turretAnimData.Travel;
		turretRecoil.CompressFrames = turretAnimData.CompressFrames;
		turretRecoil.RecoverFrames = turretAnimData.RecoverFrames;
		turretRecoil.HoldFrames = turretAnimData.HoldFrames;
		auto& barrelRecoil = pThis->BarrelRecoil.Turret;
		const auto& barrelAnimData = pCurrentType->BarrelAnimData;
		barrelRecoil.Travel = barrelAnimData.Travel;
		barrelRecoil.CompressFrames = barrelAnimData.CompressFrames;
		barrelRecoil.RecoverFrames = barrelAnimData.RecoverFrames;
		barrelRecoil.HoldFrames = barrelAnimData.HoldFrames;
	}

	// Clear AlphaImage
	if (const auto pAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pThis))
			GameDelete(pAlpha);

	if (pOldType->BombSight && !pCurrentType->BombSight)
		BombListClass::Instance->RemoveDetector(pThis);
	else if (!pOldType->BombSight && pCurrentType->BombSight)
		BombListClass::Instance->AddDetector(pThis);

	// Only FootClass* can use this.
	if (const auto pFoot = flag_cast_to<FootClass*>(pThis))
	{
		auto& pParasiteImUsing = pFoot->ParasiteImUsing;

		if (hasParasite)
		{
			if (!pParasiteImUsing)
			{
				// Rebuild a ParasiteClass
				pParasiteImUsing = GameCreate<ParasiteClass>(pFoot);
			}
		}
		else if (pParasiteImUsing)
		{
			if (pParasiteImUsing->Victim)
			{
				// Release of victims.
				pParasiteImUsing->ExitUnit();
			}

			// Delete it
			GameDelete<true, false>(pParasiteImUsing);
			pParasiteImUsing = nullptr;
		}
	}
}

void UpdateTypeData_Foot(FootClass* pThis, TechnoTypeClass* pOldType, TechnoTypeClass* pCurrentType)
{
	auto const abs = pThis->WhatAmI();
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	//auto const pOldTypeExt = TechnoTypeExt::ExtMap.Find(pOldType);

	// Update movement sound if still moving while type changed.
	if (pThis->IsMoveSoundPlaying && pThis->Locomotor->Is_Moving_Now())
	{
		if (pCurrentType->MoveSound != pOldType->MoveSound)
		{
			// End the old sound.
			pThis->MoveSoundAudioController.AudioEventHandleEndLooping();

			if (auto const count = pCurrentType->MoveSound.Count)
			{
				// Play a new sound.

				int soundIndex = count == 1 ? 0 : pCurrentType->MoveSound[Random2Class::Global->RandomFromMax(count - 1)];
				VocClass::SafeImmedietelyPlayAt(soundIndex, &pThis->Location, &pThis->MoveSoundAudioController);
				pThis->IsMoveSoundPlaying = true;
			}
			else
			{
				pThis->IsMoveSoundPlaying = false;
			}

			pThis->MoveSoundDelay = 0;
		}
	}

	if (abs == AbstractType::Infantry)
	{
		auto const pInf = static_cast<InfantryClass*>(pThis);

		// It's still not recommended to have such idea, please avoid using this
		if (static_cast<InfantryTypeClass*>(pOldType)->Deployer && !static_cast<InfantryTypeClass*>(pCurrentType)->Deployer)
		{
			switch (pInf->SequenceAnim)
			{
			case DoType::Deploy:
			case DoType::Deployed:
			case DoType::DeployedIdle:
				pInf->PlayAnim(DoType::Ready, true);
				break;
			case DoType::DeployedFire:
				pInf->PlayAnim(DoType::FireUp, true);
				break;
			default:
				break;
			}
		}
	}

	if (pOldType->Locomotor == TeleportLocomotionClass::ClassGUID() && pCurrentType->Locomotor != TeleportLocomotionClass::ClassGUID() && pThis->WarpingOut)
		pExt->HasRemainingWarpInDelay = true;

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	// OpenTopped does not work properly with buildings to begin with which is why this is here rather than in the Techno update one.

	if (pThis->Passengers.NumPassengers > 0)
	{
		const bool toOpenTopped = pCurrentType->OpenTopped;
		FootClass* pFirstPassenger = pThis->Passengers.GetFirstPassenger();

		while (true)
		{
			if (toOpenTopped)
			{
				// Add passengers to the logic layer.
				pThis->EnteredOpenTopped(pFirstPassenger);
			}
			else
			{
				// Lose target & destination
				pFirstPassenger->SetTarget(nullptr);
				pFirstPassenger->SetCurrentWeaponStage(0);
				pFirstPassenger->AbortMotion();
				pThis->ExitedOpenTopped(pFirstPassenger);

				// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
				// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
				MapClass::Logics->RemoveObject(pFirstPassenger);
			}

			pFirstPassenger->Transporter = pThis;

			if (const auto pNextPassenger = flag_cast_to<FootClass*>(pFirstPassenger->NextObject))
				pFirstPassenger = pNextPassenger;
			else
				break;
		}

		if (pCurrentType->Gunner)
			pThis->ReceiveGunner(pFirstPassenger);
	}
	else if (pCurrentType->Gunner)
	{
		pThis->RemoveGunner(nullptr);
	}

	if (!pCurrentType->CanDisguise || (!pThis->Disguise && pCurrentType->PermaDisguise))
	{
		// When it can't disguise or has lost its disguise, update its disguise.
		pThis->ClearDisguise();
	}

	if (abs != AbstractType::Aircraft)
	{
		auto const pLocomotorType = pCurrentType->Locomotor;

		// The Hover movement pattern allows for self-landing.
		if (pLocomotorType != FlyLocomotionClass::ClassGUID && pLocomotorType != HoverLocomotionClass::ClassGUID)
		{
			const bool isinAir = pThis->IsInAir() && !pThis->LocomotorSource;

			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				const int turnrate = pCurrentType->JumpJetData.TurnRate >= 127 ? 127 : pCurrentType->JumpJetData.TurnRate;
				pJJLoco->Speed = pCurrentType->JumpJetData.Speed;
				pJJLoco->Acceleration = pCurrentType->JumpJetData.Accel;
				pJJLoco->Crash = pCurrentType->JumpJetData.Crash;
				pJJLoco->Deviation = pCurrentType->JumpJetData.Deviation;
				pJJLoco->NoWobbles = pCurrentType->JumpJetData.NoWobbles;
				pJJLoco->Wobbles = pCurrentType->JumpJetData.Wobbles;
				pJJLoco->TurnRate = turnrate;
				pJJLoco->__currentHeight = pCurrentType->JumpJetData.Height;
				pJJLoco->Height = pCurrentType->JumpJetData.Height;
				pJJLoco->Facing.Set_ROT(turnrate);

				if (isinAir)
				{
					if (pCurrentType->BalloonHover)
					{
						// Makes the jumpjet think it is hovering without actually moving.
						pJJLoco->NextState = JumpjetLocomotionClass::State::Hovering;
						pJJLoco->IsMoving = true;

						if (!pJJLoco->Is_Moving_Now())
							pJJLoco->HeadToCoord = pThis->GetCoords();
					}
					else if (!pJJLoco->Is_Moving_Now())
					{
						pJJLoco->Move_To(pThis->GetCoords());
					}
				}
			}
			else if (isinAir)
			{
				// Let it go into free fall.
				pThis->IsFallingDown = true;

				const auto pCell = MapClass::Instance->TryGetCellAt(pThis->Location);

				if (pCell && !pCell->IsClearToMove(pCurrentType->SpeedType, true, true,
					ZoneType::None, pCurrentType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
				{
					// If it's landing position cannot be moved, then it is granted a crash death.
					pThis->IsABomb = true;
				}
				else
				{
					// If it's gonna land on the bridge, then it needs this.
					pThis->OnBridge = pCell ? pCell->ContainsBridge() : false;
					TechnoExtContainer::Instance.Find(pThis)->OnParachuted = true;
				}

				if (abs == AbstractType::Infantry)
				{
					// Infantry changed to parachute status (not required).
					static_cast<InfantryClass*>(pThis)->PlayAnim(DoType::Paradrop, true, false);
				}
			}
		}

		if (abs == AbstractType::Unit)
		{
			// Yes, synchronize its turret facing or it will turn strangely.
			if (pOldType->Turret != pCurrentType->Turret)
			{
				const auto primaryFacing = pThis->PrimaryFacing.Current();
				auto& secondaryFacing = pThis->SecondaryFacing;

				secondaryFacing.Set_Current(primaryFacing);
				secondaryFacing.Set_Desired(primaryFacing);
			}
		}
	}
}

bool NOINLINE TechnoExtData::ConvertToType(TechnoClass* pThis, TechnoTypeClass* pToType, bool AdjustHealth, bool IsChangeOwnership)
{
	const auto& [prevType, rtti] = GetOriginalType(pThis, pToType);

	if (!prevType)
		return false;

	const auto pOldType = prevType;
	//Debug::LogInfo("Attempt to convert TechnoType[{}] to [{}]", pOldType->ID, pToType->ID);

	if (pToType->WhatAmI() != rtti || pOldType->Spawned != pToType->Spawned || pOldType->MissileSpawn != pToType->MissileSpawn)
	{
		//Debug::LogInfo("Incompatible types between {} and {}", pOldType->ID, pToType->ID);
		return false;
	}

	const auto pToTypeExt = TechnoTypeExtContainer::Instance.Find(pToType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	HouseClass* const pOwner = pThis->Owner;

	//special cases , in this case dont need to do anything to the counter
	//just convert the techno
	if (!IsChangeOwnership)
	{
		// Remove tracking of old techno
		if (!pThis->InLimbo)
			pOwner->RegisterLoss(pThis, false);

		pOwner->RemoveTracking(pThis);
	}

	const int oldHealth = pThis->Health;

	SetType(pThis, rtti, pToType);

	if (AdjustHealth)
	{
		// Readjust health according to percentage
		pThis->SetHealthPercentage((double)(oldHealth) / (double)pOldType->Strength);
		pThis->EstimatedHealth = pThis->Health;
	}
	else
	{
		pThis->Health = pToType->Strength;
		pThis->EstimatedHealth = pToType->Strength;
	}

	//special cases , in this case dont need to do anything to the counter
	//just convert the techno
	if (!IsChangeOwnership)
	{
		// Add tracking of new techno
		pOwner->AddTracking(pThis);
		if (!pThis->InLimbo)
			pOwner->RegisterGain(pThis, true);
	}

	pOwner->RecheckTechTree = true;
	TechnoExtContainer::Instance.Find(pThis)->Is_Operated = false;
	AresAE::RemoveSpecific(&TechnoExtContainer::Instance.Find(pThis)->AeData, pThis, pOldType);
	PhobosAEFunctions::UpdateSelfOwnedAttachEffects(pThis, pToType);

	if (!pThis->IsAlive)
		return false;

	// remove previous line trail
	GameDelete<true, true>(pThis->LineTrailer);

	TechnoExtData::UpdateLaserTrails(pThis);

	// Reset AutoDeath Timer
	if (pExt->Death_Countdown.HasStarted() && pToTypeExt->Death_Countdown <= 0)
	{
		pExt->Death_Countdown.Stop();

		HouseExtContainer::Instance.AutoDeathObjects.erase(pThis);
	}

	if (pExt->PassengerDeletionTimer.HasStarted()
	&& !pToTypeExt->PassengerDeletionType.Enabled && pToTypeExt->PassengerDeletionType.Rate <= 0)
		pExt->PassengerDeletionTimer.Stop();

	// Adjust ammo
	const int originalAmmo = pThis->Ammo;
	const int maxAmmo = pToType->Ammo;

	int ammoLeft = MinImpl(originalAmmo, maxAmmo);
	pThis->Ammo = ammoLeft;
	if (ammoLeft < 0 || ammoLeft >= pToType->Ammo)
	{
		pThis->ReloadTimer.Stop();
	}
	else
	{
		int reloadLeft = pThis->ReloadTimer.GetTimeLeft();
		int reloadPrev = 0;
		int reloadNew = 0;
		if (ammoLeft == 0)
		{
			reloadPrev = pOldType->EmptyReload;
			reloadNew = pToType->EmptyReload;
		}
		else if (pThis->Ammo)
		{
			reloadPrev = pOldType->Reload;
			reloadNew = pToType->Reload;
		}
		int pass = reloadPrev - reloadLeft;
		if (pass <= 0 || pass >= reloadNew)
		{
			pThis->ReloadTimer.Stop();
		}
		else
		{
			int reload = reloadNew - pass;
			pThis->ReloadTimer.Start(reload);
		}
	}

	if (originalAmmo != pThis->Ammo)
		pThis->Mark(MarkType::Change);

	BuildingLightClass* pSpot = nullptr;

	if (pToTypeExt->HasSpotlight)
	{
		pSpot = GameCreate<BuildingLightClass>(pThis);
	}

	TechnoExtData::SetSpotlight(pThis, pSpot);
	const int value = MinImpl(pToType->ROT, 127);
	(&pThis->PrimaryFacing)->ROT.Raw = (unsigned short)(value << 8);

	const int valuesec = MinImpl(pToTypeExt->TurretRot.Get(pToType->ROT), 127);
	(&pThis->SecondaryFacing)->ROT.Raw = (unsigned short)(valuesec << 8);

	// // because we are throwing away the locomotor in a split second, piggybacking
	// // has to be stopped. otherwise the object might remain in a weird state.
	// // throw the piggybacked loco
	// while (LocomotionClass::End_Piggyback(((FootClass*)pThis)->Locomotor));

	//fucker
	const int WeaponCount = (pToType->WeaponCount > 0 ? pToType->WeaponCount : 2);
	if (pThis->CurrentWeaponNumber >= WeaponCount)
		pThis->CurrentWeaponNumber = 0;

	const int TurretCount = (pToType->TurretCount > 0 ? pToType->TurretCount : 2);
	if (pThis->CurrentTurretNumber >= TurretCount)
		pThis->CurrentTurretNumber = 0;

	TechnoExtContainer::Instance.Find(pThis)->ResetLocomotor = true;

	UpdateTypeData(pThis, pOldType, pToType);

	// Update movement sound if still moving while type changed.
	if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
	{
		// create new one if new type require it
		if (pToType->UseLineTrail)
		{
			pThis->LineTrailer = GameCreate<LineTrail>();

			if (RulesClass::Instance->LineTrailColorOverride != ColorStruct::Empty)
			{
				pThis->LineTrailer->Color = RulesClass::Instance->LineTrailColorOverride;
			}
			else
			{
				pThis->LineTrailer->Color = pToType->LineTrailColor;
			}

			pThis->LineTrailer->SetDecrement(pToType->LineTrailColorDecrement);
			pThis->LineTrailer->Owner = pThis;
		}

		UpdateTypeData_Foot(pFoot, pOldType, pToType);

		//if (pThis->LocomotorSource) {
		//	Debug::LogInfo("Attempt to convert TechnoType[%s] to [%s] when the locomotor is currently manipulated , return", pOldType->ID, pToType->ID);
		//	return true;
		//}
		bool move = true;

		// replace the original locomotor to new one
		if (pOldType->Locomotor != pToType->Locomotor)
		{
			if (pOldType->Locomotor == CLSIDs::Teleport && pToType->Locomotor != CLSIDs::Teleport && pThis->WarpingOut)
				TechnoExtContainer::Instance.Find(pThis)->HasRemainingWarpInDelay = true;

			//AbstractClass* pTarget = pThis->Target;
			//AbstractClass* pDest = pThis->ArchiveTarget;
			//Mission prevMission = pThis->GetCurrentMission();

			// throw away the current locomotor and instantiate
			// a new one of the default type for this unit.
			if (auto newLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
			{
				newLoco->Link_To_Object(pThis);
				((FootClass*)pThis)->Locomotor = std::move(newLoco);
				//pThis->Override_Mission(prevMission, pTarget, pDest);
			}
		}
		else if (pOldType->Locomotor == CLSIDs::Jumpjet() && pToType->Locomotor == CLSIDs::Jumpjet() && !(pOldType->JumpJetData == pToType->JumpJetData))
		{
			move = false;
			AbstractClass* pTarget = pThis->Target;
			AbstractClass* pDest = pThis->ArchiveTarget;
			Mission prevMission = pThis->GetCurrentMission();

			// throw away the current locomotor and instantiate
			// a new one of the default type for this unit.
			// throw away old loco to ensure the new loco properties is properly adjusted
			if (auto newLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
			{
				newLoco->Link_To_Object(pThis);
				((FootClass*)pThis)->Locomotor = std::move(newLoco);
				((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Move_To(pThis->Location);
				pThis->Override_Mission(prevMission, pTarget, pDest);
			}
		}

		if (move && pToType->BalloonHover && pToType->DeployToLand && pOldType->Locomotor != CLSIDs::Jumpjet() && pToType->Locomotor == CLSIDs::Jumpjet())
		{
			((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Move_To(pThis->Location);
		}
	}

	TechnoExtContainer::Instance.Find(pThis)->Tints.Update();
	return true;
}

int TechnoExtData::GetSelfHealAmount(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pType->SelfHealing || pThis->HasAbility(AbilityType::SelfHeal))
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pExt->SelfHealing_CombatDelay.GetTimeLeft())
			return 0x0;

		const auto rate = pTypeExt->SelfHealing_Rate.Get(
			RulesClass::Instance->RepairRate);

		const auto frames = MaxImpl(int(rate * 900.0), 1);

		if (Unsorted::CurrentFrame % frames == 0)
		{
			const auto strength = pType->Strength;

			const auto percent = pTypeExt->SelfHealing_Max.Get(pThis);
			const auto maxHealth = std::clamp(int(percent * strength), 1, strength);
			const auto health = pThis->Health;

			if (health < maxHealth)
			{
				const auto amount = pTypeExt->SelfHealing_Amount.Get(pThis);
				return std::clamp(amount, 0, maxHealth - health);
			}
		}
	}

	return 0;
}

void TechnoExtData::SpawnVisceroid(CoordStruct& crd, UnitTypeClass* pType, int chance, bool ignoreTibDeathToVisc, HouseClass* Owner)
{

	bool created = false;

	// create a small visceroid if available and the cell is free
	// dont create if `pType` is 0 strength because it is not properly listed
	if (ignoreTibDeathToVisc && pType && pType->Strength > 0)
	{
		const auto pCell = MapClass::Instance->GetCellAt(crd);

		if (!(pCell->OccupationFlags & 0x20) && ScenarioClass::Instance->Random.RandomFromMax(99) < chance)
		{
			if (auto pVisc = (UnitClass*)pType->CreateObject(Owner))
			{
				++Unsorted::ScenarioInit;
				created = pVisc->Unlimbo(crd, DirType::North);
				--Unsorted::ScenarioInit;

				if (!created)
				{
					//Debug::LogInfo(__FUNCTION__" Called ");
					TechnoExtData::HandleRemove(pVisc, nullptr, true, true);
				}
			}
		}
	}
}

void TechnoExtData::TransferOriginalOwner(TechnoClass* pFrom, TechnoClass* pTo)
{
	TechnoExtContainer::Instance.Find(pFrom)->OriginalHouseType = TechnoExtContainer::Instance.Find(pTo)->OriginalHouseType;
}

void TechnoExtData::TransferIvanBomb(TechnoClass* From, TechnoClass* To)
{
	if (auto Bomb = From->AttachedBomb)
	{
		From->AttachedBomb = nullptr;
		Bomb->Target = To;
		To->AttachedBomb = Bomb;
		To->BombVisible = From->BombVisible;
		// if there already was a bomb attached to target unit, it's gone now...
		// it shouldn't happen though, this is used for (un)deploying objects only
	}
}

void NOINLINE UpdatePassengerTurrent(TechnoClass* pThis, TechnoTypeExtData* pTypeData)
{
	const auto pType = pTypeData->This();
	if (pTypeData->PassengerTurret)
	{
		// 18 = 1 8 = A H = Adolf Hitler. Clearly we can't allow it to come to that.
		pThis->CurrentTurretNumber = MinImpl(
		MinImpl(pThis->Passengers.NumPassengers, TechnoTypeClass::MaxWeapons - 1),
		pType->TurretCount - 1);

	}

	if (pTypeData->PassengerWeapon && !pType->IsGattling && pType->WeaponCount > 0)
	{
		pThis->CurrentWeaponNumber = MinImpl(
			MinImpl(pThis->Passengers.NumPassengers, TechnoTypeClass::MaxWeapons - 1),
			pType->WeaponCount - 1);
	}
}

void NOINLINE UpdatePoweredBy(TechnoClass* pThis, TechnoTypeExtData* pTypeData)
{
	if (!pTypeData->PoweredBy.empty())
	{
		auto pExt = TechnoExtContainer::Instance.Find(pThis);

		auto pTr = pExt->GetPoweredUnit();

		if (!pTr) {
			pTr = & PhobosEntity::Emplace<PoweredUnitClass>(pExt->PoweredUnitEntity , pThis);
		}

		if (!pTr->Update())
		{
			TechnoExtData::Destroy(pThis, nullptr, nullptr, nullptr);
		}
	}
}

void NOINLINE UpdateBuildingOperation(TechnoExtData* pData, TechnoTypeExtData* pTypeData)
{
	auto const pThis = pData->This();

	if (TechnoExtContainer::Instance.Find(pThis)->Is_Operated && pThis->WhatAmI() == BuildingClass::AbsID)
	{
		if (pThis->Deactivated
			&& pThis->IsPowerOnline()
			&& !pThis->IsUnderEMP()
			&& TechnoExtData::IsPowered(pThis)
			)
		{
			pThis->Reactivate();
			pThis->Owner->RecheckTechTree = true; // #885
		}
	}
	else
	{
		auto const pBuildingBelow = pThis->GetCell()->GetBuilding();
		auto const buildingBelowIsMe = pThis == pBuildingBelow;

		if (!pBuildingBelow || (buildingBelowIsMe && pBuildingBelow->IsPowerOnline()))
		{
			bool Override = false;
			if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
			{
				if (!pBuildingBelow)
				{
					// immobile, though not disabled. like hover tanks after
					// a repair depot has been sold or warped away.
					Override = ((BYTE)pFoot->Locomotor.GetInterfacePtr()->Is_Powered() == pThis->Deactivated);
				}
			}

			if (TechnoExtData::IsOperatedB(pThis))
			{ // either does have an operator or doesn't need one, so...
				if (Override || (pThis->Deactivated && !pThis->IsUnderEMP() && TechnoExtData::IsPowered(pThis)))
				{ // ...if it's currently off, turn it on! (oooh baby)
					pThis->Reactivate();
					if (buildingBelowIsMe)
					{
						pThis->Owner->RecheckTechTree = true; // #885
					}
				}
			}
			else
			{ // doesn't have an operator, so...
				if (!pThis->Deactivated)
				{ // ...if it's not off yet, turn it off!
					pThis->Deactivate();
					if (buildingBelowIsMe)
					{
						pThis->Owner->RecheckTechTree = true; // #885
					}
				}
			}
		}
	}
}

void NOINLINE UpdateRadarJammer(TechnoExtData* pData, TechnoTypeExtData* pTypeData)
{
	auto const pThis = pData->This();

	// prevent disabled units from driving around.
	auto pJam = pData->GetRadarJammer();

	if (pThis->Deactivated)
	{
		if (auto const pUnit = cast_to<UnitClass*, false>(pThis))
		{
			if (pUnit->Locomotor->Is_Moving() && pUnit->Destination && !pThis->LocomotorSource)
			{
				pUnit->SetDestination(nullptr, true);
				pUnit->StopMoving();
			}
		}

		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		;
		if (pJam)
		{ // RadarJam should only be non-null if the object is an active radar jammer
			pJam->UnjamAll();
		}
	}
	else
	{
		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if (pTypeData->RadarJamRadius)
		{
			if (!pJam) {
				pJam = &PhobosEntity::Emplace<RadarJammerClass>(pData->RadarJammerEntity, pThis);
			}

			pJam->Update();
		}
	}
}

void NOINLINE UpdateTheType(TechnoClass* pThis, TechnoTypeExtData* pOldTypeExt)
{
	if (pOldTypeExt->Convert_Water || pOldTypeExt->Convert_Land)
	{
		TechnoTypeClass* Convert = pOldTypeExt->Convert_Land;
		if (!pThis->OnBridge)
		{ //avoid calling `GetCell()` all the time ?
			CellClass* pCell = pThis->GetCell();
			if (pCell && (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
				Convert = pOldTypeExt->Convert_Water;
		}

		if (Convert && pOldTypeExt->This() != Convert)
			TechnoExtData::ConvertToType(pThis, Convert);
	}
}

void TechnoExtData::Ares_technoUpdate(TechnoClass* pThis)
{
	const auto pOldType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	auto pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);

	UpdateTheType(pThis, pOldTypeExt);
	UpdatePassengerTurrent(pThis, pOldTypeExt);
	UpdatePoweredBy(pThis, pOldTypeExt);
	AresAE::Update(&pExt->AeData, pThis);
	UpdateBuildingOperation(pExt, pOldTypeExt);
	UpdateRadarJammer(pExt, pOldTypeExt);

	if (pExt->TechnoValueAmount)
		TechnoExtData::Ares_AddMoneyStrings(pThis, false);

	const auto pFoot = flag_cast_to<FootClass*, false>(pThis);

	if (pFoot
		&& pExt->Is_DriverKilled
		&& pThis->CurrentMission != Mission::Harmless
		&& !pFoot->IsAttackedByLocomotor
		//&& ScenarioClass::Instance->Random.RandomBool()
		)
	{
		pThis->SetTarget(nullptr);
		pThis->EnterIdleMode(false, 1);
		pThis->QueueMission(Mission::Harmless, true);
	}
}

void TechnoExtData::Ares_AddMoneyStrings(TechnoClass* pThis, bool forcedraw)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto value = pExt->TechnoValueAmount;
	static fmt::basic_memory_buffer<wchar_t> moneyStr;

	if (value && (forcedraw || Unsorted::CurrentFrame >= pExt->Pos))
	{
		pExt->Pos = Unsorted::CurrentFrame - int32_t(RulesExtData::Instance()->DisplayCreditsDelay * -900.0);
		pExt->TechnoValueAmount = 0;
		bool isPositive = value > 0;

		moneyStr.clear();
		const ColorStruct& color = isPositive
			? Drawing::DefaultColors[(int)DefaultColorList::Green] :
			Drawing::DefaultColors[(int)DefaultColorList::Red];

		fmt::format_to(std::back_inserter(moneyStr), L"{}{}{}", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, Math::abs(value));
		moneyStr.push_back(L'\0');

		CoordStruct loc = pThis->GetCoords();
		if (!MapClass::Instance->IsLocationShrouded(loc)
			&& pThis->VisualCharacter(FALSE, HouseClass::CurrentPlayer()) != VisualType::Hidden)
		{
			if (pThis->WhatAmI() == BuildingClass::AbsID)
			{
				loc.Z += 104 * ((BuildingClass*)pThis)->Type->Height;
			}
			else
			{
				loc.Z += 256;
			}

			FlyingStrings::Instance.Add(moneyStr, loc, color, {});
		}
	}
}

int TechnoExtData::ApplyTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk)
{
	int tintColor = 0;
	auto g_instance = PhobosGlobal::Instance();

	if (invulnerability && pThis->IsIronCurtained())
		tintColor |= pThis->ProtectType == ProtectTypes::ForceShield ? g_instance->ColorDatas.Forceshield_Color : g_instance->ColorDatas.IronCurtain_Color;

	if (airstrike && TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe) {
		auto const pTypeExt = GET_TECHNOTYPEEXT(TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe->Owner);
		tintColor |= pTypeExt->TintColorAirstrike;
	}

	if (berserk && pThis->Berzerk)
		tintColor |= g_instance->ColorDatas.Berserk_Color;

	return tintColor;
}

void TechnoExtData::ApplyCustomTint(TechnoClass* pThis, int* tintColor, int* intensity)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	pExt->Tints.GetTints(tintColor, intensity);

	bool calculateIntensity = intensity != nullptr;
	//const bool calculateTintColor = tintColor != nullptr;
	BuildingClass* pBld = cast_to<BuildingClass*, false>(pThis);
	bool needRedraw = false;

	if (calculateIntensity)
	{
		if (pBld)
		{
			if ((pBld->CurrentMission == Mission::Construction)
				&& pBld->BState == BStateType::Construction && pBld->Type->Buildup)
			{
				if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->BuildUp_UseNormalLIght.Get())
				{
					*intensity = 1000;
				}
			}
		}

		const bool bInf = pThis->WhatAmI() == InfantryClass::AbsID;

		// EMP
		if (pThis->IsUnderEMP())
		{
			if (!bInf || pTypeExt->Infantry_DimWhenEMPEd.Get(((InfantryTypeClass*)(pTypeExt->This()))->Cyborg))
			{
				*intensity /= 2;
				needRedraw = true;
			}
		}
		else if (pThis->IsDeactivated())
		{
			if (!bInf || pTypeExt->Infantry_DimWhenDisabled.Get(((InfantryTypeClass*)(pTypeExt->This()))->Cyborg))
			{
				*intensity /= 2;
				needRedraw = true;
			}
		}
	}

	if (pBld && needRedraw)
		BuildingExtContainer::Instance.Find(pBld)->LighningNeedUpdate = true;
}

void TechnoExtData::InitPassiveAcquireMode()
{
	auto pType = GET_TECHNOTYPE(This());
	this->PassiveAquireMode = TechnoTypeExtContainer::Instance.Find(pType)->PassiveAcquireMode.Get();
}

PassiveAcquireModes TechnoExtData::GetPassiveAcquireMode() const
{
	// if this is a passenger then obey the configuration of the transport
	if (auto pTransport = This()->Transporter)
		return TechnoExtContainer::Instance.Find(pTransport)->GetPassiveAcquireMode();

	return this->PassiveAquireMode;
}

void TechnoExtData::TogglePassiveAcquireMode(PassiveAcquireModes newMode)
{
	auto previousMode = this->PassiveAquireMode;
	this->PassiveAquireMode = newMode;

	if (newMode == previousMode)
		return;

	const auto pThis = This();
	const auto pTechnoType = GET_TECHNOTYPE(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	int voiceIndex;

	if (newMode == PassiveAcquireModes::Normal)
	{
		if (previousMode == PassiveAcquireModes::Ceasefire)
		{
			voiceIndex = pTypeExt->VoiceExitCeasefireMode.Get();

			if (voiceIndex < 0)
			{
				const auto& voiceList = pTechnoType->VoiceAttack.Count ? pTechnoType->VoiceAttack : pTechnoType->VoiceMove;

				if (const auto count = voiceList.Count)
					voiceIndex = voiceList.operator[](Random2Class::Global->Random() % count);
			}
		}
		else
		{
			pThis->SetTarget(nullptr);
			voiceIndex = pTypeExt->VoiceExitAggressiveMode.Get();

			if (voiceIndex < 0)
			{
				const auto& voiceList = pTechnoType->VoiceMove.Count ? pTechnoType->VoiceMove : pTechnoType->VoiceSelect;

				if (const auto count = voiceList.Count)
					voiceIndex = voiceList.operator[](Random2Class::Global->Random() % count);
			}
		}
	}
	else if (newMode == PassiveAcquireModes::Ceasefire)
	{
		pThis->SetTarget(nullptr);
		voiceIndex = pTypeExt->VoiceEnterCeasefireMode.Get();

		if (voiceIndex < 0)
		{
			const auto& voiceList = pTechnoType->VoiceSelect.Count ? pTechnoType->VoiceSelect : pTechnoType->VoiceMove;

			if (const auto count = voiceList.Count)
				voiceIndex = voiceList.operator[](Random2Class::Global->Random() % count);
		}
	}
	else
	{
		voiceIndex = pTypeExt->VoiceEnterAggressiveMode.Get();

		if (voiceIndex < 0)
		{
			const auto& voiceList = pTechnoType->VoiceAttack.Count ? pTechnoType->VoiceAttack : pTechnoType->VoiceMove;

			if (const auto count = voiceList.Count)
				voiceIndex = voiceList.operator[](Random2Class::Global->Random() % count);
		}
	}

	pThis->QueueVoice(voiceIndex);
}

bool TechnoExtData::CanTogglePassiveAcquireMode()
{
	if (!RulesExtData::Instance()->EnablePassiveAcquireMode)
		return false;

	return GET_TECHNOTYPEEXT(This())->PassiveAcquireMode_Togglable;
}

bool TechnoExtData::IsOnBridge(FootClass* pUnit)
{
	auto const pCell = MapClass::Instance->GetCellAt(pUnit->GetCoords());
	auto const pCellAjd = pCell->GetNeighbourCell(FacingType::North);
	bool containsBridge = pCell->ContainsBridge();
	bool containsBridgeDir = static_cast<bool>(pCell->Flags & CellFlags::BridgeDir);

	if ((containsBridge || containsBridgeDir || pCellAjd->ContainsBridge()) && (!containsBridge || pCell->GetNeighbourCell(FacingType::West)->ContainsBridge()))
		return true;

	return false;
}

int TechnoExtData::GetJumpjetIntensity(FootClass* pThis)
{
	int level = ScenarioClass::Instance->NormalLighting.Level;

	if (LightningStorm::IsActive())
		level = ScenarioClass::Instance->IonLighting.Level;
	else if (PsyDom::IsActive())
		level = ScenarioClass::Instance->DominatorLighting.Level;
	else if (NukeFlash::IsFadingIn())
		level = ScenarioClass::Instance->NukeLighting.Level;

	int levelIntensity = 0;
	int cellIntensity = 1000;
	GetLevelIntensity(pThis, level, levelIntensity, cellIntensity, RulesExtData::Instance()->JumpjetLevelLightMultiplier, 0.0, IsOnBridge(pThis));

	return levelIntensity + cellIntensity;
}

void TechnoExtData::GetLevelIntensity(TechnoClass* pThis, int level, int& levelIntensity, int& cellIntensity, double levelMult, double cellMult, bool applyBridgeBonus)
{
	double currentLevel = pThis->GetHeight() / static_cast<double>(Unsorted::LevelHeight);
	levelIntensity = static_cast<int>(level * currentLevel * levelMult);
	int bridgeBonus = applyBridgeBonus ? 4 * level : 0;
	cellIntensity = MapClass::Instance()->GetCellAt(pThis->GetMapCoords())->Color1.Red + bridgeBonus;

	if (cellMult > 0.0)
		cellIntensity = std::clamp(cellIntensity + static_cast<int>((1000 - cellIntensity) * currentLevel * cellMult), 0, 1000);
	else if (cellMult < 0.0)
		cellIntensity = 1000;
}

int TechnoExtData::GetDeployingAnimIntensity(FootClass* pThis)
{
	int intensity = 0;

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
		intensity = GetJumpjetIntensity(pThis);
	else
		intensity = pThis->GetCell()->Color1.Red;

	intensity = pThis->GetFlashingIntensity(intensity);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	return intensity;
}

bool __fastcall FakeTechnoClass::__Is_Allowed_To_Retaliate(TechnoClass* pThis , discard_t , TechnoClass* pSource, WarheadTypeClass* pWarhead)
{
	if (!pSource || !pSource->IsAlive || pSource->IsCrashing || pSource->IsSinking)
		return false;

	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	if (pExt->Is_DriverKilled || pWHExt->Nonprovocative)
		return false;

	// HOOK: 0x7087DD - TechnoClass_CanRetaliateToAttacker_CeasefireMode (0x6 bytes)
	// Checks CanRetaliate, DriverKilled, and CeasefireMode
	// Returns to 0x7087E3 after setting CL, or jumps to 0x708B17 if driver killed
	const auto pType = GET_TECHNOTYPE(pThis);
	const bool canRetaliate = pType->CanRetaliate && (pExt->GetPassiveAcquireMode() != PassiveAcquireModes::Ceasefire);

	if (!canRetaliate)
		return false;

	if (pThis->SlaveOwner)
		return false;

	if (pThis->SlaveManager)
		return false;

	if (pThis->DrainTarget && !pThis->Owner->IsHumanPlayer)
		return false;

	if (const auto pCaptureManager = pThis->CaptureManager) {
		if (pCaptureManager->CannotControlAnyMore())
			return false;
	}

	if (pThis->SpawnManager)
		return false;

	const bool bIsPlayerControl = pThis->Owner->IsControlledByHuman();

	if (bIsPlayerControl && pThis->Target)
		return false;

	if (!pThis->GetCurrentMissionControl()->Retaliate)
		return false;

	if (pThis->Owner->IsAlliedWith(pSource))
		return false;

	if (pSource->CantTarget(pThis->Owner))
		return false;

	if (pThis->CombatDamage() <= 0)
		return false;

	if (!pThis->IsArmed())
		return false;

	const int nWeaponIdx = pThis->SelectWeapon(pSource);
	UnitClass* pThisUnit = cast_to<UnitClass*, false>(pThis);
	FootClass* pThisFoot = flag_cast_to<FootClass*, false>(pThis);
	InfantryClass* pThisInf = cast_to<InfantryClass*, false>(pThis);
	BuildingClass* pThisBld = cast_to<BuildingClass*, false>(pThis);

	bool ignoreRange = false;
	// HOOK: 0x7088E3 - TechnoClass_ShouldRetaliate_DisallowMoving (0x6 bytes)
	// For units that cannot move, calls GetFireError with different parameters
	// Returns to 0x7088F3 after setting EAX, or continues normally if not applicable
	if (pThisUnit && TechnoExtData::CannotMove(pThisUnit) || pThisInf && pThisInf->Type->Speed <= 0) {
		ignoreRange = true;
	}

	FireError nFireError = pThis->GetFireError(pSource, nWeaponIdx, ignoreRange);

	if (nFireError == FireError::ILLEGAL || nFireError == FireError::CANT)
		return false;

	// Player-controlled units vs buildings - C4 logic
	if (bIsPlayerControl) {
		if (auto pSourceBuilding = cast_to<BuildingClass*, false>(pSource)) {
			if (pThisInf && pThisInf->Type->C4)
				return false;

			const bool bIsVeteran = pThis->Veterancy.IsVeteran();
			const bool bIsElite = pThis->Veterancy.IsElite();

			if (bIsVeteran || bIsElite) {
				if (bIsVeteran && pType->VeteranAbilities.C4)
					return false;

				if (bIsElite && (pType->VeteranAbilities.C4 || pType->EliteAbilities.C4))
					return false;
			}
		}
	}

	// Player-controlled artillery units should not retaliate
	if (bIsPlayerControl && pThisUnit && pThisUnit->Type->DeploysInto && pThisUnit->Type->DeploysInto->Artillary) {
		return false;
	}

	// Player-controlled units without SmartDefense should only retaliate in guard missions
	if (bIsPlayerControl && !RulesClass::Instance->Scatter && !pThisBld) {
		if (pThis->CurrentMission != Mission::Area_Guard
			&& pThis->CurrentMission != Mission::Guard
			&& pThis->CurrentMission != Mission::Patrol){
			return false;
		}
	}

	// Units in suicide teams should not retaliate
	if (pThisFoot && pThisFoot->Team && pThisFoot->Team->Type->Suicide) {
		return false;
	}

	// AI units should not switch targets if current target is closer
	if (!bIsPlayerControl) {
		if (auto pTargetFoot = flag_cast_to<FootClass*>(pThis->Target)) {
			auto emptyCoords = CoordStruct::Empty;

			const double dCurrentTargetCoeff = FakeTechnoClass::__GetThreatCoeff(pThis, discard_t(), pTargetFoot, &emptyCoords);
			const double dSourceCoeff = FakeTechnoClass::__GetThreatCoeff(pThis, discard_t(), pSource, &emptyCoords);

			if (dSourceCoeff <= dCurrentTargetCoeff)
				return false;
		}
	}

	// Units should not retaliate against the parasite eating them
	if (pThisFoot && pThisFoot->ParasiteEatingMe) {
		if (pSource == pThisFoot->ParasiteEatingMe)
			return false;
	}

	// HOOK: 0x708AF7 - TechnoClass_ShouldRetaliate_Verses (0x7 bytes)
	// Checks Nonprovocative warhead flag and custom armor/verses retaliation logic
	// Jumps to 0x708B0B (Retaliate/return true) or 0x708B17 (DoNotRetaliate/return false)
	// At this point: EBP = pSource, ECX = pWarhead->WarheadPtr, ESI = pWeapon
	const auto pWeapon = pThis->GetWeapon(nWeaponIdx)->WeaponType;

	if (!TechnoExtData::CanRetaliateICUnit(pThis, (FakeWeaponTypeClass*)pWeapon, pSource))
		return false;

	if (pWeapon) {
		if (const auto pWarheadPtr = pWeapon->Warhead) {
			Armor armor = TechnoExtData::GetTechnoArmor(pSource, pWarheadPtr);
			auto pCurWeaponWHExt = WarheadTypeExtContainer::Instance.Find(pWarheadPtr);
			auto& verses = pCurWeaponWHExt->GetVerses(armor);

			if (!verses.Flags.Retaliate && verses.Verses <= 0.0099999998)
				return false;
		}
	}

	return true;
}

void __fastcall FakeTechnoClass::__DoUncloak(TechnoClass* pThis, discard_t, char quiet)
{
	auto state = pThis->CloakState;

	if (state == CloakState::Cloaked || state == CloakState::Cloaking) {
		const auto pType = GET_TECHNOTYPE(pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		pThis->CloakState = CloakState::Uncloaking;
		auto stages = pTypeExt->CloakStages.Get(RulesClass::Instance->CloakingStages);
		pThis->CloakProgress.Start(pType->CloakingSpeed, -1, stages - 1);

		if (!quiet) {
			const int nDefault = RulesExtData::Instance()->DecloakSound.Get(RulesClass::Instance->CloakSound);
			VocClass::ImmedietelyPlayAt(pTypeExt->DecloakSound.Get(nDefault), pThis->Location);

			if (const auto pAnimType = pTypeExt->DecloakAnim.Get(RulesExtData::Instance()->DecloakAnim)) {
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->GetCoords()),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}
}

void __fastcall FakeTechnoClass::__DoCloak(TechnoClass * pThis, discard_t, char quiet)
{
	auto state = pThis->CloakState;

	if (state == CloakState::Uncloaking || state == CloakState::Uncloaked) {
		const auto pType = GET_TECHNOTYPE(pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		pThis->CloakState = CloakState::Cloaking;
		pThis->CloakProgress.Start(pType->CloakingSpeed, 1, 0);

		if (!quiet) {
			VocClass::ImmedietelyPlayAt(pTypeExt->CloakSound.Get(RulesClass::Instance->CloakSound), pThis->Location);

			if (const auto pAnimType = pTypeExt->CloakAnim.Get(RulesExtData::Instance()->CloakAnim)) {
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->GetCoords()),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x703770 , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2704 , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E431C , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E90F4 , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB4B8 , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4DC0 , FakeTechnoClass::__DoCloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F60D0 , FakeTechnoClass::__DoCloak)


DEFINE_FUNCTION_JUMP(LJMP, 0x7036C0, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(CALL6, 0x703854, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2700, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4318, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E90F0, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB4B4, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4DBC, FakeTechnoClass::__DoUncloak)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F60CC, FakeTechnoClass::__DoUncloak)

//Aircraft 7E23B4
//Building 7E3FCC
//foot 7E8DA4
//infantry 7EB168
//techno 7F4A70
//unit 7F5D80
void __fastcall FakeTechnoClass::__DrawExtras(TechnoClass* pThis, discard_t, Point2D* pLocation, RectangleStruct* pBounds)
{
	auto DrawTheStuff = [&pLocation, &pThis, &pBounds](const wchar_t* pFormat)
		{
			auto nPoint = *pLocation;
			//DrawingPart
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
			auto nIntersect = RectangleStruct::Intersect(nTextDimension, *pBounds, nullptr, nullptr);
			auto nColorInt = pThis->Owner->Color.ToInit();//0x63DAD0

			DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
			DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
			TextDrawing::Simple_Text_Print_Wide(pFormat, DSurface::Temp.get(), pBounds, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
		};

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
		{
			if (auto pTeam = pFoot->Team)
			{
				if (auto const pTeamLeader = pTeam->FetchLeader())
				{
					if (pTeamLeader == pThis)
					{
						DrawTheStuff(L"Team Leader");
					}
				}
			}
		}
	}

	pThis->DrawTechnoExtras(pLocation, pBounds);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E23B4, FakeTechnoClass::__DrawExtras);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8DA4, FakeTechnoClass::__DrawExtras);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB168, FakeTechnoClass::__DrawExtras);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4A70, FakeTechnoClass::__DrawExtras);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D80, FakeTechnoClass::__DrawExtras);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FCC, FakeBuildingClass::_DrawExtras);

int __fastcall FakeTechnoClass::__HowManySurvivors(TechnoClass* pThis)
{
	auto pType = GET_TECHNOTYPE(pThis);

	// previous default for crew count was -1
	int count = TechnoTypeExtContainer::Instance.Find(pType)->Survivors_PilotCount.Get();

	// default to original formula
	if (count < 0) {
		count = pType->Crewed ? 1 : 0;
	}

	return count;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F3950, FakeTechnoClass::__HowManySurvivors)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2574, FakeTechnoClass::__HowManySurvivors)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8F64, FakeTechnoClass::__HowManySurvivors)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB328, FakeTechnoClass::__HowManySurvivors)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4C30, FakeTechnoClass::__HowManySurvivors)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F40, FakeTechnoClass::__HowManySurvivors)


bool __fastcall FakeTechnoClass::__ShouldSelfHealOneStep(TechnoClass* pThis)
{
	auto const nAmount = TechnoExtData::GetSelfHealAmount(pThis);
	return nAmount != 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x70BE80, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2538, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4150, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8F28, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB2EC, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4BF4, FakeTechnoClass::__ShouldSelfHealOneStep)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F04, FakeTechnoClass::__ShouldSelfHealOneStep)

int __fastcall FakeTechnoClass::__TimeToBuild(TechnoClass* pThis)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto what = pThis->WhatAmI();
	const bool isNaval = what == UnitClass::AbsID && pType->Naval;
	int finalSpeed = 0;

	if (const auto pOwner = pThis->Owner)
	{
		const int cap = RulesExtData::Instance()->MultipleFactoryCap.Get(what, isNaval);
		const double nFactorySpeed = pTypeExt->BuildTime_MultipleFactory.Get(RulesClass::Instance->MultipleFactory);
		finalSpeed = (int)(pType->BuildTimeMultiplier * pOwner->GetBuildTimeMult(pType) * (double)pType->GetBuildSpeed());

		//Power
		const double nPowerPercentage = pOwner->GetPowerPercentage();

		//if the house dont have power at all disable all the penalties
		{

			const double nLowPowerPenalty = pTypeExt->BuildTime_LowPowerPenalty.Get(RulesClass::Instance->LowPowerPenaltyModifier);
			const double nMinLowPoweProductionSpeed = pTypeExt->BuildTime_MinLowPower.Get(RulesClass::Instance->MinLowPowerProductionSpeed);
			const double nMaxLowPowerProductionSpeed = pTypeExt->BuildTime_MaxLowPower.Get(RulesClass::Instance->MaxLowPowerProductionSpeed);
			double powerdivisor = 1.0 - nLowPowerPenalty * (1.0 - nPowerPercentage);

			if (powerdivisor <= nMinLowPoweProductionSpeed)
			{
				powerdivisor = nMinLowPoweProductionSpeed;
			}

			if (nPowerPercentage < 1.0 && powerdivisor >= nMaxLowPowerProductionSpeed)
			{
				powerdivisor = nMaxLowPowerProductionSpeed;
			}

			if (powerdivisor < 0.01)
			{
				powerdivisor = 0.01;
			}

			finalSpeed = int((double)finalSpeed / powerdivisor);
		}

		if (nFactorySpeed > 0)
		{//Multiple Factory

			const int factoryCount = pOwner->FactoryCount(what, isNaval);
			const int divisor = (cap > 0 && factoryCount >= cap) ? cap : factoryCount;

			for (int i = divisor - 1; i > 0; --i)
			{
				finalSpeed = int(finalSpeed * nFactorySpeed);
			}
		}

		const auto bonus = BuildingTypeExtData::GetExternalFactorySpeedBonus(pThis);
		if (bonus > 0.0)
			finalSpeed = int((double)finalSpeed * bonus);
	}

	{ //Exception
		if (what == BuildingClass::AbsID && !pTypeExt->BuildTime_Speed.isset() && static_cast<BuildingTypeClass*>(pType)->Wall)
			finalSpeed = int((double)finalSpeed * RulesClass::Instance->WallBuildSpeedCoefficient);
	}

	return finalSpeed;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F47A0, FakeTechnoClass::__TimeToBuild)
DEFINE_FUNCTION_JUMP(CALL, 0x4C9EEF, FakeTechnoClass::__TimeToBuild)
DEFINE_FUNCTION_JUMP(CALL, 0x4C9FB9, FakeTechnoClass::__TimeToBuild)
DEFINE_FUNCTION_JUMP(CALL, 0x4CA706, FakeTechnoClass::__TimeToBuild)


InfantryTypeClass* __fastcall FakeTechnoClass::__GetCrew(TechnoClass* pThis)
{
	auto pType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto pHouse = pThis->Owner;
	InfantryTypeClass* pCrewType = nullptr;

	// YR defaults to 15 for armed objects,
	// Ares < 0.5 defaulted to 0 for non-buildings.
	const int TechnicianChance = pExt->Crew_TechnicianChance.Get(
		(pThis->AbstractFlags & AbstractFlags::Foot) ?
		0 :
		pThis->IsArmed() ? 15 : 0
	);

	if (pType->Crewed) {
		// for civilian houses always technicians. random for others
		const bool isTechnician = pHouse->Type->SideIndex == -1 ? true :
			TechnicianChance > 0 && ScenarioClass::Instance->Random.RandomFromMax(99) < TechnicianChance
			? true : false;

		// chose the appropriate type
		if (!isTechnician) {
			// customize with this techno's pilot type
			// only use it if non-null, as documented

			const auto& nVec = pExt->Survivors_Pilots;

			if ((size_t)pHouse->SideIndex >= nVec.size()) {
				pCrewType = HouseExtData::GetCrew(pHouse);
			}
			else if (auto pPilotType = nVec[pHouse->SideIndex]) {
				pCrewType = pPilotType;
			} else {
				pCrewType = HouseExtData::GetCrew(pHouse);
			}
		} else {
			// either civilian side or chance
			pCrewType = HouseExtData::GetTechnician(pHouse);
		}
	}

	return pCrewType;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x707D20, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(LJMP, 0x740EE0, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F7C, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E25B0, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8FA0, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB364, FakeTechnoClass::__GetCrew)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4C6C, FakeTechnoClass::__GetCrew)

void __fastcall FakeTechnoClass:: __Activate(TechnoClass* pThis)
{
	const auto pType = GET_TECHNOTYPE(pThis);

	if (pType->PoweredUnit && pThis->Owner) {
		pThis->Owner->RecheckPower = true;
	}

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
		If any of the above conditions, bail out and don't activate the object.
	*/

	if (pThis->IsUnderEMP() || !TechnoExtData::IsPowered(pThis)){
		return;
	}

	if (TechnoExtData::IsOperatedB(pThis)) {
		pThis->Guard();

		if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis)) {
			pFoot->Locomotor.GetInterfacePtr()->Power_On();
		}

		if (auto const wasDeactivated = std::exchange(pThis->Deactivated, false)) {
			// change: don't play sound when mutex active
			if (!Unsorted::ScenarioInit) {
				VocClass::SafeImmedietelyPlayAt(pType->ActivateSound, &pThis->Location, nullptr);
			}

			// change: add spotlight
			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
			if (pTypeExt->HasSpotlight) {
				++Unsorted::ScenarioInit;
				TechnoExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
				--Unsorted::ScenarioInit;
			}

			// change: update factories
			if (auto const pBld = cast_to<BuildingClass*, false>(pThis)) {
				TechnoExtData::UpdateFactoryQueues(pBld);
			}
		}
	}
}

void FakeTechnoClass::__Deactivate(TechnoClass* pThis)
{
	const auto pType = GET_TECHNOTYPE(pThis);

	if (pType->PoweredUnit && pThis->Owner) {
		pThis->Owner->RecheckPower = true;
	}

	// don't deactivate when inside/on the linked building
	if (pThis->IsTethered) {
		auto const pLink = pThis->GetNthLink(0);

		if (pLink && pThis->GetCell()->GetBuilding() == pLink) {
			return;
		}
	}

	pThis->Guard();
	pThis->Deselect();

	if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis)) {
		pFoot->Locomotor.GetInterfacePtr()->Power_Off();
	}

	auto const wasDeactivated = std::exchange(pThis->Deactivated, true);

	if (!wasDeactivated) {
		// change: don't play sound when mutex active
		if (!Unsorted::ScenarioInit) {
			VocClass::SafeImmedietelyPlayAt(pType->DeactivateSound, &pThis->Location, nullptr);
		}

		// change: remove spotlight
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		if (pTypeExt->HasSpotlight) {
			TechnoExtData::SetSpotlight(pThis, nullptr);
		}

		// change: update factories
		if (auto const pBld = cast_to<BuildingClass*, false>(pThis)) {
			TechnoExtData::UpdateFactoryQueues(pBld);
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x70FC90, FakeTechnoClass::__Deactivate)
DEFINE_FUNCTION_JUMP(LJMP, 0x70FBE0, FakeTechnoClass::__Activate)

// Draws airstrike targeting flare/beam between aircraft and target
// With Ares/Phobos extensions for custom colors and Z-depth fix
void __fastcall FakeTechnoClass::__Draw_Airstrike_Flare(TechnoClass* techno, discard_t, CoordStruct startCoord, CoordStruct endCoord)
{
	// [Hook 1: TechnoClass_DrawAirstrikeFlare_SetContext]
	// Captures TechnoTypeExtData for custom color lookup

	const Point2D startPixel = TacticalClass::Instance->CoordsToClient(startCoord);
	const Point2D endPixel =  TacticalClass::Instance->CoordsToClient(endCoord);

	// Original: Two separate Z-depth values
	 int startDepth = -32 - Game::AdjustHeight(startCoord.Z);
	 int endDepth = -32 - Game::AdjustHeight(endCoord.Z);

	// [Hook 2: TechnoClass_DrawAirstrikeFlare]
	// Fixed: Use single Z value (minimum) + configurable adjustment
	const int zValue = MinImpl(startDepth, endDepth) + RulesExtData::Instance()->AirstrikeLineZAdjust;
	startDepth = zValue;
	endDepth = zValue;
	auto pExt = GET_TECHNOTYPEEXT(techno);
	int deltaX_abs = Math::abs(endPixel.X - startPixel.X);
	int deltaY_abs = Math::abs(endPixel.Y - startPixel.Y);

	if (!DSurface::Temp->IsDSurface())
		return;

	// [Hook 2 continued: Custom color instead of random orange]
	// Original: Random value 190-270, orange-red beam
	// New: Configurable base color with 74.5%-100% brightness variation
	const ColorStruct baseColor = pExt->AirstrikeLineColor .Get(RulesExtData::Instance()->AirstrikeLineColor);

	const double percentage = Random2Class::Global->RandomRanged(745, 1000) / 1000.0;
	ColorStruct beamColor {
		(BYTE)(baseColor.R * percentage),
		(BYTE)(baseColor.G * percentage),
		(BYTE)(baseColor.B * percentage)
	};

	const WORD crosshairColor = (WORD)baseColor.ToInit();

	// [Hook 3: TechnoClass_DrawAirstrikeFlare_DotColor]
	// Original: Only draw crosshair if endPixel.Y < startPixel.Y
	// New: Always draw crosshair (skip the Y comparison)
	{
		// Draw crosshair at target point
		// Horizontal lines
		Point2D _hzl_start(endPixel.X - 2, endPixel.Y + 1);
		Point2D _hzl_end(endPixel.X + 1, endPixel.Y + 1);

		DSurface::Temp->Draw_Line(
			_hzl_start,
			_hzl_end,
			crosshairColor
		);

		Point2D _hzl2_start(endPixel.X - 2, endPixel.Y);
		Point2D _hzl2_end(endPixel.X + 1, endPixel.Y);

		DSurface::Temp->Draw_Line(
			_hzl2_start,
			_hzl2_end,
			crosshairColor
		);
		// Vertical lines
		Point2D _vert_start(endPixel.X - 1, endPixel.Y - 1);
		Point2D _vert_end(endPixel.X - 1, endPixel.Y + 2);

		DSurface::Temp->Draw_Line(
			_vert_start,
			_vert_end,
			crosshairColor
		);

		Point2D _vert2_start(endPixel.X, endPixel.Y - 1);
		Point2D _vert2_end(endPixel.X, endPixel.Y + 2);
		DSurface::Temp->Draw_Line(
			_vert2_start,
			_vert2_end,
			crosshairColor
		);
	}

	// Draw 4-segment fading beam
	int deltaX = endPixel.X - startPixel.X;
	int deltaY = endPixel.Y - startPixel.Y;
	int deltaDepth = endDepth - startDepth;  // Now always 0 due to fix

	Point2D currentPos = startPixel;
	int currentDepth = startDepth;

	int accumX = 0;
	int accumY = 0;
	int accumDepth = 0;
	int accumAlpha = 0;
	int alpha = 100;

	while (accumAlpha > -765)
	{
		Point2D nextPos(currentPos.X + (accumX + deltaX) / 4, currentPos.Y + (accumY + deltaY) / 4);

		const int nextDepth = currentDepth + (accumDepth + deltaDepth) / 4;
		const int segmentAlpha = (accumAlpha / 4) + 255;

		// Main beam line
		DSurface::Temp->DrawLineBlit(
			&DSurface::ViewBounds(),
			&currentPos, &nextPos,
			&beamColor,
			segmentAlpha,
			currentDepth, nextDepth
		);

		// Parallel lines for thickness
		if (deltaX_abs > deltaY_abs)
		{
			// Horizontal beam - vertical offset
			Point2D off1Start = { currentPos.X, currentPos.Y + 1 };
			Point2D off1End = { nextPos.X, nextPos.Y + 1 };
			Point2D off2Start = { currentPos.X, currentPos.Y - 1 };
			Point2D off2End = { nextPos.X, nextPos.Y - 1 };

			DSurface::Temp->DrawLineBlit(&DSurface::ViewBounds(),
				&off1Start, &off1End, &beamColor, alpha, currentDepth, nextDepth);
			DSurface::Temp->DrawLineBlit(&DSurface::ViewBounds(),
				&off2Start, &off2End, &beamColor, alpha, currentDepth, nextDepth);
		}
		else
		{
			// Vertical beam - horizontal offset
			Point2D off1Start = { currentPos.X + 1, currentPos.Y };
			Point2D off1End = { nextPos.X + 1, nextPos.Y };
			Point2D off2Start = { currentPos.X - 1, currentPos.Y };
			Point2D off2End = { nextPos.X - 1, nextPos.Y };

			DSurface::Temp->DrawLineBlit(&DSurface::ViewBounds(),
				&off1Start, &off1End, &beamColor, alpha, currentDepth, nextDepth);
			DSurface::Temp->DrawLineBlit(&DSurface::ViewBounds(),
				&off2Start, &off2End, &beamColor, alpha, currentDepth, nextDepth);
		}

		currentPos = nextPos;
		currentDepth = nextDepth;
		accumX += deltaX;
		accumY += deltaY;
		accumDepth += deltaDepth;
		accumAlpha -= 255;
		alpha -= 25;
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x6D48F1, FakeTechnoClass::__Draw_Airstrike_Flare);
DEFINE_FUNCTION_JUMP(LJMP, 0x705860, FakeTechnoClass::__Draw_Airstrike_Flare);

CoordStruct* __fastcall FakeTechnoClass::__Get_FLH(TechnoClass* pThis, discard_t, CoordStruct* pBuffer, int weaponIndex, CoordStruct offset)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	//auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	bool allowOnTurret = true;
	bool useBurstMirroring = true;
	CoordStruct flh = CoordStruct::Empty;

	if (weaponIndex >= 0)
	{
		auto [found, _flh] = TechnoExtData::GetBurstFLH(pThis, weaponIndex);

		if (!found)
		{

			if (pThis->WhatAmI() == InfantryClass::AbsID)
			{
				auto res = TechnoExtData::GetInfantryFLH(reinterpret_cast<InfantryClass*>(pThis), weaponIndex);
				found = res.first;
				_flh = res.second;
			}

			if (!found)
			{
				_flh = pThis->GetWeapon(weaponIndex)->FLH;
			}

		}
		else
		{
			useBurstMirroring = false;
		}

		flh = _flh;
	}
	else
	{
		int index = -weaponIndex - 1;
		useBurstMirroring = false;

		if ((size_t)index < pTypeExt->AlternateFLHs.size())
			flh = pTypeExt->AlternateFLHs[index];

		if (!pTypeExt->AlternateFLH_OnTurret)
			allowOnTurret = false;
	}

	if (useBurstMirroring && pThis->CurrentBurstIndex % 2 != 0)
		flh.Y = -flh.Y;

	*pBuffer = TechnoExtData::GetFLHAbsoluteCoords(pThis, flh, allowOnTurret);
	return pBuffer;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6F3AD0, FakeTechnoClass::__Get_FLH);

int __fastcall FakeTechnoClass::__AdjustDamage(TechnoClass* pThis, discard_t,TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	int damage = 0;
	if (pTarget && !pWeapon->IsSonic && !pWeapon->UseFireParticles && pWeapon->Damage > 0)
	{

		double _damage = TechnoExtData::GetDamageMult(pThis, (double)pWeapon->Damage);
		int _damage_int = (int)TechnoExtData::GetArmorMult(pTarget, _damage, pWeapon->Warhead);
		if (_damage_int < 1)
			_damage_int = 1;

		damage = FakeWarheadTypeClass::ModifyDamage(_damage_int, pWeapon->Warhead, TechnoExtData::GetTechnoArmor(pTarget, pWeapon->Warhead), 0);
	}

	return damage;
}


DEFINE_FUNCTION_JUMP(LJMP, 0x6FDB80, FakeTechnoClass::__AdjustDamage);
DEFINE_FUNCTION_JUMP(CALL, 0x6FE61D, FakeTechnoClass::__AdjustDamage);
DEFINE_FUNCTION_JUMP(CALL, 0x7099B0, FakeTechnoClass::__AdjustDamage);

void DrawCustomCrosshair(DSurface* surface, const Point2D& center, int color) {
	// Use custom color instead of hardcoded red from original assembly
	// Original used: (255 >> RedShiftRight) << RedShiftLeft

	// Draw horizontal line
	Point2D start = { center.X - 2, center.Y };
	Point2D end = { center.X + 1, center.Y };
	surface->Draw_Line(start, end, color);

	// Draw additional horizontal lines for thickness
	start.Y = center.Y + 1;
	end.Y = center.Y + 1;
	surface->Draw_Line(start, end, color);

	start.Y = center.Y - 1;
	end.Y = center.Y + 2;
	start.X = center.X - 1;
	end.X = center.X - 1;
	surface->Draw_Line(start, end, color);

	// Draw vertical component
	start = { center.X, center.Y - 1 };
	end = { center.X, center.Y + 2 };
	surface->Draw_Line( start, end, color);
}

void __fastcall FakeTechnoClass::__DrawAirstrikeFlare(TechnoClass* pThis, discard_t, const CoordStruct& startCoord, int startHeight, int endHeight, const CoordStruct& endCoord) {
	// Convert 3D world coordinates to 2D screen pixels
	Point2D startPixel = TacticalClass::Instance->CoordsToClient(startCoord);
	Point2D endPixel = TacticalClass::Instance->CoordsToClient(endCoord);

	// Calculate Z-depth adjustments for height
	int startZ = -32 - Game::AdjustHeight(endHeight);
	int endZ = -32 - Game::AdjustHeight(startHeight);

	// Fix depth buffer value using minimum Z + adjustment
	int fixedStartZ = MinImpl(startZ, endZ) + RulesExtData::Instance()->AirstrikeLineZAdjust;
	int fixedEndZ = fixedStartZ; // Use same Z value for both to prevent depth issues

	// Calculate distances for beam direction
	int deltaX = Math::abs(endPixel.X - startPixel.X);
	int deltaY = Math::abs(endPixel.Y - startPixel.Y);

	if (!DSurface::Temp->IsDSurface())
	{
		return; // Surface not available
	}

	// Get custom color from techno extension or use global default
	auto const baseColor = GET_TECHNOTYPEEXT(pThis)->AirstrikeLineColor.Get(
		RulesExtData::Instance()->AirstrikeLineColor);

	// Apply random intensity variation (74.5% to 100%) instead of hardcoded range
	double percentage = Random2Class::Global->RandomRanged(745, 1000) / 1000.0;
	// Custom color generation instead of hardcoded random (190-270)
	ColorStruct beamColor = {
		(BYTE)(baseColor.R * percentage),
		(BYTE)(baseColor.G * percentage),
		(BYTE)(baseColor.B * percentage)
	};

	// Convert to integer format for drawing operations
	int beamColorInt = baseColor.ToInit();

	// Skip hardcoded crosshair color and use custom colors
	// Draw crosshair at end point if beam goes upward, using custom color instead of red
	if (endPixel.Y < startPixel.Y)
	{
		DrawCustomCrosshair(DSurface::Temp, endPixel, beamColorInt);
	}

	// Calculate beam direction vectors
	int directionX = endPixel.X - startPixel.X;
	int directionY = endPixel.Y - startPixel.Y;
	int directionZ = fixedEndZ - fixedStartZ; // Use fixed Z values

	// Main beam drawing loop - matches assembly loop structure
	Point2D currentPos = startPixel;
	int currentZ = fixedStartZ;
	int intensity = 100; // Initial intensity value from assembly
	int fadeDelta = 25;  // Fade step from assembly

	// Loop through 64 steps (matching assembly pattern)
	for (int step = 1; step <= 64; ++step)
	{
		// Calculate interpolated position (assembly divides by 4 in each step)
		Point2D nextPos(startPixel.X + (directionX * step) / 64 , startPixel.Y + (directionY * step) / 64);
		int nextZ = fixedStartZ + (directionZ * step) / 64;

		// Draw main beam segment using custom color
		Surface_4BEAC0_Blit(DSurface::Temp,
			DSurface::ViewBounds,
			currentPos,
			nextPos,
			beamColor,
			intensity + 255,
			currentZ,
			nextZ);

		// Draw beam thickness by adding adjacent pixels
		if (deltaX <= deltaY)
		{
			// Beam is more vertical, add horizontal thickness
			Point2D thickPos1 = { currentPos.X + 1, currentPos.Y };
			Point2D thickPos2 = { nextPos.X + 1, nextPos.Y };
			Surface_4BEAC0_Blit(DSurface::Temp, DSurface::ViewBounds,
				thickPos1, thickPos2, beamColor,
				intensity, currentZ, nextZ);

			thickPos1 = { currentPos.X - 1, currentPos.Y };
			thickPos2 = { nextPos.X - 1, nextPos.Y };
			Surface_4BEAC0_Blit(DSurface::Temp, DSurface::ViewBounds,
				thickPos1, thickPos2, beamColor,
				intensity, currentZ, nextZ);
		}
		else
		{
			// Beam is more horizontal, add vertical thickness
			Point2D thickPos1 = { currentPos.X, currentPos.Y + 1 };
			Point2D thickPos2 = { nextPos.X, nextPos.Y + 1 };
			Surface_4BEAC0_Blit(DSurface::Temp, DSurface::ViewBounds,
				thickPos1, thickPos2, beamColor,
				intensity, currentZ, nextZ);

			thickPos1 = { currentPos.X, currentPos.Y - 1 };
			thickPos2 = { nextPos.X, nextPos.Y - 1 };
			Surface_4BEAC0_Blit(DSurface::Temp, DSurface::ViewBounds,
				thickPos1, thickPos2, beamColor,
				intensity, currentZ, nextZ);
		}

		// Update for next iteration
		currentPos = nextPos;
		currentZ = nextZ;
		intensity -= fadeDelta;

		// Exit condition: intensity reaches minimum threshold (from assembly)
		if (intensity <= -765)
		{
			break;
		}
	}
}

// TODO : test
// DEFINE_FUNCTION_JUMP(LJMP, 0x705860, FakeTechnoClass::__DrawAirstrikeFlare);
// DEFINE_FUNCTION_JUMP(CALL, 0x6D48F1, FakeTechnoClass::__DrawAirstrikeFlare);

#include <Ext/Infantry/Body.h>

bool __fastcall FakeTechnoClass::__TargetSomethingNearby(TechnoClass* pThis, discard_t, CoordStruct* coord, ThreatType threat)
{
	pThis->__creationframe_4FC = Unsorted::CurrentFrame();
	auto pType = GET_TECHNOTYPE(pThis);

	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);

	const auto pRules = RulesClass::Instance();
	const bool IsHuman = pThis->Owner->IsHumanPlayer || pThis->Owner->IsControlledByHuman();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pThis->MegaMissionIsAttackMove())
	{
		delay += IsHuman
			? pTypeExt->PlayerAttackMoveTargetingDelay.Get(RulesExtData::Instance()->PlayerAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AIAttackMoveTargetingDelay.Get(RulesExtData::Instance()->AIAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}
	else if (pThis->CurrentMission == Mission::Area_Guard)
	{
		delay +=
			IsHuman
			? pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExtData::Instance()->PlayerGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay))
			: pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExtData::Instance()->AIGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));

	}
	else
	{
		delay += IsHuman
			? pTypeExt->PlayerNormalTargetingDelay.Get(RulesExtData::Instance()->PlayerNormalTargetingDelay.Get(pRules->NormalTargetingDelay))
			: pTypeExt->AINormalTargetingDelay.Get(RulesExtData::Instance()->AINormalTargetingDelay.Get(pRules->NormalTargetingDelay));
	}

	pThis->TargetingTimer.Start(delay);

	if (pTypeExt->AutoFire) {

		pThis->SetTarget(pTypeExt->AutoFire_TargetSelf ? pThis :
			static_cast<AbstractClass*>(pThis->GetCell()));
		return pThis->Target != nullptr;
	}

	// Check current target
	if (pThis->Target && pThis->ShouldLoseTargetNow)
	{
		const int weaponIndex = pThis->SelectWeapon(pThis->Target);
		const auto fire = pThis->GetFireError(pThis->Target, weaponIndex, 1);

		if (fire == FireError::CANT) {

			if(!pThis->SpawnManager) {
				pThis->SetTarget(nullptr);
			} else {
				pThis->SpawnManager->ResetTarget();
			}
		}

		if (fire == FireError::ILLEGAL || fire == FireError::RANGE) {
			pThis->SetTarget(nullptr);
		}
	}

	if (!pThis->Target) {
		if (pType->DistributedFire) {
			pThis->DistributedFire();
		} else if (const auto potentialTarget = pThis->GreatestThreat((threat & (ThreatType::Range | ThreatType::Area)).operator ThreatType(), coord, 0)) {

			pThis->SetTarget(potentialTarget);

			if (auto pPotentT = flag_cast_to<TechnoClass* ,false>(potentialTarget)) {
				const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));

				if (pWeapon && pWeapon->WeaponType && !pWeapon->WeaponType->Projectile->Inaccurate) {
					pPotentT->EstimatedHealth -= FakeTechnoClass::__AdjustDamage(pThis , discard_t() , pPotentT, pWeapon->WeaponType);

				}
			}
		}
	}

	return pThis->Target != nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x709820, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2640, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4258, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E9030, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB3F4, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4CFC, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F600C, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x6FA6DC, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x4D6F06, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x4D5392, FakeTechnoClass::__TargetSomethingNearby);

bool NOINLINE TechnoExtData::CanRetaliateICUnit(TechnoClass* pThis, FakeWeaponTypeClass* pWP, TechnoClass* pTarget)
{

	if (!pTarget->IsIronCurtained())
		return true;

	bool canAutoTarget = RulesExtData::Instance()->AutoTarget_IronCurtained;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pWP)
	{
		auto pWPExt = pWP->_GetExtData();
		if (pThis->Owner->IsControlledByHuman())
		{
			canAutoTarget = pWPExt->CanTarget_IronCurtained.Get(RulesExtData::Instance()->AutoTarget_IronCurtained);
		}
	}

	return pTypeExt->AllowFire_IroncurtainedTarget.Get(canAutoTarget);
}

int __fastcall FakeTechnoClass::_EvaluateJustCell(TechnoClass* pThis , discard_t,CellStruct* where)
{

	// /*
	// **  First, only computer objects are allowed to automatically scan for walls.
	// */
	if (pThis->Owner->IsControlledByHuman())
	{
		return 0;
	}

	// /*
	// **  Even then, if the difficulty indicates that it shouldn't search for wall
	// **  targets, then don't allow it to do so.
	// */
	if (!RulesClass::Instance->AIDiffs[(int)pThis->Owner->AIDifficulty].DestroyWalls)
	{
		return 0;
	}

	auto pCell = MapClass::Instance->GetCellAt(where);

	if (pCell->OverlayTypeIndex == -1 || !OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex]->Wall)
		return 0;

	auto pSelectedWeapon = pThis->SelectWeapon(pCell);

	if (!pThis->IsCloseEnough(pCell, pSelectedWeapon))
		return 0;

	auto pSelectedWeapon_ = pThis->GetWeapon(pSelectedWeapon);

	if (!pSelectedWeapon_ || !pSelectedWeapon_->WeaponType || !pSelectedWeapon_->WeaponType->Warhead)
		return 0;

	// /*
	// **  If the weapon cannot deal with ground based targets, then don't consider
	// **  this a valid cell target.
	// */
	if (pSelectedWeapon_->WeaponType->Projectile && !pSelectedWeapon_->WeaponType->Projectile->AG)
		return 0;

	// /*
	// **  If the primary weapon cannot destroy a wall, then don't give the cell any
	// **  value as a target.
	// */
	if (!pSelectedWeapon_->WeaponType->Warhead->Wall)
	{
		return 0;
	}

	// /*
	// **  If this is a friendly wall, then don't attack it.
	// */
	if (pCell->WallOwnerIndex == -1 || pThis->Owner->IsAlliedWith(HouseClass::Array->Items[pCell->WallOwnerIndex]))
	{
		return 0;
	}

	const double distance = (pThis->GetCoords() - CellClass::Cell2Coord(*where)).Length();

	// /*
	// **  Since a wall was found, then return a value adjusted according to the range the wall
	// **  is from the object. The greater the range, the lesser the value returned.
	// */

	return int((double)pThis->GetWeaponRange(pSelectedWeapon) - distance);
}

int TechnoExtData::CalculateBlockDamage(TechnoClass* pThis, TechnoClass* pSource,  int* pDamage, WarheadTypeClass* WH)
{
	int damage = *pDamage;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(WH);

	if (pWHExt->ImmuneToBlock)
		return damage;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (!pTypeExt->CanBlock)
		return damage;

	const auto pBlockType = pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	const auto pOtherBlock = !pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	std::vector<double>& blockChances = pBlockType->Block_Chances;
	std::vector<double>& blockDamageMultipliers = pBlockType->Block_DamageMultipliers;

	if (pWHExt->Block_AllowOverride)
	{
		blockChances = !pOtherBlock->Block_Chances.empty() ? pOtherBlock->Block_Chances : blockChances;
		blockDamageMultipliers = !pOtherBlock->Block_DamageMultipliers.empty() ? pOtherBlock->Block_DamageMultipliers : blockDamageMultipliers;
	}

	if (!pWHExt->Block_IgnoreChanceModifier)
		blockChances = TechnoExtData::GetBlockChance(pThis, blockChances);

	if ((blockChances.size() == 1 && blockChances[0] + pWHExt->Block_ExtraChance > 0.0) || blockChances.size() > 1)
	{
		// handle block conditions first
		Iterator<double> blockAffectBelowPercents = pBlockType->Block_AffectBelowPercents;
		auto blockAffectsHouses = pBlockType->Block_AffectsHouses.Get(AffectedHouse::All);
		bool blockCanActiveZeroDamage = pBlockType->Block_CanActive_ZeroDamage.Get(false);
		bool blockCanActiveNegativeDamage = pBlockType->Block_CanActive_NegativeDamage.Get(false);
		bool blockCanActivePowered = pBlockType->Block_CanActive_Powered.Get(false);
		bool blockCanActiveNoFirer = pBlockType->Block_CanActive_NoFirer.Get(true);
		bool blockCanActiveShieldActive = pBlockType->Block_CanActive_ShieldActive.Get(true);
		bool blockCanActiveShieldInactive = pBlockType->Block_CanActive_ShieldInactive.Get(true);
		bool blockCanActiveMove = pBlockType->Block_CanActive_Move.Get(true);
		bool blockCanActiveStationary = pBlockType->Block_CanActive_Stationary.Get(true);

		if (pWHExt->Block_AllowOverride)
		{
			blockAffectBelowPercents = !pOtherBlock->Block_AffectBelowPercents.empty() ? pOtherBlock->Block_AffectBelowPercents : blockAffectBelowPercents;
			blockAffectsHouses = pOtherBlock->Block_AffectsHouses.isset() ? pOtherBlock->Block_AffectsHouses.Get() : blockAffectsHouses;
			blockCanActiveZeroDamage = pOtherBlock->Block_CanActive_ZeroDamage.isset() ? pOtherBlock->Block_CanActive_ZeroDamage : blockCanActiveZeroDamage;
			blockCanActiveNegativeDamage = pOtherBlock->Block_CanActive_NegativeDamage.isset() ? pOtherBlock->Block_CanActive_NegativeDamage : blockCanActiveNegativeDamage;
			blockCanActivePowered = pOtherBlock->Block_CanActive_Powered.isset() ? pOtherBlock->Block_CanActive_Powered : blockCanActivePowered;
			blockCanActiveNoFirer = pOtherBlock->Block_CanActive_NoFirer.isset() ? pOtherBlock->Block_CanActive_NoFirer : blockCanActiveNoFirer;
			blockCanActiveShieldActive = pOtherBlock->Block_CanActive_ShieldActive.isset() ? pOtherBlock->Block_CanActive_ShieldActive : blockCanActiveShieldActive;
			blockCanActiveShieldInactive = pOtherBlock->Block_CanActive_ShieldInactive.isset() ? pOtherBlock->Block_CanActive_ShieldInactive : blockCanActiveShieldInactive;
			blockCanActiveMove = pOtherBlock->Block_CanActive_Move.isset() ? pOtherBlock->Block_CanActive_Move : blockCanActiveMove;
			blockCanActiveStationary = pOtherBlock->Block_CanActive_Stationary.isset() ? pOtherBlock->Block_CanActive_Stationary : blockCanActiveStationary;
		}

		if (blockAffectBelowPercents.size() > 0 && pThis->GetHealthPercentage() > blockAffectBelowPercents[0])
			return damage;

		if (damage == 0 && !blockCanActiveZeroDamage)
			return 0;
		else if (damage < 0 && !blockCanActiveNegativeDamage)
			return damage;

		unsigned int level = 0;

		if (blockAffectBelowPercents.size() > 0)
		{
			for (; level < blockAffectBelowPercents.size() - 1; level++)
			{
				if (pThis->GetHealthPercentage() > blockAffectBelowPercents[level + 1])
					break;
			}
		}

		double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (blockChances.size() == 1)
		{
			if (blockChances[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
				return damage;
		}
		else if (blockChances.size() <= level || blockChances[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
		{
			return damage;
		}

		if (blockCanActivePowered)
		{
			bool isActive = !(pThis->Deactivated || pThis->IsUnderEMP());

			if (isActive && pThis->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass const*>(pThis);
				isActive = pBuilding->IsPowerOnline();
			}

			if (!isActive)
				return damage;
		}

		if (auto const pFoot = flag_cast_to<FootClass*>(pThis))
		{
			if (pFoot->Locomotor->Is_Really_Moving_Now())
			{
				if (!blockCanActiveMove)
					return damage;
			}
			else if (!blockCanActiveStationary)
			{
				return damage;
			}
		}

		if (pSource)
		{
			if (pSource->Owner && !EnumFunctions::CanTargetHouse(blockAffectsHouses, pSource->Owner, pThis->Owner))
				return damage;
		}
		else if (!blockCanActiveNoFirer)
		{
			return damage;
		}

		const auto pShieldData = pExt->GetShield();

		if (pShieldData && pShieldData->IsActive())
		{
			if (!blockCanActiveShieldActive || !pShieldData->GetType()->CanBlock)
				return damage;
		}
		else if (!blockCanActiveShieldInactive)
		{
			return damage;
		}

		// a block is triggered
		Iterator <AnimTypeClass*> blockAnims = pBlockType->Block_Anims;
		auto blockWeapon = pBlockType->Block_Weapon.Get();
		bool blockFlash = pBlockType->Block_Flash.Get(false);
		bool blockReflectDamage = pBlockType->Block_ReflectDamage.Get(false);
		double blockReflectDamageChance = pBlockType->Block_ReflectDamage_Chance.Get(1.0);

		if (pWHExt->Block_AllowOverride)
		{
			if (!pOtherBlock->Block_Anims.empty())
				blockAnims = pOtherBlock->Block_Anims;

			if (pOtherBlock->Block_Weapon.isset())
				blockWeapon = pOtherBlock->Block_Weapon;

			if (pOtherBlock->Block_Flash.isset())
				blockFlash = pOtherBlock->Block_Flash;

			if (pOtherBlock->Block_ReflectDamage.isset())
				blockReflectDamage = pOtherBlock->Block_ReflectDamage;

			if (pOtherBlock->Block_ReflectDamage_Chance)
				blockReflectDamageChance = pOtherBlock->Block_ReflectDamage_Chance;

		}

		if (blockAnims.size() > 0)
		{
			const int idx = blockAnims.size() > 1 ?
				ScenarioClass::Instance->Random.RandomRanged(0, blockAnims.size() - 1) : 0;

			TechnoExtData::PlayAnim(blockAnims[idx], pThis);
		}

		if (blockFlash)
		{
			int size = pBlockType->Block_Flash_FixedSize.Get(damage * 2);
			SpotlightFlags flags = SpotlightFlags::NoColor;
			bool blockFlashRed = pBlockType->Block_Flash_Red.Get(true);
			bool blockFlashGreen = pBlockType->Block_Flash_Green.Get(true);
			bool blockFlashBlue = pBlockType->Block_Flash_Blue.Get(true);
			bool blockFlashBlack = pBlockType->Block_Flash_Black.Get(false);

			if (pWHExt->Block_AllowOverride)
			{
				size = pOtherBlock->Block_Flash_FixedSize.isset() ? pOtherBlock->Block_Flash_FixedSize.Get() : size;
				blockFlashRed = pOtherBlock->Block_Flash_Red.isset() ? pOtherBlock->Block_Flash_Red.Get() : blockFlashRed;
				blockFlashGreen = pOtherBlock->Block_Flash_Green.isset() ? pOtherBlock->Block_Flash_Green.Get() : blockFlashGreen;
				blockFlashBlue = pOtherBlock->Block_Flash_Blue.isset() ? pOtherBlock->Block_Flash_Blue.Get() : blockFlashBlue;
				blockFlashBlack = pOtherBlock->Block_Flash_Black.isset() ? pOtherBlock->Block_Flash_Black.Get() : blockFlashBlack;
			}

			if (blockFlashBlack)
			{
				flags = SpotlightFlags::NoColor;
			}
			else
			{
				if (!blockFlashRed)
					flags = SpotlightFlags::NoRed;
				if (!blockFlashGreen)
					flags |= SpotlightFlags::NoGreen;
				if (!blockFlashBlue)
					flags |= SpotlightFlags::NoBlue;
			}

			MapClass::FlashbangWarheadAt(size, WH, pThis->Location, true, flags);
		}

		if (blockReflectDamage && blockReflectDamageChance >= ScenarioClass::Instance->Random.RandomDouble()
			&& damage > 0 && pSource && !pWHExt->SuppressReflectDamage && !pWHExt->Reflected)
		{
			auto pWHRef = pBlockType->Block_ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
			auto blockReflectDamageAffectsHouses = pBlockType->Block_ReflectDamage_AffectsHouses.Get(blockAffectsHouses);
			double blockReflectDamageMultiplier = pBlockType->Block_ReflectDamage_Multiplier.Get(1.0);
			bool blockReflectDamageWHDetonate = pBlockType->Block_ReflectDamage_Warhead_Detonate.Get(false);
			Nullable<int>* overrider_2 = nullptr;

			if (pWHExt->Block_AllowOverride)
			{
				pWHRef = pOtherBlock->Block_ReflectDamage_Warhead.isset() ? pOtherBlock->Block_ReflectDamage_Warhead.Get() : pWHRef;
				overrider_2  = &pOtherBlock->Block_ReflectDamage_Override;
				blockReflectDamageAffectsHouses = pOtherBlock->Block_ReflectDamage_AffectsHouses.isset() ? pOtherBlock->Block_ReflectDamage_AffectsHouses.Get() : blockReflectDamageAffectsHouses;
				blockReflectDamageMultiplier = pOtherBlock->Block_ReflectDamage_Multiplier.isset() ? pOtherBlock->Block_ReflectDamage_Multiplier.Get() : blockReflectDamageMultiplier;
				blockReflectDamageWHDetonate = pOtherBlock->Block_ReflectDamage_Warhead_Detonate.isset() ? pOtherBlock->Block_ReflectDamage_Warhead_Detonate.Get() : blockReflectDamageWHDetonate;
			}

			int damageRef = pBlockType->Block_ReflectDamage_Override.Get(static_cast<int>(damage * blockReflectDamageMultiplier));

			if(overrider_2 && overrider_2->isset())
				damageRef = overrider_2->Get();

			if (EnumFunctions::CanTargetHouse(blockReflectDamageAffectsHouses, pThis->Owner, pSource->Owner))
			{
				auto const pWHExtRef = WarheadTypeExtContainer::Instance.Find(pWHRef);
				pWHExtRef->Reflected = true;

				if (blockReflectDamageWHDetonate)
					WarheadTypeExtData::DetonateAt(pWHRef, pSource, pThis, damageRef, pThis->Owner);
				else if(pSource->IsAlive && pSource->Health > 0)
					pSource->ReceiveDamage(&damage, 0, pWHRef, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}
		}

		if (blockDamageMultipliers.size() == 1)
			damage = static_cast<int>(damage * (blockDamageMultipliers[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));
		else if (blockDamageMultipliers.size() > level)
			damage = static_cast<int>(damage * (blockDamageMultipliers[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));

		if (blockWeapon)
			TechnoExtData::FireWeaponAtSelf(pThis, blockWeapon);
	}

	return damage;
}

std::vector<double> TechnoExtData::GetBlockChance(TechnoClass* pThis, std::vector<double>& blockChance)
{
	std::vector<double> result = blockChance;
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (result.empty())
		result.push_back(0.0);

	double extraChance = 0.0;

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (pType->Block_ChanceMultiplier == 1.0 && pType->Block_ExtraChance == 0.0)
			continue;

		for (auto& chance : result) {
			chance *= MaxImpl(pType->Block_ChanceMultiplier, 0.0);
		}

		extraChance += pType->Block_ExtraChance;
	}

	for (auto& chance : result) {
		chance += extraChance;
	}

	return result;
}

void TechnoExtData::AddFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker)
{
	if (Attacker->InLimbo || !Attacker->IsAlive || Attacker->IsCrashing || Attacker->IsSinking)
		return;

	const int index = this->FindFirer(Weapon);
	const OnlyAttackStruct Data { Weapon ,Attacker };

	if (index < 0)
	{
		this->OnlyAttackData.push_back(Data);
	}
	else
	{
		this->OnlyAttackData[index] = Data;
	}
}

bool TechnoExtData::ContainFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker) const
{
	const int index = this->FindFirer(Weapon);

	if (index >= 0)
		return this->OnlyAttackData[index].Attacker == Attacker;

	return true;
}

int TechnoExtData::FindFirer(WeaponTypeClass* const Weapon) const
{
	const auto& AttackerDatas = this->OnlyAttackData;
	if (!AttackerDatas.empty())
	{
		for (int index = 0; index < int(AttackerDatas.size()); index++)
		{
			const auto pWeapon = AttackerDatas[index].Weapon;

			if (pWeapon == Weapon && AttackerDatas[index].Attacker)
				return index;
		}
	}

	return -1;
}

bool TechnoExtData::MultiWeaponCanFire(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pWeaponType || pWeaponType->NeverUse
		|| (pThis->InOpenToppedTransport && !pWeaponType->FireInTransport))
	{
		return false;
	}

	const auto rtti = pTarget->WhatAmI();
	const bool isBuilding = rtti == AbstractType::Building;
	const auto pWH = pWeaponType->Warhead;
	const auto pBulletType = pWeaponType->Projectile;

	const auto pTechno = flag_cast_to<TechnoClass*, true>(pTarget);
	const bool isInAir = pTechno ? pTechno->IsInAir() : false;

	const auto pOwner = pThis->Owner;
	const auto pTechnoOwner = pTechno ? pTechno->Owner : nullptr;
	const bool isAllies = pTechnoOwner ? pOwner->IsAlliedWith(pTechnoOwner) : false;

	if (isInAir)
	{
		if (!pBulletType->AA)
			return false;
	}
	else
	{
		if (BulletTypeExtContainer::Instance.Find(pBulletType)->AAOnly.Get())
		{
			return false;
		}
		else if (pWH->ElectricAssault)
		{
			if (!isBuilding || !isAllies
				|| !static_cast<BuildingClass*>(pTarget)->Type->Overpowerable)
			{
				return false;
			}
		}
		else if (pWH->IsLocomotor)
		{
			if (isBuilding)
				return false;
		}
	}

	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTechno || !isInAir)
	{
		if (auto const pObject = flag_cast_to<ObjectClass*, true>(pTarget))
			pTargetCell = pObject->GetCell();
		else if (auto const pCell = cast_to<CellClass*, true>(pTarget))
			pTargetCell = pCell;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pWeaponExt->SkipWeaponPicking)
	{
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true, true))
			return false;

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget, false)
				|| !EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTechnoOwner)
				|| !TechnoExtData::ObjectHealthAllowFiring(pTechno, pWeaponType)
				|| !pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
			{
				return false;
			}
		}
	}

	//if (PassengersFunctional::CanFire(pThis))
	//	return false;

	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeaponType->Warhead))
		return false;

	if (!TechnoExtData::InterceptorAllowFiring(pThis, pTechno))
		return false;

	if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
		if (GeneralUtils::GetWarheadVersusArmor(pWH, TechnoExtData::GetTechnoArmor(pObj, pWH)) == 0.0)
			return false;
	}

	if (pTechno)
	{
		auto pTechnoType = GET_TECHNOTYPE(pTechno);

		if (pTechnoType->Immune && !pWHExt->IsFakeEngineer) {
			return false;
		}

		if (pTechno->IsIronCurtained()
			&& !pWeaponExt->CanTarget_IronCurtained.Get(pThis->Owner->IsControlledByHuman() ? RulesExtData::Instance()->CanTarget_IronCurtained : RulesExtData::Instance()->CanTargetAI_IronCurtained))
			return false;

		if (pThis->Berzerk && !EnumFunctions::CanTargetHouse(RulesExtData::Instance()->BerzerkTargeting, pThis->Owner, pTechno->Owner))
			return false;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pTechno, pWeaponType))
			return false;

		if (pTechno->AttachedBomb ? pWH->IvanBomb : pWH->BombDisarm)
			return false;

		if (!pWH->Temporal && pTechno->BeingWarpedOut)
			return false;

		if (pWH->Parasite
			&& (isBuilding || static_cast<FootClass*>(pTechno)->ParasiteEatingMe))
		{
			return false;
		}

		if (pWH->MindControl
			&& (pTechnoType->ImmuneToPsionics || pTechno->IsMindControlled() || pOwner == pTechnoOwner))
		{
			return false;
		}

		if (pWeaponType->DrainWeapon
			&& (!pTechnoType->Drainable || pTechno->DrainingMe || isAllies))
		{
			return false;
		}

		if (pWH->Airstrike)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, WarheadTypeExtContainer::Instance.Find(pWH)->AirstrikeTargets, false))
				return false;

			const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

			if (pTechno->AbstractFlags & AbstractFlags::Foot)
			{
				if (!pTechnoTypeExt->AllowAirstrike.Get(true))
					return false;
			}
			else if (pTechnoTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTechnoType)->CanC4)
				&& (!pTechnoType->ResourceDestination || !pTechnoType->ResourceGatherer))
			{
				return false;
			}
		}
	}
	else if (rtti == AbstractType::Cell)
	{
		if (pTargetCell->OverlayTypeIndex >= 0)
		{
			const auto pOverlayType = OverlayTypeClass::Array->Items[pTargetCell->OverlayTypeIndex];

			if (pOverlayType->Wall && !pWH->Wall && (!pWH->Wood || pOverlayType->Armor != Armor::Wood))
				return false;
		}
	}
	else if (rtti == AbstractType::Terrain)
	{
		if (!pWH->Wood)
			return false;
	}

	return true;
}

bool TechnoExtData::IsHealthInThreshold(ObjectClass* pObject, double min, double max) {

	if (!pObject->Health || !pObject->IsAlive)
		return false;

	double hp = pObject->GetHealthPercentage();
	return (hp > 0 ? hp > min : hp >= min) && hp <= max;
}

std::tuple<bool, bool, bool> TechnoExtData::CanBeAffectedByFakeEngineer(TechnoClass* pThis, TechnoClass* pTarget, bool checkBridge, bool checkCapturableBuilding, bool checkAttachedBombs) {

	const int nWeaponIndex = pThis->SelectWeapon(pTarget);

	if (nWeaponIndex < 0)
		return { false , false , false };

	const auto pWeapon = pThis->GetWeapon(nWeaponIndex)->WeaponType;

	if (!pWeapon)
		return { false , false , false };

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
	bool canAffectCapturableBuildings = false;
	bool canAffectBridges = false;
	bool canAffectAttachedBombs = false;

	//Check if an attached bomb can be disarmed
	if (checkAttachedBombs
		&& pWHExt->FakeEngineer_BombDisarm
		&& pTarget->AttachedBomb)
	{
		canAffectAttachedBombs = true;
	}

	const auto pBuilding = cast_to<BuildingClass* , false>(pTarget);
	bool isBuilding = pBuilding && pBuilding->IsAlive && pBuilding->Health > 0;

	// Check if a Bridge Repair Hut can be affected
	if (checkBridge && isBuilding && pBuilding->Type->BridgeRepairHut)
	{
		CellStruct bridgeRepairHutCell = CellClass::Coord2Cell(pBuilding->GetCenterCoords());
		bool isBridgeDamaged = MapClass::Instance->IsLinkedBridgeDestroyed(bridgeRepairHutCell);

		if ((isBridgeDamaged && pWHExt->FakeEngineer_CanRepairBridges)
		||  (!isBridgeDamaged && pWHExt->FakeEngineer_CanDestroyBridges)) {
			canAffectBridges = true;
		}
	}

	// Check if a capturable building can be affected
	if (checkCapturableBuilding
		&& isBuilding
		&& pWHExt->FakeEngineer_CanCaptureBuildings
		&& (pBuilding->Type->Capturable || pBuilding->Type->NeedsEngineer)
		&& !pThis->Owner->IsAlliedWith(pBuilding)) // Anti-crash check
	{
		canAffectCapturableBuildings = true;
	}

	return  { canAffectCapturableBuildings , canAffectBridges , canAffectAttachedBombs };
}

bool TechnoExtData::CannotMove(UnitClass* pThis)
{
	const auto pType = pThis->Type;

	if (pType->Speed <= 0)
		return true;

	const auto movementRestrictedTo = pType->MovementRestrictedTo;

	if (movementRestrictedTo == LandType::None)
		return false;

	auto landType = pThis->GetCell()->LandType;

	if (landType == LandType::Tunnel)
		return false;

	if (pThis->OnBridge && (landType == LandType::Water || landType == LandType::Beach))
		landType = LandType::Road;

	if (movementRestrictedTo != landType)
		return true;

	return false;
}

// Check adjacent cells from the center
// The current MapClass::Instance->PlacePowerupCrate(...) doesn't like slopes and maybe other cases
bool TechnoExtData::TryToCreateCrate(CoordStruct location, PowerupEffects selectedPowerup, int maxCellRange)
{
	CellStruct centerCell = CellClass::Coord2Cell(location);
	short currentRange = 0;
	bool placed = false;

	do
	{
		short x = -currentRange;
		short y = -currentRange;

		CellStruct checkedCell;
		checkedCell.Y = centerCell.Y + y;

		// Check upper line
		for (short i = -currentRange; i <= currentRange; i++)
		{
			checkedCell.X = centerCell.X + i;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.Y = centerCell.Y + Math::abs(y);

		// Check lower line
		for (short i = -currentRange; i <= currentRange; i++)
		{
			checkedCell.X = centerCell.X + i;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.X = centerCell.X + x;

		// Check left line
		for (short j = -currentRange + 1; j < currentRange; j++)
		{
			checkedCell.Y = centerCell.Y + j;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.X = centerCell.X + Math::abs(x);

		// Check right line
		for (short j = -currentRange + 1; j < currentRange; j++)
		{
			checkedCell.Y = centerCell.Y + j;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		currentRange++;
	}
	while (!placed && currentRange < maxCellRange);

	if (!placed)
		Debug::LogInfo(__FUNCTION__": Failed to place a crate in the cell ({},{}) and around that location.", centerCell.X, centerCell.Y, maxCellRange);

	return placed;
}

void TechnoExtData::UpdateRecountBurst() {
	const auto pThis = This();
	auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pThis->CurrentBurstIndex && !pThis->Target && pTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst)) {
		const auto pWeapon = this->LastWeaponType;
		if (pWeapon && pWeapon->Burst && pThis->LastFireBulletFrame + MaxImpl(pWeapon->ROF, 30) <= Unsorted::CurrentFrame) {


			const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pWeapon->Burst;
			const auto rof = static_cast<int>(ratio * pWeapon->ROF * this->AE.ROFMultiplier) - std::max(pWeapon->ROF, 30);

			if (rof > 0) {
				pThis->ROF = rof;
				pThis->RearmTimer.Start(rof);
			}

			pThis->CurrentBurstIndex = 0;
		}
	}
}

void TechnoExtData::UpdateRearmInEMPState()
{
	const auto pThis = This();
	const bool underEMP = pThis->IsUnderEMP();

	if (!underEMP && !pThis->Deactivated)
		return;

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_UnderEMP.Get(RulesExtData::Instance()->NoRearm_UnderEMP))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_UnderEMP.Get(RulesExtData::Instance()->NoReload_UnderEMP))
		pThis->ReloadTimer.StartTime++;

		// Pause building factory production under EMP / Deactivated (AI only)
	if (auto const pBuilding = cast_to<BuildingClass*,false>(pThis)) {
		if (pBuilding->Owner && !pBuilding->Owner->IsControlledByHuman()) {
			if (auto const pFactory = pBuilding->Factory) {
				if (pFactory->Production.Timer.InProgress()) {
					pFactory->Production.Timer.StartTime++;
				}
			}
		}
	}
}

void TechnoExtData::UpdateRearmInTemporal()
{
	const auto pThis = This();
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_Temporal.Get(RulesExtData::Instance()->NoRearm_Temporal))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_Temporal.Get(RulesExtData::Instance()->NoReload_Temporal))
		pThis->ReloadTimer.StartTime++;
}

void TechnoExtData::ResetDelayedFireTimer()
{
	this->DelayedFireTimer.Stop();
	this->DelayedFireWeaponIndex = -1;

	if (this->CurrentDelayedFireAnim) {
		if (AnimExtContainer::Instance.Find(this->CurrentDelayedFireAnim)->DelayedFireRemoveOnNoDelay)
			this->CurrentDelayedFireAnim.reset(nullptr);
	}
}

void TechnoExtData::CreateDelayedFireAnim(AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool onTurret, CoordStruct firingCoords)
{
	if (pAnimType)
	{
		auto coords = This()->GetCenterCoords();

		if (!center)
			coords = TechnoExtData::GetFLHAbsoluteCoords(This(), firingCoords, onTurret);

		auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);

		if (attach)
			pAnim->SetOwnerObject(This());

		AnimExtData::SetAnimOwnerHouseKind(pAnim, This()->Owner,nullptr,This() , false, false);

		auto const pAnimExt = AnimExtContainer::Instance.Find(pAnim);

		if (attach)
		{
			pAnimExt->DelayedFireRemoveOnNoDelay = removeOnNoDelay;
			this->CurrentDelayedFireAnim.reset(pAnim);
		}
	}
}

bool TechnoExtData::HandleDelayedFireWithPauseSequence(TechnoClass* pThis, WeaponTypeClass* pWeapon, int weaponIndex, int frame, int firingFrame)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto& timer = pExt->DelayedFireTimer;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
	{
		pExt->ResetDelayedFireTimer();
		pExt->DelayedFireSequencePaused = false;
	}

	if (pWeaponExt->DelayedFire_PauseFiringSequence && pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
	{
		if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
		{
			if (frame == firingFrame)
				pExt->DelayedFireSequencePaused = true;

			if (!timer.HasStarted())
			{
				pExt->DelayedFireWeaponIndex = weaponIndex;
				timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
				auto pAnimType = pWeaponExt->DelayedFire_Animation;

				if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
					pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation.Get();

				auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

				if (pWeaponExt->DelayedFire_AnimOffset.isset())
					firingCoords = pWeaponExt->DelayedFire_AnimOffset;

				pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

				if (pWeaponExt->DelayedFire_InitialBurstSymmetrical)
					pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
						pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, {firingCoords.X, -firingCoords.Y, firingCoords.Z});

				return true;
			}
			else if (timer.InProgress())
			{
				return true;
			}

			if (timer.Completed())
				pExt->ResetDelayedFireTimer();
		}

		pExt->DelayedFireSequencePaused = false;
	}

	return false;
}

void TechnoExtData::UpdateGattlingRateDownReset()
{
	const auto pThis = This();
	auto pType = GET_TECHNOTYPE(pThis);

	if (pType->IsGattling)
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->RateDown_Reset
				&& (!pThis->Target || this->LastTargetID != pThis->Target->UniqueID))
		{
			int oldStage = pThis->CurrentGattlingStage;
			this->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			this->AccumulatedGattlingValue = 0;
			this->ShouldUpdateGattlingValue = false;

			if (oldStage != 0)
			{
				pThis->GattlingRateDown(0);
			}
		}
	}
}

void TechnoExtData::SetChargeTurretDelay(TechnoClass* pThis, int rearmDelay, WeaponTypeClass* pWeapon)
{
	pThis->ROF = rearmDelay;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!pWeaponExt->ChargeTurret_Delays.empty())
	{
		size_t burstIndex = pWeapon->Burst > 1 ? pThis->CurrentBurstIndex - 1 : 0;
		size_t index = burstIndex < pWeaponExt->ChargeTurret_Delays.size() ? burstIndex : pWeaponExt->ChargeTurret_Delays.size() - 1;
		int delay = pWeaponExt->ChargeTurret_Delays[index];

		if (delay <= 0)
			return;

		pThis->ROF = delay;
		TechnoExtContainer::Instance.Find(pThis)->ChargeTurretTimer.Start(delay);
	}
}

void TechnoExtData::ApplyKillWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if ((!pWHExt->KillWeapon  && !pWHExt->KillWeapon_OnFirer) || pTypeExt->SuppressKillWeapons)
		return;

	const auto& filter = pTypeExt->SuppressKillWeapons_Types;

	// KillWeapon can be triggered without the source
	if (pWHExt->KillWeapon && (!pSource || EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_AffectsHouses, pSource->Owner, pThis->Owner))) {
		if ((filter.empty() || !filter.Contains(pWHExt->KillWeapon)) && EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_Affects, false))
		{
			WeaponTypeExtData::DetonateAt2(pWHExt->KillWeapon, pThis, pSource, pWHExt->KillWeapon->Damage, false, nullptr);
		}
	}

	// KillWeapon.OnFirer must have a source
	if (pWHExt->KillWeapon_OnFirer && pSource && EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_OnFirer_AffectsHouses, pSource->Owner, pThis->Owner)) {
		if ((filter.empty() || !filter.Contains(pWHExt->KillWeapon_OnFirer)) && EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_Affects, false))
		{
			WeaponTypeExtData::DetonateAt2(pWHExt->KillWeapon_OnFirer, pThis, pSource, pWHExt->KillWeapon->Damage, false, nullptr);
		}
	}
}

// Checks if vehicle can deploy into a building at its current location. If unit has no DeploysInto set returns noDeploysIntoDefaultValue (def = false) instead.
bool TechnoExtData::CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue)
{
	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return noDeploysIntoDefaultValue;

	bool canDeploy = true;
	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	pThis->Mark(MarkType::Remove);

	pThis->Locomotor.GetInterfacePtr()->Mark_All_Occupation_Bits((int)MarkType::Remove);

	if (!pDeployType->CanCreateHere(mapCoords, pThis->Owner))
		canDeploy = false;

	pThis->Locomotor.GetInterfacePtr()->Mark_All_Occupation_Bits((int)MarkType::Put);
	pThis->Mark(MarkType::Put);

	return canDeploy;
}

bool TechnoExtData::CanDeployIntoBuilding(UnitClass* pThis)
{
	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return true;

	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	// The vanilla game used an inappropriate approach here, resulting in potential risk of desync.
	// Now, through additional checks, we can directly exclude the unit who want to deploy.
	TechnoExtData::Deployer = pThis;
	const bool canDeploy = pDeployType->CanCreateHere(mapCoords, pThis->Owner);
	TechnoExtData::Deployer = nullptr;

	return canDeploy;
}

void TechnoExtData::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (!pTechnoTo || TechnoExtData::IsPsionicsImmune(pTechnoTo))
		return;

	const auto pBld = cast_to<BuildingClass*, false>(pTechnoTo);

	// anim must be transfered before `Free` call , because it will get invalidated !
	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		pTechnoFrom->MindControlRingAnim = nullptr;

		// kill previous anim if any
		if (pTechnoTo->MindControlRingAnim)
		{
			GameDelete<true, false>(pTechnoTo->MindControlRingAnim);
			//pTechnoTo->MindControlRingAnim->TimeToDie = true;
			//pTechnoTo->MindControlRingAnim->UnInit();
		}

		CoordStruct location = pTechnoTo->GetCoords();

		if (pBld)
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		else
			location.Z += GET_TECHNOTYPE(pTechnoTo)->MindControlRingOffset;

		Anim->SetLocation(location);
		Anim->SetOwnerObject(pTechnoTo);

		if (pBld)
			Anim->ZAdjust = -1024;

		pTechnoTo->MindControlRingAnim = Anim;
	}

	if (const auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}
	else if (pTechnoTo->MindControlledByAUnit && !pTechnoFrom->MindControlledBy)
	{
		pTechnoTo->MindControlledByAUnit = pTechnoFrom->MindControlledByAUnit; //perma MC ed
	}
	else if (auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (auto Manager = (FakeCaptureManagerClass*)Controller->CaptureManager)
		{
			const bool Succeeded =
				Manager->__FreeUnit(pTechnoFrom, true)
				&& Manager->__CaptureUnit(pTechnoTo, false, true, nullptr, 0);

			if (Succeeded)
			{
				TechnoExtContainer::Instance.Find(pTechnoTo)->BeControlledThreatFrame = TechnoExtContainer::Instance.Find(pTechnoFrom)->BeControlledThreatFrame;

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

Point2D TechnoExtData::GetScreenLocation(TechnoClass* pThis)
{
	CoordStruct absolute = pThis->GetCoords();
	Point2D position = TacticalClass::Instance->CoordsToScreen(absolute);
	position -= TacticalClass::Instance->TacticalPos;

	return position;
}

Point2D TechnoExtData::GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor)
{
	int length = 17;
	Point2D position = GetScreenLocation(pThis);

	if (pThis->WhatAmI() == AbstractType::Infantry)
		length = 8;

	RectangleStruct bracketRect =
	{
		position.X - length + (length == 8) + 1,
		position.Y - 28 + (length == 8),
		length * 2,
		length * 3
	};

	return anchor.OffsetPosition(bracketRect);
}

Point2D TechnoExtData::GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition , Point2D offset)
{
	const auto pBuildingType = static_cast<BuildingClass*>(pThis)->Type;
	Point2D position = GetScreenLocation(pThis);
	CoordStruct dim2 = CoordStruct::Empty;
	pBuildingType->Dimension2(&dim2);
	dim2 = { -dim2.X / 2, dim2.Y / 2, dim2.Z };
	Point2D positionFix = TacticalClass::Instance->CoordsToScreen(dim2);

	const int foundationWidth = pBuildingType->GetFoundationWidth();
	const int foundationHeight = pBuildingType->GetFoundationHeight(false);
	const int height = pBuildingType->Height * 12;
	const int lengthW = foundationWidth * 7 + foundationWidth / 2;
	const int lengthH = foundationHeight * 7 + foundationHeight / 2;

	position.X += positionFix.X + 3 + lengthH * 4;
	position.Y += positionFix.Y + 4 - lengthH * 2;

	switch (bracketPosition)
	{
	case BuildingSelectBracketPosition::Top:
		break;
	case BuildingSelectBracketPosition::LeftTop:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2;
		break;
	case BuildingSelectBracketPosition::LeftBottom:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::Bottom:
		position.Y += lengthW * 2 + lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightBottom:
		position.X += lengthW * 4;
		position.Y += lengthW * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightTop:
		position.X += lengthW * 4;
		position.Y += lengthW * 2;
		break;
	default:
		break;
	}

		return position + offset;
	}

std::vector<DigitalDisplayTypeClass*>* TechnoExtData::GetDisplayType(TechnoClass* pThis, TechnoTypeClass* pType, int& length) {
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			return &RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			return &RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Unit:
		{
			return &RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Aircraft:
		{
			return &RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes;
		}
		default:
		{
			return nullptr;
		}

		}
	}

	return &pTypeExt->DigitalDisplayTypes;
}

static bool GetDisplayTypeData(std::vector<DigitalDisplayTypeClass*>* ret , TechnoClass* pThis , TechnoTypeClass* pType, int& length)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			ret = &RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
			return true;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			ret = &(RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Unit:
		{
			ret = &(RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Aircraft:
		{
			ret = &(RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes);
			return true;
		}
		default:
		{
			return false;
		}

		}
	}

	ret = &(pTypeExt->DigitalDisplayTypes);
	return true;
}

void TechnoExtData::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	auto pType = GET_TECHNOTYPE(pThis);

	if (TechnoTypeExtContainer::Instance.Find(pType)->DigitalDisplay_Disable)
		return;

	int length = 17;
	if (const auto DisplayTypes = TechnoExtData::GetDisplayType(pThis, pType, length)) {

		const auto pExt = TechnoExtContainer::Instance.Find(pThis);
		const auto What = pThis->WhatAmI();
		const bool isBuilding = What == AbstractType::Building;
		const bool isInfantry = What == AbstractType::Infantry;
		ShieldClass* pShield = pExt->GetShield();
		if (pShield && pShield->IsBrokenAndNonRespawning()) {
			pShield = nullptr;
		}

		for (auto &pDisplayType : *DisplayTypes) {

			if (!pDisplayType->CanShow(pThis))
				continue;

			int value = -1;
			int maxValue = 0;

			TechnoExtData::GetValuesForDisplay(pThis, pDisplayType->InfoType, value, maxValue, pDisplayType->InfoIndex, pShield);

			if (value <= -1 || maxValue <= 0)
				continue;

			const auto divisor = pDisplayType->ValueScaleDivisor.Get(pDisplayType->ValueAsTimer ? 15 : 1);

			if (divisor > 1) {
				value = MaxImpl(value / divisor, value ? 1 : 0);
				maxValue = MaxImpl(maxValue / divisor, 1);
			}

			Point2D position = isBuilding ? GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
				: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);

			position.Y += pType->PixelSelectionBracketDelta;

			if (pDisplayType->InfoType == DisplayInfoType::Shield)
				position.Y += pExt->CurrentShieldType->BracketDelta;

			pDisplayType->Draw(position, length, value, maxValue, isBuilding, isInfantry, pShield);
		}
	}
}

void GetDigitalDisplayFakeHealth(TechnoClass* pThis, int& value, int& maxValue) {
	if (GET_TECHNOTYPEEXT(pThis)->DigitalDisplay_Health_FakeAtDisguise) {
		if(auto pType = type_cast<TechnoTypeClass*>(pThis->Disguise)){
			const int newMaxValue = pType->Strength;
			const double ratio = static_cast<double>(value) / maxValue;
			value = static_cast<int>(ratio * newMaxValue);
			maxValue = newMaxValue;
		}
	}
}

// https://github.com/Phobos-developers/Phobos/pull/1287
// TODO : update
void TechnoExtData::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex, ShieldClass* pShield)
{
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	case DisplayInfoType::Shield:
	{
		if (!pShield)
			return;

		value = pShield->GetHP();
		maxValue = pShield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		if (pType->Ammo <= 0)
			return;

		value = pThis->Ammo;
		maxValue = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		if (!pThis->CaptureManager)
			return;

		value = ((FakeCaptureManagerClass*)pThis->CaptureManager)->__GetControlledCount();
		maxValue = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		if (infoIndex == 1)
			value = pSpawnManager->CountDockedSpawns();
		else if (infoIndex == 2)
			value = pSpawnManager->CountLaunchingSpawns();
		else
			value = pSpawnManager->CountAliveSpawns();

		maxValue = pType->SpawnsNumber;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		if (pType->Passengers <= 0)
			return;

		value = TechnoTypeExtContainer::Instance.Find(pType)->Passengers_BySize ? pThis->Passengers.NumPassengers : pThis->Passengers.GetTotalSize();
		maxValue = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (pType->Storage <= 0)
			return;

		auto& tib = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

		if (infoIndex && infoIndex <= TiberiumClass::Array->Count)
			value = static_cast<int>(tib.GetAmount(infoIndex - 1));
		else
			value = static_cast<int>(tib.GetAmounts());

		maxValue = pType->Storage;
		break;
	}
	case DisplayInfoType::Experience:
	{
		if (!pType->Trainable)
			return;

		value = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetActualCost(pThis->Owner));
		maxValue = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetActualCost(pThis->Owner));
		break;
	}
	case DisplayInfoType::Occupants:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (!pBuildingType->CanBeOccupied)
			return;

		value = pBuilding->Occupants.Count;
		maxValue = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->GattlingValue ? pThis->CurrentGattlingStage + 1 : 0;
		maxValue = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::IronCurtain:
	{
		if (!pThis->IsIronCurtained())
			return;

		value = pThis->IronCurtainTimer.GetTimeLeft();
		maxValue = pThis->IronCurtainTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::TemporalLife:
	{
		const auto pTemporal = pThis->TemporalTargetingMe;

		if (!pTemporal)
			return;

		value = pTemporal->WarpRemaining;
		maxValue = pType->Strength * 10;
		break;
	}
	case DisplayInfoType::DisableWeapon:
	{
		auto& nTimer = pExt->DisableWeaponTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::SelfHealCombatDelay:
	{
		auto& nTimer = pExt->SelfHealing_CombatDelay;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::CloakDisable :
	{
		auto& nTimer = pExt->CloakSkipTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::GattlingCount:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->GattlingValue;
		maxValue = (pThis->Veterancy.IsElite() ? pType->EliteStage : pType->WeaponStage)[pThis->CurrentGattlingStage];
		break;
	}
	case DisplayInfoType::ROF:
	{
		if (!pThis->IsArmed())
			return;

		value = pThis->RearmTimer.GetTimeLeft();
		maxValue = pThis->ROF;
		break;
	}
	case DisplayInfoType::Reload:
	{
		if (pType->Ammo <= 0)
			return;

		value = (pThis->Ammo >= pType->Ammo) ? 0 : pThis->ReloadTimer.GetTimeLeft();
		maxValue = pThis->ReloadTimer.TimeLeft ? pThis->ReloadTimer.TimeLeft : ((pThis->Ammo || pType->EmptyReload <= 0) ? pType->Reload : pType->EmptyReload);
		break;
	}
	case DisplayInfoType::FactoryProcess:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		auto getFactory = [pThis, pType, infoIndex]() -> FactoryClass*
			{
				const auto pHouse = pThis->Owner;
				const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);

				if (infoIndex == 1)
				{
					if (!pHouse->IsControlledByHuman())
						return static_cast<BuildingClass*>(pThis)->Factory;
					else if (pThis->IsPrimaryFactory)
						return pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);
				}
				else if (infoIndex == 2)
				{
					if (pHouse->IsControlledByHuman() && pThis->IsPrimaryFactory && pBuildingType->Factory == AbstractType::BuildingType)
						return pHouse->Primary_ForDefenses;
				}
				else if (!pHouse->IsControlledByHuman())
				{
					return static_cast<BuildingClass*>(pThis)->Factory;
				}
				else if (pThis->IsPrimaryFactory)
				{
					const auto pFactory = pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);

					if (pFactory && pFactory->Object)
						return pFactory;
					else if (pBuildingType->Factory == AbstractType::BuildingType)
						return pHouse->Primary_ForDefenses;
				}

				return nullptr;
			};
		if (const auto pFactory = getFactory())
		{
			if (pFactory->Object)
			{
				value = pFactory->GetProgress();
				maxValue = 54;
			}
		}

		break;
	}
	case DisplayInfoType::SpawnTimer:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		if (infoIndex && infoIndex <= pSpawnManager->SpawnedNodes.Count)
		{
			value = pSpawnManager->SpawnedNodes[infoIndex - 1]->NodeSpawnTimer.GetTimeLeft();
		}
		else
		{
			for (int i = 0; i < pSpawnManager->SpawnedNodes.Count; ++i)
			{
				const auto pSpawnNode = pSpawnManager->SpawnedNodes[i];

				if (pSpawnNode->Status == SpawnNodeStatus::Dead)
				{
					const int time = pSpawnNode->NodeSpawnTimer.GetTimeLeft();

					if (!value || time < value)
						value = time;
				}
			}
		}

		maxValue = pSpawnManager->RegenRate;
		break;
	}
	case DisplayInfoType::GattlingTimer:
	{
		if (!pType->IsGattling)
			return;

		const auto thisStage = pThis->CurrentGattlingStage;
		const auto& stage = pThis->Veterancy.IsElite() ? pType->EliteStage : pType->WeaponStage;

		value = pThis->GattlingValue;
		maxValue = stage[thisStage];

		if (thisStage > 0)
		{
			value -= stage[thisStage - 1];
			maxValue -= stage[thisStage - 1];
		}

		break;
	}
	case DisplayInfoType::ProduceCash:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (pBuildingType->ProduceCashAmount <= 0)
			return;

		value = pBuilding->CashProductionTimer.GetTimeLeft();
		maxValue = pBuilding->CashProductionTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::PassengerKill:
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (!pTypeExt->PassengerDeletionType.Enabled)
			return;

		value = pExt->PassengerDeletionTimer.GetTimeLeft();
		maxValue = pExt->PassengerDeletionTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::AutoDeath:
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->Death_Method == KillMethod::None)
			return;

		if (pTypeExt->Death_Countdown > 0)
		{
			value = pExt->Death_Countdown.GetTimeLeft();
			maxValue = pExt->Death_Countdown.TimeLeft;
		}
		else if (pTypeExt->Death_NoAmmo && pType->Ammo > 0)
		{
			value = pThis->Ammo;
			maxValue = pType->Ammo;
		}

		break;
	}
	case DisplayInfoType::SuperWeapon:
	{
		if (pThis->WhatAmI() != AbstractType::Building || !pThis->Owner)
			return;

		auto getSuperTimer = [pThis, pType, infoIndex]() -> CDTimerClass*
			{
				const auto pHouse = pThis->Owner;
				const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
				const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

				if (infoIndex && infoIndex <= pBuildingTypeExt->GetSuperWeaponCount())
				{
					if (infoIndex == 1)
					{
						if (pBuildingType->SuperWeapon != -1)
							return &pHouse->Supers.operator[](pBuildingType->SuperWeapon)->RechargeTimer;
					}
					else if (infoIndex == 2)
					{
						if (pBuildingType->SuperWeapon2 != -1)
							return &pHouse->Supers.operator[](pBuildingType->SuperWeapon2)->RechargeTimer;
					}
					else
					{
						const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
						return &pHouse->Supers.operator[](superWeapons[infoIndex - 3])->RechargeTimer;
					}

					return nullptr;
				}

				if (pBuildingType->SuperWeapon != -1)
					return &pHouse->Supers.operator[](pBuildingType->SuperWeapon)->RechargeTimer;
				else if (pBuildingType->SuperWeapon2 != -1)
					return &pHouse->Supers.operator[](pBuildingType->SuperWeapon2)->RechargeTimer;

				const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
				return superWeapons.size() > 0 ? &pHouse->Supers.operator[](superWeapons[0])->RechargeTimer : nullptr;
			};

			if (const auto pTimer = getSuperTimer())
			{
				value = pTimer->GetTimeLeft();
				maxValue = pTimer->TimeLeft;
			}
		break;
	}
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;

		if (pThis->Disguised && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			GetDigitalDisplayFakeHealth(pThis, value, maxValue);

		break;
	}
	}
}

void TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTarget = std::exchange(pExt->WebbyLastTarget, nullptr);

	if (pTarget)
		pThis->Override_Mission(pExt->WebbyLastMission, pTarget, pTarget);
	else
		pThis->QueueMission(pThis->Owner->IsControlledByHuman() ? Mission::Guard : Mission::Hunt ,true);
}

void TechnoExtData::StoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->WebbyLastMission = pThis->GetCurrentMission();
	pExt->WebbyLastTarget = pThis->Target;
}

//https://blueprints.launchpad.net/ares/+spec/elite-armor
Armor TechnoExtData::GetArmor(ObjectClass* pThis) {
	//if(!pThis->IsAlive)
	//	Debug::Log("Death Techno used for GetArmor !\n");

	if(pThis->AbstractFlags & AbstractFlags::Techno){
		const auto pType = GET_TECHNOTYPE(((TechnoClass*)pThis));
		Armor res = pType->Armor;

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)pType);

		if (pTypeExt->VeteranArmor.isset() && ((TechnoClass*)pThis)->Veterancy.IsVeteran())
			res = pTypeExt->VeteranArmor;
		else if (pTypeExt->EliteArmor.isset() && ((TechnoClass*)pThis)->Veterancy.IsElite())
			res = pTypeExt->EliteArmor;

		if(pTypeExt->DeployedArmor.isset() && pThis->WhatAmI() == AbstractType::Infantry) {
			if (((InfantryClass*)pThis)->IsDeployed()) {
				res = pTypeExt->DeployedArmor;
			}
		}

		return res;
	}

	//Debug::LogInfo("{} Armor [{} = {}]", pType->ID, res, ArmorTypeClass::Array[(int)res]->Name.data());

	return pThis->GetType()->Armor;
}

void TechnoExtData::StoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pExt->HijackerLastDisguiseType = (InfantryTypeClass*)pThis->GetDisguise(true);
	pExt->HijackerLastDisguiseHouse = pThis->GetDisguiseHouse(true);
}

void TechnoExtData::RestoreStoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pThis->ClearDisguise();
	pThis->Disguise = pExt->HijackerLastDisguiseType;
	pThis->DisguisedAsHouse = pExt->HijackerLastDisguiseHouse;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary)
{
	if (!pThis)
		return nullptr;

	auto const pType = GET_TECHNOTYPE(pThis);
	weaponIndex = getSecondary ? 1 : 0;

	if (pType->TurretCount > 0 && !pType->IsGattling)
	{
		if (getSecondary)
		{
			weaponIndex = -1;
			return nullptr;
		}

		weaponIndex = pThis->CurrentWeaponNumber >= 0 ? pThis->CurrentWeaponNumber : 0;
	}
	else if (pType->IsGattling)
	{
		weaponIndex = pThis->CurrentGattlingStage * 2 + weaponIndex;
	}

	//Debug::LogInfo("{} Getting WeaponIndex {} for {}", __FUNCTION__, weaponIndex, pThis->get_ID());
	if(const auto pWpStr = pThis->GetWeapon(weaponIndex))
		return pWpStr->WeaponType;

	return nullptr;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, bool getSecondary)
{
	int weaponIndex = 0;
	return TechnoExtData::GetCurrentWeapon(pThis, weaponIndex, getSecondary);
}

bool TechnoExtData::IsCullingImmune(TechnoClass* pThis)
{
	return HasAbility(pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(TechnoClass* pThis)
{
	if (WarheadTypeExtData::IsTypeEMPProne(pThis))
		return true;

	return HasAbility(pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);

	if (pType->ImmuneToPsionics)
		return true;

	if (HasAbility(pThis, PhobosAbilityType::PsionicsImmune))
		return true;

	return false;
}

bool TechnoExtData::IsCritImmune(TechnoClass* pThis)
{
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(FootClass* pThis)
{
	if (!pThis || !pThis->IsWarpingIn())
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToRadiation)
		return true;

	if (HasAbility(pThis, PhobosAbilityType::RadImmune))
		return true;

	return false;
}

bool TechnoExtData::IsPsionicsWeaponImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToPsionicWeapons)
		return true;

	if (HasAbility(pThis, PhobosAbilityType::PsionicsWeaponImmune))
		return true;

	return false;
}

bool TechnoExtData::IsPoisonImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToPoison)
		return true;

	if (HasAbility(pThis, PhobosAbilityType::PoisonImmune))
		return true;

	return false;
}

bool TechnoExtData::IsBerserkImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(TechnoClass* pThis)
{
	if (GET_TECHNOTYPE(pThis)->Parasiteable)
		return false;

	return HasAbility(pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(TechnoClass* pThis)
{
	if (!GET_TECHNOTYPE(pThis)->Warpable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(TechnoClass* pThis)
{
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(TechnoClass* pThis)
{
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(pThis, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(TechnoClass* pThis)
{
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::IsUntrackable(TechnoClass* pThis)
{
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->Untrackable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Untrackable);
}

bool TechnoExtData::ISC4Holder(InfantryClass* pThis) {

	if (pThis->Type->C4)
		return true;

	return pThis->HasAbility(AbilityType::C4);
}

bool TechnoExtData::IsInterceptor()
{
	auto const pThis = This();
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->Interceptor)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Interceptor);
}

void TechnoExtData::CreateInitialPayload(bool forced)
{
	auto const pThis = This();
	auto const pType = GET_TECHNOTYPE(pThis);

	//if (IS_SAME_STR_("FTNKT", pType->ID))
	//	Debug::Log("FTNKT Check\n");

	if (!forced) {
		if (this->PayloadTriggered) {
			return;
		}

		this->PayloadTriggered = true;
	}

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->InitialPayload_Types.empty())
		return;

	const bool re_AppyAcademyBonusses = Unsorted::ScenarioInit && (!pType->UndeploysInto || !pType->DeploysInto);
	auto const pBld = cast_to<BuildingClass*, false>(pThis);

	auto freeSlots = (pBld && pBld->Type->CanBeOccupied)
		? pBld->Type->MaxNumberOccupants - pBld->GetOccupantCount()
		: pType->Passengers - pThis->Passengers.NumPassengers;

	auto const sizePayloadNum = pTypeExt->InitialPayload_Nums.size();
	auto const sizePayloadRank = pTypeExt->InitialPayload_Vet.size();
	auto const sizePyloadAddTeam = pTypeExt->InitialPayload_AddToTransportTeam.size();

	for (size_t i = 0u; i < pTypeExt->InitialPayload_Types.size(); ++i)
	{
		auto const pPayloadType = pTypeExt->InitialPayload_Types[i];

		if (!pPayloadType)
		{
			continue;
		}

		// buildings and aircraft aren't valid payload, and building payload
		// can only be infantry
		auto const absPayload = pPayloadType->WhatAmI();
		if (absPayload == AbstractType::BuildingType
			|| absPayload == AbstractType::AircraftType
			|| (pBld && absPayload != AbstractType::InfantryType))
		{
			continue;
		}

		// if there are no nums, index gets huge and invalid, which means 1
		auto const idxPayloadNum = MinImpl(i + 1, sizePayloadNum) - 1;
		auto const payloadNum = (idxPayloadNum < sizePayloadNum)
			? pTypeExt->InitialPayload_Nums[idxPayloadNum] : 1;

		auto const rank = idxPayloadNum < sizePayloadRank ?
			pTypeExt->InitialPayload_Vet[idxPayloadNum] : Rank::Invalid;

		auto const addtoteam = idxPayloadNum < sizePyloadAddTeam ?
			pTypeExt->InitialPayload_AddToTransportTeam[idxPayloadNum] : false;

		// never fill in more than allowed
		auto const count = MinImpl(payloadNum, freeSlots);
		freeSlots -= count;

		for (auto j = 0; j < count; ++j)
		{
			// clear the mutexes temporally
			// this is really dangerious that can cause issues
			// since Mutex is there to make stuffs go wrong or overlap eachother
			int mutex_old = std::exchange(Unsorted::ScenarioInit(), 0);
			auto const pObject = (TechnoClass*)pPayloadType->CreateObject(pThis->Owner);
			Unsorted::ScenarioInit = mutex_old;

			if (!pObject)
				continue;

			if (rank == Rank::Veteran)
				pObject->Veterancy.SetVeteran();
			else if (rank == Rank::Elite)
				pObject->Veterancy.SetElite();

				if(re_AppyAcademyBonusses) {
					HouseExtContainer::Instance.Find(pThis->Owner)->ApplyAcademyWithoutMutexCheck(pObject, absPayload);
				}

			if (pBld)
			{
				// buildings only allow infantry payload, so this in infantry
				auto const pPayload = static_cast<InfantryClass*>(pObject);

				if (pBld->Type->CanBeOccupied)
				{
					pBld->Occupants.push_back(pPayload);
					auto const pCell = pThis->GetCell();
					InfantryExtContainer::Instance.Find(pPayload)->GarrisonedIn = pBld;
					pThis->UpdateThreatInCell(pCell);
				}
				else
				{
					pPayload->Limbo();

					if (pBld->Type->InfantryAbsorb)
					{
						pPayload->Absorbed = true;

						if (pPayload->CountedAsOwnedSpecial)
						{
							--pPayload->Owner->OwnedInfantry;
							pPayload->CountedAsOwnedSpecial = false;
						}

						if (pBld->Type->ExtraPowerBonus > 0)
						{
							pBld->Owner->RecheckPower = true;
						}
					}
					else
					{
						pPayload->SendCommand(RadioCommand::RequestLink, pBld);
					}

					pBld->AddPassenger(pPayload);
					pPayload->AbortMotion();
				}
			}
			else
			{
				auto const pPayload = static_cast<FootClass*>(pObject);
				pPayload->SetLocation(pThis->Location);
				pPayload->IsInPlayfield = true;
				pPayload->Limbo();

				if (pType->OpenTopped)
				{
					pThis->EnteredOpenTopped(pPayload);
				}

				pPayload->Transporter = pThis;

				auto const old = std::exchange(VocClass::VoicesEnabled(), false);
				pThis->AddPassenger(pPayload);
				VocClass::VoicesEnabled = old;

				if (addtoteam) {
					if (auto pTeam = ((FootClass*)pThis)->Team){
						pTeam->AddMember(pPayload, true);
					}
				}
			}
		}
	}
}

bool TechnoExtData::HasAbility(TechnoClass* pThis, PhobosAbilityType nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	if (!IsVet && !IsElite)
	{
		return false;
	}

	return HasAbility(IsVet ? Rank::Veteran : Rank::Elite, GET_TECHNOTYPEEXT(pThis), nType);
}

bool TechnoExtData::HasImmunity(TechnoClass* pThis, int nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (IsVet)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (IsElite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) ||
			pTypeExt->V_ImmuneToType.Contains((int)nType) ||
			pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

bool TechnoExtData::IsCullingImmune(Rank vet, TechnoClass* pThis)
{
	return HasAbility(vet, GET_TECHNOTYPEEXT(pThis), PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(Rank vet, TechnoClass* pThis)
{
	if (WarheadTypeExtData::IsTypeEMPProne(pThis))
		return true;

	return HasAbility(vet, GET_TECHNOTYPEEXT(pThis), PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(vet, TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::PsionicsImmune);
}

bool TechnoExtData::IsCritImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(Rank vet, FootClass* pThis)
{
	if (!pThis)
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (!pThis->IsWarpingIn())
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(vet,  TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::RadImmune);
}

bool TechnoExtData::IsPsionicsWeaponImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(vet,  TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExtData::IsPoisonImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(vet,  TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::PoisonImmune);
}

bool TechnoExtData::IsBerserkImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(Rank vet, InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(vet, TechnoTypeExtContainer::Instance.Find(pThis->Type), PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(Rank vet, TechnoClass* pThis)
{	
	const auto pType = GET_TECHNOTYPE(pThis);

	if (!pType->Parasiteable)
		return false;

	return HasAbility(vet, TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(Rank vet, TechnoClass* pThis)
{
	const auto pType = GET_TECHNOTYPE(pThis);

	if (!pType->Warpable)
		return true;

	return HasAbility(vet, TechnoTypeExtContainer::Instance.Find(pType), PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::IsUntrackable(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pTypeExt->Untrackable)
		return true;

	return HasAbility(vet, pTypeExt, PhobosAbilityType::Untrackable);
}

bool TechnoExtData::HasAbility(Rank vet, const TechnoTypeExtData* pTypeExt, PhobosAbilityType nType)
{
	if (nType == PhobosAbilityType::None)
		return false;

	if (vet == Rank::Veteran)
	{
		return pTypeExt->Phobos_VeteranAbilities.at((int)nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->Phobos_VeteranAbilities.at((int)nType) || pTypeExt->Phobos_EliteAbilities.at((int)nType);
	}

	return false;
}

bool TechnoExtData::HasImmunity(Rank vet, TechnoClass* pThis, int nType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (vet == Rank::Veteran)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains((int)nType) || pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

#include <Ext/TerrainType/Body.h>

bool TechnoExtData::IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker)
{
	if (!pVictim || !pVictim->IsAlive || !pAttacker || pVictim->IsBeingWarpedOut())
		return false;

	if (pVictim->IsIronCurtained())
		return false;

	if (pAttacker->Owner && pAttacker->Owner->IsAlliedWith(pVictim))
		return false;

	auto const pAttackerType = GET_TECHNOTYPE(pAttacker);
	auto const pVictimTechno = flag_cast_to<TechnoClass*, false>(pVictim);
	auto const pAttackerTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pAttackerType);

	if (!pVictimTechno)
	{
		if (auto const pTerrain = cast_to<TerrainClass*, false>(pVictim))
		{
			if (pTerrain->Type->Immune || pTerrain->Type->SpawnsTiberium || !pTerrain->Type->Crushable)
				return false;

			const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);
			if (pTerrainExt->IsPassable)
				return false;

			return pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) > pTerrainExt->CrushableLevel;
		}

		return false;
	}

	auto const pWhatVictim = pVictim->WhatAmI();
	auto const pVictimType = pVictim->GetTechnoType();

	if (pAttackerType->OmniCrusher)
	{
		if (pWhatVictim == BuildingClass::AbsID || pVictimType->OmniCrushResistant)
			return false;
	}
	else
	{
		if (pVictimTechno->Uncrushable || !pVictimType->Crushable)
			return false;
	}

	auto const pVictimTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pVictimType);

	if (pWhatVictim == InfantryClass::AbsID)
	{
		const auto& crushableLevel = static_cast<InfantryClass*>(pVictim)->IsDeployed() ?
			pVictimTechnoTypeExt->DeployCrushableLevel :
			pVictimTechnoTypeExt->CrushableLevel;

		if (pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) < crushableLevel.Get(pVictimTechno))
			return false;
	}

	if (TechnoExtData::IsChronoDelayDamageImmune(flag_cast_to<FootClass*, false>(pVictim)))
	{
		return false;
	}

	//auto const pExt = TechnoExtContainer::Instance.Find(pVictimTechno);
	//if (auto const pShieldData = pExt->Shield.get()) {
	//	auto const pWeaponIDx = pAttacker->SelectWeapon(pVictim);
	//	auto const pWeapon = pAttacker->GetWeapon(pWeaponIDx);

	//	if (pWeapon && pWeapon->WeaponType &&
	//		pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon->WeaponType)) {
	//		return false;
	//	}
	//}

	return true;
}

AreaFireReturnFlag TechnoExtData::ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	switch (pExt->AreaFire_Target.Get())
	{
	case AreaFireTarget::Random:
	{
		std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(
			static_cast<short>(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) + 0.99));

		size_t const size = adjacentCells.size();

		for (int i = 0; i < (int)size; i++)
		{
			const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			CellStruct const tgtPos = pTargetCell->MapCoords + adjacentCells.at((i + rand) % size);
			CellClass* const tgtCell = MapClass::Instance->GetCellAt(tgtPos);
			bool allowBridges = tgtCell && tgtCell->ContainsBridge() && (pThis->OnBridge || (tgtCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

			if (pExt->SkipWeaponPicking || EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true, false, allowBridges))
			{
				pTargetCell = tgtCell;
				return AreaFireReturnFlag::Continue;
			}
		}

		return AreaFireReturnFlag::DoNotFire;
	}
	case AreaFireTarget::Self:
	{
		if(pExt->SkipWeaponPicking)
			return AreaFireReturnFlag::SkipSetTarget;

		if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, false, false, pThis->OnBridge))
			return AreaFireReturnFlag::DoNotFire;

		return AreaFireReturnFlag::SkipSetTarget;
	}
	default:
	{
		auto pCell = pTargetCell;
		bool allowBridges = pCell && pCell->ContainsBridge() && (pThis->OnBridge || (pCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

		if (!pExt->SkipWeaponPicking && !EnumFunctions::AreCellAndObjectsEligible(pTargetCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, false, false, allowBridges))
			return AreaFireReturnFlag::DoNotFire;
	}
	}

	return AreaFireReturnFlag::ContinueAndReturn;
}

int __fastcall TechnoExtData::GetThreadPosed(FootClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pShieldData = pExt->GetShield()) {
		if (pShieldData->IsActive()) {
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pShiedType->ThreadPosed.Get();
		}
	}

	return GET_TECHNOTYPE(pThis)->ThreatPosed;
}

int __fastcall TechnoExtData::GetBuildingThreadPosed(BuildingClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	int occupantCount = pThis->GetOccupantCount();

	if (occupantCount > 0)
		return RulesClass::Instance->ThreatPerOccupant * occupantCount;

	if (auto const pLinked = pThis->BunkerLinkedItem)
		return pLinked->GetThreatValue();

	auto const pType = pThis->Type;

	// Set threat value of uncaptured tech buildings to 0.
	if (pType->NeedsEngineer && pThis->Owner->Type->MultiplayPassive)
		return 0;

	if (const auto pShieldData = pExt->GetShield()) {
		if (pShieldData->IsActive()) {
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pShiedType->ThreadPosed.Get();
		}
	}

	return GET_TECHNOTYPE(pThis)->ThreatPosed;
}


DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2564, TechnoExtData::GetThreadPosed);  // AircraftClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8F54, TechnoExtData::GetThreadPosed);  // FootClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB318, TechnoExtData::GetThreadPosed);  // InfantryClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4C20, TechnoExtData::GetThreadPosed);  // TechnoClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5F30, TechnoExtData::GetThreadPosed);  // UnitClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E417C, TechnoExtData::GetBuildingThreadPosed);   // BuildingClass

bool TechnoExtData::IsReallyTechno(TechnoClass* pThis)
{
	const auto pAddr = (((DWORD*)pThis)[0]);
	if (pAddr != UnitClass::vtable
		&& pAddr != AircraftClass::vtable
		&& pAddr != InfantryClass::vtable
		&& pAddr != BuildingClass::vtable)
	{
		return false;
	}

	return true;
}

int TechnoExtData::GetDeployFireWeapon(UnitClass* pThis)
{
	if (pThis->Type->DeployFireWeapon == -1)
		return pThis->TechnoClass::SelectWeapon(pThis->Target ? pThis->Target : pThis->GetCell());

	return pThis->Type->DeployFireWeapon;
}

// Gets weapon index for a weapon to use against wall overlay.
int TechnoExtData::GetWeaponIndexAgainstWall(TechnoClass * pThis, OverlayTypeClass * pWallOverlayType)
{
	auto const pTechnoType = GET_TECHNOTYPE(pThis);
	int weaponIndex = -1;
	auto pWeapon = TechnoExtData::GetCurrentWeapon(pThis, weaponIndex);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	if ((pTechnoType->TurretCount > 0 && !pTechnoType->IsGattling)
			|| !pWallOverlayType
			|| !pWallOverlayType->Wall
			|| !pTypeExt->AllowWeaponSelectAgainstWalls.Get(RulesExtData::Instance()->AllowWeaponSelectAgainstWalls)
		)
		return weaponIndex;
	else if (weaponIndex == -1)
		return 0;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.TryFind(pWeapon);
	bool aeForbidsPrimary = pWeaponExt && pWeaponExt->AttachEffect_CheckOnFirer
	&& !pWeaponExt->SkipWeaponPicking && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

	if (!pWeapon || (!pWeapon->Warhead->Wall && (!pWeapon->Warhead->Wood || pWallOverlayType->Armor != Armor::Wood)) || TechnoExtData::CanFireNoAmmoWeapon(pThis, 1) || aeForbidsPrimary)
	{
		int weaponIndexSec = -1;
		auto pSecondaryWeapon = TechnoExtData::GetCurrentWeapon(pThis, weaponIndexSec, true);
		auto pSecondaryWeaponExt = WeaponTypeExtContainer::Instance.TryFind(pSecondaryWeapon);
		bool aeForbidsSecondary = pSecondaryWeaponExt && pSecondaryWeaponExt->AttachEffect_CheckOnFirer
		&& !pSecondaryWeaponExt->SkipWeaponPicking && !pSecondaryWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

		if (pSecondaryWeapon && (pSecondaryWeapon->Warhead->Wall || (pSecondaryWeapon->Warhead->Wood && pWallOverlayType->Armor == Armor::Wood)
			&& (!pTypeExt->NoSecondaryWeaponFallback || aeForbidsPrimary)) && !aeForbidsSecondary)
		{
			return weaponIndexSec;
		}

		return weaponIndex;
	}

	return weaponIndex;
}

void TechnoExtData::SetMissionAfterBerzerk(TechnoClass* pThis, bool Immediete)
{
	auto const pType = GET_TECHNOTYPE(pThis);

	const Mission nEndMission = pThis->IsArmed() ?
		(pThis->Owner->IsHumanPlayer ? Mission::Hunt : Mission::Guard) :
		(!pType->ResourceGatherer ? Mission::Sleep : Mission::Harvest);

	pThis->QueueMission(nEndMission, Immediete);
}

std::pair<TechnoClass*, CellClass*> TechnoExtData::GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget)
{
	TechnoClass* pTargetTechno = nullptr;
	CellClass* targetCell = nullptr;

	//pTarget nullptr check already done above this hook
	if (pTarget && pTarget->WhatAmI() == CellClass::AbsID)
	{
		return { nullptr , targetCell = static_cast<CellClass*>(pTarget) };
	}
	else if (pObjTarget)
	{
		// it is an techno target
		if (((pObjTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
		{
			pTargetTechno = static_cast<TechnoClass*>(pObjTarget);
			if (!pTargetTechno->IsInAir())	// Ignore target cell for airborne technos.
				targetCell = pTargetTechno->GetCell();
		}
		else // non techno target , but still an object
		{
			targetCell = pObjTarget->GetCell();
		}
	}

	return { pTargetTechno , targetCell };
}

bool TechnoExtData::AllowFiring(AbstractClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	//fuckoof
	return true;
}

bool TechnoExtData::ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pTargetObj)
	{
		if(pWeaponExt->Targeting_Health_Percent.isset()){
			auto const pHP = pTargetObj->GetHealthPercentage();

			if (!pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP <= pWeaponExt->Targeting_Health_Percent.Get())
				return false;
			else if (pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP >= pWeaponExt->Targeting_Health_Percent.Get())
				return false;
		}

		if(!pWeaponExt->IsHealthInThreshold(pTargetObj))
			return false;
	}

	return true;
}

bool TechnoExtData::CheckCellAllowFiring(TechnoClass* pThis, CellClass* pCell, WeaponTypeClass* pWeapon)
{	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if(!pWeaponExt->SkipWeaponPicking && pCell) {
		if (!EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true, true)
		|| (pWeaponExt->AttachEffect_CheckOnFirer && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis)) ) {
			return false;
		}
	}

	return true;
}

bool TechnoExtData::TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);;
	if(pWeaponExt->SkipWeaponPicking)
		return true;

	if (!EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget, false) ||
		!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTarget->Owner) ||
		!pWeaponExt->IsVeterancyInThreshold(pTarget) ||
		!pWeaponExt->HasRequiredAttachedEffects(pThis, pTarget))
	{
		return false;
	}

	return true;
}

bool TechnoExtData::FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);

	if (auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (!pUnit->Type->IsSimpleDeployer && !pUnit->Deployed && pTarget)
		{
			if (pUnit->Type->DeployFire && pWeapon->FireOnce)
			{
				if (pTechnoExt->DeployFireTimer.GetTimeLeft() > 0)
					return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const int nMoney = pWHExt->TransactMoney;
	if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
		return false;

	return true;
}

bool TechnoExtData::InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget)
{
	//this code is used to remove Techno as auto target consideration , so interceptor can find target faster
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pTechnoExt->IsInterceptor() && pTechnoTypeExt->Interceptor_OnlyTargetBullet.Get())
	{
		if (!pTarget || pTarget->WhatAmI() == BulletClass::AbsID)
		{
			return false;
		}
	}

	return true;
}

bool TechnoExtData::TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (const auto pShieldData = pTargetTechnoExt->GetShield())
	{
		if (pShieldData->IsActive())
		{
			if (!pShieldData->CanBePenetrated(pWeapon->Warhead))
			{
				if (pWHExt->GetVerses(pShieldData->GetType()->Armor).Verses < 0.0 && pShieldData->GetType()->CanBeHealed)
				{
					const bool IsFullHP = pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen;
					if (!IsFullHP)
						return true;
					else
					{
						if (pShieldData->GetType()->PassthruNegativeDamage)
						{
							return !(pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen);
						}
					}
				}

				return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot)
{

	if (!pFoot->IsAlive
		|| pFoot->InLimbo
		|| pFoot->IsIronCurtained()
		|| pFoot->IsSinking
		|| pFoot->IsCrashing
		|| TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled) {
		return false;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//Don't abduct the target if it has more life then the abducting percent

	if (pWeaponExt->Abductor_AbductBelowPercent < pFoot->GetHealthPercentage()) {
		return false;
	}

	if (pWeaponExt->Abductor_MaxHealth > 0 && pWeaponExt->Abductor_MaxHealth < pFoot->Health) {
		return false;
	}

	if (TechnoExtData::IsAbductorImmune(pFoot))
		return false;

	if (!TechnoExtData::IsEligibleSize(pThis, pFoot))
		return false;

	if (!TechnoTypeExtData::PassangersAllowed(GET_TECHNOTYPE(pThis), GET_TECHNOTYPE(pFoot)))
		return false;

	return true;
}

void TechnoExtData::SendPlane(AircraftTypeClass* Aircraft, size_t Amount, HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest)
{
	if (!Aircraft || !pOwner || Amount <= 0)
		return;

	//safeguard
	Mission result = Mission::None;
	switch (SendMission)
	{
	case Mission::Move:
	{
		if (!pDest)
			pDest = pTarget;

		result = SendMission;
	}
	break;
	case Mission::ParadropApproach:
	case Mission::Attack:
	case Mission::SpyplaneApproach:
		result = SendMission;
		break;
	default:
		result = Mission::SpyplaneApproach;
		break;
	}

	const auto edge = pOwner->GetHouseEdge();

	for (size_t i = 0; i < Amount; ++i)
	{
		++Unsorted::ScenarioInit;
		auto const pPlane = static_cast<AircraftClass*>(Aircraft->CreateObject(pOwner));
		--Unsorted::ScenarioInit;

		if (!pPlane)
			continue ;

		pPlane->Spawned = true;
		//randomized
		const auto nCell = MapClass::Instance->PickCellOnEdge(edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
		pPlane->QueueMission(result, false);

		if (SendRank != Rank::Rookie && SendRank != Rank::Invalid && pPlane->CurrentRanking < SendRank)
			pPlane->Veterancy.SetRank(SendRank);

		if (pDest)
			pPlane->SetDestination(pDest, true);

		if (pTarget)
			pPlane->SetTarget(pTarget);

		bool UnLimboSucceeded = AircraftExtData::PlaceReinforcementAircraft(pPlane , nCell);

		if (!UnLimboSucceeded)  {
			GameDelete<true, false>(pPlane);
		}
		else
		{

			// we cant create InitialPayload when mutex atives
			// so here we handle the InitialPayload Creation !
			// this way we can make opentopped airstrike happen !
			TechnoExtContainer::Instance.Find(pPlane)->CreateInitialPayload();
			if ((TechnoTypeExtContainer::Instance.Find(pPlane->Type)->Passengers_BySize
			? pPlane->Passengers.GetTotalSize() : pPlane->Passengers.NumPassengers) > 0)
				pPlane->HasPassengers = true;

			pPlane->NextMission();
		}
	}
}

/*
 * Object should NOT be placed on the map (->Limbo() it or don't Put in the first place)
 * otherwise Bad Things (TM) will happen. Again.
 */
bool TechnoExtData::CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ)
{
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);

	if (Object->IsCellOccupied(MyCell, FacingType::None, -1, nullptr, false) != Move::OK)
	{
		return false;
	}
	else
	{
		LocomotionClass::ChangeLocomotorTo(Object, CLSIDs::DropPod());
		CoordStruct xyz = XYZ;
		xyz.Z = 0;

		Object->SetLocation(xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor->Move_To(XYZ);
		Object->PrimaryFacing.Set_Current(DirStruct()); // TODO : random or let loco set the facing

		if (!Object->InLimbo)
		{
			Object->See(0, 0);
			Object->QueueMission(Object->Owner && Object->Owner->IsControlledByHuman() ? Mission::Area_Guard : Mission::Hunt, true);
			Object->NextMission();
			return true;
		}

		return false;
	}
}

bool TechnoExtData::TargetFootAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	if ((pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = static_cast<FootClass*>(pTarget);
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		if (pWeaponExt->Abductor
			&& pWeaponExt->Abductor_CheckAbductableWhenTargeting
			&& !TechnoExtData::IsAbductable(pThis, pWeapon, pFoot
			))
			return false;

		if (auto const pUnit = cast_to<UnitClass*, false>(pTarget))
		{
			if (pUnit->DeathFrameCounter > 0)
				return false;
		}

		if (TechnoExtData::IsChronoDelayDamageImmune(pFoot))
			return false;
	}

	return true;
}

void TechnoExtData::UpdateMCOverloadDamage(TechnoClass* pOwner)
{
	auto pThis = pOwner->CaptureManager;

	if (!pThis || !pThis->InfiniteMindControl || pOwner->InLimbo || !pOwner->IsAlive)
		return;

	const auto pOwnerTypeExt = GET_TECHNOTYPEEXT(pOwner);

	if (pThis->OverloadPipState > 0)
		--pThis->OverloadPipState;

	if (pThis->OverloadDamageDelay <= 0)
	{

		const auto OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesClass::Instance->OverloadCount);

		if (OverloadCount.empty())
			return;

		int nCurIdx = 0;
		const int nNodeCount = ((FakeCaptureManagerClass*)pThis)->__GetControlledCount();

		for (int i = 0; i < (int)(OverloadCount.size()); ++i)
		{
			if (nNodeCount > OverloadCount[i])
			{
				nCurIdx = i + 1;
			}
		}

		const auto nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesClass::Instance->OverloadFrames);
		pThis->OverloadDamageDelay = nOverloadfr.get_or_last(nCurIdx);

		const auto nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesClass::Instance->OverloadDamage);
		auto nDamage = nOverloadDmg.get_or_last(nCurIdx);

		if (nDamage <= 0)
		{
			pThis->OverloadDeathSoundPlayed = false;
		}
		else
		{
			pThis->OverloadPipState = 10;
			auto const pWarhead = pOwnerTypeExt->Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pOwner->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!pThis->OverloadDeathSoundPlayed)
			{
				VocClass::SafeImmedietelyPlayAt(pOwnerTypeExt->Overload_DeathSound.Get(RulesClass::Instance->MasterMindOverloadDeathSound), &pOwner->Location, 0);
				pThis->OverloadDeathSoundPlayed = true;
			}

			if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem))
			{
				for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto nLoc = pOwner->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { nLoc.X + nRamdomX, nRandomY + nLoc.Y, nLoc.Z };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pOwner->GetCell(), pOwner, CoordStruct::Empty, pOwner->Owner);
				}
			}

			if (nCurIdx > 0 && pOwner->IsAlive)
			{
				double const nBase = (nCurIdx != 1) ? Math::flt_2 : Math::flt_1;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pOwner->RockingSidewaysPerFrame = static_cast<float>(nCopied_base);
			}
		}

	}
	else
	{
		--pThis->OverloadDamageDelay;
	}
}

bool TechnoExtData::AllowedTargetByZone(TechnoClass* pThis, ObjectClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon, std::optional<std::reference_wrapper<const ZoneType>> zone)
{
	if (pThis->WhatAmI() == AircraftClass::AbsID)
		return true;

	const auto pThisType = GET_TECHNOTYPE(pThis);
	const MovementZone mZone = pThisType->MovementZone;
	const ZoneType currentZone = zone ? zone.value() :
		MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), mZone, pThis->OnBridge);

	if (currentZone != ZoneType::None)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		const ZoneType targetZone =
			MapClass::Instance->GetMovementZoneType(pTarget->InlineMapCoords(), mZone, pTarget->OnBridge);

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (!pWeapon)
			{
				const int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				if (const auto pWpStruct = pThis->GetWeapon(weaponIndex))
					pWeapon = pWpStruct->WeaponType;
				else
					return false;
			}

			auto const speedType = pThisType->SpeedType;
			const auto cellStruct = MapClass::Instance->NearByLocation(pTarget->InlineMapCoords(),
				speedType, ZoneType::None, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);

			auto const pCell = MapClass::Instance->GetCellAt(cellStruct);

			if (!pCell)
				return false;

			const double distance = pCell->GetCoordsWithBridge().DistanceFromSquared(pTarget->GetCenterCoords());
			const int range = pWeapon->Range;

			if (distance > range * range)
				return false;
		}
	}

	return true;
}

//ToDo : Auto regenerate and transferable passengers (Problem : Driver killed and operator stuffs )
void TechnoExtData::PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce)
{
	if (!pTransporter || !pTransporter->Passengers.NumPassengers || !MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	//TODO : check if passenger is actually allowed to go outside
	auto pPassenger = pTransporter->Passengers.RemoveFirstPassenger();
	CoordStruct nDest = nCoord;

	//if (bForce)
	{
		//TechnoTypeClass* pPassengerType = pPassenger->GetTechnoType();
		//auto const pPassengerMZone = pPassengerType->MovementZone;
		//auto const pPassengerSpeedType = pPassengerType->SpeedType;


		//auto const pCellFrom = Map.GetCellAt(nCoord);
		//auto const nZone = Map.Zone_56D230(&pCellFrom->MapCoords, pPassengerMZone, pCellFrom->ContainsBridgeEx());

		//if (!Map[nCoord]->IsClearToMove(pPassengerSpeedType, false, false, nZone, pPassengerMZone, -1, 1))
		{
			nDest = MapClass::Instance->GetRandomCoordsNear(nCoord, ScenarioClass::Instance->Random.RandomFromMax(2000), ScenarioClass::Instance->Random.RandomFromMax(1));
		}
	}

	MapClass::Instance->GetCellAt(nCoord)->ScatterContent(pTransporter->GetCoords(), true, true, false);

	bool Placed = false;
	if (bForce)
	{
		++Unsorted::ScenarioInit;
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		--Unsorted::ScenarioInit;
	}
	else
	{
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		//Placed = CreateWithDroppod(pPassenger, nDest , LocomotionClass::CLSIDs::Teleport);
	}

	//Only remove passengers from the Transporter if it succeeded
	if (Placed)
	{
		pPassenger->Mark(MarkType::Remove);
		pPassenger->OnBridge = MapClass::Instance->GetCellAt(nCoord)->ContainsBridgeEx();
		pPassenger->Mark(MarkType::Put);
		pPassenger->StopMoving();
		pPassenger->SetDestination(nullptr, true);
		pPassenger->SetTarget(nullptr);
		pPassenger->CurrentTargets.clear();
		pPassenger->SetArchiveTarget(nullptr);
		pPassenger->MissionAccumulateTime = 0; // don't ask
		pPassenger->unknown_5A0 = 0;
		pPassenger->CurrentGattlingStage = 0;
		pPassenger->SetCurrentWeaponStage(0);
		pPassenger->SetLocation(nDest);
		pPassenger->LiberateMember();

		if (pPassenger->SpawnManager)
		{
			pPassenger->SpawnManager->ResetTarget();
		}

		pPassenger->ClearPlanningTokens(nullptr);

		pPassenger->DiscoveredBy(pTransporter->GetOwningHouse());
		auto pTransporterType = GET_TECHNOTYPE(pTransporter);

		if (auto pFoot = flag_cast_to<FootClass*, false>(pTransporter))
		{
			if (pTransporterType->Gunner)
			{
				pFoot->RemoveGunner(pPassenger);
			}

			if (pTransporterType->OpenTopped)
			{
				pFoot->ExitedOpenTopped(pPassenger);
			}

			pPassenger->Transporter = nullptr;
		}
		else
		{
			auto pBuilding = static_cast<BuildingClass*>(pTransporter);

			if (pBuilding->Absorber())
			{
				pPassenger->Absorbed = false;
				pPassenger->Transporter = nullptr;
				if (pBuilding->Type->ExtraPowerBonus > 0)
				{
					pBuilding->Owner->RecheckPower = true;
				}
			}
		}

		VocClass::SafeImmedietelyPlayAt(nSound, &nDest);

		if (pAnimToPlay)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimToPlay, nDest),
				pTransporter->GetOwningHouse(),
				nullptr,
				pTransporter,
				false, false
			);
		}

		if (pPassenger->CurrentMission != Mission::Guard)
			pPassenger->Override_Mission(Mission::Area_Guard);
	}
	else
	{
		pTransporter->AddPassenger(pPassenger);
	}
}

void TechnoExtData::SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained())
	{
		bool isForceShielded = pFrom->ProtectType == ProtectTypes::ForceShield;
		const auto pTypeExt =  GET_TECHNOTYPEEXT(pFrom);
		const auto bSync = !isForceShielded ?pTypeExt->IronCurtain_KeptOnDeploy
			.Get(RulesExtData::Instance()->IronCurtain_KeptOnDeploy)
			:pTypeExt->ForceShield_KeptOnDeploy.Get(RulesExtData::Instance()->ForceShield_KeptOnDeploy)
			;

		if (bSync) {
			pFrom->IronCurtainTimer = pFrom->IronCurtainTimer;
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExtData::PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker)
{
	if (pAnim && pInvoker) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pInvoker->Location),
			pInvoker->GetOwningHouse(),
			nullptr,
			pInvoker,
			false, false
		);
	}
}

double TechnoExtData::GetArmorMult(TechnoClass* pSource, double damageIn, WarheadTypeClass* pWarhead)
{
	const auto pType = GET_TECHNOTYPE(pSource);
	double _result = damageIn;

	auto const pExt = TechnoExtContainer::Instance.Find(pSource);

	//Ares AE using techno ArmorMultiplier
	//PHobos AE using ArmorMultData 
	if (pExt->AE.ArmorMultData.Enabled()) {
		_result /= pExt->AE.ArmorMultData.Get(pSource->ArmorMultiplier, pWarhead);
	} else {
		_result /= pSource->ArmorMultiplier;
	}

	if (auto pOwner = pSource->Owner)
	_result /= pOwner->GetTypeArmorMult(pType);

	if (pSource->HasAbility(AbilityType::Stronger)) {
		_result /= RulesClass::Instance->VeteranArmor;
	}

	return _result;
}

double TechnoExtData::GetDamageMult(TechnoClass* pSource, double damageIn , bool ForceDisable)
{
	if (ForceDisable || !pSource || !pSource->IsAlive)
		return damageIn;

	const auto pType = GET_TECHNOTYPE(pSource);

	if (!pType)
		return damageIn;

	double _result = damageIn;

	if(pSource->Owner) {
		_result *= pSource->Owner->FirepowerMultiplier;
	}

	_result *= pSource->FirepowerMultiplier;

	if(pSource->HasAbility(AbilityType::Firepower)){
		_result *= RulesClass::Instance->VeteranCombat;
	}

	return _result;
}

const BurstFLHBundle* TechnoExtData::PickFLHs(TechnoClass* pThis, int weaponidx)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	std::span<BurstFLHBundle> res  = pExt->WeaponBurstFLHs;

	if (pThis->WhatAmI() == InfantryClass::AbsID) {
		if (((InfantryClass*)pThis)->IsDeployed() && !pExt->DeployedWeaponBurstFLHs.empty())
			res = pExt->DeployedWeaponBurstFLHs;
		else if (((InfantryClass*)pThis)->Crawling && !pExt->CrouchedWeaponBurstFLHs.empty())
			res = pExt->CrouchedWeaponBurstFLHs;
	}

	if (res.empty() || res.size() <= (size_t)weaponidx)
		return nullptr;

	return &res[weaponidx];
}

std::pair<bool, CoordStruct> TechnoExtData::GetBurstFLH(TechnoClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false ,  CoordStruct::Empty };

	auto pickedFLHs = PickFLHs(pThis ,weaponIndex);

	if(!pickedFLHs)
		return  { false ,  CoordStruct::Empty };

	std::span<const CoordStruct> selected = pThis->Veterancy.IsElite() ? pickedFLHs->EFlh : pickedFLHs->Flh;

	if (!selected.empty() && (size_t)pThis->CurrentBurstIndex < selected.size()) {
		return { true , selected[pThis->CurrentBurstIndex] };
	}

	return { false , CoordStruct::Empty };
}

const Nullable<CoordStruct>* TechnoExtData::GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex)
{
	const auto pTechnoType = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedPrimaryFireFLH :
				&pTechnoType->DeployedPrimaryFireFLH;
		}
		else if (weaponIndex == 1)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedSecondaryFireFLH :
				&pTechnoType->DeployedSecondaryFireFLH;
		}
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
			{
				return pThis->Veterancy.IsElite() ?

					pTechnoType->E_PronePrimaryFireFLH.isset() ?
					&pTechnoType->E_PronePrimaryFireFLH :
					&pTechnoType->Elite_PrimaryCrawlFLH
					:

					pTechnoType->PronePrimaryFireFLH.isset() ?
					&pTechnoType->PronePrimaryFireFLH :
					&pTechnoType->PrimaryCrawlFLH
					;
			}
			else if (weaponIndex == 1)
			{
				return pThis->Veterancy.IsElite() ?
					pTechnoType->E_ProneSecondaryFireFLH.isset() ?
					&pTechnoType->E_ProneSecondaryFireFLH :
					&pTechnoType->E_ProneSecondaryFireFLH
					:

					pTechnoType->ProneSecondaryFireFLH.isset() ?
					&pTechnoType->ProneSecondaryFireFLH :
					&pTechnoType->SecondaryCrawlFLH
					;
			}
		}
	}

	return nullptr;
}

const Armor TechnoExtData::GetTechnoArmor(TechnoClass* pThis, WarheadTypeClass* pWarhead)
{
	Armor nArmor = TechnoExtData::GetArmor(pThis);
	TechnoExtData::ReplaceArmor(nArmor, pThis, pWarhead);
	return nArmor;
}

const Armor TechnoExtData::GetTechnoArmor(ObjectClass* pThis, WarheadTypeClass* pWarhead)
{
	if(pThis->AbstractFlags & AbstractFlags::Techno){
		return TechnoExtData::GetTechnoArmor((TechnoClass*)pThis , pWarhead);
	}

	return pThis->GetType()->Armor;
}

std::pair<bool, CoordStruct> TechnoExtData::GetInfantryFLH(InfantryClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false , CoordStruct::Empty };

	const auto pickedFLH = TechnoExtData::GetInfrantyCrawlFLH(pThis, weaponIndex);

	if (pickedFLH && pickedFLH->isset() && pickedFLH->Get().IsValid())
	{
		return { true , pickedFLH->Get() };
	}

	return{ false , CoordStruct::Empty };
}

CoordStruct TechnoExtData::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	//guarantee
	if (maxAttempts < 1)
		maxAttempts = 1;

	CellClass* pCell = pThis->GetCell();
	CellStruct placeCoords = CellStruct::Empty;
	auto pTypePassenger = GET_TECHNOTYPE(pPassenger);
	SpeedType speedType = SpeedType::Track;
	MovementZone movementZone = MovementZone::Normal;

	if (pPassenger->WhatAmI() != AircraftClass::AbsID)
	{
		speedType = pTypePassenger->SpeedType;
		movementZone = pTypePassenger->MovementZone;
	}

	Point2D ExtDistance = { 1,1 };
	for (int i = 0; i < maxAttempts; ++i)
	{

		placeCoords = pCell->MapCoords - CellStruct { (short)(ExtDistance.X / 2), (short)(ExtDistance.Y / 2) };
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, ZoneType::None, movementZone, false, ExtDistance.X, ExtDistance.Y, true, false, false, false, CellStruct::Empty, false, false);

		pCell = MapClass::Instance->GetCellAt(placeCoords);

		if ((pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) == Move::OK) && MapClass::Instance->IsWithinUsableArea(pCell->GetCoordsWithBridge())) {
			return pCell->GetCoordsWithBridge();
		}

		++ExtDistance;
	}

	return CoordStruct::Empty;
}

/*
void TechnoExtData::DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry, bool sIsDisguised)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->UseCustomSelectBrd.Get(RulesExtData::Instance()->UseSelectBrd.Get(Phobos::Config::EnableSelectBrd)))
		return;

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP
		.Get(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdSHP_INF : RulesExtData::Instance()->SHP_SelectBrdSHP_UNIT);

	if (!SelectBrdSHP)
		return;

	ConvertClass* SelectBrdPAL = (pTypeExt->SHP_SelectBrdPAL ?
		pTypeExt->SHP_SelectBrdPAL :
		(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdPAL_INF : RulesExtData::Instance()->SHP_SelectBrdPAL_UNIT))
		->GetOrDefaultConvert<PaletteManager::Mode::Temperate>(FileSystem::ANIM_PAL);

	if (!SelectBrdPAL)
		return;

	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;

	const Point3D selectbrdFrame = pTypeExt->SelectBrd_Frame.Get((isInfantry ? RulesExtData::Instance()->SelectBrd_Frame_Infantry : RulesExtData::Instance()->SelectBrd_Frame_Unit));

	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExtData::Instance()->SelectBrd_DefaultTranslucentLevel.Get()));
	const auto canSee = sIsDisguised && pThis->DisguisedAsHouse ? pThis->DisguisedAsHouse->IsAlliedWith(HouseClass::CurrentPlayer) :
		pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer)
		|| HouseClass::CurrentPlayer->IsObserver()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExtData::Instance()->SelectBrd_DefaultShowEnemy.Get());

	const Point2D offs = pTypeExt->SelectBrd_DrawOffset.Get((isInfantry ?
		RulesExtData::Instance()->SelectBrd_DrawOffset_Infantry : RulesExtData::Instance()->SelectBrd_DrawOffset_Unit));

	XOffset = offs.X;
	YOffset = pTypeExt->This()->PixelSelectionBracketDelta + offs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;

		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}
*/
#include <New/Type/SelectBoxTypeClass.h>

void TechnoExtData::DrawSelectBox(TechnoClass* pThis,Point2D* pLocation,RectangleStruct* pBounds, bool drawBefore)
{
	const auto whatAmI = pThis->WhatAmI();
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	SelectBoxTypeClass* pSelectBox = nullptr;

	if (pTypeExt->SelectBox.isset())
		pSelectBox = pTypeExt->SelectBox.Get();
	else if (whatAmI == InfantryClass::AbsID)
		pSelectBox = RulesExtData::Instance()->DefaultInfantrySelectBox.Get();
	else if (whatAmI != BuildingClass::AbsID)
		pSelectBox = RulesExtData::Instance()->DefaultUnitSelectBox.Get();

	if (!pSelectBox || pSelectBox->DrawAboveTechno == drawBefore)
		return;

	const bool canSee = HouseClass::IsCurrentPlayerObserver() ? pSelectBox->VisibleToHouses_Observer : EnumFunctions::CanTargetHouse(pSelectBox->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer);

	if (!canSee)
		return;

	const double healthPercentage = pThis->GetHealthPercentage();
	//defaultFrame
	const Point3D defaultFrame = whatAmI == AbstractType::Infantry ? Point3D { 1,1,1 } : Point3D { 0,0,0 };
	const auto pSurface = DSurface::Temp();
	const auto flags = (drawBefore ? BlitterFlags::Flat | BlitterFlags::Alpha : BlitterFlags::Nonzero | BlitterFlags::MultiPass) | BlitterFlags::Centered | pSelectBox->Translucency;
	const int zAdjust = drawBefore ? pThis->GetZAdjustment() - 2 : 0;
	const auto pGroundShape = pSelectBox->GroundShape.Get();
	const auto pFoot = flag_cast_to<FootClass*, false>(pThis);

	if ((pGroundShape || pSelectBox->GroundLine)
	&& pSelectBox->Grounded && whatAmI != BuildingClass::AbsID
	&& (pSelectBox->Ground_AlwaysDraw || pThis->IsInAir()))
	{
		CoordStruct coords = pThis->GetCenterCoords();
		auto[outClient, visible] = TacticalClass::Instance->GetCoordsToClientSituation(coords);

		if(pThis->WhatAmI() == AbstractType::Aircraft)
			outClient.Y += Game::AdjustHeight(pThis->GetHeight());
		else
			outClient += pFoot->Locomotor->Shadow_Point();

		if (visible && pGroundShape)
		{
			const auto pPalette = pSelectBox->GroundPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

			const Point3D frames = pSelectBox->GroundFrames.Get(defaultFrame);
			const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;
			auto drawPoint = (outClient + pSelectBox->GroundOffset);
			pSurface->DrawSHP(pPalette, pGroundShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}

		if (pSelectBox->GroundLine)
		{
			Point2D start = *pLocation; // Copy to prevent be modified
			ColorStruct clr = pSelectBox->GroundLineColor.Get(healthPercentage, RulesClass::Instance->ConditionYellow, RulesClass::Instance->ConditionRed);
			const int color = clr.ToInit();

			if (pSelectBox->GroundLine_Dashed)
				pSurface->Draw_Dashed_Line(start, outClient, color, nullptr, 0);
			else
				pSurface->Draw_Line(start, outClient, color);
		}
	}

	if (const auto pShape = pSelectBox->Shape.Get())
	{
		const auto pPalette = pSelectBox->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		const Point3D frames = pSelectBox->Frames.Get(defaultFrame);
		const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;

		const Point2D offset = whatAmI == InfantryClass::AbsID ? Point2D { 8, -3 } : Point2D { 1, -4 };
		Point2D drawPoint = *pLocation + offset + pSelectBox->Offset;

		if (pSelectBox->DrawAboveTechno)
			drawPoint.Y += pType->PixelSelectionBracketDelta;

		pSurface->DrawSHP(pPalette, pShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
}

std::pair<TechnoTypeClass*, HouseClass*> TechnoExtData::GetDisguiseType(TechnoClass* pTarget, bool CheckHouse, bool CheckVisibility, bool bVisibleResult)
{
	TechnoTypeClass* pTypeOut = GET_TECHNOTYPE(pTarget);
	HouseClass* pHouseOut = pTarget->GetOwningHouse();

	//Building cant disguise , so dont bother to check
	if (pTarget->WhatAmI() == BuildingClass::AbsID)
		return { pTypeOut , pHouseOut };

	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible)
	{
		if (CheckHouse)
		{
			if (const auto pDisguiseHouse = pTarget->GetDisguiseHouse(true))
			{
				pHouseOut = pDisguiseHouse;
			}
		}

		if (pTarget->Disguise != pTypeOut)
		{
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise))
			{
				return { pDisguiseType, pHouseOut };
			}
		}
	}

	return { pTypeOut, pHouseOut };
}

TechnoTypeClass* TechnoExtData::GetSimpleDisguiseType(TechnoClass* pTarget, bool CheckVisibility, bool bVisibleResult)
{
	TechnoTypeClass* pTypeOut = GET_TECHNOTYPE(pTarget);

	//Building cant disguise , so dont bother to check
	if (pTarget->WhatAmI() == BuildingClass::AbsID)
		return pTypeOut;

	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible) {
		if (pTarget->Disguise != pTypeOut) {
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise)) {
				return pDisguiseType;
			}
		}
	}

	return pTypeOut;
}

static FORCEDINLINE std::pair<SHPStruct*, int> GetInsigniaDatas(TechnoClass* pThis, TechnoTypeExtData* pTypeExt)
{
	bool isCustomInsignia = false;
	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;
	const auto nCurRank = pThis->CurrentRanking;

	if (SHPStruct* pCustomShapeFile = pTypeExt->Insignia.GetFromSpecificRank(nCurRank))
	{
		pShapeFile = pCustomShapeFile;
		defaultFrameIndex = 0;
		isCustomInsignia = true;
	}

	Vector3D<int> insigniaFrames = pTypeExt->InsigniaFrames.Get();
	int insigniaFrame = insigniaFrames.X;
	int frameIndex = pTypeExt->InsigniaFrame.GetFromSpecificRank(nCurRank);

	if (pTypeExt->This()->Passengers > 0)
	{
		int passengersIndex = pTypeExt->Passengers_BySize ?
			 pThis->Passengers.GetTotalSize() : pThis->Passengers.NumPassengers;
		passengersIndex = MinImpl(passengersIndex, pTypeExt->This()->Passengers);

		if(!pTypeExt->Insignia_Passengers.empty() && (size_t)passengersIndex < pTypeExt->Insignia_Passengers.size()){
			if (auto const pCustomShapeFile = pTypeExt->Insignia_Passengers[passengersIndex].GetFromSpecificRank(nCurRank)) {
				pShapeFile = pCustomShapeFile;
				defaultFrameIndex = 0;
				isCustomInsignia = true;
			}
		}

		if(!pTypeExt->InsigniaFrame_Passengers.empty() && (size_t)passengersIndex < pTypeExt->InsigniaFrame_Passengers.size()){
			int frame = pTypeExt->InsigniaFrame_Passengers[passengersIndex].GetFromSpecificRank(nCurRank);

			if (frame != -1)
				frameIndex = frame;
		}

		if(!pTypeExt->InsigniaFrames_Passengers.empty() && (size_t)passengersIndex < pTypeExt->InsigniaFrames_Passengers.size()){
			auto const& frames = pTypeExt->InsigniaFrames_Passengers[passengersIndex];

			if (!frames->operator==(Vector3D<int>(-1, -1, -1)))
				insigniaFrames = frames.Get();
		}
	}

	if (pTypeExt->This()->Gunner)
	{
		int weaponIndex = pThis->CurrentWeaponNumber;
		auto weaponInsignia = pTypeExt->Insignia_Weapon.data();

		if (auto const pCustomShapeFile = weaponInsignia[weaponIndex].Shapes.GetFromSpecificRank(nCurRank))
		{
			pShapeFile = pCustomShapeFile;
			defaultFrameIndex = 0;
			isCustomInsignia = true;
		}

		int frame = weaponInsignia[weaponIndex].Frame.GetFromSpecificRank(nCurRank);

		if (frame != -1)
			frameIndex = frame;

		auto const& frames = weaponInsignia[weaponIndex].Frames;

		if (!frames->operator==(Vector3D<int>(-1, -1, -1)))
			insigniaFrames = frames.Get();
	}

	switch (nCurRank)
	{
	case Rank::Elite:
		defaultFrameIndex = !isCustomInsignia ? 15 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Z;
		break;
	case Rank::Veteran:
		defaultFrameIndex = !isCustomInsignia ? 14 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Y;
		break;
	default:
		break;
	}

	frameIndex = frameIndex == -1 ? insigniaFrame : frameIndex;

	if (frameIndex == -1)
		frameIndex = defaultFrameIndex;

	return { pShapeFile  , frameIndex };
}

static FORCEDINLINE void GetAdjustedInsigniaOffset(TechnoClass* pThis , Point2D& offset , const CoordStruct& a_) {

	Point2D a__ { a_.X , a_.Y};
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Infantry->operator+(a__));
		break;
	case AbstractType::Building:
		if (RulesExtData::Instance()->DrawInsignia_AdjustPos_BuildingsAnchor.isset())
				offset = (TechnoExtData::GetBuildingSelectBracketPosition(pThis,
						RulesExtData::Instance()->DrawInsignia_AdjustPos_BuildingsAnchor) +
						RulesExtData::Instance()->DrawInsignia_AdjustPos_Buildings) + a__;
			else
				offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Buildings->operator+(a__));

		break;
	default:
		offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Units->operator+(a__));
		break;
	}
}

static FORCEDINLINE TechnoTypeExtData* GetTypeExtData(TechnoClass* pThis , bool isObserver)
{
	TechnoTypeClass* pTechnoType = nullptr;
	auto[pTechnoTyper, pOwner] = TechnoExtData::GetDisguiseType(pThis, true, true, false);
	const bool isDisguised = pTechnoTyper != GET_TECHNOTYPE(pThis);

	if (isDisguised && isObserver) {
		pTechnoType = GET_TECHNOTYPE(pThis);
	} else {
		pTechnoType = pTechnoTyper;
	}

	if (!pTechnoType)
		return nullptr;

	return TechnoTypeExtContainer::Instance.Find(pTechnoType);
}

void TechnoExtData::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->CurrentRanking == Rank::Invalid
		|| RulesExtData::Instance()->DrawInsigniaOnlyOnSelected.Get() && !pThis->IsSelected && !pThis->IsMouseHovering)
		return;

	const bool IsObserverPlayer = HouseExtData::IsObserverPlayer();
	auto pTypeExt = GetTypeExtData(pThis , IsObserverPlayer);

	if (!pTypeExt)
		return;

	Point2D offset = *pLocation;
	const bool IsAlly = pThis->Owner && pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer);

	const bool isVisibleToPlayer = IsAlly
		|| IsObserverPlayer
		|| pTypeExt->Insignia_ShowEnemy.Get(RulesExtData::Instance()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	const auto& [pShapeFile, frameIndex] = GetInsigniaDatas(pThis, pTypeExt);

	if (frameIndex != -1 && pShapeFile)
	{
		GetAdjustedInsigniaOffset(pThis , offset , CoordStruct::Empty);
		offset.Y += RulesExtData::Instance()->DrawInsignia_UsePixelSelectionBracketDelta ? GET_TECHNOTYPE(pThis)->PixelSelectionBracketDelta : 0;
		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

bool TechnoExtData::CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget)
{
	const int wpnIdx = pThis->SelectWeapon(pTarget);
	const FireError fErr = pThis->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)

	{
		return pThis->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExtData::ForceJumpjetTurnToTarget(TechnoClass* pThis)
{
	const auto pFoot = cast_to<UnitClass*, false>(pThis);
	if (!pFoot)
		return;

	const auto pType = pFoot->Type;
	const auto pLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pFoot->Locomotor);

	if (pLoco && pThis->IsInAir()
		&& !pType->TurretSpins)
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->JumpjetTurnToTarget.Get(RulesExtData::Instance()->JumpjetTurnToTarget)
		   && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				if (!pLoco->Facing.Is_Rotating() && TechnoExtData::CheckIfCanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));

					if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
						pLoco->Facing.Set_Desired(tgtDir);
				}
			}
		}
	}
}

// convert UTF-8 string to wstring
static std::wstring Str2Wstr(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void TechnoExtData::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage, WarheadTypeClass* pWH)
{
	if (!pThis || !pThis->IsAlive || pThis->InLimbo || !pThis->IsOnMyView())
		return;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const ColorStruct color = isShieldDamage ? damage > 0 ? Phobos::Defines::ShieldPositiveDamageColor : Phobos::Defines::ShieldPositiveDamageColor :
		damage > 0 ? Drawing::DefaultColors[(int)DefaultColorList::Red] : Drawing::DefaultColors[(int)DefaultColorList::Green];

	static fmt::basic_memory_buffer<wchar_t> damageStr;
	damageStr.clear();

	if (Phobos::Otamaa::IsAdmin)
		fmt::format_to(std::back_inserter(damageStr) ,L"[{}] {} ({})", PhobosCRT::StringToWideString(pThis->get_ID()), PhobosCRT::StringToWideString(pWH->ID), damage);
	else
		fmt::format_to(std::back_inserter(damageStr) ,L"{}", damage);

	damageStr.push_back(L'\0');
	auto coords = pThis->GetCenterCoords();
	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr.data(), &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset == INT32_MIN)
		pExt->DamageNumberOffset = -maxOffset;

	if (auto pBuilding = cast_to<BuildingClass*, false>(pThis))
		coords.Z += 104 * pBuilding->Type->Height;
	else
		coords.Z += 256;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
	{
		if (!pCell->IsFogged() && !pCell->IsShrouded())
		{
			if (pThis->VisualCharacter(0, HouseClass::CurrentPlayer()) != VisualType::Hidden)
			{
				FlyingStrings::Instance.Add(damageStr, coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });
			}
		}
	}

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

void TechnoExtData::Stop(TechnoClass* pThis, Mission const& eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.clear();
	pThis->SetArchiveTarget(nullptr);
	pThis->Stun();
}

bool TechnoExtData::IsOnLimbo(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->InLimbo && !pThis->Transporter;
}

bool TechnoExtData::IsDeactivated(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->Deactivated;
}

bool TechnoExtData::IsUnderEMP(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->IsUnderEMP();
}

bool TechnoExtData::IsActive(TechnoClass* pThis, bool bCheckEMP, bool bCheckDeactivated, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!TechnoExtData::IsAlive(pThis, bIgnoreLimbo, bIgnoreIsOnMap, bIgnoreAbsorb))
		return false;

	if (pThis->BeingWarpedOut || pThis->TemporalTargetingMe || IsUnderEMP(pThis, !bCheckEMP) || IsDeactivated(pThis, !bCheckDeactivated))
		return false;

	return true;
}

bool TechnoExtData::IsAlive(TechnoClass* pThis, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!pThis || !pThis->IsAlive || pThis->Health <= 0)
		return false;

	if ((IsOnLimbo(pThis, !bIgnoreLimbo)) || (pThis->Absorbed && !bIgnoreAbsorb) || (!pThis->IsOnMap && !bIgnoreIsOnMap))
		return false;

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	if (pThis->WhatAmI() == UnitClass::AbsID)
		return (static_cast<UnitClass*>(pThis)->DeathFrameCounter > 0) ? false : true;

	return true;
}

void TechnoExtData::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;

	auto pKillerType = GET_TECHNOTYPE(pKiller);
	auto pVictimType = GET_TECHNOTYPE(pVictim);

	TechnoClass* pObjectKiller = (pKillerType->Spawned || pKillerType->MissileSpawn) && pKiller->SpawnOwner ?
		pKiller->SpawnOwner : pKiller;

	if (pObjectKiller)
	{
		TechnoExtData::ObjectKilledBy(pVictim, pObjectKiller->Owner);

		if(pObjectKiller->BelongsToATeam()) {
			if (auto const pFootKiller = flag_cast_to<FootClass*, false>(pObjectKiller)) {
				auto pKillerExt = TechnoExtContainer::Instance.Find(pObjectKiller);

				if (auto const pFocus = flag_cast_to<TechnoClass*>(pFootKiller->Team->ArchiveTarget))
				{
					auto pFocusType = GET_TECHNOTYPE(pFocus);

					pKillerExt->LastKillWasTeamTarget =
						pFocusType == pVictimType
						|| TeamExtData::IsEligible(pFocus, pVictimType)
						|| TeamExtData::IsEligible(pVictim, pFocusType)
						;
				}

				auto pKillerTeamExt = TeamExtContainer::Instance.Find(pFootKiller->Team);

				if (pKillerTeamExt->ConditionalJump_EnabledKillsCount)
				{
					bool isValidKill =
						 pKillerTeamExt->ConditionalJump_Index < 0 ?
						 false :
						ScriptExtData::EvaluateObjectWithMask(pVictim, pKillerTeamExt->ConditionalJump_Index, -1, -1, pKiller);

					if (isValidKill || pKillerExt->LastKillWasTeamTarget)
						pKillerTeamExt->ConditionalJump_Counter++;
				}

				// Special case for interrupting current action
				if (pKillerTeamExt->AbortActionAfterKilling
					&& pKillerExt->LastKillWasTeamTarget)
				{
					pKillerTeamExt->AbortActionAfterKilling = false;
					auto pTeam = pFootKiller->Team;

					const auto&[curAction , curArgs] = pTeam->CurrentScript->GetCurrentAction();
					const auto&[nextAction , nextArgs] = pTeam->CurrentScript->GetNextAction();

					Debug::LogInfo("DEBUG: [{}] [{}] {} = {},{} - Force next script action after successful kill: {} = {},{}"
						, pTeam->Type->ID
						, pTeam->CurrentScript->Type->ID
						, pTeam->CurrentScript->CurrentMission
						, (int)curAction
						, curArgs
						, pTeam->CurrentScript->CurrentMission + 1
						, (int)nextAction
						, nextArgs
					);

					// Jumping to the next line of the script list
					pTeam->StepCompleted = true;

				}

			}
		}
	}
}

void TechnoExtData::ObjectKilledBy(TechnoClass* pVictim, HouseClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;

	if (pKiller != pVictim->Owner) {
		auto pHouseExt = HouseExtContainer::Instance.Find(pKiller);

		if (pHouseExt->AreBattlePointsEnabled()) {
			pHouseExt->UpdateBattlePoints(pHouseExt->CalculateBattlePoints(pVictim));
		}
	}
}

void TechnoExtData::UpdateMCRangeLimit()
{
	auto const pThis = This();
	auto const pCManager = pThis->CaptureManager;

	if (!pCManager)
		return;

	const int Range = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis))->MindControlRangeLimit.Get();

	if (Range <= 0)
		return;

	for (const auto& node : pCManager->ControlNodes) {
		if (pCManager->Owner->DistanceFrom(node->Unit) > Range) {
			pCManager->FreeUnit(node->Unit);
		}
	}
}

void TechnoExtData::UpdateInterceptor()
{
	auto const pThis = This();

	if (!this->IsInterceptor())
		return;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	const int delay = Math::abs(pTypeExt->Interceptor_TargetingDelay.Get(pThis));

	if (Unsorted::CurrentFrame % delay != 0)
		return;

	if (auto pTarget = pThis->Target) {
		if (pTarget->WhatAmI() != AbstractType::Bullet)
			return;

		const auto pTargetExt = BulletExtContainer::Instance.Find(static_cast<BulletClass*>(pTarget));

		if ((pTargetExt->InterceptedStatus & InterceptedStatus::Locked) == InterceptedStatus::None)
			return;
	}

	const int count = BulletClass::Array->Count;

	if (this->IsBurrowed || !count)
		return;

	if (TechnoExtData::IsInWarfactory(pThis))
		return;

	if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir())
		return;

	if (auto const pTransport = pThis->Transporter)
	{
		if(!pTransport->IsAlive)
			return;

		if (pTransport->WhatAmI() == AircraftClass::AbsID && !pTransport->IsInAir())
			return;

		if (TechnoExtData::IsInWarfactory(pTransport))
			return;

		if (TechnoExtContainer::Instance.Find(pTransport)->IsBurrowed)
			return;
	}


	BulletClass* pTargetBullet = nullptr;

	const auto& guardRange = pTypeExt->Interceptor_GuardRange.Get(pThis);
	const double guardRangeSq = guardRange * guardRange;
	const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);
	const double minguardRangeSq = minguardRange * minguardRange;

	// Interceptor weapon is always fixed
	const auto pWeapon = pThis->GetWeapon(pTypeExt->Interceptor_Weapon)->WeaponType;
	const auto wpnRange = pWeapon->Range;
	const auto wpnRangeS1q = wpnRange * wpnRange;

	const auto wpnminRange = pWeapon->MinimumRange;
	const auto wpnminRangeS1q = wpnminRange * wpnminRange;

	// DO NOT iterate BulletExt::ExtMap here, the order of items is not deterministic
	// so it can differ across players throwing target management out of sync.
	int i = 0;

	for (; i < count; ++i)
	{
		const auto& pBullet = BulletClass::Array->Items[i];
		const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const bool isTargetedOrLocked = bool(pBulletExt->InterceptedStatus &
			(InterceptedStatus::Targeted | InterceptedStatus::Locked));

		// If we already have an optional target skip ones that are already being targeted etc.
		if (pTargetBullet && isTargetedOrLocked)
			continue;

		const auto distanceSq = pBullet->Location.DistanceFromSquared(pThis->Location);

		if (pTypeExt->Interceptor_ConsiderWeaponRange.Get() &&
			(distanceSq > wpnRangeS1q || distanceSq < wpnminRangeS1q))
			continue;

		if (distanceSq > guardRangeSq || distanceSq < minguardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			auto const pWhExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (Math::abs(pWhExt->GetVerses(pBulletTypeExt->Armor).Verses)
				//GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead , pBulletTypeExt->Armor))
				< 0.001)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->Owner;

		if (!EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses, pThis->Owner, bulletOwner))
			continue;

		if (!pTargetBullet && isTargetedOrLocked)
		{
			// Set as optional target
			pTargetBullet = pBullet;
			break;
		}

		// Establish target
		pThis->SetTarget(pBullet);
		return;
	}

	if (pTargetBullet)
		pThis->SetTarget(pTargetBullet);  // There is no more suitable target, establish optional target
}

void TechnoExtData::UpdateTiberiumEater()
{
	const auto pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	const auto pEaterType = pTypeExt->TiberiumEaterType.get();

	if (!pEaterType)
		return;

	const int transDelay = pEaterType->TransDelay;

	if (transDelay && this->TiberiumEaterTimer.InProgress())
		return;

	const auto pOwner = pThis->Owner;
	bool active = false;
	const bool displayCash = pEaterType->Display && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer);
	int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

	if (facing >= 7)
		facing = 0;
	else
		facing++;

	const int cellCount = static_cast<int>(pEaterType->Cells.size());

	for (int idx = 0; idx < cellCount; idx++)
	{
		const auto& cellOffset = pEaterType->Cells[idx];
		const auto pos = TechnoExtData::GetFLHAbsoluteCoords(pThis, CoordStruct { cellOffset.X, cellOffset.Y, 0 }, false);
		const auto pCell = MapClass::Instance->TryGetCellAt(pos);

		if (!pCell)
			continue;

		if (const int contained = pCell->GetContainedTiberiumValue())
		{
			const int tiberiumIdx = pCell->GetContainedTiberiumIndex();
			const int tiberiumValue = TiberiumClass::Array->Items[tiberiumIdx]->Value;
			const int tiberiumAmount = static_cast<int>(static_cast<double>(contained) / tiberiumValue);
			const int amount = pEaterType->AmountPerCell > 0 ? MinImpl(pEaterType->AmountPerCell.Get(), tiberiumAmount) : tiberiumAmount;
			pCell->ReduceTiberium(amount);
			const float multiplier = pEaterType->CashMultiplier * (1.0f + pOwner->NumOrePurifiers * RulesClass::Instance->PurifierBonus);
			const int value = static_cast<int>(std::round(amount * tiberiumValue * multiplier));
			pOwner->TransactMoney(value);
			active = true;

			if (displayCash)
			{
				auto cellCoords = pCell->GetCoords();
				cellCoords.Z = std::max(pThis->Location.Z, cellCoords.Z);
				FlyingStrings::Instance.AddMoneyString(true, value, pOwner, pEaterType->DisplayToHouse, cellCoords, pEaterType->DisplayOffset, ColorStruct::Empty);
			}

			const auto& anims = pEaterType->Anims_Tiberiums[tiberiumIdx].GetElements(pEaterType->Anims);
			const int animCount = static_cast<int>(anims.size());

			if (animCount == 0)
				continue;

			AnimTypeClass* pAnimType = nullptr;

			switch (animCount)
			{
			case 1:
				pAnimType = anims[0];
				break;

			case 8:
				pAnimType = anims[facing];
				break;

			default:
				pAnimType = anims[ScenarioClass::Instance->Random.RandomRanged(0, animCount - 1)];
				break;
			}

			if (pAnimType)
			{
				const auto pAnim = GameCreate<AnimClass>(pAnimType, pos);
				AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, pThis, true, false);

				if (pEaterType->AnimMove)
					pAnim->SetOwnerObject(pThis);
			}
		}
	}

	if (active && transDelay)
		this->TiberiumEaterTimer.Start(pEaterType->TransDelay);
}

void TechnoExtData::UpdateSpawnLimitRange()
{
	auto const pThis = This();
	const auto pExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	auto const pManager = pThis->SpawnManager;

	if (!pExt->Spawn_LimitedRange || !pManager)
		return;

	int weaponRange = pThis->Veterancy.IsElite() ? pExt->EliteSpawnerRange : pExt->SpawnerRange;
	if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
		pManager->ResetTarget();
}

bool TechnoExtData::IsHarvesting(TechnoClass* pThis)
{
	const auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	const auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExtData::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExtData::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const& pBld : GET_TECHNOTYPE(pThis)->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld) > 0)
			return true;
	}

	return false;
}

void TechnoExtData::InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pType = GET_TECHNOTYPE(pThis);

	if (pThis->WhatAmI() == AbstractType::Building || pType->Invisible)
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (bIsconverted)
		pExt->LaserTrails.clear();

	auto const pOwner = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExtData::FindFirstCivilianHouse();

	if (pExt->LaserTrails.empty())
	{
		pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());
		for (auto const& entry : pTypeExt->LaserTrailData) {
			pExt->LaserTrails.emplace_back((std::make_unique<LaserTrailClass>(
					LaserTrailTypeClass::Array[entry.idxType].get(), pOwner->LaserColor, entry.FLH, entry.IsOnTurret)));
		}
	}

}

void TechnoExtData::UpdateLaserTrails(TechnoClass* pThis) {

	HelperedVector<std::unique_ptr<LaserTrailClass>> dummy;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	dummy.reserve(pExt->LaserTrails.size());

	for (auto& entry : pExt->LaserTrails) {
		if (entry->Permanent)
			dummy.emplace_back(std::move(entry));
	}

	pExt->LaserTrails.clear();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size() + dummy.size());

	for (auto const& entry : pTypeExt->LaserTrailData) {
		pExt->LaserTrails.emplace_back((std::make_unique<LaserTrailClass>(
			LaserTrailTypeClass::Array[entry.idxType].get(), pThis->Owner->LaserColor, entry.FLH, entry.IsOnTurret)));
	}

	for (auto& entry_d : dummy) {
		pExt->LaserTrails.emplace_back(std::move(entry_d));
	}
}

void TechnoExtData::InitializeAttachEffects(TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->PhobosAttachEffects.AttachTypes.size() < 1)
		return;

	PhobosAttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, &pTypeExt->PhobosAttachEffects);
}

bool TechnoExtData::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	if (!pWeaponType)
		return false;

	WeaponTypeExtData::DetonateAt1(pWeaponType, pThis, pThis, true, nullptr);
	return true;
}

Matrix3D TechnoExtData::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D Mtx {};
	// Step 1: get body transform matrix
	if (pThis && (pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor) {

		if(isShadow)
			((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Shadow_Matrix(&Mtx, pKey);
		else
			((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Draw_Matrix(&Mtx, pKey);

		return Mtx;
	}

	// no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
	return Matrix3D::GetIdentity();
}

// reversed from 6F3D60
CoordStruct TechnoExtData::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret, int turIdx)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	Matrix3D mtx = TechnoExtData::GetTransform(pThis);
	auto pFoot = flag_cast_to<FootClass*>(pThis);

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && (pThis->HasTurret() || !pFoot))
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, 1.0);
		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		// For BuildingClass turret facing is equal to primary facing

		float angle = pFoot ? (float)(turretRad - bodyRad) : (float)(turretRad);
		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(flh.X), static_cast<float>(flh.Y), static_cast<float>(flh.Z));

	Vector3D<float> result {};
	Matrix3D::MatrixMultiply(&result, &mtx, &Vector3D<float>::Empty);
	//Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	//auto result = mtx.GetTranslation();

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetRenderCoords();
	location += CoordStruct { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };
	// += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

double TechnoExtData::GetROFMult(TechnoClass const* pTech)
{
	const bool rofAbility = pTech->HasAbility(AbilityType::ROF);

	return !rofAbility ? 1.0 :
		RulesClass::Instance->VeteranROF * ((!pTech->Owner || !pTech->Owner->Type) ?
			1.0 : pTech->Owner->Type->ROFMult);
}

int TechnoExtData::GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass* pPassenger)
{
	auto const pData = TechnoTypeExtContainer::Instance.Find(pTransporterData);
	auto const pThis = This();
	auto const pDelType = &pData->PassengerDeletionType;

	int nRate = 0;
	auto pPassengerType = GET_TECHNOTYPE(pPassenger);

	if (pDelType->UseCostAsRate.Get())
	{
		// Use passenger cost as countdown.
		auto timerLength = static_cast<int>(pPassengerType->Cost * pDelType->CostMultiplier);
		const auto nCostRateCap = pDelType->CostRateCap.Get(-1);
		if (nCostRateCap > 0)
			timerLength = MinImpl(timerLength, nCostRateCap);

		nRate = timerLength;
	}
	else
	{
		// Use explicit rate optionally multiplied by unit size as countdown.
		auto timerLength = pDelType->Rate.Get();
		if (pDelType->Rate_SizeMultiply.Get() && pPassengerType->Size > 1.0)
			timerLength *= static_cast<int>(pPassengerType->Size + 0.5);

		nRate = timerLength;
	}

	if (pDelType->Rate_AffectedByVeterancy)
	{
		auto const nRof = TechnoExtData::GetROFMult(pThis);
		if (nRof != 1.0)
		{
			nRate = static_cast<int>(nRate * nRof);
		}
	}

	return nRate;
}

static int GetTotalSoylentOfPassengers(TechnoClass* pThis, PassengerDeletionTypeClass* pDelType, FootClass* pPassenger)
{
	FootClass* pPassengerL2;
	int nMoneyToGive = 0;
	while (pPassenger->Passengers.NumPassengers > 0)
	{
		pPassengerL2 = pPassenger->Passengers.RemoveFirstPassenger();

		if (pPassengerL2) {
			auto pSource = pDelType->DontScore ? nullptr : pThis;
			nMoneyToGive += (int)(GET_TECHNOTYPE(pPassengerL2)->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);
			if (pPassengerL2->Passengers.NumPassengers > 0) {
				nMoneyToGive += GetTotalSoylentOfPassengers(pThis, pDelType, pPassengerL2);
			}

			if (const auto pParasite = pPassenger->ParasiteEatingMe) {
				nMoneyToGive += (int)(GET_TECHNOTYPE(pPassengerL2)->GetRefund(pParasite->Owner, true) * pDelType->SoylentMultiplier);
				pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
				pParasite->ParasiteImUsing->ExitUnit();
			}

			const int hijack = pPassenger->HijackerInfantryType;

			if (hijack != -1) {
				const auto pHijackerType = InfantryTypeClass::Array->Items[hijack];
				nMoneyToGive += (int)(pHijackerType->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);
			}

			pPassengerL2->KillPassengers(pSource);
			pPassengerL2->RegisterDestruction(pSource);
			pPassengerL2->UnInit();
		}
	}
	return nMoneyToGive;
}

void TechnoExtData::UpdateEatPassengers()
{
	auto const pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
	auto const pDelType = &pTypeExt->PassengerDeletionType;

	if (!pDelType->Enabled || !TechnoExtData::IsActive(pThis, false))
		return;

	if (!pDelType->UnderEMP && (pThis->Deactivated || pThis->IsUnderEMP())) {
		if (this->PassengerDeletionTimer.InProgress())
			this->PassengerDeletionTimer.StartTime++;

		return;
	}

		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(pDelType->AllowedHouses, pThis->Owner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = flag_cast_to <FootClass* , false>(pLastPassenger);
					pPassenger = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = flag_cast_to<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPassenger)
			{
				this->PassengerDeletionTimer.Stop();
				return;
			}

			if (!this->PassengerDeletionTimer.HasStarted()) // Execute only if timer has been stopped or not started
			{
				// Setting & start countdown. Bigger units needs more time
				int timerLength = this->GetEatPassangersTotalTime(GET_TECHNOTYPE(pThis), pPassenger);

				if (timerLength <= 0)
					return;

				this->PassengerDeletionTimer.Start(timerLength);
			}

			if (this->PassengerDeletionTimer.Completed()) // Execute only if timer has ran out after being started
			{
				--pThis->Passengers.NumPassengers;

				if (pLastPassenger)
					pLastPassenger->NextObject = nullptr;

				if (pPreviousPassenger)
					pPreviousPassenger->NextObject = pPassenger->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				auto const pPassengerType = GET_TECHNOTYPE(pPassenger);

				{
					pPassenger->LiberateMember();

					auto const& nReportSound = pDelType->ReportSound;
					if(nReportSound.isset())
						VocClass::SafeImmedietelyPlayAt(nReportSound, &pThis->Location);

					auto const pThisOwner = pThis->GetOwningHouse();

					if (const auto pAnimType = pDelType->Anim.Get(nullptr))
					{
						auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
						pAnim->SetOwnerObject(pThis);
						AnimExtData::SetAnimOwnerHouseKind(pAnim, pThisOwner, pPassenger->GetOwningHouse(), pThis, false, false);
					}

					// Check if there is money refund
					if (pDelType->Soylent &&
						EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
					{
						int nMoneyToGive = (int)(pPassengerType->GetRefund(pPassenger->Owner, true) *
							pDelType->SoylentMultiplier);

						if (pPassenger->Passengers.NumPassengers > 0) {
							nMoneyToGive += GetTotalSoylentOfPassengers(pThis, pDelType, pPassenger);
						}

						if (const auto pParasite = pCurrentPassenger->ParasiteEatingMe) {
							nMoneyToGive += (int)(GET_TECHNOTYPE(pCurrentPassenger)->GetRefund(pParasite->Owner, true) * pDelType->SoylentMultiplier);
							pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
							pParasite->ParasiteImUsing->ExitUnit();
						}

						const int hijack = pCurrentPassenger->HijackerInfantryType;

						if (hijack != -1) {
							const auto pHijackerType = InfantryTypeClass::Array->Items[hijack];
							nMoneyToGive += (int)(pHijackerType->GetRefund(pThis->Owner, true) * pDelType->SoylentMultiplier);
						}

						if (nMoneyToGive)
						{
							pThis->Owner->TransactMoney(nMoneyToGive);

							if (pDelType->DisplaySoylent)
							{
								FlyingStrings::Instance.AddMoneyString(true, nMoneyToGive, pThis,
									pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset, ColorStruct::Empty);
							}
						}
					}

					// Handle gunner change.
					if (GET_TECHNOTYPE(pThis)->Gunner)
					{
						if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
						{
							pFoot->RemoveGunner(pPassenger);
							FootClass* pGunner = nullptr;

							for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = flag_cast_to<FootClass*>(pNext->NextObject))
								pGunner = pNext;

							pFoot->ReceiveGunner(pGunner);
						}
					}

					if (GET_TECHNOTYPE(pThis)->OpenTopped)
					{
						pThis->ExitedOpenTopped(pPassenger);
					}

					if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
					{
						if (pBld->Absorber())
						{
							pPassenger->Absorbed = false;

							if (pBld->Type->ExtraPowerBonus || pBld->Type->ExtraPowerDrain)
							{
								pBld->Owner->RecheckPower = true;
							}
						}
					}

					pPassenger->Transporter = nullptr;
					pPassenger->BunkerLinkedItem = nullptr;
					//auto const pPassengerOwner = pPassenger->Owner;

					//if (!pPassengerOwner->IsNeutral() && !GET_TECHNOTYPE(pThis)->Insignificant)
					//{
					//	pPassengerOwner->RegisterLoss(pPassenger, false);
					//	pPassengerOwner->RemoveTracking(pPassenger);

					//	if (!pPassengerOwner->RecheckTechTree)
					//		pPassengerOwner->RecheckTechTree = true;
					//}


					pPassenger->RegisterDestruction(pDelType->DontScore ? nullptr : pThis);
					//Debug::LogInfo(__FUNCTION__" Called ");
					TechnoExtData::HandleRemove(pPassenger, pDelType->DontScore ? nullptr : pThis, false, false);
				}

				this->PassengerDeletionTimer.Stop();
			}
		}
}

bool NOINLINE TechnoExtData::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (GET_TECHNOTYPE(pThis)->Ammo > 0)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));
		if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
			return true;
	}

	return false;
}

#include <AircraftTrackerClass.h>

void TechnoExtData::HandleRemove(TechnoClass* pThis, TechnoClass* pSource, bool SkipTrackingRemove, bool Delete)
{
	// kill passenger cargo to prevent memleak
	const auto pThisType = GET_TECHNOTYPE(pThis);

	if (pThisType->OpenTopped) {
		pThis->MarkPassengersAsExited();
	}

	pThis->KillPassengers(pSource);

	const auto nWhat = pThis->WhatAmI();
	Debug::LogInfo(__FUNCTION__" Called For[({}){} - {}][{} - {}](Method :{})",
		AbstractClass::GetAbstractClassName(nWhat),
		pThisType->ID,
		(void*)pThis,
		pThis->Owner ? pThis->Owner->get_ID() : GameStrings::NoneStr(),
		(void*)pThis->Owner,
		Delete ? "Delete" :  "UnInit"
	);

	if (nWhat == BuildingClass::AbsID)
	{
		static_cast<BuildingClass*>(pThis)->KillOccupants(nullptr);
	} else {

		if (pThis->InLimbo && ((FootClass*)pThis)->ParasiteImUsing && ((FootClass*)pThis)->ParasiteImUsing->Victim)
			((FootClass*)pThis)->ParasiteImUsing->ExitUnit();

		const auto flight = pThis->GetLastFlightMapCoords();
		if(flight.IsValid())
			AircraftTrackerClass::Instance->Remove((FootClass*)pThis);

		if(auto& pTeam = pThis->OldTeam){
			pTeam->RemoveMember((FootClass*)pThis);
			pTeam->Reacalculate();
			pTeam = nullptr;
		}
	}

	//if (!Delete && !SkipTrackingRemove)
	//{
	//	if (const auto pOwner = pThis->GetOwningHouse())
	//	{
	//		if(!pThis->InLimbo)
	//			pOwner->RegisterLoss(pThis, false);
	//
	//		pOwner->RemoveTracking(pThis);
	//
	//		if (!pOwner->RecheckTechTree)
	//			pOwner->RecheckTechTree = true;
	//	}
	//}


	if (Delete)
		GameDelete<true, false>(pThis);
	else
	pThis->UnInit();

	// Handle extra power
	if (pThis->Absorbed && pThis->Transporter)
		pThis->Transporter->Owner->RecheckPower = true;
}

void TechnoExtData::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		// this shit is not really good idea to pull of
		// some stuffs doesnt really handled properly , wtf
		bool SkipRemoveTracking = false;
		if (!pThis->InLimbo)
		{
			SkipRemoveTracking = true;
			pThis->Limbo();
		}

		//Debug::LogInfo(__FUNCTION__" (2args) Called ");
		TechnoExtData::HandleRemove(pThis, nullptr, SkipRemoveTracking, false);
	}else{
		TechnoExtData::Kill(pThis, nullptr);
	}
}

static KillMethod NOINLINE GetKillMethod(KillMethod deathOption)
{
	if (deathOption == KillMethod::Random)
	{ //ensure no death loop , only random when needed to
		return ScenarioClass::Instance->Random.RandomRangedSpecific<KillMethod>(KillMethod::Explode, KillMethod::Sell);
	}

	return deathOption;
}

void TechnoExtData::Kill(TechnoClass* pThis, TechnoClass* pKiller) {
	if (pThis->IsAlive) {
		auto nHealth = pThis->GetType()->Strength;
		pThis->ReceiveDamage(&nHealth, 0, RulesClass::Instance()->C4Warhead, pKiller, true, false, pKiller ? pKiller->Owner : nullptr);
	}
}

void TechnoExtData::KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill, AnimTypeClass* pVanishAnim)
{
	if (!pThis || deathOption == KillMethod::None || !pThis->IsAlive)
		return;

	if (auto pBld = cast_to<BuildingClass*, false>(pThis)) {
		auto pBldExt = BuildingExtContainer::Instance.Find(pBld);

		if (pBldExt->LimboID >= 0) {
			BuildingExtData::LimboKill(pBld);
			return;
		}
	}

	auto const pWhat = VTable::Get(pThis);

	switch (GetKillMethod(deathOption))
	{
	case KillMethod::Explode:
	{
		TechnoExtData::Kill(pThis, nullptr);
	}break;
	case KillMethod::Vanish:
	{
		if (pWhat == BuildingClass::vtable) {
			const auto pBld = static_cast<BuildingClass*>(pThis);

			if (pThis->BunkerLinkedItem)
				pBld->UnloadBunker();
		}

		// this shit is not really good idea to pull off
		// some stuffs doesnt really handled properly , wtf

		if (pVanishAnim && !pThis->InLimbo && (pThis->Location.IsValid() && pThis->InlineMapCoords().IsValid()))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pVanishAnim, pThis->GetCoords()),
				pThis->GetOwningHouse(),
				nullptr,
				 true
			);
		}

		pThis->Stun();
		bool skiptrackingremove = false;

		if (!pThis->InLimbo){
			skiptrackingremove = true;
			pThis->Limbo();
		}

		if (RegisterKill)
			pThis->RegisterKill(pThis->Owner);

		//Debug::LogInfo(__FUNCTION__" Called ");
		TechnoExtData::HandleRemove(pThis, nullptr, skiptrackingremove, false);

	}break;
	case KillMethod::Sell:
	{
		if (pWhat == BuildingClass::vtable)
		{
			const auto pBld = static_cast<BuildingClass*>(pThis);

			if (pBld->HasBuildup && (pBld->CurrentMission != Mission::Selling || pBld->CurrentMission != Mission::Unload))
			{
				BuildingExtContainer::Instance.Find(pBld)->Silent = true;
				pBld->Sell(true);
				return;
			}
		}
		else if (pThis->AbstractFlags & AbstractFlags::Foot)
		{
			const auto pFoot = static_cast<FootClass*>(pThis);

			if (pWhat != InfantryClass::vtable && pFoot->CurrentMission != Mission::Unload)
			{
				if (auto const pCell = pFoot->GetCell())
				{
					if (auto const pBuilding = pCell->GetBuilding())
					{
						if (pBuilding->Type->UnitRepair)
						{
							pFoot->Sell(true);
							return;
						}
					}
				}
			}
		}

		if (pThis) {
			TechnoExtData::Kill(pThis, nullptr);
		}

	}break;
	case KillMethod::Convert:
	{
		if(pThis && pThis->IsAlive && pThis->AbstractFlags & AbstractFlags::Foot) {
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

			if (auto pTo = pTypeExt->Convert_AutoDeath) {
				TechnoExtData::ConvertToType(pThis, pTo);
			}
		}
	}break;
	default:
	break;
	}
}

// Otamaa : preparation
// not sure when it gonna finish
enum class DeathConditions : char {
	CountDown = 0 ,
	NoAmmo,
	ListOfIfExist,
	ListOfIfNoExist,
	SlaveOwnerDie,
	NeutralTechno,
	CivilianTechno,
	AIControlled, //https://github.com/Phobos-developers/Phobos/discussions/1198
	PreWeaponFiring,
	AfterWeaponFiring,
	ChangeOwner
};

bool NOINLINE ImmeditelyReturn(TechnoClass* pTech, bool any, bool& result)
{
	//only immedieltely return if the techno dies
	if (!any && !pTech->IsAlive) {
		result = true;
		return true;
	} else if(any) { // immedietely retyrn and check the result
		result = !pTech->IsAlive;
		return true;
	}

	return false; //continue func
}

// Feature: Kill Object Automatically
// https://github.com/Phobos-developers/Phobos/pull/1346
// TODO : update
bool TechnoExtData::CheckDeathConditions()
{
	auto const pThis = This();
	const auto pTypeThis = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTypeThis);

	const KillMethod nMethod = pTypeExt->Death_Method.Get();
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation.Get();
	bool result = false;

	if (nMethod == KillMethod::None)
		return result;

	const bool Any = pTypeExt->AutoDeath_ContentIfAnyMatch;

	// Death by owning house
	if (pTypeExt->AutoDeath_OwnedByPlayer)
	{
		if (pThis->Owner && pThis->Owner->IsControlledByHuman()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}
		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_OwnedByAI)
	{
		if (pThis->Owner && !pThis->Owner->IsControlledByHuman()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by money
	if (pTypeExt->AutoDeath_MoneyExceed >= 0) {
		if (pThis->Owner && pThis->Owner->Available_Money() >= pTypeExt->AutoDeath_MoneyExceed) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_MoneyBelow >= 0) {
		if (pThis->Owner && pThis->Owner->Available_Money() <= pTypeExt->AutoDeath_MoneyBelow) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by power
	if (pTypeExt->AutoDeath_LowPower) {
		if (pThis->Owner && pThis->Owner->HasLowPower()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_FullPower) {
		if (pThis->Owner && pThis->Owner->HasFullPower()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo) {
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by passengers
	if (pTypeExt->AutoDeath_PassengerExceed >= 0)
	{
		if (pTypeThis->Passengers > 0 && pThis->Passengers.NumPassengers >= pTypeExt->AutoDeath_PassengerExceed) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_PassengerBelow >= 0) {
		if (pTypeThis->Passengers > 0 && pThis->Passengers.NumPassengers <= pTypeExt->AutoDeath_PassengerBelow) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	const auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
	{
		const auto existSingleType = [pThis, affectedHouse, allowLimbo](TechnoTypeClass* pType)
		{
			if(affectedHouse == AffectedHouse::Owner)
			{
				if(allowLimbo) {
					for (const auto& limbo : HouseExtContainer::Instance.LimboTechno) {
						if (!limbo->IsAlive || limbo->Owner != pThis->Owner)
							continue;

						const auto limboType = GET_TECHNOTYPE(limbo);
						if (!limboType->Insignificant && !limboType->DontScore && limboType== pType)
							return true;
					}
				}

				return pThis->Owner->CountOwnedAndPresent(pType) > 0;

			} else if(affectedHouse != AffectedHouse::None){

				for (HouseClass* pHouse : *HouseClass::Array)
				{
					if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse))
					{
						if(allowLimbo) {
							for (const auto& limbo : HouseExtContainer::Instance.LimboTechno) {
								if (!limbo->IsAlive || limbo->Owner != pHouse)
									continue;

								const auto limboType = GET_TECHNOTYPE(limbo);
								if (!limboType->Insignificant && !limboType->DontScore && limboType== pType)
									return true;
							}
						}

						return  pHouse->CountOwnedAndPresent(pType) > 0;
					}
				}
			}

			return false;
		};

		return any
			? vTypes.Any_Of(existSingleType)
			: vTypes.All_Of(existSingleType);
	};

	// Death if nonexist
	if (!pTypeExt->AutoDeath_Nonexist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_Nonexist,
			pTypeExt->AutoDeath_Nonexist_House,
			!pTypeExt->AutoDeath_Nonexist_Any, pTypeExt->AutoDeath_Nonexist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if exist
	if (!pTypeExt->AutoDeath_Exist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_Exist,
			pTypeExt->AutoDeath_Exist_House,
			pTypeExt->AutoDeath_Exist_Any,
			pTypeExt->AutoDeath_Exist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (!Death_Countdown.HasStarted())
		{
			Death_Countdown.Start(pTypeExt->Death_Countdown);
			HouseExtContainer::Instance.AutoDeathObjects.insert(pThis, nMethod);
		}
		else if (Death_Countdown.Completed())
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	return result;
}

//TODO : finish this if merged i suppose
constexpr void CountSelfHeal(HouseClass* pOwner, int& count, Nullable<int>& cap, bool allowPlayerControl, bool allowAllies, SelfHealGainType type)
{
	if (pOwner->Defeated ||
		(pOwner->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive))
		return;

		switch (type)
		{
		case SelfHealGainType::Infantry:
		{
			if (pOwner->InfantrySelfHeal <= 0)
				return;

			count = pOwner->InfantrySelfHeal;
			break;
		}
		case SelfHealGainType::Units:
		{
			if (pOwner->UnitsSelfHeal <= 0)
				return;

				count = pOwner->UnitsSelfHeal;
			break;
		}
		default:
			break;
		}

		if (cap.isset() && count >= cap) {
			count = cap;
		}
/*
	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->Defeated ||
			(pHouse->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive))
			continue;

		//TODO : causing desync , disable it
		//if (allowPlayerControl && !pHouse->ControlledByCurrentPlayer())
		//	continue;

		if (pHouse != pOwner && (allowAllies && !pHouse->IsAlliedWith(pOwner)))
			continue;

		switch (type)
		{
		case SelfHealGainType::Infantry:
		{
			if (!pOwner->InfantrySelfHeal)
				continue;

			count += pHouse->InfantrySelfHeal;
			break;
		}
		case SelfHealGainType::Units:
		{
			if (!pOwner->UnitsSelfHeal)
				continue;

			count += pHouse->UnitsSelfHeal;
			break;
		}
		default:
			break;
		}

		if (cap.isset() && count >= cap)
		{
			count = cap;
			break;//dont need to loop further , end it there
		}
	}
*/
}

constexpr int countSelfHealing(TechnoClass* pThis, const bool infantryHeal)
{
	auto const pOwner = pThis->Owner;

	int count = infantryHeal ? pOwner->InfantrySelfHeal : pOwner->UnitsSelfHeal;

	const bool hasCap = infantryHeal ? RulesExtData::Instance()->InfantryGainSelfHealCap.isset() : RulesExtData::Instance()->UnitsGainSelfHealCap.isset();
	const int cap = infantryHeal ? RulesExtData::Instance()->InfantryGainSelfHealCap.Get() : RulesExtData::Instance()->UnitsGainSelfHealCap.Get();

	if (hasCap && count >= cap)
	{
		count = cap;
		return count;
	}

	const bool allowPlayerControl = RulesExtData::Instance()->GainSelfHealFromPlayerControl && SessionClass::IsCampaign()&& (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
	const bool allowAlliesInCampaign = RulesExtData::Instance()->GainSelfHealFromAllies && SessionClass::IsCampaign();
	const bool allowAlliesDefault = RulesExtData::Instance()->GainSelfHealFromAllies && !SessionClass::IsCampaign();

	if (allowPlayerControl || allowAlliesInCampaign || allowAlliesDefault)
	{
		for (auto pHouse : *HouseClass::Array)
		{
			if (pHouse != pOwner && !pHouse->Type->MultiplayPassive)
			{
				const bool isHuman = pHouse->IsControlledByHuman();

				if ((allowPlayerControl && isHuman)
					|| (allowAlliesInCampaign && !isHuman && pHouse->IsAlliedWith(pOwner))
					|| (allowAlliesDefault && pHouse->IsAlliedWith(pOwner)))
				{
					count += infantryHeal ? pHouse->InfantrySelfHeal : pHouse->UnitsSelfHeal;

					if (hasCap && count >= cap)
					{
						count = cap;
						return count;
					}
				}
			}
		}
	}

	return count;
}

constexpr bool CanDoSelfHeal(TechnoClass* pThis , SelfHealGainType type , int& amount)
{
	switch (type)
	{
	case SelfHealGainType::Infantry:
	{
		if (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames)
			return false;

		amount = RulesClass::Instance->SelfHealInfantryAmount * countSelfHealing(pThis , true);

		if (amount <= 0)
			return false;

		return true;
	}
	case SelfHealGainType::Units:
	{
		if (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames)
			return false;

		amount = RulesClass::Instance->SelfHealUnitAmount * countSelfHealing(pThis, false);

		if (amount <= 0)
			return false;

		return true;
	}

	default:
		break;
	}

	return false;
}

constexpr SelfHealGainType GetSelfHealGainType(AbstractType what, bool organic , Nullable<SelfHealGainType>& type)
{
	if(!type.isset()){
		const bool isBuilding = what == AbstractType::Building;
		const bool isOrganic = what == AbstractType::Infantry || (what == AbstractType::Unit && organic);
		return isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
	}

	return type.Get();
}


void TechnoExtData::ApplyGainedSelfHeal(TechnoClass* pThis , bool wasDamaged)
{
	TechnoTypeClass* pType = GET_TECHNOTYPE(pThis);

	if (pThis->Health)
	{
		const auto pWhat = pThis->WhatAmI();
		const bool isBuilding = pWhat == AbstractType::Building;
		const int healthDeficit = pType->Strength - pThis->Health;

		if(healthDeficit > 0) {

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
			const SelfHealGainType selfHealType = GetSelfHealGainType(pWhat, pType->Organic, pTypeExt->SelfHealGainType);
				int amount = 0;

			if (CanDoSelfHeal(pThis , selfHealType, amount)) {

				if (amount >= healthDeficit)
					amount = healthDeficit;

				if (bool(Phobos::Debug_DisplayDamageNumbers > DrawDamageMode::disabled) && Phobos::Debug_DisplayDamageNumbers < DrawDamageMode::count )
					FlyingStrings::Instance.AddNumberString(amount, pThis->Owner, AffectedHouse::All, Drawing::DefaultColors[(int)DefaultColorList::White], pThis->Location, Point2D::Empty, false, L"");

				pThis->Health += amount;
			}
		}

		if ((wasDamaged || pThis->Sys.Damage) && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
			|| pThis->GetHeight() < -10))
		{
			bool Rubbled = false;

			if (isBuilding) {

				const auto pBuilding = static_cast<BuildingClass*>(pThis);
				//const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

				// restore rubble after reaching green health ?
				// not sure if this good implementation
				// maybe better to put it somewhere on update function ,..
				//if(pThis->GetHealthPercentage() >= RulesClass::Instance->ConditionGreen
				//	&& (pExt->RubbleIntact || pExt->RubbleIntactRemove))
				//{
				//	auto pRubble = TechnoExtData::CreateBuilding(pBuilding,
				//				pExt->RubbleIntactRemove,
				//				pExt->RubbleIntact,
				//				pExt->RubbleIntactOwner,
				//				pExt->RubbleIntactStrength,
				//				pExt->RubbleIntactAnim
				//				);
				//
				//	if (pRubble)
				//	{
				//		const bool wasSelected = pBuilding->IsSelected;
				//		Rubbled = true;
				//
				//		if(pExt->TunnelType != -1 || pBuilding->Absorber() || pBuilding->Type->CanBeOccupied)
				//		{
				//			CellStruct Cell = pThis->GetMapCoords();
				//			if(auto pData = HouseExtData::GetTunnelVector(pBuilding->Type , pBuilding->Owner)) {
				//				if (!pData->Vector.empty() && !TunnelFuncs::FindSameTunnel(pBuilding))
				//				{
				//					int nOffset = 0;
				//					auto nPos = pData->Vector.end();
				//
				//					while (std::begin(pData->Vector) != std::end(pData->Vector))
				//					{
				//						nOffset++;
				//						const auto Offs = CellSpread::CellOfssets[nOffset % CellSpread::CellOfssets.size()];
				//						auto const& [status, pPtr] = TunnelFuncs::UnlimboOne(&pData->Vector, pBuilding, (Cell.X + Offs.X) | ((Cell.Y + Offs.Y) << 16));
				//						if (!status)
				//						{
				//							TunnelFuncs::KillFootClass(pPtr, nullptr);
				//						}
				//					}
				//
				//					pData->Vector.clear();
				//				}
				//
				//			}else
				//			if(pBuilding->Absorber())
				//			{
				//				for(auto pFirst = pBuilding->Passengers.GetFirstPassenger();
				//						 pFirst;
				//						pFirst = generic_cast<FootClass*>(pFirst->NextObject)
				//				){
				//					pRubble->KickOutUnit(pFirst, Cell);
				//				}
				//			}
				//			else
				//			if(pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count != 0) {
				//				pBuilding->KickAllOccupants(false , false);
				//				if ( pBuilding->Owner->IsControlledByCurrentPlayer() )
				//				{
				//					auto idx = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
				//					if(RadarEventClass::Create(RadarEventType::GarrisonAbandoned , Cell))
				//						VoxClass::Play(GameStrings::EVA_StructureAbandoned());
				//				}
				//			}
				//		}
				//
				//		if (wasSelected) {
				//			pRubble->Select();
				//		}
				//
				//		TechnoExtData::HandleRemove(pBuilding, nullptr, false, false);
				//	}
				//}
				//else
				{
					pBuilding->Mark(MarkType::Change);
					pBuilding->ToggleDamagedAnims(false);
				}

			}

			if(!Rubbled) {
				if (auto& dmgParticle = pThis->Sys.Damage) {
					dmgParticle->UnInit();
				}
			}
		}
	}

	return;
}

void TechnoExtData::ApplyDrainMoney(TechnoClass* pThis)
{
	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = GET_TECHNOTYPEEXT(pSource);
	const auto pRules = RulesClass::Instance();
	const auto nDrainDelay = pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay);

	if ((Unsorted::CurrentFrame % nDrainDelay) == 0)
	{
		auto nDrainAmount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount);
		if (nDrainAmount != 0)
		{
			if (!pThis->Owner->CanTransactMoney(nDrainAmount)
				|| !pSource->Owner->CanTransactMoney(nDrainAmount))
				return;

			pThis->Owner->TransactMoney(-nDrainAmount);
			pSource->Owner->TransactMoney(nDrainAmount);

			if (pTypeExt->DrainMoney_Display.Get(RulesExtData::Instance()->DrainMoneyDisplay)) {
				const auto displayTo = pTypeExt->DrainMoney_Display_Houses.Get(RulesExtData::Instance()->DrainMoneyDisplay_Houses);
				FlyingStrings::Instance.AddMoneyString(true, nDrainAmount, pSource, displayTo, pSource->Location, pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
			}

			if (pTypeExt->DrainMoney_Display_OnTarget.Get(RulesExtData::Instance()->DrainMoneyDisplay_OnTarget) && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer)) {
				if (!pTypeExt->DrainMoney_Display_OnTarget_UseDisplayIncome.Get(RulesExtData::Instance()->DrainMoneyDisplay_OnTarget_UseDisplayIncome)) {
					const auto displayTo = pTypeExt->DrainMoney_Display_Houses.Get(RulesExtData::Instance()->DrainMoneyDisplay_Houses);
					// use firer for owner check
					FlyingStrings::Instance.AddMoneyString(false, -nDrainAmount, pThis, displayTo, pThis->GetRenderCoords(), pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
				} else if (const auto pBld = cast_to<BuildingClass*, false>(pThis)) {
					const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
					const auto displayTo = pBldTypeExt->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses);
					// use target for owner check
					FlyingStrings::Instance.AddMoneyString(false, -nDrainAmount, pThis, displayTo, pThis->GetRenderCoords(), pBldTypeExt->DisplayIncome_Offset, ColorStruct::Empty);
				}
			}
		}
	}
}

constexpr int GetFrames(SelfHealGainType type , HouseClass* Owner){
	switch (type)
	{
	case SelfHealGainType::Infantry:
		if(Owner->InfantrySelfHeal > 0){
			return RulesClass::Instance->SelfHealInfantryFrames;
		}
		break;
	case SelfHealGainType::Units:
		if(Owner->UnitsSelfHeal > 0){
			return RulesClass::Instance->SelfHealUnitFrames;
		}
		break;
	default:
		break;
	}

	return -1;
}

bool hasSelfHeal(TechnoClass* pThis , const bool infantryHeal)
{
	auto const pOwner = pThis->Owner;

	if (infantryHeal ? pOwner->InfantrySelfHeal > 0 : pOwner->UnitsSelfHeal > 0)
		return true;

	const bool allowPlayerControl = RulesExtData::Instance()->GainSelfHealFromPlayerControl && SessionClass::IsCampaign() && (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
	const bool allowAlliesInCampaign = RulesExtData::Instance()->GainSelfHealFromAllies && SessionClass::IsCampaign();
		const bool allowAlliesDefault = RulesExtData::Instance()->GainSelfHealFromAllies && !SessionClass::IsCampaign();

	if (allowPlayerControl || allowAlliesInCampaign || allowAlliesDefault) {
		for (auto pHouse : *HouseClass::Array) {
			if (pHouse != pOwner && !pHouse->Type->MultiplayPassive) {
				const bool isHuman = pHouse->IsControlledByHuman();

				if ((allowPlayerControl && isHuman)
					|| (allowAlliesInCampaign && !isHuman && pHouse->IsAlliedWith(pOwner))
					|| (allowAlliesDefault && pHouse->IsAlliedWith(pOwner)))
				{
					if (infantryHeal ? pHouse->InfantrySelfHeal > 0 : pHouse->UnitsSelfHeal > 0)
						return true;
				}
			}
		}
	}

	return false;
}

void TechnoExtData::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds, SHPStruct* shape, ConvertClass* convert)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->NoExtraSelfHealOrRepair)
		return;

	if (pThis->Owner->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive)
		return;

	auto const pWhat = pThis->WhatAmI();
	const bool isOrganic = pWhat == InfantryClass::AbsID
		|| (pType->Organic && (pWhat == UnitClass::AbsID));

	auto const selfHealType = GetSelfHealGainType(pWhat , isOrganic, pExt->SelfHealGainType) ;

	if (selfHealType == SelfHealGainType::None || !hasSelfHeal(pThis, isOrganic))
		return;

	int selfHealFrames = GetFrames(selfHealType, pThis->Owner);

	if(selfHealFrames <= 0 )
		return;

	{
		Point2D pipFrames { 0,0 };
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < GET_TECHNOTYPE(pThis)->Strength)
		{
			isSelfHealFrame = true;
		}

		int nBracket = TechnoExtData::GetDisguiseType(pThis, false, true).first->PixelSelectionBracketDelta;

		switch (pWhat)
		{
		case UnitClass::AbsID:
		case AircraftClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Units.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Infantry.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case BuildingClass::AbsID:
		{
			const auto pBldType = static_cast<BuildingTypeClass*>(pType);
			int fHeight = pBldType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Buildings.Get();
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pBldType->Height * yAdjust;
		}
		break;
		default:
			break;
		}

		int pipFrame = selfHealType == SelfHealGainType::Infantry ? pipFrames.X : pipFrames.Y;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(convert, shape,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExtData::TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret, double factor, int turIdx)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	const bool isFoot = (pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;

	// turret offset and rotation
	if (isOnTurret && (pType->Turret || !isFoot)) {
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, factor, turIdx);
		float angle {};

		const double turretRad = pThis->TurretFacing().GetFacing<32>();

		if (isFoot) {
			angle = (float)(turretRad - pThis->PrimaryFacing.Current().GetFacing<32>());
		}else {
			angle = float(turretRad);
		}

		mtx.RotateZ(angle);
	}
}

Matrix3D TechnoExtData::GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret, double factor, bool isShadow, int turIdx)
{
	Matrix3D nMTX = TechnoExtData::GetTransform(pThis, nullptr, isOnTurret);
	TechnoExtData::TransformFLHForTurret(pThis, nMTX, isOnTurret, factor, turIdx);

	// apply FLH offset
	nMTX.Translate((float)nCoord.X, (float)nCoord.Y, (float)nCoord.Z);



	return nMTX;
}

void TechnoExtData::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	const auto pType = GET_TECHNOTYPE(pThis);

	if (!pType->OpenTopped || !pThis->Passengers.NumPassengers)
		return;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pExt->Ammo_Shared || !pType->Ammo)
		return;

	auto passenger = pThis->Passengers.FirstPassenger;

	do
	{
		TechnoTypeClass* passengerType = GET_TECHNOTYPE(passenger);

		if (TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared.Get())
		{
			if (pExt->Ammo_Shared_Group.Get() < 0 ||
				pExt->Ammo_Shared_Group.Get() == TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared_Group.Get())
			{
				if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
				{
					pThis->Ammo--;
					passenger->Ammo++;
				}
			}
		}

		passenger = static_cast<FootClass*>(passenger->NextObject);

	}
	while (passenger);
}

double TechnoExtData::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	const double houseMultiplier = pThis->Owner->GetSpeedMult(GET_TECHNOTYPE(pThis));
	double speed = pThis->SpeedMultiplier * houseMultiplier;

	if(pThis->HasAbility(AbilityType::Faster))
		speed *=  RulesClass::Instance->VeteranSpeed;

	return speed;
}

void TechnoExtData::UpdateMindControlAnim()
{
	const auto pThis = This();

	if (pThis->IsMindControlled())
	{
		if (pThis->MindControlRingAnim && !MindControlRingAnimType)
		{
			MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && MindControlRingAnimType &&
			pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;
			const auto pBuilding = cast_to<BuildingClass*, false>(pThis);

			if (pBuilding)
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = GET_TECHNOTYPE(pThis)->MindControlRingOffset;

			coords.Z += offset;

			{
				auto anim = GameCreate<AnimClass>(MindControlRingAnimType, coords, 0, 1);
				pThis->MindControlRingAnim = anim;
				pThis->MindControlRingAnim->SetOwnerObject(pThis);

				if (pBuilding)
					pThis->MindControlRingAnim->ZAdjust = -1024;
			}
		}
	}
	else if (MindControlRingAnimType)
	{
		MindControlRingAnimType = nullptr;
	}
}

std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> TechnoExtData::GetFireSelfData()
{
	const auto pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty() && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_RedHeath , &pTypeExt->FireSelf_ROF_RedHeath };
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_YellowHeath , &pTypeExt->FireSelf_ROF_YellowHeath };
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_GreenHeath , &pTypeExt->FireSelf_ROF_GreenHeath };
	}

	return  { &pTypeExt->FireSelf_Weapon , &pTypeExt->FireSelf_ROF };

}

void TechnoExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (auto pShieldData = this->GetShield())
			pShieldData->SetAnimationVisibility(false);

		for (auto& pos : this->LaserTrails)
		{
			pos->Visible = false;
			pos->LastLocation.clear();
		}

		//TrailsManager::Hide(This());

		this->IsInTunnel = true;
	}

	const auto pType = GET_TECHNOTYPE(This());
	const auto pImage = pType->AlphaImage;

	if (pImage)
	{
		auto& alphaExt = PhobosGlobal::Instance()->ObjectLinkedAlphas;

		if (const auto pAlpha = alphaExt.get_or_default(This()))
		{
			GameDelete<true,false>(pAlpha);

			const auto tacticalPos = TacticalClass::Instance->TacticalPos;
			Point2D off = { tacticalPos.X - (pImage->Width / 2), tacticalPos.Y - (pImage->Height / 2) };
			const auto point = TacticalClass::Instance->CoordsToClient(This()->GetCoords()) + off;
			RectangleStruct dirty = { point.X - tacticalPos.X, point.Y - tacticalPos.Y, pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(dirty, true);
		}
	}
}

std::pair<WeaponTypeClass*, int> TechnoExtData::GetDeployFireWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	int weaponIndex = pType->DeployFireWeapon;

	if (pThis->WhatAmI() == UnitClass::AbsID)
	{
		if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				if (pThis->GetFireError(pThis->GetCell(), 0, true) != FireError::OK)
					weaponIndex = 1;
			}
		}
	}

	if (weaponIndex == -1)
	{
		weaponIndex = pThis->SelectWeapon(pTarget);
	}
	else if (weaponIndex < -1)
		return { nullptr   , -1 };

	if (auto const pWs = pThis->GetWeapon(weaponIndex))
	{
		return { pWs->WeaponType  , weaponIndex };
	}

	return { nullptr , -1 };
}

void TechnoExtData::UpdateType(TechnoTypeClass* currentType)
{
	static_assert(true, "Donot Use!");
	//auto const pThis = This();
	//const auto pOldType = this->Type;
	//const auto pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);
	//this->Type = currentType;
	//auto const pTypeExtData = TechnoTypeExtContainer::Instance.Find(currentType);

	//TechnoExtData::InitializeLaserTrail(pThis, true);

	// Reset Shield
	// This part should have been done by UpdateShield

	// Reset AutoDeath Timer
	//if (this->Death_Countdown.HasStarted())
	//{
	//	this->Death_Countdown.Stop();

	//	{
	//		HouseExtData::AutoDeathObjects.erase(pThis);
	//	}
	//}

	// Reset PassengerDeletion Timer - TODO : unchecked
	//if (this->PassengerDeletionTimer.IsTicking()
	//	&& !pTypeExtData->PassengerDeletionType.Enabled)
	//	this->PassengerDeletionTimer.Stop();

	//TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);

	/*if (!pTypeExtData->MyFighterData.Enable && this->MyFighterData)
		this->MyFighterData.reset(nullptr);

	else if (pTypeExtData->MyFighterData.Enable && !this->MyFighterData)
	{
		this->MyFighterData = std::make_unique<FighterAreaGuard>();
		this->MyFighterData->OwnerObject = (AircraftClass*)pThis;
	}

	if(!pTypeExtData->DamageSelfData.Enable && this->DamageSelfState)
		this->DamageSelfState.reset(nullptr);
	else if (pTypeExtData->DamageSelfData.Enable && !this->DamageSelfState)
		DamageSelfState::OnPut(this->DamageSelfState, pTypeExtData->DamageSelfData);

	if (!pTypeExtData->MyGiftBoxData.Enable && this->MyGiftBox)
		this->MyGiftBox.reset(nullptr);
	else if (pTypeExtData->MyGiftBoxData.Enable && !this->MyGiftBox)
		GiftBoxFunctional::Init(this, pTypeExtData);*/

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	//bool toOpenTopped = currentType->OpenTopped && !pOldType->OpenTopped;

	//if ((toOpenTopped || (!currentType->OpenTopped && pOldType->OpenTopped)) && pThis->Passengers.NumPassengers > 0)
	//{
	//	auto pPassenger = pThis->Passengers.FirstPassenger;

	//	while (pPassenger)
	//	{
	//		if (toOpenTopped)
	//		{
	//			pThis->EnteredOpenTopped(pPassenger);
	//		}
	//		else
	//		{
	//			pThis->ExitedOpenTopped(pPassenger);

	//			// Lose target & destination
	//			pPassenger->Guard();

	//			// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
	//			// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
	//			MapClass::Logics.get().RemoveObject(pPassenger);
	//		}

	//		pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject);
	//	}
	//}
}

void TechnoExtData::UpdateBuildingLightning()
{
	auto const pThis = This();

	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto pBldExt = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis));
	if (pBldExt->LighningNeedUpdate)
	{
		pThis->Mark(MarkType::Redraw);
		pBldExt->LighningNeedUpdate = false;

	}
}

void TechnoExtData::UpdateShield()
{
	auto const pThis = This();

	if (!this->CurrentShieldType)
		Debug::FatalErrorAndExit("Techno[%s] Missing CurrentShieldType ! ", pThis->get_ID());

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(GET_TECHNOTYPE(pThis));

	// Set current shield type if it is not set.
	if (!this->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		this->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && !PhobosEntity::Has<ShieldClass>(this->ShieldEntity))
	{
		auto& shield = PhobosEntity::Emplace<ShieldClass>(this->ShieldEntity, pThis);
		shield.UpdateTint();
	}

	if (const  auto pShieldData = this->GetShield())
		pShieldData->OnUpdate();
}

#include <Ext/Cell/Body.h>

void TechnoExtData::UpdateRevengeWeapons()
{
	this->RevengeWeapons.remove_all_if([](TimedWarheadValue<WeaponTypeClass*>& item) {
	   return item.Timer.Expired();
	});
}

void TechnoExtData::UpdateAircraftOpentopped()
{
	auto const pThis = This();

	if (!TechnoExtData::IsAlive(pThis, true, true, true))
		return;

	const auto pType = GET_TECHNOTYPE(pThis);

	if (pType->Passengers > 0 && !AircraftOpentoppedInitEd)
	{

		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (auto const pInf = flag_cast_to<FootClass*, false>(*object))
			{
				if (!pInf->Transporter || !pInf->InOpenToppedTransport)
				{
					if (pType->OpenTopped)
						pThis->EnteredOpenTopped(pInf);

					if (pType->Gunner)
						flag_cast_to<FootClass*, false>(pThis)->ReceiveGunner(pInf);

					pInf->Transporter = pThis;
					pInf->Undiscover();
				}
			}
		}

		AircraftOpentoppedInitEd = true;
	}
}

bool TechnoExtData::HasAmmoToDeploy(TechnoClass* pThis)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	const int min = pTypeExt->Ammo_DeployUnlockMinimumAmount;
	const int max = pTypeExt->Ammo_DeployUnlockMaximumAmount;

	if (min < 0 && max < 0)
		return true;

	const int ammo = pThis->Ammo;

	if ((min < 0 || ammo >= min) && (max < 0 || ammo <= max))
		return true;

	return false;
}

void TechnoExtData::HandleOnDeployAmmoChange(TechnoClass* pThis, int maxAmmoOverride)
{
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	int add = pTypeExt->Ammo_AddOnDeploy;

	if (add != 0)
	{
		int maxAmmo = pType->Ammo;

		if (maxAmmoOverride >= 0)
			maxAmmo = maxAmmoOverride;

		int originalAmmo = pThis->Ammo;
		pThis->Ammo = std::clamp(originalAmmo + add, 0, maxAmmo);

		if (originalAmmo != pThis->Ammo) {
			pThis->StartReloading();
			pThis->Mark(MarkType::Change);
		}
	}
}

bool TechnoExtData::SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes) {
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return defaultValue;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pTypeConvert = pTypeExt->Convert_Deploy;

	if (alwaysCheckLandTypes || pTypeExt->IsSimpleDeployer_ConsiderPathfinding) {
		bool isHover = pType->Locomotor == HoverLocomotionClass::ClassGUID();
		bool isJumpjet = pType->Locomotor == JumpjetLocomotionClass::ClassGUID();
		bool isLander = pType->DeployToLand && (isJumpjet || isHover);
		auto const defaultLandTypes = isLander ? (LandTypeFlags)(LandTypeFlags::Water | LandTypeFlags::Beach) : LandTypeFlags::None;
		auto const disallowedLandTypes = pTypeExt->IsSimpleDeployer_DisallowedLandTypes.Get(defaultLandTypes);

		if (IsLandTypeInFlags(disallowedLandTypes, pThis->GetCell()->LandType))
			return false;

		if (alwaysCheckLandTypes && !pTypeExt->IsSimpleDeployer_ConsiderPathfinding)
			return true;
	} else {
		return defaultValue;
	}

	const SpeedType speed = pTypeConvert ? pTypeConvert->SpeedType : pType->SpeedType;
	const MovementZone mZone = pTypeConvert ? pTypeConvert->MovementZone : pType->MovementZone;

	if (speed != SpeedType::None && mZone != MovementZone::None) {
		auto const pCell = pThis->GetCell();
		return pCell->IsClearToMove(speed, true, true, ZoneType::None, mZone, -1, pCell->ContainsBridge());
	}

	return true;
}

void TechnoExtData::DepletedAmmoActions()
{
	auto const pThis = this->This();
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->Ammo <= 0)
		return;

	auto const rtti = pThis->WhatAmI();
	UnitClass* pUnit = nullptr;

	if (rtti == AbstractType::Unit)
	{
		pUnit = static_cast<UnitClass*>(pThis);
		auto const pUnitType = pUnit->Type;

		if (!pUnitType->IsSimpleDeployer && !pUnitType->DeploysInto && !pUnitType->DeployFire
			&& pUnitType->Passengers < 1 && pUnit->Passengers.NumPassengers < 1)
		{
			return;
		}
	}

	int const min = pTypeExt->Ammo_AutoDeployMinimumAmount;
	int const max = pTypeExt->Ammo_AutoDeployMaximumAmount;

	if (min < 0 && max < 0)
		return;

	int const ammo = pThis->Ammo;
	bool canDeploy = TechnoExtData::HasAmmoToDeploy(pThis) && (min < 0 || ammo >= min) && (max < 0 || ammo <= max);
	bool isDeploying = pThis->CurrentMission == Mission::Unload || pThis->QueuedMission == Mission::Unload;

	if (canDeploy && !isDeploying)
	{
		pThis->QueueMission(Mission::Unload, true);
	}
	else if (!canDeploy && isDeploying)
	{
		pThis->QueueMission(Mission::Guard, true);

		if (pUnit && pUnit->Type->IsSimpleDeployer && pThis->InAir)
		{
			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
				pJJLoco->NextState = JumpjetLocomotionClass::State::Ascending;
		}
	}
}

void TechnoExtData::UpdateLaserTrails()
{
	auto const pThis = (FootClass*)this->This();

	if (LaserTrails.empty())
		return;

	const bool IsDroppod = VTable::Get(pThis->Locomotor.GetInterfacePtr()) == DropPodLocomotionClass::vtable;

	for (auto& trail : LaserTrails)
	{
		if (trail->Type->DroppodOnly && !IsDroppod)
			continue;

		if (pThis->CloakState == CloakState::Cloaked)
		{
			if (!trail->Type->CloakVisible)
			{
				trail->Cloaked = true;
			} else if (trail->Type->CloakVisible_Houses && !HouseClass::IsCurrentPlayerObserver() && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer)) {
				auto const pCell = pThis->GetCell();
				trail->Cloaked = !pCell || !pCell->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
			}
		}

		if (!IsInTunnel)
			trail->Visible = true;

		if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir() && trail->LastLocation.isset())
			trail->LastLocation.clear();

		CoordStruct trailLoc = TechnoExtData::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret);
		if (pThis->CloakState == CloakState::Uncloaking && !trail->Type->CloakVisible)
		trail->LastLocation = trailLoc;
		else
		trail->Update(trailLoc);
	}
}

void TechnoExtData::UpdateGattlingOverloadDamage()
{
	auto const pThis = This();

	if (!pThis->IsAlive)
		return;

	const auto pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);


	if (!pType->IsGattling || !pTypeExt->Gattling_Overload.Get())
		return;

	auto const nDelay = Math::abs(pTypeExt->Gattling_Overload_Frames.Get(0));

	if (!nDelay)
		return;

	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue)
		{
			GattlingDmageDelay = -1;
			GattlingDmageSound = false;
			return;
		}

		GattlingDmageDelay = nDelay;
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			GattlingDmageSound = false;
		}
		else
		{
			auto const pWarhead = pTypeExt->Gattling_Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pThis->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.isset())
					VocClass::SafeImmedietelyPlayAt(pTypeExt->Gattling_Overload_DeathSound, &pThis->Location, 0);

				GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get(nullptr))
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(1); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random(-200, 200);
					auto nLoc = pThis->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->Owner);
				}
			}

			if (pThis->WhatAmI() == UnitClass::AbsID && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioClass::Instance->Random.RandomBool() ? Math::flt_2 : Math::flt_1;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--GattlingDmageDelay;
	}
}

bool TechnoExtData::UpdateKillSelf_Slave()
{
	auto const pThis = This();

	if (VTable::Get(pThis) != InfantryClass::vtable)
		return false;

	const auto pInf = static_cast<InfantryClass*>(pThis);

	if (!pInf->Type->Slaved)
		return false;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pInf->Type);

	if (!pInf->SlaveOwner && (pTypeExt->Death_WithMaster.Get()
		|| pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide))
	{

		const KillMethod nMethod = pTypeExt->Death_Method.Get();

		if (nMethod != KillMethod::None)
			TechnoExtData::KillSelf(pInf, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	}

	return !pThis->IsAlive;
}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExtData::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno,
 AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (auto const pCell = cast_to<CellClass*>(pTarget))
			pTargetCell = pCell;
		else if (auto const pObject = flag_cast_to<ObjectClass*>(pTarget)) {
			if (!pObject->IsAlive)
				pTargetCell = MapClass::Instance->TryGetCellAt(pObject->Location);
			else
				pTargetCell = pObject->GetCell();
		}
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	auto const pWeaponOne = pWeaponStructOne->WeaponType;
	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (auto const pSecondExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponTwo))
	{
		if(!pSecondExt->SkipWeaponPicking){
			if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true, true))
				return weaponIndexOne;

			if (
				(pTargetTechno &&
					(
						!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget, false) ||
						!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
						!TechnoExtData::ObjectHealthAllowFiring(pTargetTechno, pWeaponTwo) ||
						!pSecondExt->IsVeterancyInThreshold(pTargetTechno) ||
						!pSecondExt->HasRequiredAttachedEffects(pTargetTechno, pThis)
					)))
			{
				return weaponIndexOne;
			}
		}

		if (auto const pFirstExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponOne))
		{
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
			const bool firstAllowedAE = pFirstExt->HasRequiredAttachedEffects(pTargetTechno, pThis);

			if (!allowFallback
					&& (!allowAAFallback || !secondaryIsAA)
					&& !TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)
					&& firstAllowedAE)
				return weaponIndexOne;

			if(!pFirstExt->SkipWeaponPicking){
				if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true, true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget, false) ||
						!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
						!TechnoExtData::ObjectHealthAllowFiring(pTargetTechno, pWeaponOne) ||
						!pFirstExt->IsVeterancyInThreshold(pTargetTechno) ||
						!firstAllowedAE
						)
					))
					{
					return weaponIndexTwo;
				}
			}
		}
	}

	auto const pType = GET_TECHNOTYPE(pThis);

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && pTargetCell && (pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary)
		&& pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
	{
		return weaponIndexTwo;
	}

	return -1;
}

bool TechnoExtData::IsInWarfactory(TechnoClass* pThis, bool bCheckNaval)
{
	if (pThis->WhatAmI() != UnitClass::AbsID || pThis->IsInAir() || !pThis->IsTethered)
		return false;

	auto const pContact = pThis->GetNthLink();

	if (!pContact)
		return false;

	auto const pCell = pThis->GetCell();

	if (!pCell)
		return false;

	auto const pBld = pCell->GetBuilding();

	if (!pBld || pBld != pContact)
		return false;

	if (pBld->Type->WeaponsFactory || (bCheckNaval && pBld->Type->Naval))
	{
		return true;
	}

	return false;
}

CoordStruct TechnoExtData::GetPutLocation(CoordStruct current, int distance)
{
	// this whole thing does not at all account for cells which are completely occupied.
	const CellStruct tmpCoords = CellSpread::AdjacentCell[ScenarioClass::Instance->Random.RandomFromMax(7)];

	current.X += tmpCoords.X * distance;
	current.Y += tmpCoords.Y * distance;

	const auto tmpCell = MapClass::Instance->TryGetCellAt(current);

	if (!tmpCell)
	{
		return CoordStruct::Empty;
	}

	return tmpCell->FindInfantrySubposition(current, false, false, false, current.Z);
}

bool TechnoExtData::EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select, std::optional<bool> InAir)
{
	const CellClass* pCell = MapClass::Instance->GetCellAt(loc);
	//auto pType = Survivor->GetTechnoType();

	if (const auto pBld = pCell->GetBuilding())
	{
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

		if (!pTypeExt->IsPassable)
			return false;

		if (pTypeExt->Firestorm_Wall &&
			pBld->Owner &&
			pBld->Owner->FirestormActive)
			return false;
	}

	Survivor->OnBridge = pCell->ContainsBridge();
	const int floorZ = pCell->GetCoordsWithBridge().Z;

	bool Chuted = false;
	if(!InAir.has_value()){
		Chuted = (loc.Z - floorZ > 2 * Unsorted::LevelHeight);
	}
	else { Chuted = InAir.value(); }

	if (Chuted)
	{
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Limbo();

		if (pCell->GetBuilding())
			return false;

		++Unsorted::ScenarioInit;
		if (!Survivor->SpawnParachuted(loc))
		{
			--Unsorted::ScenarioInit;
			return false;
		}
	}
	else
	{
		loc.Z = floorZ;
		//if (!MapClass::Instance->GetCellAt(loc)->IsClearToMove(pType->SpeedType, pType->MovementZone))
		//	return false;
		++Unsorted::ScenarioInit;
		if (!Survivor->Unlimbo(loc, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::North, DirType::NorthWest))) {
			--Unsorted::ScenarioInit;
			return false;
		}
	}

	--Unsorted::ScenarioInit;

	Survivor->Transporter = nullptr;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if (Chuted)
	{
		const bool scat = Survivor->OnBridge;
		const auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;

		if ((occupation & 0x1C) == 0x1C)
		{
			pCell->ScatterContent(CoordStruct::Empty, true, true, scat);
		}
	}
	else
	{
		Survivor->Scatter(CoordStruct::Empty, true, false);
		Survivor->QueueMission(!Survivor->Owner->IsControlledByHuman() ? Mission::Hunt : Mission::Guard, 0);
	}

	Survivor->ShouldEnterOccupiable = false;
	Survivor->ShouldGarrisonStructure = false;

	if (Select)
	{
		Survivor->Select();
	}

	return true;
}

void TechnoExtData::EjectPassengers(FootClass* pThis, int howMany)
{
	if (howMany < 0)
	{
		howMany = pThis->Passengers.NumPassengers;
	}

	auto const openTopped = GET_TECHNOTYPE(pThis)->OpenTopped;

	for (int i = 0; i < howMany && pThis->Passengers.FirstPassenger; ++i)
	{
		FootClass* passenger = pThis->RemoveFirstPassenger();
		if (!EjectRandomly(passenger, pThis->Location, 128, false))
		{
			passenger->RegisterDestruction(nullptr);
			passenger->UnInit();
		}
		else if (openTopped)
		{
			pThis->ExitedOpenTopped(passenger);
		}
	}
}

bool TechnoExtData::EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select, std::optional<bool> InAir)
{
	CoordStruct destLoc = GetPutLocation(location, distance);

	if (destLoc == CoordStruct::Empty || !MapClass::Instance->IsWithinUsableArea(destLoc)) {

		std::vector<CoordStruct> usableCoords;

		for (int direction = 0; direction < 8; ++direction)
		{
			const CellStruct tmpCoords = CellSpread::AdjacentCell[direction];
			CoordStruct ejectCoords { location.X + tmpCoords.X * distance, location.Y + tmpCoords.Y * distance, location.Z };
			const auto pCell = MapClass::Instance->TryGetCellAt(ejectCoords);

			if (!pCell)
				continue;

			const auto occupied = pEjectee->IsCellOccupied(pCell, FacingType::None, -1, nullptr, true);

			if (occupied != Move::OK && occupied != Move::MovingBlock)
				continue;

			if (pEjectee->WhatAmI() == InfantryClass::AbsID) {
				ejectCoords = pCell->FindInfantrySubposition(ejectCoords, false, false, false, location.Z);
			} else {
				ejectCoords = CellClass::Cell2Coord(pCell->MapCoords, location.Z);
			}

			if(!MapClass::Instance->IsWithinUsableArea(ejectCoords))
				continue;

			usableCoords.emplace_back(ejectCoords);
		}

		const int count = static_cast<int>(usableCoords.size());

		if (!count)
			return false;

		destLoc = usableCoords[ScenarioClass::Instance->Random(0, count - 1)];
	}

	return EjectSurvivor(pEjectee, destLoc, select , InAir);
}

FORCEINLINE void TechnoExtData::ReplaceArmor(Armor& armor, ObjectClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoExtData::ReplaceArmor(armor, pTarget, pWeapon->Warhead);
}

FORCEINLINE void TechnoExtData::ReplaceArmor(Armor& armor, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoExtData::ReplaceArmor(armor, pTarget, pWeapon->Warhead);
}

FORCEINLINE void TechnoExtData::ReplaceArmor(Armor& armor, ObjectClass* pTarget, WarheadTypeClass* pWH)
{
	if(pTarget->AbstractFlags & AbstractFlags::Techno) {
		TechnoExtData::ReplaceArmor(armor , (TechnoClass*)pTarget, pWH);
	}
}

FORCEINLINE void TechnoExtData::ReplaceArmor(Armor& armor, TechnoClass* pTarget, WarheadTypeClass* pWH)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTarget);

	auto pShieldData = pExt->GetShield();

	if(pShieldData && pShieldData->IsActive() && !pShieldData->CanBePenetrated(pWH)){
		armor = pShieldData->GetArmor(armor);
	}
}

int TechnoExtData::GetInitialStrength(TechnoTypeClass* pType, int nHP)
{
	return TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(nHP);
}

bool TechnoExtData::IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger)
{
	auto pThisType = GET_TECHNOTYPE(pThis);
	auto const pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);
	auto pThatType = GET_TECHNOTYPE(pPassanger);

	if (pThatType->Size > pThisType->SizeLimit)
		return false;

	if (pThisTypeExt->Passengers_BySize.Get())
	{
		if (pThatType->Size > (pThisType->Passengers - pThis->Passengers.GetTotalSize()))
			return false;
	}
	else if (pThis->Passengers.NumPassengers >= pThisType->Passengers)
	{
		return false;
	}

	return true;
}

bool TechnoExtData::IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource)
{
	if (!pThis || !pSource)
		return false;

	auto const pType = GET_TECHNOTYPE(pThis);

	if (!pType->TypeImmune)
		return false;

	return pType == GET_TECHNOTYPE(pSource) && pThis->Owner == pSource->Owner;
}

#include <Ext/Unit/Body.h>

void TechnoExtData::InitializeUnitIdleAction(TechnoClass* pThis, TechnoTypeClass* pType)
{
	if (pThis->WhatAmI() != AbstractType::Unit || !pThis->HasTurret())
		return;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = UnitExtContainer::Instance.Find((UnitClass*)pThis);

	if (pTypeExt->AutoFire || pType->TurretSpins)
		return;

	if (pTypeExt->UnitIdleRotateTurret.Get(RulesExtData::Instance()->UnitIdleRotateTurret))
		pExt->UnitIdleAction = true;

	if (!SessionClass::IsSingleplayer())
		return;

	if (pTypeExt->UnitIdlePointToMouse.Get(RulesExtData::Instance()->UnitIdlePointToMouse))
		pExt->UnitIdleActionSelected = true;
}

void TechnoExtData::StopIdleAction()
{
	if (!this->UnitIdleAction)
		return;

	if (this->UnitIdleActionTimer.IsTicking())
		this->UnitIdleActionTimer.Stop();

	if (this->UnitIdleActionGapTimer.IsTicking())
	{
		this->UnitIdleActionGapTimer.Stop();
		auto const pTypeExt = GET_TECHNOTYPEEXT(This());
		this->StopRotateWithNewROT(pTypeExt->TurretRot.Get(pTypeExt->This()->ROT));
	}
}

void TechnoExtData::ApplyIdleAction()
{
	TechnoClass* const pThis = This();
	FacingClass* const turret = &pThis->SecondaryFacing;

	if (this->UnitIdleActionTimer.Completed()) // Set first direction
	{
		this->UnitIdleActionTimer.Stop();
		this->UnitIdleActionGapTimer.Start(ScenarioClass::Instance->Random.
			RandomRanged(RulesExtData::Instance()->UnitIdleActionIntervalMin, RulesExtData::Instance()->UnitIdleActionIntervalMax));
		bool noNeedTurnForward = false;

		if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
			noNeedTurnForward = pUnit->BunkerLinkedItem ||!pUnit->Type->Speed || (pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
		else if (pThis->WhatAmI() == AbstractType::Building)
			noNeedTurnForward = true;

		const double extraRadian = ScenarioClass::Instance->Random.RandomDouble() - 0.5;

		DirStruct unitIdleFacingDirection;
		unitIdleFacingDirection.SetRadian<32>(pThis->PrimaryFacing.Current().GetRadian<32>() + (noNeedTurnForward ? extraRadian * Math::GAME_TWOPI : extraRadian));

		this->StopRotateWithNewROT(ScenarioClass::Instance->Random.RandomRanged(2, 4) >> 1);
		turret->Set_Desired(unitIdleFacingDirection);
	}
	else if (this->UnitIdleActionGapTimer.IsTicking()) // Check change direction
	{
		if (!this->UnitIdleActionGapTimer.HasTimeLeft()) // Set next direction
		{
			this->UnitIdleActionGapTimer.Start(ScenarioClass::Instance->Random.
				RandomRanged(RulesExtData::Instance()->UnitIdleActionIntervalMin, RulesExtData::Instance()->UnitIdleActionIntervalMax));
			bool noNeedTurnForward = false;

			if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
				noNeedTurnForward = pUnit->BunkerLinkedItem ||!pUnit->Type->Speed || (pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
			else if (pThis->WhatAmI() == AbstractType::Building)
				noNeedTurnForward = true;

			const double extraRadian = ScenarioClass::Instance->Random.RandomDouble() - 0.5;

			DirStruct unitIdleFacingDirection;
			unitIdleFacingDirection.SetRadian<32>(pThis->PrimaryFacing.Current().GetRadian<32>() + (noNeedTurnForward ? extraRadian * Math::GAME_TWOPI : extraRadian));

			this->StopRotateWithNewROT(ScenarioClass::Instance->Random.RandomRanged(2, 4) >> 1);
			turret->Set_Desired(unitIdleFacingDirection);
		}
	}
	else if (!this->UnitIdleActionTimer.IsTicking()) // In idle now
	{
		this->UnitIdleActionTimer.Start(ScenarioClass::Instance->Random.
			RandomRanged(RulesExtData::Instance()->UnitIdleActionRestartMin, RulesExtData::Instance()->UnitIdleActionRestartMax));
		bool noNeedTurnForward = false;

		if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
			noNeedTurnForward = pUnit->BunkerLinkedItem || !pUnit->Type->Speed ||(pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
		else if (pThis->WhatAmI() == AbstractType::Building)
			noNeedTurnForward = true;

		if (!noNeedTurnForward)
			turret->Set_Desired(pThis->PrimaryFacing.Current());
	}
}

void TechnoExtData::ManualIdleAction()
{
	TechnoClass* const pThis = This();
	FacingClass* const turret = &pThis->SecondaryFacing;

	if (pThis->IsSelected)
	{
		this->StopIdleAction();
		this->UnitIdleIsSelected = true;
		const CoordStruct mouseCoords = TacticalClass::Instance->ClientToCoords(WWMouseClass::Instance->XY1);

		if (mouseCoords != CoordStruct::Empty) // Mouse in tactical
		{
			CoordStruct technoCoords = This()->GetCoords();
			const int offset = -static_cast<int>(technoCoords.Z * ((Unsorted::LeptonsPerCell / 2.0) / Unsorted::LevelHeight));
			const double nowRadian = Math::atan2(double(technoCoords.Y + offset - mouseCoords.Y), double(mouseCoords.X - technoCoords.X - offset)) - 0.125;
			DirStruct unitIdleFacingDirection;
			unitIdleFacingDirection.SetRadian<32>(nowRadian);
			turret->Set_Desired(unitIdleFacingDirection);
		}
	}
	else if (this->UnitIdleIsSelected) // Immediately stop when is not selected
	{
		this->UnitIdleIsSelected = false;
		this->StopRotateWithNewROT();
	}
}

void TechnoExtData::StopRotateWithNewROT(int ROT)
{
	FacingClass* const turret = &This()->SecondaryFacing;

	const DirStruct currentFacingDirection = turret->Current();
	turret->DesiredFacing = currentFacingDirection;
	turret->StartFacing = currentFacingDirection;
	turret->RotationTimer.Start(0);

	if (ROT >= 0)
		turret->Set_ROT(ROT);
}

// =============================
// load / save

void TechnoExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type)
{
	this->RadioExtData::InvalidatePointer(ptr, bRemoved, type);

	switch (type)
	{
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::Building:
	case AbstractType::Infantry:
	{
		if (ptr && bRemoved)
		{
			auto& AttackerDatas = this->OnlyAttackData;
			if (!AttackerDatas.empty())
			{
				for (int index = int(AttackerDatas.size()) - 1; index >= 0; --index)
				{
					if (AttackerDatas[index].Attacker != ptr)
						continue;

					AttackerDatas.erase(AttackerDatas.begin() + index);
				}
			}
		}
		break;
	}
	case AbstractType::Airstrike:
		AnnounceInvalidPointer(AirstrikeTargetingMe, ptr);
		break;
	case AbstractType::BuildingLight:
		AnnounceInvalidPointer(BuildingLight, ptr);
		break;
	case AbstractType::House:

		AnnounceInvalidPointer(OriginalPassengerOwner, ptr);
		break;
	case AbstractType::Super:
		AnnounceInvalidPointer(LinkedSW, ptr);

		break;
	default:break;
	}

	if (auto pSpawn = (FakeSpawnManagerClass*)This()->SpawnManager)
		pSpawn->_DetachB(ptr, bRemoved);

	AnnounceInvalidPointer(WebbyLastTarget, ptr);

	for (auto& _phobos_AE : PhobosAE) {
		if (_phobos_AE) {
			_phobos_AE->InvalidatePointer(ptr, bRemoved, type);
		}
	}
}

TechnoExtContainer TechnoExtContainer::Instance;

TechnoExtData::~TechnoExtData()
{
	auto pThis = This();
	FakeHouseClass* pOwner = (FakeHouseClass*)pThis->Owner;
	auto pOwnerExt = pOwner->_GetExtData();

	ScenarioExtData::Instance()->LimboLaunchers.erase(pThis);

	if (this->UndergroundTracked)
		ScenarioExtData::Instance()->UndergroundTracker.erase(pThis);

	if(this->FallingDownTracked)
		ScenarioExtData::Instance()->FallingDownTracker.erase(pThis);

	if (!Phobos::Otamaa::ExeTerminated)
	{
		if (auto pTemp = std::exchange(this->MyOriginalTemporal, nullptr))
		{
			GameDelete<true, false>(pTemp);
		}
	}

	this->ClearElectricBolts();

	HouseExtContainer::Instance.AutoDeathObjects.erase_all_if([pThis](std::pair<TechnoClass*, KillMethod>& item) {
		return item.first == pThis;
	});

	HouseExtContainer::Instance.LimboTechno.remove(pThis);

	pOwnerExt->OwnedCountedHarvesters.erase(pThis);

	if (this->AbsType != AbstractType::Building) {
		for (auto& tun : pOwnerExt->Tunnels) {
			tun.Vector.remove((FootClass*)pThis);
		}

		if (RulesExtData::Instance()->ExtendedBuildingPlacing && this->AbsType == AbstractType::Unit && ((UnitClass*)pThis)->Type->DeploysInto)
		{
			pOwnerExt->OwnedDeployingUnits.remove((UnitClass*)pThis);
		}
	}

	PhobosEntity::DestroyEntity<true>(this->ShieldEntity);
	PhobosEntity::DestroyEntity<true>(this->PoweredUnitEntity);
	PhobosEntity::DestroyEntity<true>(this->RadarJammerEntity);
}

// =============================
// container hooks

//ASMJIT_PATCH(0x6F3183, TechnoClass_CTOR, 0x5)
//{
//	GET(TechnoClass*, pItem, ESI);
//	HouseExtData::LimboTechno.push_back_unique(pItem);
//	TechnoExtContainer::Instance.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6F4500, TechnoClass_DTOR, 0x5)
//{
//	GET(TechnoClass*, pItem, ECX);
//
//
//	//TechnoExtContainer::Instance.RemoveExtOf(pItem , pExt);
//	return 0;
//}

//ASMJIT_PATCH(0x7077C0, TechnoClass_Detach, 0x7)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, target, 0x4);
//	GET_STACK(bool, all, 0x8);
//
//
//	TechnoExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
//
//	return 0x0;
//}

ASMJIT_PATCH(0x710415, TechnoClass_AnimPointerExpired_add, 6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(TechnoClass*, pThis, ECX);

	if (auto pExt = TechnoExtContainer::Instance.Find(pThis))
	{
		if (pExt->EMPSparkleAnim.get() == pAnim)
			pExt->EMPSparkleAnim.release();

		if (auto pShield = pExt->GetShield())
			pShield->InvalidateAnimPointer(pAnim);

		if (pExt->WebbedAnim.get() == pAnim)
			pExt->WebbedAnim.release();

		if (pExt->CurrentDelayedFireAnim.get() == pAnim)
			pExt->CurrentDelayedFireAnim.release();

		pExt->AeData.InvalidateAnimPointer(pAnim);

		for (auto& _phobos_AE : pExt->PhobosAE) {

			if(!_phobos_AE)
				continue;

			_phobos_AE->InvalidateAnimPointer(pAnim);
		}
	}

	return 0x0;
}

void TechnoExtData::RecordRecoilData()
{
	const auto pThis = this->This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (auto turretIndex = pTypeExt->BurstPerTurret
		? ((pThis->CurrentBurstIndex / pTypeExt->BurstPerTurret) % (pTypeExt->ExtraTurretCount + 1))
		: 0)
	{
		turretIndex -= 1;
		this->ExtraTurretRecoil[turretIndex].TravelSoFar = 0.0;
		this->ExtraTurretRecoil[turretIndex].Fire();
	}
	else
	{
		pThis->TurretRecoil.TravelSoFar = 0.0;
		pThis->TurretRecoil.Fire();
	}

	if (auto barrelIndex = (pTypeExt->ExtraTurretCount || pTypeExt->ExtraBarrelCount)
		? (pThis->CurrentBurstIndex % ((pTypeExt->ExtraBarrelCount + 1) * (pTypeExt->ExtraTurretCount + 1)))
		: 0)
	{
		barrelIndex -= 1;
		this->ExtraBarrelRecoil[barrelIndex].TravelSoFar = 0.0;
		this->ExtraBarrelRecoil[barrelIndex].Fire();
	}
	else
	{
		pThis->BarrelRecoil.TravelSoFar = 0.0;
		pThis->BarrelRecoil.Fire();
	}
}

void TechnoExtData::UpdateRecoilData()
{
	const auto pThis = this->This();

	if(pThis->GetTechnoType()->TurretRecoil)
		return;

	pThis->TurretRecoil.Update();
	pThis->BarrelRecoil.Update();

	for (auto& data : this->ExtraTurretRecoil)
		data.Update();

	for (auto& data : this->ExtraBarrelRecoil)
		data.Update();
}