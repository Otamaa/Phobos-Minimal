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
#include <Notifications.h>

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
#include <Utilities/Helpers.h>

#include <Locomotor/Cast.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
// ASMJIT_PATCH(0x423365, AnimClass_SHPShadowCheck, 0x8)
// {
// 	GET(AnimClass* const, pAnim, ESI);
// 	return (pAnim->Type->Shadow && pAnim->HasExtras) ?
// 		0x42336D :
// 		0x4233EE;
// }

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
DEFINE_JUMP(LJMP, 0x546C23, 0x546CBF) //Phobos_BugFixes_Tileset255_RefNonMMArray

ASMJIT_PATCH(0x5F452E, TechnoClass_Selectable_DeathCounter, 0x6) // 8
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (pUnit->DeathFrameCounter > 0)
		{
			return 0x5F454E;
		}
	}

	return 0x0;
}

// issue #250: Building placement hotkey not responding
// Author: Uranusian
//DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBD5 + 7); // DisplayClass_MouseLeftRelease_HotkeyFix
DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBDC);

ASMJIT_PATCH(0x4FB2DE, HouseClass_PlaceObject_HotkeyFix, 0x6)
{
	GET(TechnoClass* const, pObject, ESI);

	pObject->ClearSidebarTabObject();

	return 0;
}

// Issue #46: Laser is mirrored relative to FireFLH
// Author: Starkku
// ASMJIT_PATCH(0x6FF2BE, TechnoClass_FireAt_BurstOffsetFix_1, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
//
// 	--pThis->CurrentBurstIndex;
//
// 	return 0x6FF2D1;
// }

bool IsUndeploy = false;

// issue #290: Undeploy building into a unit plays EVA_NewRallyPointEstablished
// Author: secsome
ASMJIT_PATCH(0x44377E, BuildingClass_ActiveClickWith, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(CellStruct*, pCell, STACK_OFFS(0x84, -0x8));

	if (pThis->Type->UndeploysInto)
	{
		IsUndeploy = true;
		pThis->SetRallypoint(pCell, false);
		IsUndeploy = false;
	}
	else if (pThis->IsUnitFactory())
		pThis->SetRallypoint(pCell, true);

	return 0x4437AD;
}

ASMJIT_PATCH(0x443892, BuildingClass_SetRallyPoint_Naval_UndeploysInto, 0x6)
{
	if (!IsUndeploy)
		return 0;

	GET(BuildingTypeClass*, pType, EAX);
	const auto pUnitType = pType->UndeploysInto;

	R->Stack(STACK_OFFSET(0xA4, -0x84), pUnitType->SpeedType);
	R->ESI(pUnitType->MovementZone);
	return 0;
}

// issue #232: Naval=yes overrides WaterBound=no and prevents move orders onto Land cells
// Author: Uranusian
DEFINE_JUMP(LJMP, 0x47CA05, 0x47CA33);

// bugfix: DeathWeapon not properly detonates
// Author: Uranusian
// ASMJIT_PATCH(0x70D77F, TechnoClass_FireDeathWeapon_ProjectileFix, 0x8)
// {
// 	GET(BulletClass*, pBullet, EBX);
// 	GET(CoordStruct*, pCoord, EAX);
//
// 	pBullet->SetLocation(*pCoord);
// 	pBullet->Explode(true);
//
// 	return 0x70D787;
// }

static bool NOINLINE IsTemporalptrValid(TemporalClass* pThis)
{
	return VTable::Get(pThis) == TemporalClass::vtable;
}

static NOINLINE void IsTechnoShouldBeAliveAfterTemporal(TechnoClass* pThis)
{
	if (pThis->TemporalTargetingMe)
	{
		// Also check for vftable here to guarantee the TemporalClass not being destoryed already.
		if (IsTemporalptrValid(pThis->TemporalTargetingMe)) // TemporalClass::`vtable`
		{
			if (pThis->TemporalTargetingMe->Owner == pThis->TemporalTargetingMe->Target) {
				pThis->BeingWarpedOut = false;
				pThis->TemporalTargetingMe->Detach();
				pThis->TemporalTargetingMe = nullptr;
				return;
			}
			pThis->TemporalTargetingMe->Update();
		}
		else // It should had being warped out, delete this object
		{
			pThis->TemporalTargetingMe = nullptr;
			pThis->Limbo();
			//Debug::LogInfo(__FUNCTION__" Called ");
			TechnoExtData::HandleRemove(pThis, nullptr, true, false);
		}
	} else if (!pThis->TemporalTargetingMe && pThis->BeingWarpedOut) {
		pThis->BeingWarpedOut = false;
	}
}

// Fix the crash of TemporalTargetingMe related "stack dump starts with 0051BB7D"
// Author: secsome
ASMJIT_PATCH(0x43FCF9, BuildingClass_AI_TemporalTargetingMe, 0x6) // BuildingClass
{
	IsTechnoShouldBeAliveAfterTemporal(R->ESI<BuildingClass*>());
	return 0x43FD08;
}

ASMJIT_PATCH(0x414BDB, AircraftClass_AI_TemporalTargetingMe, 0x6) //
{
	IsTechnoShouldBeAliveAfterTemporal(R->ESI<AircraftClass*>());
	return 0x414BEA;
}

ASMJIT_PATCH(0x736204, UnitClass_AI_TemporalTargetingMe, 0x6) //
{
	IsTechnoShouldBeAliveAfterTemporal(R->ESI<UnitClass*>());
	return 0x736213;
}

ASMJIT_PATCH(0x51BB6E, InfantryClass_AI_TemporalTargetingMe_Fix, 0x6) //
{
	IsTechnoShouldBeAliveAfterTemporal(R->ESI<InfantryClass*>());
	return 0x51BB7D;
}

// Fix the issue that AITriggerTypes do not recognize building upgrades
// Author: Uranusian
ASMJIT_PATCH(0x41EB43, AITriggerTypeClass_Condition_SupportPowersup, 0x7)		//AITriggerTypeClass_EnemyHouseOwns_SupportPowersup
{
	GET(HouseClass* const, pHouse, EDX);
	GET(int const, idxBld, EBP);

	const auto pType = BuildingTypeClass::Array->Items[idxBld];
	int count = BuildingTypeExtData::GetUpgradesAmount(pType, pHouse);

	if (count == -1)
		count = pHouse->ActiveBuildingTypes.get_count(idxBld);

	R->EAX(count);

	return R->Origin() + 0xC;
}ASMJIT_PATCH_AGAIN(0x41EEE3, AITriggerTypeClass_Condition_SupportPowersup, 0x7)	//AITriggerTypeClass_OwnerHouseOwns_SupportPowersup


// Dehardcode the stupid Wall-Gate relationships
// Author: Uranusian
ASMJIT_PATCH(0x441053, BuildingClass_Unlimbo_EWGate, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);

	return !RulesClass::Instance->EWGates.contains(pThis) ? 0 : 0x441065;
}

ASMJIT_PATCH(0x4410E1, BuildingClass_Unlimbo_NSGate, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);

	return !RulesClass::Instance->NSGates.contains(pThis) ? 0 : 0x4410F3;
}

ASMJIT_PATCH(0x480534, CellClass_AttachesToNeighbourOverlay, 5)
{
	GET(int, idxOverlay, EAX);
	GET(CellClass* const, pThis, EBP);
	GET_STACK(int const, state, STACK_OFFS(0x10, -0x8));
	const bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->Items[idxOverlay]->Wall;

	if (Wall) {
		return 0x480549;

	}

	if (auto pBuilding = pThis->GetBuilding()) {
		if (pBuilding->Health > 0) {
			const auto pBType = pBuilding->Type;

			if ((RulesClass::Instance->EWGates.contains(pBType)) && (state == 2 || state == 6))
				return 0x480549;
			else if ((RulesClass::Instance->NSGates.contains(pBType)) && (state == 0 || state == 4))
				return 0x480549;
			else if (RulesExtData::Instance()->WallTowers.Contains(pBType))
				return 0x480549;
		}
	}

	return 0x480552;
}

// WW take 1 second as 960 milliseconds, this will fix that back to the actual time.
// Author: secsome
ASMJIT_PATCH(0x6C919F, StandaloneScore_SinglePlayerScoreDialog_ActualTime, 0x5)
{
	R->ECX(static_cast<int>(std::round(R->ECX() * 0.96)));
	return 0;
}

// Fixed the bug that units' lighting get corrupted after loading a save with a different lighting being set
// Author: secsome
ASMJIT_PATCH(0x67E6E5, LoadGame_RecalcLighting, 0x7)
{
	ScenarioClass::Instance->RecalcLighting(
		ScenarioClass::Instance->NormalLighting.Tint.Red * 10,
		ScenarioClass::Instance->NormalLighting.Tint.Green * 10,
		ScenarioClass::Instance->NormalLighting.Tint.Blue * 10,
		0
	);

	return 0;
}

ASMJIT_PATCH(0x4CDA6F, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x9)
{
	GET(FlyLocomotionClass* const, pThis, ESI);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = GET_TECHNOTYPE(pLinked)->Speed 
			* pThis->CurrentSpeed *
			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CDA78;
	}

	return 0;
}

ASMJIT_PATCH(0x4CE4B3, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass* const, pThis, ECX);

	if (const auto pLinked = pThis->LinkedTo)
	{
		const double currentSpeed = GET_TECHNOTYPE(pLinked)->Speed 
			* pThis->CurrentSpeed *
			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);

		R->EAX(int(currentSpeed));
		return 0x4CE4BF;
	}

	return 0;
}

ASMJIT_PATCH(0x73B2A2, UnitClass_DrawObject_DrawerBlitterFix, 0x6)
{
	enum { SkipGameCode = 0x73B2C3 };

	GET(UnitClass* const, pThis, ESI);
	GET(BlitterFlags, blitterFlags, EDI);

	R->EAX(pThis->GetRemapColour()->Select_Blitter(blitterFlags));

	return SkipGameCode;
}

ASMJIT_PATCH(0x710021, FootClass_ImbueLocomotor_SpawnRate, 0x5)
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
ASMJIT_PATCH(0x6ED6E5, TeamClass_TMission_Deploy_DeploysInto, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	// Handle searching for free space in Hunt mission handler
	pThis->ForceMission(Mission::Hunt);
	pThis->MissionStatus = 2; // Tells UnitClass::Mission_Hunt to omit certain checks.

	return 0;
}

ASMJIT_PATCH(0x73EFD8, UnitClass_Mission_Hunt_DeploysInto, 0x6)
{
	enum { SkipToDeploy = 0x73F015 };

	GET(UnitClass*, pThis, ESI);

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

		// Skip MCV-specific & player control checks if coming from deploy script action.
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
ASMJIT_PATCH(0x4DACDD, FootClass_CrashingVoice, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (pThis->IsCrashing != pThis->WasCrashingAlready)
	{
		if (pThis->IsCrashing)
		{
			pThis->MoveSoundAudioController.ShutUp();
			auto const nCoord = pThis->GetCoords();

			if (!pThis->IsAttackedByLocomotor)
			{
				const auto pType = GET_TECHNOTYPE(pThis);

				if (pThis->Owner->IsControlledByHuman())
					VocClass::SafeImmedietelyPlayAt(pType->VoiceCrashing, &nCoord);

				VocClass::SafeImmedietelyPlayAt(pType->CrashingSound, &nCoord, &pThis->MoveSoundAudioController);

			}
			else
			{
				VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->ScoldSound, &nCoord, &pThis->MoveSoundAudioController);
			}
		}
		else if (pThis->IsMoveSoundPlaying ) // done playing
			pThis->MoveSoundAudioController.ShutUp();

		pThis->WasCrashingAlready = pThis->IsCrashing;
	}

	return 0x4DADC8;
}

// Bugfix: TAction 7,80,107.
ASMJIT_PATCH(0x65DF67, TeamTypeClass_CreateMembers_LoadOntoTransport, 0x6)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);
	GET(TeamTypeClass const*, pThis, EBX);

	auto unmarkPayloadCreated = [](FootClass* member){TechnoExtContainer::Instance.Find(member)->PayloadCreated = false;};

	if (!pTransport) {
		for (auto pNext = pPayload;
		pNext && pNext != pTransport && pNext->Team == pTeam;
		pNext = flag_cast_to<FootClass*>(pNext->NextObject))
			unmarkPayloadCreated(pNext);

		return 0x65DFE8;
	}

	unmarkPayloadCreated(pTransport);
	if (!pPayload || !pThis->Full)
		return 0x65E004;

	const bool isTransportOpenTopped = GET_TECHNOTYPE(pTransport)->OpenTopped;
	FootClass* pGunner = nullptr;

	for (auto pNext = pPayload; pNext; pNext = flag_cast_to<FootClass*>(pNext->NextObject))
	{
		if (pNext && pNext != pTransport && pNext->Team == pTeam)
		{
			pNext->Transporter = pTransport;
			pGunner = pNext;

			if (isTransportOpenTopped)
				pTransport->EnteredOpenTopped(pNext);

			pNext->SetLocation(pTransport->Location);
		}
	}

	// Add to transport - this will load the payload object and everything linked to it (rest of the team) in reverse order
	pTransport->Passengers.AddPassenger(pPayload);

	// Handle gunner change - this is the 'last' passenger because of reverse order
	if (GET_TECHNOTYPE(pTransport)->Gunner && pGunner)
		pTransport->ReceiveGunner(pGunner);

	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}

