#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>

#include <Base/Always.h>

#include <SlaveManagerClass.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Helpers.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include "Header.h"

#include <Conversions.h>
#include <GameOptionsClass.h>
#include <TacticalClass.h>
#include <RadarEventClass.h>
#include <SpawnManagerClass.h>
#include <AirstrikeClass.h>


ASMJIT_PATCH(0x702DD6, TechnoClass_RegisterDestruction_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			// 85
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0;
}

ASMJIT_PATCH(0x7032B0, TechnoClass_RegisterLoss_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(HouseClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x703A79, TechnoClass_VisualCharacter_CloakingStages, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	int stages = GET_TECHNOTYPEEXT(pThis)->CloakStages.Get(RulesClass::Instance->CloakingStages);
	R->EAX(int(pThis->CloakProgress.Stage * 256.0 / stages));
	return 0x703A94;
}

#include <ExtraHeaders/StackVector.h>

// Support per unit modification of Iron Curtain effect duration
ASMJIT_PATCH(0x70E2B0, TechnoClass_IronCurtain, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, duration, STACK_OFFS(0x0, -0x4));
	//GET_STACK(HouseClass*, source, STACK_OFFS(0x0, -0x8));
	GET_STACK(bool, force, STACK_OFFS(0x0, -0xC));

	// if it's no force shield then it's the iron curtain.
	const auto pData =  GET_TECHNOTYPEEXT(pThis);
	const auto modifier = (force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier).Get();

	pThis->IronCurtainTimer.Start(int(duration * modifier));
	pThis->IronTintStage = 0;
	pThis->ProtectType = force ? ProtectTypes::ForceShield : ProtectTypes::IronCurtain;

	R->EAX(DamageState::Unaffected);
	return 0x70E2FD;
}

ASMJIT_PATCH(0x7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	R->EAX<int>(TechnoTypeExtData::HasSelectionGroupID(GET_TECHNOTYPE(pThis), pID));
	return 0x7327B2;
}

#include <CaptureManagerClass.h>

ASMJIT_PATCH(0x707B09, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EBP);
	GET_STACK(bool, remove, 0x28);

	// issues 1002020, 896263, 895954: clear stale mind control pointer to prevent
	// crashes when accessing properties of the destroyed controllers.
	if (pThis->MindControlledBy == ptr) {
		pThis->MindControlledBy = nullptr;
	}

	if(pThis->CaptureManager) {
		pThis->CaptureManager->DetachTarget(ptr);
	}

	// #912875: respect the remove flag for invalidating SpawnManager owners
	if(pThis->SpawnManager && (pThis->Owner != ptr || !(!remove && pThis->Owner == ptr))){
		pThis->SpawnManager->UnlinkPointer(ptr);
	}

	return 0x707B29;
}

static void PlayEva(const char* pEva, CDTimerClass& nTimer, double nRate) {
	if (!nTimer.GetTimeLeft()) {
		nTimer.Start(GameOptionsClass::Instance->GetAnimSpeed(static_cast<int>(nRate * 900.0)));
		VoxClass::Play(pEva);
	}
}

ASMJIT_PATCH(0x70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (detect && pTypeExt->SensorArray_Warn)
	{
		switch (detect)
		{
		case 1:
			PlayEva("EVA_CloakedUnitDetected", HouseExtContainer::Instance.CloakEVASpeak, RulesExtData::Instance()->StealthSpeakDelay);
			break;
		case 2:
			PlayEva("EVA_SubterraneanUnitDetected", HouseExtContainer::Instance.SubTerraneanEVASpeak, RulesExtData::Instance()->SubterraneanSpeakDelay);
			break;
		}

		CellStruct cell = CellClass::Coord2Cell(pThis->Location);
		RadarEventClass::Create(RadarEventType::EnemySensed, cell);
	}

	return 0x70DADC;
}

ASMJIT_PATCH(0x70CBB0, TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (!pWeapon->AmbientDamage)
		return 0x70CC3E;

	R->EDI(pWeapon);
	R->ESI(0);
	return (!(R->EAX<int>() <= 0)) ? 0x70CBB9 : 0x70CBF7;
}

// the fuck , game calling `MapClass[]` multiple times , fixed it
ASMJIT_PATCH(0x6FB5F0, TechnoClass_DeleteGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	const int nDecrease = pCell->GapsCoveringThisCell - 1;
	pCell->GapsCoveringThisCell = nDecrease;

	if (!HouseClass::CurrentPlayer->SpySatActive || nDecrease > 0)
		return 0x6FB69E;

	--pCell->ShroudCounter;

	if (pCell->ShroudCounter <= 0)
		pCell->AltFlags |= (AltCellFlags::NoFog | AltCellFlags::Mapped);

	return 0x6FB69E;
}

