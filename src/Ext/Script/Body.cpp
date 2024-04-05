#include "Body.h"

#include <Utilities/Cast.h>
#include <AITriggerTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/Team/Body.h>

/*
*	Scripts is a part of `TeamClass` that executed sequentally form `ScriptTypeClass`
*	Each script contains function that behave as it programmed
*/

ScriptActionNode NOINLINE ScriptExtData::GetSpecificAction(ScriptClass* pScript, int nIdx)
{
	if (nIdx == -1)
		return { -1 , 0 };

	if (nIdx < pScript->Type->ActionsCount)
		return pScript->Type->ScriptActions[nIdx];
	//	nIdx = pScript->Type->ActionsCount;
	//
	//auto const nIdxR = nIdx - ScriptTypeClass::MaxActions;
	//auto const pTypeExt = ScriptTypeExt::ExtMap.Find(pScript->Type);
	//
	//if (!pTypeExt->PhobosNode.empty() && nIdxR < (int)pTypeExt->PhobosNode.size()) {
	//	return pTypeExt->PhobosNode[nIdxR];
	//}
	//constexpr auto const nMax = ScriptTypeClass::MaxActions - 1;
	//return pScript->Type->ScriptActions[nMax];

	return { -1 , 0 };
}

// =============================
// container
ScriptExtContainer ScriptExtContainer::Instance;

#define stringify( name ) #name

NOINLINE const char* ToStrings(PhobosScripts from)
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
		return "ModifyHateHousesList1Randoms";
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
	default:
		return GameStrings::NoneStr();
	}
}

#include "Lua/Wrapper.h"

