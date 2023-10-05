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
#include <TemporalClass.h>
#include <CellClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/EnumFunctions.h>

#include <Locomotor/Cast.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(0x423365, AnimClass_SHPShadowCheck, 0x8)
{
	GET(AnimClass* const, pAnim, ESI);
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

	GET(int const, health, EAX);
	GET(ObjectClass* const, pThis, ESI);

	if (health <= 0 || !pThis->IsAlive)
		return PostMortem;

	return ContinueCheck;
}

DEFINE_HOOK(0x4D7431, FootClass_ReceiveDamage_DyingFix, 0x5)
{
	GET(FootClass* const, pThis, ESI);
	GET(DamageState const, result, EAX);

	if (result != DamageState::PostMortem)
		if ((pThis->IsSinking || (!pThis->IsAttackedByLocomotor && pThis->IsCrashing)))
			R->EAX(DamageState::PostMortem);

	return 0;
}

DEFINE_HOOK(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(DamageState const, result, EAX);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (result == DamageState::NowDead)
	{

		if (pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
			pThis->IsAttackedByLocomotor = false;

#ifdef COMPILE_PORTED_DP_FEATURES
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

// Restore DebrisMaximums logic (issue #109)
// Author: Otamaa
DEFINE_HOOK(0x702299, TechnoClass_ReceiveDamage_DebrisMaximumsFix, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pType = pThis->GetTechnoType();

	// If DebrisMaximums has one value, then legacy behavior is used
	//if (pType->DebrisMaximums.Count == 1 &&
	//	pType->DebrisMaximums.GetItem(0) > 0 &&
	//	pType->DebrisTypes.Count > 0)
	//{
	//	return 0;
	//}
	auto totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(pType->MinDebris, pType->MaxDebris);

	if (totalSpawnAmount && pType->DebrisTypes.Count > 0 && pType->DebrisMaximums.Count > 0)
	{
		auto nCoords = pThis->GetCoords();

		for (int currentIndex = 0; currentIndex < pType->DebrisTypes.Count; ++currentIndex)
		{
			if (currentIndex >= pType->DebrisMaximums.Count)
				break;

			if (!pType->DebrisMaximums[currentIndex])
				continue;

			//this never goes to 0
			int amountToSpawn = (abs(int(ScenarioClass::Instance->Random.Random())) % pType->DebrisMaximums[currentIndex]) + 1;
			amountToSpawn = LessOrEqualTo(amountToSpawn, totalSpawnAmount);
			totalSpawnAmount -= amountToSpawn;

			for (; amountToSpawn > 0; --amountToSpawn)
			{
				if (auto pVoxAnim = GameCreate<VoxelAnimClass>(pType->DebrisTypes.GetItem(currentIndex),
					&nCoords, pThis->Owner))
				{
					if (auto pExt = VoxelAnimExt::ExtMap.FindOrAllocate(pVoxAnim))
						pExt->Invoker = pThis;
				}
			}

			if (totalSpawnAmount <= 0)
			{
				totalSpawnAmount = 0;
				break;
			}
		}
	}

	// debrisanim has no owner , duh
	R->EBX(totalSpawnAmount);

	return 0x7023E5;
}

// issue #250: Building placement hotkey not responding
// Author: Uranusian
//DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBD5 + 7); // DisplayClass_MouseLeftRelease_HotkeyFix
DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBDC);

DEFINE_HOOK(0x4FB2DE, HouseClass_PlaceObject_HotkeyFix, 0x6)
{
	GET(TechnoClass* const, pObject, ESI);

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

// issue #290: Undeploy building into a unit plays EVA_NewRallyPointEstablished
// Author: secsome
DEFINE_HOOK(0x44377E, BuildingClass_ActiveClickWith, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(CellStruct*, pCell, STACK_OFFS(0x84, -0x8));

	if (pThis->GetTechnoType()->UndeploysInto)
		pThis->SetRallypoint(pCell, false);
	else if (pThis->IsUnitFactory())
		pThis->SetRallypoint(pCell, true);

	return 0x4437AD;
}

// issue #232: Naval=yes overrides WaterBound=no and prevents move orders onto Land cells
// Author: Uranusian
DEFINE_JUMP(LJMP, 0x47CA05, 0x47CA33);

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

bool NOINLINE IsTemporalptrValid(TemporalClass* pThis)
{
	return VTable::Get(pThis) == TemporalClass::vtable;
}

DWORD NOINLINE TechnoClass_AI_TemporalTargetingMe_Fix(REGISTERS* R ,AbstractType)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->TemporalTargetingMe)
	{
		// Also check for vftable here to guarantee the TemporalClass not being destoryed already.
		if (IsTemporalptrValid(pThis->TemporalTargetingMe))
		{
			if(pThis->TemporalTargetingMe->Owner)
				pThis->TemporalTargetingMe->Update();
			else
			{
				GameDelete<true, false>(pThis->TemporalTargetingMe);
				pThis->TemporalTargetingMe = nullptr;
			}
		}
		else // It should had being warped out, delete this object
		{
			pThis->TemporalTargetingMe = nullptr;

			if (pThis->IsAlive)
			{
				pThis->Limbo();
				Debug::Log(__FUNCTION__" Called \n");
				TechnoExt::HandleRemove(pThis, nullptr, true, false);
			}
			else
			{
				switch (R->Origin())
				{
				case 0x51BB6E:
					return 0x51BF80;
				case 0x736204:
					return 0x7363C1;
				case 0x414BDB:
					return 0x414E25;
				case 0x43FCF9:
					return 0x440573;
				}
			}
		}
	}

	return R->Origin() + 0xF; //skip updating the temporal
}
// Fix the crash of TemporalTargetingMe related "stack dump starts with 0051BB7D"
// Author: secsome
DEFINE_HOOK(0x43FCF9, BuildingClass_AI_TemporalTargetingMe, 0x6) // BuildingClass
{
	return TechnoClass_AI_TemporalTargetingMe_Fix(R , BuildingClass::AbsID);
}

DEFINE_HOOK(0x414BDB, AircraftClass_AI_TemporalTargetingMe, 0x6) //
{
	return TechnoClass_AI_TemporalTargetingMe_Fix(R, AircraftClass::AbsID);
}

DEFINE_HOOK(0x736204, UnitClass_AI_TemporalTargetingMe, 0x6) //
{
	return TechnoClass_AI_TemporalTargetingMe_Fix(R, UnitClass::AbsID);
}

DEFINE_HOOK(0x51BB6E, InfantryClass_AI_TemporalTargetingMe_Fix, 0x6) //
{
	return TechnoClass_AI_TemporalTargetingMe_Fix(R, InfantryClass::AbsID);
}

// Fix the issue that AITriggerTypes do not recognize building upgrades
// Author: Uranusian
DEFINE_HOOK_AGAIN(0x41EEE3, AITriggerTypeClass_Condition_SupportPowersup, 0x7)	//AITriggerTypeClass_OwnerHouseOwns_SupportPowersup
DEFINE_HOOK(0x41EB43, AITriggerTypeClass_Condition_SupportPowersup, 0x7)		//AITriggerTypeClass_EnemyHouseOwns_SupportPowersup
{
	GET(HouseClass* const, pHouse, EDX);
	GET(int const, idxBld, EBP);

	const auto pType = BuildingTypeClass::Array->Items[idxBld];
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
	GET(BuildingTypeClass* const, pThis, ECX);

	return RulesClass::Instance->EWGates.FindItemIndex(pThis) == -1 ? 0 : 0x441065;
}

DEFINE_HOOK(0x4410E1, BuildingClass_Unlimbo_NSGate, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);

	return RulesClass::Instance->NSGates.FindItemIndex(pThis) == -1 ? 0 : 0x4410F3;
}

