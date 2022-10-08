#include <AircraftClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <VoxelAnimClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <FlyLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <TemporalClass.h>
#include <CellClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/EnumFunctions.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(0x423365, Phobos_BugFixes_SHPShadowCheck, 0x8)
{
	GET(AnimClass*, pAnim, ESI);
	return (pAnim->Type->Shadow && pAnim->HasExtras) ?
		0x42336D :
		0x4233EE;
}

/*
	Allow usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable

	When TileSet number crosses 255 in theater INI files, the NE-SW broken bridges
	become non-repairable. The reason is that the NonMarbleMadnessTile array of size 256
	overflows when filled and affects the variables like BridgeTopRight1 and BridgeBottomLeft2
	that come after it. This patch removes the filling of the unused NonMarbleMadnessTile array
	and its references.

	Author : E1 Elite
*/
DEFINE_JUMP(LJMP,0x545CE2, 0x545CE9) //Phobos_BugFixes_Tileset255_RemoveNonMMArrayFill
DEFINE_JUMP(LJMP,0x546C23, 0x546C8B) //Phobos_BugFixes_Tileset255_RefNonMMArray

// WWP's shit code! Wrong check.
// To avoid units dying when they are already dead.
DEFINE_HOOK(0x5F53AA, ObjectClass_ReceiveDamage_DyingFix, 0x6)
{
	enum { PostMortem = 0x5F583E, ContinueCheck = 0x5F53B0 };

	GET(int, health, EAX);
	GET(ObjectClass*, pThis, ESI);

	if (health <= 0 || !pThis->IsAlive)
		return PostMortem;

	return ContinueCheck;
}

DEFINE_HOOK(0x4D7431, FootClass_ReceiveDamage_DyingFix, 0x5)
{
	GET(FootClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	if (result != DamageState::PostMortem)
		if((pThis->IsSinking || (!pThis->IsAttackedByLocomotor && pThis->IsCrashing)))
			R->EAX(DamageState::PostMortem);

	return 0;
}

DEFINE_HOOK(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (result == DamageState::NowDead){

		if(pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
			pThis->IsAttackedByLocomotor = false;

#ifndef COMPILE_PORTED_DP_FEATURES
		if(!pThis->Type->Voxel){
			if (pThis->Type->MaxDeathCounter > 0
				&& !pThis->InLimbo
				&& !pThis->IsCrashing
				&& !pThis->IsSinking
				&& !pThis->TemporalTargetingMe
				&& !pThis->IsInAir()
				&& pThis->DeathFrameCounter <= 0
				) {

				pThis->Stun();
				if (pThis->Locomotor->Is_Moving_Now())
					pThis->Locomotor->Stop_Moving();

				pThis->DeathFrameCounter = 1;
			}
		}
#endif
	}

	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0){
		R->EAX(DamageState::PostMortem);
	}

	return 0;
}

#ifndef COMPILE_PORTED_DP_FEATURES
DEFINE_HOOK(0x5F452E, TechnoClass_Selectable_DeathCounter, 0x6) // 8
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
		if (pUnit->DeathFrameCounter > 0)
			return 0x5F454E;

	return 0x0;
}

DEFINE_HOOK(0x737CBB, UnitClass_ReceiveDamage_DeathCounter, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
		if (pUnit->DeathFrameCounter > 0)
			return 0x737D26;

	return 0x0;
}
#endif

/*
DEFINE_HOOK(0x5F53A1, ObjectClass_ReceiveDamage_DeathcCounter, 0x5)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
		if (pUnit->DeathFrameCounter > 0)
			return 0x5F5830;

	return 0x0;
}*/

/*
DEFINE_HOOK(0x737DBF, UnitClass_ReceiveDamage_DeathAnim, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->MaxDeathCounter > 0)
		pThis->DeathFrameCounter = 1;
	else
		pThis->DeathFrameCounter = 0;

	return 0x737DC9;
}*/

