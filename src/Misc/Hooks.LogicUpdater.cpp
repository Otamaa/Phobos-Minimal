
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>
#include <Kamikaze.h>

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

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>
#include <Commands/ShowTechnoNames.h>

#include <Locomotor/TunnelLocomotionClass.h>

DEFINE_HOOK(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
	{
		pShieldData->SetAnimationVisibility(false);
	}

	return 0;
}

DEFINE_HOOK(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PRE_DIG_OUT)
	{
		if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
			pShieldData->SetAnimationVisibility(true);
	}

	return 0;
}

void UpdateWebbed(FootClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pExt->IsWebbed)
		return;

	if (auto pInf = specific_cast<InfantryClass*>(pThis)){
		if (pInf->ParalysisTimer.Completed()) {

			pExt->IsWebbed = false;

			if (pExt->WebbedAnim) {
				pExt->WebbedAnim.clear();
			}

			TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(pInf);
		}
	}
}

#include <Misc/Ares/Hooks/Header.h>

#ifndef aaa
DEFINE_OVERRIDE_HOOK(0x6F9E50, TechnoClass_AI_Early, 0x5)
#else
DEFINE_HOOK(0x6F9E50, TechnoClass_AI_Early, 0x5)
#endif
{
	enum { retDead = 0x6FB004 , Continue = 0x0};

	GET(TechnoClass*, pThis, ECX);

	if (!pThis || !pThis->IsAlive)
		return retDead;

#ifndef aaa
	TechnoExt_ExtData::Ares_technoUpdate(pThis);
#endif

	//type may already change ,..
	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto IsBuilding = pThis->WhatAmI() == BuildingClass::AbsID;
	bool IsInLimboDelivered = false;

	if(IsBuilding) {
		IsInLimboDelivered = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LimboID >= 0;
	}

	if (pThis->Location == CoordStruct::Empty || pThis->InlineMapCoords() == CellStruct::Empty) {
		if (!pType->Spawned && !IsInLimboDelivered && !pThis->InLimbo) {
			Debug::Log("Techno[%x : %s] With Invalid Location ! , Removing ! \n", pThis, pThis->get_ID());
			TechnoExtData::HandleRemove(pThis, nullptr, false, false);
			return retDead;
		}
	}

	// Set only if unset or type is changed
	// Notice that Ares may handle type conversion in the same hook here, which is executed right before this one thankfully
	if (!pExt->Type || pExt->Type != pType)
		pExt->UpdateType(pType);

	pExt->IsInTunnel = false; // TechnoClass::AI is only called when not in tunnel.

	if (pExt->UpdateKillSelf_Slave()) {
		return retDead;
	}

	if (pExt->CheckDeathConditions()) {
		return retDead;
	}

	pExt->UpdateBuildingLightning();
	pExt->UpdateShield();
	pExt->UpdateInterceptor();
	//pExt->UpdateFireSelf();
	//pExt->UpdateMobileRefinery();
	pExt->UpdateMCRangeLimit();
	pExt->UpdateSpawnLimitRange();
	pExt->UpdateEatPassengers();
	pExt->UpdateGattlingOverloadDamage();
	if(!pThis->IsAlive) {
		return retDead;
	}
	//TODO : improve this to better handle delay anims !
	//pExt->UpdateDelayFireAnim();

	pExt->UpdateRevengeWeapons();
	if (!pThis->IsAlive) {
		return retDead;
	}

	pExt->DepletedAmmoActions();

	return Continue;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->UpdateMindControlAnim();
	return 0;
}

DEFINE_HOOK_AGAIN(0x7363B5, TechnoClass_AI_Tunnel, 0x6) // Unit
DEFINE_HOOK(0x51BAC7, TechnoClass_AI_Tunnel, 0x6) // Inf
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}

DEFINE_HOOK(0x6F9EAD, TechnoClass_AI_AfterAres, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	PassengersFunctional::AI(pThis);
	//SpawnSupportFunctional::AI(pThis);

	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);

	if(pThis->IsAlive)
		GiftBoxFunctional::AI(pExt, pTypeExt);

	if(pThis->IsAlive){
		if (auto& pPBState = pExt->PaintBallState) {
			pPBState->Update(pThis);
		}

		if (auto& pDSState = pExt->DamageSelfState) {
			pDSState->TechnoClass_Update_DamageSelf(pThis);
		}
	}

	return pThis->IsAlive ? 0x6F9EBB : 0x6FAFFD;
	//return 0x6F9EBB;
}

