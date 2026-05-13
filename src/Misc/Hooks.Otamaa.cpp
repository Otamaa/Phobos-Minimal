#pragma region Includes
#include <Utilities/Macro.h>

#include <AbstractClass.h>
#include <ObjectClass.h>
#include <AnimClass.h>
#include <BulletClass.h>
#include <VoxelAnimClass.h>
#include <BounceClass.h>
#include <MissionClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <AircraftClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ParticleClass.h>
#include <WaveClass.h>
#include <RadSiteClass.h>
#include <SmudgeTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>
#include <TeamTypeClass.h>
#include <TeamClass.h>
#include <TiberiumClass.h>
#include <TerrainClass.h>
#include <TerrainTypeClass.h>
#include <TEventClass.h>
#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <CellClass.h>
#include <VocClass.h>

#include <MapClass.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/House/Body.h>

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <IsometricTileTypeClass.h>

#include <TiberiumClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <AircraftTrackerClass.h>

#include <Memory.h>

#include <Locomotor/Cast.h>

#include <Ext/TerrainType/Body.h>
#include <Misc/DamageArea.h>

#include <Surface.h>

#include <Audio.h>

#include <Commands/ShowTeamLeader.h>

#include <Commands/ToggleRadialIndicatorDrawMode.h>

#include <TextDrawing.h>
#include <format>

#include <Ext/SWType/Body.h>
#include <New/Type/CrateTypeClass.h>

#include <SpotlightClass.h>
#include <New/Entity/FlyingStrings.h>

#include <ExtraHeaders/StackVector.h>

#include <Ext/RadSite/Body.h>
#include <Misc/PhobosGlobal.h>

#pragma endregion








// ASMJIT_PATCH(0x6FC22A, TechnoClass_CanFire_AttackICUnit, 0x6)
// {
// 	enum { ContinueCheck = 0x6FC23A, BypassCheck = 0x6FC24D };
// 	GET(TechnoClass* const, pThis, ESI);
//
// 	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
//
// 	if(!pWHExt->CanTargetIronCurtained)
// 	//TODO : re-eval check below  , i if desync/the behaviour is not good , change it to pThis->Owner->IsControlledByCurrentPlayer()
//
// 	const bool IsHuman = pThis->Owner->IsControlledByHuman();
// 	const bool AllowAI = !pWHExt->CanTargetIronCurtained.Get(pTypeExt->AllowFire_IroncurtainedTarget.Get(RulesExtData::Instance()->AutoAttackICedTarget));
// 	const bool Allow = pTypeExt->AllowFire_IroncurtainedTarget.Get(RulesExtData::Instance()->AutoAttackICedTarget);
//
// 	return IsHuman &&  Allow || !IsHuman && AllowAI
// 		? BypassCheck : ContinueCheck;
// }
DEFINE_JUMP(LJMP, 0x6FC22A, 0x6FC24D) // Skip IronCurtain check

static_assert(offsetof(HouseClass, IsHumanPlayer) == 0x1EC, "ClassMember Shifted !");
static_assert(offsetof(HouseClass, IsInPlayerControl) == 0x1ED, "ClassMember Shifted !");









// ASMJIT_PATCH(0x508EE5, HouseClass_UpdateRadar_LimboDeliver, 0x6)
// {
// 	GET(FakeBuildingClass*, pBld, EAX);
// 	enum
// 	{
// 		ContinueLoop = 0x508F08,
// 		ContinueCheck = 0x0,
// 		EligibleRadar = 0x508F2A
// 	};
//
// 	if (TechnoExtContainer::Instance.Find(pBld)->AE.DisableRadar)
// 		return ContinueLoop;
//
// 	if (!pBld->_GetExtData()->RegisteredJammers.empty())
// 		return ContinueLoop;
//
// 	if (pBld->EMPLockRemaining > 0)
// 		return ContinueLoop;
//
// 	// if the `Limboed` Building has radar , just accept it
// 	if(pBld->_GetExtData()->LimboID != -1)
// 		return EligibleRadar;
//
// 	return ContinueCheck;
// }



// ASMJIT_PATCH(0x6E93BE, TeamClass_AI_TransportTargetLog, 0x5)
// {
// 	GET(FootClass* const, pThis, EDI);
// 	Debug::LogInfo("[{}][{}] Transport just recieved orders to go home after unloading ", (void*)pThis, pThis->get_ID());
// 	return 0x6E93D6;
// }

// ASMJIT_PATCH(0x6EF9B0, TeamMissionClass_GatherAtEnemyCell_Log, 0x5)
// {
// 	GET_STACK(short const, nCellX, 0x10);
// 	GET_STACK(short const, nCellY, 0x12);
// 	GET(TeamClass* const, pThis, ESI);
// 	GET(TechnoClass* const, pTechno, EDI);
// 	Debug::LogInfo("[{}][{}] Team with Owner '{}' has chosen ({} , {}) for its GatherAtEnemy cell.", (void*)pThis, pThis->Type->ID, pTechno->Owner ? pTechno->Owner->get_ID() : GameStrings::NoneStrb(), nCellX, nCellY);
// 	return 0x6EF9D0;
// }



#include <Ext/Tiberium/Body.h>


#include <Ext/Cell/Body.h>









// ASMJIT_PATCH(0x4242F4, AnimClass_Trail_Override, 0x6)
// {
// 	GET(AnimClass*, pAnim, EDI);
// 	GET(AnimClass*, pThis, ESI);
//
// 	auto nCoord = pThis->GetCoords();
// 	pAnim->AnimClass::AnimClass(pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_600, 0, false);
// 	//const auto pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
// 	TechnoClass* const pTech = AnimExtData::GetTechnoInvoker(pThis);
// 	HouseClass* const pOwner = !pThis->Owner && pTech ? pTech->Owner : pThis->Owner;
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);
//
// 	return 0x424322;
// }






//TechnoClass_GetWeaponState
// ASMJIT_PATCH(0x6FCA30, TechnoClass_CanFire_DecloakToFire, 6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(WeaponTypeClass* const, pWeapon, EBX);

// 	const auto pTransporter = pThis->Transporter;

// 	if (pTransporter && pTransporter->CloakState != CloakState::Uncloaked)
// 		return 0x6FCA4F;

// 	if (pThis->CloakState == CloakState::Uncloaked)
// 		return 0x6FCA5E;

// 	if (!pWeapon->DecloakToFire && pThis->WhatAmI() == AircraftClass::AbsID)
// 		return 0x6FCA4F;

// 	return pThis->CloakState == CloakState::Cloaked ? 0x6FCA4F : 0x6FCA5E;
// 	//return 0x0;
// }


// ASMJIT_PATCH(0x740015, UnitClass_WhatAction_NoManualEject, 0x6)
// {
// 	GET(TechnoTypeClass* const, pType, EAX);
// 	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x7400F0 : 0x0;
// }




// Various call of TechnoClass::SetOwningHouse not respecting overloaded 2nd args fix !







// static BuildingClass* IsAnySpysatActive(HouseClass* pThis)
// {
// 	const bool IsCurrentPlayer = pThis->ControlledByCurrentPlayer();
// 	const bool IsCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
// 	const bool IsSpysatActulallyAllowed = !IsCampaign ? pThis == HouseClass::CurrentPlayer() :  IsCurrentPlayer;

// 	//===============reset all
// 	pThis->CostDefensesMult = 1.0;
// 	pThis->CostUnitsMult = 1.0;
// 	pThis->CostInfantryMult = 1.0;
// 	pThis->CostBuildingsMult = 1.0;
// 	pThis->CostAircraftMult = 1.0;
// 	BuildingClass* Spysat = nullptr;
// 	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis);

// 	pHouseExt->Building_BuildSpeedBonusCounter.clear();
// 	pHouseExt->Building_OrePurifiersCounter.clear();
// 	pHouseExt->RestrictedFactoryPlants.clear();
// 	pHouseExt->BattlePointsCollectors.clear();

// 	//==========================
// 	//const bool LowpOwerHouse = pThis->HasLowPower();

// 	for (auto const& pBld : pThis->Buildings)
// 	{
// 		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
// 		{
// 			const auto pExt = BuildingExtContainer::Instance.Find(pBld);
// 			const bool IsLimboDelivered = pExt->LimboID != -1;

// 			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
// 				continue;