DEFINE_HOOK(0x480552, CellClass_AttachesToNeighbourOverlay_Gate, 0x7)
{
	GET(CellClass* const, pThis, EBP);
	GET(int const, idxOverlay, EBX);
	GET_STACK(int const, state, STACK_OFFS(0x10, -0x8));

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
	GET(TechnoClass* const, pThis, ESI);
	REF_STACK(int, nTintColor, STACK_OFFS(0x54, -0x2C));

	BuildingClass* pBld = nullptr;
	if(pThis->WhatAmI() == BuildingClass::AbsID )
		pBld = (BuildingClass*)pThis;

	if (pBld) {
		if ((pBld->CurrentMission == Mission::Construction)
			&& pBld->BState == BStateType::Construction && pBld->Type->Buildup ) {
			if (BuildingTypeExt::ExtMap.Find(pBld->Type)->BuildUp_UseNormalLIght.Get()) {
				R->EBP(1000);
			}
		}
	}

	GET(int const, nIntensity, EBP);
	bool NeedUpdate = false;
	if (pThis->IsIronCurtained())
	{
		if (pThis->ProtectType == ProtectTypes::IronCurtain)
			nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->IronCurtainColor);
		else
			nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->ForceShieldColor);

		NeedUpdate = true;
	}

	if (pThis->Berzerk)
	{
		nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->BerserkColor);
		NeedUpdate = true;
	}


	// Boris
	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
	{
		nTintColor |= GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->LaserTargetColor);
		NeedUpdate = true;
	}
	// EMP
	if (pThis->Deactivated)
	{
		R->EBP(nIntensity / 2);
		NeedUpdate = true;
	}

	if (pBld && NeedUpdate)
		BuildingExt::ExtMap.Find(pBld)->LighningNeedUpdate = true;

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

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		if (pBuilding->IsThisBreathing())
		{
			pBuilding->UpdatePlacement(PlacementType::Redraw);
			pBuilding->ToggleDamagedAnims(false);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CDA6F, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x9)
{
	GET(FlyLocomotionClass* const, pThis, ESI);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CDA78;
	}

	return 0;
}

