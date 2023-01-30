
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>
#include <Kamikaze.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/DamageSelf/DamageSelfType.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTargetFunctional.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterAreaGuardFunctional.h>

#endif

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>
#include <Phobos_ECS.h>

//DEFINE_HOOK(0x6F9E50, TechnoClass_AI_Early, 0x5)
//{
//	GET(TechnoClass*, pThis, ECX);
//
//	if (!pThis)
//		return 0x0;
//
//
//	return 0x0;
//}

DEFINE_HOOK(0x51BAC7, InfantryClass_AI_Tunnel, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	TechnoExt::ExtMap.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}

DEFINE_HOOK(0x7363B5, UnitClass_AI_Tunnel, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	TechnoExt::ExtMap.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}

DEFINE_HOOK(0x6F9EAD, TechnoClass_AI_AfterAres, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->IsInTunnel = false; // TechnoClass::AI is only called when not in tunnel.

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (auto pBldExt = BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis)))
		{
			//auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldExt->Get()->Type);

			if (pBldExt->LighningNeedUpdate)
			{
				pThis->UpdatePlacement(PlacementType::Redraw);
				pBldExt->LighningNeedUpdate = false;

			}

			//if (pBldTypeExt->RubbleIntact && pThis->GetHealthPercentage_() >= RulesGlobal->ConditionRed)
			//{
			//	pBldExt->RubbleYell(true);
			//}
		}
	}

	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt->CheckDeathConditions())
		return 0x6F9EBB;

	//bool LimboID != -1 = false;
	//if (auto pBuilding = specific_cast<BuildingClass*>(pThis)) {
	//	LimboID != -1 = BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1;
	//}

	//if (pThis->Health  == 0 && !pThis->InLimbo)
	//	FlyingStrings::AddString(L"Already Dead !", true , pThis ,AffectedHouse::All,pThis->Location,{0,0},{0,0,0});

	//if (pExt->AbsType.empty() || pExt->AbsType == AbstractType::None)
	//	pExt->AbsType = pThis->WhatAmI();

	//if(AresData::AresDllHmodule != nullptr) {
	//	auto pGGI = TechnoTypeClass::Find("GGI");

	//	if(pGGI != pTypeExt->Get())
	//		AresData::HandleConvert::Exec(pThis, pGGI);
	//}
	
	if (!pExt->CurrentShieldType)
		Debug::FatalErrorAndExit("Techno[%s] Missing CurrentShieldType ! \n", pThis->GetTechnoType()->get_ID());

	// Set current shield type if it is not set.
	if (!pExt->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		pExt->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (pExt->CurrentShieldType && pExt->CurrentShieldType->Strength && !pExt->Shield) {
		pExt->Shield = std::make_unique<ShieldClass>(pThis);
		pExt->Shield->OnInit();
	}

	if (const  auto pShieldData = pExt->GetShield())
		pShieldData->OnUpdate();

	TechnoExt::ApplyInterceptor(pThis);
	//#ifdef ENABLE_NEWHOOKS
	pExt->RunFireSelf();
	pExt->UpdateMindControlAnim();
	TechnoExt::ApplyMobileRefinery(pThis);
	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::ApplySpawn_LimitRange(pThis);
	TechnoExt::KillSlave(pThis);
	pExt->EatPassengers();

#ifdef COMPILE_PORTED_DP_FEATURES
	PassengersFunctional::AI(pThis);
	SpawnSupportFunctional::AI(pThis);
#endif
	pExt->GattlingDamage();

#ifdef COMPILE_PORTED_DP_FEATURES

	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);
	GiftBoxFunctional::AI(pExt, pTypeExt);

	//if (!LimboID != -1) {
	if (pExt->PaintBallState)
	{
		pExt->PaintBallState->Update(pThis);
	}
	//}

	if (pExt->DamageSelfState)
	{
		pExt->DamageSelfState->TechnoClass_Update_DamageSelf(pThis);
	}
#endif

	if (pExt->DelayedFire_Anim && !pThis->Target && pThis->GetCurrentMission() != Mission::Attack)
	{
		pThis->ArmTimer.Start(pThis->ArmTimer.GetTimeLeft() + 5);

		// Reset Delayed fire animation
		pExt->DelayedFire_Anim = nullptr;
		pExt->DelayedFire_Anim_LoopCount = 0;
		pExt->DelayedFire_DurationTimer = -1;
	}


	for (size_t i = 0; i < pExt->RevengeWeapons.size(); i++)
	{
		auto const& weapon = pExt->RevengeWeapons[i];

		if (weapon.Timer.Expired())
			pExt->RevengeWeapons.erase(pExt->RevengeWeapons.begin() + i);
	}

	return 0x6F9EBB;
}

DEFINE_HOOK(0x414DA1, AircraftClass_AI_FootClass_AI, 0x7)
{
	GET(AircraftClass*, pThis, ESI);

	if (auto pExt = TechnoExt::ExtMap.Find<false>(pThis))
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pThis->GetTechnoType()))
		{
			if (pThis->Type->OpenTopped && pExt && !pExt->AircraftOpentoppedInitEd)
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

				pExt->AircraftOpentoppedInitEd = true;
			}

#ifdef COMPILE_PORTED_DP_FEATURES

			AircraftPutDataFunctional::AI(pExt, pTypeExt);
			AircraftDiveFunctional::AI(pExt, pTypeExt);
			FighterAreaGuardFunctional::AI(pExt, pTypeExt);