// 			if (pBld->TemporalTargetingMe
// 				|| pExt->AboutToChronoshift
// 				|| pBld->IsBeingWarpedOut())
// 				continue;

// 			const bool Online = pBld->IsPowerOnline(); // check power
// 			const auto pTypes = pBld->GetTypes(); // building types include upgrades
// 			const bool Jammered = !pExt->RegisteredJammers.empty();  // is this building jammed

// 			for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin)
// 			{

// 				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(*begin);
// 				//const auto Powered_ = pBld->IsOverpowered || (!PowerDown && !((*begin)->PowerDrain && LowpOwerHouse));

// 				const bool IsBattlePointsCollectorPowered = !pTypeExt->BattlePointsCollector_RequirePower || ((*begin)->Powered && Online);
// 				if (pTypeExt->BattlePointsCollector && IsBattlePointsCollectorPowered)
// 				{
// 					++pHouseExt->BattlePointsCollectors[(*begin)];
// 				}

// 				const bool IsFactoryPowered = !pTypeExt->FactoryPlant_RequirePower || ((*begin)->Powered && Online);

// 				//recalculate the multiplier
// 				if ((*begin)->FactoryPlant && IsFactoryPowered)
// 				{
// 					if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
// 					{
// 						pHouseExt->RestrictedFactoryPlants.emplace(pBld);
// 					}

// 					pThis->CostDefensesMult *= (*begin)->DefensesCostBonus;
// 					pThis->CostUnitsMult *= (*begin)->UnitsCostBonus;
// 					pThis->CostInfantryMult *= (*begin)->InfantryCostBonus;
// 					pThis->CostBuildingsMult *= (*begin)->BuildingsCostBonus;
// 					pThis->CostAircraftMult *= (*begin)->AircraftCostBonus;
// 				}

// 				if(IsSpysatActulallyAllowed && !Spysat) {
// 					//only pick avaible spysat
// 					if (!TechnoExtContainer::Instance.Find(pBld)->AE.DisableSpySat) {
// 						const bool IsSpySatPowered = !pTypeExt->SpySat_RequirePower || ((*begin)->Powered && Online);
// 						if ((*begin)->SpySat && !Jammered && IsSpySatPowered) {
// 							if (IsLimboDelivered || !IsCampaign || pBld->DiscoveredByCurrentPlayer) {
// 								Spysat = pBld;
// 							}
// 						}
// 					}
// 				}

// 				// add eligible building
// 				if (pTypeExt->SpeedBonus.Enabled && Online)
// 					++pHouseExt->Building_BuildSpeedBonusCounter[(*begin)];

// 				const bool IsPurifierRequirePower = !pTypeExt->PurifierBonus_RequirePower || ((*begin)->Powered && Online);
// 				// add eligible purifier
// 				if ((*begin)->OrePurifier && IsPurifierRequirePower)
// 					++pHouseExt->Building_OrePurifiersCounter[(*begin)];
// 			}
// 		}
// 	}

// 	//count them
// 	for (auto& purifier : pHouseExt->Building_OrePurifiersCounter)
// 		pThis->NumOrePurifiers += purifier.second;

// 	return Spysat;
// }

// ASMJIT_PATCH(0x508F79, HouseClass_AI_CheckSpySat, 0x5)
// {
// 	enum
// 	{
// 		ActivateSpySat = 0x509054,
// 		DeactivateSpySat = 0x509002
// 	};

// 	GET(HouseClass*, pThis, ESI);
// 	return IsAnySpysatActive(pThis) ? ActivateSpySat : DeactivateSpySat;
// }






//// 7384C3 ,7385BB UnitClass take damage func











// ASMJIT_PATCH(0x6ECF67, TeamClass_ChangeHouse_nullptrresult, 0x6)
// {
// 	GET(TeamClass*, pThis, ESI);
// 	GET(int, args, ECX);
// 	GET(FootClass*, pCurMember, EDI);
//
// 	const auto pHouse = HouseClass::FindByCountryIndex(args);
// 	if (!pHouse)
// 	{
// 		const auto nonestr = GameStrings::NoneStr();
// 		Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]",
// 			pThis->OwnerHouse ? pThis->OwnerHouse->get_ID() : nonestr, pThis->OwnerHouse,
// 			pThis->get_ID(), pThis, args);
// 	}
//
// 	pCurMember->SetOwningHouse(pHouse);
// 	R->EBP(pCurMember->NextTeamMember);
// 	return 0x6E96A8;
// }
























static_assert(offsetof(TechnoClass, Airstrike) == 0x294, "ClassMember Shifted !");



// ASMJIT_PATCH(0x6FDB80, TechnoClass_AdjustDamage_Handle, 0x6)
// {
// 	GET(TechnoClass*, pThis, ECX);
// 	GET_STACK(TechnoClass*, pVictim, 0x4);
// 	GET_STACK(WeaponTypeClass*, pWeapon, 0x8);
//
//
// 	R->EAX(damage);
// 	return 0x6FDD35;
// }







#include <Ext/Bomb/Body.h>



//  ASMJIT_PATCH(0x6E20AC, TActionClass_DetroyAttachedTechno, 0x8)
//  {
//  	GET(TechnoClass*, pTarget, ESI);
//
//  	if (auto pBld = cast_to<BuildingClass*>(pTarget))
//  	{
//  		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
//  		{
//  			BuildingExtData::LimboKill(pBld);
//  			return 0x6E20D8;
//  		}
//  	}
//
//  	return 0x0;
//  }






#include <VoxClass.h>

#ifndef CRATE_HOOKS





#endif











#pragma region Assaulter
#pragma endregion

// ASMJIT_PATCH(0x4580CB, BuildingClass_KickAllOccupants_HousePointerMissing, 0x6)
// {
// 	GET(BuildingClass*, pThis, ESI);
// 	GET(FootClass*, pOccupier, EDI);

// 	if (!pThis->Owner)
// 	{
// 		Debug::FatalErrorAndExit("BuildingClass::KickAllOccupants for [%x(%s)] Missing Occupier [%x(%s)] House Pointer !",
// 			pThis,
// 			pThis->get_ID(),
// 			pOccupier,
// 			pOccupier->get_ID()
// 		);
// 	}

// 	return 0x0;
// }


#include <SlaveManagerClass.h>






// ASMJIT_PATCH(0x4DA64D, FootClass_Update_IsInPlayField, 0x6)
// {
// 	GET(UnitTypeClass* const, pType, EAX);
// 	return pType->BalloonHover || pType->JumpJet ? 0x4DA655 : 0x4DA677;
// }

//ASMJIT_PATCH(0x4145B6, AircraftClass_RenderCrash_, 0x6)
//{
//	GET(AircraftTypeClass*, pType, ESI);
//
//	if (!pType->MainVoxel.HVA)
//	{
//		Debug::LogInfo("Aircraft[{}] Has No HVA ! ", pType->ID);
//		return 0x4149F6;
//	}
//
//	return 0x0;
//}


// ASMJIT_PATCH(0x6EDA50, Team_DoMission_Harvest, 0x5)
// {
// 	GET(Mission, setTo, EBX);
// 	GET(FootClass*, pMember, ESI);

// 	if (setTo == Mission::Harvest)
// 	{
// 		pMember->EnterIdleMode(false, true);
// 		return 0x6EDA77;
// 	}

// 	return 0x0;
// }

// Pathfinding code moved to src/Ext/Pathfinding/