DEFINE_HOOK(0x4CE4B3, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass* const, pThis, ECX);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CE4BF;
	}

	return 0;
}

DEFINE_HOOK(0x73B2A2, UnitClass_DrawObject_DrawerBlitterFix, 0x6)
{
	enum { SkipGameCode = 0x73B2C3 };

	GET(UnitClass* const, pThis, ESI);
	GET(BlitterFlags, blitterFlags, EDI);

	R->EAX(pThis->GetRemapColour()->Select_Blitter(blitterFlags));

	return SkipGameCode;
}

DEFINE_HOOK(0x710021, FootClass_ImbueLocomotor_SpawnRate, 0x5)
{
	GET(SpawnManagerClass*, pManager, ECX);

	pManager->KillNodes();

	for (const auto& nNodes : pManager->SpawnedNodes)
	{
		nNodes->NodeSpawnTimer.Start(pManager->RegenRate);
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

				if (pThis->Owner->IsControlledByCurrentPlayer())
					VocClass::PlayIndexAtPos(pType->VoiceCrashing, nCoord);

				VocClass::PlayIndexAtPos(pType->CrashingSound, nCoord, &pThis->Audio7);

			}
			else
			{
				VocClass::PlayIndexAtPos(RulesClass::Instance->ScoldSound, nCoord, &pThis->Audio7);
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
DEFINE_HOOK(0x65DF81, TeamTypeClass_CreateMembers_LoadOntoTransport, 0x7)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);

	TechnoExt::ExtMap.Find(pTransport)->CreatedFromAction = true;
	const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;
	FootClass* pGunner = nullptr;

	for (auto pNext = pPayload; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		if (pNext && pNext != pTransport && pNext->Team == pTeam)
		{
			pNext->Transporter = pTransport;
			pGunner = pNext;

			if (isTransportOpenTopped)
				pTransport->EnteredOpenTopped(pNext);

			pNext->SetLocation(pTransport->Location);

			if (pNext->WhatAmI() != InfantryClass::AbsID && pNext->Passengers.NumPassengers <= 0)
			{
				TechnoExt::ExtMap.Find(pNext)->CreatedFromAction = true;
			}
		}
	}

	// Add to transport - this will load the payload object and everything linked to it (rest of the team) in reverse order
	pTransport->AddPassenger(pPayload);

	// Handle gunner change - this is the 'last' passenger because of reverse order
	if (pTransport->GetTechnoType()->Gunner && pGunner)
		pTransport->ReceiveGunner(pGunner);

	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}

