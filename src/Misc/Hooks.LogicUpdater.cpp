
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

void TechnoExt::GattlingDamage(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	auto const pThis = pExt->Get();
	auto const pType = pTypeExt->Get();

	if (!pType->IsGattling)
		return;

	if (!pTypeExt->Gattling_Overload.Get())
		return;

	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (pExt->GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue)
		{
			pExt->GattlingDmageDelay = -1;
			pExt->GattlingDmageSound = false;
			return;
		}

		pExt->GattlingDmageDelay = pTypeExt->Gattling_Overload_Frames.Get();
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			pExt->GattlingDmageSound = false;
		}
		else
		{
			pThis->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, 0, 0, 0, 0);

			if (!pExt->GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.Get(-1) >= 0)
					VocClass::PlayAt(pTypeExt->Gattling_Overload_DeathSound, pThis->Location, 0);

				pExt->GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get())
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(); i > 0; --i)
				{
					auto const nRandomY = ScenarioGlobal->Random(-200, 200);
					auto const nRamdomX = ScenarioGlobal->Random(-200, 200);
					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
				}
			}

			if (pThis->WhatAmI() == AbstractType::Unit && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioGlobal->Random(0, 1) ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioGlobal->Random(0, 100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--pExt->GattlingDmageDelay;
	}

}

void TechnoExt::KillSlave(TechnoClass* pThis, TechnoTypeExt::ExtData* pExt)
{
	if (const auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (pInf->Type->Slaved && !pInf->InLimbo && pInf->IsAlive && pInf->Health > 0 && !pInf->TemporalTargetingMe)
		{
			if (!pInf->SlaveOwner && (pExt->Death_WithMaster.Get() || pExt->Slaved_ReturnTo == SlaveReturnTo::Suicide))
				TechnoExt::KillSelf(pInf, pExt->Death_Method);
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
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pExt || !pTypeExt)
		return;

	//pExt->InitFunctionEvents();
	pExt->ID = pThis->get_ID();
	pExt->CurrentShieldType = pTypeExt->ShieldType;
#ifdef COMPILE_PORTED_DP_FEATURES
	pExt->PaintBallState = std::make_unique<PaintBall>();
#endif
	if (pThis->WhatAmI() != AbstractType::Building)
	{
#ifdef ENABLE_HOMING_MISSILE
		if (const auto pFoot = specific_cast<AircraftClass*>(pThis))
		{
			if (auto const pLoco = static_cast<LocomotionClass*>(pFoot->Locomotor.get()))
			{
				CLSID nID { };
				pLoco->GetClassID(&nID);

				if (nID == LocomotionClass::CLSIDs::Rocket && pTypeExt->MissileHoming)
				{
					pExt->MissileTargetTracker = GameCreate<HomingMissileTargetTracker>();
				}
			}
		}
#endif
		if (pTypeExt->LaserTrailData.size() > 0 && !pThis->GetTechnoType()->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

#ifdef ENABLE_HOMING_MISSILE
		pExt->IsMissileHoming = pTypeExt->MissileHoming.Get();
#endif
		TechnoExt::InitializeLaserTrail(pThis, false);

#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(pThis);
#endif
	}
}

void TechnoExt::ApplyMobileRefinery(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	auto const pThis = pExt->Get();

	if (!pTypeExt->MobileRefinery || !abstract_cast<FootClass*>(pThis) || !pThis->Owner)
		return;

	const int cellCount = static_cast<int>(Math::max(pTypeExt->MobileRefinery_FrontOffset.size(), pTypeExt->MobileRefinery_LeftOffset.size()));

	CoordStruct flh = { 0,0,0 };

	for (int idx = 0; idx <= cellCount; idx++)
	{
		flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()) > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] : 0;
		flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()) > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] : 0;
		const CellClass* pCell = MapClass::Instance->GetCellAt(TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false));

		if (!pCell)
			continue;

		const int tValue = pCell->GetContainedTiberiumValue();
		int value = pTypeExt->MobileRefinery_MaxAmount ? Math::min(pTypeExt->MobileRefinery_MaxAmount.Get(), tValue) : tValue;

		if (value) {

			pCell->ReduceTiberium(value);
			value = static_cast<int>(value * pTypeExt->MobileRefinery_TransRate.Get());

			if (pThis->Owner->CanTransactMoney(value)) {
				pThis->Owner->TransactMoney(value);
				FlyingStrings::AddMoneyString(pTypeExt->MobileRefinery_Display , value,pThis,AffectedHouse::All, pThis->GetCoords());
			}
		}
	}
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	// this one is bad idea i think
	// what if they go converted on middle of something here
	// we fcked up , need to revise this
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		if (CRT::strlen(pExt->ID.data()) && CRT::strcmp(pExt->ID.data(), pThis->get_ID())) {
			pExt->ID = pThis->get_ID();
		}

		TechnoExt::RunFireSelf(pExt, pTypeExt);
		TechnoExt::ApplyMobileRefinery(pExt , pTypeExt);
		TechnoExt::UpdateMindControlAnim(pExt);
		TechnoExt::ApplyMindControlRangeLimit(pThis);
		TechnoExt::ApplyInterceptor(pThis, pTypeExt);
		TechnoExt::ApplySpawn_LimitRange(pThis, pTypeExt);
		TechnoExt::KillSlave(pThis, pTypeExt);
		TechnoExt::CheckDeathConditions(pExt, pTypeExt);
		TechnoExt::EatPassengers(pExt, pTypeExt);