//ASMJIT_PATCH(0x418072, AircraftClass_Mission_Attack_PickAttackLocation, 0x5)
//{
//	GET(AircraftClass*, pAir, ESI);
//
//	if (!pAir->Type->MissileSpawn && !pAir->Type->Fighter && !pAir->Is_Strafe())
//	{
//		AbstractClass* pTarget = pAir->Target;
//
//		int weaponIdx = pAir->SelectWeapon(pTarget);
//		if (pAir->IsCloseEnough(pTarget, weaponIdx))
//		{
//			pAir->IsLocked = true;
//			CoordStruct pos = pAir->GetCoords();
//			CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);
//			pAir->SetDestination(pCell, true);
//			return 0x418087;
//		}
//		else if (WeaponTypeClass* pWeapon = pAir->GetWeapon(weaponIdx)->WeaponType)
//		{
//			int dest = pAir->DistanceFrom(pAir->Target);
//			CoordStruct nextPos = CoordStruct::Empty;
//
//			if (dest < pWeapon->MinimumRange)
//			{
//				CoordStruct flh = CoordStruct::Empty;
//				flh.X = (int)(pWeapon->Range * 0.5);
//				nextPos = TechnoExtData::GetFLHAbsoluteCoords(pAir, flh, true);
//			}
//			else if (dest > pWeapon->Range)
//			{
//				int length = (int)(pWeapon->Range * 0.5);
//				int flipY = 1;
//
//				if (ScenarioClass::Instance->Random.RandomRanged(0, 1) == 1)
//				{
//					flipY *= -1;
//				}
//
//				CoordStruct sourcePos = pAir->GetCoords();
//				int r = (dest - length) * Unsorted::LeptonsPerCell;
//				r = ScenarioClass::Instance->Random.RandomRanged(0, r);
//				CoordStruct flh { 0, r * flipY, 0 };
//				CoordStruct targetPos = pAir->Target->GetCoords();
//				DirStruct dir = Helpers_DP::Point2Dir(sourcePos, targetPos);
//				sourcePos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, flh, dir);
//				sourcePos.Z = 0;
//				targetPos.Z = 0;
//				nextPos = Helpers_DP::GetForwardCoords(targetPos, sourcePos, length);
//			}
//			if (!nextPos.IsEmpty())
//			{
//				CellClass* pCell = MapClass::Instance->TryGetCellAt(nextPos);
//				pAir->SetDestination(pCell, true);
//				return 0x418087;
//			}
//		}
//	}
//
//	return 0;
//}

//ASMJIT_PATCH(0x4181CF, AircraftClass_Mission_Attack_FlyToPostion, 0x5)
//{
//	GET(AircraftClass*, pAir, ESI);
//	if (!pAir->Type->MissileSpawn && !pAir->Type->Fighter)
//	{
//		pAir->MissionStatus = 0x4; // AIR_ATT_FIRE_AT_TARGET0
//		return 0x4181E6;
//	}
//
//	return 0;
//}

DEFINE_JUMP(LJMP, 0x4184FC, 0x418506);
// ASMJIT_PATCH(0x4184FC, AircraftClass_Mission_Attack_Fire_Zero, 0x6) {
// 	return 0x418506;
// }

#include <WeaponTypeClass.h>



// ASMJIT_PATCH(0x4C2A02, Ebolt_DTOR_TechnoIsNotTechno, 0x6)
// {
// 	GET(TechnoClass*, pTr, ECX);
// 	const auto vtable = VTable::Get(pTr);

// 	if (vtable != AircraftClass::vtable
// 		&& vtable != UnitClass::vtable
// 		&& vtable != BuildingClass::vtable
// 		&& vtable != InfantryClass::vtable
// 		)
// 	{
// 		return R->Origin() + 0x6; //skip setting ebolt for the techno because it corrupted pointer
// 	}

// 	return 0x0;
// }ASMJIT_PATCH_AGAIN(0x4C2C19, Ebolt_DTOR_TechnoIsNotTechno, 0x6)



//ASMJIT_PATCH(0x4AED70, Game_DrawSHP_WhoCallMe, 0x6)
//{
//	GET(ConvertClass*, pConvert, EDX);
//	GET_STACK(SHPStruct*, pSHP, 0xA4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pConvert)
//	{
//		auto pSHPref = pSHP->AsReference();
//		Debug::FatalErrorAndExit("Draw SHP[%s] missing Convert , caller [%0x]", pSHPref ? pSHPref->Filename : "unknown", caller);
//	}
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x4AED70, Game_DrawSHP_WhoCallMe, 0x6)
//{
//	GET(ConvertClass*, pConvert, EDX);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pConvert) {
//		Debug::FatalErrorAndExit("Draw SHP missing Convert , caller [%0x]" , caller);
//	}
//
//	return 0x0;
//}
ASMJIT_PATCH(0x42CB61, AstarClass_Find_Path_FailLog_Hierarchical, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Hierarchical findpath failure: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CB86;
}

ASMJIT_PATCH(0x42CBC9, AstarClass_Find_Path_FailLog_WithoutHierarchical, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Warning.  A* without HS: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CBE6;
}

ASMJIT_PATCH(0x42CC48, AstarClass_Find_Path_FailLog_FindPath, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Regular findpath failure: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CC6D;
}

//DEFINE_JUMP(LJMP, 0x052CAD7, 0x52CAE9);
//ASMJIT_PATCH(0x50B6F0, HouseClass_Player_Has_Control_WhoTheFuckCalling, 0x5)
//{
//	GET(HouseClass*, pHouyse, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pHouyse)
//		Debug::FatalError("Fucking no House %x", caller);
//
//	return 0x0;
//}



//ASMJIT_PATCH(0x6F89D1, TechnoClass_EvaluateCell_DeadTechno, 0x6)
//{
//	GET(ObjectClass*, pCellObj, EDI);
//
//	if (pCellObj && !pCellObj->IsAlive)
//		pCellObj = nullptr;
//
//	R->EDI(pCellObj);
//
//	return 0x0;
//}

// ASMJIT_PATCH(0x51C251, InfantryClass_CanEnterCell_InvalidObject, 0x8)
// {
// 	GET(ObjectClass*, pCellObj, ESI);
//
// 	if (!pCellObj->IsAlive)
// 	{
// 		return 0x51C78F;
// 	}
//
// 	return R->ESI() == R->EBP() ? 0x51C70F : 0x51C259;
// }

//ASMJIT_PATCH(0x417CC0, AircraftClass_WhatAction_caller, 0x5)
//{
//	GET(AircraftClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0);
//
//	if (!pThis->IsAlive)
//		Debug::LogInfo(__FUNCTION__" DeadTechno[{}] is used , called from [{}]", (void*)pThis, (unsigned)caller);
//
//	return 0x0;
//}



//ASMJIT_PATCH(0x6F7CA0, TechnoClass_EvalObject_EarlyObjectEval, 0x5)
//{
//	GET_STACK(AbstractClass*, pTarget, 0x10);
//	retfunc_fixed<bool> _return (R, 0x6F8958, false);
//
//	if(!pTarget) {
//		return _return();
//	}
//
//	if (auto pObj = flag_cast_to<ObjectClass* , false>(pTarget)) {
//		if (!pObj->IsAlive) {
//			return _return();
//		}
//	}
//
//	if (const auto pTechno = flag_cast_to<TechnoClass*, false>(pTarget))
//	{
//		if (pTechno->IsCrashing || pTechno->IsSinking) {
//			return _return();
//		}
//
//
//		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());
//
//		if (pTypeExt->IsDummy) {
//			return _return();
//		}
//
//		switch (pTechno->WhatAmI())
//		{
//		case AbstractType::Building:
//		{
//			const auto pBld = (BuildingClass*)pTarget;
//
//			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) {
//				return _return();
//			}
//
//			break;
//		}
//		case AbstractType::Unit:
//		{
//
//			const auto pUnit = (UnitClass*)pTarget;
//
//			if (pUnit->DeathFrameCounter > 0) {
//				return _return();
//			}
//
//			break;
//		}
//		default:
//			break;
//		}
//	}
//
//	return 0x0;
//}



#include <OverlayClass.h>



//ASMJIT_PATCH(0x7C8B3D, game_Dele_whoCall, 0x9)
//{
//	GET_STACK(void* , ptr , 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("Caller 0x%x \n" , caller);
//	CRT::free(ptr);
//	return 0x007C8B47;
//}
//
//ASMJIT_PATCH(0x7C93E8, game_freeMem_caller, 0x5) {
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("CRT::free Caller 0x%x \n", caller);
//	return 0x0;
//}
//ASMJIT_PATCH(0x5C5070, DVC_NoneNameType_clear_, 0x6)
//{
//	GET(DynamicVectorClass<NodeNameType*>*, pThis, ECX);
//
//	pThis->Count = 0;
//	if (pThis->Items && pThis->IsAllocated)
//	{
//		Debug::Log("Caller DVC_NoneNameType_clear_ \n");
//		CRT::free(pThis->Items);
//
//		pThis->Items = nullptr;
//	}
//	pThis->IsAllocated = 0;
//	pThis->Capacity = 0;
//	return 0x5C5099;
//}