// BibShape checks for BuildingClass::BState which needs to not be 0 (constructing) for bib to draw.
// It is possible for BState to be 1 early during construction for frame or two which can result in BibShape being drawn during buildup, which somehow depends on length of buildup.
// Trying to fix this issue at its root is problematic and most of the time causes buildup to play twice, it is simpler to simply fix the BibShape to not draw until the buildup is done - Starkku
DEFINE_HOOK(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
{
	enum { DontDrawBib = 0x43D8EE };

	GET(BuildingClass* const, pThis, ESI);
	return !pThis->ActuallyPlacedOnMap ? DontDrawBib : 0x0;
}

DEFINE_HOOK(0x4DE652, FootClass_AddPassenger_NumPassengerGeq0, 0x7)
{
	enum { GunnerReception = 0x4DE65B, EndFuntion = 0x4DE666 };
	GET(FootClass* const, pThis, ESI);
	// Replace NumPassengers==1 check to allow multipassenger IFV using the fix above
	return pThis->Passengers.NumPassengers > 0 ? GunnerReception : EndFuntion;
}

DEFINE_HOOK(0x440EBB, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x5)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExt::ExtMap.Find(pThis)->IsCreatedFromMapFile ? DoNotCreateParticle : 0;
}

// Ares didn't have something like 0x7397E4 in its UnitDelivery code
DEFINE_HOOK(0x44FBBF, CreateBuildingFromINIFile_AfterCTOR_BeforeUnlimbo, 0x8)
{
	GET(BuildingClass* const, pBld, ESI);

	if (auto pExt = BuildingExt::ExtMap.Find(pBld))
		pExt->IsCreatedFromMapFile = true;

	return 0;
}

//https://github.com/Phobos-developers/Phobos/pull/818
DEFINE_HOOK(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));

	const int XP = 2 * MapClass::Instance->VisibleRect.X - MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomFromMax(2 * MapClass::Instance->VisibleRect.Width);
	const int YP = 2 * MapClass::Instance->VisibleRect.Y + MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomFromMax(2 * MapClass::Instance->VisibleRect.Height + 2);

	cell = { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };

	const auto pCell = MapClass::Instance->TryGetCellAt(cell);
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
		//GameDelete<true,false>(pAnim);
		pAnim->TimeToDie = true;
		pAnim->UnInit();
		pAnim = nullptr;
	}

	pBld->MindControlledByAUnit = false;

	return 0;
}

// Enable sorted add for Air/Top layers to fix issues with attached anims etc.
DEFINE_HOOK(0x4A9750, DisplayClass_Submit_LayerSort, 0x9)
{
	GET(Layer const, layer, EDI);
	R->ECX(layer != Layer::Surface && layer != Layer::Underground);
	return 0;
}

// Fixes C4=no amphibious infantry being killed in water if Chronoshifted/Paradropped there.
DEFINE_HOOK(0x51A996, InfantryClass_PerCellProcess_KillOnImpassable, 0x5)
{
	enum { ContinueChecks = 0x51A9A0, SkipKilling = 0x51A9EB };

	GET(InfantryClass* const, pThis, ESI);
	GET(LandType const, landType, EBX);

	if (landType == LandType::Rock)
		return ContinueChecks;

	if (landType == LandType::Water) {
		if (GroundType::GetCost(landType ,pThis->Type->SpeedType) == 0.0)
			return ContinueChecks;
	}

	return SkipKilling;
}