ASMJIT_PATCH(0x6FB306, TechnoClass_CreateGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	int nCounter = pCell->ShroudCounter;
	int nCounter_b = nCounter;
	if (nCounter >= 0 && nCounter != 1)
	{
		nCounter_b = nCounter + 1;
		pCell->ShroudCounter = nCounter + 1;
	}
	++pCell->GapsCoveringThisCell;
	if (nCounter_b >= 1)
		pCell->UINTAltFlags &= 0xFFFFFFE7;

	return 0x6FB3BD;
}

//ASMJIT_PATCH(0x6FB757, TechnoClass_UpdateCloak, 8)
//{
//	GET(TechnoClass*, pThis, ESI);
//	return !TechnoExt_ExtData::CloakDisallowed(pThis, false) ? 0x6FB7FD : 0x6FB75F;
//}
//
//ASMJIT_PATCH(0x6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
//{
//	GET(TechnoClass*, pThis, ECX);
//
//	R->EAX(TechnoExt_ExtData::CloakDisallowed(pThis, true));
//	return 0x6FBDBC;
//}
//
//ASMJIT_PATCH(0x6FBDC0, TechnoClass_ShouldBeCloaked, 5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	R->EAX(TechnoExt_ExtData::CloakAllowed(pThis));
//	return 0x6FBF93;
//}

ASMJIT_PATCH(0x6F6AC9, TechnoClass_Limbo_Early, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// if the removed object is a radar jammer, unjam all jammed radars
	TechnoExtContainer::Instance.Find(pThis)->RadarJammer.reset();
	// #617 powered units
	TechnoExtContainer::Instance.Find(pThis)->PoweredUnit.reset();


	//#1573, #1623, #255 attached effects
	AresAE::Remove(&TechnoExtContainer::Instance.Find(pThis)->AeData , pThis);

	if (TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount != 0) {
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, true);
	}

	return pThis->InLimbo ? 0x6F6C93u : 0x6F6AD5u;
}

ASMJIT_PATCH(0x6F6F20, TechnoClass_Unlimbo_BuildingLight, 6)
{
	GET(TechnoClass*, pThis, ESI);

	//const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if(R->Origin() == 0x6F6F20){
		HugeBar::InitializeHugeBar(pThis);
		//TechnoExtData::UnlimboAttachments(pThis);
	}

	//only update the SW if really needed it
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsUnavailable();

	if (pTypeExt->HasSpotlight)
	{
		TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6F6D0E, TechnoClass_Unlimbo_BuildingLight, 7)

ASMJIT_PATCH(0x6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(LaserDrawClass*, pBeam, EAX);

	auto const pData = WeaponTypeExtContainer::Instance.Find(pWeapon);
	if (!pBeam->IsHouseColor && WeaponTypeExtContainer::Instance.Find(pWeapon)->Laser_IsSingleColor)
		pBeam->IsHouseColor = true;

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	if (pData->Laser_Thickness > 1) {
		pBeam->Thickness = pData->Laser_Thickness;
	}

	pBeam->IsSupported = pBeam->Thickness > 3;

	return 0;
}