bool ScriptExtData::ProcessScriptActions(TeamClass* pTeam)
{
	auto const& [action, argument] = pTeam->CurrentScript->GetCurrentAction();

	//Debug::Log("[%s - %x] Executing[%s - %x] [%d - %d]\n",
	//pTeam->Owner->get_ID(),
	//pTeam->Owner,
	//pTeam->get_ID(),
	//pTeam, action ,
	//argument
	//);

	//only find stuffs on the range , reducing the load
	//if ((AresScripts)action >= AresScripts::count)
	{
		//Debug::Log("[%s - %x] Executing[%s - %x] [%d (%s) - %d]\n",
		//	pTeam->Owner->get_ID(),
		//	pTeam->Owner,
		//	pTeam->get_ID(),
		//	pTeam, action ,
		//	ToStrings((PhobosScripts)action), argument
		//);
		int Action = LuaBridge::GetAppropriateAction(int(action));

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
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			break;
		}
		//TODO
		case PhobosScripts::DistributedLoading:
		{
			//ScriptExtData::DistributedLoadOntoTransport(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::FollowFriendlyByGroup:
		{
			//ScriptExtData::FollowFriendlyByGroup(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::RallyUnitWithSameGroup:
		{
			//ScriptExtData::RallyUnitInMap(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			break;
		}
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
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			break;
		}
		case PhobosScripts::ChronoshiftToEnemyBase: //#1077
		{
			// Chronoshift to enemy base, argument is additional distance modifier
			ScriptExtData::ChronoshiftToEnemyBase(pTeam, argument);
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
			// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
			if (IsExtVariableAction(Action))
			{
				VariablesHandler(pTeam, static_cast<PhobosScripts>(Action), argument);
				break;
			}

			//dont prematurely finish the `Script` ,...
			//bailout the script if the `Action` already -1
			//this will free the Member and allow them to be recuited
			if ((TeamMissionType)Action == TeamMissionType::none || (TeamMissionType)Action >= TeamMissionType::count && (AresScripts)action >= AresScripts::count)
			{
				// Unknown action. This action finished
				pTeam->StepCompleted = true;
				auto const pAction = pTeam->CurrentScript->GetCurrentAction();
				Debug::Log("AI Scripts : [%x] Team [%s][%s]  ( %x CurrentScript %s / %s line %d): Unknown Script Action: %d\n",
					pTeam,
					pTeam->Type->ID,
					pTeam->Type->Name,

					pTeam->CurrentScript,
					pTeam->CurrentScript->Type->ID,
					pTeam->CurrentScript->Type->Name,
					pTeam->CurrentScript->CurrentMission,

					pAction.Action);
			}

			return false;
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

void ScriptExtData::LoadIntoTransports(TeamClass* pTeam)
{
	std::vector<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = pUnit->GetTechnoType();

		if (pType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pType->Passengers)
		{
			transports.push_back(pUnit);
		}
	}

	if (transports.empty())
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Now load units into transports
	for (auto pTransport : transports)
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto const pTransportType = pTransport->GetTechnoType();
			auto const pUnitType = pUnit->GetTechnoType();

			if (pTransport != pUnit
				&& pUnitType->WhatAmI() != AbstractType::AircraftType
				&& !pUnit->InLimbo && !pUnitType->ConsideredAircraft
				&& pUnit->Health > 0)
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					// If is still flying wait a bit more
					if (pTransport->IsInAir())
						break;

					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

						break;
					}
				}
			}
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			break;
	}

	auto const pExt = TeamExtContainer::Instance.Find(pTeam);

	if (pExt)
	{
		FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);
		pExt->TeamLeader = pLeaderUnit;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (ScriptExtData::IsUnitAvailable(pUnit, false))
		{
			const auto pType = pUnit->GetTechnoType();

			if (pType->Ammo > 0 && pUnit->Ammo < pType->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (auto const pAircraft = abstract_cast<AircraftClass*>(pUnit))
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
			pLeaderUnit = ScriptExtData::FindTheTeamLeader(pTeam);
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
				auto pTypeUnit = pUnit->GetTechnoType();

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				// Aircraft case
				if (pTypeUnit->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = static_cast<AircraftTypeClass*>(pUnit->GetTechnoType());

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
						pUnit->QueueMission(Mission::Move, false);
					}
				}
				else
				{
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
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
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

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

	TeamExtContainer::Instance.Find(pTeam)->WaitNoTargetAttempts = abs(attempts);

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

	const auto& scriptList = RulesExtData::Instance()->AIScriptsLists;
	if ((size_t)idxScriptsList < scriptList.size())
	{
		const auto& objectsList = scriptList[idxScriptsList];

		if (!objectsList.empty())
		{
			int IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

			ScriptTypeClass* pNewScript = objectsList[IdxSelectedObject];

			if (pNewScript->ActionsCount > 0)
			{
				TeamExtContainer::Instance.Find(pTeam)->PreviousScript =
					std::exchange(pTeam->CurrentScript, GameCreate<ScriptClass>(pNewScript));
				// Ready for jumping to the first line of the new script
				pTeam->CurrentScript->CurrentMission = -1;
				pTeam->StepCompleted = true;

				return;
			}
			else
			{
				pTeam->StepCompleted = true;
				ScriptExtData::Log("AI Scripts - PickRandomScript: [%s] Aborting Script change because [%s] has 0 Action scripts!\n",
					pTeam->Type->ID,
					pNewScript->ID
				);

				return;
			}
		}
	}

	pTeam->StepCompleted = true;
	ScriptExtData::Log("AI Scripts - PickRandomScript: [%s] [%s] Failed to change the Team Script with index [%d]!\n",
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
		pTeam->Focus = nullptr;
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
			&& !pUnit->BeingWarpedOut)
		{
			if (mode == 2)
			{
				// Default mode: all members in range
				if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

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
							pUnit->QueueMission(Mission::Move, false);

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
								pUnit->QueueMission(Mission::Move, false);

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
	//	ScriptExtData::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No team members alive)\n",
	//		pTeam->Type->ID,
	//		pTeam->CurrentScript->Type->ID,
	//		pTeam->CurrentScript->CurrentMission,
	//		curAct,
	//		curArg,
	//		pTeam->CurrentScript->CurrentMission + 1,
	//		nextAct,
	//		nextArg);
	//
	//	return;
	//}

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	int percentage = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (percentage <= successPercentage)
	{
		const auto& [curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();
		const auto& [nextAct, nextArg] = ScriptExtData::GetSpecificAction(pTeam->CurrentScript, pTeam->CurrentScript->CurrentMission + 2);

		ScriptExtData::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d = %d,%d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArg,
			pTeam->CurrentScript->CurrentMission + 2,
			nextAct,
			nextArg
		);

		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
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
	}
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

FootClass* ScriptExtData::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;

	if (!pTeam)
		return pLeaderUnit;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!IsUnitAvailable(pUnit, true) || !(pUnit->IsTeamLeader || pUnit->WhatAmI() == AbstractType::Aircraft))
			continue;

		// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
		int unitLeadershipRating = pUnit->GetTechnoType()->LeadershipRating;

		if (unitLeadershipRating > bestUnitLeadershipValue)
		{
			pLeaderUnit = pUnit;
			bestUnitLeadershipValue = unitLeadershipRating;
		}
	}

	return pLeaderUnit;
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
	ScriptExtData::Log("AI Scripts - SetForceJumpCountdown: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		curAct,
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
	ScriptExtData::Log("AI Scripts - StopForceJumpCountdown: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		curAct,
		curArgs
	);
}

void ScriptExtData::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	auto pScript = pTeam->CurrentScript;
	auto const pLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	const auto& [nextAct, nextArgs] = pTeam->CurrentScript->GetNextAction();

	char logText[1024];
	sprintf_s(logText, "AI Scripts - ChronoshiftToEnemyBase: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: %s)\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		curAct,
		curArgs,
		pScript->CurrentMission + 1,
		nextAct,
		nextArgs,
		"%s");

	if (!pLeader)
	{
		ScriptExtData::Log(logText, "No team leader found");
		pTeam->StepCompleted = true;
		return;
	}

	int houseIndex = pLeader->Owner->EnemyHouseIndex;
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array->Items[houseIndex] : nullptr;

	if (!pEnemy)
	{
		ScriptExtData::Log(logText, "No enemy house found");
		pTeam->StepCompleted = true;
		return;
	}

	auto const pTargetCell = HouseExtData::GetEnemyBaseGatherCell(pEnemy, pLeader->Owner, pLeader->GetCoords(), pLeader->GetTechnoType()->SpeedType, extraDistance);

	if (!pTargetCell)
	{
		ScriptExtData::Log(logText, "No target cell found");
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExtData::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

void ScriptExtData::ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget)
{
	if (!pTeam || !pTeamLeader || !pTarget)
		return;

	auto pScript = pTeam->CurrentScript;
	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperChronosphere = nullptr;
	SuperClass* pSuperChronowarp = nullptr;

	for (auto& pSuper : pOwner->Supers)
	{
		if (!pSuperChronosphere && pSuper->Type->Type == SuperWeaponType::ChronoSphere)
			pSuperChronosphere = pSuper;

		if (!pSuperChronowarp && pSuper->Type->Type == SuperWeaponType::ChronoWarp)
			pSuperChronowarp = pSuper;

		if (pSuperChronosphere && pSuperChronowarp)
			break;
	}

	char logTextBase[1024];
	char logTextJump[1024];
	char jump[256];

	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();
	const auto& [nextAct, nextArgs] = pTeam->CurrentScript->GetNextAction();

	sprintf_s(jump, "Jump to next line: %d = %d,%d -> (Reason: %s)",
		pScript->CurrentMission + 1,
		nextAct,
		nextArgs,
		"%s"
	);

	sprintf_s(logTextBase, "AI Scripts - ChronoshiftTeamToTarget: [%s] [%s] (line: %d = %d,%d) %s\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		curAct,
		curArgs,
		"%s"
	);

	sprintf_s(logTextJump, logTextBase, jump);

	if (!pSuperChronosphere || !pSuperChronowarp)
	{
		ScriptExtData::Log(logTextJump, "No Chronosphere or ChronoWarp superweapon found");
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
				ScriptExtData::Log(logTextBase, "Chronosphere superweapon charge not at AIMinorSuperReadyPercent yet, not jumping to next line yet");
				return;
			}
		}
		else
		{
			ScriptExtData::Log(logTextJump, "Chronosphere superweapon is not available");
			pTeam->StepCompleted = true;
			return;
		}
	}

	auto pTargetCell = MapClass::Instance->TryGetCellAt(pTarget->GetCoords());

	if (pTargetCell)
	{
		pOwner->Fire_SW(pSuperChronosphere->Type->ArrayIndex, pTeam->SpawnCell->MapCoords);
		pOwner->Fire_SW(pSuperChronowarp->Type->ArrayIndex, pTargetCell->MapCoords);
		pTeam->AssignMissionTarget(pTargetCell);
		ScriptExtData::Log(logTextJump, "Finished successfully");
	}
	else
	{
		ScriptExtData::Log(logTextJump, "No target cell found");
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

	auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->Owner);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();

	if (mode < 0 || mode > 2)
		mode = curArgs;

	if (mode < -1 || mode > 2)
		mode = -1;

	HouseExtData::ForceOnlyTargetHouseEnemy(pTeam->Owner, mode);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::JumpBackToPreviousScript(TeamClass* pTeam)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (pTeamData->PreviousScript)
	{
		pTeam->CurrentScript->AnnounceExpiredPointer(true);
		pTeam->CurrentScript = std::exchange(pTeamData->PreviousScript, nullptr);
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		auto pScript = pTeam->CurrentScript;
		auto const& [curAct, curArgs] = pScript->GetCurrentAction();

		Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Can't find the previous script! This script action must be used after PickRandomScript.\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			curAct,
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

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

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

void ScriptExtData::Log(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgs(pFormat, args);
	va_end(args);
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
			ScriptExtData::Log("AI Scripts - [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No repair huts found)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				curAction,
				curArgument,
				pScript->CurrentMission + 1,
				nextAction,
				nextArgument);

			return;
		}
	}

	// Reset Team's target if the current target isn't a repair hut
	if (pTeam->Focus)
	{
		if (pTeam->Focus->WhatAmI() != AbstractType::Building)
		{
			pTeam->Focus = nullptr;
		}
		else
		{
			auto pBuilding = static_cast<BuildingClass*>(pTeam->Focus);

			if (!pBuilding->Type->BridgeRepairHut)
			{
				pTeam->Focus = nullptr;
			}
			else
			{
				CellStruct cell = pBuilding->GetCell()->MapCoords;

				// If the Bridge was repaired then the repair hut isn't valid anymore
				if (!MapClass::Instance->IsLinkedBridgeDestroyed(cell))
					pTeam->Focus = nullptr;
			}
		}
	}

	TechnoClass* selectedTarget = pTeam->Focus ? static_cast<TechnoClass*>(pTeam->Focus) : nullptr;
	bool isEngineerAmphibious = false;
	std::vector<FootClass*> engineers;
	std::vector<FootClass*> otherTeamMembers;

	// Check if there are no engineers
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!IsUnitAvailable(pUnit, true))
			continue;

		if (!pTeam->Focus)
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

				engineers.push_back(pUnit);

				continue;
			}
		}

		// Non-engineers will receive a different command
		otherTeamMembers.push_back(pUnit);
	}

	if (engineers.empty())
	{
		pTeam->StepCompleted = true;
		ScriptExtData::Log("AI Scripts - [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: Team has no engineers)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			curAction,
			curArgument,
			pScript->CurrentMission + 1,
			nextAction,
			nextArgument);

		return;
	}

	std::vector<BuildingClass*> validHuts;

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
				validHuts.push_back(pTechno);
			}
			else
			{
				auto coords = pTechno->GetCenterCoords();

				// Only huts reachable by the (first) engineer are valid
				if (engineers[0]->IsInSameZone(&pTechno->GetCenterCoords()))
					validHuts.push_back(pTechno);
			}
		}

		// Find the best repair hut
		int bestVal = -1;

		for (auto pTechno : validHuts)
		{
			//auto hut = pTechno;

			if (mode < 0)
				mode = curArgument;

			if (mode < 0)
			{
				// Pick a random bridge
				selectedTarget = validHuts[ScenarioClass::Instance->Random.RandomFromMax(validHuts.size() - 1)];
				break;
			}
			else
			{
				for (auto pHut : validHuts)
				{
					if (mode > 0)
					{
						// Pick the farthest target
						int value = engineers[0]->DistanceFrom(pHut); // Note: distance is in leptons (*256)

						if (value >= bestVal || bestVal < 0)
						{
							bestVal = value;
							selectedTarget = pHut;
						}
					}
					else
					{
						// Pick the closest target
						int value = engineers[0]->DistanceFrom(pHut); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
						{
							bestVal = value;
							selectedTarget = pHut;
						}
					}
				}
			}
		}
	}

	validHuts.clear();

	if (!selectedTarget)
	{
		pTeam->StepCompleted = true;

		ScriptExtData::Log("AI Scripts - [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: Can not select a bridge repair hut)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			curAction,
			curArgument,
			pScript->CurrentMission + 1,
			nextAction,
			nextArgument);

		return;
	}

	// Setting the team's target & mission
	pTeam->Focus = selectedTarget;

	for (auto engineer : engineers)
	{
		if (engineer->Destination != selectedTarget)
		{
			engineer->SetTarget(selectedTarget);
			engineer->QueueMission(Mission::Capture, true);
		}
	}

	if (!otherTeamMembers.empty())
	{
		double closeEnough = 0.0; // Note: this value is in leptons (*256)
		if (pTeamData->CloseEnough > 0)
			closeEnough = pTeamData->CloseEnough * 256.0;
		else
			closeEnough = RulesClass::Instance->CloseEnough.value;

		for (auto pFoot : otherTeamMembers)
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
		}
	}
}
//
//DEFINE_HOOK(0x6913F8, ScriptClass_CTOR, 0x5)
//{
//	GET(ScriptClass* const, pThis, ESI);
//	ScriptExtData::ExtMap.FindOrAllocate(pThis);
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x691F06, ScriptClass_DTOR, 0x6)
//DEFINE_HOOK(0x691486, ScriptClass_DTOR, 0x6)
//{
//	GET(ScriptClass*, pThis, ESI);
//	ScriptExtData::ExtMap.Remove(pThis);
//	return 0x0;
//}
//
//
//DEFINE_HOOK_AGAIN(0x691690, ScriptClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x691630, ScriptClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(ScriptClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	ScriptExtData::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x69166F, ScriptClass_Load_Suffix, 0x9)
//{
//	GET(ScriptClass*, pThis, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Type);
//	TeamExtContainer::Instance.LoadStatic();
//
//	return 0x69167D;
//}
//
//DEFINE_HOOK(0x6916A4, ScriptClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT const, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//	{
//		TeamExtContainer::Instance.SaveStatic();
//		return 0x6916A8;
//	}
//
//	return 0x6916AA;
//}