//#pragma optimize("", off )
//std::string _tempName = GameStrings::NoneStr();
//ASMJIT_PATCH(0x69E149, SHPStruct_deleteptr_check_getName, 0x5)
//{
//	GET(SHPStruct*, ptr, ESI);
//	_tempName = ptr->AsReference()->Filename;
//	return 0x0;
//}
//int count_ = 0;
//ASMJIT_PATCH(0x69E1EC, SHPStruct_deleteptr_check, 0x6)
//{
//	GET(SHPStruct*, ptr, ESI);
//	Debug::Log("Caller SHPStruct_deleteptr_check deleting [%d][0x%x][%s]\n", count_++,ptr , _tempName.c_str());
//
//	if (count_ == 251)
//	{
//		auto as_ = ptr->AsReference();
//		DebugBreak();
//	}
//
//
//	_tempName = GameStrings::NoneStr();
//	CRT::free(ptr);
//	return 0x69E1F5;
//}
//#pragma optimize("", on )



//ASMJIT_PATCH(0x7B4940, WString_OperatorSet_empty, 0x5)
//{
//	GET_STACK(Wstring*, pString, 0x4);
//	GET_STACK(DWORD, caller , 0x0);
//
//	if (!pString)
//		Debug::FatalError("Empty String set for wstring caller  %x\n", caller);
//
//	return 0x0;
//}


//ASMJIT_PATCH(0x7BB350, XSurface_DrawSurface_InvalidSurface, 0x5)
//{
//	GET(XSurface*, pThis, ESI);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pThis ||
//		(
//			VTable::Get(pThis) != XSurface::vtable &&
//			VTable::Get(pThis) != DSurface::vtable &&
//			VTable::Get(pThis) != BSurface::vtable &&
//			VTable::Get(pThis) != Surface::vtable
//
//		))
//		Debug::FatalError("Invalid XSurface Caller %x\n", caller);
//
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x7086C3, HouseClass_Is_Attacked_Exclude, 0x6)
//{
//	GET(UnitClass*, pCandidate, ESI);
//
//
//	return 0x70874C;//increment
//}

//ASMJIT_PATCH(0x4E0052, FootClass_TryBunkering, 0x5)
//{
//	GET(FootClass*, pThis, EDI);
//	GET(TechnoClass*, pRecipient, ESI);
//
//	if (!pThis->__ProtectMe_3CF) {
//		if (pThis->SendCommand(RadioCommand::RequestLink, pRecipient) == RadioCommand::AnswerPositive)
//		{
//			//check it first
//			return 0x4E005F;
//		}
//	}
//
//	return 0x4E003B;
//}

//bunker state AI 458E50


//ASMJIT_PATCH(0x4DEBB0, ObjectClass_Crash_Probe, 0x6)
//{
//	GET(ObjectClass*, pThis, ECX);
//
//	//return false case
//	if(pThis->GetHeight() <= 0 || Unsorted::ScenarioInit())
//		Debug::FatalError("Crash Probe ssomething is wrong\n");
//
//	return 0x0;
//}

// ASMJIT_PATCH(0x41D9A0, AirstrikClass_Setup, 0x6)
// {
// 	//GET(AirstrikeClass*, pThis, EDI);
// 	GET(BuildingClass*, pTarget, ESI);

// 	pTarget->IsAirstrikeTargetingMe = true;
// 	pTarget->Mark(MarkType::Redraw);

// 	return 0x41DA0B;
// }


//ASMJIT_PATCH(0x42C2B8 , FootClass_Find_Path_Hirarcial_Dies, 0x7){
//	GET(FootClass* , pFoot , ESI);
//
//	if(!pFoot->IsAlive || pFoot->IsCrashing || pFoot->IsSinking)
//		return 0x42C2CF;
//
//	R->EAX(pFoot->GetThreatAvoidance());
//	return 0x42C2BF;
//}

//#include <TubeClass.h>
//static int idx_pathfind;
//static DWORD calladdr;
//
//CellStruct* __fastcall TubeFacing_429780(CellStruct* pRet, CellStruct* pLoc , int idx, int* path) {
//
//	if (size_t(idx) < 24)
//	{
//		for (int count = idx; count > 0; --count)
//		{
//			for (auto begin = path; begin < (path + 24); ++begin)
//			{
//				if (size_t(*begin) > 7u)
//				{
//					const int TubeIndex = MapClass::Instance->GetCellAt(pLoc)->TubeIndex;
//					if ((size_t)TubeIndex < TubeClass::Array.size())
//					{
//						*pRet = TubeClass::Array.Items[TubeIndex]->ExitCell;
//						return pRet;
//					}
//				}
//				else
//				{
//					*pRet = pLoc->operator+(CellSpread::AdjacentCell[*begin]);
//					return pRet;
//				}
//			}
//		}
//	}
//	else {
//		Debug::Log("%x FindPath with idx %d : in %d\n", calladdr , idx , idx_pathfind);
//	}
//
//	*pRet = *pLoc;
//	return pRet;
//}
//
//DEFINE_FUNCTION_JUMP(LJMP, 0x429780, TubeFacing_429780);
//
//ASMJIT_PATCH(0x4D3920, FootClass_basic_rememberIdx, 0x5)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(int, idx, 0x8);
//
//	if(size_t(idx) > 24 ){
//		idx_pathfind = idx;
//		calladdr = caller;
//	}
//
//	return 0x0;
//}
//#pragma optimize("", off )
//ASMJIT_PATCH(0x4D3E5A, FootClass_basic_idkCrash, 0x5)
//{
//	GET(int, idx, EBX);
//	GET(FootClass*, pFoot, EBP);
//	GET(PathType*, pFind, EDI);
//	LEA_STACK(PathType*, pPath, 0x4C);
//	std::memcpy(pPath, pFind, sizeof(PathType));
//
//	pFoot->FixupPath((DWORD)pPath);
//
//	int _idx = 24 - idx;
//	if (pPath->Length < _idx)
//		_idx = pPath->Length;
//
//	LEA_STACK(int*, _dummy, 0x6C);
//
//	if (size_t(_idx) > 24) {
//		Debug::Log("Pathfind for ([%x]%s - %s) trying to find path with overflow index %d !\n" , pFoot, pFoot->get_ID() , pFoot->GetThisClassName() , _idx);
//		//_idx = pPath->Length;
//		//idx = 0; //??
//	}
//
//	std::memcpy(&pFoot->PathDirections[idx],  _dummy, 4 * _idx);
//	return 0x4D3EA1;
//}
//#pragma optimize("", on )

//ASMJIT_PATCH(0x5D4E3B, DispatchingMessage_ReloadResources, 0x5)
//{
//	LEA_STACK(LPMSG, pMsg, 0x10);
//
//	if (pMsg->message == 16 || pMsg->message == 2 || pMsg->message == 0x112 && pMsg->wParam == 0xF060)
//		ExitProcess(1u);
//
//	//const bool altDown = (pMsg->lParam & 0x20000000) != 0;
//	//if ((pMsg->message == 0x104 || pMsg->message == 0x100) && pMsg->wParam == 0xD && altDown) {}
//
//	TranslateMessage(pMsg);
//	DispatchMessageA(pMsg);
//	return 0x5D4E4D;
//}



// class NOVTABLE FakeLayerClass : LayerClass
// {
// public:


// };
// static_assert(sizeof(FakeLayerClass) == sizeof(LayerClass), "Invalid Size !");

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E607C, FakeLayerClass::_Submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x55BABB, FakeLayerClass::_Submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x4A9759, FakeLayerClass::_Submit);


//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E7F30, FakeDriveLocomotionClass::_Is_Moving_Now);


#pragma region ElectricAssultStuffs






// ASMJIT_PATCH(0x534849, Game_Destroyvector_SpawnManage, 0x6)
// {
// 	for (int i = 0; i < SpawnManagerClass::Array->Count; ++i)
// 	{
// 		if (auto pManager = SpawnManagerClass::Array->Items[i])
// 		{
//
// 			if (VTable::Get(pManager) != 0x7F3650)
// 				continue;
//
// 			pManager->~SpawnManagerClass();
// 		}
// 	}
//
// 	return 0x53486B;
// }

