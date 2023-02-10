
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
#include <Commands/ShowTechnoNames.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_Early, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0x0;

	auto const pType = pThis->GetTechnoType();
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	// Set only if unset or type is changed
	// Notice that Ares may handle type conversion in the same hook here, which is executed right before this one thankfully
	if (!pExt->Type || pExt->Type != pType)
		pExt->UpdateType(pType);

	pExt->IsInTunnel = false; // TechnoClass::AI is only called when not in tunnel.

	if (pExt->CheckDeathConditions() || pExt->UpdateKillSelf_Slave())
		return 0x0;

	pExt->UpdateBuildingLightning();
	pExt->UpdateShield();
	pExt->UpdateInterceptor();
	//pExt->UpdateFireSelf();
	pExt->UpdateMobileRefinery();
	pExt->UpdateMCRangeLimit();
	pExt->UpdateSpawnLimitRange();
	pExt->UpdateEatPassengers();
	pExt->UpdateGattlingOverloadDamage();

	//TODO : improve this to better handle delay anims !
	//pExt->UpdateDelayFireAnim();

	pExt->UpdateRevengeWeapons();

	return 0x0;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtMap.Find(pThis)->UpdateMindControlAnim();
	return 0;
}

DEFINE_HOOK_AGAIN(0x7363B5, TechnoClass_AI_Tunnel, 0x6) // Unit
DEFINE_HOOK(0x51BAC7, TechnoClass_AI_Tunnel, 0x6) // Inf
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtMap.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}

DEFINE_HOOK(0x6F9EAD, TechnoClass_AI_AfterAres, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	pThis->UpdateIronCurtainTimer();
	pThis->UpdateAirstrikeTimer();

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

#ifdef COMPILE_PORTED_DP_FEATURES
	PassengersFunctional::AI(pThis);
	SpawnSupportFunctional::AI(pThis);

	pExt->MyWeaponManager.TechnoClass_Update_CustomWeapon(pThis);
	GiftBoxFunctional::AI(pExt, pTypeExt);

	if (pExt->PaintBallState)
	{
		pExt->PaintBallState->Update(pThis);
	}

	if (pExt->DamageSelfState)
	{
		pExt->DamageSelfState->TechnoClass_Update_DamageSelf(pThis);
	}
#endif

	return 0x6F9EBB;
}

DEFINE_HOOK(0x414DA1, AircraftClass_AI_FootClass_AI, 0x7)
{
	GET(AircraftClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pExt)
	{
		pExt->UpdateAircraftOpentopped();
		if (pTypeExt) {

#ifdef COMPILE_PORTED_DP_FEATURES

			AircraftPutDataFunctional::AI(pExt, pTypeExt);
			AircraftDiveFunctional::AI(pExt, pTypeExt);
			FighterAreaGuardFunctional::AI(pExt, pTypeExt);

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
		if (pExt)
		{
			pExt->UpdateLaserTrails();
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

	if (const auto pExt = BuildingExt::ExtMap.Find(pThis)) {
		pExt->UpdatePoweredKillSpawns();
		pExt->UpdateAutoSellTimer();
	}

	return 0x0;
}

DEFINE_HOOK(0x4F8440, HouseClass_Update, 0x5)
{
	GET(HouseClass* const, pThis, ECX);
	HouseExt::ExtMap.Find(pThis)->UpdateAutoDeathObjects();
	return 0;
}
//
//DEFINE_HOOK(0x55B5FB, LogicClass_AI_AfterEMPulse, 0x6)
//{
//
//	return 0x0;
//}