DEFINE_HOOK(0x414DA1, AircraftClass_AI_FootClass_AI, 0x7)
{
	GET(AircraftClass*, pThis, ESI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	pExt->UpdateAircraftOpentopped();
	AircraftPutDataFunctional::AI(pExt, pTypeExt);
	AircraftDiveFunctional::AI(pExt, pTypeExt);
	FighterAreaGuardFunctional::AI(pExt, pTypeExt);

	pThis->FootClass::Update();
	return 0x414DA8;
}

DEFINE_HOOK(0x4DA698, FootClass_AI_IsMovingNow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET8(bool, IsMovingNow, AL);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	 DriveDataFunctional::AI(pExt);
	 //UpdateWebbed(pThis);

	if (IsMovingNow)
	{
		// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
		// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
		pExt->UpdateLaserTrails();

		TrailsManager::AI(static_cast<TechnoClass*>(pThis));

		return 0x4DA6A0;
	}

	return 0x4DA7B0;
}

// Ares-hook jmp to this offset
DEFINE_HOOK(0x71A88D, TemporalClass_AI_Add, 0x8) //0
{
	GET(TemporalClass*, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		if (pTarget->IsMouseHovering)
			pTarget->IsMouseHovering = false;

		auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

		if (const auto pShieldData = pTargetExt->GetShield())
		{
			if (pShieldData->IsAvailable())
				pShieldData->OnTemporalUpdate(pThis);
		}

		//pTargetExt->UpdateFireSelf();
		//pTargetExt->UpdateRevengeWeapons();

		if (auto pBldTarget = specific_cast<BuildingClass*>(pTarget))
		{
			auto pExt = BuildingExtContainer::Instance.Find(pBldTarget);

			std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
			{ {
			{ pBldTarget->Type , &pBldTarget->CashProductionTimer },
			{ pBldTarget->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
			{ pBldTarget->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
			 { pBldTarget->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
			} };

			for (auto& [pbld, timer] : Timers)
			{
				if (pbld)
					timer->Pause();
			}
		}
	}

	// Recovering vanilla instructions that were broken by a hook call
	return R->EAX<int>() <= 0 ? 0x71A895 : 0x71AB08;
}

//DEFINE_HOOK_AGAIN(0x6FAFFD, TechnoClass_LateUpdate,  7)
//DEFINE_HOOK(0x6FAF7A, TechnoClass_LateUpdate, 7)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	return 0;
//}

//DEFINE_HOOK_AGAIN(0x44055D, TechnoClass_WarpUpdate , 6) //Building
//DEFINE_HOOK_AGAIN(0x51BBDF, TechnoClass_WarpUpdate , 6) //Infantry
//DEFINE_HOOK_AGAIN(0x736321, TechnoClass_WarpUpdate , 6) //Unit
//DEFINE_HOOK(0x414CF2, TechnoClass_WarpUpdate ,6) //Aircraft
//// If pObject.Is_Being_Warped() is ture, will skip Foot::AI and Techno::AI
//{
//	GET(TechnoClass*, pThis, ESI);
//	return 0;
//}


//DEFINE_HOOK(0x736479, UnitClass_AI_FootClass_AI, 0x7)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	//const auto pExt = TechnoExtContainer::Instance.Find(pThis);
//	//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
//
//	//JJFacingFunctional::AI(pExt, pTypeExt);
//
//	pThis->FootClass::Update();
//
//	return 0x736480;
//}

//DEFINE_HOOK(0x4DA63B, FootClass_AI_AfterRadSite, 0x6)
//{
//	GET(FootClass*, pThis, ESI);
//
//	auto pExt = TechnoExtContainer::Instance.Find(pThis);
//
//	if (pThis->SpawnOwner && !pExt->IsMissisleSpawn)
//	{
//		auto pSpawnTechnoType = pThis->SpawnOwner->GetTechnoType();
//		auto pSpawnTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pSpawnTechnoType);
//
//		if (const auto pTargetTech = abstract_cast<TechnoClass*>(pThis->Target))
//		{
//			//Spawnee trying to chase Aircraft that go out of map until it reset
//			//fix this , so reset immedietely if target is not on map
//			if (!MapClass::Instance->IsValid(pTargetTech->Location)
//				|| pTargetTech->TemporalTargetingMe
//				|| (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
//				)
//			{
//				if (pThis->SpawnOwner->Target == pThis->Target)
//					pThis->SpawnOwner->SetTarget(nullptr);
//
//				pThis->SpawnOwner->SpawnManager->ResetTarget();
//			}
//
//		}
//		else if (pSpawnTechnoTypeExt->MySpawnSupportDatas.Enable && pThis->SpawnOwner->GetCurrentMission() != Mission::Attack && pThis->GetCurrentMission() == Mission::Attack)
//		{
//			if (pThis->SpawnOwner->Target == pThis->Target)
//				pThis->SpawnOwner->SetTarget(nullptr);
//
//			pThis->SpawnOwner->SpawnManager->ResetTarget();
//		}
//	}
//
//	//return pThis->IsLocked ? 0x4DA677 : 0x4DA643;
//	return 0;
//}

//
//DEFINE_HOOK(0x55B5FB, LogicClass_AI_AfterEMPulse, 0x6)
//{
//
//	return 0x0;
//}