ASMJIT_PATCH(0x6f526c, TechnoClass_DrawExtras_PowerOff, 5)
{
	GET(TechnoClass*, pTechno, EBP);

	if(!pTechno->IsAlive)
		return 0x6F5347;

	GET_STACK(RectangleStruct*, pRect, 0xA0);

	if (auto pBld = cast_to<BuildingClass*, false>(pTechno))
	{
		const auto pBldExt = BuildingExtContainer::Instance.Find(pBld);
		const auto isObserver = HouseClass::IsCurrentPlayerObserver();

		// allies and observers can always see by default
		const bool canSeeRepair = HouseClass::CurrentPlayer->IsAlliedWith(pBld->Owner)
			|| isObserver;

		const bool showRepair = FileSystem::WRENCH_SHP
			&& pBld->IsBeingRepaired
			// fixes the wrench playing over a temporally challenged building
			&& !pBld->IsBeingWarpedOut()
			&& !pBld->WarpingOut
			// never show to enemies when cloaked, and only if allowed
			&& (canSeeRepair || (pBld->CloakState == CloakState::Uncloaked
				&& RulesExtData::Instance()->EnemyWrench));

		// display power off marker only for current player's buildings
		const bool showPower = FileSystem::POWEROFF_SHP
			&& (!pBldExt->TogglePower_HasPower)
			// only for owned buildings, but observers got magic eyes
			&& ((pBld->GetCurrentMission() != Mission::Selling) && (pBld->GetCurrentMission() != Mission::Construction))
			&& (pBld->Owner->ControlledByCurrentPlayer() || isObserver);

		// display any?
		if (showPower || showRepair)
		{
			auto cell = pBld->GetMapCoords();

			if (!MapClass::Instance->GetCellAt(cell)->IsShrouded())
			{
				CoordStruct crd = pBld->GetCenterCoords();
				Point2D point = TacticalClass::Instance->CoordsToClient(crd);

				// offset the markers
				Point2D ptRepair = point;
				if (showPower)
				{
					ptRepair.X -= 7;
					ptRepair.Y -= 7;
				}

				Point2D ptPower = point;
				if (showRepair)
				{
					ptPower.X += 18;
					ptPower.Y += 18;
				}

				// animation display speed
				// original frame calculation: ((currentframe%speed)*6)/(speed-1)
				const int speed = MaxImpl(GameOptionsClass::Instance->GetAnimSpeed(14) / 4, 2);

				// draw the markers
				if (showRepair)
				{
					int frame = (FileSystem::WRENCH_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::WRENCH_SHP,
						frame, &ptRepair, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}

				if (showPower)
				{
					int frame = (FileSystem::POWEROFF_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::POWEROFF_SHP,
						frame, &ptPower, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0x6F5347;
}

void __fastcall FakeTechnoClass::__Draw_Stuff_When_Selected(TechnoClass* pThis, discard_t, Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect)
{
}

DEFINE_FUNCTION_JUMP(LJMP, 0x70AA60 , FakeTechnoClass::__Draw_Stuff_When_Selected)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E26FC, FakeTechnoClass::__Draw_Stuff_When_Selected)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E90EC, FakeTechnoClass::__Draw_Stuff_When_Selected)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB4B0, FakeTechnoClass::__Draw_Stuff_When_Selected)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4DB8, FakeTechnoClass::__Draw_Stuff_When_Selected)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F60C8, FakeTechnoClass::__Draw_Stuff_When_Selected)

DEFINE_FUNCTION_JUMP(VTABLE , 0x7E4314 , FakeBuildingClass::_DrawStuffsWhenSelected)

ASMJIT_PATCH(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	pThis->GapRadius = TechnoTypeExtContainer::Instance.Find(pType)->GapRadiusInCells;
	return R->Origin() + 0xD;
}ASMJIT_PATCH_AGAIN(0x6FB4A3, TechnoClass_CreateGap_LargeGap, 7)


// Radar Jammers (#305) unjam all on owner change
ASMJIT_PATCH(0x7014D5, TechnoClass_SetOwningHouse_Additional, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	//Debug::LogInfo("ChangeOwnershipFor [%s]" , pThis->get_ID());

	//TechnoClass::ClearWhoTargetingThis(pThis);

	//for (auto pTemporal : *TemporalClass::Array) {
	//	if (pTemporal->Target == pThis)
	//		pTemporal->LetGo();
	//}

	//for (auto pAirstrike : *AirstrikeClass::Array) {
	//	if (pAirstrike->Target == pThis)
	//		pAirstrike->ResetTarget();
	//}

	//for (auto pSpawn : *SpawnManagerClass::Array) {
	//	if (pSpawn->Target == pThis)
	//		pSpawn->ResetTarget();
	//}

	if (auto& pJammer = TechnoExtContainer::Instance.Find(pThis)->RadarJammer) {
		pJammer->UnjamAll();
	}

	if (auto pBuilding = cast_to<BuildingClass*, false>(pThis)) {

		const auto nTunnelVec = HouseExtData::GetTunnelVector(pBuilding->Type, pThis->Owner);

		if (!nTunnelVec || TunnelFuncs::FindSameTunnel(pBuilding))
			return 0x0;

		for (auto nPos = nTunnelVec->Vector.begin();
			nPos != nTunnelVec->Vector.end(); ++nPos) {
			TunnelFuncs::KillFootClass(*nPos, nullptr);
		}

		nTunnelVec->Vector.clear();
	}

	if (TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount != 0)
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, true);

	return 0;
}

ASMJIT_PATCH(0x702E64, TechnoClass_RegisterDestruction_Bounty, 6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);

	TechnoExt_ExtData::GiveBounty(pVictim, pKiller);

	return 0x0;
}

