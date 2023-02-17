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
#include <Ext/House/Body.h>

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
DEFINE_JUMP(LJMP, 0x545CE2, 0x545CE9) //Phobos_BugFixes_Tileset255_RemoveNonMMArrayFill
DEFINE_JUMP(LJMP, 0x546C23, 0x546C8B) //Phobos_BugFixes_Tileset255_RefNonMMArray

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
		if ((pThis->IsSinking || (!pThis->IsAttackedByLocomotor && pThis->IsCrashing)))
			R->EAX(DamageState::PostMortem);

	return 0;
}

DEFINE_HOOK(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (result == DamageState::NowDead)
	{

		if (pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
			pThis->IsAttackedByLocomotor = false;

#ifndef COMPILE_PORTED_DP_FEATURES
		if (!pThis->Type->Voxel)
		{
			if (pThis->Type->MaxDeathCounter > 0
				&& !pThis->InLimbo
				&& !pThis->IsCrashing
				&& !pThis->IsSinking
				&& !pThis->TemporalTargetingMe
				&& !pThis->IsInAir()
				&& pThis->DeathFrameCounter <= 0
				)
			{

				pThis->Stun();
				if (pThis->Locomotor->Is_Moving_Now())
					pThis->Locomotor->Stop_Moving();

				pThis->DeathFrameCounter = 1;
			}
		}
#endif
	}

	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
	{
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
		auto nCoords = pThis->GetCoords();

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
DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBD5 + 7); // DisplayClass_MouseLeftRelease_HotkeyFix

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
DEFINE_JUMP(LJMP, 0x47CA05, 0x47CA33); // CellClass_IsClearToBuild_SkipNaval

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

// Fix the crash of TemporalTargetingMe related "stack dump starts with 0051BB7D"
// Author: secsome
DEFINE_HOOK_AGAIN(0x43FCF9, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // BuildingClass
DEFINE_HOOK_AGAIN(0x414BDB, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // AircraftClass
DEFINE_HOOK_AGAIN(0x736204, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // UnitClass
DEFINE_HOOK(0x51BB6E, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // InfantryClass
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->TemporalTargetingMe)
	{
		// Also check for vftable here to guarantee the TemporalClass not being destoryed already.
		if (((int*)pThis->TemporalTargetingMe)[0] == 0x7F5180)
			pThis->TemporalTargetingMe->Update();
		else // It should had being warped out, delete this object
		{
			pThis->TemporalTargetingMe = nullptr;
			pThis->Limbo();
			TechnoExt::HandleRemove(pThis);
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

	if (isWall)
	{
		for (auto pObject = pThis->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->Health > 0)
			{
				if (const auto pBuilding = specific_cast<BuildingClass*>(pObject))
				{
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
		if (pThis->ForceShielded != 1)
			nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesGlobal->IronCurtainColor);
		else
			nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesGlobal->ForceShieldColor);

		NeedUpdate = true;
	}

	if (pThis->Berzerk)
	{
		nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesGlobal->BerserkColor);
		NeedUpdate = true;
	}


	// Boris
	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
	{
		nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesGlobal->LaserTargetColor);
		NeedUpdate = true;
	}
	// EMP
	if (pThis->Deactivated)
	{
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

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		if (pBuilding->IsThisBreathing())
		{
			pBuilding->UpdatePlacement(PlacementType::Redraw);
			pBuilding->ToggleDamagedAnims(false);
		}
	}

	return 0;
}

//DEFINE_HOOK(0x4CDA78, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x6)
//{
//	GET(FlyLocomotionClass*, pThis, ESI);
//
//	if (auto const pLinked = pThis->LinkedTo)
//	{
//		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
//			TechnoExt::GetCurrentSpeedMultiplier(pLinked);
//
//		R->EAX(Game::F2I(currentSpeed));
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x4CE4BF, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
//{
//	GET(FlyLocomotionClass*, pThis, ECX);
//
//	if (auto const pLinked = pThis->LinkedTo)
//	{
//		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
//			TechnoExt::GetCurrentSpeedMultiplier(pLinked);
//
//		R->EAX(Game::F2I(currentSpeed));
//	}
//
//	return 0;
//}

DEFINE_HOOK(0x4CDA6F, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x9)
{
	GET(FlyLocomotionClass*, pThis, ESI);

	if (auto const pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(Game::F2I(currentSpeed));
		return 0x4CDA78;
	}

	return 0;
}

DEFINE_HOOK(0x4CE4B3, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	if (auto const pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(Game::F2I(currentSpeed));
		return 0x4CE4BF;
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

	for (const auto& nNodes : pManager->SpawnedNodes)
	{
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
	case 1:
	{
		if (const auto conyType = pThis->Type->DeploysInto)
		{
			if (conyType->ConstructionYard || conyType->WeaponsFactory)
				pThis->QueueMission(Mission::Guard, false);
		}
	}
	break;
	case 2:
	{


		if (pThis->Type->Category == Category::Support && !pThis->IsOwnedByCurrentPlayer)
		{
			pThis->QueueMission(Mission::Guard, false);
		}
		else
		{
			pThis->MissionStatus = 0;
		}

		return SkipToDeploy;
	}
	}
	return 0;
}

// Fixes an issue in TechnoClass::Record_The_Kill that prevents vehicle kills from being recorded
// correctly if killed by damage that has owner house but no owner techno (animation warhead damage, radiation with owner etc.
// Author: Starkku (modified by Otamaa)
DEFINE_JUMP(LJMP, 0x7032BC, 0x7032D0); //this was checking (IsActive) twice , wtf

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

			if (!pThis->IsAttackedByLocomotor)
			{
				const auto pType = pThis->GetTechnoType();

				if (pType->VoiceCrashing != -1 && pThis->Owner->IsControlledByCurrentPlayer())
					VocClass::PlayAt(pType->VoiceCrashing, nCoord);

				if (pType->CrashingSound != -1)
					VocClass::PlayAt(pType->CrashingSound, nCoord, &pThis->Audio7);

			}
			else
			{
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

	if (HouseExt::IsObserverPlayer())
	{
		return ContinueDraw;
	}

	if (BuildingExt::ExtMap.Find(pThis)->LimboID != -1)
	{
		return DoNotDraw;
	}

	const auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pBldTypeExt->RadialIndicator_Visibility.isset())
	{
		if (EnumFunctions::CanTargetHouse(pBldTypeExt->RadialIndicator_Visibility.Get(), pThis->Owner, HouseClass::CurrentPlayer()))
			return ContinueDraw;
	}
	else
	{
		if (pThis->Owner && pThis->Owner == HouseClass::CurrentPlayer())
			return ContinueDraw;

	}

	return DoNotDraw;
}

// Bugfix: TAction 7,80,107.
DEFINE_HOOK(0x65DF81, TeamTypeClass_CreateMembers_WhereTheHellIsIFV, 0x7)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);

	const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;

	for (auto pNext = pPayload; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		if (pNext && pNext != pTransport && pNext->Team == pTeam)
		{
			pNext->Transporter = pTransport;
			if (isTransportOpenTopped)
				pTransport->EnteredOpenTopped(pNext);
		}
	}

	pPayload->SetLocation(pTransport->Location);
	pTransport->AddPassenger(pPayload); // ReceiveGunner is done inside FootClass::AddPassenger
	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}

// BibShape checks for BuildingClass::BState which needs to not be 0 (constructing) for bib to draw.
// It is possible for BState to be 1 early during construction for frame or two which can result in BibShape being drawn during buildup, which somehow depends on length of buildup.
// Trying to fix this issue at its root is problematic and most of the time causes buildup to play twice, it is simpler to simply fix the BibShape to not draw until the buildup is done - Starkku
DEFINE_HOOK(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
{
	enum { DontDrawBib = 0x43D8EE };

	GET(BuildingClass*, pThis, ESI);
	return !pThis->ActuallyPlacedOnMap ? DontDrawBib : 0x0;
}

//TODO : finish this instead hacky way below 
//static CoordStruct* UnitClass_GetFLH(UnitClass* pThis , void* , CoordStruct* pBuffer , int nWpIdx , CoordStruct nFrom)
//{
//	if(pThis->InOpenToppedTransport && pThis->Transporter &&)
//}

//DEFINE_HOOK(0x6FDDCA, TechnoClass_Fire_Suicide, 0xA)
//{
//	GET(TechnoClass* const, pThis, ESI);
//
//	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
//		nullptr, true, false, pThis->Owner);
//
//	return 0x6FDE03;
//}
//

// Kill the vxl unit when flipped over
//DEFINE_HOOK(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
//{
//	GET(TechnoClass* const, pThis, ESI);
//
//	auto const pFlipper = pThis->DirectRockerLinkedUnit;
//	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
//		nullptr, true, false, pFlipper ? pFlipper->Owner : nullptr);
//
//	return 0x70BCA4;
//}

//DEFINE_HOOK(0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6)
//{
//	GET(TechnoClass* const, pRadio, EAX);
//
//	pRadio->ReceiveDamage(&pRadio->Health, 0, RulesClass::Instance->C4Warhead,
//		nullptr, true, true, nullptr);
//
//	return 0x4425F4;
//}
//
//DEFINE_HOOK(0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA)
//{
//	GET(TechnoClass* const, pItem, ESI);
//
//	pItem->ReceiveDamage(&pItem->Health, 0, RulesClass::Instance->C4Warhead,
//		nullptr, true, true, nullptr);
//
//	return 0x50150E;
//}

// Something unfinished for later
//DEFINE_HOOK(0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8)
//{
//	GET(FootClass* const, pOwner, ECX);
//	pOwner->ReceiveDamage(&pOwner->Health, 0, RulesClass::Instance->C4Warhead,
//		nullptr, true, false, nullptr);
//	return 0x71880A;
//}
//718B1E

DEFINE_HOOK(0x4DE652, FootClass_AddPassenger_NumPassengerGeq0, 0x7)
{
	enum { GunnerReception = 0x4DE65B, EndFuntion = 0x4DE666 };
	GET(FootClass* const, pThis, ESI);
	// Replace NumPassengers==1 check to allow multipassenger IFV using the fix above
	return pThis->Passengers.NumPassengers > 0 ? GunnerReception : EndFuntion;
}

//template<typename T>
//static bool NOINLINE InvalidateVector(DynamicVectorClass<T>& nVec, T pItem)
//{
//	auto datafirst = std::addressof(nVec.Items[0]);
//	auto dataend = std::addressof(nVec.Items[nVec.Count]);
//
//	if (datafirst != dataend)
//	{
//		while (*datafirst != pItem)
//		{
//			if (datafirst == dataend)
//				return false;
//		}
//
//		std::memmove(datafirst, datafirst + 1, dataend - (datafirst + 1));
//		--nVec.Count;
//		return true;
//	}
//
//	return false;
//}

DEFINE_HOOK(0x440E99, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x6)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);

	return pThis->BeingProduced ? 0 : DoNotCreateParticle;
}

//https://github.com/Phobos-developers/Phobos/pull/818
DEFINE_HOOK(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));

	const int XP = 2 * MapClass::Instance->VisibleRect.X - MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance->VisibleRect.Width);
	const int YP = 2 * MapClass::Instance->VisibleRect.Y + MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance->VisibleRect.Height + 2);
	cell = { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };

	auto pCell = MapClass::Instance->TryGetCellAt(cell);
	if (!pCell)
		return SkipSpawn;

	if (!MapClass::Instance->IsWithinUsableArea(pCell, true))
		return SkipSpawn;

	const bool isWater = pCell->LandType == LandType::Water;
	if (isWater && RulesExt::Global()->Crate_LandOnly.Get())
		return SkipSpawn;

	cell = MapClass::Instance->NearByLocation(pCell->MapCoords,
		isWater ? SpeedType::Float : SpeedType::Track,
		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	R->EAX(&cell);

	return SpawnCrate;
}