// BibShape checks for BuildingClass::BState which needs to not be 0 (constructing) for bib to draw.
// It is possible for BState to be 1 early during construction for frame or two which can result in BibShape being drawn during buildup, which somehow depends on length of buildup.
// Trying to fix this issue at its root is problematic and most of the time causes buildup to play twice, it is simpler to simply fix the BibShape to not draw until the buildup is done - Starkku
// ASMJIT_PATCH(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
// {
// 	enum { DontDrawBib = 0x43D8EE };

// 	GET(BuildingClass* const, pThis, ESI);
// 	return !pThis->ActuallyPlacedOnMap ? DontDrawBib : 0x0;
// }

ASMJIT_PATCH(0x4DE652, FootClass_AddPassenger_NumPassengerGeq0, 0x7)
{
	enum { GunnerReception = 0x4DE65B, EndFuntion = 0x4DE666 };
	GET(FootClass* const, pThis, ESI);
	// Replace NumPassengers==1 check to allow multipassenger IFV using the fix above
	return pThis->Passengers.NumPassengers > 0 ? GunnerReception : EndFuntion;
}

ASMJIT_PATCH(0x440EBB, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x5)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExtContainer::Instance.Find(pThis)->IsCreatedFromMapFile ? DoNotCreateParticle : 0;
}

// Ares didn't have something like 0x7397E4 in its UnitDelivery code
ASMJIT_PATCH(0x44FBBF, CreateBuildingFromINIFile_AfterCTOR_BeforeUnlimbo, 0x8)
{
	GET(BuildingClass* const, pBld, ESI);

	BuildingExtContainer::Instance.Find(pBld)->IsCreatedFromMapFile = true;

	return 0;
}

//https://github.com/Phobos-developers/Phobos/pull/818
ASMJIT_PATCH(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));

	const int XP = 2 * MapClass::Instance->VisibleRect.X - MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomFromMax(2 * MapClass::Instance->VisibleRect.Width);
	const int YP = 2 * MapClass::Instance->VisibleRect.Y + MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomFromMax(2 * MapClass::Instance->VisibleRect.Height + 2);

	cell = { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };

	const auto pCell = MapClass::Instance->TryGetCellAt(cell);

	if (!pCell || !MapClass::Instance->IsWithinUsableArea(pCell, true))
		return SkipSpawn;

	const bool isWater = pCell->LandType == LandType::Water;

	if (isWater && RulesExtData::Instance()->Crate_LandOnly.Get())
		return SkipSpawn;

	cell = MapClass::Instance->NearByLocation(pCell->MapCoords,
		isWater ? SpeedType::Float : SpeedType::Track,
		ZoneType::None, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	R->EAX(&cell);

	return SpawnCrate;
}

ASMJIT_PATCH(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1
	 || TechnoTypeExtContainer::Instance.Find(pBuilding->Type)->IgnoreForBaseCenter
	 || !MapClass::Instance->CoordinatesLegal(pBuilding->GetMapCoords()))
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;

	if(VTable::Get(pBuilding) != BuildingClass::vtable)
		Debug::FatalError("%x invalid building ptr !", pBuilding);

	return 0;
}ASMJIT_PATCH_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)


ASMJIT_PATCH(0x4AC534, DisplayClass_ComputeStartPosition_IllegalCoords, 0x6)
{
	enum { SkipTechno = 0x4AC55B };

	GET(TechnoClass* const, pTechno, ECX);

	if (!MapClass::Instance->CoordinatesLegal(pTechno->GetMapCoords()) 
		|| TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType())->IgnoreForBaseCenter)
		return SkipTechno;


	return 0;
}

ASMJIT_PATCH(0x519F84, InfantryClass_UpdatePosition_EngineerPreUninit, 0x6)
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
// ASMJIT_PATCH(0x4A9750, DisplayClass_Submit_LayerSort, 0x9)
// {
// 	GET(Layer const, layer, EDI);
// 	R->ECX(layer != Layer::Surface && layer != Layer::Underground);
// 	return 0;
// }

// Fixes C4=no amphibious infantry being killed in water if Chronoshifted/Paradropped there.
ASMJIT_PATCH(0x51A996, InfantryClass_UpdatePositio_KillOnImpassable, 0x5)
{
	enum { ContinueChecks = 0x51A9A0, SkipKilling = 0x51A9EB };

	GET(InfantryClass* const, pThis, ESI);
	GET(LandType const, landType, EBX);

	if (landType == LandType::Rock)
		return ContinueChecks;

	if (landType == LandType::Water)
	{
		if (GroundType::GetCost(landType, pThis->Type->SpeedType) == 0.0)
			return ContinueChecks;
	}

	return SkipKilling;
}

// ASMJIT_PATCH(0x6FDDD4, TechnoClass_FireAt_Suicide_UseCurrentHP, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	R->ECX(pThis->GetType()->Strength);
// 	return 0x6FDDDA;
// }

ASMJIT_PATCH(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pFlipper = pThis->DirectRockerLinkedUnit;
	pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pFlipper ? pFlipper->Owner : nullptr);

	return 0x70BCA4;
}

// ASMJIT_PATCH(0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA)
// {
// 	GET(TechnoClass* const, pItem, ESI);
//
// 	pItem->ReceiveDamage(&pItem->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, true, nullptr);
//
// 	return 0x50150E;
// }

// ASMJIT_PATCH(0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8)
// {
// 	GET(FootClass* const, pOwner, ECX);
//
// 	pOwner->ReceiveDamage(&pOwner->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, false, nullptr);
//
// 	return 0x71880A;
// }

// Check WaterBound when setting rally points / undeploying instead of just Naval.
ASMJIT_PATCH(0x4438B4, BuildingClass_SetRallyPoint_Naval, 0x6)
{
	enum { IsNaval = 0x4438BC, NotNaval = 0x4438C9 };

	GET(BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(bool, playEVA, STACK_OFFSET(0xA4, 0x8));
	REF_STACK(SpeedType, spdtp, STACK_OFFSET(0xA4, -0x84));

	if (!playEVA)// assuming the hook above is the only place where it's set to false when UndeploysInto
	{
		if (auto pInto = pBuildingType->UndeploysInto)// r u sure this is not too OP?
		{
			R->ESI(pInto->MovementZone);
			spdtp = pInto->SpeedType;
			return NotNaval;
		}
	}

	if (pBuildingType->Naval || pBuildingType->SpeedType == SpeedType::Float)
		return IsNaval;

	return NotNaval;
}

ASMJIT_PATCH(0x6DAAB2, TacticalClass_DrawRallyPointLines_NoUndeployBlyat, 0x6)
{
	GET(BuildingClass*, pBld, EDI);
	if (pBld->ArchiveTarget && pBld->CurrentMission != Mission::Selling)
		return 0x6DAAC0;
	return 0x6DAD45;
}

// BuildingClass_What_Action() - Fix no attack cursor if AG=no projectile on primary
//DEFINE_SKIP_HOOK(0x447380, BuildingClass_What_Action_RemoveAGCheckA, 0x6, 44739E);
//DEFINE_SKIP_HOOK(0x447709, BuildingClass_What_Action_RemoveAGCheckB, 0x6, 447727);
DEFINE_JUMP(LJMP, 0x447380, 0x44739E);
DEFINE_JUMP(LJMP, 0x447709, 0x447727);

//// AG=no projectiles shouldn't fire at land.
//ASMJIT_PATCH(0x6FC87D, TechnoClass_CanFire_AG, 0x6)
//{
//	enum { RetFireIllegal = 0x6FC86A , Continue = 0x0 };
//
//	GET(WeaponTypeClass*, pWeapon, EDI);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));
//
//	return !pWeapon->Projectile->AG && !pTarget->IsInAir() ?
//		RetFireIllegal : Continue;
//}


// Updates layers of all animations attached to the given techno.
static void UpdateAttachedAnimLayers(TechnoClass* pThis)
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
// ASMJIT_PATCH(0x54B188, JumpjetLocomotionClass_Process_LayerUpdate, 0x6)
// {
// 	GET(TechnoClass*, pLinkedTo, EAX);
//
// 	UpdateAttachedAnimLayers(pLinkedTo);
//
// 	return 0;
// }
//
// ASMJIT_PATCH(0x4CD4E1, FlyLocomotionClass_Update_LayerUpdate, 0x6)
// {
// 	GET(TechnoClass*, pLinkedTo, ECX);
//
// 	if (pLinkedTo->LastLayer != pLinkedTo->InWhichLayer())
// 		UpdateAttachedAnimLayers(pLinkedTo);
//
// 	return 0;
// }

// Update attached anim layers after parent unit changes layer.
static void __fastcall DisplayClass_Submit_Wrapper(DisplayClass* pThis, void* _, ObjectClass* pObject)
{
	pThis->SubmitObject(pObject);
	//Known to be techno already , dont need to cast
	UpdateAttachedAnimLayers((TechnoClass*)pObject);
}

DEFINE_FUNCTION_JUMP(CALL, 0x54B18E, DisplayClass_Submit_Wrapper);  // JumpjetLocomotionClass_Process
DEFINE_FUNCTION_JUMP(CALL, 0x4CD4E7, DisplayClass_Submit_Wrapper); // FlyLocomotionClass_Update

ASMJIT_PATCH(0x688F8C, ScenarioClass_ScanPlaceUnit_CheckMovement, 0x5)
{
	GET(TechnoClass*, pTechno, EBX);
	LEA_STACK(CoordStruct*, pHomeCoords, STACK_OFFSET(0x6C, -0x30));

	if (pTechno->WhatAmI() == AbstractType::Building)
		return 0;

	const auto pCell = MapClass::Instance->GetCellAt(*pHomeCoords);
	const auto pTechnoType = GET_TECHNOTYPE(pTechno);
	if (!pCell->IsClearToMove(pTechnoType->SpeedType, pTechno->WhatAmI() == InfantryClass::AbsID, 0, ZoneType::None, pTechnoType->MovementZone, -1, 1))
	{
		if (Phobos::Otamaa::IsAdmin)
			Debug::LogInfo("Techno[{} - {}] Not Allowed to exist at cell [{} . {}] !", pTechnoType->ID, pTechno->GetThisClassName(), pCell->MapCoords.X, pCell->MapCoords.Y);

		return 0x688FB9;
	}

	return 0;
}

ASMJIT_PATCH(0x68927B, ScenarioClass_ScanPlaceUnit_CheckMovement2, 0x5)
{
	GET(TechnoClass*, pTechno, EDI);
	LEA_STACK(CoordStruct*, pCellCoords, STACK_OFFSET(0x6C, -0xC));

	if (pTechno->WhatAmI() == AbstractType::Building)
		return 0;

	const auto pCell = MapClass::Instance->GetCellAt(*pCellCoords);
	const auto pTechnoType = GET_TECHNOTYPE(pTechno);
	if (!pCell->IsClearToMove(pTechnoType->SpeedType, pTechno->WhatAmI() == InfantryClass::AbsID, 0, ZoneType::None, pTechnoType->MovementZone, -1, 1))
	{
		if (Phobos::Otamaa::IsAdmin)
			Debug::LogInfo("Techno[{} - {}] Not Allowed to exist at cell [{} . {}] !", pTechnoType->ID, pTechno->GetThisClassName(), pCell->MapCoords.X, pCell->MapCoords.Y);

		return 0x689295;
	}

	return 0;

}

// In vanilla YR, game destroys building animations directly by calling constructor.
// Ares changed this to call UnInit() which has a consequence of doing pointer invalidation on the AnimClass pointer.
// This notably causes an issue with Grinder that restores ActiveAnim if the building is sold/destroyed while SpecialAnim is playing even if the building is gone or in limbo.
// Now it does not do this if the building is in limbo, which covers all cases from being destroyed, sold, to erased by Temporal weapons.
// There is another potential case for this with ProductionAnim & IdleAnim which is also patched here just in case.
//ASMJIT_PATCH(0x44E9FA, BuildingClass_Detach_RestoreAnims, 0x6)
//{
//	enum { SkipAnimOne = 0x44E9A4, SkipAnimTwo = 0x44EA07 };
//
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->InLimbo || !pThis->IsAlive)
//		return R->Origin() == 0x44E997 ? SkipAnimOne : SkipAnimTwo;
//
//	return 0;
//}ASMJIT_PATCH_AGAIN(0x44E997, BuildingClass_Detach_RestoreAnims, 0x6)



// WWP for some reason passed nullptr as source to On_Destroyed even though the real source existed
ASMJIT_PATCH(0x738467, UnitClass_TakeDamage_FixOnDestroyedSource, 0x6)
{
	enum { Continue = 0x73866E, ForceKill = 0x73847B };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0x44, 0x10));

	R->AL(pThis->Crash(pSource));
	return 0x738473;
	//return  ? Continue : ForceKill;
}

// Fixes second half of Colors list not getting retinted correctly by map triggers, superweapons etc.
ASMJIT_PATCH(0x53AD85, IonStormClass_AdjustLighting_ColorSchemes, 0x5)
{
	enum { SkipGameCode = 0x53ADD6 };

	GET_STACK(bool, tint, STACK_OFFSET(0x20, 0x8));
	GET(HashIterator*, it, ECX);
	GET(int, red, EBP);
	GET(int, green, EDI);
	GET(int, blue, EBX);

	int paletteCount = 0;

	for (auto pSchemes = ColorScheme::GetPaletteSchemesFromIterator(it); pSchemes; pSchemes = ColorScheme::GetPaletteSchemesFromIterator(it))
	{
		for (int i = 1; i < pSchemes->Count; i += 2)
		{
			pSchemes->Items[i]->LightConvert->UpdateColors(red, green, blue, tint);
		}

		paletteCount++;
	}

	if (paletteCount > 0)
	{
		int schemeCount = ColorScheme::GetNumberOfSchemes();
		Debug::LogInfo("Recalculated {} extra palettes across {} color schemes (total: {}).", paletteCount, schemeCount, schemeCount * paletteCount);
	}

	return SkipGameCode;
}