void TechnoExtData::InitializeRecoilData(TechnoClass* pThis, TechnoTypeClass* pType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pType->TurretRecoil)
		return;

	if (pTypeExt->ExtraTurretCount)
	{
		if (static_cast<int>(pExt->ExtraTurretRecoil.size()) < pTypeExt->ExtraTurretCount)
			pExt->ExtraTurretRecoil.resize(pTypeExt->ExtraTurretCount);

		const auto& refData = pType->TurretAnimData;

		for (auto& data : pExt->ExtraTurretRecoil)
		{
			data.Turret.Travel = refData.Travel;
			data.Turret.CompressFrames = refData.CompressFrames;
			data.Turret.RecoverFrames = refData.RecoverFrames;
			data.Turret.HoldFrames = refData.HoldFrames;
			data.TravelPerFrame = 0.0;
			data.TravelSoFar = 0.0;
			data.State = RecoilData::RecoilState::Inactive;
			data.TravelFramesLeft = 0;
		}
	}

	if (pTypeExt->ExtraTurretCount || pTypeExt->ExtraBarrelCount)
	{
		const auto dataCount = (pTypeExt->ExtraBarrelCount + 1) * (pTypeExt->ExtraTurretCount + 1) - 1;

		if (static_cast<int>(pExt->ExtraBarrelRecoil.size()) < dataCount)
			pExt->ExtraBarrelRecoil.resize(dataCount);

		const auto& refData = pType->BarrelAnimData;

		for (auto& data : pExt->ExtraBarrelRecoil)
		{
			data.Turret.Travel = refData.Travel;
			data.Turret.CompressFrames = refData.CompressFrames;
			data.Turret.RecoverFrames = refData.RecoverFrames;
			data.Turret.HoldFrames = refData.HoldFrames;
			data.TravelPerFrame = 0.0;
			data.TravelSoFar = 0.0;
			data.State = RecoilData::RecoilState::Inactive;
			data.TravelFramesLeft = 0;
		}
	}
}


ASMJIT_PATCH(0x6F3F43, TechnoClass_Init, 6)
{
	GET(TechnoClass* , pThis, ESI);

	auto const pType = GET_TECHNOTYPE(pThis);

	if(pType)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		pExt->TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);
		HouseExtData* pHouseExt = nullptr;

		if (pThis->Owner) {
			pThis->IsOwnedByCurrentPlayer = pThis->Owner == HouseClass::CurrentPlayer();
			pHouseExt = HouseExtContainer::Instance.Find(pThis->Owner);
		}

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);


		//TechnoExtData::InitializeAttachments(pThis);

		CaptureManagerClass* pCapturer = nullptr;
		ParasiteClass* pParasite = nullptr;
		TemporalClass* pTemporal = nullptr;
		SpawnManagerClass* pSpawnManager = nullptr;
		SlaveManagerClass* pSlaveManager = nullptr;
		AirstrikeClass* pAirstrike = nullptr;

		//AircraftDiveFunctional::Init(pExt, pTypeExt);

		if (pHouseExt && pTypeExt->Harvester_Counted)
			pHouseExt->OwnedCountedHarvesters.emplace(pThis);

		if (pType->Spawns) {
			pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pType->Spawns, pType->SpawnsNumber, pType->SpawnRegenRate, pType->SpawnReloadRate);
		}

		if (pType->Enslaves) {
			pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pType->Enslaves, pType->SlavesNumber, pType->SlaveRegenRate, pType->SlaveReloadRate);
		}

		if (pType->AirstrikeTeam > 0 && pType->AirstrikeTeamType) {
			pAirstrike = GameCreate<AirstrikeClass>(pThis);
		}

		const bool IsFoot = pThis->WhatAmI() != BuildingClass::AbsID;
		const int WeaponCount = pType->TurretCount <= 0 ? 2 : pType->WeaponCount;

		for (auto i = 0; i < WeaponCount; ++i) {

			if (auto const pWeapon = pType->GetWeapon(i)->WeaponType) {
				TechnoExt_ExtData::InitWeapon(pThis, pType, pWeapon, i, pCapturer, pParasite, pTemporal, "Weapon", IsFoot);
			}

			if (auto const pWeaponE = pType->GetEliteWeapon(i)->WeaponType) {
				TechnoExt_ExtData::InitWeapon(pThis, pType, pWeaponE, i, pCapturer, pParasite, pTemporal, "EliteWeapon", IsFoot);
			}
		}

		pThis->CaptureManager = pCapturer;
		pThis->TemporalImUsing = pTemporal;
		if (IsFoot) {
			((FootClass*)pThis)->ParasiteImUsing = pParasite;
		}

		pThis->SpawnManager = pSpawnManager;
		pThis->SlaveManager = pSlaveManager;
		pThis->Airstrike = pAirstrike;

		if (auto pOwner = pThis->Owner) {
			const auto pHouseType = pOwner->Type;
			const auto pParentHouseType = pHouseType->FindParentCountry();
			TechnoExtContainer::Instance.Find(pThis)->OriginalHouseType = pParentHouseType ? pParentHouseType : pHouseType;
		} else {
			Debug::LogInfo("Techno[{}] Init Without any ownership!", pType->ID);
		}

		// if override is in effect, do not create initial payload.
		// this object might have been deployed, undeployed, ...
		if (Unsorted::ScenarioInit && Unsorted::CurrentFrame) {
			TechnoExtContainer::Instance.Find(pThis)->PayloadCreated = true;
		}

		TechnoExtData::InitializeItems(pThis, pType);
		TechnoExtData::InitializeAttachEffects(pThis, pType);
		TechnoExtData::InitializeRecoilData(pThis, pType);

		const auto pPrimary = pThis->GetWeapon(0)->WeaponType;

		if (pPrimary && pType->LandTargeting != LandTargetingType::Land_not_okay)
			pThis->RearmTimer.TimeLeft = pPrimary->ROF;
		else if (const auto pSecondary = pThis->GetWeapon(1)->WeaponType)
			pThis->RearmTimer.TimeLeft = pSecondary->ROF;

		pThis->RearmTimer.StartTime = MinImpl(-2, -pThis->RearmTimer.TimeLeft);

		TechnoExtData::InitializeUnitIdleAction(pThis, pType);

		pExt->InitPassiveAcquireMode();
		if (!pExt->AE.flags.HasTint && pExt->CurrentShieldType == ShieldTypeClass::Array[0].get())
			pExt->Tints.Update();

		R->EAX(pType);
		return 0x6F4212;
	}

	return 0x6F42F7;
}

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
// ASMJIT_PATCH(0x6F7561, TechnoClass_Targeting_Arcing_Aircraft, 0x5)
// {
// 	GET(AbstractType, pTarget, EAX);
// 	GET(CoordStruct*, pCoord, ESI);
// 	R->EAX(pCoord->X);
//	return pTarget == AbstractType::Aircraft ? 0x6F75B2 : 0x6F7568;
// }
DEFINE_PATCH_ADDR_OFFSET(byte, 0x6F7561, 0x2 , 0x2);