#ifdef ENABLE_HOMING_MISSILE
			if (pTypeExt->MissileHoming
				&& pThis->Spawned
				&& pThis->Type->MissileSpawn)
			{
				const auto pLoco = static_cast<LocomotionClass*>(pThis->Locomotor.get());
				CLSID nID { };
				pLoco->GetClassID(&nID);

				if (nID == LocomotionClass::CLSIDs::Rocket())
				{
					if (auto const pTracker = pExt->MissileTargetTracker)
					{
						pTracker->AI();
						auto const pRocket = static_cast<RocketLocomotionClass*>(pLoco);

						//check if the coord is actually valid
						// if not , just move on
						if (pRocket->MissionState > 2 && pTracker->Coord && (Map.GetCellAt(pTracker->Coord)))
							pRocket->MovingDestination = pTracker->Coord;
					}
				}
		}
#endif
#endif
	}
}

	pThis->FootClass::Update();
	return 0x414DA8;
	}

DEFINE_HOOK(0x736479, UnitClass_AI_FootClass_AI, 0x7)
{
	GET(UnitClass*, pThis, ESI);

#ifdef COMPILE_PORTED_DP_FEATURES
	if (auto pExt = TechnoExt::ExtMap.Find<false>(pThis))
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pThis->GetTechnoType()))
		{
			JJFacingFunctional::AI(pExt, pTypeExt);
		}
	}
#endif
	pThis->FootClass::Update();

	return 0x736480;
}

DEFINE_HOOK(0x4DA63B, FootClass_AI_AfterRadSite, 0x6)
{
	GET(const FootClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis->SpawnOwner && pExt->IsMissisleSpawn)
	{

		auto pSpawnTechnoType = pThis->SpawnOwner->GetTechnoType();
		auto pSpawnTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSpawnTechnoType);

		if (const auto pTargetTech = abstract_cast<TechnoClass*>(pThis->Target))
		{
			//Spawnee trying to chase Aircraft that go out of map until it reset
			//fix this , so reset immedietely if target is not on map
			if (!Map.IsValid(pTargetTech->Location)
				|| pTargetTech->TemporalTargetingMe
#ifdef COMPILE_PORTED_DP_FEATURES
				|| (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
#endif
				)
			{
				if (pThis->SpawnOwner->Target == pThis->Target)
					pThis->SpawnOwner->SetTarget(nullptr);

				pThis->SpawnOwner->SpawnManager->ResetTarget();
			}

		}
#ifdef COMPILE_PORTED_DP_FEATURES
		else if (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
		{
			if (pThis->SpawnOwner->Target == pThis->Target)
				pThis->SpawnOwner->SetTarget(nullptr);

			pThis->SpawnOwner->SpawnManager->ResetTarget();
		}
#endif
	}

	//return pThis->IsLocked ? 0x4DA677 : 0x4DA643;
	return 0;
}

DEFINE_HOOK(0x4DA698, FootClass_AI_IsMovingNow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET8(bool, IsMovingNow, AL);

	auto pExt = TechnoExt::ExtMap.Find<false>(pThis);

#ifdef COMPILE_PORTED_DP_FEATURES
	if (pExt)
		DriveDataFunctional::AI(pExt);
#endif

	if (IsMovingNow)
	{
		// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
		// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
		if (pExt && !pExt->LaserTrails.empty())
		{
			for (auto const& trail : pExt->LaserTrails)
			{
				if (pThis->CloakState == CloakState::Cloaked && !trail->Type->CloakVisible)
					continue;

				if (!pExt->IsInTunnel)
					trail->Visible = true;

				if (pThis->WhatAmI() == AbstractType::Aircraft && !pThis->IsInAir() && trail->LastLocation.isset())
					trail->LastLocation.clear();

				CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret);

				if (pThis->CloakState == CloakState::Uncloaking && !trail->Type->CloakVisible)
					trail->LastLocation = trailLoc;
				else
					trail->Update(trailLoc);
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

	if (auto pExt = BuildingExt::ExtMap.Find(pThis))
	{
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
		auto pTechTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		if (pTypeExt && pTechTypeExt)
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
									RulesClass::Instance()->C4Warhead, nullptr, true, true, nullptr);
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
						if (pThis->Owner && !pThis->Owner->IsCurrentPlayer() && !pThis->Owner->Type->MultiplayPassive)
						{
							auto nValue = pRulesExt->AI_AutoSellHealthRatio.at(pThis->Owner->GetCorrectAIDifficultyIndex());

							if (nValue > 0.0f && pThis->GetHealthPercentage() <= nValue)
								pThis->Sell(-1);
						}
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
*/


//DEFINE_HOOK(0x55DC99, LoGicClassUpdate_probe, 0x5)
//{
//	return 0x0;
//}
//
//DEFINE_HOOK(0x55B4EB, KamikazeClass_AI_Probe, 0x5)
//{
//   return 0x0;
//}
//
//DEFINE_HOOK(0x55AFB3, LogicClass_Update, 0x6) {
//	//VerticalLaserClass::OnUpdateAll();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x55B719, LogicClass_Update_Late, 0x5)
//{
//	//VerticalLaserClass::OnUpdateAll();
//#ifdef ENABLE_HOMING_MISSILE
//	HomingMissileTargetTracker::Update_All();
//#endif
//	return 0x0;
//}
//
//// in progress: Initializing Tactical display
//DEFINE_HOOK(0x6875F3 , Scenario_Start1, 0x6) {
//
//	return 0;
//}