//// Set ShadeCount to 53 to initialize the palette fully shaded - this is required to make it not draw over shroud for some reason.
ASMJIT_PATCH(0x68C4C4, GenerateColorSpread_ShadeCountSet, 0x5)
{
	// some mod dont like the result of this fix
	// so toggle is added
	if (Phobos::Config::ApplyShadeCountFix)
	{
		//shade count
		if (R->EDX<int>() == 1)
			R->EDX(53);
	}

	return 0;
}

ASMJIT_PATCH(0x4C780A, EventClass_Execute_DeployEvent_NoVoiceFix, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	pThis->VoiceDeploy();
	return 0x0;
}

ASMJIT_PATCH(0x730D0F, ProcessDeployCommand_LowDeployPriority, 0x6)
{
	enum { SkipDeploy = 0x730D24 };

	GET_STACK(const int, selectedObjectCount, STACK_OFFSET(0x18, -0x4));

	if (Phobos::Config::PriorityDeployFiltering && selectedObjectCount > 1) {
		GET(TechnoClass* const, pTechno, ESI);

		auto const pExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (pExt->LowDeployPriority) {
			for (const auto pObject : ObjectClass::CurrentObjects.get()) {
				if ((pObject->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None) {
					if (!TechnoTypeExtContainer::Instance.Find(static_cast<TechnoClass*>(pObject)->GetTechnoType())->LowDeployPriority)
						return SkipDeploy;
				}
			}
		}
	}

	return 0;

	return 0;
}

ASMJIT_PATCH(0x730D1F, DeployCommandClass_Execute_VoiceDeploy, 0x5)
{
	GET_STACK(const int, unitsToDeploy, STACK_OFFSET(0x18, -0x4));

	if (unitsToDeploy != 1)
		return 0;

	GET(TechnoClass* const, pThis, ESI);

	pThis->VoiceDeploy();
	return 0x0;
}

// Fix DeployToFire not working properly for WaterBound DeploysInto buildings and not recalculating position on land if can't deploy.
ASMJIT_PATCH(0x4D580B, FootClass_ApproachTarget_DeployToFire, 0x6)
{
	enum { SkipGameCode = 0x4D583F };

	GET(UnitClass*, pThis, EBX);

	R->EAX(TechnoExtData::CanDeployIntoBuilding(pThis, true));

	return SkipGameCode;
}

#include <VeinholeMonsterClass.h>

ASMJIT_PATCH(0x5349A5, Map_ClearVectors_Veinhole, 0x5)
{
	VeinholeMonsterClass::DeleteAll();
	VeinholeMonsterClass::DeleteVeinholeGrowthData();
	return 0;
}

// Fixes a literal edge-case in passability checks to cover cells with bridges that are not accessible when moving on the bridge and
// normally not even attempted to enter but things like MapClass::NearByLocation() can still end up trying to pick.
ASMJIT_PATCH(0x4834E5, CellClass_IsClearToMove_BridgeEdges, 0x5)
{
	enum { IsNotClear = 0x48351E, Continue = 0x0 };

	GET(CellClass*, pThis, ESI);
	GET(int, level, EAX);
	GET(bool, isBridge, EBX);

	if (isBridge && pThis->ContainsBridge()
		&& (level == -1 || level == (pThis->Level + Unsorted::BridgeLevels))
		&& !(pThis->Flags & CellFlags::Unknown_200))
	{
		return IsNotClear;
	}

	return Continue;
}

// Fix a glitch related to incorrect target setting for missiles
// Author: Belonit
ASMJIT_PATCH(0x6B75AC, SpawnManagerClass_AI_SetDestinationForMissiles, 0x5)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(TechnoClass*, pSpawnTechno, EDI);

	auto const pTarget = pSpawnManager->Target;

	// Oct 27, 2025 - Starkku: Restore old behaviour for building destinations to eliminate inaccuracy issues.
	if (pTarget->WhatAmI() == AbstractType::Building) {
		pSpawnTechno->SetDestination(pTarget, true);
	} else {
		const CoordStruct coord = pSpawnManager->Target->GetCenterCoords();
		CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);
		pSpawnTechno->SetDestination(pCellDestination, true);
	}

	return 0x6B75BC;
}

DEFINE_JUMP(LJMP, 0x6E0BD4, 0x6E0BFE);
DEFINE_JUMP(LJMP, 0x6E0C1D, 0x6E0C8B);//Simplify TAction 36

#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x689EB0, ScenarioClass_ReadMap_SkipHeaderInCampaign, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EDI);

	ScenarioExtData::s_LoadFromINIFile(pItem, pINI);

	if (SessionClass::IsCampaign())
	{
		Debug::LogInfo("Skipping [Header] Section for Campaign Mode!");
		return 0x689FC0;
	}

	return  0;
}

// Skip incorrect load ctor call in various LocomotionClass_Load
DEFINE_JUMP(LJMP, 0x719CBC, 0x719CD8);//Teleport, notorious CLEG frozen state removal on loading game
DEFINE_JUMP(LJMP, 0x72A16A, 0x72A186);//Tunnel, not a big deal
DEFINE_JUMP(LJMP, 0x663428, 0x663445);//Rocket, not a big deal
DEFINE_JUMP(LJMP, 0x5170CE, 0x5170E0);//Hover, not a big deal

ASMJIT_PATCH(0x4D4B43, FootClass_Mission_Capture_ForbidUnintended, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	enum { LosesDestination = 0x4D4BD1 };

	if(pThis){
		const auto pBld = cast_to<BuildingClass*>(pThis->Destination);

		if (!pBld || pThis->Target)
			return 0;

		if (pThis->Type->Engineer)
			return 0;

		// interaction issues with Ares,
		// no more further checking to make life easier.
		// If someone still try to abuse the bug I won't try to stop them
		if (pThis->Type->Infiltrate && !pThis->Owner->IsAlliedWith(pBld->Owner))
			return 0;

		if (pBld->IsStrange())
			return 0;

		if (pBld->Type->CanBeOccupied && (pThis->Type->Occupier || TechnoExtData::IsAssaulter(pThis)))
			return 0;

		if (TechnoExtData::ISC4Holder(pThis))
			return 0;

		pThis->SetDestination(nullptr, false);
		return 0x4D4BD1;
	}

	return 0;
}

static void SetSkirmishHouseName(HouseClass* pHouse, bool IsHuman)
{
	int spawn_position = pHouse->GetSpawnPosition();

	// Default behaviour if something went wrong
	if (spawn_position < 0 || spawn_position > 7)
	{
		if (IsHuman || pHouse->IsHumanPlayer)
		{
			strncpy_s(pHouse->PlainName, GameStrings::human_player(), 14u);
		}
		else
		{
			strncpy_s(pHouse->PlainName, GameStrings::Computer_(), 8u);
		}
	}
	else
	{
		strncpy_s(pHouse->PlainName, GameStrings::PlayerAt[7u - spawn_position], 12u);
	}

	Debug::LogInfo("{}, {}, position {}", pHouse->PlainName, PhobosCRT::WideStringToString(pHouse->UIName), spawn_position);
}

//ASMJIT_PATCH(0x68804A, AssignHouses_PlayerHouses, 0x5)
//{
//	GET(HouseClass*, pPlayerHouse, EBP);
//
//	SetSkirmishHouseName(pPlayerHouse, true);
//
//	return 0x68808E;
//}

//ASMJIT_PATCH(0x688210, AssignHouses_ComputerHouses, 0x5)
//{
//	GET(HouseClass*, pAiHouse, EBP);
//
//	SetSkirmishHouseName(pAiHouse, false);
//
//	return 0x688252;
//}

// Reverted on Develop #7ad3506
// // Allow infantry to use all amphibious/water movement zones and still display sequence correctly.
// ASMJIT_PATCH(0x51D793, InfantryClass_DoAction_MovementZoneCheck, 0x6)
// {
// 	enum { Amphibious = 0x51D7A6, NotAmphibious = 0x51D8BF };
//
// 	GET(InfantryClass*, pThis, ESI);
//
// 	auto const mZone = pThis->Type->MovementZone;
//
// 	if (mZone == MovementZone::Amphibious || mZone == MovementZone::AmphibiousDestroyer || mZone == MovementZone::AmphibiousCrusher ||
// 		mZone == MovementZone::Water || mZone == MovementZone::WaterBeach)
// 	{
// 		return Amphibious;
// 	}
//
// 	return NotAmphibious;
// }

// Game removes deploying vehicles from map temporarily to check if there's enough
// space to deploy into a building when displaying allow/disallow deploy cursor.
// This can cause desyncs if there are certain types of units around the deploying unit.
// Only reasonable way to solve this is to perform the cell clear check on every client per frame
// and use that result in cursor display which is client-specific. This is now implemented in multiplayer games only.
#pragma region DeploysIntoDesyncFix
//bugged
// ASMJIT_PATCH(0x73635B, UnitClass_AI_DeploysIntoDesyncFix, 0x6)
// {
// 	if (!SessionClass::Instance->IsMultiplayer())
// 		return 0;
//
// 	GET(UnitClass*, pThis, ESI);
//
// 	if (pThis->Type->DeploysInto)
// 		TechnoExtContainer::Instance.Find(pThis)->CanCurrentlyDeployIntoBuilding = TechnoExtData::CanDeployIntoBuilding(pThis);
//
// 	return 0;
// }

ASMJIT_PATCH(0x73FEC1, UnitClass_WhatAction_DeploysIntoDesyncFix, 0x6)
{
	if (!SessionClass::Instance->IsMultiplayer())
		return 0;

	enum { SkipGameCode = 0x73FFDF };

	GET(UnitClass*, pThis, ESI);
	LEA_STACK(Action*, pAction, STACK_OFFSET(0x20, 0x8));

	if (!TechnoExtData::CanDeployIntoBuilding(pThis))
		*pAction = Action::NoDeploy;

	return SkipGameCode;
}

#pragma endregion

#include <Pipes.h>
#include <Straws.h>

struct OverlayByteReader
{
	OverlayByteReader(CCINIClass* pINI, const char* pSection)
		:
		uuLength { 0u },
		pBuffer { YRMemory::AllocateChecked(512000) },
		ls { TRUE, 0x2000 },
		bs { nullptr, 0 }
	{
		uuLength = pINI->ReadUUBlock(pSection, pBuffer, 512000);
		if (this->IsAvailable())
		{
			bs.Buffer.Buffer = pBuffer;
			bs.Buffer.Size = uuLength;
			bs.Buffer.Allocated = false;
			ls.Get_From(bs);
		}
	}

	~OverlayByteReader()
	{
		if (pBuffer)
			YRMemory::Deallocate(pBuffer);
	}

	bool IsAvailable() const { return uuLength > 0; }

	unsigned char GetByte()
	{
		if (IsAvailable())
		{
			unsigned char ret;
			ls.Get(&ret, sizeof(ret));
			return ret;
		}
		return 0;
	}

	unsigned short GetWord()
	{
		if (IsAvailable())
		{
			unsigned short ret;
			ls.Get(&ret, sizeof(ret));
			return ret;
		}

		return 0;
	}

public:
	size_t uuLength;
	void* pBuffer;
	LCWStraw ls;
	BufferStraw bs;
};

struct OverlayReader
{
	int Get()
	{
		if (ScenarioClass::NewINIFormat >= 5)
		{
			unsigned short shrt = ByteReader.GetWord();
			if (shrt != static_cast<unsigned short>(-1))
				return shrt;
		}
		else
		{
			unsigned char byte = ByteReader.GetByte();
			if (byte != static_cast<unsigned char>(-1))
				return byte;
		}

		return -1;
	}

	OverlayReader(CCINIClass* pINI)
		: ByteReader{ pINI, GameStrings::OverlayPack() }	{
	}

	~OverlayReader() = default;

private:
	OverlayByteReader ByteReader;
};

ASMJIT_PATCH(0x5FD2E0, OverlayClass_ReadINI, 0x7)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->CurrentSectionName = nullptr;
	pINI->CurrentSection = nullptr;

	if (ScenarioClass::NewINIFormat > 1)
	{

		OverlayReader reader(pINI);

		for (short i = 0; i < 0x200; ++i)
		{
			for (short j = 0; j < 0x200; ++j)
			{
				CellStruct mapCoord { j,i };
				int nOvl = reader.Get();

				if (nOvl != 0xFFFFFFFF)
				{
					auto const pType = OverlayTypeClass::Array->operator[](nOvl);

					if (pType->GetImage() || pType->CellAnim)
					{
						if (SessionClass::Instance->GameMode != GameMode::Campaign && pType->Crate)
							continue;
						if (!MapClass::Instance->CoordinatesLegal(mapCoord))
							continue;

						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						auto const nOriginOvlData = pCell->OverlayData;
						GameCreate<OverlayClass>(pType, mapCoord, -1);
						if (nOvl == 24 || nOvl == 25 || nOvl == 237 || nOvl == 238) // bridges
							pCell->OverlayData = nOriginOvlData;
					}
				}
			}
		}

		auto pBuffer = YRMemory::AllocateChecked(256000);
		size_t uuLength = pINI->ReadUUBlock(GameStrings::OverlayDataPack(), pBuffer, 256000);

		if (uuLength > 0)
		{
			BufferStraw bs(pBuffer, uuLength);
			LCWStraw ls(TRUE, 0x2000);
			ls.Get_From(bs);

			for (short i = 0; i < 0x200; ++i)
			{
				for (short j = 0; j < 0x200; ++j)
				{
					CellStruct mapCoord { j,i };
					unsigned char buffer;
					ls.Get(&buffer, sizeof(buffer));
					if (MapClass::Instance->CoordinatesLegal(mapCoord))
					{
						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						pCell->OverlayData = buffer;
					}
				}
			}
		}

		if (pBuffer)
			YRMemory::Deallocate(pBuffer);
	}

	AbstractClass::RemoveAllInactive();

	return 0x5FD69A;
}