//DEFINE_HOOK(0x56BDE7, MapClass_PlaceRandomCrate_GenerationMechanism, 0x6)
//{
//	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };
//
//	GET(CellStruct, candidate, ECX);
//	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));
//
//	if (!MapClass::Instance->IsWithinUsableArea(candidate, true))
//		return SkipSpawn;
//
//	const auto pCell = MapClass::Instance->TryGetCellAt(candidate);
//
//	const bool isWater = pCell->LandType == LandType::Water;
//	if (isWater && RulesExt::Global()->Crate_LandOnly.Get())
//		return SkipSpawn;
//
//	cell = MapClass::Instance->NearByLocation(pCell->MapCoords,
//		isWater ? SpeedType::Float : SpeedType::Track,
//		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
//
//	R->EAX(&cell);
//
//	return SpawnCrate;
//}


DEFINE_HOOK_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)
DEFINE_HOOK(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	if (BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1)
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;

	return 0;
}

DEFINE_HOOK(0x519F84, InfantryClass_UpdatePosition_EngineerPreUninit, 0x6)
{
	GET(TechnoClass*, pBld, EDI);

	if (auto pBy = pBld->MindControlledBy)
		pBy->CaptureManager->FreeUnit(pBld);

	if (auto& pAnim = pBld->MindControlRingAnim)
	{
		pAnim->UnInit();
		pAnim = nullptr;
	}

	std::exchange(pBld->MindControlledByAUnit, false);

	return 0;
}