//#pragma optimize("", off )
//ASMJIT_PATCH(0x6ED155, TMissionAttack_WhatTarget, 0x5) {
//	GET(FootClass*, pTeam, EDI);
//	GET(TeamClass*, pThis, EBP);
//	GET(ThreatType, threat, EAX);
//	LEA_STACK(CoordStruct*, pCoord, 0x18);
//
//	auto pTarget = pTeam->GreatestThreat(threat, pCoord, (bool)R->CL());
//
//	if (IS_SAME_STR_("HTNK", pTeam->get_ID()) && flag_cast_to<TechnoClass*>(pTarget))
//		Debug::Log("HTNK Target %s - %s \n", pTarget->GetThisClassName() , ((TechnoClass*)pTarget)->get_ID());
//
//	pThis->AssignMissionTarget(pTarget);
//
//	return 0x6ED16C;
//}
//#pragma optimize("", on )

#ifndef disabled_
// ASMJIT_PATCH(0x6F9C80, TechnoClass_GreatestThread_DeadTechno, 0x9)
// {

// 	GET(TechnoClass*, pThis, ESI);

// 	auto pTechno = TechnoClass::Array->Items[R->EBX<int>()];

// 	if (!pTechno->IsAlive)
// 	{
// 		//Debug::LogInfo("TechnoClass::GreatestThread Found DeadTechno[{} - {}] on TechnoArray!", (void*)pTechno, pTechno->get_ID());
// 		return  0x6F9D93; // next
// 	}

// 	R->ECX(pThis->Owner);
// 	R->EDI(pTechno);
// 	return 0x6F9C89;//contunye
// }

// ASMJIT_PATCH(0x6F91EC, TechnoClass_GreatestThreat_DeadTechnoInsideTracker, 0x6)
// {
// 	GET(TechnoClass*, pTrackerTechno, EBP);

// 	if (!pTrackerTechno->IsAlive)
// 	{
// 		//Debug::LogInfo("Found DeadTechno[{} - {}] on AircraftTracker!", (void*)pTrackerTechno, pTrackerTechno->get_ID());
// 		return 0x6F9377; // next
// 	}

// 	return 0x0;//contunye
// }

WeaponTypeClass* GetWeaponType(TechnoClass* pThis, int which)
{
	WeaponTypeClass* pBuffer = nullptr;

	if (which == -1)
	{
		auto const pType = GET_TECHNOTYPE(pThis);

		if (pType->TurretCount > 0 || pType->WeaponCount > 2)
		{
			if (auto const pCurWeapon = pThis->GetWeapon(pThis->CurrentGattlingStage))
			{
				pBuffer = pCurWeapon->WeaponType;
			}
		}
		else
		{
			if (auto const pPriStruct = pThis->GetWeapon(0))
			{
				pBuffer = pPriStruct->WeaponType;
			}

			if (auto const pSecStruct = pThis->GetWeapon(1))
			{
				pBuffer = pSecStruct->WeaponType;
			}
		}
	}
	else
	{
		if (auto const pSelected = pThis->GetWeapon(which))
		{
			pBuffer = pSelected->WeaponType;
		}
	}

	return  pBuffer;
}

// ASMJIT_PATCH(0x6F9039, TechnoClass_GreatestThreat_GuardRange, 0x9)
// {
// 	GET(TechnoClass*, pTechno, ESI);
// 	auto const pTypeGuardRange = pTechno->GetTechnoType()->GuardRange;
// 	auto nGuarRange = pTypeGuardRange == -1 ? 512 : pTypeGuardRange;

// 	if (auto pPri = GetWeaponType(pTechno, 0))
// 	{
// 		if (pPri->Range > nGuarRange)
// 			nGuarRange = pPri->Range;
// 	}

// 	if (auto pSec = GetWeaponType(pTechno, 1))
// 	{
// 		if (pSec->Range > nGuarRange)
// 			nGuarRange = pSec->Range;
// 	}

// 	R->EDI(nGuarRange);
// 	return 0x6F903E;
// }
#endif

#ifdef __old

ASMJIT_PATCH(0x5F6500, AbstractClass_Distance2DSquared_1, 8)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pThat, 0x4);

	int nResult = 0;
	if (pThat)
	{
		auto nThisCoord = pThis->GetCoords();
		auto nThatCoord = pThat->GetCoords();
		nResult = //(int)nThisCoord.DistanceFromXY(nThatCoord)
			cell_Distance_Squared(nThisCoord, nThatCoord);
		;
	}

	R->EAX(nResult);
	return 0x5F655D;
}

ASMJIT_PATCH(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);
	R->EAX(
		//(int)nThisCoord.DistanceFromXY(*pThatCoord)
		cell_Distance_Squared(nThisCoord, *pThatCoord)
	);
	return 0x5F659B;
}

#else


#endif
#pragma endregion

#ifdef DEBUG_STUPID_HUMAN_CHECKS

ASMJIT_PATCH(0x50B730, HouseClass_IsControlledByHuman_LogCaller, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	if (!pThis)
		Debug::LogInfo(__FUNCTION__"Caller [{}]", (uintptr_t)R->Stack<DWORD>(0x0));

	return 0x0;
}

ASMJIT_PATCH(0x50B6F0, HouseClass_ControlledByCurrentPlayer_LogCaller, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	if (!pThis)
		Debug::LogInfo(__FUNCTION__"Caller [{}]", (uintptr_t)R->Stack<DWORD>(0x0));

	return 0x0;
}
#endif

#ifdef IONSHITS

#include <RectangleStruct.h>

struct IonBlastData
{
	int PixX;
	int PixY;
};

ASMJIT_PATCH(0x53CB91, IonBlastClass_DTOR, 6)
{
	GET(IonBlastClass*, IB, ECX);
	WarheadTypeExtData::IonBlastExt.erase(IB);
	return 0;
}

class NOVTABLE FakeIonBlastClass : public IonBlastClass
{
public:

	//static bool IonBlastClass_inited;
	//static Surface* IonBlastClass_Surfaces[80];
	//static uint16_t ionblast_A9FAE8[289];
	//static size_t LUT_SIZE;
	//static int IonBlastPitch;
	static COMPILETIMEEVAL reference<bool, 0xAA014C> IonBlastClass_inited {};
	static COMPILETIMEEVAL reference<Surface*, 0xA9FFC8, 80u> IonBlastClass_Surfaces {};
	static COMPILETIMEEVAL reference<int, 0xA9FAE8, 289u> ionblast_A9FAE8 {};
	static COMPILETIMEEVAL reference<int, 0xAA0150> IonBlastPitch {};

	static COMPILETIMEEVAL IonBlastData IonBlastData_53D8E0(int number)
	{
		int index = number - 1;
		int spiralLayer = 1;

		// Find the spiral layer
		if (index >= 8)
		{
			for (int step = 8; step <= index; step += 8)
			{
				index -= step;
				++spiralLayer;
			}
		}

		if (index >= 2 * spiralLayer + 1)
		{
			if (index >= 4 * spiralLayer + 1)
			{
				if (index >= 6 * spiralLayer + 1)
				{
					// Right side
					return { index - 7 * spiralLayer  ,-spiralLayer };
				}
				else
				{
					// Bottom side
					return { -spiralLayer  ,5 * spiralLayer - index };
				}
			}
			else
			{
				// Left side
				return { 3 * spiralLayer - index  ,spiralLayer };
			}
		}

		// Top side
		return { spiralLayer  ,index - spiralLayer };
	}

	static COMPILETIMEEVAL int IonBlastData_Index(int x, int y)
	{
		if (x == 0 && y == 0)
		{
			return 0;
		}

		int absX = Math::abs(x);
		int absY = Math::abs(y);
		int layer = std::max(absX, absY);  // Spiral layer based on distance from center

		int index = 1;
		for (int i = 1; i < layer; ++i)
		{
			index += 8 * i;
		}

		if (x == layer)
		{
			return index + y + layer;
		}
		if (y == layer)
		{
			return index + 3 * layer - x;
		}
		if (x == -layer)
		{
			return index + 5 * layer - y;
		}
		// y == -layer
		return index + x + 7 * layer;
	}