struct OverlayByteWriter
{
	OverlayByteWriter(const char* pSection, size_t nBufferLength)
		: lpSectionName { pSection }, uuLength { 0 }, bp { nullptr,0 }, lp { FALSE,0x2000 }
	{
		this->Buffer = YRMemory::AllocateChecked(nBufferLength);
		bp.Buffer.Buffer = this->Buffer;
		bp.Buffer.Size = nBufferLength;
		bp.Buffer.Allocated = false;
		lp.Put_To(bp);
	}

	~OverlayByteWriter()
	{
		YRMemory::Deallocate(this->Buffer);
	}

	void PutByte(unsigned char data)
	{
		uuLength += lp.Put(&data, sizeof(data));
	}

	void PutWord(unsigned short data)
	{
		uuLength += lp.Put(&data, sizeof(data));
	}

	void PutBlock(CCINIClass* pINI)
	{
		pINI->Clear(this->lpSectionName, nullptr);
		pINI->WriteUUBlock(this->lpSectionName, this->Buffer, uuLength);
	}

	const char* lpSectionName;
	size_t uuLength;
	void* Buffer;
	BufferPipe bp;
	LCWPipe lp;
};

struct OverlayWriter
{
	OverlayWriter(size_t nLen)
		: ByteWriter { GameStrings::OverlayPack() , nLen }
	{ }

	void Put(int nOverlay)
	{
		if (ScenarioClass::NewINIFormat >= 5)
			ByteWriter.PutByte(static_cast<unsigned char>(nOverlay));
		else
			ByteWriter.PutWord(static_cast<unsigned short>(nOverlay));
	}

	void PutBlock(CCINIClass* pINI)
	{
		ByteWriter.PutBlock(pINI);
	}

private:
	OverlayByteWriter ByteWriter;
};

ASMJIT_PATCH(0x5FD6A0, OverlayClass_WriteINI, 0x6)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->Clear("OVERLAY", nullptr);

	size_t len = DSurface::Alternate->Width * DSurface::Alternate->Height;
	OverlayWriter writer(len);
	OverlayByteWriter datawriter(GameStrings::OverlayDataPack(), len);

	for (short i = 0; i < 0x200; ++i)
	{
		for (short j = 0; j < 0x200; ++j)
		{
			CellStruct mapCoord { j,i };
			auto const pCell = MapClass::Instance->GetCellAt(mapCoord);
			writer.Put(pCell->OverlayTypeIndex);
			datawriter.PutByte(pCell->OverlayData);
		}
	}

	writer.PutBlock(pINI);
	datawriter.PutBlock(pINI);

	return 0x5FD8EB;
}


// Ares InitialPayload fix: Man, what can I say
// Otamaa : this can cause deadlock , or crashes , better write proper fix
// ASMJIT_PATCH(0x65DE21, TeamTypeClass_CreateMembers_MutexOut, 0x6)
// {
// 	GET(TeamClass*, pTeam, EBP);
// 	GET(TechnoTypeClass*, pType, EDI);
// 	R->ESI(pType->CreateObject(pTeam->Owner));
// 	return 0x65DE53;
// }

ASMJIT_PATCH(0x74691D, UnitClass_UpdateDisguise_EMP, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	// Remove mirage disguise if under emp or being flipped, approximately 15 deg
	if (pThis->Deactivated
		|| pThis->IsUnderEMP()
		|| TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled
		|| Math::abs(pThis->AngleRotatedForwards) > 0.25
		|| Math::abs(pThis->AngleRotatedSideways) > 0.25)
	{
		pThis->ClearDisguise();
		R->Stack(0x7 , false);
	}

	return 0x0;
}

#include <Misc/Hooks.Otamaa.h>

bool FakeHouseClass::_IsAlliedWith(HouseClass* pOther)
{
	return (Phobos::Config::DevelopmentCommands && SessionClass::IsSingleplayer())
		|| this->ControlledByCurrentPlayer()
		|| this->IsAlliedWith(pOther);
}

DEFINE_FUNCTION_JUMP(CALL, 0x63B136, FakeHouseClass::_IsAlliedWith);
DEFINE_FUNCTION_JUMP(CALL, 0x63B100, FakeHouseClass::_IsAlliedWith);
DEFINE_FUNCTION_JUMP(CALL, 0x63B17F, FakeHouseClass::_IsAlliedWith);
DEFINE_FUNCTION_JUMP(CALL, 0x63B1BA, FakeHouseClass::_IsAlliedWith);
DEFINE_FUNCTION_JUMP(CALL, 0x63B2CE, FakeHouseClass::_IsAlliedWith);

// An attempt to fix an issue where the ATC->CurrentVector does not contain every air Techno in given range that increases in frequency as the range goes up.
// Real talk: I have absolutely no clue how the original function works besides doing vector looping and manipulation, as far as I can tell it never even explicitly
// clears CurrentVector but somehow it only contains applicable items afterwards anyway. It is possible this one does not achieve everything the original does functionality and/or
// performance-wise but it does work and produces results with greater accuracy than the original for large ranges. - Starkku
ASMJIT_PATCH(0x412B40, AircraftTrackerClass_FillCurrentVector, 0x5)
{
	enum { SkipGameCode = 0x413482 };

	GET(AircraftTrackerClass*, pThis, ECX);
	GET_STACK(CellClass*, pCell, 0x4);
	GET_STACK(int, range, 0x8);

	pThis->CurrentVector.clear();

	if (range < 1)
		range = 1;

	const CellStruct mapCoords = pCell->MapCoords;
	const int sectorWidth = MapClass::MapCellDimension->Width / 20;
	const int sectorHeight = MapClass::MapCellDimension->Height / 20;
	const int sectorIndexXStart = std::clamp((mapCoords.X - range) / sectorWidth, 0, 19);
	const int sectorIndexYStart = std::clamp((mapCoords.Y - range) / sectorHeight, 0, 19);
	const int sectorIndexXEnd = std::clamp((mapCoords.X + range) / sectorWidth, 0, 19);
	const int sectorIndexYEnd = std::clamp((mapCoords.Y + range) / sectorHeight, 0, 19);

	for (int y = sectorIndexYStart; y <= sectorIndexYEnd; y++) {
		for (int x = sectorIndexXStart; x <= sectorIndexXEnd; x++) {
			for (auto const pTechno : pThis->TrackerVectors[y][x]) {
				pThis->CurrentVector.push_back(pTechno);
			}
		}
	}

	return SkipGameCode;
}

#ifdef aaaaa___
#pragma region BlitterFix_
#include <Helpers/Macro.h>
#include <stdint.h>

// Half of an RGB range in 16 bit
// (equivalent of (127, 127, 127))
#define HALF_RANGE_MASK 0x7BEFu

// Blending mask for 16-bit pixels
// https://medium.com/@luc.trudeau/fast-averaging-of-high-color-16-bit-pixels-cb4ac7fd1488

#define BLENDING_MASK 0xF7DEu

static OPTIONALINLINE uint16_t  Blit50TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	return (((src ^ dst) & BLENDING_MASK) >> 1) + (src & dst);
}

static OPTIONALINLINE uint16_t  Blit75TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	uint16_t  div = Blit50TranslucencyFix(dst, src);
	return (div >> 1 & HALF_RANGE_MASK) + (dst >> 1 & HALF_RANGE_MASK);
}

//same as 75, just reversed order of args
static OPTIONALINLINE uint16_t  Blit25TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	return Blit75TranslucencyFix(src, dst);
}

#undef HALF_RANGE_MASK
#undef BLENDING_MASK

// =============================
// container hooks

ASMJIT_PATCH(492866, BlitTransLucent50_Fix, 0)
{
	GET(uint16_t, color, EAX);
	GET(uint16_t*, dest, EDI);

	*dest = Blit50TranslucencyFix(*dest, color);

	return 0x492878;
}

ASMJIT_PATCH(492956, BlitTransLucent25_Fix, 0)
{
	GET(uint16_t, color, EAX);
	GET(uint16_t*, dest, ESI);

	*dest = Blit25TranslucencyFix(*dest, color);

	return 0x49296D;
}

ASMJIT_PATCH(492776, BlitTransLucent75_Fix, 0)
{
	GET(uint16_t, color, EBP);
	GET(uint16_t*, dest, ESI);

	*dest = Blit75TranslucencyFix(*dest, color);

	return 0x49278D;
}
#pragma endregion
#endif

//ASMJIT_PATCH(0x51C9B8, InfantryClass_CanFire_HitAndRun1, 0x17)
//{
//	enum { CheckPass = 0x51C9CF, CheckNotPass = 0x51CAFA };
//
//	GET(InfantryClass*, pThis, EBX);
//
//	const auto pType = pThis->GetTechnoType();
//	if ((pThis->CanAttackOnTheMove() && pType && pType->OpportunityFire)
//		|| pThis->SpeedPercentage <= 0.1) // vanilla check
//	{
//		return CheckPass;
//	}
//	else
//	{
//		return CheckNotPass;
//	}
//}
//
//ASMJIT_PATCH(0x51CAAC, InfantryClass_CanFire_HitAndRun2, 0x13)
//{
//	enum { CheckPass = 0x51CACD, CheckNotPass = 0x51CABF };
//
//	GET(InfantryClass*, pThis, EBX);
//
//	const auto pType = pThis->GetTechnoType();
//	if ((pThis->CanAttackOnTheMove() && pType && pType->OpportunityFire)
//		|| !pThis->Locomotor.GetInterfacePtr()->Is_Moving_Now()) // vanilla check
//	{
//		return CheckPass;
//	}
//	else
//	{
//		return CheckNotPass;
//	}
//}

// this fella was { 0, 0, 1 } before and somehow it also breaks both the light position a bit and how the lighting is applied when voxels rotate - Kerbiter
ASMJIT_PATCH(0x753D86, VoxelCalcNormals_NullAdditionalVector, 0x8)
{
	REF_STACK(Vector3D<float>, secondaryLightVector, STACK_OFFSET(0xD8, -0xC0))

		if (RulesExtData::Instance()->UseFixedVoxelLighting)
			secondaryLightVector = { 0, 0, 0 };
		else
			secondaryLightVector = { 0, 0, 1 };

	return 0x753D9E;
}

//DEFINE_PATCH(0x753D96, 0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00);

#include <Misc/Ares/Hooks/Header.h>

ASMJIT_PATCH(0x705D74, TechnoClass_GetRemapColour_DisguisePalette, 0x8)
{
	enum { SkipGameCode = 0x705D7C };

	GET(TechnoClass* const, pThis, ESI);

	R->EAX(TechnoExtData::GetSimpleDisguiseType(pThis, true, false));

	return SkipGameCode;
}

ASMJIT_PATCH(0x467C1C, BulletClass_AI_UnknownTimer, 0x6)
{
	GET(BulletTypeClass*, projectile, EAX);
	return projectile->Inviso ? 0x467C2A : 0;
}

ASMJIT_PATCH(0x51A67E, InfantryClass_UpdatePosition_DamageBridgeFix, 0x6)
{
	enum { SkipDamageArea = 0x51A7F8 };

	GET(CellClass*, pCell, EAX);

	return pCell->ContainsBridge() ? 0 : SkipDamageArea;
}

#include <Ext/AnimType/Body.h>
#include <EventClass.h>

#pragma region FrameCRC

static bool IsHashable(ObjectClass* pObj)
{
	auto const rtti = pObj->WhatAmI();
	switch (rtti)
	{
	case AbstractType::Anim:
	{
		if (pObj->UniqueID == -2)
			return false;

		auto const pAnim = static_cast<AnimClass*>(pObj);
		auto const pType = pAnim->Type;

		if (pType->Damage != 0.0 || pType->Bouncer || pType->IsMeteor || pType->IsTiberium || pType->TiberiumChainReaction
			|| pType->IsAnimatedTiberium || pType->MakeInfantry != -1 || AnimTypeExtContainer::Instance.Find(pType)->CreateUnitType)
		{
			return true;
		}

		return false;
	}
		case AbstractType::Particle:
	{
		auto const pParticle = static_cast<ParticleClass*>(pObj);
		return pParticle->Type->Damage;
	}
	default:
		break;
	}

	return true;
}

static COMPILETIMEEVAL void FORCEDINLINE AddCRC(DWORD* crc, unsigned int val)
{
	*crc = val + (*crc >> 31) + (*crc << 1);
}

static COMPILETIMEEVAL int FORCEDINLINE GetCoordHash(CoordStruct location)
{
	return location.X / 10 + ((location.Y / 10) << 16);
}