// Restore DebrisMaximums logic (issue #109)
// Author: Otamaa
DEFINE_HOOK(0x702299, TechnoClass_ReceiveDamage_DebrisMaximumsFix, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	auto pType = pThis->GetTechnoType();

	// If DebrisMaximums has one value, then legacy behavior is used
	if (pType->DebrisMaximums.Count == 1 &&
		pType->DebrisMaximums.GetItem(0) > 0 &&
		pType->DebrisTypes.Count > 0)
	{
		return 0;
	}

	auto totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(
		pType->MinDebris, pType->MaxDebris);

	if (pType->DebrisTypes.Count > 0 && pType->DebrisMaximums.Count > 0)
	{
		auto nCoords = pThis->GetCenterCoord();

		for (int currentIndex = 0; currentIndex < pType->DebrisTypes.Count; ++currentIndex)
		{
			if (pType->DebrisMaximums.GetItem(currentIndex) > 0)
			{
				int amountToSpawn = abs(int(ScenarioGlobal->Random.Random())) % pType->DebrisMaximums.GetItem(currentIndex) + 1;
				amountToSpawn = Math::LessOrEqualTo(amountToSpawn, totalSpawnAmount);
				totalSpawnAmount -= amountToSpawn;

				for (; amountToSpawn > 0; --amountToSpawn)
				{
					if (auto pVoxAnim = GameCreate<VoxelAnimClass>(pType->DebrisTypes.GetItem(currentIndex),
						&nCoords, pThis->Owner))
							VoxelAnimExt::ExtMap.Find(pVoxAnim)->Invoker = pThis;
				}

				if (totalSpawnAmount <= 0)
				{
					totalSpawnAmount = 0;
					break;
				}
			}
		}
	}

	// debrisanim has no owner , duh
	R->EBX(totalSpawnAmount);

	return 0x7023E5;
}

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
DEFINE_HOOK(0x4C7518, EventClass_Execute_StopUnitDeployFire, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);
	if (pUnit && pUnit->CurrentMission == Mission::Unload && pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer)
		pUnit->QueueMission(Mission::Guard, true);

	// Restore overridden instructions
	GET(Mission, eax, EAX);
	return eax == Mission::Construction ? 0x4C8109 : 0x4C7521;
}

// issue #250: Building placement hotkey not responding
// Author: Uranusian
DEFINE_JUMP(LJMP,0x4ABBD5, 0x4ABBD5 + 7); // DisplayClass_MouseLeftRelease_HotkeyFix

DEFINE_HOOK(0x4FB2DE, HouseClass_PlaceObject_HotkeyFix, 0x6)
{
	GET(TechnoClass*, pObject, ESI);

	pObject->ClearSidebarTabObject();

	return 0;
}

// Issue #46: Laser is mirrored relative to FireFLH
// Author: Starkku
DEFINE_HOOK(0x6FF2BE, TechnoClass_FireAt_BurstOffsetFix_1, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	--pThis->CurrentBurstIndex;

	return 0x6FF2D1;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_BurstOffsetFix_2, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_BASE(int, weaponIndex, 0xC);

	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pThis->GetWeapon(weaponIndex)->WeaponType->Burst;

	return 0;
}

// issue #290: Undeploy building into a unit plays EVA_NewRallyPointEstablished
// Author: secsome
DEFINE_HOOK(0x44377E, BuildingClass_ActiveClickWith, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(CellStruct*, pCell, STACK_OFFS(0x84, -0x8));

	if (pThis->GetTechnoType()->UndeploysInto)
		pThis->SetRallypoint(pCell, false);
	else if (pThis->IsUnitFactory())
		pThis->SetRallypoint(pCell, true);

	return 0x4437AD;
}

// issue #232: Naval=yes overrides WaterBound=no and prevents move orders onto Land cells
// Author: Uranusian
DEFINE_JUMP(LJMP,0x47CA05, 0x47CA33); // CellClass_IsClearToBuild_SkipNaval

// bugfix: DeathWeapon not properly detonates
// Author: Uranusian
DEFINE_HOOK(0x70D77F, TechnoClass_FireDeathWeapon_ProjectileFix, 0x8)
{
	GET(BulletClass*, pBullet, EBX);
	GET(CoordStruct*, pCoord, EAX);

	pBullet->SetLocation(*pCoord);
	pBullet->Explode(true);

	return 0x70D787;
}

