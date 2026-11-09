#include "Body.h"

#include <Utilities/Cast.h>
#include <Utilities/Debug.h>
#include <AITriggerTypeClass.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>

#include <New/Type/GenericPrerequisite.h>

#include <Locomotor/HoverLocomotionClass.h>
//#include <ExtraHeaders/StackVector.h>

#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <VocClass.h>
#include <VoxClass.h>
#include <TeamClass.h>

/*
*	Scripts is a part of `TeamClass` that executed sequentally form `ScriptTypeClass`
*	Each script contains function that behave as it programmed
*/

ScriptActionNode NOINLINE ScriptExtData::GetSpecificAction(ScriptClass* pScript, int nIdx)
{
	if ((size_t)nIdx < (size_t)pScript->Type->ActionsCount)
		return pScript->Type->ScriptActions[nIdx];
	//	nIdx = pScript->Type->ActionsCount;
	//
	//auto const nIdxR = nIdx - ScriptTypeClass::MaxActions;
	//auto const pTypeExt = ScriptTypeExt::ExtMap.Find(pScript->Type);
	//
	//if (!pTypeExt->PhobosNode.empty() && nIdxR < (int)pTypeExt->PhobosNode.size()) {
	//	return pTypeExt->PhobosNode[nIdxR];
	//}
	//COMPILETIMEEVAL auto const nMax = ScriptTypeClass::MaxActions - 1;
	//return pScript->Type->ScriptActions[nMax];

	return { -1 , 0 };
}

// =============================
// container
ScriptExtContainer ScriptExtContainer::Instance;

#define stringify( name ) #name

static NOINLINE const char* ToStrings(PhobosScripts from)
{
	switch (from)
	{
	case PhobosScripts::TimedAreaGuard:
		return "TimedAreaGuard";
	case PhobosScripts::LoadIntoTransports:
		return "LoadIntoTransports";
	case PhobosScripts::WaitUntilFullAmmo:
		return "WaitUntilFullAmmo";
	case PhobosScripts::RepeatAttackCloserThreat:
		return "RepeatAttackCloserThreat";
	case PhobosScripts::RepeatAttackFartherThreat:
		return "RepeatAttackFartherThreat";
	case PhobosScripts::RepeatAttackCloser:
		return "RepeatAttackCloser";
	case PhobosScripts::RepeatAttackFarther:
		return "RepeatAttackFarther";
	case PhobosScripts::SingleAttackCloserThreat:
		return "SingleAttackCloserThreat";
	case PhobosScripts::SingleAttackFartherThreat:
		return "SingleAttackFartherThreat";
	case PhobosScripts::SingleAttackCloser:
		return "SingleAttackCloser";
	case PhobosScripts::SingleAttackFarther:
		return "SingleAttackFarther";
	case PhobosScripts::DecreaseCurrentAITriggerWeight:
		return "DecreaseCurrentAITriggerWeight";
	case PhobosScripts::IncreaseCurrentAITriggerWeight:
		return "IncreaseCurrentAITriggerWeight";
	case PhobosScripts::RepeatAttackTypeCloserThreat:
		return "RepeatAttackTypeCloserThreat";
	case PhobosScripts::RepeatAttackTypeFartherThreat:
		return "RepeatAttackTypeFartherThreat";
	case PhobosScripts::RepeatAttackTypeCloser:
		return "RepeatAttackTypeCloser";
	case PhobosScripts::RepeatAttackTypeFarther:
		return "RepeatAttackTypeFarther";
	case PhobosScripts::SingleAttackTypeCloserThreat:
		return "SingleAttackTypeCloserThreat";
	case PhobosScripts::SingleAttackTypeFartherThreat:
		return "SingleAttackTypeFartherThreat";
	case PhobosScripts::SingleAttackTypeCloser:
		return "SingleAttackTypeCloser";
	case PhobosScripts::SingleAttackTypeFarther:
		return "SingleAttackTypeFarther";
	case PhobosScripts::WaitIfNoTarget:
		return "WaitIfNoTarget";
	case PhobosScripts::TeamWeightReward:
		return "TeamWeightReward";
	case PhobosScripts::PickRandomScript:
		return "PickRandomScript";
	case PhobosScripts::MoveToEnemyCloser:
		return "MoveToEnemyCloser";
	case PhobosScripts::MoveToEnemyFarther:
		return "MoveToEnemyFarther";
	case PhobosScripts::MoveToFriendlyCloser:
		return "MoveToFriendlyCloser";
	case PhobosScripts::MoveToFriendlyFarther:
		return "MoveToFriendlyFarther";
	case PhobosScripts::MoveToTypeEnemyCloser:
		return "MoveToTypeEnemyCloser";
	case PhobosScripts::MoveToTypeEnemyFarther:
		return "MoveToTypeEnemyFarther";
	case PhobosScripts::MoveToTypeFriendlyCloser:
		return "MoveToTypeFriendlyCloser";
	case PhobosScripts::MoveToTypeFriendlyFarther:
		return "MoveToTypeFriendlyFarther";
	case PhobosScripts::ModifyTargetDistance:
		return "ModifyTargetDistance";
	case PhobosScripts::RandomAttackTypeCloser:
		return "RandomAttackTypeCloser";
	case PhobosScripts::RandomAttackTypeFarther:
		return "RandomAttackTypeFarther";
	case PhobosScripts::RandomMoveToTypeEnemyCloser:
		return "RandomMoveToTypeEnemyCloser";
	case PhobosScripts::RandomMoveToTypeEnemyFarther:
		return "RandomMoveToTypeEnemyFarther";
	case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		return "RandomMoveToTypeFriendlyCloser";
	case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		return "RandomMoveToTypeFriendlyFarther";
	case PhobosScripts::SetMoveMissionEndMode:
		return "SetMoveMissionEndMode";
	case PhobosScripts::UnregisterGreatSuccess:
		return "UnregisterGreatSuccess";
	case PhobosScripts::GatherAroundLeader:
		return "GatherAroundLeader";
	case PhobosScripts::RandomSkipNextAction:
		return "RandomSkipNextAction";
	case PhobosScripts::ChangeTeamGroup:
		return "ChangeTeamGroup";
	case PhobosScripts::DistributedLoading:
		return "DistributedLoading";
	case PhobosScripts::FollowFriendlyByGroup:
		return "FollowFriendlyByGroup";
	case PhobosScripts::RallyUnitWithSameGroup:
		return "RallyUnitWithSameGroup";
	case PhobosScripts::ScatterAttack:
		return "ScatterAttack";
	case PhobosScripts::ChronoTeleportToWaypoint:
		return "ChronoTeleportToWaypoint";
	case PhobosScripts::ScatterAttackByAITargetTypes:
		return "ScatterAttackByAITargetTypes";
	case PhobosScripts::PatrolToEnemyBuildingNearby:
		return "PatrolToEnemyBuildingNearby";
	case PhobosScripts::PatrolToEnemyRally:
		return "PatrolToEnemyRally";
	case PhobosScripts::PatrolToFriendlyBuildingNearby:
		return "PatrolToFriendlyBuildingNearby";
	case PhobosScripts::PatrolToFriendlyRally:
		return "PatrolToFriendlyRally";
	case PhobosScripts::ApplyAttachEffect:
		return "ApplyAttachEffect";
	case PhobosScripts::RemoveAttachEffect:
		return "RemoveAttachEffect";
	case PhobosScripts::RemoveAllAttachEffects:
		return "RemoveAllAttachEffects";
	case PhobosScripts::EnterMissionForDuration:
		return "EnterMissionForDuration";
	case PhobosScripts::JumpToLineByLocalVariable:
		return "JumpToLineByLocalVariable";
	case PhobosScripts::JumpToLineByGlobalVariable:
		return "JumpToLineByGlobalVariable";
	case PhobosScripts::WaitForLocalVariable:
		return "WaitForLocalVariable";
	case PhobosScripts::WaitForGlobalVariable:
		return "WaitForGlobalVariable";
	case PhobosScripts::JumpToLineIfLocalVariableSet:
		return "JumpToLineIfLocalVariableSet";
	case PhobosScripts::JumpToLineIfGlobalVariableSet:
		return "JumpToLineIfGlobalVariableSet";
	case PhobosScripts::JumpToLocalLineIfLocalSet:
		return "JumpToLocalLineIfLocalSet";
	case PhobosScripts::JumpToGlobalLineIfGlobalSet:
		return "JumpToGlobalLineIfGlobalSet";
	case PhobosScripts::JumpToLocalLineIfGlobalSet:
		return "JumpToLocalLineIfGlobalSet";
	case PhobosScripts::JumpToGlobalLineIfLocalSet:
		return "JumpToGlobalLineIfLocalSet";
	case PhobosScripts::StopForceJumpCountdown:
		return "StopForceJumpCountdown";
	case PhobosScripts::NextLineForceJumpCountdown:
		return "NextLineForceJumpCountdown";
	case PhobosScripts::SameLineForceJumpCountdown:
		return "SameLineForceJumpCountdown";
	case PhobosScripts::ForceGlobalOnlyTargetHouseEnemy:
		return "ForceGlobalOnlyTargetHouseEnemy";
	case PhobosScripts::OverrideOnlyTargetHouseEnemy:
		return "OverrideOnlyTargetHouseEnemy";
	case PhobosScripts::SetHouseAngerModifier:
		return "SetHouseAngerModifier";
	case PhobosScripts::ModifyHateHouseIndex:
		return "ModifyHateHouseIndex";
	case PhobosScripts::ModifyHateHousesList:
		return "ModifyHateHousesList";
	case PhobosScripts::ModifyHateHousesList1Random:
		return "ModifyHateHousesList1Random";
	case PhobosScripts::SetTheMostHatedHouseMinorNoRandom:
		return "SetTheMostHatedHouseMinorNoRandom";
	case PhobosScripts::SetTheMostHatedHouseMajorNoRandom:
		return "SetTheMostHatedHouseMajorNoRandom";
	case PhobosScripts::SetTheMostHatedHouseRandom:
		return "SetTheMostHatedHouseRandom";
	case PhobosScripts::ResetAngerAgainstHouses:
		return "ResetAngerAgainstHouses";
	case PhobosScripts::AggroHouse:
		return "AggroHouse";
	case PhobosScripts::SetSideIdxForManagingTriggers:
		return "SetSideIdxForManagingTriggers";
	case PhobosScripts::SetHouseIdxForManagingTriggers:
		return "SetHouseIdxForManagingTriggers";
	case PhobosScripts::ManageAllAITriggers:
		return "ManageAllAITriggers";
	case PhobosScripts::EnableTriggersFromList:
		return "EnableTriggersFromList";
	case PhobosScripts::DisableTriggersFromList:
		return "DisableTriggersFromList";
	case PhobosScripts::DisableTriggersWithObjects:
		return "DisableTriggersWithObjects";
	case PhobosScripts::EnableTriggersWithObjects:
		return "EnableTriggersWithObjects";
	case PhobosScripts::ConditionalJumpResetVariables:
		return "ConditionalJumpResetVariables";
	case PhobosScripts::ConditionalJumpManageResetIfJump:
		return "ConditionalJumpManageResetIfJump";
	case PhobosScripts::AbortActionAfterSuccessKill:
		return "AbortActionAfterSuccessKill";
	case PhobosScripts::ConditionalJumpManageKillsCounter:
		return "ConditionalJumpManageKillsCounter";
	case PhobosScripts::ConditionalJumpSetCounter:
		return "ConditionalJumpSetCounter";
	case PhobosScripts::ConditionalJumpSetComparatorMode:
		return "ConditionalJumpSetComparatorMode";
	case PhobosScripts::ConditionalJumpSetComparatorValue:
		return "ConditionalJumpSetComparatorValue";
	case PhobosScripts::ConditionalJumpSetIndex:
		return "ConditionalJumpSetIndex";
	case PhobosScripts::ConditionalJumpIfFalse:
		return "ConditionalJumpIfFalse";
	case PhobosScripts::ConditionalJumpIfTrue:
		return "ConditionalJumpIfTrue";
	case PhobosScripts::ConditionalJumpKillEvaluation:
		return "ConditionalJumpKillEvaluation";
	case PhobosScripts::ConditionalJumpCheckCount:
		return "ConditionalJumpCheckCount";
	case PhobosScripts::ConditionalJumpCheckAliveHumans:
		return "ConditionalJumpCheckAliveHumans";
	case PhobosScripts::ConditionalJumpCheckObjects:
		return "ConditionalJumpCheckObjects";
	case PhobosScripts::ConditionalJumpCheckHumanIsMostHated:
		return "ConditionalJumpCheckHumanIsMostHated";
	case PhobosScripts::JumpBackToPreviousScript:
		return "JumpBackToPreviousScript";
	case PhobosScripts::RepairDestroyedBridge:
		return "RepairDestroyedBridge";
	case PhobosScripts::ChronoshiftToEnemyBase:
		return "ChronoshiftToEnemyBase";
	case PhobosScripts::LocalVariableSet:
		return "LocalVariableSet";
	case PhobosScripts::LocalVariableAdd:
		return "LocalVariableAdd";
	case PhobosScripts::LocalVariableMinus:
		return "LocalVariableMinus";
	case PhobosScripts::LocalVariableMultiply:
		return "LocalVariableMultiply";
	case PhobosScripts::LocalVariableDivide:
		return "LocalVariableDivide";
	case PhobosScripts::LocalVariableMod:
		return "LocalVariableMod";
	case PhobosScripts::LocalVariableLeftShift:
		return "LocalVariableLeftShift";
	case PhobosScripts::LocalVariableRightShift:
		return "LocalVariableRightShift";
	case PhobosScripts::LocalVariableReverse:
		return "LocalVariableReverse";
	case PhobosScripts::LocalVariableXor:
		return "LocalVariableXor";
	case PhobosScripts::LocalVariableOr:
		return "LocalVariableOr";
	case PhobosScripts::LocalVariableAnd:
		return "LocalVariableAnd";
	case PhobosScripts::GlobalVariableSet:
		return "GlobalVariableSet";
	case PhobosScripts::GlobalVariableAdd:
		return "GlobalVariableAdd";
	case PhobosScripts::GlobalVariableMinus:
		return "GlobalVariableMinus";
	case PhobosScripts::GlobalVariableMultiply:
		return "GlobalVariableMultiply";
	case PhobosScripts::GlobalVariableDivide:
		return "GlobalVariableDivide";
	case PhobosScripts::GlobalVariableMod:
		return "GlobalVariableMod";
	case PhobosScripts::GlobalVariableLeftShift:
		return "GlobalVariableLeftShift";
	case PhobosScripts::GlobalVariableRightShift:
		return "GlobalVariableRightShift";
	case PhobosScripts::GlobalVariableReverse:
		return "GlobalVariableReverse";
	case PhobosScripts::GlobalVariableXor:
		return "GlobalVariableXor";
	case PhobosScripts::GlobalVariableOr:
		return "GlobalVariableOr";
	case PhobosScripts::GlobalVariableAnd:
		return "GlobalVariableAnd";
	case PhobosScripts::LocalVariableSetByLocal:
		return "LocalVariableSetByLocal";
	case PhobosScripts::LocalVariableAddByLocal:
		return "LocalVariableAddByLocal";
	case PhobosScripts::LocalVariableMinusByLocal:
		return "LocalVariableMinusByLocal";
	case PhobosScripts::LocalVariableMultiplyByLocal:
		return "LocalVariableMultiplyByLocal";
	case PhobosScripts::LocalVariableDivideByLocal:
		return "LocalVariableDivideByLocal";
	case PhobosScripts::LocalVariableModByLocal:
		return "LocalVariableModByLocal";
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		return "LocalVariableLeftShiftByLocal";
	case PhobosScripts::LocalVariableRightShiftByLocal:
		return "LocalVariableRightShiftByLocal";
	case PhobosScripts::LocalVariableReverseByLocal:
		return "LocalVariableReverseByLocal";
	case PhobosScripts::LocalVariableXorByLocal:
		return "LocalVariableXorByLocal";
	case PhobosScripts::LocalVariableOrByLocal:
		return "LocalVariableOrByLocal";
	case PhobosScripts::LocalVariableAndByLocal:
		return "LocalVariableAndByLocal";
	case PhobosScripts::GlobalVariableSetByLocal:
		return "GlobalVariableSetByLocal";
	case PhobosScripts::GlobalVariableAddByLocal:
		return "GlobalVariableAddByLocal";
	case PhobosScripts::GlobalVariableMinusByLocal:
		return "GlobalVariableMinusByLocal";
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		return "GlobalVariableMultiplyByLocal";
	case PhobosScripts::GlobalVariableDivideByLocal:
		return "GlobalVariableDivideByLocal";
	case PhobosScripts::GlobalVariableModByLocal:
		return "GlobalVariableModByLocal";
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		return "GlobalVariableLeftShiftByLocal";
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		return "GlobalVariableRightShiftByLocal";
	case PhobosScripts::GlobalVariableReverseByLocal:
		return "GlobalVariableReverseByLocal";
	case PhobosScripts::GlobalVariableXorByLocal:
		return "GlobalVariableXorByLocal";
	case PhobosScripts::GlobalVariableOrByLocal:
		return "GlobalVariableOrByLocal";
	case PhobosScripts::GlobalVariableAndByLocal:
		return "GlobalVariableAndByLocal";
	case PhobosScripts::LocalVariableSetByGlobal:
		return "LocalVariableSetByGlobal";
	case PhobosScripts::LocalVariableAddByGlobal:
		return "LocalVariableAddByGlobal";
	case PhobosScripts::LocalVariableMinusByGlobal:
		return "LocalVariableMinusByGlobal";
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		return "LocalVariableMultiplyByGlobal";
	case PhobosScripts::LocalVariableDivideByGlobal:
		return "LocalVariableDivideByGlobal";
	case PhobosScripts::LocalVariableModByGlobal:
		return "LocalVariableModByGlobal";
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		return "LocalVariableLeftShiftByGlobal";
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		return "LocalVariableRightShiftByGlobal";
	case PhobosScripts::LocalVariableReverseByGlobal:
		return "LocalVariableReverseByGlobal";
	case PhobosScripts::LocalVariableXorByGlobal:
		return "LocalVariableXorByGlobal";
	case PhobosScripts::LocalVariableOrByGlobal:
		return "LocalVariableOrByGlobal";
	case PhobosScripts::LocalVariableAndByGlobal:
		return "LocalVariableAndByGlobal";
	case PhobosScripts::GlobalVariableSetByGlobal:
		return "GlobalVariableSetByGlobal";
	case PhobosScripts::GlobalVariableAddByGlobal:
		return "GlobalVariableAddByGlobal";
	case PhobosScripts::GlobalVariableMinusByGlobal:
		return "GlobalVariableMinusByGlobal";
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		return "GlobalVariableMultiplyByGlobal";
	case PhobosScripts::GlobalVariableDivideByGlobal:
		return "GlobalVariableDivideByGlobal";
	case PhobosScripts::GlobalVariableModByGlobal:
		return "GlobalVariableModByGlobal";
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		return "GlobalVariableLeftShiftByGlobal";
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		return "GlobalVariableRightShiftByGlobal";
	case PhobosScripts::GlobalVariableReverseByGlobal:
		return "GlobalVariableReverseByGlobal";
	case PhobosScripts::GlobalVariableXorByGlobal:
		return "GlobalVariableXorByGlobal";
	case PhobosScripts::GlobalVariableOrByGlobal:
		return "GlobalVariableOrByGlobal";
	case PhobosScripts::GlobalVariableAndByGlobal:
		return "GlobalVariableAndByGlobal";
	case PhobosScripts::SimpleDeployerDeploy:
		return "SimpleDeployerDeploy";
	case PhobosScripts::ChangeToScriptByID:
		return "ChangeToScriptByID";
	case PhobosScripts::ChangeToTeamTypeByID:
		return "ChangeToTeamTypeByID";
	case PhobosScripts::ChangeToHouseByID:
		return "ChangeToHouseByID";
	case PhobosScripts::PlaySpeechByID:
		return "PlaySpeechByID";
	case PhobosScripts::PlaySoundByID:
		return "PlaySoundByID";
	case PhobosScripts::PlayMovieByID:
		return "PlayMovieByID";
	case PhobosScripts::PlayThemeByID:
		return "PlayThemeByID";
	case PhobosScripts::AttackEnemyStructureByID:
		return "AttackEnemyStructureByID";
	case PhobosScripts::MoveToEnemyStructureByID:
		return "MoveToEnemyStructureByID";
	case PhobosScripts::PlayAnimationByID:
		return "PlayAnimationByID";
	case PhobosScripts::ChronoshiftTaskForceToStructureByID:
		return "ChronoshiftTaskForceToStructureByID";
	case PhobosScripts::MoveToFriendlyStructureByID:
		return "MoveToFriendlyStructureByID";
	default:
		return GameStrings::NoneStr();
	}
}

