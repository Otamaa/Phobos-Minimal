
#include <Ext/Anim/Body.h>
#include <Ext/Aircraft/Body.h>
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

#include <InfantryClass.h>

#define ENABLE_THESE

ASMJIT_PATCH(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	const auto pExt = TechnoExtContainer::Instance.Find(pLoco->LinkedTo);

	pExt->Get_TechnoStateComponent()->IsBurrowed = true;

	if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
	{
		pShieldData->SetAnimationVisibility(false);
	}

	for (auto& attachEffect : pExt->PhobosAE){
		if(attachEffect)
			attachEffect->SetAnimationTunnelState(false);
	}

	return 0;
}

ASMJIT_PATCH(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PRE_DIG_OUT)
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pLoco->LinkedTo);
		pExt->Get_TechnoStateComponent()->IsBurrowed = false;

		if (const auto pShieldData = TechnoExtContainer::Instance.Find(pLoco->LinkedTo)->GetShield())
			pShieldData->SetAnimationVisibility(true);

		for (auto& attachEffect : pExt->PhobosAE) {
			if(attachEffect)
				attachEffect->SetAnimationTunnelState(true);
		}
	}

	return 0;
}

void UpdateWebbed(FootClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	//webbed component only avaible for infantry !
	if(auto pWebbedComp = pExt->Get_WebbedStateComponent()){
		pThis->ParalysisTimer.Stop();
		pWebbedComp->RestoreLastTargetAndMissionAfterWebbed(static_cast<InfantryClass*>(pThis));
		pWebbedComp->ClearAnim();
		Phobos::gEntt->remove<WebbedStateComponent>(pExt->MyEntity);
	}
}

#include <Misc/Ares/Hooks/Header.h>
#include <New/PhobosAttachedAffect/Functions.h>

#ifndef _Uhh

#endif

ASMJIT_PATCH(0x703789, TechnoClass_Cloak_BeforeDetach, 0x6)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->UpdateMindControlAnim();
	pExt->Get_TechnoStateComponent()->IsDetachingForCloak = true;

	return 0;
}ASMJIT_PATCH_AGAIN(0x6FBBC3, TechnoClass_Cloak_BeforeDetach, 0x5)  // TechnoClass_Cloaking_AI


ASMJIT_PATCH(0x703799, TechnoClass_Cloak_AfterDetach, 0xA)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDetachingForCloak = false;
	return 0;
}ASMJIT_PATCH_AGAIN(0x6FBBCE, TechnoClass_Cloak_AfterDetach, 0x7)  // TechnoClass_Cloaking_AI


ASMJIT_PATCH(0x6FB9D7, TechnoClass_Cloak_RestoreMCAnim, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
		pExt->UpdateMindControlAnim();

	return 0;
}

ASMJIT_PATCH(0x51B389, FootClass_TunnelAI_Enter, 0x6) // Inf
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x735A26, FootClass_TunnelAI_Enter, 0x6) // Unit

// ASMJIT_PATCH(0x6F9EAD, TechnoClass_AI_AfterAres, 0x7)
// {
// 	GET(TechnoClass*, pThis, ESI);

// 	pThis->UpdateIronCurtainTimer();
// 	pThis->UpdateAirstrikeTimer();

// 	auto pExt = TechnoExtContainer::Instance.Find(pThis);

// 	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

// #ifdef ENABLE_THESE
// 	PassengersFunctional::AI(pThis);
// 	SpawnSupportFunctional::AI(pThis);

// 	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);

// 	if(pThis->IsAlive)
// 		GiftBoxFunctional::AI(pExt, pTypeExt);

// 	if(pThis->IsAlive){
// 		pExt->PaintBallStates.erase_all_if([pThis](auto& pb){
// 				if(pb.second.timer.GetTimeLeft()) {
// 					if (pThis->WhatAmI() == BuildingClass::AbsID) {
// 						BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LighningNeedUpdate = true;
// 					}
// 					return false;
// 				}