// Fix [JumpjetControls] obsolete in RA2/YR
// Author: Uranusian
DEFINE_HOOK(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	const auto pRules = RulesClass::Instance();
	const auto pRulesExt = RulesExt::Global();

	pThis->JumpjetTurnRate = pRules->TurnRate;
	pThis->JumpjetSpeed = pRules->Speed;
	pThis->JumpjetClimb = static_cast<float>(pRules->Climb);
	pThis->JumpjetCrash = static_cast<float>(pRulesExt->JumpjetCrash.Get());
	pThis->JumpjetHeight = pRules->CruiseHeight;
	pThis->JumpjetAccel = static_cast<float>(pRules->Acceleration);
	pThis->JumpjetWobbles = static_cast<float>(pRules->WobblesPerSecond);
	pThis->JumpjetNoWobbles = pRulesExt->JumpjetNoWobbles.Get();
	pThis->JumpjetDeviation = pRules->WobbleDeviation;

	return 0x711601;
}

// skip vanilla JumpjetControls and make it earlier load
DEFINE_JUMP(LJMP,0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls

DEFINE_HOOK(0x52D0F9, InitRules_EarlyLoadJumpjetControls, 0x6)
{
	GET(RulesClass*, pThis, ECX);
	GET(CCINIClass*, pINI, EAX);

	RulesExt::LoadEarlyBeforeColor(pThis, pINI);
	pThis->Read_JumpjetControls(pINI);

	return 0;
}

DEFINE_HOOK(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	if (const auto pRulesExt = RulesExt::Global())
	{
		GET(CCINIClass*, pINI, EDI);

		INI_EX exINI(pINI);

		pRulesExt->JumpjetCrash.Read(exINI, "JumpjetControls", "Crash");
		pRulesExt->JumpjetNoWobbles.Read(exINI, "JumpjetControls", "NoWobbles");
	}

	return 0;
}

// Fix the crash of TemporalTargetingMe related "stack dump starts with 0051BB7D"
// Author: secsome
DEFINE_HOOK_AGAIN(0x43FCF9, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // BuildingClass
DEFINE_HOOK_AGAIN(0x414BDB, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // AircraftClass
DEFINE_HOOK_AGAIN(0x736204, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // UnitClass
DEFINE_HOOK(0x51BB6E, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // InfantryClass
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->TemporalTargetingMe) {
		// Also check for vftable here to guarantee the TemporalClass not being destoryed already.
		if (((int*)pThis->TemporalTargetingMe)[0] == 0x7F5180)
			pThis->TemporalTargetingMe->Update();
		else // It should had being warped out, delete this object
		{
			pThis->TemporalTargetingMe = nullptr;
			pThis->Limbo();
			pThis->UnInit();
		}
	}

	return R->Origin() + 0xF;
}

// Fix the issue that AITriggerTypes do not recognize building upgrades
// Author: Uranusian
DEFINE_HOOK_AGAIN(0x41EEE3, AITriggerTypeClass_Condition_SupportPowersup, 0x7)	//AITriggerTypeClass_OwnerHouseOwns_SupportPowersup
DEFINE_HOOK(0x41EB43, AITriggerTypeClass_Condition_SupportPowersup, 0x7)		//AITriggerTypeClass_EnemyHouseOwns_SupportPowersup
{
	GET(HouseClass*, pHouse, EDX);
	GET(int, idxBld, EBP);
	auto const pType = BuildingTypeClass::Array->Items[idxBld];
	int count = BuildingTypeExt::GetUpgradesAmount(pType, pHouse);

	if (count == -1)
		count = pHouse->ActiveBuildingTypes.GetItemCount(idxBld);

	R->EAX(count);

	return R->Origin() + 0xC;
}

// Dehardcode the stupid Wall-Gate relationships
// Author: Uranusian
DEFINE_HOOK(0x441053, BuildingClass_Unlimbo_EWGate, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	return RulesClass::Instance->EWGates.FindItemIndex(pThis) == -1 ? 0 : 0x441065;
}

DEFINE_HOOK(0x4410E1, BuildingClass_Unlimbo_NSGate, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	return RulesClass::Instance->NSGates.FindItemIndex(pThis) == -1 ? 0 : 0x4410F3;
}

DEFINE_HOOK(0x480552, CellClass_AttachesToNeighbourOverlay_Gate, 0x7)
{
	GET(CellClass*, pThis, EBP);
	GET(int, idxOverlay, EBX);
	GET_STACK(int, state, STACK_OFFS(0x10, -0x8));
	const bool isWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	enum { Attachable = 0x480549 };

	if (isWall) {
		for (auto pObject = pThis->FirstObject; pObject; pObject = pObject->NextObject) {
			if (pObject->Health > 0) {
				if (const auto pBuilding = specific_cast<BuildingClass*>(pObject)) {
					const auto pBType = pBuilding->Type;

					if ((RulesClass::Instance->EWGates.FindItemIndex(pBType) != -1) && (state == 2 || state == 6))
						return Attachable;
					else if ((RulesClass::Instance->NSGates.FindItemIndex(pBType) != -1) && (state == 0 || state == 4))
						return Attachable;
					else if (RulesExt::Global()->WallTowers.Contains(pBType))
						return Attachable;
				}
			}
		}
	}

	return 0;
}

// WW take 1 second as 960 milliseconds, this will fix that back to the actual time.
// Author: secsome
DEFINE_HOOK(0x6C919F, StandaloneScore_SinglePlayerScoreDialog_ActualTime, 0x5)
{
	R->ECX(static_cast<int>(std::round(R->ECX() * 0.96)));
	return 0;
}

// Fix the issue that SHP units doesn't apply IronCurtain or other color effects and doesn't accept EMP intensity
// Author: secsome
DEFINE_HOOK(0x706389, TechnoClass_DrawAsSHP_TintAndIntensity, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nIntensity, EBP);
	REF_STACK(int, nTintColor, STACK_OFFS(0x54, -0x2C));

	bool NeedUpdate = false;
	if (pThis->IsIronCurtained())
	{
		if(pThis->ForceShielded != 1)
			nTintColor |= Drawing::RGB2DWORD(RulesGlobal->ColorAdd[RulesGlobal->IronCurtainColor]);
		else
			nTintColor |= Drawing::RGB2DWORD(RulesGlobal->ColorAdd[RulesGlobal->ForceShieldColor]);

		NeedUpdate = true;
	}

	if (pThis->Berzerk)
	{
		nTintColor |= Drawing::RGB2DWORD(RulesGlobal->ColorAdd[RulesGlobal->BerserkColor]);
		NeedUpdate = true;
	}


	// Boris
	if (pThis->Airstrike && pThis->Airstrike->Target == pThis){
		nTintColor |= Drawing::RGB2DWORD(RulesGlobal->ColorAdd[RulesGlobal->LaserTargetColor]);
		NeedUpdate = true;
	}
	// EMP
	if (pThis->Deactivated){
		R->EBP(nIntensity / 2);
		NeedUpdate = true;
	}

	if (pThis->WhatAmI() == AbstractType::Building && NeedUpdate)
		BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LighningNeedUpdate = true;

	return 0;
}

// Fixed the bug that units' lighting get corrupted after loading a save with a different lighting being set
// Author: secsome
DEFINE_HOOK(0x67E6E5, LoadGame_RecalcLighting, 0x7)
{
	ScenarioClass::Instance->RecalcLighting(
		ScenarioClass::Instance->NormalLighting.Tint.Red * 10,
		ScenarioClass::Instance->NormalLighting.Tint.Green * 10,
		ScenarioClass::Instance->NormalLighting.Tint.Blue * 10,
		0
	);

	return 0;
}

DEFINE_HOOK(0x6FA781, TechnoClass_AI_SelfHealing_BuildingGraphics, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
		if(pBuilding->IsThisBreathing()){
			pBuilding->UpdatePlacement(PlacementType::Redraw);
			pBuilding->ToggleDamagedAnims(false);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CDA78, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ESI);

	if (auto const pLinked = pThis->LinkedTo) {
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(Game::F2I(currentSpeed));
	}

	return 0;
}

DEFINE_HOOK(0x4CE4BF, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	if(auto const pLinked = pThis->LinkedTo){
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(Game::F2I(currentSpeed));
	}

	return 0;
}

DEFINE_HOOK(0x54D138, JumpjetLocomotionClass_Movement_AI_SpeedModifiers, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (auto const pLinked = pThis->LinkedTo) {
		const double multiplier = TechnoExt::GetCurrentSpeedMultiplier(pLinked);
		pThis->Speed = Game::F2I(pLinked->GetTechnoType()->JumpjetSpeed * multiplier);
	}

	return 0;
}

DEFINE_HOOK(0x73B2A2, UnitClass_DrawObject_DrawerBlitterFix, 0x6)
{
	enum { SkipGameCode = 0x73B2C3 };

	GET(UnitClass* const, pThis, ESI);
	GET(BlitterFlags, blitterFlags, EDI);

	R->EAX(pThis->GetDrawer()->Select_Blitter(blitterFlags));

	return SkipGameCode;
}

DEFINE_HOOK(0x710021, FootClass_ImbueLocomotor_SpawnRate, 0x5)
{
	GET(SpawnManagerClass*, pManager, ECX);

	pManager->KillNodes();

	for (const auto& nNodes : pManager->SpawnedNodes) {
		nNodes->SpawnTimer.Start(pManager->RegenRate);
	}

	return 0x710026;
}

// Mitigate DeploysInto vehicles getting stuck trying to deploy while using deploy AI script action
DEFINE_HOOK(0x6ED6E5, TeamClass_TMission_Deploy_DeploysInto, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	// Handle searching for free space in Hunt mission handler
	pThis->ForceMission(Mission::Hunt);
	pThis->MissionStatus = 2; // Tells UnitClass::Mission_Hunt to omit certain checks.

	return 0;
}

DEFINE_HOOK(0x73EFD8, UnitClass_Mission_Hunt_DeploysInto, 0x6)
{
	enum { SkipToDeploy = 0x73F015 };

	GET(UnitClass*, pThis, ESI);

	// Skip MCV-specific & player control checks if coming from deploy script action.
	switch (pThis->MissionStatus)
	{
	case 1: {
		if (const auto conyType = pThis->Type->DeploysInto) {
			if (conyType->ConstructionYard || conyType->WeaponsFactory)
				pThis->QueueMission(Mission::Guard, false);
		}
	}
		  break;
	case 2: {
		pThis->MissionStatus = 0;
		return SkipToDeploy;
	}
	}
	return 0;
}

// Fixes an issue in TechnoClass::Record_The_Kill that prevents vehicle kills from being recorded
// correctly if killed by damage that has owner house but no owner techno (animation warhead damage, radiation with owner etc.
// Author: Starkku (modified by Otamaa)
DEFINE_JUMP(LJMP,0x7032BC, 0x7032D0); //this was checking (IsActive) twice , wtf

// Fix unit will play crash voice when crashing after attacked by locomotor warhead
// Author : NetsuNegi
DEFINE_HOOK(0x4DACDD, FootClass_CrashingVoice, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (pThis->IsCrashing != pThis->WasCrashingAlready)
	{
		if (pThis->IsCrashing)
		{
			pThis->Audio7.ShutUp();
			auto const nCoord = pThis->GetCoords();

			if(!pThis->IsAttackedByLocomotor){
				const auto pType = pThis->GetTechnoType();

				if (pType->VoiceCrashing != -1 && pThis->Owner->IsControlledByCurrentPlayer())
					VocClass::PlayAt(pType->VoiceCrashing, nCoord);

				if (pType->CrashingSound != -1)
					VocClass::PlayAt(pType->CrashingSound, nCoord, &pThis->Audio7);

			} else {
				VocClass::PlayAt(RulesGlobal->ScoldSound, nCoord, &pThis->Audio7);
			}


		}
		else if (pThis->__PlayingMovingSound) // done playing
			pThis->Audio7.ShutUp();

		pThis->WasCrashingAlready = pThis->IsCrashing;
	}

	return 0x4DADC8;
}

DEFINE_HOOK(0x456776, BuildingClass_DrawRadialIndicator_Visibility, 0x6)
{
	enum { ContinueDraw = 0x456789, DoNotDraw = 0x456962 };
	GET(BuildingClass* const, pThis, ESI);

	if(HouseExt::IsObserverPlayer()) {
		return ContinueDraw;
	}

	if (BuildingExt::ExtMap.Find(pThis)->IsInLimboDelivery) {
		return DoNotDraw;
	}

	auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pBldTypeExt->RadialIndicator_Visibility.isset()) {
		if(EnumFunctions::CanTargetHouse(pBldTypeExt->RadialIndicator_Visibility.Get(), pThis->Owner, HouseClass::CurrentPlayer()))
			return ContinueDraw;
	} else {
		if (pThis->Owner && pThis->Owner == HouseClass::CurrentPlayer())
			return ContinueDraw;

	}

	return DoNotDraw;
}