#ifdef COMPILE_PORTED_DP_FEATURES
		PassengersFunctional::AI(pThis);
		SpawnSupportFunctional::AI(pThis);
#endif
		TechnoExt::GattlingDamage(pExt, pTypeExt);
		//if (pExt->GenericFuctions.AfterLoadGame)
			//pExt->InitFunctionEvents();

		//pExt->GenericFuctions.run_each(pThis);

#ifdef COMPILE_PORTED_DP_FEATURES
		pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);
		GiftBoxFunctional::AI(pExt, pTypeExt);

		if (auto const pPaintBall = pExt->PaintBallState.get())
		{
			if (pPaintBall->IsActive())
			{
				if (pThis->WhatAmI() == AbstractType::Building)
					pThis->UpdatePlacement(PlacementType::Redraw);
			}
			else
			{
				pPaintBall->Disable(false);
			}
		}
#endif
	}


	return 0;
}

static void __fastcall AircraftClass_AI_(AircraftClass* pThis, void* _)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	// this one is bad idea i think
	// what if they go converted on middle of something here
	// we fcked up , need to revise this
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
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

	pThis->FootClass::Update();
}

DEFINE_JUMP(CALL, 0x414DA3, GET_OFFSET(AircraftClass_AI_));

static void __fastcall UnitClass_AI_(UnitClass* pThis, void* _)
{
#ifdef COMPILE_PORTED_DP_FEATURES

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	// this one is bad idea i think
	// what if they go converted on middle of something here
	// we fcked up , need to revise this
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		JJFacingFunctional::AI(pExt, pTypeExt);
	}

#endif
	pThis->FootClass::Update();
}

DEFINE_JUMP(CALL, 0x73647B, GET_OFFSET(UnitClass_AI_));

DEFINE_HOOK(0x4DA63B, FootClass_AI_AfterRadSite, 0x6)
{
	GET(const FootClass*, pThis, ESI);

	if (const auto pTargetTech = abstract_cast<TechnoClass*>(pThis->Target)) {
		//Spawnee trying to chase Aircraft that go out of map until it reset
		//fix this , so reset immedietely if target is not on map
		if (pThis->SpawnOwner && (!Map.IsValid(pTargetTech->Location) || pTargetTech->TemporalTargetingMe)) {
			pThis->SpawnOwner->SetTarget(nullptr);
			pThis->SpawnOwner->SpawnManager->ResetTarget();
		}
	}

	return pThis->IsLocked() ? 0x4DA677 : 0x4DA643;
	//return 0;
}

DEFINE_HOOK(0x4DA698, FootClass_AI_IsMovingNow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET8(const bool, IsMovingNow, AL);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

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
				if (pThis->WhatAmI() == AbstractType::Aircraft && !pThis->IsInAir() && trail->LastLocation.isset())
					trail->LastLocation.clear();

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

	const auto pExt = BuildingExt::ExtMap.Find(pThis);
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	const auto pTechTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

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

DEFINE_JUMP(CALL, 0x55DC9E, GET_OFFSET(LogicClass_AI_));

void __fastcall KamikazeClass_AI_(Kamikaze* pThis, void* _)
{
	pThis->Update();
}

DEFINE_JUMP(CALL, 0x55B4F0, GET_OFFSET(KamikazeClass_AI_));

//DEFINE_HOOK(0x55AFB3, LogicClass_Update, 0x6) {
//	return 0x0;
//}

DEFINE_HOOK(0x55B719, LogicClass_Update_Late, 0x5)
{
	VerticalLaserClass::Draw_All();
#ifdef ENABLE_HOMING_MISSILE
	HomingMissileTargetTracker::Update_All();
#endif
	return 0x0;
}