static void __fastcall ComputeGameCRC()
{
	EventClass::CurrentFrameCRC = 0;

	for (auto const pInf : *InfantryClass::Array)
	{
		int primaryFacing = pInf->PrimaryFacing.Current().GetValue<8>();
		AddCRC(EventClass::CurrentFrameCRC.operator->(), GetCoordHash(pInf->Location) + primaryFacing);
	}

	for (auto const pUnit : *UnitClass::Array)
	{
		int primaryFacing = pUnit->PrimaryFacing.Current().GetValue<8>();
		int secondaryFacing = pUnit->SecondaryFacing.Current().GetValue<8>();
		AddCRC(EventClass::CurrentFrameCRC.operator->(), GetCoordHash(pUnit->Location) + primaryFacing + secondaryFacing);
	}

	for (auto const pBuilding : *BuildingClass::Array)
	{
		int primaryFacing = pBuilding->PrimaryFacing.Current().GetValue<8>();
		AddCRC(EventClass::CurrentFrameCRC.operator->(), GetCoordHash(pBuilding->Location) + primaryFacing);
	}

	for (auto const pHouse : *HouseClass::Array)
	{
		AddCRC(EventClass::CurrentFrameCRC.operator->(), pHouse->MapIsClear);
	}

	for (int i = 0; i < 5; i++)
	{
		auto const layer = DisplayClass::GetLayer((Layer)i);

		for (auto const pObj : *layer)
		{
			if (IsHashable(pObj))
				AddCRC(EventClass::CurrentFrameCRC.operator->(), GetCoordHash(pObj->Location) + (int)pObj->WhatAmI());
		}
	}

	for (auto const pObj : MapClass::Logics.get())
	{
		if (IsHashable(pObj))
			AddCRC(EventClass::CurrentFrameCRC.operator->(), GetCoordHash(pObj->Location) + (int)pObj->WhatAmI());
	}

	AddCRC(EventClass::CurrentFrameCRC.operator->(), ScenarioClass::Instance->Random.Random());
	Game::LogFrameCRC(Unsorted::CurrentFrame % 256);
}

DEFINE_FUNCTION_JUMP(CALL, 0x64731C, ComputeGameCRC);
DEFINE_FUNCTION_JUMP(CALL, 0x647684, ComputeGameCRC);

#pragma endregion

// Fix TechnoTypeClass::CanBeHidden

//DEFINE_PATCH(0x711229, 0xC6, 0x86, 0x24, 0x07, 0x00, 0x00, 0x00) // TechnoTypeClass::CTOR
//DEFINE_PATCH(0x6FA2AA, 0x75, 0x2E) // TechnoClass::AI
//
//ASMJIT_PATCH(0x7121EB, TechnoTypeClass_LoadFromINI_CanBeHidden, 0x6)
//{
//	enum { SkipGameCode = 0x712208 };
//
//	GET(TechnoTypeClass*, pTechnoType, EBP);
//	pTechnoType->CanBeHidden = CCINIClass::INI_Art->ReadBool(pTechnoType->ImageFile, "CanBeHidden", pTechnoType->CanBeHidden);
//
//	return SkipGameCode;
//}

ASMJIT_PATCH(0x71464A, TechnoTypeClass_ReadINI_Speed, 0x7)
{
	enum { SkipGameCode = 0x71469F };

	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	GET(char*, pSection, EBX);
	GET(int, eliteAirstrikeRechargeTime, EAX);

	// Restore overridden instructions.
	pThis->EliteAirstrikeRechargeTime = eliteAirstrikeRechargeTime;

	INI_EX exINI(pINI);
	exINI.ReadSpeed(pSection, "Speed", &pThis->Speed);

	return SkipGameCode;
}


// In the following three places the distance check was hardcoded to compare with 20, 17 and 16 respectively,
// which means it didn't consider the actual speed of the unit. Now we check it and the units won't get stuck
// even at high speeds - NetsuNegi
ASMJIT_PATCH(0x72958E, TunnelLocomotionClass_ProcessDigging_SlowdownDistance, 0x8) {
	enum { KeepMoving = 0x72980F, CloseEnough = 0x7295CE };

	//this fix reqire change of `pType->Speed`
	//which is ridicculus really - Otamaa
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	auto& currLoc = pLoco->LinkedTo->Location;
	int distance = (int) CoordStruct{currLoc.X - pLoco->_CoordsNow.X, currLoc.Y - pLoco->_CoordsNow.Y,0}.Length() ;

	auto const pTypeExt = GET_TECHNOTYPEEXT(pLoco->LinkedTo);
	int currentSpeed = pTypeExt->SubterraneanSpeed >= 0 ?
			pTypeExt->SubterraneanSpeed : RulesExtData::Instance()->SubterraneanSpeed;

	// Calculate speed multipliers.
	pLoco->LinkedTo->SpeedPercentage = 1.0; // Subterranean locomotor doesn't normally use this so it would be 0.0 here and cause issues.		int maxSpeed = pTypeExt->This()->Speed;
	int maxSpeed = pTypeExt->This()->Speed;
	pTypeExt->This()->Speed = currentSpeed;
	currentSpeed = pLoco->LinkedTo->GetCurrentSpeed();
	pTypeExt->This()->Speed = maxSpeed;

	if (distance > currentSpeed)
	{
		REF_STACK(CoordStruct, newLoc, STACK_OFFSET(0x40, -0xC));
		double angle = -std::atan2((float)(currLoc.Y - pLoco->_CoordsNow.Y), (float)(pLoco->_CoordsNow.X - currLoc.X));
		newLoc = currLoc + CoordStruct { int((double)currentSpeed * Math::cos(angle)), int((double)currentSpeed * Math::sin(angle)), 0 };
		return 0x7298D3;
	}
	return 0x7295CE;
}

ASMJIT_PATCH(0x75BD70, WalkLocomotionClass_ProcessMoving_SlowdownDistance, 0x9) {
	enum { KeepMoving = 0x75BF85, CloseEnough = 0x75BD79 };

	GET(FootClass* const, pLinkedTo, ECX);
	GET(int const, distance, EAX);
	return distance >= pLinkedTo->GetCurrentSpeed() ? KeepMoving : CloseEnough;
	//return distance < 17 ? CloseEnough : KeepMoving;
}

ASMJIT_PATCH(0x5B11DD, MechLocomotionClass_ProcessMoving_SlowdownDistance, 0x9) {
	enum { KeepMoving = 0x5B14AA, CloseEnough = 0x5B11E6 };

	GET(FootClass* const, pLinkedTo, ECX);
	GET(int const, distance, EAX);
	return distance >= pLinkedTo->GetCurrentSpeed() ? KeepMoving : CloseEnough;
	//return distance < 16 ? CloseEnough : KeepMoving;
}

// Apply cell lighting on UseNormalLight=no MakeInfantry anims.
ASMJIT_PATCH(0x4232BF, AnimClass_DrawIt_MakeInfantry, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (pThis->Type->MakeInfantry != -1) {
		R->EAX(pThis->GetCell()->Color1.Red);
		return 0x4232C5;
	}

	return 0;
}


// Map <Player @ X> as object owner name to correct HouseClass index.
ASMJIT_PATCH(0x50C186, GetHouseIndexFromName_PlayerAtX, 0x6)
{
	enum { ReturnFromFunction = 0x50C203 };
	GET(const char*, name, ECX);

	// Bail out early in campaign mode or if the name does not start with <
	if (SessionClass::IsCampaign() || *name != '<')
		return 0;

	int playerAtIndex = HouseClass::GetPlayerAtFromString(name);

	if (playerAtIndex != -1) {
		if (auto const pHouse = HouseClass::FindByPlayerAt(playerAtIndex)) {
			R->EDX(pHouse->ArrayIndex);
			return ReturnFromFunction;
		}
	}

	return 0;
}

// Skip check that prevents buildings from being created for local player.
DEFINE_JUMP(LJMP, 0x44F8D5, 0x44F8E1);

// Starkku: These fix issues with follower train cars etc) indices being thrown off by preplaced vehicles not being created, having other vehicles as InitialPayload etc.
// This fix basically works by not using the global UnitClass array at all for setting the followers, only a list of preplaced units, successfully created or not.
#pragma region Follower
namespace UnitParseTemp
{
	std::vector<UnitClass*> ParsedUnits;
	bool WasCreated = false;
}

// Add vehicles successfully created to list of parsed vehicles.
ASMJIT_PATCH(0x7435DE, UnitClass_ReadFromINI_Follower1, 0x6)
{
	GET(UnitClass*, pUnit, ESI);
	UnitParseTemp::ParsedUnits.push_back(pUnit);
	UnitParseTemp::WasCreated = true;
	return 0;
}

// Add vehicles that were not successfully created to list of parsed vehicles as well as to followers list.
ASMJIT_PATCH(0x74364C, UnitClass_ReadFromINI_Follower2, 0x8)
{
	REF_STACK(TypeList<int>, followers, STACK_OFFSET(0xD0, -0xC0));
	if (!UnitParseTemp::WasCreated)
	{
		followers.push_back(-1);
		UnitParseTemp::ParsedUnits.push_back(nullptr);
	}
	UnitParseTemp::WasCreated = false;
	return 0;
}

// Set followers based on parsed vehicles.
ASMJIT_PATCH(0x743664, UnitClass_ReadFromINI_Follower3, 0x6)
{
	enum { SkipGameCode = 0x7436AC };
	REF_STACK(TypeList<int>, followers, STACK_OFFSET(0xCC, -0xC0));
	auto& units = UnitParseTemp::ParsedUnits;
	for (size_t i = 0; i < units.size(); i++)
	{
		auto const pUnit = units[i];
		if (!pUnit)
			continue;
		int followerIndex = followers[i];
		if (followerIndex < 0 || followerIndex >= static_cast<int>(units.size()))
		{
			pUnit->FollowerCar = nullptr;
		}
		else
		{
			auto const pFollower = units[followerIndex];
			pUnit->FollowerCar = pFollower;
			pFollower->IsFollowerCar = true;
		}
	}
	units.clear();
	return SkipGameCode;
}
#pragma endregion

#pragma region End_Piggyback PowerOn
// Author: tyuah8

NOINLINE LocomotionClass* getILoco(REGISTERS* R) {

	ILocomotion* pIloco = nullptr;

	if (R->Origin() == 0x719F17)
		pIloco = R->ECX<ILocomotion*>();
	else
		pIloco = R->EAX<ILocomotion*>();

	return static_cast<LocomotionClass*>(pIloco);
}

//TODO :Evaluate these bullshit
ASMJIT_PATCH(0x4AF94D, LocomotionClass_End_Piggyback_PowerOn, 0x7)//Drive
{
	if(const auto pLoco = getILoco(R)) {
		if(auto pLinkedTo = pLoco->LinkedTo ? pLoco->LinkedTo : pLoco->Owner){
			if (!pLinkedTo->Deactivated && !pLinkedTo->IsUnderEMP())
				pLoco->Power_On();
			else
				pLoco->Power_Off();
		}
	}
	return 0;
}ASMJIT_PATCH_AGAIN(0x719F17, LocomotionClass_End_Piggyback_PowerOn, 0x5)//Teleport
ASMJIT_PATCH_AGAIN(0x69F05D, LocomotionClass_End_Piggyback_PowerOn, 0x7) //Ship
ASMJIT_PATCH_AGAIN(0x54DADC, LocomotionClass_End_Piggyback_PowerOn, 0x5)//Jumpjet


#pragma endregion

ASMJIT_PATCH(0x4C75DA, EventClass_RespondToEvent_Stop, 0x6)
{
	enum { SkipGameCode = 0x4C762A };

	GET(TechnoClass* const, pTechno, ESI);

	// Check aircraft
	const auto pAircraft = cast_to<AircraftClass* , false>(pTechno);
	const bool commonAircraft = pAircraft && !pAircraft->Airstrike && !pAircraft->Spawned;
	const auto mission = pTechno->CurrentMission;

	// To avoid aircraft overlap by keep link if is returning or is in airport now.
	if (!commonAircraft || (mission != Mission::Sleep && mission != Mission::Guard && mission != Mission::Enter)
		|| !pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink()))
	{
		pTechno->SendToEachLink(RadioCommand::NotifyUnlink);
	}

	// To avoid technos being unable to stop in attack move mega mission
	if (pTechno->MegaMissionIsAttackMove())
		pTechno->ClearMegaMissionData();

	// Clearing the current target should still be necessary for all technos
	pTechno->SetTarget(nullptr);

	// Stop any enter action
	pTechno->QueueUpToEnter = nullptr;

	if (commonAircraft)
	{
		if (pAircraft->Type->AirportBound)
		{
			// To avoid `AirportBound=yes` aircraft with ammo at low altitudes cannot correctly receive stop command and queue Mission::Guard with a `Destination`.
			if (pAircraft->Ammo)
				pTechno->SetDestination(nullptr, true);

			// To avoid `AirportBound=yes` aircraft pausing in the air and let they returning to air base immediately.
			if (!pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink())) // If the aircraft have no valid dock, try to find a new one
				pAircraft->EnterIdleMode(false, true);
		}
		else if (pAircraft->Ammo)
		{
			// To avoid `AirportBound=no` aircraft ignoring the stop task or directly return to the airport.
			if (pAircraft->Destination && static_cast<int>(CellClass::Coord2Cell(pAircraft->Destination->GetCoords()).DistanceFromSquared(pAircraft->GetMapCoords())) > 2) // If the aircraft is moving, find the forward cell then stop in it
				pAircraft->SetDestination(pAircraft->GetCell()->GetNeighbourCell(static_cast<FacingType>(pAircraft->PrimaryFacing.Current().GetValue<3>())), true);
		}
		else if (!pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink()))
		{
			pAircraft->EnterIdleMode(false, true);
		}
		// Otherwise landing or idling normally without answering the stop command
	}
	else
	{
		const auto pFoot = flag_cast_to<FootClass* , false>(pTechno);

		// Clear archive target for infantries and vehicles like receive a mega mission
		if (pFoot && !pAircraft)
			pTechno->SetArchiveTarget(nullptr);

		// Only stop when it is not under the bridge (meeting the original conditions which has been skipped)
		if (!pTechno->vt_entry_2B0() || pTechno->OnBridge || pTechno->IsInAir() || pTechno->GetCell()->SlopeIndex)
		{
			// To avoid foots stuck in Mission::Area_Guard
			if (pTechno->CurrentMission == Mission::Area_Guard 
					&& !GET_TECHNOTYPE(pTechno)->DefaultToGuardArea)
				pTechno->QueueMission(Mission::Guard, true);

			// Check Jumpjets
			const auto pJumpjetLoco = pFoot ? locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor) : nullptr;

			// To avoid jumpjets falling into a state of standing idly by
			if (!pJumpjetLoco) // If is not jumpjet, clear the destination is enough
				pTechno->SetDestination(nullptr, true);
			else if (!pFoot->Destination) // When in attack move and have had a target, the destination will be cleaned up, enter the guard mission can prevent the jumpjets stuck in a status of standing idly by
				pTechno->QueueMission(Mission::Guard, true);
			else if (static_cast<int>(CellClass::Coord2Cell(pFoot->Destination->GetCoords()).DistanceFromSquared(pTechno->GetMapCoords())) > 2) // If the jumpjet is moving, find the forward cell then stop in it
				pTechno->SetDestination(pTechno->GetCell()->GetNeighbourCell(static_cast<FacingType>(pJumpjetLoco->Facing.Current().GetValue<3>())), true);

			// Otherwise landing or idling normally without answering the stop command
		}
	}

	return SkipGameCode;
}