	static void __fastcall InitOneTime()
	{
		if (IonBlastClass_inited)
			return;

		constexpr int SurfaceCount = 80;
		constexpr int Width = 512;
		constexpr int Height = 256;
		constexpr double RadiusStep = 7.1125;

		double currentMaxRadius = 0.0;
		double currentMinRadius = -57.0;
		int step = 0;

		for (int i = 0; i < SurfaceCount; ++i)
		{
			IonBlastClass_Surfaces[i] = GameCreate<BSurface>(Width, Height, 1, nullptr);
			IonBlastClass_Surfaces[i]->Fill(0xFFFFFFFF);

			auto* lockPtr = IonBlastClass_Surfaces[i]->Lock(0, 0);
			auto* pixels = reinterpret_cast<uint8_t*>(lockPtr) + 0x8080;

			for (int y = 127; y >= 0; --y)
			{
				for (int x = 255; x >= 0; --x)
				{
					int dx = x;
					int dy = y;
					(double dist = Math::sqrt(dx * dx + 4 * dy * dy);

					if (dist >= currentMinRadius && dist <= currentMaxRadius)
					{
						double wave = (dist - step * RadiusStep + 38.0) * 0.11;
						double val = (Math::sin(wave) * 3.5 + 3.0) / (dist / 51.0 + 1.0) + 0.5;

						// originally IonBlastData_53D960 was used, but this call would always return 0
						// since a1 == 0 && a2 == 0 → adjust if you have coordinates instead
						uint8_t pixelVal = static_cast<uint8_t>(val);

						// 4-way symmetry
						pixels[256 * y + x] = pixelVal;
						pixels[256 * y + (255 - x)] = pixelVal;
						pixels[256 * (255 - y) + x] = pixelVal;
						pixels[256 * (255 - y) + (255 - x)] = pixelVal;
					}
				}
			}

			if ((256.0 - RadiusStep) > currentMaxRadius)
				currentMaxRadius += RadiusStep;

			currentMinRadius += RadiusStep * 1.2;
			if (currentMinRadius > currentMaxRadius)
				currentMinRadius = currentMaxRadius;

			++step;
		}

		IonBlastClass_inited = true;
	}

	static void __fastcall DestroySurfaces()
	{
		constexpr int SurfaceCount = 80;
		for (int i = 0; i < SurfaceCount; ++i)
		{
			GameAllocator<BSurface> alloc {};
			std::allocator_traits<GameAllocator<BSurface>>::destroy(alloc, IonBlastClass_Surfaces[i]);
			IonBlastClass_Surfaces[i] = nullptr;
		}
	}

	static void _DrawAll()
	{
		if (DSurface::Temp->Get_Pitch() != IonBlastPitch)
		{
			IonBlastPitch = DSurface::Temp->Get_Pitch();
			ionblast_A9FAE8[0] = 0;

			for (int i = 1; i < ionblast_A9FAE8.size(); ++i)
			{
				IonBlastData data = IonBlastData_53D8E0(i);
				ionblast_A9FAE8[i] = data.PixX + IonBlastPitch * data.PixY;
			}
		}

		for (int i = IonBlastClass::Array->Count - 1; i >= 0; --i)
		{
			static_cast<FakeIonBlastClass*>(IonBlastClass::Array->Items[i])->_Draw();
		}
	}

	void _Draw()
	{
		if (!RulesExtData::DetailsCurrentlyEnabled())
			return;

		auto [screenPos, IsIn] = TacticalClass::Instance->GetCoordsToClientSituation(this->Location);

		if (!IsIn)
			return;

		DSurface* targetSurface = DSurface::Temp();
		DSurface* sourceSurface = static_cast<DSurface*>(IonBlastClass_Surfaces[this->Lifetime]);

		RectangleStruct viewportRect {
			.X = DSurface::ViewBounds->X,
			.Y = DSurface::ViewBounds->Y,
			.Width = DSurface::ViewBounds->Width,
			.Height = DSurface::ViewBounds->Height - 7
		};

		RectangleStruct destRect {
			.X = screenPos.X - 256,
			.Y = screenPos.Y - 128,
			.Width = 512,
			.Height = 256
		};

		RectangleStruct srcRect {
			.X = 0,
			.Y = 0,
			.Width = 512,
			.Height = 256
		};

		RectangleStruct srcSubRect {
			.X = 0,
			.Y = 0,
			.Width = 512,
			.Height = 256
		};

		bool regionClipped = false;
		int32_t destBufferOffset = 0;
		int32_t srcBufferOffset = 0;

		if (!Blit_helper_lockregion(
			targetSurface,
			&viewportRect,
			&destRect,
			sourceSurface,
			&srcRect,
			&srcSubRect,
			&regionClipped,
			(int16_t*)(&destBufferOffset),
			(int16_t*)(&srcBufferOffset)))
		{
			return;
		}

		uint16_t* destBuffer = reinterpret_cast<uint16_t*>(destBufferOffset);
		int32_t* srcBuffer = reinterpret_cast<int32_t*>(srcBufferOffset);
		int8_t* srcBuffer_8 = reinterpret_cast<int8_t*>(srcBufferOffset);

		const int pitch = targetSurface->Get_Pitch();
		const int surfaceWidth = targetSurface->Get_Width();
		const int zBufferWidth = ZBuffer::Instance->Width;

		const int zCoord = this->Location.Z;
		const int16_t zRef = static_cast<int16_t>(ZBuffer::Instance->MaxValue - Game::AdjustHeight(zCoord));
		int16_t zThreshold = zRef - static_cast<int16_t>(destRect.Y) - 3;

		int16_t* zBufferRow = (int16_t*)ZBuffer::Instance->GetBuffer(0, destRect.Y);

		// Safety bounds check before doing optimized access
		uintptr_t zBufferCheck = reinterpret_cast<uintptr_t>(&zBufferRow[surfaceWidth + (srcSubRect.Height + 1) * zBufferWidth]);
		if (zBufferCheck >= reinterpret_cast<uintptr_t>(ZBuffer::Instance->BufferTail))
		{
			// Fallback path for conservative access
			for (int row = 0; row < srcSubRect.Height; ++row)
			{
				for (int col = 0; col < srcSubRect.Width; ++col)
				{
					uint8_t pixel = *srcBuffer_8++;
					if (pixel > 0)
					{
						uint16_t* zPtr = (uint16_t*)ZBuffer::Instance->GetBuffer(destRect.X + col, row + destRect.Y);
						if (*zPtr > zThreshold && pixel < ionblast_A9FAE8.size())
						{
							destBuffer[col] = destBuffer[ionblast_A9FAE8[pixel]];
						}
					}
				}

				destBuffer = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destBuffer) + pitch);
				srcBuffer += 512 - srcSubRect.Width;
				srcBuffer_8 = reinterpret_cast<int8_t*>(srcBuffer);
				--zThreshold;
			}
		}
		else
		{
			// Fast path: linear access
			uint16_t* zPtr = reinterpret_cast<uint16_t*>(&zBufferRow[destRect.X]);

			for (int row = 0; row < srcSubRect.Height; ++row)
			{
				for (int col = 0; col < srcSubRect.Width; ++col)
				{
					uint8_t pixel = *srcBuffer_8++;
					if (pixel > 0 && zPtr[col] > zThreshold && pixel < ionblast_A9FAE8.size())
					{
						destBuffer[col] = destBuffer[ionblast_A9FAE8[pixel]];
					}
				}

				destBuffer = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destBuffer) + pitch);
				zPtr += zBufferWidth;
				srcBuffer += 512 - srcSubRect.Width;
				srcBuffer_8 = reinterpret_cast<int8_t*>(srcBuffer);
				--zThreshold;
			}
		}

		targetSurface->Unlock();
		sourceSurface->Unlock();
	}

	void _AI()
	{

		const auto pData = WarheadTypeExtData::IonBlastExt.get_or_default(this);
		const int Ripple_Radius = pData ? MinImpl((int)ionblast_A9FAE8.Size, pData->Ripple_Radius + 1) : ionblast_A9FAE8.Size;

		if (this->Lifetime >= Ripple_Radius)
		{
			GameDelete<true, false>(this);
			return;
		}

		const auto screenPos = TacticalClass::Instance->CoordsToClient(this->Location);

		if (!this->DisableIonBeam && this->Lifetime == 0)
		{
			CoordStruct spawnCoord = this->Location;
			spawnCoord.Z += 5;
			const auto Rules = RulesClass::Instance();

			auto* mapCell = MapClass::Instance->GetCellAt(this->Location);
			const bool isWater = mapCell->LandType == LandType::Water;

			if (const auto animId = isWater ? Rules->SplashList[Rules->SplashList.Count - 1] :
				pData ? pData->Ion_Blast.Get(Rules->IonBlast) : Rules->IonBlast)
			{
				GameCreate<AnimClass>(animId, spawnCoord);
			}

			if (const auto pBeam = pData ? pData->Ion_Beam.Get(Rules->IonBeam) : Rules->IonBeam)
			{
				GameCreate<AnimClass>(pBeam, spawnCoord);
			}

			if (const auto pWH = pData ? pData->Ion_WH.Get(Rules->IonCannonWarhead) : Rules->IonCannonWarhead)
			{

				const int nDamage = pData ? pData->Ion_Damage.Get(Rules->IonCannonDamage) : Rules->IonCannonDamage;

				if (mapCell->ContainsBridge())
				{
					CoordStruct target = this->Location;
					target.Z += CellClass::BridgeHeight;
					DamageArea::Apply(&target, nDamage, nullptr, pWH, true, nullptr);
				}

				DamageArea::Apply(&this->Location, nDamage, nullptr, pWH, true, nullptr);
				MapClass::FlashbangWarheadAt(nDamage, pWH, this->Location, false, SpotlightFlags::None);
			}
		}

		if (!pData || pData->Ion_Rocking)
		{
			int16_t centerX = static_cast<int16_t>(this->Location.X / 256);
			int16_t centerY = static_cast<int16_t>(this->Location.Y / 256);

			for (int16_t dy = -3; dy <= 3; ++dy)
			{
				for (int16_t dx = -3; dx <= 3; ++dx)
				{
					CellStruct cell { static_cast<int16_t>(centerX + dx), static_cast<int16_t>(centerY + dy) };
					auto* mapCell = MapClass::Instance->GetCellAt(cell);
					FootClass* unit = flag_cast_to<FootClass*>(mapCell->FirstObject);

					while (unit)
					{
						if (unit->WhatAmI() == InfantryClass::AbsID || unit->WhatAmI() == UnitClass::AbsID)
						{

							CoordStruct unitCoord = unit->Location;
							Point2D unitScreen = TacticalClass::Instance->CoordsToClient(unitCoord);

							int dxPix = unitScreen.X - screenPos.X;
							int dyPix = unitScreen.Y - screenPos.Y;
							int dist = static_cast<int>(Math::sqrt(dxPix * dxPix + dyPix * dyPix)) + 8;

							if (dist < 256)
							{

								Surface* surf = IonBlastClass_Surfaces[this->Lifetime];
								char* locked = static_cast<char*>(surf->Lock(dist + 0x100, 128));
								if (*locked > 0)
								{
									unit->SetSpeedPercentage(0.0f);
									IonBlastData data = IonBlastData_53D8E0(*locked);
									unit->height_subtract_6B4 = 2 * data.PixY;
								}

								auto vox = unit->GetTechnoType()->MainVoxel.VXL;

								if (vox && !vox->LoadFailed && *locked >= 0)
								{
									float deltax = static_cast<float>(this->Location.X - unit->Location.X);
									float deltay = static_cast<float>(this->Location.Y - unit->Location.Y);
									float deltaz = static_cast<float>(this->Location.Z - unit->Location.Z);
									const float len = Math::sqrt(deltax * deltax + deltay * deltay + deltaz * deltaz);

									if (Math::abs(len) > 0.00002f)
									{
										deltax /= len;
										deltay /= len;
										deltaz /= len;

										const auto& facing_ = unit->PrimaryFacing;
										const auto facing_Current = facing_.Current();

										const float facingAngle = (facing_Current.Raw - Math::BINARY_ANGLE_MASK) * -0.0000958767f;
										const float sinA = Math::sin((double)facingAngle);
										const float cosA = Math::cos((double)facingAngle);

										const float ux = deltax * cosA + deltay * sinA;
										const float uz = deltax * sinA - deltay * cosA;
										const float uy = deltaz;

										float proj = Math::sqrtux * ux + uz * uz + uy * uy);
					const float align = cosA * ux - sinA * proj;

					if (Math::abs(align - deltax) > 0.0002f || Math::abs(cosA * proj + sinA * ux - deltay) > 0.0002f)
					{
						proj = -proj;
					}

					const float blastDist = len + 51.0f;
					const float blastOffset = (Math::sin(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f) * 3.5f + 3.0f) * 51.0f;
					const float blastFactor = Math::cos(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f);
					const float curve = (blastFactor * 0.11f * 51.0f * 3.5f * blastDist - blastOffset) / (blastDist * blastDist);

					unit->AngleRotatedSideways = proj * curve * Math::GAME_TWOPIf;
					unit->AngleRotatedForwards = -ux * curve * Math::GAME_TWOPIf;
									}
								}
							}
						}
					}
				}
			}
		}

		++this->Lifetime;
	}

	// Helper structures
	struct Vector3D
	{
		float X, Y, Z;
	};

	//DEFINE_FUNCTION_JUMP(CALL, 0x531758, FakeIonBlastClass::InitOneTime)
	//DEFINE_FUNCTION_JUMP(CALL, 0x6BE3CE, FakeIonBlastClass::DestroySurfaces)
	DEFINE_FUNCTION_JUMP(CALL, 0x53D326, FakeIonBlastClass::_AI)

		//bool FakeIonBlastClass::IonBlastClass_inited {};
		//Surface* FakeIonBlastClass::IonBlastClass_Surfaces[80] {};
		//uint16_t FakeIonBlastClass::ionblast_A9FAE8[289] {};
		//size_t FakeIonBlastClass::LUT_SIZE { std::size(FakeIonBlastClass::ionblast_A9FAE8) };
		//int FakeIonBlastClass::IonBlastPitch {};