// No data found on .inj for this
//ASMJIT_PATCH(0x5F7933, TechnoTypeClass_FindFactory_ExcludeDisabled, 0x6)
//{
//	GET(BuildingClass*, pBld, ESI);
//
//	 //add the EMP check to the limbo check
//	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
//		0x5F7A57 : 0x5F7941;
//}

// ASMJIT_PATCH(0x6F90F8, TechnoClass_GreatestThreat_Demacroize, 0x6)
// {
// 	GET(int, nVal1, EDI);
// 	GET(int, nVal2, EAX);
//
// 	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
// 	return 0x6F9116;
// }

ASMJIT_PATCH(0x70133E, TechnoClass_GetWeaponRange_Demacroize, 0x5)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EBX);

	R->EAX(nVal1 >= nVal2 ? nVal2 : nVal1);
	return 0x701388;
}

// ASMJIT_PATCH(0x707EEA, TechnoClass_GetGuardRange_Demacroize, 0x6)
// {
// 	GET(int, nVal1, EBX);
// 	GET(int, nVal2, EAX);

// 	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
// 	return 0x707F08;
// }

// customizable berserk fire rate modification
// ASMJIT_PATCH(0x6FF28F, TechnoClass_Fire_BerserkROFMultiplier, 6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(int, ROF, EAX);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	if (pThis->Berzerk) {
// 		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
// 		const double multiplier = pExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
// 		ROF = static_cast<int>(ROF * multiplier);
// 	}
//
// 	TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);
//
// 	R->EAX(ROF);
// 	return 0x6FF2A4;
// }

ASMJIT_PATCH(0x6FE709, TechnoClass_Fire_BallisticScatter1, 6)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for FlakScatter && !Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(0));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE71C;
}

ASMJIT_PATCH(0x6FE7FE, TechnoClass_Fire_BallisticScatter2, 5)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for !FlakScatter || Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(RulesClass::Instance->BallisticScatter / 2));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE821;
}

ASMJIT_PATCH(0x707A47, TechnoClass_PointerGotInvalid_LastTarget, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->LastTarget == ptr)
		pThis->LastTarget = nullptr;

	return 0;
}

//TechnoClass_SetTarget_Burst
DEFINE_JUMP(LJMP, 0x6FCF53, 0x6FCF61);