size_t __fastcall Gamestrtohex(char* str) {
	JMP_FAST(0x412610);
}

#include <TaskForceClass.h>

// Suppress Ares' swizzle warning
static size_t __fastcall HexStr2Int_replacement(const char* str) {
	// Fake a pointer to trick Ares
	return std::hash<std::string_view>{}(str) & 0xFFFFFF;
}

ASMJIT_PATCH(0x6E5FA3, HexStr2Int_replacement_logTagType, 0x8) {
	GET(char*, HexID, EDI);
	//GET(TagTypeClass*, pType, ESI);

	size_t ID = Gamestrtohex(HexID);
	Debug::Log("TagType[%s] want to remap as [%x] \n", HexID, ID);
	//PhobosSwizzle::Instance.Here_I_Am((void*)ID, pType);

	return 0x6E5FB6;
}

ASMJIT_PATCH(0x6E8300, HexStr2Int_replacement_logTaskForce, 0xA)
{
	LEA_STACK(char*, HexID, 0x18);
	//GET(TaskForceClass*, pType, ESI);

	size_t ID = Gamestrtohex(HexID);
	Debug::Log("TaskForce[%s] want to remap as [%x] \n", HexID , ID);
	//PhobosSwizzle::Instance.Here_I_Am((void*)ID, pType);

	return 0x6E8315;
}

//DEFINE_FUNCTION_JUMP(CALL, 0x6E8305, HexStr2Int_replacement); // TaskForce
//DEFINE_FUNCTION_JUMP(CALL, 0x6E5FA6, HexStr2Int_replacement); // TagType

// Save GameModeOptions in campaign modes
DEFINE_JUMP(LJMP, 0x67E3BD, 0x67E3D3); // Save
DEFINE_JUMP(LJMP, 0x67F72E, 0x67F744); // Load
DEFINE_JUMP(LJMP, 0x65B3F7, 0x65B416);//RadSite, no effect

#pragma region TeamCloseRangeFix

  static int __fastcall Check2DDistanceInsteadOf3D(ObjectClass* pSource, void* _, AbstractClass* pTarget)
  {
  	return (pSource->IsInAir() && pSource->WhatAmI() != AbstractType::Aircraft) // Jumpjets or sth in the air
  		? pSource->DistanceFrom(pTarget) // 2D distance
  		: pSource->DistanceFromSquared(pTarget); // 3D distance (vanilla)
  }
  DEFINE_FUNCTION_JUMP(CALL, 0x6EBCC9, Check2DDistanceInsteadOf3D);

#pragma endregion

// ASMJIT_PATCH(0x6B7CC1, SpawnManagerClass_Detach_ExitGame, 0x7)
// {
// 	GET(SpawnManagerClass*, pThis, ESI);
//
// 	if (Phobos::Otamaa::ExeTerminated)
// 		return 0x6B7CCF;
//
// 	pThis->KillNodes();
// 	pThis->ResetTarget();
// 	return 0x6B7CCF;
// }

ASMJIT_PATCH(0x71872C, TeleportLocomotionClass_MakeRoom_OccupationFix, 0x9)
{
	enum { SkipMarkOccupation = 0x71878F };

	GET(const LocomotionClass* const, pLoco, EBP);

	const auto pFoot = pLoco->LinkedTo;

	return (pFoot && !pFoot->InLimbo && pFoot->IsAlive && pFoot->Health > 0 && !pFoot->IsSinking) ? 0 : SkipMarkOccupation;
}

// Fix a crash at 0x7BAEA1 when trying to access a point outside of surface bounds.
class NOVTABLE FakeXSurface final : public XSurface {
public:

	int _GetPixel(Point2D const& point) const {
		int color = 0;

		Point2D finalPoint = point;

		if (finalPoint.X > Width || finalPoint.Y > Height)
			finalPoint = Point2D::Empty;

		void* pointer = ((Surface*)this)->Lock(finalPoint.X, finalPoint.Y);

		if (pointer != nullptr)
		{

			if (BytesPerPixel == 2)
				color = *static_cast<unsigned short*>(pointer);
			else
				color = *static_cast<unsigned char*>(pointer);

			((Surface*)this)->Unlock();

		}

		return color;
	}
};
static_assert(sizeof(XSurface) == sizeof(FakeXSurface), "Invalid Size !");

DEFINE_FUNCTION_JUMP(CALL, 0x4A3E8A, FakeXSurface::_GetPixel)
DEFINE_FUNCTION_JUMP(CALL, 0x4A3EB7, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x4A3F7C, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x642213, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x6423D6, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2098, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E212C, FakeXSurface::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E85FC, FakeXSurface::_GetPixel);

// IsSonic wave drawing uses fixed-size arrays accessed with index that is determined based on factors like wave lifetime,
// distance of pixel from start coords etc. The result is that at certain distance invalid memory is being accessed leading to crashes.
// Easiest solution to this is simply clamping the final color index so that no memory beyond the size 14 color data buffer in WaveClass
// is being accessed with it. Last index of color data is uninitialized, changing that or trying to access it just results in glitchy behaviour
// so the cutoff is at 12 here instead of 13.
ASMJIT_PATCH(0x75EE49, WaveClass_DrawSonic_CrashFix, 0x7)
{
	GET(int, colorIndex, EAX);

	if (colorIndex > 12)
		R->EAX(12);

	return 0;

}

ASMJIT_PATCH(0x73AE70, UnitClass_UpdatePosition_Bridge, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	return pThis->OnBridge
	&& GroundType::Array[static_cast<int>(LandType::Road)].
	Cost[static_cast<int>(pThis->Type->SpeedType)] == 0.0f ? 0x73AEB4 : 0;
}

// Change enter to move when unlink
ASMJIT_PATCH(0x6F4C50, TechnoClass_ReceiveCommand_NotifyUnlink, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(TechnoClass* const, pCall, STACK_OFFSET(0x18, 0x4));
	// If the link connection is cancelled and foot A is entering techno B, it may cause A and B to overlap
	if (!pCall->InLimbo // Has not already entered
		&& (pCall->AbstractFlags & AbstractFlags::Foot) // Is foot
		&& pCall->CurrentMission == Mission::Enter // Is entering
		&& static_cast<FootClass*>(pCall)->Destination == pThis // Is entering techno B
		&& pCall->WhatAmI() != AbstractType::Aircraft // Not aircraft
		&& GET_TECHNOTYPE(pThis)->Passengers > 0) // Have passenger seats
	{
		pCall->SetDestination(pThis->GetCell(), false); // Set the destination at its feet
		pCall->QueueMission(Mission::Move, false); // Replace entering with moving
		pCall->NextMission(); // Immediately respond to the Mission::Move
	}

	return 0;
}

// Radio: do not untether techno who have other tether link
ASMJIT_PATCH(0x6F4BB3, TechnoClass_ReceiveCommand_RequestUntether, 0x7)
{
	// Place the hook after processing to prevent functions from calling each other and getting stuck in a dead loop.
	GET(TechnoClass* const, pThis, ESI);
	// The radio link capacity of some technos can be greater than 1 (like airport)
	// Here is a specific example, there may be other situations as well:
	// - Untether without check may result in `AirportBound=no` aircraft being unable to release from `IsTether` status.
	// - Specifically, all four aircraft are connected to the airport and have `RadioLink` settings, but when the first aircraft
	//   is `Unlink` from the airport, all subsequent aircraft will be stuck in `IsTether` status.
	// - This is because when both parties who are `RadioLink` to each other need to `Unlink`, they need to `Untether` first,
	//   and this requires ensuring that both parties have `IsTether` flag (0x6F4C50), otherwise `Untether` cannot be successful,
	//   which may lead to some unexpected situations.
	if(!pThis->IsTethered) {
		for (int i = 0; i < pThis->RadioLinks.Capacity; ++i) {
			if (const auto pLink = pThis->RadioLinks.Items[i]) {
				if (pLink->IsTethered) // If there's another tether link, reset flag to true
					pThis->IsTethered = true; // Ensures that other links can be properly untether afterwards
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6FC617, TechnoClass_CanFire_AirCarrierSkipCheckNearBridge, 0x8)
{
	enum { ContinueCheck = 0x6FC61F, TemporaryCannotFire = 0x6FCD0E };

	GET(TechnoClass* const, pThis, ESI);
	GET(const bool, nearBridge, EAX);

	return (nearBridge && !pThis->IsInAir()) ? TemporaryCannotFire : ContinueCheck;
}

// WW used SetDesired here, causing the barrel drawn incorrectly.
ASMJIT_PATCH(0x6F6DEE, TechnoClass_Unlimbo_BarrelFacingBugFix, 0x7)
{
	enum { SkipGameCode = 0x6F6DFA };

	GET(DirStruct*, pDir, ECX);
	GET(TechnoClass*, pThis, ESI);

	pThis->BarrelFacing.Set_Current(*pDir);

	return SkipGameCode;
}

#ifndef _AI_ZONE_CHECK
ASMJIT_PATCH(0x4DFC39, FootClass_FindBioReactor_CheckValid, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	return pThis->IsInAir() || pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x6;
}

ASMJIT_PATCH(0x4DFED2, FootClass_FindGarrisonStructure_CheckValid, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInAir() || pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x6;
}

ASMJIT_PATCH(0x4E0024, FootClass_FindTankBunker_CheckValid, 0x8)
{
	GET(FootClass*, pThis, EDI);
	GET(BuildingClass*, pBuilding, ESI);

	return pThis->IsInAir() ||pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}

ASMJIT_PATCH(0x4DFD92, FootClass_FindBattleBunker_CheckValid, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInAir() || pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}

ASMJIT_PATCH(0x4DFB28, FootClass_FindGrinder_CheckValid, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInAir() || pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}
#endif

ASMJIT_PATCH(0x4C7643, EventClass_RespondToEvent_StopTemporal, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	auto const pTemporal = pTechno->TemporalImUsing;

	if (pTemporal && pTemporal->Target)
		pTemporal->LetGo();

	return 0;
}

// I don't know how can WW miscalculated
// In fact, there should be three different degrees of tilt angles
// - EBX -> atan((2*104)/(256√2)) should only be used on the steepest slopes (13-16)
// - EBP -> atan(104/256) should be used on the most common slopes (1-4)
// - A smaller radian atan(104/(256√2)) should be use to other slopes (5-12)
// But this position is too far ahead, I can't find a good way to solve it perfectly
// Using hooks and filling in floating-point numbers will cause the register to reset to zero
// So I have to do it this way for now, make changes based on the existing data
// Thanks to NetsuNegi for providing a simpler patch method to replace the hook method
DEFINE_PATCH(0x75546D, 0x55) // push ebp
DEFINE_PATCH(0x755484, 0x55) // push ebp
DEFINE_PATCH(0x7554A1, 0x55) // push ebp
DEFINE_PATCH(0x7554BE, 0x55) // push ebp
DEFINE_PATCH(0x755656, 0x55) // push ebp
DEFINE_PATCH(0x755677, 0x55) // push ebp
DEFINE_PATCH(0x755698, 0x55) // push ebp
DEFINE_PATCH(0x7556B9, 0x55) // push ebp
// Although it is not the perfectest
// It can still solve the most common situations on slopes - CrimRecya
ASMJIT_PATCH(0x73C43F, UnitClass_DrawAsVXL_Shadow_IsLocomotorFix, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, EAX);
	R->AL(pType->BalloonHover || pThis->IsAttackedByLocomotor);
	return 0x73C445;
}

// Skip incorrect copy, why do copy like this?
DEFINE_JUMP(LJMP, 0x715326, 0x715333); // TechnoTypeClass::LoadFromINI
// Then EDI is BarrelAnimData now, not incorrect TurretAnimData


ASMJIT_PATCH(0x481778, CellClass_ScatterContent_Scatter, 0x6)
{
	enum { NextTechno = 0x4817D9 };

	GET(TechnoClass*, pTechno, ESI);

	if (!pTechno)
		return NextTechno;

	REF_STACK(const CoordStruct, coords, STACK_OFFSET(0x2C, 0x4));
	GET_STACK(const bool, ignoreMission, STACK_OFFSET(0x2C, 0x8));
	GET_STACK(const bool, ignoreDestination, STACK_OFFSET(0x2C, 0xC));

	if (ignoreDestination || pTechno->HasAbility(AbilityType::Scatter)
		|| (pTechno->Owner->IsControlledByHuman() ? RulesClass::Instance->PlayerScatter : pTechno->Owner->IQLevel2 >= RulesClass::Instance->Scatter))
		pTechno->Scatter(coords, ignoreMission, ignoreDestination);

	return NextTechno;
}

#include <Ext/WarheadType/Body.h>

// make a minimally permissible attack judgment.
const bool CanElectricAssault(FootClass* pThis, BuildingClass* pBuilding)
{
	const auto pWarhead = pThis->GetWeapon(1)->WeaponType->Warhead;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	return pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(pThis , pWarhead)).Verses != 0.0;
}

ASMJIT_PATCH(0x4D7005, FootClass_ElectricAssultFix, 0x5)			// Mission_AreaGuard
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);
	enum { SkipGuard = 0x4D5225, SkipAreaGuard = 0x4D7025 };

	return !CanElectricAssault(pThis, pBuilding) ?
		R->Origin() == 0x4D51B2 ? SkipGuard : SkipAreaGuard
		: 0;
}ASMJIT_PATCH_AGAIN(0x4D51B2, FootClass_ElectricAssultFix, 0x5)	// Mission_Guard

