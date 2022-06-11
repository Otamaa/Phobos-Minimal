#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTargetFunctional.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterAreaGuardFunctional.h>

#endif

#ifdef GATTLING_OVERLOAD
static std::vector<int> GattValues { { 450 } };
static std::vector<int> GattFrames { { 20} };
static std::vector<int> GattDamage { { 2 } };
static int GattSound { -1 };
static ParticleSystemTypeClass* GattParticles {  };
static int GattParticlesCount { 1 };

void TechnoClass_AI_GattlingDamage(TechnoClass* pThis)
{
	auto pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pExt = TechnoExt::GetExtData(pThis);

	if (!pThisTypeExt || !pExt) {
		return;
	}

	if (pExt->GattlingDmageDelay <= 0)
	{
		int nStage = pThis->GetCurrentGattlingValue();
		if (nStage != 450) {
			pExt->GattlingDmageDelay = -1;
			pExt->GattlingDmageSound = false;
			return;
		}

		pExt->GattlingDmageDelay = 15;

		auto nDamage = 15;

		if (nDamage <= 0) {
			pExt->GattlingDmageSound = false;
		} else {
			pThis->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, 0, 0, 0, 0);

			if (!pExt->GattlingDmageSound) {
				if(GattSound >= 0 )
					VocClass::PlayAt(GattSound, pThis->Location, 0);

				pExt->GattlingDmageSound = true;
			}

			if (auto const pParticle = RulesGlobal->DefaultSparkSystem) {
				for (int i = GattParticlesCount; i > 0; --i)
				{
					auto const nRandomY = ScenarioGlobal->Random(-200, 200);
					auto const nRamdomX = ScenarioGlobal->Random(-200, 200);
					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
				}
			}

			if (pThis->WhatAmI() == AbstractType::Unit &&  pThis->IsAlive) {
				double const nBase = ScenarioGlobal->Random(0,1) ? 0.015:0.029999999;
				double const nCopied_base = (ScenarioGlobal->Random(0, 100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	} else {
		--pExt->GattlingDmageDelay;
	}

}
#endif

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::UpdateMindControlAnim(pThis);
	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::ApplyInterceptor(pThis);
	TechnoExt::ApplySpawn_LimitRange(pThis);
	TechnoExt::CheckDeathConditions(pThis);
	TechnoExt::EatPassengers(pThis);

#ifdef COMPILE_PORTED_DP_FEATURES
	PassengersFunctional::AI(pThis);
	SpawnSupportFunctional::AI(pThis);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pExt = TechnoExt::GetExtData(pThis);

	if (pExt && pTypeExt)
	{
		pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);
		DriveDataFunctional::AI(pExt);
		GiftBoxFunctional::AI(pExt, pTypeExt);
#ifdef GATTLING_OVERLOAD
		if (pTypeExt->OwnerObject()->IsGattling) {
			TechnoClass_AI_GattlingDamage(pThis);
		}
#endif
	}

#endif

	return 0;
}

static void __fastcall AircraftClass_AI_(AircraftClass* pThis, void* _)
{
	if (pThis->Type->OpenTopped)
	{
		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (auto const pInf = generic_cast<FootClass*>(*object))
			{
				if (!pInf->Transporter || !pInf->InOpenToppedTransport)
				{
					pThis->EnteredOpenTopped(pInf);
					pInf->Transporter = pThis;
					pInf->Undiscover();
				}
			}
		}
	}

	auto const pFoot = static_cast<FootClass*>(pThis);
#ifdef COMPILE_PORTED_DP_FEATURES
	auto const pExt = TechnoExt::GetExtData(pThis);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		AircraftPutDataFunctional::AI(pExt, pTypeExt);
		AircraftDiveFunctional::AI(pExt, pTypeExt);
		FighterAreaGuardFunctional::AI(pExt, pTypeExt);
#ifdef HOMING_MISSILE
		if (pExt->IsMissileHoming
			&& pThis->Spawned
			&& pThis->Type->MissileSpawn)
		{
			auto pLoco = static_cast<LocomotionClass*>(pThis->Locomotor.get());

			if (pThis->Type->Locomotor == LocomotionClass::CLSIDs::Rocket())
			{
				if (auto pTechnoTarget = generic_cast<TechnoClass*>(pThis->Target))
				{
					if (!Helpers_DP::IsDeadOrInvisibleOrCloaked(pTechnoTarget))
					{
						pExt->HomingTargetLocation = pTechnoTarget->GetCoords();
					}
				}

				if (pExt->HomingTargetLocation)
				{
					auto const pRocket = static_cast<RocketLocomotionClass*>(pLoco);
					if (pRocket->MissionState > 2)
						pRocket->MovingDestination = pExt->HomingTargetLocation;
				}
			}
		}
#endif
	}

#endif
	pFoot->AI();
}