DEFINE_HOOK(0x718B29, LocomotionClass_SomethingWrong_ReceiveDamage_UseCurrentHP, 0x6)
{
	GET(FootClass* const, pLinked, ECX);
	R->ECX(pLinked->GetType()->Strength);
	return R->Origin() + 0x6;
}

// DEFINE_HOOK(0x6FDDD4, TechnoClass_FireAt_Suicide_UseCurrentHP, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	R->ECX(pThis->GetType()->Strength);
// 	return 0x6FDDDA;
// }

DEFINE_HOOK(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pFlipper = pThis->DirectRockerLinkedUnit;
	pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pFlipper ? pFlipper->Owner : nullptr);

	return 0x70BCA4;
}

// DEFINE_HOOK(0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6)
// {
// 	GET(TechnoClass* const, pRadio, EDI);
//
// 	pRadio->ReceiveDamage(&pRadio->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, true, nullptr);
//
// 	return 0x4425F4;
// }

// DEFINE_HOOK(0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA)
// {
// 	GET(TechnoClass* const, pItem, ESI);
//
// 	pItem->ReceiveDamage(&pItem->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, true, nullptr);
//
// 	return 0x50150E;
// }

// DEFINE_HOOK(0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8)
// {
// 	GET(FootClass* const, pOwner, ECX);
//
// 	pOwner->ReceiveDamage(&pOwner->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, false, nullptr);
//
// 	return 0x71880A;
// }

// Check WaterBound when setting rally points / undeploying instead of just Naval.
DEFINE_HOOK(0x4438B4, BuildingClass_SetRallyPoint_Naval, 0x6)
{
	enum { IsNaval = 0x4438BC, NotNaval = 0x4438C9 };

	GET(BuildingTypeClass* const, pBuildingType, EAX);

	if (pBuildingType->Naval || pBuildingType->SpeedType == SpeedType::Float)
		return IsNaval;

	return NotNaval;
}

FireError __stdcall JumpjetLocomotionClass_Can_Fire(ILocomotion* pThis)
{
	// do not use explicit toggle for this
	if (static_cast<JumpjetLocomotionClass*>(pThis)->NextState == JumpjetLocomotionClass::State::Crashing)
		return FireError::CANT;

	return FireError::OK;
}

DEFINE_JUMP(VTABLE, 0x7ECDF4, GET_OFFSET(JumpjetLocomotionClass_Can_Fire))

// BuildingClass_What_Action() - Fix no attack cursor if AG=no projectile on primary
//DEFINE_SKIP_HOOK(0x447380, BuildingClass_What_Action_RemoveAGCheckA, 0x6, 44739E);
//DEFINE_SKIP_HOOK(0x447709, BuildingClass_What_Action_RemoveAGCheckB, 0x6, 447727);
DEFINE_JUMP(LJMP, 0x447380, 0x44739E);
DEFINE_JUMP(LJMP, 0x447709, 0x447727);

//// AG=no projectiles shouldn't fire at land.
//DEFINE_HOOK(0x6FC87D, TechnoClass_CanFire_AG, 0x6)
//{
//	enum { RetFireIllegal = 0x6FC86A , Continue = 0x0 };
//
//	GET(WeaponTypeClass*, pWeapon, EDI);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));
//
//	return !pWeapon->Projectile->AG && !pTarget->IsInAir() ?
//		RetFireIllegal : Continue;
//}

// Do not display SuperAnimThree for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
DEFINE_HOOK(0x44643E, BuildingClass_Place_SuperAnim, 0x6)
{
	enum { UseSuperAnimOne = 0x4464F6 };

	GET(BuildingClass*, pThis, EBP);
	GET(SuperClass*, pSuper, EAX);

	if (pSuper->RechargeTimer.StartTime == 0 &&
		pSuper->RechargeTimer.TimeLeft == 0 &&
		!SWTypeExt::ExtMap.Find(pSuper->Type)->SW_InitialReady)
	{
		R->ECX(pThis);
		return UseSuperAnimOne;
	}

	return 0;
}