#endif

	

		ASMJIT_PATCH(0x7BB350, XSurface_Func_check, 0x6)
	{
		GET(XSurface*, pThis, ECX);
		GET_STACK(uintptr_t, caller, 0x0);

		if (!pThis || VTable::Get(pThis) != XSurface::vtable)
		{
			Debug::LogInfo("XSurface Invalid caller [0x{0:x}]!!", caller);
		}

		return 0x0;
	} ASMJIT_PATCH_AGAIN(0x7BBAF0, XSurface_Func_check, 0x5)

		ASMJIT_PATCH(0x6D471A, TechnoClass_Render_dead, 0x6)
	{
		GET(TechnoClass*, pTech, ESI);

		if (!pTech->IsAlive)
			return 0x6D48FA;
		auto vtable = VTable::Get(pTech);

		if (vtable != AircraftClass::vtable
			&& vtable != BuildingClass::vtable
			&& vtable != InfantryClass::vtable
			&& vtable != UnitClass::vtable)
			return 0x6D48FA;

		return 0x0;
	}

	ASMJIT_PATCH(0x438D72, BombListClass_DetectorMissingHouse, 0x7)
	{
		GET(HouseClass*, pDetectorOwner, EAX);
		GET(TechnoClass*, pDetector, ESI);

		if (!pDetectorOwner)
		{
			Debug::FatalErrorAndExit("BombListClass Detector[%s - %s] Missing Ownership !\n", pDetector->GetThisClassName(), pDetector->get_ID());
			//return 0x438E11;
		}

		R->AL(pDetectorOwner->ControlledByCurrentPlayer());
		return 0x438D79;
	}





	//ASMJIT_PATCH(0x7399EE, UnitClass_TryToDeploy_BrokenEBP, 0x5)
	//{
	   // GET(UnitClass*, pThis, EBP);

	   // if (pThis->AttachedTag)
	   // {
	   //	 R->EAX(pThis->AttachedTag);
	   //	 return 0x7399F5;
	   // }

	   // return 0x739A0E;
	//}