// 			return true;
// 		});

// 		if (auto pDSState = pExt->DamageSelfState) {
// 			pDSState->TechnoClass_Update_DamageSelf(pThis);
// 		}
// 	}
// #endif
// 	return pThis->IsAlive ? 0x6F9EBB : 0x6FAFFD;
// 	//return 0x6F9EBB;
// }

bool Spawned_Check_Destruction(AircraftClass* aircraft)
{
	if (aircraft->SpawnOwner == nullptr
		|| !aircraft->SpawnOwner->IsAlive
		|| aircraft->SpawnOwner->IsCrashing
		|| aircraft->SpawnOwner->IsSinking
		)
	{
		return false;
	}

	/**
	 *  If our TarCom is null, our original target has died.
	 *  Try targeting something else that is nearby,
	 *  unless we've already decided to head back to the spawner.
	 */
	if (aircraft->Target == nullptr && aircraft->Destination != aircraft->SpawnOwner)
	{
		CoordStruct loc = aircraft->GetCoords();
		aircraft->TargetAndEstimateDamage(&loc, ThreatType::Area);
	}

	/**
	 *  If our TarCom is still null or we're run out of ammo, return to
	 *  whoever spawned us. Once we're close enough, we should be erased from the map.
	 */
	if (aircraft->Target == nullptr || aircraft->Ammo == 0)
	{

		if (aircraft->Destination != aircraft->SpawnOwner)
		{
			aircraft->SetDestination(aircraft->SpawnOwner, true);
			aircraft->ForceMission(Mission::Move);
			aircraft->NextMission();
		}

		CoordStruct myloc = aircraft->GetCoords();
		CoordStruct spawnerloc = aircraft->GetCoords();
		if (myloc.DistanceFrom(spawnerloc) < Unsorted::LeptonsPerCell)
			return true;
	}

	return false;
}

DEFINE_FUNCTION_JUMP(CALL , 0x414DA3  , FakeAircraftClass::_FootClass_Update_Wrapper);

ASMJIT_PATCH(0x4DA63B, FootClass_AI_IsMovingNow, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if(!pThis->IsOnMap && pThis->GetTechnoType()->BalloonHover){
		if(MapClass::Instance->IsWithinUsableArea(pThis->GetCell(), true))
			pThis->IsOnMap = true;
	}

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	DriveDataFunctional::AI(pExt);
	 //UpdateWebbed(pThis);

	return pThis->Locomotor.GetInterfacePtr() ? 0x4DA689 : 0x4DA67F;
}

// Ares-hook jmp to this offset
//ASMJIT_PATCH_AGAIN(0x6FAFFD, TechnoClass_LateUpdate,  7)
//ASMJIT_PATCH(0x6FAF7A, TechnoClass_LateUpdate, 7)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	return 0;
//}

//ASMJIT_PATCH_AGAIN(0x44055D, TechnoClass_WarpUpdate , 6) //Building
//ASMJIT_PATCH_AGAIN(0x51BBDF, TechnoClass_WarpUpdate , 6) //Infantry
//ASMJIT_PATCH_AGAIN(0x736321, TechnoClass_WarpUpdate , 6) //Unit
//ASMJIT_PATCH(0x414CF2, TechnoClass_WarpUpdate ,6) //Aircraft
//// If pObject.Is_Being_Warped() is ture, will skip Foot::AI and Techno::AI
//{
//	GET(TechnoClass*, pThis, ESI);
//	return 0;
//}


//ASMJIT_PATCH(0x736479, UnitClass_AI_FootClass_AI, 0x7)
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

//ASMJIT_PATCH(0x4DA63B, FootClass_AI_AfterRadSite, 0x6)
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
//ASMJIT_PATCH(0x55B5FB, LogicClass_AI_AfterEMPulse, 0x6)
//{
//
//	return 0x0;
//}