// Do not advance SuperAnim for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
DEFINE_HOOK(0x451033, BuildingClass_AnimationAI_SuperAnim, 0x6)
{
	enum { SkipSuperAnimCode = 0x451048 };

	GET(SuperClass*, pSuper, EAX);

	if (pSuper->RechargeTimer.StartTime == 0
		&& pSuper->RechargeTimer.TimeLeft == 0
		&& !SWTypeExt::ExtMap.Find(pSuper->Type)->SW_InitialReady)
		return SkipSuperAnimCode;

	return 0;
}

// Updates layers of all animations attached to the given techno.
void UpdateAttachedAnimLayers(TechnoClass* pThis)
{
	// Skip if has no attached animations.
	if (!pThis || !pThis->HasParachute || !pThis->IsAlive)
		return;

	// Could possibly be faster to track the attached anims in TechnoExt but the profiler doesn't show this as a performance hog so whatever.
	for (auto pAnim : *AnimClass::Array)
	{
		if (pAnim->OwnerObject != pThis)
			continue;

		DisplayClass::Instance->SubmitObject(pAnim);
	}
}

//causing desyncs , need to be retest
DEFINE_HOOK(0x54B188, JumpjetLocomotionClass_Process_LayerUpdate, 0x6)
{
	GET(TechnoClass*, pLinkedTo, EAX);

	UpdateAttachedAnimLayers(pLinkedTo);

	return 0;
}

DEFINE_HOOK(0x4CD4E1, FlyLocomotionClass_Update_LayerUpdate, 0x6)
{
	GET(TechnoClass*, pLinkedTo, ECX);

	if (pLinkedTo->LastLayer != pLinkedTo->InWhichLayer())
		UpdateAttachedAnimLayers(pLinkedTo);

	return 0;
}

DEFINE_HOOK(0x688F8C, ScenarioClass_ScanPlaceUnit_CheckMovement, 0x5)
{
	GET(TechnoClass*, pTechno, EBX);
	LEA_STACK(CoordStruct*, pHomeCoords, STACK_OFFSET(0x6C, -0x30));

	if (pTechno->WhatAmI() == AbstractType::Building)
		return 0;

	const auto pCell = MapClass::Instance->GetCellAt(*pHomeCoords);
	const auto pTechnoType = pTechno->GetTechnoType();
	if (!pCell->IsClearToMove(pTechnoType->SpeedType, 0, 0, (int)MovementZone::None, MovementZone::Normal, -1, 1))
	{
		Debug::Log("Techno[%s - %s] Not Allowed to exist at cell [%d . %d] !\n", pTechnoType->ID, pTechno->GetThisClassName(), pCell->MapCoords.X, pCell->MapCoords.Y);
		return 0x688FB9;
	}

	return 0;
}

DEFINE_HOOK(0x68927B, ScenarioClass_ScanPlaceUnit_CheckMovement2, 0x5)
{
	GET(TechnoClass*, pTechno, EDI);
	LEA_STACK(CoordStruct*, pCellCoords, STACK_OFFSET(0x6C, -0xC));

	if (pTechno->WhatAmI() == AbstractType::Building)
		return 0;

	const auto pCell = MapClass::Instance->GetCellAt(*pCellCoords);
	const auto pTechnoType = pTechno->GetTechnoType();
	if (!pCell->IsClearToMove(pTechnoType->SpeedType, 0, 0, (int)MovementZone::None, MovementZone::Normal, -1, 1))
	{
		Debug::Log("Techno[%s - %s] Not Allowed to exist at cell [%d . %d] !\n", pTechnoType->ID, pTechno->GetThisClassName(), pCell->MapCoords.X, pCell->MapCoords.Y);
		return 0x689295;
	}

	return 0;

}