ASMJIT_PATCH(0x717823, TechnoTypeClass_UpdatePalette_Reset, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->Palette = nullptr;

	return 0;
}ASMJIT_PATCH_AGAIN(0x717855, TechnoTypeClass_UpdatePalette_Reset, 0x6)


ASMJIT_PATCH(0x71136F, TechnoTypeClass_CTOR_Initialize, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->WeaponCount = 0; //default
	pThis->Bunkerable = false;
	pThis->Parasiteable = false;
	pThis->ImmuneToPoison = false;
	pThis->ConsideredAircraft = false;

	return 0;
}

// ASMJIT_PATCH(0x7119D5, TechnoTypeClass_CTOR_NoInit_Particles, 0x6)
// {
// 	GET(TechnoTypeClass*, pThis, ESI)

// 	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DamageParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();
// 	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DestroyParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();

// 	return 0x711A00;
// }

//TechnoClass_GetActionOnObject_IvanBombsB
DEFINE_JUMP(LJMP, 0x6FFF9E, 0x700006);

// ASMJIT_PATCH(0x6FF2D1, TechnoClass_FireAt_Facings, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	int nIdx = 0;
//
// 	if (pWeapon->Anim.Count > 1) { //only execute if the anim count is more than 1
// 		const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);
//
// 		// 2^highest is the frame count, 3 means 8 frames
// 		if (highest >= 3) {
// 			nIdx = pThis->GetRealFacing().GetValue(highest, 1u << (highest - 3));
// 		}
// 	}
//
// 	R->EDI(pWeapon->Anim.GetItemOrDefault(nIdx , nullptr));
// 	return 0x6FF31B;
// }

namespace UnlimboDetonateFireTemp
{
	BulletClass* Bullet;
	bool InSelected;
	bool InLimbo;
}

ASMJIT_PATCH(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);


	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto pBulletExt = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	const auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	UnlimboDetonateFireTemp::Bullet = ret;
	UnlimboDetonateFireTemp::InSelected = pThis->IsSelected;
	UnlimboDetonateFireTemp::InLimbo = pThis->InLimbo;
	R->EAX(ret);
	return 0x6FE562;
}

#include <Ext/Scenario/Body.h>


ASMJIT_PATCH(0x6FF7FF, TechnoClass_Fire_UnlimboDetonate, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WarheadTypeClass* const, pWH, EAX);

	const auto pBullet = UnlimboDetonateFireTemp::Bullet;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pThis->IsAlive && pThis->Health > 0 && pBullet
		&& !UnlimboDetonateFireTemp::InLimbo && !pWH->Parasite && pWHExt->UnlimboDetonate) {
		if (pWHExt->UnlimboDetonate_KeepSelected) {
			TechnoExtContainer::Instance.Find(pThis)->IsSelected = UnlimboDetonateFireTemp::InSelected;
			ScenarioExtData::Instance()->LimboLaunchers.emplace(pThis);
		}

		pBullet->Owner = pThis;
	}

	return 0;
}

ASMJIT_PATCH(0x48DC90, MapClass_UnselectAll_ClearLimboLaunchers, 0x5)
{
	for (const auto pExt : ScenarioExtData::Instance()->LimboLaunchers) {
		TechnoExtContainer::Instance.Find(pExt)->IsSelected = false;
	}

	ScenarioExtData::Instance()->LimboLaunchers.clear();

	return 0;
}

ASMJIT_PATCH(0x6F826E, TechnoClass_EvaluateObject_CivilianEnemy, 0x5)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EBP);

	enum {
		Undecided = 0,
		ConsiderEnemy = 0x6F8483,
		ConsiderCivilian = 0x6F83B1,
		Ignore = 0x6F894F
	};

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	// always consider this an enemy
	if (pExt->CivilianEnemy) {
		return ConsiderEnemy;
	}

	// if the potential target is attacking an allied object, consider it an enemy
	// to not allow civilians to overrun a player
	if (const auto pTargetTarget = flag_cast_to<TechnoClass*>(pTarget->Target)) {
		if (pThis->Owner->IsAlliedWith(pTargetTarget)) {
			const auto pData = RulesExtData::Instance();

			if (pThis->Owner->IsControlledByHuman() ?
				pData->AutoRepelPlayer : pData->AutoRepelAI) {
				return ConsiderEnemy;
			}
		}
	}

	return Undecided;
}