#include "Lua/Wrapper.h"

bool ScriptExtData::Handle(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd)
{

	switch ((AresScripts)pTeamMission->Action)
	{
	case AresScripts::AuxilarryPower:
	{
		HouseExtContainer::Instance.Find(pTeam->OwnerHouse)->AuxPower += pTeamMission->Argument;
		pTeam->OwnerHouse->RecheckPower = true;
		pTeam->StepCompleted = true;
		return true;
	}
	case AresScripts::KillDrivers:
	{
		const auto pToHouse = HouseExtData::FindSpecial();
		FootClass* pCur = nullptr;
		if (auto pFirst = pTeam->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				if (pFirst->Health > 0 && pFirst->IsAlive && pFirst->IsOnMap && !pFirst->InLimbo)
				{
					if (!TechnoExtContainer::Instance.Find(pFirst)->Is_DriverKilled
						&& TechnoExtData::IsDriverKillable(pFirst, 1.0))
					{
						TechnoExtData::ApplyKillDriver(pFirst, nullptr, pToHouse, false, Mission::Harmless);
					}
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		pTeam->StepCompleted = true;
		return true;
	}
	case AresScripts::TakeVehicles:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = pTeam->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				TechnoExtContainer::Instance.Find(pFirst)->TakeVehicleMode = true;

				if (pFirst->GarrisonStructure())
					pTeam->RemoveMember(pFirst, -1, 1);

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		pTeam->StepCompleted = true;
		return true;
	}
	case AresScripts::ConvertType:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = pTeam->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				const auto pTypeExt = GET_TECHNOTYPEEXT(pFirst);
				if (pTypeExt->Convert_Script)
				{
					const auto& pConvertReq = pTypeExt->Convert_Script_Prereq;
					if (pConvertReq.empty() || Prereqs::HouseOwnsAll(pTeam->OwnerHouse, pConvertReq))
					{
						TechnoExtData::ConvertToType(pFirst, pTypeExt->Convert_Script);
					}
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		pTeam->StepCompleted = true;
		return true;
	}
	case AresScripts::SonarReveal:
	{
		const auto nDur = pTeamMission->Argument;
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto& nSonarTime = TechnoExtContainer::Instance.Find(pUnit)->CloakSkipTimer;
			if (nDur > nSonarTime.GetTimeLeft()) {
				nSonarTime.Start(nDur);
			} else if (nDur <= 0) {
				nSonarTime.Stop();
			}
		}

		pTeam->StepCompleted = true;
		return true;
	}
	case AresScripts::DisableWeapons:
	{
		const auto nDur = pTeamMission->Argument;
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto& nTimer = TechnoExtContainer::Instance.Find(pUnit)->DisableWeaponTimer;
			if (nDur > nTimer.GetTimeLeft())
				nTimer.Start(nDur);
			else if (nDur <= 0)
				nTimer.Stop();
		}

		pTeam->StepCompleted = true;
		return true;
	}
	default:
		break;
	}

	return false;
}