// In vanilla YR, game destroys building animations directly by calling constructor.
// Ares changed this to call UnInit() which has a consequence of doing pointer invalidation on the AnimClass pointer.
// This notably causes an issue with Grinder that restores ActiveAnim if the building is sold/destroyed while SpecialAnim is playing even if the building is gone or in limbo.
// Now it does not do this if the building is in limbo, which covers all cases from being destroyed, sold, to erased by Temporal weapons.
// There is another potential case for this with ProductionAnim & IdleAnim which is also patched here just in case.
DEFINE_HOOK_AGAIN(0x44E997, BuildingClass_Detach_RestoreAnims, 0x6)
DEFINE_HOOK(0x44E9FA, BuildingClass_Detach_RestoreAnims, 0x6)
{
	enum { SkipAnimOne = 0x44E9A4, SkipAnimTwo = 0x44EA07 };

	GET(BuildingClass*, pThis, ESI);

	if (pThis->InLimbo)
		return R->Origin() == 0x44E997 ? SkipAnimOne : SkipAnimTwo;

	return 0;
}

// Fix initial facing when jumpjet locomotor is being attached
// there is bug with preplaced units , wait for fix
//DEFINE_HOOK(0x54AE44, JumpjetLocomotionClass_LinkToObject_FixFacing, 0x7)
//{
//	GET(ILocomotion*, iLoco, EBP);
//	auto const pThis = static_cast<JumpjetLocomotionClass*>(iLoco);
//
//	pThis->Facing.Set_Current(pThis->LinkedTo->PrimaryFacing.Current());
//	pThis->Facing.Set_Desired(pThis->LinkedTo->PrimaryFacing.Desired());
//
//	return 0;
//}

// Fix initial facing when jumpjet locomotor is being attached
void __stdcall JumpjetLocomotionClass_Unlimbo(ILocomotion* pThis)
{
	auto const pThisLoco = static_cast<JumpjetLocomotionClass*>(pThis);
	pThisLoco->Facing.Set_Current(pThisLoco->LinkedTo->PrimaryFacing.Current());
	pThisLoco->Facing.Set_Desired(pThisLoco->LinkedTo->PrimaryFacing.Desired());
}

DEFINE_JUMP(VTABLE, 0x7ECDB8, GET_OFFSET(JumpjetLocomotionClass_Unlimbo))

// This fixes the issue when locomotor is crashing in grounded or
// hovering state and the crash processing code won't be reached.
// Can be observed easily when Crashable=yes jumpjet is attached to
// a unit and then destroyed.
DEFINE_HOOK(0x54AEDC, JumpjetLocomotionClass_Process_CheckCrashing, 0x9)
{
	enum { ProcessMovement = 0x54AEED, Skip = 0x54B16C };

	GET(ILocomotion*, iLoco, ESI);
	auto const pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);

	return pLoco->Is_Moving_Now()  // stolen code
		|| pLoco->LinkedTo->IsCrashing
		? ProcessMovement
		: Skip;
}

// WWP for some reason passed nullptr as source to On_Destroyed even though the real source existed
DEFINE_HOOK(0x738467, UnitClass_TakeDamage_FixOnDestroyedSource, 0x6)
{
	enum { Continue = 0x73866E, ForceKill = 0x73847B };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0x44, 0x10));

	R->AL(pThis->Crash(pSource));
	return 0x738473;
	//return  ? Continue : ForceKill;
}

// Fixes second half of Colors list not getting retinted correctly by map triggers, superweapons etc.
#pragma region LightingColorSchemesFix

namespace AdjustLightingTemp
{
	int colorSchemeCount = 0;
}

DEFINE_HOOK(0x53AD7D, IonStormClass_AdjustLighting_SetContext, 0x8)
{
	AdjustLightingTemp::colorSchemeCount = ColorScheme::GetNumberOfSchemes() * 2;

	return 0;
}

int __fastcall NumberOfSchemes_Wrapper() {
	return AdjustLightingTemp::colorSchemeCount;
}

DEFINE_JUMP(CALL, 0x53AD92, GET_OFFSET(NumberOfSchemes_Wrapper));