ASMJIT_PATCH(0x7162B0, TechnoTypeClass_GetPipMax_MindControl, 0x6)
{
	GET(TechnoTypeClass* const, pThis, ECX);

	int count = 0;
	for (int i = 0; i < 3; ++i) {
		if (auto pWeapon = pThis->GetWeapon(i)->WeaponType) {
			if (pWeapon->Warhead->MindControl && pWeapon->Damage > 0) {
				count = pWeapon->Damage;
				break;
			}
		}
	}

	R->EAX(count);
	return 0x7162BC;
}

ASMJIT_PATCH(0x6FE31C, TechnoClass_Fire_AllowDamage, 8)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	// whether conventional damage should be used
	const bool applyDamage =
		WeaponTypeExtContainer::Instance.Find(pWeapon)->ApplyDamage.Get(!pWeapon->IsSonic && !pWeapon->UseFireParticles);

	if (!applyDamage)
	{
		// clear damage
		R->EDI(0);
		return 0x6FE3DFu;
	}

	return 0x6FE32Fu;
}

// health bar for detected submerged units
ASMJIT_PATCH(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388  , CheckDrawHealthAllowed = 0x6F538E};

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x98, -0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExtData::DrawInsignia(pThis, pLocation, pBounds);

	bool drawHealth = pThis->IsSelected;
	if (!drawHealth)
	{
		// sensed submerged units
		drawHealth = !pThis->IsSurfaced()
			&& pThis->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
	}

	R->EAX(drawHealth);
	return CheckDrawHealthAllowed;
}

// ASMJIT_PATCH(0x70CBC3, TechnoClass_DealParticleDamage_FixArgs, 0x6)
// {
// 	GET(WeaponTypeClass*, pWeapon, EDI);
// 	GET(float, nDamage , ECX);
// 	GET(ObjectClass**, pVec, EDX);
// 	GET(int, nIdx, ESI);
// 	GET_STACK(TechnoClass*, pThis, 0xC0 - 0x44);
//
// 	int iDamage = (int)nDamage;
// 	pVec[nIdx]->ReceiveDamage(&iDamage, 0, pWeapon->Warhead, pThis, false, false, pThis->Owner);
//
// 	return 0x70CBEE;
// }

static inline bool CheckAttackMoveCanResetTarget(FootClass* pThis)
{
	const auto pTarget = pThis->Target;

	if (!pTarget || pTarget == pThis->MegaTarget)
		return false;

	const auto pTargetTechno = flag_cast_to<TechnoClass*, false>(pTarget);

	if (!pTargetTechno || pTargetTechno->IsArmed())
		return false;

	if (pThis->TargetingTimer.InProgress())
		return false;

	const auto pPrimaryWeapon = pThis->GetWeapon(0)->WeaponType;

	if (!pPrimaryWeapon)
		return false;

	const auto pNewTarget = flag_cast_to<TechnoClass*>(pThis->GreatestThreat(ThreatType::Range, &pThis->Location, false));

	if (!pNewTarget
		|| GET_TECHNOTYPE(pNewTarget) == GET_TECHNOTYPE(pTargetTechno))
		return false;

	const auto pSecondaryWeapon = pThis->GetWeapon(1)->WeaponType;

	if (!pSecondaryWeapon || !pSecondaryWeapon->NeverUse) // Melee unit's virtual scanner
		return true;

	return pSecondaryWeapon->Range <= pPrimaryWeapon->Range;
}

ASMJIT_PATCH(0x4DF3A0, FootClass_UpdateAttackMove_SelectNewTarget, 0x6)
{
	GET(FootClass* const, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->AttackMove_UpdateTarget.Get(RulesExtData::Instance()->AttackMove_UpdateTarget)
		&& CheckAttackMoveCanResetTarget(pThis))
	{
		pThis->Target = nullptr;
		pThis->HaveAttackMoveTarget = false;
		pExt->UpdateGattlingRateDownReset();
	}

	return 0;
}

#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x4DF4DB, FootClass_RefreshMegaMission_CheckMissionFix, 0xA)
{
	enum { ClearMegaMission = 0x4DF4F9, ContinueMegaMission = 0x4DF4CF };
	GET(FootClass*, pThis, ESI);

	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const mission = pThis->GetCurrentMission();
	bool stopWhenTargetAcquired = pTypeExt->AttackMove_StopWhenTargetAcquired.Get(RulesExtData::Instance()->AttackMove_StopWhenTargetAcquired.Get(!pType->OpportunityFire));
	bool clearMegaMission = mission != Mission::Guard;

	if (stopWhenTargetAcquired && clearMegaMission)
		clearMegaMission = !(mission == Mission::Move && pThis->MegaDestination && pThis->DistanceFrom(pThis->MegaDestination) > 256);

	return clearMegaMission ? ClearMegaMission : ContinueMegaMission;
}

