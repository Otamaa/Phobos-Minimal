#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Misc/AresData.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Helpers/Enumerators.h>

struct TechnoExperienceData
{

	static void AddAirstrikeFactor(TechnoClass*& pKiller, double& d_factor)
	{
		// before we do any other logic, check if this kill was committed by an
		// air strike and its designator shall get the experience.
		if (pKiller->Airstrike)
		{
			if (const auto pDesignator = pKiller->Airstrike->Owner)
			{
				if (const auto pDesignatorExt = TechnoTypeExt::ExtMap.Find(pDesignator->GetTechnoType()))
				{
					if (pDesignatorExt->ExperienceFromAirstrike)
					{
						pKiller = pDesignator;
						d_factor *= pDesignatorExt->AirstrikeExperienceModifier;
					}
				}
			}
		}
	}

	static bool KillerInTransporterFactor(TechnoClass* pKiller, TechnoClass*& pExpReceiver, double& d_factor, bool& promoteImmediately)
	{
		const auto pTransporter = pKiller->Transporter;
		if (!pTransporter)
			return false;

		const auto pTTransporterData = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType());
		const auto TransporterAndKillerAllied = pTransporter->Owner->IsAlliedWith(pKiller);

		if (pKiller->InOpenToppedTransport)
		{
			// check for passenger of an open topped vehicle. transporter can get
			// experience from passengers; but only if the killer and its transporter
			// are allied. so a captured opentopped vehicle won't get experience from
			// the enemy's orders.

			// if passengers can get promoted and this transport is already elite,
			// don't promote this transport in favor of the real killer.
			const TechnoTypeClass* pTTransporter = pTransporter->GetTechnoType();

			if ((!pTTransporter->Trainable || pTTransporterData->PassengersGainExperience) && (pTransporter->Veterancy.IsElite() || !TransporterAndKillerAllied) && pKiller->GetTechnoType()->Trainable)
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

	static void AddExperience(TechnoClass* pExtReceiver, int victimCost, double factor)
	{
		const auto TechnoCost = pExtReceiver->GetTechnoType()->GetActualCost(pExtReceiver->Owner);
		const auto WeightedVictimCost = static_cast<int>(victimCost * factor);
		if (TechnoCost > 0 && WeightedVictimCost > 0)
		{
			pExtReceiver->Veterancy.Add(TechnoCost, WeightedVictimCost);
		}
	}

	static void MCControllerGainExperince(TechnoClass* pExpReceiver, TechnoClass* pVictim, double& d_factor, int victimCost)
	{
		// mind-controllers get experience, too.
		if (auto pController = pExpReceiver->MindControlledBy)
		{
			if (!pController->Owner->IsAlliedWith(pVictim->Owner))
			{

				// get the mind controllers extended properties
				const auto pTController = pController->GetTechnoType();
				const auto pTControllerData = TechnoTypeExt::ExtMap.Find(pTController);

				// promote the mind-controller
				if (pTController->Trainable)
				{
					// the mind controller gets its own factor
					AddExperience(pController, victimCost, d_factor * pTControllerData->MindControlExperienceSelfModifier);
				}

				// modify the cost of the victim.
				d_factor *= pTControllerData->MindControlExperienceVictimModifier;
			}
		}
	}

	static void GetSpawnerData(TechnoClass*& pSpawnOut, TechnoClass*& pExpReceiver, double& d_spawnFacor, double& d_ExpFactor)
	{
		if (const auto pSpawner = pExpReceiver->SpawnOwner)
		{
			const auto pTSpawner = pSpawner->GetTechnoType();
			if (!pTSpawner->MissileSpawn && pTSpawner->Trainable)
			{
				const auto pTSpawnerData = TechnoTypeExt::ExtMap.Find(pTSpawner);

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

	static void PromoteImmedietely(TechnoClass* pExpReceiver, bool bSilent, bool Flash)
	{
		auto newRank = pExpReceiver->Veterancy.GetRemainingLevel();

		if (pExpReceiver->CurrentRanking != newRank)
		{
			if (pExpReceiver->CurrentRanking != Rank::Invalid)
			{
				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pExpReceiver->GetTechnoType());

				if (pTypeExt->Promote_IncludePassengers)
				{
					auto const& nCur = pExpReceiver->Veterancy;

					for (NextObject object(pExpReceiver->Passengers.GetFirstPassenger()); object; ++object)
					{
						if (auto const pFoot = generic_cast<FootClass*>(*object))
						{
							if (!pFoot->GetTechnoType()->Trainable)
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

				if (newRank == Rank::Veteran)
				{
					flash = pTypeExt->Promote_Vet_Flash.Get(RulesExt::Global()->VeteranFlashTimer);
					sound = pTypeExt->Promote_Vet_Sound.Get(pRules->UpgradeVeteranSound);
					eva = pTypeExt->Promote_Vet_Eva;
					pNewType = pTypeExt->Promote_Vet_Type;
					promoteExp = pTypeExt->Promote_Vet_Exp;
				}
				else if (newRank == Rank::Elite)
				{
					flash = pTypeExt->Promote_Elite_Flash.Get(pRules->EliteFlashTimer);
					sound = pTypeExt->Promote_Elite_Sound.Get(pRules->UpgradeEliteSound);
					eva = pTypeExt->Promote_Elite_Eva;
					pNewType = pTypeExt->Promote_Elite_Type;
					promoteExp = pTypeExt->Promote_Elite_Exp;
				}

				if (pNewType && AresData::ConvertTypeTo(pExpReceiver, pNewType) && promoteExp != 0.0)
				{
					pExpReceiver->Veterancy.Add(promoteExp);
					newRank = pExpReceiver->Veterancy.GetRemainingLevel();
				}

				if (!bSilent && pExpReceiver->Owner->IsControlledByCurrentPlayer())
				{
					VocClass::PlayAt(sound, (pExpReceiver->Transporter ? pExpReceiver->Transporter : pExpReceiver)->Location, nullptr);
					VoxClass::PlayIndex(eva);
				}

				if (Flash && flash > 0)
				{
					pExpReceiver->Flashing.DurationRemaining = flash;
				}

				AresData::RecalculateStat(pExpReceiver);
			}

			pExpReceiver->CurrentRanking = newRank;
		}
	}

	static void UpdateVeterancy(TechnoClass*& pExpReceiver, TechnoClass* pKiller, TechnoClass* pVictim, int VictimCost, double& d_factor, bool promoteImmediately)
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
				AddExperience(pExpReceiver, VictimCost, d_factor);

				// if there is a spawn, let it get its share.
				if (pSpawn)
				{
					AddExperience(pSpawn, VictimCost, d_factor * SpawnFactor);
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

	static void EvaluateExtReceiverData(TechnoClass*& pExpReceiver, TechnoClass* pKiller, double& d_factor, bool& promoteImmediately)
	{
		const auto pKillerTechnoType = pKiller->GetTechnoType();
		const auto pKillerTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pKillerTechnoType);

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
					&& pGunner->GetTechnoType()->Trainable && pKillerTechnoTypeExt->PassengersGainExperience)
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
					TechnoTypeClass* pTSpawner = pSpawner->GetTechnoType();
					if (pTSpawner->Trainable)
					{
						pExpReceiver = pSpawner;
					}
				}

			}
			else if (pKiller->CanOccupyFire())
			{
				// game logic, with added check for Trainable
				if (BuildingClass* pKillerBld = specific_cast<BuildingClass*>(pKiller))
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
};

DEFINE_OVERRIDE_HOOK(0x6FA054, TechnoClass_Update_Veterancy, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExperienceData::PromoteImmedietely(pThis, false, true);
	return 0x6FA14B;
}

// #346, #464, #970, #1014
// handle all veterancy gains ourselves
DEFINE_OVERRIDE_HOOK(0x702E9D, TechnoClass_RegisterDestruction_Veterancy, 0x6)
{

	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ESI);
	GET(const int, VictimCost, EBP);

	// get the unit that receives veterancy
	TechnoClass* pExpReceiver = nullptr;
	double ExpFactor = 1.0;
	bool promoteImmediately = false;

	// this replace Killer with Airstrike Designator
	TechnoExperienceData::AddAirstrikeFactor(pKiller, ExpFactor);

	// this evalueate ExpReceiver with various Killer properties also update the Expfactor
	TechnoExperienceData::EvaluateExtReceiverData(pExpReceiver, pKiller, ExpFactor, promoteImmediately);

	// update the veterancy
	TechnoExperienceData::UpdateVeterancy(pExpReceiver, pKiller, pVictim, VictimCost, ExpFactor, promoteImmediately);

	// skip the entire veterancy handling
	return 0x702FF5;
}