ASMJIT_PATCH(0x7077FD, TechnoClass_PointerExpired_SpawnOwnerFix, 0x6) {
	GET_STACK(bool, removed, STACK_OFFSET(0x20, 0x8));
	// Skip the reset for SpawnOwner if !removed.
	return removed ? 0 : 0x707803;
}

ASMJIT_PATCH(0x44E910, BuildingClass_PointerExpired_C4ExpFix, 0x6)
{
	GET_STACK(bool, removed, STACK_OFFSET(0xC, 0x8));
	// Skip the reset for C4AppliedBy if !removed.
	return removed ? 0 : 0x44E916;
}

// I think no one wants to see wild pointers caused by WW's negligence
ASMJIT_PATCH(0x4D9A1B, FootClass_PointerExpired_RemoveDestination, 0x6)
{

	GET_STACK(bool, removed, STACK_OFFSET(0x1C, 0x8));

	if (removed)
		return 0x4D9ABD;

	R->BL(true);
	return 0x4D9A25;
}


//namespace RemoveSpawneeHelper
//{
//	bool removed = false;
//}
//
//ASMJIT_PATCH(0x707B23, TechnoClass_PointerExpired_RemoveSpawnee, 0x6)
//{
//	GET(SpawnManagerClass*, pSpawnManager, ECX);
//	GET(AbstractClass*, pRemove, EBP);
//	GET_STACK(bool, removed, STACK_OFFSET(0x20, 0x8));
//
//	RemoveSpawneeHelper::removed = removed;
//	pSpawnManager->UnlinkPointer(pRemove);
//	RemoveSpawneeHelper::removed = false;
//
//	return 0x707B29;
//}
//
//ASMJIT_PATCH(0x6B7CE4, SpawnManagerClass_UnlinkPointer_RemoveSpawnee, 0x6)
//{
//	return RemoveSpawneeHelper::removed ? 0x6B7CF4 : 0;
//}

#ifndef _BalloonHoverFix
ASMJIT_PATCH(0x64D592, Game_PreProcessMegaMissionList_CheckForTargetCrdRecal1, 0x6)
{
	enum { SkipTargetCrdRecal = 0x64D598 };
	GET(TechnoClass*, pTechno, EBP);
	return GET_TECHNOTYPE(pTechno)->BalloonHover ? SkipTargetCrdRecal : 0;
}

ASMJIT_PATCH(0x64D575, Game_PreProcessMegaMissionList_CheckForTargetCrdRecal2, 0x6)
{
	enum { SkipTargetCrdRecal = 0x64D598 };
	GET(TechnoClass*, pTechno, EBP);
	return GET_TECHNOTYPE(pTechno)->BalloonHover ? SkipTargetCrdRecal : 0;
}

ASMJIT_PATCH(0x64D5C5, Game_PreProcessMegaMissionList_CheckForTargetCrdRecal3, 0x6)
{
	enum { SkipTargetCrdRecal = 0x64D659 };
	GET(TechnoClass*, pTechno, EBP);
	return GET_TECHNOTYPE(pTechno)->BalloonHover ? SkipTargetCrdRecal : 0;
}

ASMJIT_PATCH(0x51BFA2, InfantryClass_IsCellOccupied_Start, 0x6)
{
	enum { MoveOK = 0x51C02D };
	GET(InfantryClass*, pThis, EBP);
	return pThis->Type->BalloonHover && pThis->IsInAir() ? MoveOK : 0;
}

ASMJIT_PATCH(0x73F0A7, UnitClass_IsCellOccupied_Start, 0x9)
{
	enum { MoveOK = 0x73F23F };
	GET(UnitClass*, pThis, ECX);
	return pThis->Type->BalloonHover && pThis->IsInAir() ? MoveOK : 0;
}
#endif

#ifndef PassengerRelatedFix

#include <Locomotor/LocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>
#include <Ext/Infantry/Body.h>

DEFINE_FUNCTION_JUMP(CALL6 ,0x51A657 , FakeInfantryClass::_DummyScatter);

ASMJIT_PATCH(0x737945, UnitClass_ReceiveCommand_MoveTransporter, 0x7)
{
	enum { SkipGameCode = 0x737952 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	// Move to the vicinity of the passenger
	CellStruct cell = CellStruct::Empty;
	pThis->NearbyLocation(&cell, pPassenger);
	pThis->SetDestination((cell != CellStruct::Empty ? static_cast<AbstractClass*>(MapClass::Instance->GetCellAt(cell)) : pPassenger), true);

	return SkipGameCode;
}

ASMJIT_PATCH(0x710352, FootClass_ImbueLocomotor_ResetStatusses , 0x7)
{
	GET(FootClass*, pTarget, ESI);

	pTarget->OnBridge = false;
	if (const auto pUnit = cast_to<UnitClass* , false>(pTarget))
		pUnit->Unloading = false;

	pTarget->Mark(MarkType::Up);
	pTarget->OnBridge = false;
	return 0;
}

#endif

#ifdef DamageAreaItemsFix
ASMJIT_PATCH(0x489BDB, DamageArea_RockerItemsFix1, 0x6)
{
	enum { SkipGameCode = 0x489C29 };
	// Get cell coordinates
	GET(const short, cellX, ESI);
	GET(const short, cellY, EBX);
	// Record the current cell for linked list getting
	const auto pCell = MapClass::Instance->GetCellAt(CellStruct { cellX, cellY });
	DamageAreaTemp::CheckingCell = pCell;
	// First, check the FirstObject linked list
	auto pObject = pCell->FirstObject;
	// Check if there are objects in the linked list
	if (pObject)
	{
		// When it exists, start the vanilla processing
		R->EAX(pObject);
		return SkipGameCode;
	}
	// When it does not exist, check AltObject linked list
	pObject = pCell->AltObject;
	// If there is an object, record the flag
	if (pObject)
		DamageAreaTemp::CheckingCellAlt = true;
	// Return the original check
	R->EAX(pObject);
	return SkipGameCode;
}

ASMJIT_PATCH(0x489E47, DamageArea_RockerItemsFix2, 0x6)
{
	// Prior to this, there was already pObject = pCell->FirstObject;
	GET(const ObjectClass*, pObject, EDI);
	// As vanilla, first look at the next object in the linked list
	if (pObject)
		return 0;
	// When it does not exist, check which linked list it is currently in
	if (DamageAreaTemp::CheckingCellAlt)
	{
		// If it is already in the AltObject linked list, reset the flag and return the original check
		DamageAreaTemp::CheckingCellAlt = false;
		return 0;
	}
	// If it is still in the FirstObject linked list, take the first object in the  linked list and continue checking
	pObject = DamageAreaTemp::CheckingCell->AltObject;
	// If there is an object, record the flag
	if (pObject)
		DamageAreaTemp::CheckingCellAlt = true;
	// Return the original check
	R->EDI(pObject);
	return 0;
}

#endif

ASMJIT_PATCH(0x51A298, InfantryClass_UpdatePosition_EnterBuilding_CheckSize, 0x6)
{
	enum { CannotEnter = 0x51A4BF };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pDestination, EDI);
	// Compared to `Vehicle entering building` / `Infantry entering vehicle` / `Vehicle entering vehicle`,
	// `Infantry entering building` lacks the judgment of this
	return (pThis->SendCommand(RadioCommand::QueryCanEnter, pDestination) == RadioCommand::AnswerPositive) ? 0 : CannotEnter;
}

DEFINE_JUMP(LJMP, 0x4C752A, 0x4C757D); // Skip cell under bridge check

// Fix a potential edge case where aircraft gets stuck in 'sleep' (reload/repair) on dock if it gets assigned target from team mission etc.
ASMJIT_PATCH(0x41915D, AircraftClass_ReceiveCommand_QueryPreparedness, 0x8)
{
	enum { CheckAmmo = 0x419169 };

	GET(AircraftClass*, pThis, ESI);

	if (pThis->Team && pThis->Team->ArchiveTarget == pThis->Target && pThis->CurrentMission == Mission::Sleep)
		return CheckAmmo;

	return 0;
}

ASMJIT_PATCH(0x6F9222, TechnoClass_SelectAutoTarget_HealingTargetAir, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return pThis->CombatDamage(-1) < 0 ? 0x6F922E : 0;
}

ASMJIT_PATCH(0x418CF3, AircraftClass_Mission_Attack_PlanningFix, 0x5)
{

	GET(AircraftClass* const, pThis, ESI);

	const auto pSpawnOwner = pThis->SpawnOwner;

	if (!pSpawnOwner || pThis->Ammo <= 0 || !pThis->TryNextPlanningTokenNode())
		return 0;

	if(pSpawnOwner) {
		pThis->SetDestination(pSpawnOwner, true);
		pThis->QueueMission(Mission::Move, false);
	}

	return 0x418D00;
}

#include <WWMouseClass.h>

DWORD WINAPI Mouse_Thread(MouseThreadParameter* lpThreadParameter)
{
    lpThreadParameter->SomeState18 = 1;  // ← Fixed: was dword14
    lpThreadParameter->SkipSleep = 0;

    if (lpThreadParameter->SkipProcessing) {
        lpThreadParameter->SkipSleep = 1;
    } else {
        do
        {
            if (Imports::WaitForSingleObject.invoke()(MouseThreadParameter::Mutex(), 10000u) == 258) {
                Debug::Log("Warning: Probable deadlock occurred on MouseMutex.");
            }
            if (WWMouseClass::Thread_Instance()) {
                WWMouseClass::Thread_Instance->Process();
            }
            Imports::ReleaseMutex.invoke()(MouseThreadParameter::Mutex());
            Imports::Sleep.invoke()((long long)lpThreadParameter->SleepTime);
            ++lpThreadParameter->RefCount;
        }
        while (!lpThreadParameter->SkipProcessing);
        lpThreadParameter->SkipSleep = 1;
    }
    return 0;
}

void __fastcall StartMouseThread() {
    HANDLE MutexA = MouseThreadParameter::Mutex();
    char Buffer[1024];

    if (!MouseThreadParameter::Mutex()) {
        MutexA = Imports::CreateMutexA.invoke()(0, 0, 0);
        MouseThreadParameter::Mutex = MutexA;
    }

    if (!MouseThreadParameter::ThreadNotActive())
    {
        if (MutexA)
        {

            MouseThreadParameter::Thread().SkipProcessing = 0;
            MouseThreadParameter::Thread().SkipSleep = 0;
            MouseThreadParameter::Thread().SomeState18 = 0;
            MouseThreadParameter::Thread().SleepTime = 16;
            MouseThreadParameter::Thread().dword14 = 0;
            MouseThreadParameter::Thread().RefCount = 0;

            HANDLE Thread = Imports::CreateThread.invoke()(
                0, 0x1000u,
                (LPTHREAD_START_ROUTINE)Mouse_Thread,
                &MouseThreadParameter::Thread(),  // Pass address of struct
                0,
                &MouseThreadParameter::StaticThreadID()  // ← Use separate variable!
            );

            MouseThreadParameter::Thread_Handle() = Thread;  // ← Store in separate variable!

            if (Thread)
            {
                MouseThreadParameter::ThreadNotActive() = 1;
                if (!Imports::SetThreadPriority.invoke()(Thread, 15))
                {
                    DWORD LastError = GetLastError();
                    Imports::FormatMessageA.invoke()(0x1000u, 0, LastError, 0, Buffer, 0x400u, 0);
                    Debug::Log("Unable to change the priority of the mouse thread - %s\n", Buffer);

                    while (!MouseThreadParameter::Thread().SkipSleep)
                    {
                        Imports::Sleep.invoke()(0);
                    }
                    Imports::WaitForSingleObject.invoke()(MouseThreadParameter::Thread_Handle(), 5000u);
                    Imports::CloseHandle.invoke()(MouseThreadParameter::Thread_Handle());
                    MouseThreadParameter::ThreadNotActive() = 0;
                }
            }
        }
    }
}