#pragma endregion


//// Set ShadeCount to 53 to initialize the palette fully shaded - this is required to make it not draw over shroud for some reason.
DEFINE_HOOK(0x68C4C4, GenerateColorSpread_ShadeCountSet, 0x5)
{
	if (!Phobos::Config::ApplyShadeCountFix)
		return 0x0;

	//shade count
	if (R->EDX<int>() == 1)
		R->EDX(53);

	return 0;
}

DEFINE_HOOK(0x4C780A, EventClass_Execute_DeployEvent_NoVoiceFix, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	pThis->VoiceDeploy();
	return 0x0;
}

// Checks if vehicle can deploy into a building at its current location. If unit has no DeploysInto set returns noDeploysIntoDefaultValue (def = false) instead.
bool CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue)
{
	if (!pThis)
		return false;

	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return noDeploysIntoDefaultValue;

	bool canDeploy = true;
	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	pThis->UpdatePlacement(PlacementType::Remove);

	pThis->Locomotor->Mark_All_Occupation_Bits((int)PlacementType::Remove);

	if (!pDeployType->CanCreateHere(mapCoords, pThis->Owner))
		canDeploy = false;

	pThis->Locomotor->Mark_All_Occupation_Bits((int)PlacementType::Put);
	pThis->UpdatePlacement(PlacementType::Put);

	return canDeploy;
}

// Fix DeployToFire not working properly for WaterBound DeploysInto buildings and not recalculating position on land if can't deploy.
DEFINE_HOOK(0x4D580B, FootClass_ApproachTarget_DeployToFire, 0x6)
{
	enum { SkipGameCode = 0x4D583F };

	GET(UnitClass*, pThis, EBX);

	R->EAX(CanDeployIntoBuilding(pThis, true));

	return SkipGameCode;
}

DEFINE_HOOK(0x741050, UnitClass_CanFire_DeployToFire, 0x6)
{
	enum { SkipGameCode = 0x741086, MustDeploy = 0x7410A8 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->DeployToFire && pThis->CanDeployNow() && !CanDeployIntoBuilding(pThis, true))
		return MustDeploy;

	return SkipGameCode;
}

// skip call DrawInfoTipAndSpiedSelection
// Note that Ares have the TacticalClass_DrawUnits_ParticleSystems hook at 0x6D9427
DEFINE_JUMP(LJMP, 0x6D9430, 0x6D95A1); // Tactical_RenderLayers

// Fixed position and layer of info tip and reveal production cameo on selected building
// Author: Belonit
#pragma region DrawInfoTipAndSpiedSelection
// Call DrawInfoTipAndSpiedSelection in new location
DEFINE_HOOK(0x6D9781, Tactical_RenderLayers_DrawInfoTipAndSpiedSelection, 0x5)
{
	GET(TechnoClass*, pThis, EBX);
	GET(Point2D*, pLocation, EAX);

	const auto pBuilding = specific_cast<BuildingClass*>(pThis);

	if (pBuilding && pBuilding->IsSelected && pBuilding->IsOnMap && BuildingExt::ExtMap.Find(pBuilding)->LimboID <= -1)
	{
		const int foundationHeight = pBuilding->Type->GetFoundationHeight(0);
		const int typeHeight = pBuilding->Type->Height;
		const int yOffest = (Unsorted::CellHeightInPixels * (foundationHeight + typeHeight)) >> 2;

		Point2D centeredPoint = { pLocation->X, pLocation->Y - yOffest };
		pBuilding->DrawInfoTipAndSpiedSelection(&centeredPoint, &DSurface::ViewBounds);
	}

	return 0;
}
#pragma endregion DrawInfoTipAndSpiedSelection

#include <VeinholeMonsterClass.h>

DEFINE_HOOK(0x5349A5, Map_ClearVectors_Veinhole, 0x5)
{
	VeinholeMonsterClass::DeleteAll();
	VeinholeMonsterClass::DeleteVeinholeGrowthData();
	return 0;
}