#include <Utilities/Swizzle.h>

	DWORD LastKnown;
	AbstractClass* pAbs;

	ASMJIT_PATCH(0x4103D0, AbstractClass_Load_LogValue, 0x5)
	{
		GET(AbstractClass*, pThis, ESI);
		//GET_STACK(IStream*, pStream, 0x0);

		//immedietely update the extension pointer value and the extension AttachedToObject itself !
		ExtensionSwizzleManager::SwizzleExtensionPointer(reinterpret_cast<void**>(&pThis->unknown_18), pThis);
		LastKnown = pThis->unknown_18;
		pAbs = pThis;

		return 0x0;
	}

	//more specific
	//ASMJIT_PATCH(0x41096D, AbstractTypeClass_NoInt_cleaupPtr,0x6)
	//{
	   //  GET(AbstractClass*, pThis, EAX);

	   //  if (Phobos::Otamaa::DoingLoadGame) {
	   //	  if (pAbs != pThis)  //avoid missmatching
	   //		  LastKnown = 0;
	   //  }

	   //  pThis->unknown_18 = std::exchange(LastKnown, 0u);
	   //  return 0x0;
	//}

	ASMJIT_PATCH(0x410182, AbstractClass_cleaupPtr_B, 0x6)
	{
		GET(AbstractClass*, pThis, EAX);

		if (Phobos::Otamaa::DoingLoadGame)
		{
			if (pAbs != pThis) //avoid missmatching
				LastKnown = 0;
		}

		pThis->unknown_18 = std::exchange(LastKnown, 0u);
		pThis->RefCount = 0l;
		return 0x410188;
	}

	ASMJIT_PATCH(0x4101E4, AbstractClass_cleaupPtr, 0x7)
	{
		GET(AbstractClass*, pThis, EAX);

		if (Phobos::Otamaa::DoingLoadGame)
		{

			if (pAbs != pThis) //avoid missmatching
				LastKnown = 0;
		}

		pThis->unknown_18 = std::exchange(LastKnown, 0u);
		return 0x0;
	}

	//ASMJIT_PATCH(0x521960, InfantryClass_Load_test, 0x5)
	// {
	   // GET(InfantryClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x521A11, InfantryClass_NoInit_test, 0x6)
	//{
	   // GET(AbstractClass*, pThis, EAX);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5219E8, InfantryClass_Load_test, 0x5)
	//{
	   // GET(InfantryClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5F3B5D, ObjectClass_Load_checkExt, 0x5)
	//{
	   // GET(ObjectClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x65A7F7, RadioClass_Load_checkExt, 0x5)
	//{
	   // GET(RadioClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x6F44E9, TechnoClass_Load_checkExt, 0x5)
	//{
	   // GET(TechnoClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x4D3568, FootClass_Load_checkExt, 0x6)
	//{
	   // GET(FootClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x744527, UnitClass_Load_checkExt, 0x6)
	//{
	   // GET(UnitClass*, pThis, ESI);
	   // return 0x0;
	//}
	//ASMJIT_PATCH(0x410361, AbstractClass_Save_LogValue, 0x5)
	//{
	   // GET(AbstractClass*, pThis, ESI);
	   // Debug::Log("Saving Ext of %x", pThis->unknown_18);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x6D4912, TechnoClass_Render_deadRemoval, 0x6)
	//{
	   // TechnoClass::Array->remove_if([](TechnoClass* ptr) {
	   //	 auto vtable = VTable::Get(ptr);
	   //	 if (vtable != AircraftClass::vtable
	   //		 && vtable != BuildingClass::vtable
	   //		 && vtable != InfantryClass::vtable
	   //		 && vtable != UnitClass::vtable)
	   //		 return true;

	   //	 return false;
	   //});

	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5F4870, ObjectClass_func_BrokenObj, 0x5)
	//{
	   // GET(ObjectClass*, pObj, ECX);
	   // GET_STACK(DWORD, caller, 0x0);

	   // if (!pObj->IsAlive)
	   //	 Debug::Log("Dead obj %x caller %x\n", pObj , caller);
	   // //auto vtable = VTable::Get(pObj);
	   // //BulletClass
	   //	// IsometricTileClass
	   //	// OverlayClass
	   //	// ParticleClass
	   //	// ParticleSystemClass
	   //	// SmudgeClass
	   //	// TerrainClass
	   //	// VeinholeMonsterClass
	   //	// VoxelAnimClass
	   //	// WaveClass
	   // //if (&& vtable != BuildingLightClass::vtable  && vtable != AnimClass::vtable
	   //	// && vtable != AircraftClass::vtable
	   //	// && vtable != BuildingClass::vtable
	   //	// && vtable != InfantryClass::vtable
	   //	// && vtable != UnitClass::vtable)

	   // return 0x0;
	//}

	
#ifdef CHECK_PTR_VALID


	ASMJIT_PATCH(0x4F9A90, HouseClass_IsAlly_ObjectClass, 0x7)
	{
		GET_STACK(ObjectClass*, pTarget, 0x4);
		GET(HouseClass*, pThis, ECX);
		GET_STACK(DWORD, caller, 0x0);

		bool result = false;

		if (pTarget)
		{

			if (flag_cast_to<TechnoClass*>(pTarget))
			{
				if ((VTable::Get(pTarget) != AircraftClass::vtable &&
					VTable::Get(pTarget) != BuildingClass::vtable &&
					VTable::Get(pTarget) != UnitClass::vtable &&
					VTable::Get(pTarget) != InfantryClass::vtable))
				{
					Debug::FatalError("Missing valid vtable %x , caller %x", pTarget, caller);
				}
			}

			auto pTargetOwner = pTarget->GetOwningHouse();
			result = pThis->IsAlliedWith(pTargetOwner);
		}

		R->AL(result);
		return 0x4F9ADE;
	}

	ASMJIT_PATCH(0x6F8A0F, TechnoClass_EvalCell_deadTechno, 0x8)
	{
		GET(ObjectClass*, pCellObj, EDI);
		return !pCellObj || !pCellObj->IsAlive ? 0x6F8B4D : 0x6F8A17;
	}
	//ASMJIT_PATCH(0x4F9A90, HouseClass_IsAlliedWith, 0x7)
	//{
	//	GET(HouseClass*, pThis, ECX);
	//	GET_STACK(DWORD, called, 0x0);
	//	GET_STACK(AbstractClass*, pAbs, 0x4);
	//
	//	if (!pThis || VTable::Get(pThis) != HouseClass::vtable) {
	//		Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` pointer !", R->Origin(), called);
	//	}
	//	else if (auto pTechno = flag_cast_to<TechnoClass*>(pAbs)){
	//			if(VTable::Get(pTechno) != AircraftClass::vtable &&
	//				VTable::Get(pTechno) != BuildingClass::vtable &&
	//				VTable::Get(pTechno) != UnitClass::vtable &&
	//				VTable::Get(pTechno) != InfantryClass::vtable
	//			) {
	//			Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` abstract pointer !", R->Origin(), called);
	//		}
	//	}
	//
	//	return 0;
	//}
	//ASMJIT_PATCH_AGAIN(0x4F9AF0, HouseClass_IsAlliedWith, 0x7)
	//ASMJIT_PATCH_AGAIN(0x4F9A10, HouseClass_IsAlliedWith, 0x6)
	//ASMJIT_PATCH_AGAIN(0x4F9A50, HouseClass_IsAlliedWith, 0x6)

#endif

//ASMJIT_PATCH(0x7564B0, VoxClass_GetData, 7) {
//	GET(VoxLib*, pVox, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(int, header, 0x4);
//	GET_STACK(int, layer, 0x8);
//
//	if (!pVox->HeaderData || !pVox->TailerData)
//		Debug::FatalError("VoxelLibraryClass::Get_Voxel_Layer_Info input is broken ! caller 0x%x", caller);
//
//	auto pData = &pVox->TailerData[layer + pVox->HeaderData[header].limb_number];
//
//	R->EAX(pData);
//	return 0x7564CF;
//}

//loading save game will crash after this function
//not sure atm, weird shit
//ASMJIT_PATCH(0x5F7577, ObjectTypeClass_DTOR_Voxel, 0x6) {
//	GET(AbstractTypeClass*, pThis, ESI);
//
//	Debug::Log("Destroying Voxel for %s ! \n", pThis->ID);
//	return 0x0;
//}

//ASMJIT_PATCH(0x6f4974, TechnoClass_UpdateDiscovered_ByPlayer_Announce, 0x6) {
//	//play eva , once ?
//}

//ASMJIT_PATCH(0x5F6360, ObjectClass_Distance, 0x5)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(ObjectClass*, pTarget, 0x4);
//
//	if (!pTarget || !pTarget->IsAlive && pTarget->AbstractFlags != AbstractFlags::None) {
//		Debug::Log("Caller %x\n", caller);
//		R->EAX(0);
//		return 0x5F6376;
//	}
//
//	return 0x0;
//}


	