DEFINE_POINTER_CALL(0x414DA3, &AircraftClass_AI_);

static void __fastcall UnitClass_AI_(UnitClass* pThis, void* _)
{
	auto const pFoot = static_cast<FootClass*>(pThis);


#ifdef COMPILE_PORTED_DP_FEATURES

	auto const pExt = TechnoExt::GetExtData(pThis);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		JJFacingFunctional::AI(pExt, pTypeExt);
	}

#endif
	pFoot->AI();
}

DEFINE_POINTER_CALL(0x73647B, &UnitClass_AI_);

DEFINE_HOOK(0x4DA63B, FootClass_AI_AfterRadSite, 0x6)
{
	GET(FootClass*, pThis, ESI);

	//auto const pExt = TechnoExt::GetExtData(pThis);
	//auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (const auto pTargetTech = abstract_cast<TechnoClass*>(pThis->Target))
	{
		//Spawnee trying to chase Aircraft that go out of map until it reset
		//fix this , so reset immedietely if target is not on map
		if (pThis->SpawnOwner && (!pTargetTech->IsOnMap || pTargetTech->TemporalTargetingMe))
		{
			pThis->SpawnOwner->SetTarget(nullptr);
			pThis->SpawnOwner->SpawnManager->ResetTarget();
		}
	}


	//if (pExt && pTypeExt) { }

	return pThis->IsLocked() ? 0x4DA677 : 0x4DA643;
}

DEFINE_HOOK(0x4DA698, FootClass_AI_IsMovingNow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	bool const IsMovingNow = R->AL();

	auto const pExt = TechnoExt::GetExtData(pThis);
	//auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (IsMovingNow)
	{
		// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
		// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
		if (pExt && !pExt->LaserTrails.empty())
		{
			for (auto const& trail : pExt->LaserTrails)
			{
				if (pThis->WhatAmI() == AbstractType::Aircraft && !pThis->IsInAir() && trail->LastLocation.isset())
					trail->LastLocation.Reset();

				trail->Update(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));
			}
		}
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::AI(static_cast<TechnoClass*>(pThis));
#endif
		return 0x4DA6A0;
	}

	return 0x4DA7B0;
}

// this updated after TechnoClass::AI
// then check if the techno itself is still active/alive/present
DEFINE_HOOK(0x43FE69, BuildingClass_AI_Add, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExt::ExtMap.Find(pThis);
	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	auto pTechTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt && pExt && pTechTypeExt)
	{
		{
			//TechnoExt::ApplyPowered_KillSpawns
			if (pTechTypeExt->Powered_KillSpawns && pThis->Type->Powered && !pThis->IsPowerOnline())
			{
				if (auto pManager = pThis->SpawnManager)
				{
					pManager->ResetTarget();
					for (auto pItem : pManager->SpawnedNodes)
					{
						if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
						{
							if (pItem->Unit)
								pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
									RulesClass::Instance()->C4Warhead, nullptr, false, false, nullptr);
						}
					}
				}
			}
		}

		if (!pThis->Type->Unsellable && pThis->Type->TechLevel != -1)
		{
			auto pRulesExt = RulesExt::Global();
			auto nMission = pThis->GetCurrentMission();

			if (pTypeExt->AutoSellTime.isset())
			{
				if (pTypeExt->AutoSellTime.Get() > 0.0f && nMission != Mission::Selling)
				{
					if (pExt->AutoSellTimer.StartTime == -1 || nMission == Mission::Attack)
						pExt->AutoSellTimer.Start(static_cast<int>(pTypeExt->AutoSellTime.Get() * 900.0));
					else
						if (pExt->AutoSellTimer.Completed())
							pThis->Sell(-1);
				}
			}

			if (!pRulesExt->AI_AutoSellHealthRatio.empty() && pRulesExt->AI_AutoSellHealthRatio.size() >= 3 && (nMission != Mission::Selling))
			{
				if (!pThis->Occupants.Count)
				{
					if (pThis->Owner && !pThis->Owner->IsPlayer() && !pThis->Owner->Type->MultiplayPassive)
					{
						auto nValue = pRulesExt->AI_AutoSellHealthRatio.at(pThis->Owner->GetCorrectAIDifficultyIndex());

						if (nValue > 0.0f && pThis->GetHealthPercentage() <= nValue)
							pThis->Sell(-1);
					}
				}
			}
		}
	}

	return 0x0;
}

/*
DEFINE_HOOK(0x4F8FE1, Houseclass_AI_Add, 0x5)
{
	return 0x0;
}

void __fastcall HouseClass_AI_SWHandler_Add(HouseClass* pThis, void* _)
{
	pThis->SuperWeapon_Handler();
}

DEFINE_POINTER_CALL(0x4F92F6, &HouseClass_AI_SWHandler_Add);
*/