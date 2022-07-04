
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>

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

#include <Phobos_ECS.h>

void TechnoClass_AI_GattlingDamage(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (!pType->IsGattling)
		return;

	auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pThisTypeExt->Gattling_Overload.Get())
		return;

	auto const pExt = TechnoExt::GetExtData(pThis);
	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (pExt->GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue) {
			pExt->GattlingDmageDelay = -1;
			pExt->GattlingDmageSound = false;
			return;
		}

		pExt->GattlingDmageDelay = pThisTypeExt->Gattling_Overload_Frames.Get();
		auto nDamage = pThisTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0) {
			pExt->GattlingDmageSound = false;
		} else {
			pThis->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, 0, 0, 0, 0);

			if (!pExt->GattlingDmageSound) {
				if(pThisTypeExt->Gattling_Overload_DeathSound.Get(-1) >= 0)
					VocClass::PlayAt(pThisTypeExt->Gattling_Overload_DeathSound, pThis->Location, 0);

				pExt->GattlingDmageSound = true;
			}

			if (auto const pParticle = pThisTypeExt->Gattling_Overload_ParticleSys.Get()) {
				for (int i = pThisTypeExt->Gattling_Overload_ParticleSysCount.Get() ; i > 0; --i)
				{
					auto const nRandomY = ScenarioGlobal->Random(-200, 200);
					auto const nRamdomX = ScenarioGlobal->Random(-200, 200);
					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
				}
			}

			if (pThis->WhatAmI() == AbstractType::Unit &&  pThis->IsAlive && pThis->IsVoxel()) {
				double const nBase = ScenarioGlobal->Random(0,1) ? 0.015:0.029999999;
				double const nCopied_base = (ScenarioGlobal->Random(0, 100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	} else {
		--pExt->GattlingDmageDelay;
	}

}

static void KillSlave(TechnoClass* pThis)
{
	if (auto pInf = specific_cast<InfantryClass*>(pThis)) {
		if (pInf->Type->Slaved && !pInf->InLimbo && pInf->IsAlive && pInf->Health > 0 && !pInf->TemporalTargetingMe) {
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type);
			if (!pInf->SlaveOwner && pTypeExt->Death_WithMaster.Get())
				TechnoExt::KillSelf(pInf, pTypeExt->Death_Method);
		}
	}
}

void TechnoExt::ExtData::InitFunctionEvents()
{
	/*
	GenericFuctions.clear();

	//register desired functions !
	GenericFuctions += TechnoExt::UpdateMindControlAnim;
	GenericFuctions += TechnoExt::ApplyMindControlRangeLimit;
	GenericFuctions += TechnoExt::ApplyInterceptor;
	GenericFuctions += TechnoExt::ApplySpawn_LimitRange;
	GenericFuctions += TechnoExt::CheckDeathConditions;
	GenericFuctions += TechnoExt::EatPassengers;
#ifdef COMPILE_PORTED_DP_FEATURES
	GenericFuctions += PassengersFunctional::AI;
	GenericFuctions += SpawnSupportFunctional::AI;
#endif
	GenericFuctions += TechnoClass_AI_GattlingDamage;
	*/
}

void TechnoExt::InitializeItems(TechnoClass* pThis)
{
	auto pExt = TechnoExt::GetExtData(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pExt || !pTypeExt)
		return;

	//pExt->InitFunctionEvents();
	pExt->ID = pThis->get_ID();
	pExt->CurrentShieldType = pTypeExt->ShieldType;

	if (pThis->WhatAmI() != AbstractType::Building)
	{
		if (pTypeExt->LaserTrailData.size() > 0 && !pThis->GetTechnoType()->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

#ifdef COMPILE_PORTED_DP_FEATURES
		pExt->IsMissileHoming = pTypeExt->MissileHoming.Get();
#endif
		TechnoExt::InitializeLaserTrail(pThis, false);

#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(pThis);
#endif
	}
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	auto const pExt = TechnoExt::GetExtData(pThis);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt) {
		if (CRT::strlen(pExt->ID.data()) && CRT::strcmp(pExt->ID.data(), pThis->get_ID()))
			pExt->ID = pThis->get_ID();

		TechnoExt::UpdateMindControlAnim(pThis);
		TechnoExt::ApplyMindControlRangeLimit(pThis);
		TechnoExt::ApplyInterceptor(pThis);
		TechnoExt::ApplySpawn_LimitRange(pThis);
		//KillSlave(pThis);
		TechnoExt::CheckDeathConditions(pThis);
		TechnoExt::EatPassengers(pThis);
#ifdef COMPILE_PORTED_DP_FEATURES
		PassengersFunctional::AI(pThis);
		SpawnSupportFunctional::AI(pThis);
#endif
		TechnoClass_AI_GattlingDamage(pThis);
		//if (pExt->GenericFuctions.AfterLoadGame)
			//pExt->InitFunctionEvents();

		//pExt->GenericFuctions.run_each(pThis);

#ifdef COMPILE_PORTED_DP_FEATURES
		pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);
		DriveDataFunctional::AI(pExt);
		GiftBoxFunctional::AI(pExt, pTypeExt);

		if (!pExt->PaintBallState.IsActive())
			pExt->PaintBallState.Disable(false);
		else
			if (pThis->WhatAmI() == AbstractType::Building)
				pThis->UpdatePlacement(PlacementType::Redraw);
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
	pThis->FootClass::Update();
}

DEFINE_JUMP(CALL,0x414DA3,GET_OFFSET(AircraftClass_AI_));

static void __fastcall UnitClass_AI_(UnitClass* pThis, void* _)
{

#ifdef COMPILE_PORTED_DP_FEATURES

	auto const pExt = TechnoExt::GetExtData(pThis);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		JJFacingFunctional::AI(pExt, pTypeExt);
	}

#endif
	pThis->FootClass::Update();
}

DEFINE_JUMP(CALL,0x73647B, GET_OFFSET(UnitClass_AI_));

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

DEFINE_JUMP(CALL,0x4F92F6, GET_OFFSET(HouseClass_AI_SWHandler_Add));
*/

void __fastcall LogicClass_AI_(LogicClass* pLogic, void* _)
{
	pLogic->Update();
}

DEFINE_JUMP(CALL,0x55DC9E ,GET_OFFSET(LogicClass_AI_));