//massive FPS losses
//DEFINE_FUNCTION_JUMP(CALL , 0x6BD849 , StartMouseThread)

ASMJIT_PATCH(0x70E126, TechnoClass_GetDeployWeapon_InfantryDeployFireWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pInfantry = cast_to<InfantryClass*, false>(pThis)) {
		const int deployFireWeapon = pInfantry->Type->DeployFireWeapon;

		R->EAX(deployFireWeapon == -1 ? pInfantry->SelectWeapon(pInfantry->Target) : deployFireWeapon);
	} else {
		R->EAX(pThis->IsNotSprayAttack());
	}

	return 0x70E12C;
}

DEFINE_JUMP(LJMP, 0x6FBC0B, 0x6FBC38) // TechnoClass::UpdateCloak

//ASMJIT_PATCH(0x457DEB, BuildingClass_ClearOccupants_Redraw, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//	pThis->Mark(MarkType::Change);
//	return 0;
//}

#pragma region AStarBuffer

// Buffer doubled
DEFINE_PATCH(0x42A752, 0x08);
DEFINE_PATCH(0x42A765, 0x02);
DEFINE_PATCH(0x42A7E3, 0x20);
DEFINE_PATCH(0x42A7FA, 0x02);

#pragma endregion


// AStarClass::FindHierarchicalPath
// Replace sign-extend to zero-extend

// 42C34A: 0F BF 1C 70
// To avoid incorrect negative int index
DEFINE_PATCH(0x42C34B, 0xB7);
// movsx ebx, word ptr [eax+esi*2] -> movzx ebx, word ptr [eax+esi*2]

// 42C36A: 0F BF 04 70
// To avoid incorrect negative int index
DEFINE_PATCH(0x42C36B, 0xB7);
// movsx eax, word ptr [eax+esi*2] -> movzx eax, word ptr [eax+esi*2]

// Fix Jumpjets can not spawn missiles in air.
ASMJIT_PATCH(0x6B72FE, SpawnerManagerClass_AI_MissileCheck, 0x9)
{
	enum { SpawnMissile = 0x6B735C, NoSpawn = 0x6B795A };

	GET(SpawnManagerClass*, pThis, ESI);

	auto& pLoco = ((FootClass*)pThis->Owner)->Locomotor; // Ares has already handled the building case.
	auto pLocoInterface = pLoco.GetInterfacePtr();

	return (pLocoInterface->Is_Moving_Now()
		|| (!locomotion_cast<JumpjetLocomotionClass*>(pLoco) && pLocoInterface->Is_Moving())) // Jumpjet should only check Is_Moving_Now.
		? NoSpawn : SpawnMissile;
}

DEFINE_PATCH(0x6656B3, 0x89, 0x4E);

#pragma region FixPlanningNodeConnect

#include <PlanningTokenClass.h>

// Restore the original three pop to prevent stack imbalance
void NAKED _PlanningNodeClass_UpdateHoverNode_FixCheckValidity_RET()
{
	POP_REG(EDI);
	POP_REG(EBP);
	POP_REG(EBX);
	JMP(0x638F2A);
}

ASMJIT_PATCH(0x638F1E, PlanningNodeClass_UpdateHoverNode_FixCheckValidity, 0x5)
{
	// Newly added checks to prevent not in-time updates
	return PlanningNodeClass::PlanningModeActive ? (int)_PlanningNodeClass_UpdateHoverNode_FixCheckValidity_RET : 0;
}

ASMJIT_PATCH(0x638F70, PlanningNodeClass_UpdateHoverNode_SkipDuplicateLog, 0x8)
{
	enum { SkipLogString = 0x638F81 };

	GET(const PlanningNodeClass* const, pCurrentNode, ESI);

	const auto& pHoveringNode = Make_Global<const PlanningNodeClass* const>(0xAC4CCC);

	// Only output logs when they are not the same, to avoid outputting every frame
	return (pCurrentNode != pHoveringNode) ? 0 : SkipLogString;
}

#pragma endregion

#pragma region JumpjetSetDestFix

// Fix JJ infantries stop incorrectly when assigned a target out of range.
ASMJIT_PATCH(0x51AB5C, InfantryClass_SetDestination_JJInfFix, 0x6)
{
	enum { FuncRet = 0x51B1D7 };

	GET(InfantryClass* const, pThis, EBP);
	GET(AbstractClass* const, pDest, EBX);

	auto pJumpjetLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor);

	if (pThis->Type->BalloonHover && !pDest && pThis->Destination && pJumpjetLoco && pThis->Target)
	{
		if (pThis->IsCloseEnoughToAttack(pThis->Target))
		{
			pThis->StopMoving();
		}

		pThis->ForceMission(Mission::Attack);
		return FuncRet;
	}

	return 0;
}

// Fix JJ vehicles can not stop correctly when assigned a target in range.
ASMJIT_PATCH(0x741A66, UnitClass_SetDestination_JJVehFix, 0x5)
{
	GET(UnitClass* const, pThis, EBP);

	auto pJumpjetLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor);

	if (pJumpjetLoco && pThis->IsCloseEnoughToAttack(pThis->Target))
	{
		pThis->StopMoving();
	}

	return 0;
}

#pragma endregion

// 42A80A: 89 98 00 00 10 00
// Set new Count offset
DEFINE_PATCH(0x42A80E, 0x20);
// mov [eax+100000h], ebx -> mov [eax+200000h], ebx
// 42A840: 89 98 00 00 10 00

// Set new Count offset
DEFINE_PATCH(0x42A844, 0x20);
// mov [eax+100000h], ebx -> mov [eax+200000h], ebx

// AStarClass::CleanUp

// 42A5C3: 89 B2 00 00 10 00
// Set new Count offset
DEFINE_PATCH(0x42A5C7, 0x20);
// mov [edx+100000h], esi -> mov [edx+200000h], esi

// AStarClass::CreatePathNode

// 42A466: 8B 90 00 00 10 00
// Set new Count offset
DEFINE_PATCH(0x42A46A, 0x20);
// mov edx, [eax+100000h] -> mov edx, [eax+200000h]

// 42A479: 89 90 00 00 10 00
// Set new Count offset
DEFINE_PATCH(0x42A47D, 0x20);
// mov [eax+100000h], edx -> mov [eax+200000h], edx

// Fix the issue that the jumpjet vehicles cannot stop correctly after going berserk
ASMJIT_PATCH(0x74431F, UnitClass_ReadyToNextMission_HuntCheck, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->GetCurrentMission() != Mission::Hunt ? 0 : 0x744329;
}

ASMJIT_PATCH(0x54CC9C, JumpjetLocomotionClass_ProcessCrashing_DropFix, 0x5)
{
	enum { SkipGameCode = 0x54CDC3, SkipGameCode2 = 0x54CFB7 };

	GET(ObjectClass* const, pObject, ESI);
	GET(JumpjetLocomotionClass*, pLoco, EDI);
	const auto pLinkedTo = pLoco->LinkedTo;
	bool fallOnSomething = false;

	for (NextObject object(pObject); object; ++object)
	{
		if (*object == pLinkedTo || !(*object)->IsAlive)
			continue;

		const auto whatAmObject = object->WhatAmI();

		if (whatAmObject == UnitClass::AbsID || whatAmObject == BuildingClass::AbsID || whatAmObject == AircraftClass::AbsID)
		{
			fallOnSomething = true;
			continue;
		}

		if (whatAmObject == InfantryClass::AbsID)
		{
			const auto pInfantry = static_cast<InfantryClass*>(*object);

			VocClass::SafeImmedietelyPlayAt(object->GetType()->CrushSound, object->Location);

			if (const auto pManipulater = pLinkedTo->BeingManipulatedBy)
				pInfantry->RegisterDestruction(pManipulater);
			else if (const auto pSourceHouse = pLinkedTo->ChronoWarpedByHouse)
				pInfantry->RegisterKill(pSourceHouse);
			else
				pInfantry->RegisterDestruction(pLinkedTo);

			pInfantry->Mark(MarkType::Up);
			pInfantry->Limbo();
			pInfantry->UnInit();
			continue;
		}

		if (whatAmObject == TerrainClass::AbsID)
		{
			const auto pTerrain = static_cast<TerrainClass*>(*object);

			if (pTerrain->Type->SpawnsTiberium || pTerrain->Type->Immune)
				continue;
		}

		if (const auto pManipulater = pLinkedTo->BeingManipulatedBy)
			object->ReceiveDamage(&object->Health, 0, RulesClass::Instance->CrushWarhead, pManipulater, true, false, pManipulater->Owner);
		else if (const auto pSourceHouse = pLinkedTo->ChronoWarpedByHouse)
			object->ReceiveDamage(&object->Health, 0, RulesClass::Instance->CrushWarhead, pLinkedTo, true, false, pSourceHouse);
		else
			object->ReceiveDamage(&object->Health, 0, RulesClass::Instance->CrushWarhead, pLinkedTo, true, false, pLinkedTo->Owner);
	}

	return fallOnSomething ? SkipGameCode2 : SkipGameCode;
}

#pragma region OwnerChangeBuildupFix

void __fastcall BuildingClass_Place_Wrapper(BuildingClass* pThis, void*, bool captured)
{
	// Skip calling Place() here if we're in middle of buildup.
	if (pThis->CurrentMission != Mission::Construction || pThis->BState != BStateType::Construction)
		pThis->Place(captured);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x448CEF, BuildingClass_Place_Wrapper);

ASMJIT_PATCH(0x44939F, BuildingClass_Captured_BuildupFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	// If we're supposed to be playing buildup during/after owner change reset any changes to mission or BState made during owner change.
	if (pThis->CurrentMission == Mission::Construction && pThis->BState == BStateType::Construction)
	{
		pThis->IsReadyToCommence = false;
		pThis->QueueBState = BStateType::None;
		pThis->QueuedMission = Mission::None;
	}

	return 0;
}

#pragma endregion

#pragma region ClearTargetOnOwnerChanged
#include <AirstrikeClass.h>

ASMJIT_PATCH(0x70D4A0, AbstractClass_ClearTargetToMe_ClearManagerTarget, 0x5)
{
	GET(AbstractClass*, pThis, ECX);

	for (const auto pTemporal : *TemporalClass::Array) {
		if (pTemporal->Target == pThis)
			pTemporal->LetGo();
	}

	// WW don't clear target if the techno has airstrike manager.
	// No idea why, but for now we respect it and don't handle the airstrike target.
	for (const auto pAirstrike : *AirstrikeClass::Array) {
		if (pAirstrike->Target == pThis)
			pAirstrike->ResetTarget();
	}

	for (const auto pSpawn : *SpawnManagerClass::Array) {
		if (pSpawn->Target == pThis)
			pSpawn->ResetTarget();
	}

	if (const auto pTechno = flag_cast_to<TechnoClass*, false>(pThis))
		pTechno->LastTarget = nullptr;

	if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
		pFoot->LastDestination = nullptr;

	return 0;
}

ASMJIT_PATCH(0x70D4FD, AbstractClass_ClearTargetToMe_ClearLastTarget, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(const bool, shouldClear, ECX);
	GET(AbstractClass*, pThis, EBP);

	if (pTechno->LastTarget == pThis && shouldClear)
		pTechno->LastTarget = nullptr;

	return 0;
}

#pragma endregion

// Nov 22, 2025 - Starkku: Fixes an issue that causes preplaced aircraft placed outside visible map to be flagged as crashing even if
// there are preplaced docks, due to parsing order (preplaced aircraft go before buildings) as well as outside visible map aircraft
// being instantly elevated to FlightLevel on Unlimbo which makes it bypass a height check in FootClass::Crash().
// This fix makes preplaced aircraft immediately return from FootClass::Crash() during unlimbo process.
// Related GitHub issue: https://github.com/Phobos-developers/Phobos/issues/1958

#pragma region PrePlacedAircraftFix

namespace PrePlacedAircraftFixTemp
{
	bool SkipCrashing;
}

static bool __fastcall AircraftClass_Unlimbo_Wrapper(AircraftClass* pThis, discard_t, const CoordStruct& coords, DirType facing)
{
	PrePlacedAircraftFixTemp::SkipCrashing = true;
	bool retVal = pThis->Unlimbo(coords, facing);
	PrePlacedAircraftFixTemp::SkipCrashing = false;

	return retVal;

}
DEFINE_FUNCTION_JUMP(CALL6, 0x41B39B, AircraftClass_Unlimbo_Wrapper);

ASMJIT_PATCH(0x4DEBC4, FootClass_Crash_PreplacedAircraft, 0x7)
{
	enum { ReturnFromFunction = 0x4DED5B };

	if (PrePlacedAircraftFixTemp::SkipCrashing)
		return ReturnFromFunction;

	return 0;

}

#pragma endregion

ASMJIT_PATCH(0x6FBFA3, TechnoClass_Select_SkipLimboDelivery, 0x6)
{
	enum { SkipSelect = 0x6FC029 };

	GET(TechnoClass* const, pThis, ESI);

	if (auto const pBuilding = cast_to<BuildingClass*, false>(pThis)){
		if(BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1)
		   return SkipSelect;
	}

	return 0;
}