// Enable sorted add for Air/Top layers to fix issues with attached anims etc.
DEFINE_HOOK(0x4A9750, DisplayClass_Submit_LayerSort, 0x9)
{
	GET(Layer, layer, EDI);
	R->ECX(layer != Layer::Surface && layer != Layer::Underground);
	return 0;
}

//DEFINE_HOOK(0x56BD8B, MapClass_PlaceCrate_InMapFix, 0x5)
//{
//	enum { CreateCrate = 0x56BE7B, DontCreate = 0x56BE91 };
//
//	const int MapX = MapClass::Instance->MapRect.Width * 60;
//	const int MapY = MapClass::Instance->MapRect.Height * 30;
//	const int X = ScenarioClass::Instance->Random.RandomRanged(0, MapX) - MapX / 2;
//	const int Y = ScenarioClass::Instance->Random.RandomRanged(0, MapY) + MapY / 2;
//
//	const auto result = Matrix3D::MatrixMultiply(TacticalClass::Instance->IsoTransformMatrix, { (float)X,(float)Y,0.0f });
//
//	if (const auto pCell = MapClass::Instance->TryGetCellAt({ (int)result.X,(int)result.Y,0 }))
//	{
//		REF_STACK(CellStruct, cell, STACK_OFFS(0x28, 0x18));
//
//		const auto SpeedType = pCell->LandType == LandType::Water ? SpeedType::Float : SpeedType::Track;
//		cell = MapClass::Instance->NearByLocation(pCell->MapCoords, SpeedType, -1, MovementZone::Normal, false, 1, 1, false,
//		false, false, true, CellStruct::Empty, false, false);
//
//		R->EAX(&cell);
//		return CreateCrate;
//	}
//
//	return DontCreate;
//}
//DEFINE_HOOK(0x65AAC0, RadioClass_Detach, 0x5)
//{
//	GET(RadioClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	pThis->ObjectClass::PointerExpired(pTarget, bRemoved);
//	auto datafirst = std::addressof(pThis->RadioLinks.Items[0]);
//	auto dataend = std::addressof(pThis->RadioLinks.Items[pThis->RadioLinks.Capacity]);
//
//	if (datafirst != dataend)
//	{
//		while (*datafirst != pTarget)
//		{
//			if (datafirst == dataend)
//				break;
//		}
//
//		std::memmove(datafirst, datafirst + 1, dataend - (datafirst + 1));
//		--pThis->RadioLinks.Capacity;
//	}
//
//	return 0x65AB08;
//}