ASMJIT_PATCH(0x4DF410, FootClass_UpdateAttackMove_TargetAcquired, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pThis->IsCloseEnoughToAttack(pThis->Target)
		&& pTypeExt->AttackMove_StopWhenTargetAcquired.Get(RulesExtData::Instance()->AttackMove_StopWhenTargetAcquired.Get(!pType->OpportunityFire)))
	{
		if (auto const pJumpjetLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
		{
			auto const crd = pThis->GetCoords();
			pJumpjetLoco->HeadToCoord.X = crd.X;
			pJumpjetLoco->HeadToCoord.Y = crd.Y;
			pJumpjetLoco->Speed = 0;
			pJumpjetLoco->__maxSpeed = 0;
			pJumpjetLoco->NextState = JumpjetLocomotionClass::State::Hovering;
			pThis->AbortMotion();
		}
		else
		{
			pThis->StopMoving();
			pThis->AbortMotion();
		}
	}

	if (pTypeExt->AttackMove_PursuitTarget)
		pThis->SetDestination(pThis->Target, true);

	return 0;
}

ASMJIT_PATCH(0x711E90, TechnoTypeClass_CanAttackMove_IgnoreWeapon, 0x6)
{
	enum { SkipGameCode = 0x711E9A };
	return RulesExtData::Instance()->AttackMove_IgnoreWeaponCheck ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x4DF3A6, FootClass_UpdateAttackMove_Follow, 0x6)
{
	enum { FuncRet = 0x4DF425 };

	GET(FootClass*, pThis, ESI);

	Mission mission = pThis->GetCurrentMission();

	// Refresh mega mission if mission is somehow changed to incorrect missions.
	if (mission != Mission::Attack && mission != Mission::Move){

		bool continueMission = true;

		// Aug 30, 2025 - Starkku: SimpleDeployer needs special handling here.
		// Without this if you interrupt waypoint mode path with deploy command
		// it will not execute properly as it interrupts it with movement.

		if (mission == Mission::Unload) {
			if (auto const pUnit = cast_to<UnitClass*>(pThis)) {
				if (pUnit->Type->IsSimpleDeployer)
					continueMission = false;


			}
		}

		if (continueMission)
			pThis->ContinueMegaMission();

	}

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->AttackMove_Follow || pTypeExt->AttackMove_Follow_IfMindControlIsFull && pThis->CaptureManager && pThis->CaptureManager->CannotControlAnyMore())
	{
		auto const& pTechnoVectors = Helpers::Alex::getCellSpreadItems(pThis->GetCoords(),
			pThis->GetGuardRange(2) / Unsorted::LeptonsPerCell, pTypeExt->AttackMove_Follow_IncludeAir,false , true , true , false);

		TechnoClass* pClosestTarget = nullptr;
		int closestRange = 65536;
		auto pMegaMissionTarget = pThis->MegaDestination ? pThis->MegaDestination : (pThis->MegaTarget ? pThis->MegaTarget : pThis);

		for (auto const pTechno : pTechnoVectors)
		{
			if ((pTechno->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None
				&& pTechno != pThis && pTechno->Owner == pThis->Owner
				&& pTechno->MegaMissionIsAttackMove())
			{
				auto const pTargetExt = TechnoExtContainer::Instance.Find(pTechno);

				// Check this to prevent the followed techno from being surrounded
				if (pTargetExt->AttackMoveFollowerTempCount >= 6)
					continue;

				auto const pTargetTypeExt = GET_TECHNOTYPEEXT(pTechno);

				if (!pTargetTypeExt->AttackMove_Follow)
				{
					auto const dist = pTechno->DistanceFrom(pMegaMissionTarget);

					if (dist < closestRange)
					{
						pClosestTarget = pTechno;
						closestRange = dist;
					}
				}
			}
		}

		if (pClosestTarget)
		{
			auto const pTargetExt = TechnoExtContainer::Instance.Find(pClosestTarget);
			pTargetExt->AttackMoveFollowerTempCount += pThis->WhatAmI() == AbstractType::Infantry ? 1 : 3;
			pThis->SetDestination(pClosestTarget, false);
			pThis->SetArchiveTarget(pClosestTarget);
			pThis->QueueMission(Mission::Area_Guard, true);
		}
		else
		{
			if (pThis->MegaTarget)
				pThis->SetDestination(pThis->MegaTarget, false);
			else // MegaDestination can be nullptr
				pThis->SetDestination(pThis->MegaDestination, false);
		}

		pThis->ClearMegaMissionData();

		R->EAX(pClosestTarget);
		return FuncRet;
	}

	return 0;
}