bool ScriptExtData::ProcessScriptActions(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd)
{
	auto const& [action, argument] = *pTeamMission;
	auto pTeamExt = TeamExtContainer::Instance.Find(pTeam);
	const auto pNodeIndex = pTeam->CurrentScript->CurrentMission;

	//Debug::LogInfo("[{} - {}] Executing[{} - {}] [{} - {}]",
	//pTeam->Owner->get_ID(),
	//pTeam->Owner,
	//pTeam->get_ID(),
	//pTeam, action ,
	//argument
	//);
	const bool fresh = (pNodeIndex != pTeamExt->LastProcessedMission);
	pTeamExt->LastProcessedMission = pNodeIndex;

	//only find stuffs on the range , reducing the load
	//if ((AresScripts)action >= AresScripts::count)
	{
		//Debug::LogInfo("[{} - {}] Executing[{} - {}] [{} ({}) - {}]",
		//	pTeam->Owner->get_ID(),
		//	pTeam->Owner,
		//	pTeam->get_ID(),
		//	pTeam, action ,
		//	ToStrings((PhobosScripts)action), argument
		//);
		int Action = (int)action;

		switch (PhobosScripts(Action))
		{
		case PhobosScripts::TimedAreaGuard:
		{
			ScriptExtData::ExecuteTimedAreaGuardAction(pTeam); //checked
			break;
		}
		case PhobosScripts::LoadIntoTransports:
		{
			ScriptExtData::LoadIntoTransports(pTeam); //fixed
			break;
		}
		case PhobosScripts::WaitUntilFullAmmo:
		{
			ScriptExtData::WaitUntilFullAmmoAction(pTeam); //
			break;
		}
		case PhobosScripts::DecreaseCurrentAITriggerWeight:
		{
			ScriptExtData::DecreaseCurrentTriggerWeight(pTeam, true, 0); //
			break;
		}
		case PhobosScripts::IncreaseCurrentAITriggerWeight:
		{
			ScriptExtData::IncreaseCurrentTriggerWeight(pTeam, true, 0);
			break;
		}
		case PhobosScripts::WaitIfNoTarget:
		{
			ScriptExtData::WaitIfNoTarget(pTeam, -1);
			break;
		}
		case PhobosScripts::TeamWeightReward:
		{
			ScriptExtData::TeamWeightReward(pTeam, -1);
			break;
		}
		case PhobosScripts::PickRandomScript:
		{
			ScriptExtData::PickRandomScript(pTeam, -1);
			break;
		}
		case PhobosScripts::ModifyTargetDistance:
		{
			// AISafeDistance equivalent for Mission_Move()
			ScriptExtData::SetCloseEnoughDistance(pTeam, -1);
			break;
		}
		case PhobosScripts::SetMoveMissionEndMode:
		{
			// Set the condition for ending the Mission_Move Actions.
			ScriptExtData::SetMoveMissionEndMode(pTeam, -1);
			break;
		}
		case PhobosScripts::UnregisterGreatSuccess:
		{
			// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
			ScriptExtData::UnregisterGreatSuccess(pTeam);
			break;
		}
		case PhobosScripts::GatherAroundLeader:
		{
			ScriptExtData::Mission_Gather_NearTheLeader(pTeam, -1);
			break;
		}
		case PhobosScripts::RandomSkipNextAction:
		{
			ScriptExtData::SkipNextAction(pTeam, -1);
			break;
		}
		case PhobosScripts::ForceGlobalOnlyTargetHouseEnemy:
		{
			ScriptExtData::ForceGlobalOnlyTargetHouseEnemy(pTeam, -1);
			break;
		}

		case PhobosScripts::ChangeTeamGroup:
		{
			//	ScriptExtData::TeamMemberSetGroup(pTeam, argument);
			Debug::LogInfo("Team[{} - {} , Script [{} - {}] Action [{}] - No AttachedFunction", (void*)pTeam, pTeam->get_ID(), (void*)pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), (int)action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::DistributedLoading:
		{
			//ScriptExtData::DistributedLoadOntoTransport(pTeam, argument); //which branch is this again ?
			Debug::LogInfo("Team[{} - {} , Script [{} - {}] Action [{}] - No AttachedFunction", (void*)pTeam, pTeam->get_ID(), (void*)pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), (int)action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::FollowFriendlyByGroup:
		{
			//ScriptExtData::FollowFriendlyByGroup(pTeam, argument); //which branch is this again ?
			Debug::LogInfo("Team[{} - {} , Script [{} - {}] Action [{}] - No AttachedFunction", (void*)pTeam, pTeam->get_ID(), (void*)pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), (int)action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::RallyUnitWithSameGroup:
		{
			//ScriptExtData::RallyUnitInMap(pTeam, argument); //which branch is this again ?
			Debug::LogInfo("Team[{} - {} , Script [{} - {}] Action [{}] - No AttachedFunction", (void*)pTeam, pTeam->get_ID(), (void*)pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), (int)action);
			pTeam->StepCompleted = true;
			break;
		}

		case PhobosScripts::ScatterAttack:{
			ScriptExtData::Mission_ScatterAttack(pTeam, -1);
			break;
		}
		case PhobosScripts::ScatterAttackByAITargetTypes:
			ScriptExtData::Mission_ScatterAttack(pTeam, static_cast<unsigned>(argument) >> 16);
			break;

		case PhobosScripts::PatrolToEnemyBuildingNearby:
			ScriptExtData::PatrolToBuildingNearby(
				pTeam, argument & 0xFFFF, static_cast<unsigned>(argument) >> 16, fresh, true);
			break;

		case PhobosScripts::PatrolToEnemyRally:
			ScriptExtData::PatrolToRally(pTeam, fresh, true);
			break;

		case PhobosScripts::PatrolToFriendlyBuildingNearby:
			ScriptExtData::PatrolToBuildingNearby(
				pTeam, argument & 0xFFFF, static_cast<unsigned>(argument) >> 16, fresh, false);
			break;

		case PhobosScripts::PatrolToFriendlyRally:
			ScriptExtData::PatrolToRally(pTeam, fresh, false);
			break;

		case PhobosScripts::ApplyAttachEffect:
			//Parameter packaging/decoding errors are a common source of "the action ran but had no effect", so the original Argument and decoding results are both printed.
			Debug::Log("[AE] Script %d ApplyAttachEffect: team=[%s] Argument=0x%08X -> effectKey=%d duration=%d\n",
				static_cast<int>(action), pTeam->Type ? pTeam->Type->get_ID() : "<none>",
				static_cast<unsigned>(argument), argument & 0xFFFF, static_cast<unsigned>(argument) >> 16);
			ScriptExtData::ApplyAttachEffect(pTeam, argument & 0xFFFF, static_cast<unsigned>(argument) >> 16);
			break;

		case PhobosScripts::RemoveAttachEffect:
			Debug::Log("[AE] Script %d RemoveAttachEffect: team=[%s] Argument=0x%08X -> effectKey=%d\n",
				static_cast<int>(action), pTeam->Type ? pTeam->Type->get_ID() : "<none>",
				static_cast<unsigned>(argument), argument & 0xFFFF);
			ScriptExtData::RemoveAttachEffect(pTeam, argument & 0xFFFF);
			break;

		case PhobosScripts::RemoveAllAttachEffects:
			Debug::Log("[AE] Script %d RemoveAllAttachEffects: team=[%s]\n",
				static_cast<int>(action), pTeam->Type ? pTeam->Type->get_ID() : "<none>");
			ScriptExtData::RemoveAllAttachEffects(pTeam);
			break;

		case PhobosScripts::ChronoTeleportToWaypoint:
			ScriptExtData::ChronoshiftToWaypoint(pTeam, argument & 0xFFFF);
			break;

		case PhobosScripts::EnterMissionForDuration:
			ScriptExtData::EnterMissionForDuration(pTeam, argument, fresh, pNodeIndex);
			break;

		case PhobosScripts::JumpToLineByLocalVariable:
			ScriptExtData::JumpToLineByVariable(pTeam, argument, ScenarioVariableScope::Local);
			break;

		case PhobosScripts::JumpToLineByGlobalVariable:
			ScriptExtData::JumpToLineByVariable(pTeam, argument, ScenarioVariableScope::Global);
			break;

		case PhobosScripts::WaitForLocalVariable:
			ScriptExtData::WaitForVariable(pTeam, argument, ScenarioVariableScope::Local, fresh);
			break;

		case PhobosScripts::WaitForGlobalVariable:
			ScriptExtData::WaitForVariable(pTeam, argument, ScenarioVariableScope::Global, fresh);
			break;

		case PhobosScripts::JumpToLineIfLocalVariableSet:
			ScriptExtData::JumpToLineIfVariableSet(pTeam, argument, ScenarioVariableScope::Local);
			break;

		case PhobosScripts::JumpToLineIfGlobalVariableSet:
			ScriptExtData::JumpToLineIfVariableSet(pTeam, argument, ScenarioVariableScope::Global);
			break;

		case PhobosScripts::JumpToLocalLineIfLocalSet:
			ScriptExtData::JumpToVariableLineIfSet(pTeam, argument,
				ScenarioVariableScope::Local, ScenarioVariableScope::Local);
			break;

		case PhobosScripts::JumpToGlobalLineIfGlobalSet:
			ScriptExtData::JumpToVariableLineIfSet(pTeam, argument,
				ScenarioVariableScope::Global, ScenarioVariableScope::Global);
			break;

		case PhobosScripts::JumpToLocalLineIfGlobalSet:
			ScriptExtData::JumpToVariableLineIfSet(pTeam, argument,
				ScenarioVariableScope::Global, ScenarioVariableScope::Local);
			break;

		case PhobosScripts::JumpToGlobalLineIfLocalSet:
			ScriptExtData::JumpToVariableLineIfSet(pTeam, argument,
				ScenarioVariableScope::Local, ScenarioVariableScope::Global);
			break;

		case PhobosScripts::AbortActionAfterSuccessKill:
		{
			ScriptExtData::SetAbortActionAfterSuccessKill(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::JumpBackToPreviousScript:
		{
			ScriptExtData::JumpBackToPreviousScript(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::RepairDestroyedBridge:
		{
			// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
			ScriptExtData::RepairDestroyedBridge(pTeam, -1);  //which branch is this again ?
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::ChronoshiftToEnemyBase: //#1077
		{
			// Chronoshift to enemy base, argument is additional distance modifier
			ScriptExtData::ChronoshiftToEnemyBase(pTeam, argument);
			break;
		}
		case PhobosScripts::SimpleDeployerDeploy:
		{
			// Deploy/undeploy SimpleDeployer units
			//Debug::LogInfo("[SCRIPT DEBUG] SimpleDeployerDeploy case triggered! Action: %d, Argument: %d", (int)action, argument);
			ScriptExtData::SimpleDeployerDeploy(pTeam, argument);
			break;
		}

#pragma region Mission_Attack
		case PhobosScripts::RepeatAttackCloserThreat:
		{
			// Threats that are close have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack(pTeam, true, DistanceMode::idkZero, -1, -1); //done
			break;
		}
		case PhobosScripts::RepeatAttackFartherThreat:
		{
			// Threats that are far have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack(pTeam, true, DistanceMode::idkOne, -1, -1); //done
			break;
		}
		case PhobosScripts::RepeatAttackCloser:
		{
			// Closer targets from Team Leader have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack(pTeam, true, DistanceMode::Closest, -1, -1); //done
			break;
		}
		case PhobosScripts::RepeatAttackFarther:
		{
			// Farther targets from Team Leader have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack(pTeam, true, DistanceMode::Furtherst, -1, -1); //done
			break;
		}
		case PhobosScripts::SingleAttackCloserThreat:
		{
			// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack(pTeam, false, DistanceMode::idkZero, -1, -1); //done
			break;
		}
		case PhobosScripts::SingleAttackFartherThreat:
		{
			// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack(pTeam, false, DistanceMode::idkOne, -1, -1); //done
			break;
		}
		case PhobosScripts::SingleAttackCloser:
		{
			// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack(pTeam, false, DistanceMode::Closest, -1, -1); //done
			break;
		}
		case PhobosScripts::SingleAttackFarther:
		{
			// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack(pTeam, false, DistanceMode::Furtherst, -1, -1); //done
			break;
		}
#pragma endregion

#pragma region Mission_Attack_List
		case PhobosScripts::RepeatAttackTypeCloserThreat:
		{
			// Threats specific targets that are close have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack_List(pTeam, true, DistanceMode::idkZero, -1);
			break;
		}
		case PhobosScripts::RepeatAttackTypeFartherThreat:
		{
			// Threats specific targets that are far have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack_List(pTeam, true, DistanceMode::idkOne, -1);
			break;
		}
		case PhobosScripts::RepeatAttackTypeCloser:
		{
			// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack_List(pTeam, true, DistanceMode::Closest, -1);
			break;
		}
		case PhobosScripts::RepeatAttackTypeFarther:
		{
			// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
			ScriptExtData::Mission_Attack_List(pTeam, true, DistanceMode::Furtherst, -1);
			break;
		}
		case PhobosScripts::SingleAttackTypeCloserThreat:
		{
			// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack_List(pTeam, false, DistanceMode::idkZero, -1);
			break;
		}
		case PhobosScripts::SingleAttackTypeFartherThreat:
		{
			// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack_List(pTeam, false, DistanceMode::idkOne, -1);
			break;
		}
		case PhobosScripts::SingleAttackTypeCloser:
		{
			// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack_List(pTeam, false, DistanceMode::Closest, -1);
			break;
		}
		case PhobosScripts::SingleAttackTypeFarther:
		{
			// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExtData::Mission_Attack_List(pTeam, false, DistanceMode::Furtherst, -1);
			break;
		}
#pragma endregion

#pragma region Mission_Attack_List1Random
		case PhobosScripts::RandomAttackTypeCloser:
		{
			// Pick 1 closer random objective from specific list for attacking it
			ScriptExtData::Mission_Attack_List1Random(pTeam, true, DistanceMode::Closest, -1);
			break;
		}
		case PhobosScripts::RandomAttackTypeFarther:
		{
			// Pick 1 farther random objective from specific list for attacking it
			ScriptExtData::Mission_Attack_List1Random(pTeam, true, DistanceMode::Furtherst, -1);
			break;
		}
#pragma endregion

#pragma region Mission_Move
		case PhobosScripts::MoveToEnemyCloser:
		{
			// Move to the closest enemy target
			ScriptExtData::Mission_Move(pTeam, DistanceMode::Closest, false, -1, -1);
			break;
		}
		case PhobosScripts::MoveToEnemyFarther:
		{
			// Move to the farther enemy target
			ScriptExtData::Mission_Move(pTeam, DistanceMode::Furtherst, false, -1, -1);
			break;
		}
		case PhobosScripts::MoveToFriendlyCloser:
		{
			// Move to the closest friendly target
			ScriptExtData::Mission_Move(pTeam, DistanceMode::Closest, true, -1, -1);
			break;
		}
		case PhobosScripts::MoveToFriendlyFarther:
		{
			// Move to the farther friendly target
			ScriptExtData::Mission_Move(pTeam, DistanceMode::Furtherst, true, -1, -1);
			break;
		}
#pragma endregion

#pragma region Mission_Move_List
		case PhobosScripts::MoveToTypeEnemyCloser:
		{
			// Move to the closest specific enemy target
			ScriptExtData::Mission_Move_List(pTeam, DistanceMode::Closest, false, -1);
			break;
		}
		case PhobosScripts::MoveToTypeEnemyFarther:
		{
			// Move to the farther specific enemy target
			ScriptExtData::Mission_Move_List(pTeam, DistanceMode::Furtherst, false, -1);
			break;
		}
		case PhobosScripts::MoveToTypeFriendlyCloser:
		{
			// Move to the closest specific friendly target
			ScriptExtData::Mission_Move_List(pTeam, DistanceMode::Closest, true, -1);
			break;
		}
		case PhobosScripts::MoveToTypeFriendlyFarther:
		{
			// Move to the farther specific friendly target
			ScriptExtData::Mission_Move_List(pTeam, DistanceMode::Furtherst, true, -1);
			break;
		}
#pragma endregion

#pragma region Mission_Move_List1Random
		case PhobosScripts::RandomMoveToTypeEnemyCloser:
		{
			// Pick 1 closer enemy random objective from specific list for moving to it
			ScriptExtData::Mission_Move_List1Random(pTeam, DistanceMode::Closest, false, -1, -1);
			break;
		}
		case PhobosScripts::RandomMoveToTypeEnemyFarther:
		{
			// Pick 1 farther enemy random objective from specific list for moving to it
			ScriptExtData::Mission_Move_List1Random(pTeam, DistanceMode::Furtherst, false, -1, -1);
			break;
		}
		case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		{
			// Pick 1 closer friendly random objective from specific list for moving to it
			ScriptExtData::Mission_Move_List1Random(pTeam, DistanceMode::Closest, true, -1, -1);
			break;
		}
		case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		{
			// Pick 1 farther friendly random objective from specific list for moving to it
			ScriptExtData::Mission_Move_List1Random(pTeam, DistanceMode::Furtherst, true, -1, -1);
			break;
		}
#pragma endregion

#pragma region Stop_ForceJump_Countdown
		case PhobosScripts::StopForceJumpCountdown:
		{
			// Stop Timed Jump
			ScriptExtData::Stop_ForceJump_Countdown(pTeam);
			break;
		}
		case PhobosScripts::NextLineForceJumpCountdown:
		{
			// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
			ScriptExtData::Set_ForceJump_Countdown(pTeam, false, -1);
			break;
		}
		case PhobosScripts::SameLineForceJumpCountdown:
		{
			// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
			ScriptExtData::Set_ForceJump_Countdown(pTeam, true, -1);
			break;
		}
#pragma endregion

#pragma region AngerNodes
		case PhobosScripts::SetHouseAngerModifier:
		{
			ScriptExtData::SetHouseAngerModifier(pTeam, 0); //which branch is this again ?
			break;
		}
		case PhobosScripts::OverrideOnlyTargetHouseEnemy:
		{
			ScriptExtData::OverrideOnlyTargetHouseEnemy(pTeam, -1); //which branch is this again ?
			break;
		}
		case PhobosScripts::ModifyHateHouseIndex:
		{
			ScriptExtData::ModifyHateHouse_Index(pTeam, -1); //which branch is this again ?
			break;
		}
		case PhobosScripts::ModifyHateHousesList:
		{
			ScriptExtData::ModifyHateHouses_List(pTeam, -1); //which branch is this again ?
			break;
		}
		case PhobosScripts::ModifyHateHousesList1Random:
		{
			ScriptExtData::ModifyHateHouses_List1Random(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::SetTheMostHatedHouseMinorNoRandom:
		{
			// <, no random
			ScriptExtData::SetTheMostHatedHouse(pTeam, 0, 0, false);  //which branch is this again ?
			break;
		}
		case PhobosScripts::SetTheMostHatedHouseMajorNoRandom:
		{
			// >, no random
			ScriptExtData::SetTheMostHatedHouse(pTeam, 0, 1, false);
			break;
		}
		case PhobosScripts::SetTheMostHatedHouseRandom:
		{
			// random
			ScriptExtData::SetTheMostHatedHouse(pTeam, 0, 0, true);
			break;
		}
		case PhobosScripts::ResetAngerAgainstHouses:
		{
			ScriptExtData::ResetAngerAgainstHouses(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::AggroHouse:
		{
			ScriptExtData::AggroHouse(pTeam, -1);  //which branch is this again ?
			break;
		}
#pragma endregion

#pragma region ConditionalJump //#599
		case PhobosScripts::ConditionalJumpSetCounter:
		{
			ScriptExtData::ConditionalJump_SetCounter(pTeam, -100000000);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpSetComparatorMode:
		{
			ScriptExtData::ConditionalJump_SetComparatorMode(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpSetComparatorValue:
		{
			ScriptExtData::ConditionalJump_SetComparatorValue(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpSetIndex:
		{
			ScriptExtData::ConditionalJump_SetIndex(pTeam, -1000000);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpResetVariables:
		{
			ScriptExtData::ConditionalJump_ResetVariables(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpIfFalse:
		{
			ScriptExtData::ConditionalJumpIfFalse(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpIfTrue:
		{
			ScriptExtData::ConditionalJumpIfTrue(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpManageKillsCounter:
		{
			ScriptExtData::ConditionalJump_ManageKillsCounter(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpCheckAliveHumans:
		{
			ScriptExtData::ConditionalJump_CheckAliveHumans(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpCheckHumanIsMostHated:
		{
			ScriptExtData::ConditionalJump_CheckHumanIsMostHated(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpKillEvaluation:
		{
			ScriptExtData::ConditionalJump_KillEvaluation(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpCheckObjects:
		{
			ScriptExtData::ConditionalJump_CheckObjects(pTeam);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpCheckCount:
		{
			ScriptExtData::ConditionalJump_CheckCount(pTeam, 0);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ConditionalJumpManageResetIfJump:
		{
			ScriptExtData::ConditionalJump_ManageResetIfJump(pTeam, -1);  //which branch is this again ?
			break;
		}
#pragma endregion

#pragma region ManagingTriggers
		case PhobosScripts::SetSideIdxForManagingTriggers:
		{
			ScriptExtData::SetSideIdxForManagingTriggers(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::SetHouseIdxForManagingTriggers:
		{
			ScriptExtData::SetHouseIdxForManagingTriggers(pTeam, 1000000);  //which branch is this again ?
			break;
		}
		case PhobosScripts::ManageAllAITriggers:
		{
			ScriptExtData::ManageAITriggers(pTeam, -1);  //which branch is this again ?
			break;
		}
		case PhobosScripts::EnableTriggersFromList:
		{
			ScriptExtData::ManageTriggersFromList(pTeam, -1, true);  //which branch is this again ?
			break;
		}
		case PhobosScripts::DisableTriggersFromList:
		{
			ScriptExtData::ManageTriggersFromList(pTeam, -1, false);  //which branch is this again ?
			break;
		}
		case PhobosScripts::EnableTriggersWithObjects:
		{
			ScriptExtData::ManageTriggersWithObjects(pTeam, -1, true);  //which branch is this again ?
			break;
		}
		case PhobosScripts::DisableTriggersWithObjects:
		{
			ScriptExtData::ManageTriggersWithObjects(pTeam, -1, false);  //which branch is this again ?
			break;
		}
#pragma endregion
		default:
			return ScriptExtData::VariablesHandler(pTeam, PhobosScripts(Action), argument);
		}
	}

	return true;
}

void NOINLINE ScriptExtData::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto const pScript = pTeam->CurrentScript;
	auto const pScriptType = pScript->Type;

	if (pScriptType->ScriptActions[pScript->CurrentMission].Argument <= 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	const auto Isticking = pTeam->GuardAreaTimer.IsTicking();
	const auto TimeLeft = pTeam->GuardAreaTimer.GetTimeLeft();

	if (!Isticking && !TimeLeft)
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (TechnoExtData::IsInWarfactory(pUnit))
				continue; // held back Timed area guard if one of the member still in warfactory

			pUnit->QueueMission(Mission::Area_Guard, true);
		}

		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->CurrentMission].Argument);
	}
	else if (Isticking && !TimeLeft)
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

#include <ExtraHeaders/StackVector.h>

void ScriptExtData::LoadIntoTransports(TeamClass* pTeam)
{
	StackVector<FootClass*, 10> transports {};

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = GET_TECHNOTYPE(pUnit);

		if (pType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pType->Passengers
			&& (TechnoTypeExtContainer::Instance.Find(pType)->Passengers_BySize
				? pUnit->Passengers.GetTotalSize() : pUnit->Passengers.NumPassengers)
			 < pType->Passengers)
		{
			transports->push_back(pUnit);
		}
	}

	if (transports->empty())
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Now load units into transports
	for (auto pTransport : transports.container())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto const pTransportType = GET_TECHNOTYPE(pTransport);
			auto const pUnitType = GET_TECHNOTYPE(pUnit);

			if (pTransport != pUnit
				&& pUnitType->WhatAmI() != AbstractType::AircraftType
				&& !pUnit->InLimbo && !pUnitType->ConsideredAircraft
				&& pUnit->Health > 0)
			{
				if (pUnitType->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					// If is still flying wait a bit more
					if (pTransport->IsInAir())
						return;

					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);
						pUnit->NextMission();
						return;
					}
				}
			}
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
	}

	auto const pExt = TeamExtContainer::Instance.Find(pTeam);

	FootClass* pLeaderUnit = pTeam->FetchLeader();
	pLeaderUnit->IsTeamLeader = 1;
	pExt->TeamLeader = pLeaderUnit;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (ScriptExtData::IsUnitAvailable(pUnit, false))
		{
			const auto pType = GET_TECHNOTYPE(pUnit);

			if (pType->Ammo > 0 && pUnit->Ammo < pType->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (auto const pAircraft = cast_to<AircraftClass*, false>(pUnit))
				{
					if (pAircraft->Type->AirportBound)
					{
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pUnit->SetTarget(nullptr);
						pUnit->LastTarget = nullptr;
						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pUnit->IsInAir() && pUnit->CurrentMission != Mission::Enter)
							pUnit->QueueMission(Mission::Enter, true);

						break;
					}
				}
				else if (pType->Reload != 0) // Don't skip units that can reload themselves
					break;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExtData::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown = -1)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->GetCurrentAction().Argument;
	bool gatherUnits = false;
	auto const pExt = TeamExtContainer::Instance.Find(pTeam);

	// Load countdown
	if (pExt->Countdown_RegroupAtLeader >= 0)
		countdown = pExt->Countdown_RegroupAtLeader;

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
		gatherUnits = true;

	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else if (countdown == 0) // Countdown ended
			countdown = -1;
		else // Start countdown.
		{
			countdown = initialCountdown * 15;
			gatherUnits = true;
		}

		// Save counter
		pExt->Countdown_RegroupAtLeader = countdown;
	}

	if (!gatherUnits)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
		int nTogether = 0;
		int nUnits = -1; // Leader counts here
		double closeEnough;

		// Find the Leader
		pLeaderUnit = pExt->TeamLeader;

		if (!ScriptExtData::IsUnitAvailable(pLeaderUnit, true))
		{
			if(pLeaderUnit)
				pLeaderUnit->IsTeamLeader = false;

			pLeaderUnit = pTeam->FetchLeader();
			pLeaderUnit->IsTeamLeader= true;
			pExt->TeamLeader = pLeaderUnit;
		}

		if (!pLeaderUnit)
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pExt->CloseEnough > 0)
		{
			// This a one-time-use value
			closeEnough = std::exchange(pExt->CloseEnough, -1);
		}
		else
		{
			closeEnough = RulesClass::Instance->CloseEnough.ToCell();
		}

		// The leader should stay calm & be the group's center
		if (pLeaderUnit->Locomotor.GetInterfacePtr()->Is_Moving_Now())
			pLeaderUnit->SetDestination(nullptr, false);

		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExtData::IsUnitAvailable(pUnit, true))
			{
				auto pTypeUnit = GET_TECHNOTYPE(pUnit);

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				auto pAir_ = cast_to<AircraftClass*, false>(pUnit);

				// Aircraft case
				if (pAir_ && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = pAir_->Type;

					if (pAircraft->AirportBound)
					{
						// This aircraft won't count for the script action
						pUnit->EnterIdleMode(false, true);

						continue;
					}
				}

				nUnits++;

				if ((pUnit->DistanceFrom(pLeaderUnit) / 256.0) > closeEnough)
				{
					// Leader's location is too far from me. Regroup
					if (pUnit->Destination != pLeaderUnit)
					{
						pUnit->SetDestination(pLeaderUnit, false);
						pUnit->QueueMission(Mission::Move, true);
					}
				}
				else
				{
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard && pUnit->GetCurrentMission() != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}

		if (nUnits >= 0
			&& nUnits == nTogether
			&& (initialCountdown == 0
				|| (initialCountdown > 0
					&& countdown <= 0)))
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}
	}
}

void ScriptExtData::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ScriptExtData::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExtData::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = Math::abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExtData::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExtData::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;

	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->Items[i]->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->Items[i]->Team2;

		if (pTeamType
			&& ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType)
				|| (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->Items[i];
		}
	}

	if (found)
	{
		pTriggerType->Weight_Current += modifier;

		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum)
		{
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
		}
		else
		{
			if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
				pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
		}
	}
}

void ScriptExtData::WaitIfNoTarget(TeamClass* pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->GetCurrentAction().Argument;

	auto pExt = TeamExtContainer::Instance.Find(pTeam);

	if (attempts <= 0)
		pExt->WaitNoTargetAttempts = -1; // Infinite waits if no target
	else
		pExt->WaitNoTargetAttempts = attempts;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExtData::TeamWeightReward(TeamClass* pTeam, double award = 0)
{
	if (award <= 0)
		award = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (award > 0)
		TeamExtContainer::Instance.Find(pTeam)->NextSuccessWeightAward = award;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExtData::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
{
	if (idxScriptsList < 0)
		idxScriptsList = pTeam->CurrentScript->GetCurrentAction().Argument;

	const auto& scriptList = FakeRulesClass::Instance()->AIScriptsLists;
	if ((size_t)idxScriptsList < scriptList.size())
	{
		const auto& objectsList = scriptList[idxScriptsList];

		if (!objectsList.empty())
		{
			int IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

			ScriptTypeClass* pNewScript = objectsList[IdxSelectedObject];

			if (pNewScript->ActionsCount > 0 && pNewScript != pTeam->CurrentScript->Type)
			{
				TeamExtContainer::Instance.Find(pTeam)->PreviousScript = pTeam->CurrentScript->Type;
				GameDelete<true, false>(pTeam->CurrentScript);
				pTeam->CurrentScript = GameCreate<ScriptClass>(pNewScript);
				// Ready for jumping to the first line of the new script
				pTeam->CurrentScript->CurrentMission = -1;
				pTeam->StepCompleted = true;

				return;
			}
			else
			{
				pTeam->StepCompleted = true;
				Debug::LogInfo("AI Scripts - PickRandomScript: [{}] Aborting Script change because [{}] has 0 Action scripts!",
					pTeam->Type->ID,
					pNewScript->ID
				);

				return;
			}
		}
	}

	pTeam->StepCompleted = true;
	Debug::LogInfo("AI Scripts - PickRandomScript: [{}] [{}] Failed to change the Team Script with index [{}]!",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		idxScriptsList
	);
}

void ScriptExtData::SetCloseEnoughDistance(TeamClass* pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->GetCurrentAction().Argument;

	auto const pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (distance > 0)
		pTeamData->CloseEnough = distance;

	if (distance <= 0)
		pTeamData->CloseEnough = RulesClass::Instance->CloseEnough.ToCell();

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExtData::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExtData::SetMoveMissionEndMode(TeamClass* pTeam, int mode = 0)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (auto const pTeamData = TeamExtContainer::Instance.Find(pTeam))
	{
		if (mode >= 0 && mode <= 2)
			pTeamData->MoveMissionEndMode = mode;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExtData::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader = nullptr, int mode = 0)
{
	if (!pTeam || mode < 0)
		return false;

	if (!ScriptExtData::IsUnitAvailable(pFocus, false))
	{
		pTeam->ArchiveTarget = nullptr;
		return false;
	}

	auto const pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (mode != 2 && mode != 1 && !ScriptExtData::IsUnitAvailable(pLeader, false))
	{
		pTeamData->TeamLeader = nullptr;
		return false;
	}

	double closeEnough = 0.0;

	if (pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;
	else
		closeEnough = RulesClass::Instance->CloseEnough.ToCell();

	bool bForceNextAction = mode == 2;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (ScriptExtData::IsUnitAvailable(pUnit, true)
			&& !pUnit->TemporalTargetingMe
			&& !pUnit->IsBeingWarpedOut())
		{
			if (mode == 2)
			{
				// Default mode: all members in range
				if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, true);

					continue;
				}
				else
				{
					if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
					{
						pUnit->EnterIdleMode(false, true);

						continue;
					}
				}
			}
			else
			{
				if (mode == 1)
				{
					// Any member in range
					if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
					{
						if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, true);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
						{
							pUnit->EnterIdleMode(false, true);

							continue;
						}
					}
				}
				else
				{
					// All other cases: Team Leader mode in range
					if (ScriptExtData::IsUnitAvailable(pLeader, false))
					{
						if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
						{
							if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, true);

							continue;
						}
						else
						{
							if (pUnit->IsTeamLeader)
								bForceNextAction = true;

							if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
							{
								pUnit->EnterIdleMode(false, true);

								continue;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	return bForceNextAction;
}

void ScriptExtData::SkipNextAction(TeamClass* pTeam, int successPercentage = 0)
{
	// This team has no units! END
	//if (!pTeam)
	//{
	//	// This action finished
	//	pTeam->StepCompleted = true;
	//	const auto&[curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();
	//	const auto&[nextAct, nextArg] = pTeam->CurrentScript->GetNextAction();
	//	Debug::LogInfo("AI Scripts - SkipNextAction: [{}] [{}] (line: {}) Jump to next line: {} = {},{} -> (No team members alive)",
	//		pTeam->Type->ID,
	//		pTeam->CurrentScript->Type->ID,
	//		pTeam->CurrentScript->CurrentMission,
	//		curAct,
	//		curArg,
	//		pTeam->CurrentScript->CurrentMission + 1,
	//		nextAct,
	//		nextArg);
	//
	// 	return;
	// }

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	int percentage = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (percentage <= successPercentage)
	{
		// const auto& [curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();
		// const auto& [nextAct, nextArg] = ScriptExtData::GetSpecificAction(pTeam->CurrentScript, pTeam->CurrentScript->CurrentMission + 2);

		// Debug::LogInfo("AI Scripts - SkipNextAction: [{}] [{}] (line: {} = {},{}) Next script line skipped successfuly. Next line will be: {} = {},{}",
		// 	pTeam->Type->ID,
		// 	pTeam->CurrentScript->Type->ID,
		// 	pTeam->CurrentScript->CurrentMission,
		// 	(int)curAct,
		// 	curArg,
		// 	pTeam->CurrentScript->CurrentMission + 2,
		// 	(int)nextAct,
		// 	nextArg
		// );

		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

bool ScriptExtData::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
{
	struct operation_set { int operator()(const int& a, const int& b) { return b; } };
	struct operation_add { int operator()(const int& a, const int& b) { return a + b; } };
	struct operation_minus { int operator()(const int& a, const int& b) { return a - b; } };
	struct operation_multiply { int operator()(const int& a, const int& b) { return a * b; } };
	struct operation_divide { int operator()(const int& a, const int& b) { return a / b; } };
	struct operation_mod { int operator()(const int& a, const int& b) { return a % b; } };
	struct operation_leftshift { int operator()(const int& a, const int& b) { return a << b; } };
	struct operation_rightshift { int operator()(const int& a, const int& b) { return a >> b; } };
	struct operation_reverse { int operator()(const int& a, const int& b) { return ~a; } };
	struct operation_xor { int operator()(const int& a, const int& b) { return a ^ b; } };
	struct operation_or { int operator()(const int& a, const int& b) { return a | b; } };
	struct operation_and { int operator()(const int& a, const int& b) { return a & b; } };

	int nLoArg = LOWORD(nArg);
	int nHiArg = HIWORD(nArg);

	switch (eAction)
	{
	case PhobosScripts::LocalVariableSet:
		VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	default:
		return false;
	}

	return true;
}

template<bool IsGlobal, class _Pr>
void ScriptExtData::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	if (auto itr = ScenarioExtData::GetVariables(IsGlobal)->tryfind(nVariable))
	{
		itr->Value = _Pr()(itr->Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}

	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExtData::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	if (auto itr = ScenarioExtData::GetVariables(IsGlobal)->tryfind(nVarToOperate))
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->Value);

	pTeam->StepCompleted = true;
}

bool ScriptExtData::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExtData::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine = false, int count = 0)
{
	if (!pTeam)
		return;
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();

	if (count <= 0)
		count = 15 * curArgs;

	if (count > 0)
	{
		pTeamData->ForceJump_InitialCountdown = count;
		pTeamData->ForceJump_Countdown.Start(count);
		pTeamData->ForceJump_RepeatMode = repeatLine;
	}
	else
	{
		pTeamData->ForceJump_InitialCountdown = -1;
		pTeamData->ForceJump_Countdown.Stop();
		pTeamData->ForceJump_RepeatMode = false;
	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	Debug::LogInfo("AI Scripts - SetForceJumpCountdown: [{}] [{}](line: {} = {},{}) Set Timed Jump -> (Countdown: {}, repeat action: {})",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		(int)curAct,
		curArgs,
		count, repeatLine
	);
}

void ScriptExtData::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_RepeatMode = false;

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	Debug::LogInfo("AI Scripts - StopForceJumpCountdown: [{}] [{}](line: {} = {},{}): Stopped Timed Jump",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		(int)curAct,
		curArgs
	);
}

void ScriptExtData::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	//auto pScript = pTeam->CurrentScript;
	auto const pLeader = pTeam->FetchLeader();
	//const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	//const auto& [nextAct, nextArgs] = pTeam->CurrentScript->GetNextAction();
	pLeader->IsTeamLeader = true;

	if (!pLeader)
	{
		pTeam->StepCompleted = true;
		return;
	}

	int houseIndex = pLeader->Owner->EnemyHouseIndex;
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array->Items[houseIndex] : nullptr;

	if (!pEnemy)
	{
		pTeam->StepCompleted = true;
		return;
	}

	auto const pTargetCell = HouseExtData::GetEnemyBaseGatherCell(pTeam->Type, pEnemy, 
		pLeader->Owner, 
		pLeader->GetCoords(), 
		GET_TECHNOTYPE(pLeader)->SpeedType,
		extraDistance
	);

	if (!pTargetCell)
	{
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExtData::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

void ScriptExtData::ChronoshiftToWaypoint(TeamClass* pTeam, int waypoint)
{
	//auto pScript = pTeam->CurrentScript;
	auto const pLeader = pTeam->FetchLeader();
	//const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	//const auto& [nextAct, nextArgs] = pTeam->CurrentScript->GetNextAction();
	pLeader->IsTeamLeader = true;

	if (!pLeader)
	{
		pTeam->StepCompleted = true;
		return;
	}

	int houseIndex = pLeader->Owner->EnemyHouseIndex;
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array->Items[houseIndex] : nullptr;

	if (!pEnemy)
	{
		pTeam->StepCompleted = true;
		return;
	}

	CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(waypoint);

	auto const pTargetCell = MapClass::Instance->GetCellAt(cell);

	if (!pTargetCell)
	{
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExtData::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

//5514/5515: The line number is taken from the script variable, the semantics are the same as the original action 6 (the line number is 1 base)
//Original 0x6E98B5 SetMission(Arg -2) first, then StepCompleted, NextMission +1 in the next frame, exactly falling on the Arg line
void ScriptExtData::JumpToLineByVariable(TeamClass* pTeam, int variableIndex, ScenarioVariableScope scope)
{
	auto pVariable = ScenarioExtData::GetVariables(scope);
	auto scope_str = EnumFunctions::ScenarioVariableScope_ToStrings[static_cast<int>(scope)].second.data();

	//When the variable index is out of bounds or the variable storage is unavailable, it is directly released to avoid the entire script being stuck on this line.
	if (!pVariable->contains(variableIndex)) {
		Debug::Log("[Script] JumpToLine: variable read failed, scope=%s index=%d (valid [0, %d))\n",
			scope_str, variableIndex, pVariable->size());
		pTeam->StepCompleted = true;
		return;
	}

	auto pIter = pVariable->get_key_iterator(variableIndex);

	//0 is consistent with the original version (the script stops advancing); if it is negative or exceeds the length of the script, skip this line, otherwise the engine will take action based on the out-of-bounds line number
	int const maxLine = std::min(pTeam->CurrentScript->Type->ActionsCount, 50);
	int line = pIter->second.Value;

	if (line < 0 || line > maxLine)
	{
		Debug::Log("[Script] JumpToLine: line %d out of range [1, %d], skip\n", line, maxLine);
		pTeam->StepCompleted = true;
		return;
	}

	Debug::Log("[Script] JumpToLine: team=[%s] %s var[%d]=%d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>",
		scope_str,
		variableIndex, line);

	pTeam->CurrentScript->SetMission(line - 2);
	pTeam->StepCompleted = true;
}

// 5516/5517: When the variable is 0, the team stops at this line and waits. When it becomes non-0, the next line is executed.
void ScriptExtData::WaitForVariable(TeamClass* pTeam, int variableIndex, ScenarioVariableScope scope, bool fresh)
{
	auto pVariable = ScenarioExtData::GetVariables(scope);
	auto scope_str = EnumFunctions::ScenarioVariableScope_ToStrings[static_cast<int>(scope)].second.data();

	// Do not wait when the index is illegal or the variable storage is unavailable, otherwise the team will be permanently stuck.
	if (!pVariable->contains(variableIndex)) {
		Debug::Log("[Script] WaitForVariable: variable read failed, scope=%s index=%d (valid [0, %d))\n",
			scope_str, variableIndex, pVariable->size());
		pTeam->StepCompleted = true;
		return;
	}

	auto pIter = pVariable->get_key_iterator(variableIndex);
	int value = pIter->second.Value;

	if (value == 0) {
		pTeam->StepCompleted = false;

		if (fresh) {
			Debug::Log("[Script] WaitForVariable: team=[%s] %s var[%d]=0 -> waiting\n",
				pTeam->Type ? pTeam->Type->get_ID() : "<none>",
				scope_str,
				variableIndex);
		}

		return;
	}

	Debug::Log("[Script] WaitForVariable: team=[%s] %s var[%d]=%d -> continue\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>",
		scope_str,
		variableIndex, value);

	pTeam->StepCompleted = true;
}

void ScriptExtData::JumpToLineIfVariableSet(TeamClass* pTeam, int argument, ScenarioVariableScope scope)
{
	int const variableIndex = argument & 0xFFFF;
	int const line = static_cast<unsigned>(argument) >> 16;

	auto pVariable = ScenarioExtData::GetVariables(scope);
	auto scope_str = EnumFunctions::ScenarioVariableScope_ToStrings[static_cast<int>(scope)].second.data();

	if (!pVariable->contains(variableIndex))
	{
		Debug::Log("[Script] JumpIfVariableSet: variable read failed, scope=%s index=%d (valid [0, %d))\n",
			scope_str, variableIndex, pVariable->size());
		pTeam->StepCompleted = true;
		return;
	}

	auto pIter = pVariable->get_key_iterator(variableIndex);
	int value = pIter->second.Value;

	if (value == 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	int const maxLine = std::min(pTeam->CurrentScript->Type->ActionsCount, 50);

	if (line > maxLine)
	{
		Debug::Log("[Script] JumpIfVariableSet: line %d out of range [1, %d], skip\n", line, maxLine);
		pTeam->StepCompleted = true;
		return;
	}

	Debug::Log("[Script] JumpIfVariableSet: team=[%s] %s var[%d]=%d -> line %d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>",
		scope_str,
		variableIndex, value, line);

	pTeam->CurrentScript->SetMission(line - 2);
	pTeam->StepCompleted = true;
}

void ScriptExtData::JumpToVariableLineIfSet(TeamClass* pTeam, int argument,
	ScenarioVariableScope conditionScope, ScenarioVariableScope lineScope)
{
	auto pVariableconditionScope = ScenarioExtData::GetVariables(conditionScope);
	auto conditionScope_str = EnumFunctions::ScenarioVariableScope_ToStrings[static_cast<int>(conditionScope)].second.data();

	int const conditionIndex = argument & 0xFFFF;
	int const lineIndex = static_cast<unsigned>(argument) >> 16;

	if (!pVariableconditionScope->contains(conditionIndex))
	{
		Debug::Log("[Script] JumpToVariableLine: A read failed, scope=%s index=%d (valid [0, %d))\n",
			conditionScope_str, conditionIndex, pVariableconditionScope->size());
		pTeam->StepCompleted = true;
		return;
	}

	auto pConditionIndexIter = pVariableconditionScope->get_key_iterator(conditionIndex);
	int value = pConditionIndexIter->second.Value;

	if (value == 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	auto pVariablelineScope = ScenarioExtData::GetVariables(lineScope);
	auto lineScope_str = EnumFunctions::ScenarioVariableScope_ToStrings[static_cast<int>(lineScope)].second.data();

	if (!pVariablelineScope->contains(lineIndex))
	{
		Debug::Log("[Script] JumpToVariableLine: B read failed, scope=%s index=%d (valid [0, %d))\n",
			lineScope_str, lineIndex, pVariablelineScope->size());
		pTeam->StepCompleted = true;
		return;
	}
	auto plineIndexIter = pVariableconditionScope->get_key_iterator(lineIndex);
	int line = plineIndexIter->second.Value;
	int const maxLine = std::min(pTeam->CurrentScript->Type->ActionsCount, 50);

	if (line < 0 || line > maxLine)
	{
		Debug::Log("[Script] JumpToVariableLine: line %d out of range [1, %d], skip\n", line, maxLine);
		pTeam->StepCompleted = true;
		return;
	}

	Debug::Log("[Script] JumpToVariableLine: team=[%s] %s[%d]=%d -> %s[%d]=%d -> line %d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>",
		conditionScope_str, conditionIndex, value,
		lineScope_str, lineIndex, line);

	pTeam->CurrentScript->SetMission(line - 2);
	pTeam->StepCompleted = true;
}

// 5513: The whole team enters the designated Mission and maintains it for N seconds. The lower 16 digits are Mission and the higher 16 digits are seconds.
void ScriptExtData::EnterMissionForDuration(TeamClass* pTeam, int argument, bool fresh, int nodeIndex)
{
	auto const pExt = TeamExtContainer::Instance.Find(pTeam);
	const int missionIndex = argument & 0xFFFF;
	const int duration = static_cast<int>(static_cast<unsigned>(argument) >> 16) * 15; // 原版 15 帧/秒

	// When the status code is illegal, it is directly released, otherwise the entire script is stuck in this line.
	if (missionIndex > static_cast<int>(Mission::SpyplaneOverfly))
	{
		Debug::Log("[Script] EnterMissionForDuration: invalid mission %d (Argument=0x%08X)\n",
			missionIndex, argument);
		pExt->TimedMissionNode = -1;
		pExt->TimedMissionLeft = 0;
		pTeam->StepCompleted = true;
		return;
	}

	const auto mission = static_cast<Mission>(missionIndex);

	if (fresh || pExt->TimedMissionNode != nodeIndex)
	{
		pExt->TimedMissionNode = nodeIndex;
		pExt->TimedMissionLeft = duration;
		pExt->TimedMissionLastFrame = Unsorted::CurrentFrame();
	}
	else if (pExt->TimedMissionLeft > 0)
	{
		int const now = Unsorted::CurrentFrame();
		if (pExt->TimedMissionLastFrame < 0 || now < pExt->TimedMissionLastFrame)
		{
			pExt->TimedMissionLastFrame = now;
		}
		else
		{
			pExt->TimedMissionLeft -= now - pExt->TimedMissionLastFrame;
			pExt->TimedMissionLastFrame = now;
		}
	}

	// DoMissionFromScript is not called: it requires that Team Target /Destination are empty,
	// The Destination of the team that has just completed the movement action is still there, and the entire team will be skipped.
	int total = 0;
	int assigned = 0;

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (!pFoot->IsAlive || pFoot->Health <= 0 || pFoot->InLimbo || !pFoot->IsOnMap || pFoot->Transporter)
			continue;

		++total;

		if (pFoot->GetCurrentMission() == mission || pFoot->QueuedMission == mission)
			continue;

		// The first release will take effect unconditionally; subsequent leaks will only be filled when there is no combat to avoid pulling back the attack.
		if (!fresh && pFoot->Target)
			continue;

		// Switch the state first and then clear the old destination, otherwise the team member will continue to move to the path point of the previous action.
		pFoot->QueueMission(mission, true);
		pFoot->SetDestination(nullptr, true);
		++assigned;
	}

	if (fresh)
	{
		Debug::Log("[Script] EnterMissionForDuration: team=[%s] mission=%d frames=%d members=%d assigned=%d\n",
			pTeam->Type ? pTeam->Type->get_ID() : "<none>", missionIndex, duration, total, assigned);
	}

	if (pExt->TimedMissionLeft > 0)
	{
		pTeam->StepCompleted = false;
	}
	else
	{
		pExt->TimedMissionLeft = 0;
		pTeam->StepCompleted = true;
	}
}

void ScriptExtData::Mission_ScatterAttack(TeamClass* pTeam, int attackAITargetType)
{
	auto const pExt = TeamExtContainer::Instance.Find(pTeam);

	// [AITargetTypes] Index out of bounds: No custom target list available, push script directly
	if (attackAITargetType >= 0
		&& attackAITargetType >= static_cast<int>(FakeRulesClass::Instance->AITargetTypesLists.size()))
	{
		Debug::Log("[Developer warning] ScatterAttackByAITargetTypes: [AITargetTypes] index %d out of range (Count: %d)\n",
			attackAITargetType, static_cast<int>(FakeRulesClass::Instance->AITargetTypesLists.size()));
		pExt->ScatterAttackGroups.clear();
		return;
	}

	// Collect available members (alive, on the map, not equipped)
	std::vector<FootClass*> members;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->IsAlive || pUnit->Health <= 0 || pUnit->InLimbo || !pUnit->IsOnMap)
			continue;
		if (pUnit->Transporter)
			continue;
		members.push_back(pUnit);
	}

	// No available members -> Clear the group and advance the script
	if (members.empty())
	{
		pExt->ScatterAttackGroups.clear();
		return;
	}

	// One-time grouping: When entering this action for the first time (or after loading files/after completing a round), sorted by azimuth and divided evenly
	if (pExt->ScatterAttackGroups.empty())
	{
		// Number of groups: lower 16 digits (at least 1 group), if it exceeds the number of members, it will be based on the number of members (one group per person)
		int groupCount = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument & 0xFFFF;
		if (groupCount < 1) groupCount = 1;
		if (groupCount > static_cast<int>(members.size()))
			groupCount = static_cast<int>(members.size());

		// Squad center (used for orientation sequencing)
		double teamCenterX = 0.0;
		double teamCenterY = 0.0;
		for (auto pFoot : members)
		{
			const CoordStruct c = pFoot->GetCoords();
			teamCenterX += c.X;
			teamCenterY += c.Y;
		}
		teamCenterX /= members.size();
		teamCenterY /= members.size();

		//Sort the members according to their azimuth angle relative to the center of the team, and then divide them into each group continuously:
		//Each group occupies one direction sector, allowing each group to disperse in different directions.
		//stable_sort: Maintain the original order when the azimuth angles are parallel, and each machine is consistent
		std::stable_sort(members.begin(), members.end(), [&](FootClass* a, FootClass* b)
 {
	 const CoordStruct ca = a->GetCoords();
	 const CoordStruct cb = b->GetCoords();
	 const double ba = std::atan2(static_cast<double>(ca.Y) - teamCenterY, static_cast<double>(ca.X) - teamCenterX);
	 const double bb = std::atan2(static_cast<double>(cb.Y) - teamCenterY, static_cast<double>(cb.X) - teamCenterX);
	 return ba < bb;
		});

		// Evenly divided: the first extra group has 1 more person in each group
		const int groupBase = static_cast<int>(members.size()) / groupCount;
		const int groupExtra = static_cast<int>(members.size()) % groupCount;

		int begin = 0;
		for (int g = 0; g < groupCount; ++g)
		{
			const int groupSize = groupBase + (g < groupExtra ? 1 : 0);
			pExt->ScatterAttackGroups.emplace_back(members.begin() + begin, members.begin() + begin + groupSize);
			begin += groupSize;
		}
	}

	// Check every frame if group members are still valid: removal if not among currently available members, empty group removal
	for (auto it = pExt->ScatterAttackGroups.begin(); it != pExt->ScatterAttackGroups.end(); )
	{
		auto& group = *it;
		group.erase(std::remove_if(group.begin(), group.end(), [&](FootClass* pFoot)
			{
				return std::find(members.begin(), members.end(), pFoot) == members.end();
			}), group.end());

		if (group.empty())
			it = pExt->ScatterAttackGroups.erase(it);
		else
			++it;
	}

	if (pExt->ScatterAttackGroups.empty())
	{
		pExt->ScatterAttackGroups.clear(); // All groups have been cleared (all members have expired)
		return;
	}

	// Newly added members (not in any group) are merged into the group with the fewest members, keeping the groupings from being rearranged.
	for (auto pFoot : members)
	{
		bool inGroup = false;
		for (auto& group : pExt->ScatterAttackGroups)
		{
			if (std::find(group.begin(), group.end(), pFoot) != group.end())
			{
				inGroup = true;
				break;
			}
		}
		if (!inGroup)
		{
			size_t bestIdx = 0;
			size_t bestSize = pExt->ScatterAttackGroups[0].size();
			for (size_t gi = 1; gi < pExt->ScatterAttackGroups.size(); ++gi)
			{
				if (pExt->ScatterAttackGroups[gi].size() < bestSize)
				{
					bestSize = pExt->ScatterAttackGroups[gi].size();
					bestIdx = gi;
				}
			}
			pExt->ScatterAttackGroups[bestIdx].push_back(pFoot);
		}
	}

	const int targetMask = static_cast<unsigned>(pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument) >> 16;

	constexpr DistanceMode threatMode = DistanceMode::idkZero;

	// A set of targets that have been locked by this group (different groups have priority in using different targets)
	std::vector<TechnoClass*> assignedTargets;

	// One copy of the enemy search context for each group: group center + whether there is an infiltration agent/engineer in the group (aligned upstream)
	auto const computeGroupContext = [](const std::vector<FootClass*>& group, CoordStruct& groupCenter) -> bool
		{
			groupCenter = CoordStruct { 0, 0, 0 };

			for (auto pFoot : group)
			{
				const CoordStruct c = pFoot->GetCoords();
				groupCenter.X += c.X;
				groupCenter.Y += c.Y;
				groupCenter.Z += c.Z;
			}

			const int groupSize = static_cast<int>(group.size());
			groupCenter.X /= groupSize;
			groupCenter.Y /= groupSize;
			groupCenter.Z /= groupSize;

			// agentMode：Skip some weapon/immunity determinations when there are infiltration agents or engineers in the group (align upstream)
			for (auto pFoot : group)
			{
				if (pFoot->WhatAmI() == AbstractType::Infantry)
				{
					const auto pTypeInf = static_cast<InfantryTypeClass*>(pFoot->GetTechnoType());

					if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
						return true;
				}
			}

			return false;
		};

	auto CanEngageTarget = [](FootClass* pFoot, TechnoClass* pTarget)
		{
			if (!pFoot || !pTarget)
				return false;

			// 与引擎一致：逐个武器槽判定；空武器槽不参与
			for (int i = 0; i < 2; ++i)
			{
				const auto pWeapon = pFoot->GetWeapon(i);
				if (!pWeapon || !pWeapon->WeaponType)
					continue;

				if (pFoot->GetFireError(pTarget, i, true) != FireError::ILLEGAL)
					return true;
			}

			return false;
		};

	// Searching for enemies in this group: GreatestThreat + excluding assigned/unreachable targets + accessibility pathfinding
	auto const selectGroupTarget = [&](const std::vector<FootClass*>& group, FootClass* const pLeader,
		const CoordStruct& groupCenter, bool agentMode) -> TechnoClass*
		{
			std::vector<TechnoClass*> unreachableTargets;

			while (true)
			{
				// Preferred: Exclude "Already assigned to other groups" and "Unavailable" targets
				std::vector<TechnoClass*> excludedTargets = assignedTargets;
				excludedTargets.insert(excludedTargets.end(), unreachableTargets.begin(), unreachableTargets.end());

				TechnoClass* candidate = ScriptExtData::GreatestThreat(pLeader, targetMask, threatMode,
					nullptr, attackAITargetType, -1, agentMode, &excludedTargets, &group, &groupCenter);

				// When the number of targets is less than the number of groups, it degenerates to allow shared targets (only unavailable ones are excluded), which is consistent with the original behavior.
				if (!candidate && !assignedTargets.empty())
				{
					candidate = ScriptExtData::GreatestThreat(pLeader, targetMask, threatMode,
						nullptr, attackAITargetType , -1, agentMode, &unreachableTargets, &group, &groupCenter);
				}

				if (!candidate)
					break; // Candidates exhausted: There are no more targets to hit.

				//At least one member of the group can actually attack the target, otherwise the target will be considered unavailable and removed.
				//(Align with original enemy detection GetFireError != ILLEGAL: Exclude surface-to-air/air-to-ground ballistics
				//Do not match targets that cannot be hit, so as to avoid locking the target to units that cannot hit it and causing hangup)
				bool hasShooter = false;
				for (auto pFoot : group)
				{
					if (CanEngageTarget(pFoot, candidate))
					{
						hasShooter = true;
						break;
					}
				}

				if (!hasShooter)
				{
					unreachableTargets.push_back(candidate); // The entire group cannot be defeated, so they will be eliminated and then re-selected.
					continue;
				}

				// There are already members of the group attacking the target -> It is considered reachable (in combat), no pathfinding check is required
				bool alreadyEngaging = false;
				for (auto pFoot : group)
				{
					if (pFoot->Target == candidate)
					{
						alreadyEngaging = true;
						break;
					}
				}

				if (alreadyEngaging || pLeader->UpdatePathfinding(candidate->GetMapCoords(), false, 0))
					return candidate;

				unreachableTargets.push_back(candidate); // Unreachable, eliminate and re-select
			}

			return nullptr;
		};

	//Align the original behavior: no periodic relocking after locking once, only when the group locks the target
	//Only when it expires (death/leaving the field/no longer hostile) will the enemy be searched again. In other cases, the original lock will remain unchanged.
	bool anyGroupActionable = false;

	for (auto& group : pExt->ScatterAttackGroups)
	{
		FootClass* const pLeader = group.front();

		// Group targets inherit the group member's current lock that is still in effect
		TechnoClass* pGroupTarget = nullptr;
		for (auto pFoot : group)
		{
			TechnoClass* const pCurrent = flag_cast_to<TechnoClass*>(pFoot->Target);
			if (pCurrent && pCurrent->Health > 0 && !pCurrent->InLimbo && pCurrent->IsOnMap
				&& !pFoot->Owner->IsAlliedWith(pCurrent->Owner))
			{
				pGroupTarget = pCurrent;
				break;
			}
		}

		// The group's target lock fails -> the group searches for the enemy again
		if (!pGroupTarget)
		{
			CoordStruct groupCenter;
			const bool agentMode = computeGroupContext(group, groupCenter);
			pGroupTarget = selectGroupTarget(group, pLeader, groupCenter, agentMode);
		}

		if (!pGroupTarget)
			continue; // This group cannot find an accessible target and maintains the status quo.

		anyGroupActionable = true;

		//All members in the group lock the same target (newly joined/disbanded members will re-lock to the group target here)
		for (auto pFoot : group)
		{
			//Members who cannot attack the target will stay on alert and will not lock the target.
			//(CanEngageTarget has covered the main/secondary weapon slots, no need to judge IsArmed anymore)
			if (!CanEngageTarget(pFoot, pGroupTarget))
			{
				if (pFoot->GetCurrentMission() != Mission::Guard)
					pFoot->QueueMission(Mission::Guard, false);
				continue;
			}

			//Already in the attack group target -> keep, no interruption
			if (pFoot->Target == pGroupTarget)
				continue;

			pFoot->SetTarget(pGroupTarget);
			pFoot->QueueMission(Mission::Attack, true);
		}

		//This group has locked the target, and subsequent groups will not select it first.
		assignedTargets.push_back(pGroupTarget);
	}

	//All groups cannot find reachable targets -> none of the enemies can reach them, marking is completed (clear the group, advance the script)
	if (!anyGroupActionable)
	{
		pExt->ScatterAttackGroups.clear();
		return;
	}

	//There are still combatable targets -> stay in this action until the last enemy is destroyed
	pTeam->StepCompleted = false;
}

//============================================================================
//Patrol system (action 5504~5507) assistance and implementation
//============================================================================
//Action Argument encoding (aligned to original action 47):
	//High 16 bits = building type index (0 = first type, such as GAPOWR; 0xFFFF = any type)
	//Low 16 bits = Search method: 0 smallest threat /1 largest threat /2 nearest /3 farthest
enum class TargetSelection : int
{
	MinimumThreat = 0,
	MaximumThreat = 1,
	Closest = 2,
	Farthest = 3,
};

//Find the target building of the designated camp and select it from the candidate set according to the enemy search method.
//typeIndex:BuildingTypeClass::Array index;0xFFFF = any type
//wantEnemy:true = Belongs to the enemy camp pointed to by HouseClass::EnemyHouseIndex;
//false = the team owner's own building (TeamClass::Owner)
static BuildingClass* FindTeamBuilding(TeamClass* pTeam, int typeIndex, TargetSelection selection, bool wantEnemy)
{
	if (!pTeam || !pTeam->OwnerHouse)
		return nullptr;

	BuildingTypeClass* pType = BuildingTypeClass::Array->get_or_default(typeIndex);
	if (!pType)
		return nullptr;

	HouseClass* const pOwner = pTeam->OwnerHouse;
	CoordStruct teamPos { 0, 0, 0 };
	if (pTeam->FirstUnit)
		teamPos = pTeam->FirstUnit->GetCoords();

	BuildingClass* best = nullptr;
	long long bestScore = 0;
	bool hasBest = false;

	for (BuildingClass* pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding || !pBuilding->IsAlive || pBuilding->InLimbo)
			continue;
		if (!pBuilding->Owner)
			continue;
		if (pType && pBuilding->GetType() != static_cast<ObjectTypeClass*>(pType))
			continue;

		const bool isOwn = (pBuilding->Owner == pOwner);
		const bool isEnemy = (pBuilding->Owner->ArrayIndex == pOwner->EnemyHouseIndex);
		if (wantEnemy ? !isEnemy : !isOwn)
			continue;

		long long score = 0;
		switch (selection)
		{
		case TargetSelection::MinimumThreat:
			score = -pBuilding->GetThreatValue();
			break;
		case TargetSelection::MaximumThreat:
			score = pBuilding->GetThreatValue();
			break;
		case TargetSelection::Closest:
		default:
		{
			const CoordStruct pos = pBuilding->GetCoords();
			const long long dx = pos.X - teamPos.X;
			const long long dy = pos.Y - teamPos.Y;
			score = -(dx * dx + dy * dy);
			break;
		}
		case TargetSelection::Farthest:
		{
			const CoordStruct pos = pBuilding->GetCoords();
			const long long dx = pos.X - teamPos.X;
			const long long dy = pos.Y - teamPos.Y;
			score = dx * dx + dy * dy;
			break;
		}
		}

		if (!hasBest || score > bestScore)
		{
			bestScore = score;
			best = pBuilding;
			hasBest = true;
		}
	}

	return best;
}

//Enemy camp: "Current enemy" pointed to by HouseClass::EnemyHouseIndex
//(Original HouseClass+0x5600;= -1 means there are currently no enemies)
static HouseClass* FindEnemyHouse(TeamClass* pTeam)
{
	if (!pTeam || !pTeam->OwnerHouse)
		return nullptr;

	const int enemyIdx = pTeam->OwnerHouse->EnemyHouseIndex;
	HouseClass* pHouse = HouseClass::Array->get_or_default(enemyIdx);
	if (!pHouse || pHouse->ArrayIndex != enemyIdx)
		return nullptr; //The index is inconsistent with the instance (defense)

	return pHouse;
}

static bool AllMembersInPosition(TeamClass* pTeam, CellClass* pTarget)
{
	if (!pTarget)
		return false;

	CoordStruct targetPos;
	pTarget->GetCoords(&targetPos);
	const CellStruct targetCell = CellClass::Coord2Cell(targetPos);

	const int tolerance = RulesClass::Instance->Stray;

	for (FootClass* pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
	{
		if (!pMember->IsAlive || pMember->Health <= 0 || pMember->InLimbo)
			continue;

		CoordStruct pos;
		pMember->GetCoords(&pos);
		const CellStruct memberCell = CellClass::Coord2Cell(pos);

		const int dx = memberCell.X - targetCell.X;
		const int dy = memberCell.Y - targetCell.Y;

		int tolerance2 = tolerance;
		if (pMember->WhatAmI() == AbstractType::Aircraft)
			tolerance2 *= 2;

		if (dx * dx + dy * dy > tolerance2 * tolerance2)
			return false;
	}

	return true;
}

//The main direction of from → to is quantified as a 4-way grid offset (±1); when the same grid is used, it defaults to north
static CellStruct CellDeltaToward(const CoordStruct& from, const CoordStruct& to)
{
	const int dx = to.X - from.X;
	const int dy = to.Y - from.Y;
	const int ax = dx < 0 ? -dx : dx;
	const int ay = dy < 0 ? -dy : dy;

	if (ax > ay)
		return CellStruct { static_cast<short>(dx > 0 ? 1 : -1), 0 };
	if (ay > 0)
		return CellStruct { 0, static_cast<short>(dy > 0 ? 1 : -1) };
	return CellStruct { 0, -1 };
}

//Cells near the building: the cell where the building is located + extrapolate the rangeCells cells toward the team.
static CellStruct CellNearBuilding(BuildingClass* pBuilding, const CoordStruct& teamPos, int rangeCells)
{
	const CellStruct buildingCell = pBuilding->GetMapCoords();
	if (rangeCells <= 0)
		return buildingCell;

	const CellStruct delta = CellDeltaToward(pBuilding->GetCoords(), teamPos);
	return CellStruct {
		static_cast<short>(buildingCell.X + delta.X * rangeCells),
		static_cast<short>(buildingCell.Y + delta.Y * rangeCells) };
}

static bool TryGetEnemyBaseCell(HouseClass* pEnemy, CellStruct& outCell)
{
	if (!pEnemy)
		return false;

	const CellStruct center = pEnemy->BaseCenter;
	if (center.X != 0 || center.Y != 0)
	{
		outCell = center;
		return true;
	}

	int sumX = 0, sumY = 0, count = 0;
	for (BuildingClass* pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding || pBuilding->Owner != pEnemy)
			continue;
		if (!pBuilding->IsAlive || pBuilding->InLimbo)
			continue;

		const CellStruct cell = pBuilding->GetMapCoords();
		sumX += cell.X;
		sumY += cell.Y;
		++count;
	}

	if (count == 0)
		return false;

	outCell = CellStruct { static_cast<short>(sumX / count), static_cast<short>(sumY / count) };
	return true;
}

bool ScriptExtData::PatrolAI(FakeTeamClass* pTeam, CellStruct targetCell, bool fresh)
{
	if (!pTeam)
		return false;

	CellClass* pTarget = MapClass::Instance->GetCellAt(targetCell);
	if (!pTarget || pTarget == MapClass::InvalidCell.operator->())
		return false;

	if (fresh)
		pTeam->_AssignMissionTarget(pTarget);

	if (!pTeam->ArchiveTarget)
		pTeam->_AssignMissionTarget(pTarget);

	const int dispatchPeriod = static_cast<int>(RulesClass::Instance->PatrolScan * 150.0);
	if (dispatchPeriod > 0 && Unsorted::CurrentFrame() % dispatchPeriod == 0)
	{
		FootClass* pChosen = nullptr;
		int bestScore = -1;

		for (FootClass* pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
		{
			if (!pMember->IsAlive || pMember->Health <= 0)
				continue;
			if (!Unsorted::ScenarioInit() && pMember->InLimbo)
				continue;

			//Original filter conditions: alive /with health /(scene initializing || non-limbo) /(enlisted || aircraft)
			const bool isAircraft = (pMember->WhatAmI() == AbstractType::Aircraft);
			if (!pMember->IsTeamLeader && !isAircraft)
				continue;

			const int score = pMember->GetTechnoType()->LeadershipRating;
			if (score > bestScore)
			{
				bestScore = score;
				pChosen = pMember;
			}
		}

		if (pChosen)
		{
			CoordStruct coords;
			pChosen->GetCoords(&coords);
			AbstractClass* pResult = pChosen->GreatestThreat(ThreatType::Range, &coords, false);
			if (pResult)
				pTeam->AssignMissionTarget(pResult);
			else if (pTeam->ArchiveTarget != pTarget)
				pTeam->_AssignMissionTarget(nullptr);
		}
	}
	if (flag_cast_to<TechnoClass*>(pTeam->ArchiveTarget))
	{
		pTeam->_Coordinate_Attack();
	}
	else if (AllMembersInPosition(pTeam, pTarget))
	{
		ScriptClass* const pScript = pTeam->CurrentScript;
		if (pScript && pScript->CurrentMission < pScript->Type->ActionsCount)
		{
			Debug::Log("[PT] completed, push to line %d\n", pScript->CurrentMission + 1);
			pScript->CurrentMission += 1;
			pTeam->_AssignMissionTarget(nullptr);
			pTeam->StepCompleted = false;
		}
	}
	else
	{
		pTeam->_CoordinateMove(); //Not in place: original driver members move/assemble
	}

	return true;
}

bool ScriptExtData::PatrolAIBuildingNearby(FakeTeamClass* pTeam, BuildingClass* pBuilding, int rangeCells, bool fresh)
{
	if (!pTeam || !pBuilding)
		return false;
	if (!pBuilding->IsAlive || pBuilding->InLimbo)
		return false;

	CoordStruct teamPos { 0, 0, 0 };
	if (pTeam->FirstUnit)
		teamPos = pTeam->FirstUnit->GetCoords();

	return PatrolAI(pTeam, CellNearBuilding(pBuilding, teamPos, rangeCells), fresh);
}

bool ScriptExtData::PatrolAIRally(FakeTeamClass* pTeam, HouseClass* pHouse, bool fresh)
{
	if (!pTeam)
		return false;

	CellStruct baseCell;
	if (!TryGetEnemyBaseCell(pHouse, baseCell))
		return false;
	return PatrolAI(pTeam, baseCell, fresh);
}

void ScriptExtData::PatrolToBuildingNearby(TeamClass* pTeam, int typeIndex, int selectionMode, bool fresh, bool wantEnemy)
{
	if (!pTeam->FirstUnit)
	{
		Debug::Log("[PT] skip(no member) type=%d sel=%d enemy=%d\n", typeIndex, selectionMode, wantEnemy);
		return; //No members → keep StepCompleted, skip this article
	}

	BuildingClass* pTarget = FindTeamBuilding(
		pTeam, typeIndex, static_cast<TargetSelection>(selectionMode & 0x3), wantEnemy);
	if (!pTarget)
	{
		Debug::Log("[PT] skip(no target) type=%d sel=%d enemy=%d EHI=%d\n",
			typeIndex, selectionMode, wantEnemy,
			pTeam->OwnerHouse ? pTeam->OwnerHouse->EnemyHouseIndex : -999);
		return; //No target building → Keep StepCompleted, skip this article
	}

	if (!PatrolAIBuildingNearby((FakeTeamClass*)pTeam, pTarget, 2, fresh))
	{
		Debug::Log("[PT] skip(AI fail) target=%s\n",
			pTarget->GetTechnoType()->get_ID());
		return; //The building is invalid /the target grid is invalid → keep StepCompleted and skip this article
	}

	if (fresh)
	{
		Debug::Log("[PT] patrolling to building %s (type=%d sel=%d enemy=%d)\n",
			static_cast<TechnoTypeClass*>(pTarget->GetType())->get_ID(),
			typeIndex, selectionMode, wantEnemy);
	}

	//Patrol in progress: stay in this action and wait for the completion of the position check.
	//Landing point = building grid + offset 2 grids toward the team (to avoid the target grid being occupied by the building body, resulting in pathfinding/abnormal branches)
	pTeam->StepCompleted = false;
}

void ScriptExtData::PatrolToRally(TeamClass* pTeam, bool fresh, bool wantEnemy)
{
	if (!pTeam->FirstUnit)
	{
		Debug::Log("[PT] skip(no member) rally enemy=%d\n", wantEnemy);
		return; //No members → keep StepCompleted, skip this article
	}

	HouseClass* pHouse = wantEnemy ? FindEnemyHouse(pTeam) : pTeam->OwnerHouse;
	if (!pHouse)
	{
		Debug::Log("[PT] skip(no house) rally enemy=%d EHI=%d\n",
			wantEnemy, pTeam->OwnerHouse ? pTeam->OwnerHouse->EnemyHouseIndex : -999);
		return; //There is no enemy camp (or our own side is invalid) → Keep StepCompleted and skip this article
	}

	if (!PatrolAIRally((FakeTeamClass*)pTeam, pHouse, fresh))
	{
		Debug::Log("[PT] skip(AI fail) rally house=%s\n", pHouse->get_ID());
		return; //The camp has no base → keep StepCompleted and skip this article
	}

	if (fresh)
	{
		Debug::Log("[PT] patrolling to rally house=%s base=(%d,%d)\n",
			pHouse->get_ID(), pHouse->BaseCenter.X, pHouse->BaseCenter.Y);
	}

	//Patrol in progress: stay in this action
	pTeam->StepCompleted = false;
}

void ScriptExtData::ApplyAttachEffect(TeamClass* pTeam, int nameIndex, int durationOverride)
{
	auto IsEligibleMember = [](TechnoClass* pTechno)
		{
			return pTechno
				&& pTechno->IsAlive
				&& pTechno->Health > 0
				&& pTechno->IsOnMap
				&& !pTechno->InLimbo
				&& !pTechno->Transporter;
		};

	auto pAE = PhobosAttachEffectTypeClass::FindFromIndexFix(nameIndex);
	if (!pAE) {
		pTeam->StepCompleted = true;
		return;
	}

	int attached = 0;
	AEAttachInfoTypeClass _Info { };

	_Info.AttachTypes.push_back(pAE);
	_Info.DurationOverrides.push_back(durationOverride);

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
		if (!IsEligibleMember(pUnit)) {
			continue;
		}

		attached += PhobosAttachEffectClass::Attach(pUnit,
			pTeam->OwnerHouse, nullptr, pTeam,&_Info,false , false);
	}

	Debug::Log("[AE] Script ApplyAttachEffect: team=[%s] effectKey=%d duration=%d -> attached=%d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>", nameIndex, durationOverride, attached);

	pTeam->StepCompleted = true;
}

void ScriptExtData::RemoveAttachEffect(TeamClass* pTeam, int nameIndex)
{
	auto IsEligibleMember = [](TechnoClass* pTechno)
		{
			return pTechno
				&& pTechno->IsAlive
				&& pTechno->Health > 0
				&& pTechno->IsOnMap
				&& !pTechno->InLimbo
				&& !pTechno->Transporter;
		};
	auto pAE = PhobosAttachEffectTypeClass::FindFromIndexFix(nameIndex);
	if (!pAE) {
		pTeam->StepCompleted = true;
		return;
	}

	int removed = 0;
	AEAttachInfoTypeClass _Info { };
	_Info.RemoveTypes.push_back(pAE);

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
		if (!IsEligibleMember(pUnit)) {
			continue;
		}

		removed += PhobosAttachEffectClass::Detach(pUnit,&_Info);
	}

	Debug::Log("[AE] Script RemoveAttachEffect: team=[%s] effectKey=%d -> removed=%d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>", nameIndex, removed);

	pTeam->StepCompleted = true;
}

void ScriptExtData::RemoveAllAttachEffects(TeamClass* pTeam)
{
	auto IsEligibleMember = [](TechnoClass* pTechno)
		{
			return pTechno
				&& pTechno->IsAlive
				&& pTechno->Health > 0
				&& pTechno->IsOnMap
				&& !pTechno->InLimbo
				&& !pTechno->Transporter;
		};

	int removed = 0;
	AEAttachInfoTypeClass _Info { };
	for (auto& _AllAE : PhobosAttachEffectTypeClass::Array) {
		_Info.RemoveTypes.push_back(_AllAE.get());
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
		if (!IsEligibleMember(pUnit)) {
			continue;
		}

		removed += PhobosAttachEffectClass::Detach(pUnit, &_Info);
	}

	Debug::Log("[AE] Script RemoveAllAttachEffects: team=[%s] -> removed=%d\n",
		pTeam->Type ? pTeam->Type->get_ID() : "<none>", removed);
	pTeam->StepCompleted = true;
}

void ScriptExtData::ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget)
{
	if (!pTeam || !pTeamLeader || !pTarget)
		return;

	//auto pScript = pTeam->CurrentScript;
	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperChronosphere = nullptr;
	SuperClass* pSuperChronowarp = nullptr;

	for (auto& pSuper : pOwner->Supers)
	{
		if (!SWTypeExtData::IsAvailable(pOwner, pSuper))
			continue;

		if (!pSuperChronosphere && pSuper->Type->Type == SuperWeaponType::ChronoSphere)
			pSuperChronosphere = pSuper;

		if (!pSuperChronowarp && pSuper->Type->Type == SuperWeaponType::ChronoWarp)
			pSuperChronowarp = pSuper;

		if (pSuperChronosphere && pSuperChronowarp)
			break;
	}

	//const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	//const auto& [nextAct, nextArgs] = pTeam->CurrentScript->GetNextAction();

	if (!pSuperChronosphere || !pSuperChronowarp)
	{
		pTeam->StepCompleted = true;
		return;
	}

	if (!pSuperChronosphere->IsCharged || (pSuperChronosphere->IsPowered() && !pOwner->Is_Powered()))
	{
		if (pSuperChronosphere->Granted)
		{
			int rechargeTime = pSuperChronosphere->GetRechargeTime();
			int timeLeft = pSuperChronosphere->RechargeTimer.GetTimeLeft();

			if ((1.0 - RulesClass::Instance->AIMinorSuperReadyPercent) < ((double)timeLeft / rechargeTime))
			{
				return;
			}
		}
		else
		{
			pTeam->StepCompleted = true;
			return;
		}
	}

	auto pTargetCell = MapClass::Instance->TryGetCellAt(pTarget->GetCoords());

	if (pTargetCell)
	{
		auto coords = pTeam->Zone->GetCoords();
		auto firing = CellClass::Coord2Cell(coords);
		((FakeHouseClass*)pOwner)->_FireSW(pSuperChronosphere->Type->ArrayIndex, firing);
		((FakeHouseClass*)pOwner)->_FireSW(pSuperChronowarp->Type->ArrayIndex, pTargetCell->MapCoords);
		pTeam->AssignMissionTarget(pTargetCell);
	}

	pTeam->StepCompleted = true;
	return;
}

void ScriptExtData::ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam->CurrentScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	//auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->Owner);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();

	if (mode < 0 || mode > 2)
		mode = curArgs;

	if (mode < -1 || mode > 2)
		mode = -1;

	HouseExtData::ForceOnlyTargetHouseEnemy(pTeam->OwnerHouse, mode);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::JumpBackToPreviousScript(TeamClass* pTeam)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (pTeamData->PreviousScript)
	{
		GameDelete<true, false>(pTeam->CurrentScript);
		pTeam->CurrentScript = GameCreate<ScriptClass>(pTeamData->PreviousScript);
		pTeam->CurrentScript->CurrentMission = -1;
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		auto pScript = pTeam->CurrentScript;
		auto const& [curAct, curArgs] = pScript->GetCurrentAction();

		Debug::LogInfo("DEBUG: [{}] [{}](line: {} = {},{}): Can't find the previous script! This script action must be used after PickRandomScript.",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			(int)curAct,
			curArgs
		);

		pTeam->StepCompleted = true;
		return;
	}
}

void ScriptExtData::SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	int scriptArgument = enable;

	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		const auto& [curAct, curArgs] = pScript->GetCurrentAction();
		scriptArgument = curArgs;
	}

	if (scriptArgument >= 1)
		pTeamData->AbortActionAfterKilling = true;
	else
		pTeamData->AbortActionAfterKilling = false;

	// This action finished
	pTeam->StepCompleted = true;
}

bool ScriptExtData::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno || !pTechno->Owner)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap && !pTechno->IsSinking && !pTechno->IsCrashing;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

std::pair<WeaponTypeClass*, WeaponTypeClass*> ScriptExtData::GetWeapon(TechnoClass* pTechno)
{
	if (!pTechno)
		return { nullptr , nullptr };

	return { TechnoExtData::GetCurrentWeapon(pTechno, false),TechnoExtData::GetCurrentWeapon(pTechno, true) };
}

void ScriptExtData::RepairDestroyedBridge(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	auto const pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
		return;

	auto pScript = pTeam->CurrentScript;
	auto const& [curAction, curArgument] = pTeam->CurrentScript->GetCurrentAction();
	auto const& [nextAction, nextArgument] = pTeam->CurrentScript->GetNextAction();

	// The first time this team runs this kind of script the repair huts list will updated. The only reason of why it isn't stored in ScenarioClass is because always exists the possibility of a modder to make destroyable Repair Huts
	if (pTeamData->BridgeRepairHuts.empty())
	{
		for (auto pBuilding : *BuildingClass::Array)
		{
			if (pBuilding->Type->BridgeRepairHut)
				pTeamData->BridgeRepairHuts.push_back(pBuilding);
		}

		if (pTeamData->BridgeRepairHuts.empty())
		{
			pTeam->StepCompleted = true;
			Debug::LogInfo("AI Scripts - [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} -> (Reason: No repair huts found)",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				(int)curAction,
				curArgument,
				pScript->CurrentMission + 1,
				(int)nextAction,
				nextArgument);

			return;
		}
	}

	// Reset Team's target if the current target isn't a repair hut
	if (pTeam->ArchiveTarget)
	{
		if (pTeam->ArchiveTarget->WhatAmI() != AbstractType::Building)
		{
			pTeam->ArchiveTarget = nullptr;
		}
		else
		{
			auto pBuilding = static_cast<BuildingClass*>(pTeam->ArchiveTarget);

			if (!pBuilding->Type->BridgeRepairHut)
			{
				pTeam->ArchiveTarget = nullptr;
			}
			else
			{
				CellStruct cell = pBuilding->GetCell()->MapCoords;

				// If the Bridge was repaired then the repair hut isn't valid anymore
				if (!MapClass::Instance->IsLinkedBridgeDestroyed(cell))
					pTeam->ArchiveTarget = nullptr;
			}
		}
	}

	TechnoClass* selectedTarget = pTeam->ArchiveTarget ? static_cast<TechnoClass*>(pTeam->ArchiveTarget) : nullptr;
	bool isEngineerAmphibious = false;
	StackVector<FootClass*, 512> engineers {};
	StackVector<FootClass*, 512> otherTeamMembers {};

	// Check if there are no engineers
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!IsUnitAvailable(pUnit, true))
			continue;

		if (!pTeam->ArchiveTarget)
		{
			pUnit->SetTarget(nullptr);
			pUnit->SetDestination(nullptr, false);
			pUnit->ForceMission(Mission::Guard);
		}

		if (pUnit->WhatAmI() == AbstractType::Infantry)
		{
			auto pInf = static_cast<InfantryClass*>(pUnit);

			if (pInf->IsEngineer())
			{
				if (pInf->Type->MovementZone == MovementZone::Amphibious
				|| pInf->Type->MovementZone == MovementZone::AmphibiousCrusher
				|| pInf->Type->MovementZone == MovementZone::AmphibiousDestroyer)
				{
					isEngineerAmphibious = true;
				}

				engineers->push_back(pUnit);

				continue;
			}
		}

		// Non-engineers will receive a different command
		otherTeamMembers->push_back(pUnit);
	}

	if (engineers->empty())
	{
		pTeam->StepCompleted = true;
		Debug::LogInfo("AI Scripts - [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} -> (Reason: Team has no engineers)",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			(int)curAction,
			curArgument,
			pScript->CurrentMission + 1,
			(int)nextAction,
			nextArgument);

		return;
	}

	StackVector<BuildingClass*, 10> validHuts {};

	if (!selectedTarget)
	{
		for (auto pTechno : pTeamData->BridgeRepairHuts)
		{
			CellStruct cell = pTechno->GetCell()->MapCoords;

			// Skip all huts linked to non-destroyed bridges
			if (!MapClass::Instance->IsLinkedBridgeDestroyed(cell))
				continue;

			if (isEngineerAmphibious)
			{
				validHuts->push_back(pTechno);
			}
			else
			{
				auto coords = pTechno->GetCenterCoords();

				// Only huts reachable by the (first) engineer are valid
				if (engineers[0]->IsInSameZone(&coords))
					validHuts->push_back(pTechno);
			}
		}

		// Find the best repair hut
		int bestVal = -1;

		//auto hut = pTechno;

		if (mode < 0)
			mode = curArgument;

		if (mode < 0)
		{
			// Pick a random bridge
			selectedTarget = validHuts[ScenarioClass::Instance->Random.RandomFromMax(validHuts->size() - 1)];
		}
		else
		{
			for (auto& pHut : validHuts.container())
			{
				int value = engineers[0]->DistanceFrom(pHut); // Note: distance is in leptons (*256)

				if (mode > 0)
				{
					// Pick the farthest target
					if (value >= bestVal || bestVal < 0)
					{
						bestVal = value;
						selectedTarget = pHut;
					}
				}
				else
				{
					// Pick the closest target
					if (value < bestVal || bestVal < 0)
					{
						bestVal = value;
						selectedTarget = pHut;
					}
				}
			}
		}
	}

	if (!selectedTarget)
	{
		pTeam->StepCompleted = true;

		//Debug::LogInfo("AI Scripts - [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} -> (Reason: Can not select a bridge repair hut)",
		//	pTeam->Type->ID,
		//	pScript->Type->ID,
		//	pScript->CurrentMission,
		//	(int)curAction,
		//	curArgument,
		//	pScript->CurrentMission + 1,
		//	(int)nextAction,
		//	nextArgument);

		return;
	}

	// Setting the team's target & mission
	pTeam->ArchiveTarget = selectedTarget;

	for (auto engineer : engineers.container())
	{
		if (engineer->Destination != selectedTarget)
		{
			engineer->SetTarget(selectedTarget);
			engineer->QueueMission(Mission::Capture, true);
		}
	}

	if (!otherTeamMembers->empty())
	{
		double closeEnough = 0.0; // Note: this value is in leptons (*256)
		if (pTeamData->CloseEnough > 0)
			closeEnough = pTeamData->CloseEnough * 256.0;
		else
			closeEnough = RulesClass::Instance->CloseEnough.value;

		for (auto pFoot : otherTeamMembers.container())
		{
			if (!pFoot->Destination
				|| (selectedTarget->DistanceFrom(pFoot->Destination) > closeEnough))
			{
				// Reset previous command
				pFoot->SetTarget(nullptr);
				pFoot->SetDestination(nullptr, false);
				pFoot->ForceMission(Mission::Guard);

				// Get a cell near the target
				pFoot->QueueMission(Mission::Move, false);
				CoordStruct coord = TechnoExtData::PassengerKickOutLocation(selectedTarget, pFoot);
				CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);
				pFoot->SetDestination(pCellDestination, true);
			}

			// Reached destination, stay in guard until next action
			if (pFoot->DistanceFrom(pFoot->Destination) < closeEnough)
				pFoot->QueueMission(Mission::Area_Guard, false);

			pFoot->NextMission();
		}
	}
}

// Works like the old Tiberian Sun ScriptType Action "24"
void ScriptExtData::PlaySpeech(TeamClass* pTeam)
{
	VoxClass::PlayIndex(pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	pTeam->StepCompleted = true;
}

void ScriptExtData::SimpleDeployerDeploy(TeamClass* pTeam, int mode)
{
	// mode: -1 = use argument, 0 = deploy, 1 = undeploy
	if (!pTeam)
	{
		Debug::Log("[SimpleDeployerDeploy] Error: pTeam is null\n");
		return;
	}

	auto pScript = pTeam->CurrentScript;
	if (!pScript)
	{
		Debug::Log("[SimpleDeployerDeploy] Error: pScript is null\n");
		pTeam->StepCompleted = true;
		return;
	}

	// Static optimization caches - declare at top of function
	static std::map<UnitClass*, std::pair<CellClass*, int>> cellSearchCache;
	static std::map<UnitClass*, int> unloadTimeoutCounter;
	static int frameCounter = 0;
	static int lastLogFrame = 0;
	frameCounter++;

	const auto& [curAct, scriptArgument] = pScript->GetCurrentAction();
	if (mode < 0)
		mode = scriptArgument;

	// Reduce logging frequency to avoid spam
	bool shouldLog = (frameCounter - lastLogFrame) > 30; // Log every second

	if (shouldLog)
	{
		lastLogFrame = frameCounter;
		Debug::Log("[SimpleDeployerDeploy] Team: %s, Mode: %d (Frame %d)\n",
			pTeam->Type ? pTeam->Type->get_ID() : "Unknown", mode, frameCounter);
	}

	// Clean up dead team members first
	FootClass* pCur = pTeam->FirstUnit;
	while (pCur)
	{
		FootClass* pNext = pCur->NextTeamMember;
		if (!ScriptExtData::IsUnitAvailable(pCur, false))
		{
			pTeam->RemoveMember(pCur, -1, 1);
		}
		pCur = pNext;
	}

	// Check if team has any units left
	if (!pTeam->FirstUnit)
	{
		Debug::Log("[SimpleDeployerDeploy] Error: No units in team\n");
		pTeam->StepCompleted = true;
		return;
	}

	bool allUnitsProcessed = true;
	int unitsFound = 0;
	int simpleDeployerUnits = 0;

	// Process each unit in the team
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		unitsFound++;

		if (!ScriptExtData::IsUnitAvailable(pUnit, false))
			continue;

		auto pSimpleUnit = cast_to<UnitClass*, false>(pUnit);
		if (!pSimpleUnit || !pSimpleUnit->Type->IsSimpleDeployer)
			continue;

		simpleDeployerUnits++;
#ifdef _DEBUG
		Debug::Log("[SimpleDeployerDeploy] Processing SimpleDeployer unit %d (%s), Deployed: %d\n",
			simpleDeployerUnits, pSimpleUnit->Type->get_ID(), pSimpleUnit->Deployed);
#endif

		bool shouldDeploy = false;
		bool shouldUndeploy = false;

		// Determine action based on mode
		switch (mode)
		{
		case 0: // Deploy
		default:
			if (!pSimpleUnit->Deployed)
				shouldDeploy = true;
			break;
		case 1: // Undeploy
			if (pSimpleUnit->Deployed)
				shouldUndeploy = true;
			break;
		}

		// Check if unit is already in deployment process
		if (pSimpleUnit->Deploying || pSimpleUnit->DeployAnim)
		{
			// Unit is busy, wait for it to complete
			allUnitsProcessed = false;
			continue;
		}

		// Special handling for Mission::Unload completion issues with timeout
		if (pSimpleUnit->CurrentMission == Mission::Unload)
		{
			// Initialize timeout counter if not exists
			if (unloadTimeoutCounter.find(pSimpleUnit) == unloadTimeoutCounter.end())
			{
				unloadTimeoutCounter[pSimpleUnit] = frameCounter;
			}

			// Check if Mission::Unload should complete for SimpleDeployer
			bool shouldComplete = (!pSimpleUnit->Deploying && !pSimpleUnit->DeployAnim);
			bool timedOut = (frameCounter - unloadTimeoutCounter[pSimpleUnit]) > 150; // ~5 seconds timeout

			if (shouldComplete || timedOut)
			{
				if (timedOut)
				{
					Debug::Log("[SimpleDeployerDeploy] Mission::Unload timeout for %s, forcing completion\n", pSimpleUnit->Type->get_ID());
				}
				else
				{
					Debug::Log("[SimpleDeployerDeploy] Force completing Mission::Unload for %s\n", pSimpleUnit->Type->get_ID());
				}
				pSimpleUnit->ForceMission(Mission::Guard);
				unloadTimeoutCounter.erase(pSimpleUnit); // Clean up timeout counter
			}
			else
			{
				// Still in unload process, wait
				allUnitsProcessed = false;
				continue;
			}
		}
		else
		{
			// Clean up timeout counter if unit is not in Mission::Unload
			unloadTimeoutCounter.erase(pSimpleUnit);
		}

		// Execute deployment/undeployment action
		if (shouldDeploy)
		{
			// Additional terrain checks before deployment
			bool canDeploy = pSimpleUnit->CanDeploySlashUnload();
			bool terrainSuitable = true;

			if (canDeploy)
			{
				auto pCell = pSimpleUnit->GetCell();
				if (pCell)
				{
					// Check terrain suitability for deployment
					bool cellClear = pCell->Cell_Occupier() == nullptr || pCell->Cell_Occupier() == pSimpleUnit;
					bool flatGround = pCell->GetLevel() == 0; // Level 0 = flat ground
					bool passableTerrain = pCell->CanThisExistHere(pSimpleUnit->Type->SpeedType, nullptr, pSimpleUnit->Owner);
					bool noBridge = !pCell->ContainsBridge();
					bool notRampCliff = !pCell->Tile_Is_Ramp() && !pCell->Tile_Is_Cliff();

					terrainSuitable = cellClear && flatGround && passableTerrain && noBridge && notRampCliff;

					Debug::Log("[SimpleDeployerDeploy] Terrain check for %s: Clear=%d, Flat=%d, Passable=%d, NoBridge=%d, NotRamp=%d\n",
						pSimpleUnit->Type->get_ID(), cellClear ? 1 : 0, flatGround ? 1 : 0,
						passableTerrain ? 1 : 0, noBridge ? 1 : 0, notRampCliff ? 1 : 0);
				}
			}

			Debug::Log("[SimpleDeployerDeploy] Unit %s shouldDeploy=true, canDeploy=%d, terrainSuitable=%d, deployed=%d\n",
				pSimpleUnit->Type->get_ID(), canDeploy ? 1 : 0, terrainSuitable ? 1 : 0, pSimpleUnit->Deployed ? 1 : 0);

			if (canDeploy && terrainSuitable)
			{
				// Try direct deployment first
				if (pSimpleUnit->TryToDeploy())
				{
					Debug::Log("[SimpleDeployerDeploy] TryToDeploy succeeded for %s\n", pSimpleUnit->Type->get_ID());
					allUnitsProcessed = false; // Wait for deployment animation
				}
				else
				{
					// Fallback to Mission::Unload (working method)
					Debug::Log("[SimpleDeployerDeploy] TryToDeploy failed, using Mission::Unload fallback for %s\n", pSimpleUnit->Type->get_ID());
					pSimpleUnit->QueueMission(Mission::Unload, false);
					pSimpleUnit->NextMission();
					allUnitsProcessed = false; // Wait for deployment to complete
				}
			}
			else if (canDeploy && !terrainSuitable)
			{
				// Check cache first to avoid expensive repeated searches
				CellClass* pBestCell = nullptr;
				auto currentCell = pSimpleUnit->GetMapCoords();

				// Check if we have a cached result that's still valid
				auto cacheIt = cellSearchCache.find(pSimpleUnit);
				if (cacheIt != cellSearchCache.end())
				{
					auto&[cachedCell, cacheFrame] = cacheIt->second;
					// Use cache if it's less than 30 frames old (~1 second)
					if (frameCounter - cacheFrame < 30 && cachedCell)
					{
						// Validate cached cell is still suitable
						bool cellClear = cachedCell->Cell_Occupier() == nullptr;
						if (cellClear)
						{
							pBestCell = cachedCell;
							Debug::Log("[SimpleDeployerDeploy] Using cached suitable cell for %s\n", pSimpleUnit->Type->get_ID());
						}
						else
						{
							// Cache invalid, remove it
							cellSearchCache.erase(cacheIt);
						}
					}
					else
					{
						// Cache expired, remove it
						cellSearchCache.erase(cacheIt);
					}
				}

				// If no valid cache, perform search
				if (!pBestCell)
				{
					Debug::Log("[SimpleDeployerDeploy] Current terrain not suitable, searching nearby cells for %s\n", pSimpleUnit->Type->get_ID());

					// Search in 3x3 area around current position
					for (int dx = -1; dx <= 1 && !pBestCell; dx++)
					{
						for (int dy = -1; dy <= 1 && !pBestCell; dy++)
						{
							if (dx == 0 && dy == 0) continue; // Skip current cell (already checked)

							CellStruct targetCell = { short(currentCell.X + dx), short(currentCell.Y + dy) };
							if (!MapClass::Instance->IsValidCell(targetCell)) continue;

							auto pNearbyCell = MapClass::Instance->GetCellAt(targetCell);
							if (!pNearbyCell) continue;

							// Check if nearby cell is suitable
							bool cellClear = pNearbyCell->Cell_Occupier() == nullptr;
							bool flatGround = pNearbyCell->GetLevel() == 0;
							bool passableTerrain = pNearbyCell->CanThisExistHere(pSimpleUnit->Type->SpeedType, nullptr, pSimpleUnit->Owner);
							bool noBridge = !pNearbyCell->ContainsBridge();
							bool notRampCliff = !pNearbyCell->Tile_Is_Ramp() && !pNearbyCell->Tile_Is_Cliff();

							if (cellClear && flatGround && passableTerrain && noBridge && notRampCliff)
							{
								pBestCell = pNearbyCell;
								// Cache the result
								cellSearchCache[pSimpleUnit] = std::make_pair(pBestCell, frameCounter);
								Debug::Log("[SimpleDeployerDeploy] Found suitable cell at [%d,%d] for %s\n",
									targetCell.X, targetCell.Y, pSimpleUnit->Type->get_ID());
							}
						}
					}
				}

				if (pBestCell)
				{
					// Move to suitable cell first, then deploy
					Debug::Log("[SimpleDeployerDeploy] Moving %s to suitable cell for deployment\n", pSimpleUnit->Type->get_ID());
					pSimpleUnit->SetDestination(pBestCell, true);
					pSimpleUnit->QueueMission(Mission::Move, false);
					pSimpleUnit->NextMission();
					allUnitsProcessed = false; // Wait for movement to complete
				}
				else
				{
					// No suitable cells found - force deploy at current location as fallback
					Debug::Log("[SimpleDeployerDeploy] No suitable cells found, forcing deployment at current location for %s\n", pSimpleUnit->Type->get_ID());
					pSimpleUnit->QueueMission(Mission::Unload, false);
					pSimpleUnit->NextMission();
					allUnitsProcessed = false; // Wait for deployment to complete
				}
			}
			else
			{
				Debug::Log("[SimpleDeployerDeploy] Cannot deploy %s (CanDeploySlashUnload=false)\n", pSimpleUnit->Type->get_ID());
			}
		}
		else if (shouldUndeploy)
		{
			// Try to undeploy using proper game method
			Debug::Log("[SimpleDeployerDeploy] Unit %s shouldUndeploy=true, deployed=%d\n",
				pSimpleUnit->Type->get_ID(), pSimpleUnit->Deployed ? 1 : 0);

			if (pSimpleUnit->Deployed)
			{
				pSimpleUnit->Undeploy();
				allUnitsProcessed = false; // Wait for undeployment to complete
			}
		}
		else
		{
			Debug::Log("[SimpleDeployerDeploy] Unit %s no action (deployed=%d, mode=%d)\n",
				pSimpleUnit->Type->get_ID(), pSimpleUnit->Deployed ? 1 : 0, mode);
		}
	}

	// Clean up cache entries for dead/invalid units every 300 frames (~10 seconds)
	if (frameCounter % 300 == 0)
	{
		for (auto it = cellSearchCache.begin(); it != cellSearchCache.end();)
		{
			if (!ScriptExtData::IsUnitAvailable(it->first, false))
			{
				it = cellSearchCache.erase(it);
			}
			else
			{
				++it;
			}
		}

		for (auto it = unloadTimeoutCounter.begin(); it != unloadTimeoutCounter.end();)
		{
			if (!ScriptExtData::IsUnitAvailable(it->first, false))
			{
				it = unloadTimeoutCounter.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	// Script action completes when all units have finished their deployment actions
	pTeam->StepCompleted = allUnitsProcessed;
}


ASMJIT_PATCH(0x6913F8, ScriptClass_CTOR, 0x5)
{
	GET(ScriptClass* const, pThis, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
		ScriptExtContainer::Instance.Allocate(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x691486, ScriptClass_DTOR, 0x6)
{
	GET(ScriptClass*, pThis, ESI);
	ScriptExtContainer::Instance.Remove(pThis);
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x691F06, ScriptClass_DTOR, 0x6)

HRESULT __stdcall FakeScriptClass::__Load(IStream* pStm)
{
	HRESULT hr = this->ScriptClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!ScriptExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0F8C, FakeScriptClass::__Load)

HRESULT __stdcall FakeScriptClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->ScriptClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!ScriptExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0F90, FakeScriptClass::__Save)

void ScriptExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type)
{
	if (bRemoved) {
		for (auto& pVFoot : ScatterAttackGroups) {
			pVFoot.remove((FootClass*)ptr);
		}
	}
}

void FakeScriptClass::__PointerExpired(AbstractClass* pAbstract, bool bremoved)
{
	ScriptExtContainer::Instance.Find(this)->
		InvalidatePointer(pAbstract, bremoved, AbstractType::None);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0FA0, FakeScriptClass::__PointerExpired)