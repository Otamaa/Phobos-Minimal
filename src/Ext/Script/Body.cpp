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

// =============================
// container
ScriptExt::ExtContainer ScriptExt::ExtMap;

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass") { }
ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessScriptActions(TeamClass* pTeam)
{
	auto const& [action, argument] = pTeam->CurrentScript->GetCurrentAction();

	//only find stuffs on the range , reducing the load
	if ((AresScripts)action >= AresScripts::count)
	{
		switch ((PhobosScripts)action)
		{
		case PhobosScripts::TimedAreaGuard:
		{
			ScriptExt::ExecuteTimedAreaGuardAction(pTeam); //checked
			return;
		}
		case PhobosScripts::LoadIntoTransports:
		{
			ScriptExt::LoadIntoTransports(pTeam); //fixed
			return;
		}
		case PhobosScripts::WaitUntilFullAmmo:
		{
			ScriptExt::WaitUntilFullAmmoAction(pTeam); //
			return;
		}

#pragma region Mission_Attack
		case PhobosScripts::RepeatAttackCloserThreat:
		{
			// Threats that are close have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack(pTeam, true, 0, -1, -1); //done
			return;
		}
		case PhobosScripts::RepeatAttackFartherThreat:
		{
			// Threats that are far have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack(pTeam, true, 1, -1, -1); //done
			return;
		}
		case PhobosScripts::RepeatAttackCloser:
		{
			// Closer targets from Team Leader have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack(pTeam, true, 2, -1, -1); //done
			return;
		}
		case PhobosScripts::RepeatAttackFarther:
		{
			// Farther targets from Team Leader have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack(pTeam, true, 3, -1, -1); //done
			return;
		}
		case PhobosScripts::SingleAttackCloserThreat:
		{
			// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack(pTeam, false, 0, -1, -1); //done
			return;
		}
		case PhobosScripts::SingleAttackFartherThreat:
		{
			// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack(pTeam, false, 1, -1, -1); //done
			return;
		}
		case PhobosScripts::SingleAttackCloser:
		{
			// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack(pTeam, false, 2, -1, -1); //done
			return;
		}
		case PhobosScripts::SingleAttackFarther:
		{
			// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack(pTeam, false, 3, -1, -1); //done
			return;
		}
#pragma endregion

		case PhobosScripts::DecreaseCurrentAITriggerWeight:
		{
			ScriptExt::DecreaseCurrentTriggerWeight(pTeam, true, 0); //
			return;
		}
		case PhobosScripts::IncreaseCurrentAITriggerWeight:
		{
			ScriptExt::IncreaseCurrentTriggerWeight(pTeam, true, 0);
			return;
		}

#pragma region Mission_Attack_List
		case PhobosScripts::RepeatAttackTypeCloserThreat:
		{
			// Threats specific targets that are close have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack_List(pTeam, true, 0, -1);
			return;
		}
		case PhobosScripts::RepeatAttackTypeFartherThreat:
		{
			// Threats specific targets that are far have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack_List(pTeam, true, 1, -1);
			return;
		}
		case PhobosScripts::RepeatAttackTypeCloser:
		{
			// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack_List(pTeam, true, 2, -1);
			return;
		}
		case PhobosScripts::RepeatAttackTypeFarther:
		{
			// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
			ScriptExt::Mission_Attack_List(pTeam, true, 3, -1);
			return;
		}
		case PhobosScripts::SingleAttackTypeCloserThreat:
		{
			// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack_List(pTeam, false, 0, -1);
			return;
		}
		case PhobosScripts::SingleAttackTypeFartherThreat:
		{
			// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack_List(pTeam, false, 1, -1);
			return;
		}
		case PhobosScripts::SingleAttackTypeCloser:
		{
			// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack_List(pTeam, false, 2, -1);
			return;
		}
		case PhobosScripts::SingleAttackTypeFarther:
		{
			// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
			ScriptExt::Mission_Attack_List(pTeam, false, 3, -1);
			return;
		}
#pragma endregion

		case PhobosScripts::WaitIfNoTarget:
		{
			ScriptExt::WaitIfNoTarget(pTeam, -1);
			return;
		}
		case PhobosScripts::TeamWeightReward:
		{
			ScriptExt::TeamWeightReward(pTeam , -1);
			return;
		}
		case PhobosScripts::PickRandomScript:
		{
			ScriptExt::PickRandomScript(pTeam, -1);
			return;
		}

#pragma region Mission_Move
		case PhobosScripts::MoveToEnemyCloser:
		{
			// Move to the closest enemy target
			ScriptExt::Mission_Move(pTeam, 2, false, -1, -1);
			return;
		}
		case PhobosScripts::MoveToEnemyFarther:
		{
			// Move to the farther enemy target
			ScriptExt::Mission_Move(pTeam, 3, false, -1, -1);
			return;
		}
		case PhobosScripts::MoveToFriendlyCloser:
		{
			// Move to the closest friendly target
			ScriptExt::Mission_Move(pTeam, 2, true, -1, -1);
			return;
		}
		case PhobosScripts::MoveToFriendlyFarther:
		{
			// Move to the farther friendly target
			ScriptExt::Mission_Move(pTeam, 3, true, -1, -1);
			return;
		}
#pragma endregion

#pragma region Mission_Move_List
		case PhobosScripts::MoveToTypeEnemyCloser:
		{
			// Move to the closest specific enemy target
			ScriptExt::Mission_Move_List(pTeam, 2, false, -1);
			return;
		}
		case PhobosScripts::MoveToTypeEnemyFarther:
		{
			// Move to the farther specific enemy target
			ScriptExt::Mission_Move_List(pTeam, 3, false, -1);
			return;
		}
		case PhobosScripts::MoveToTypeFriendlyCloser:
		{
			// Move to the closest specific friendly target
			ScriptExt::Mission_Move_List(pTeam, 2, true, -1);
			return;
		}
		case PhobosScripts::MoveToTypeFriendlyFarther:
		{
			// Move to the farther specific friendly target
			ScriptExt::Mission_Move_List(pTeam, 3, true, -1);
			return;
		}
#pragma endregion

		case PhobosScripts::ModifyTargetDistance:
		{
			// AISafeDistance equivalent for Mission_Move()
			ScriptExt::SetCloseEnoughDistance(pTeam , -1);
			return;
		}

#pragma region Mission_Attack_List1Random
		case PhobosScripts::RandomAttackTypeCloser:
		{
			// Pick 1 closer random objective from specific list for attacking it
			ScriptExt::Mission_Attack_List1Random(pTeam, true, 2, -1);
			return;
		}
		case PhobosScripts::RandomAttackTypeFarther:
		{
			// Pick 1 farther random objective from specific list for attacking it
			ScriptExt::Mission_Attack_List1Random(pTeam, true, 3, -1);
			return;
		}
#pragma endregion

#pragma region Mission_Move_List1Random
		case PhobosScripts::RandomMoveToTypeEnemyCloser:
		{
			// Pick 1 closer enemy random objective from specific list for moving to it
			ScriptExt::Mission_Move_List1Random(pTeam, 2, false, -1, -1);
			return;
		}
		case PhobosScripts::RandomMoveToTypeEnemyFarther:
		{
			// Pick 1 farther enemy random objective from specific list for moving to it
			ScriptExt::Mission_Move_List1Random(pTeam, 3, false, -1, -1);
			return;
		}
		case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		{
			// Pick 1 closer friendly random objective from specific list for moving to it
			ScriptExt::Mission_Move_List1Random(pTeam, 2, true, -1, -1);
			return;
		}
		case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		{
			// Pick 1 farther friendly random objective from specific list for moving to it
			ScriptExt::Mission_Move_List1Random(pTeam, 3, true, -1, -1);
			return;
		}
#pragma endregion

		case PhobosScripts::SetMoveMissionEndMode:
		{
			// Set the condition for ending the Mission_Move Actions.
			ScriptExt::SetMoveMissionEndMode(pTeam, -1);
			return;
		}
		case PhobosScripts::UnregisterGreatSuccess:
		{
			// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
			ScriptExt::UnregisterGreatSuccess(pTeam);
			return;
		}
		case PhobosScripts::GatherAroundLeader:
		{
			ScriptExt::Mission_Gather_NearTheLeader(pTeam, -1);
			return;
		}
		case PhobosScripts::RandomSkipNextAction:
		{
			ScriptExt::SkipNextAction(pTeam, -1);
			return;
		}

#pragma region Stop_ForceJump_Countdown
		case PhobosScripts::StopForceJumpCountdown:
		{
			// Stop Timed Jump
			ScriptExt::Stop_ForceJump_Countdown(pTeam);
			return;
		}
		case PhobosScripts::NextLineForceJumpCountdown:
		{
			// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
			ScriptExt::Set_ForceJump_Countdown(pTeam, false, -1);
			return;
		}
		case PhobosScripts::SameLineForceJumpCountdown:
		{
			// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
			ScriptExt::Set_ForceJump_Countdown(pTeam, true, -1);
			return;
		}
#pragma endregion


		case PhobosScripts::ForceGlobalOnlyTargetHouseEnemy:
		{
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n",pTeam,pTeam->get_ID() , pTeam->CurrentScript, pTeam->CurrentScript->get_ID() , action);
			pTeam->StepCompleted = true;
			//ScriptExt::ForceGlobalOnlyTargetHouseEnemy(pTeam, -1);
			return;
		}
		case PhobosScripts::ChangeTeamGroup:
		{
		//	ScriptExt::TeamMemberSetGroup(pTeam, argument);
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}

		//TODO
		case PhobosScripts::DistributedLoading:
		{
			//ScriptExt::DistributedLoadOntoTransport(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}

		case PhobosScripts::FollowFriendlyByGroup:
		{
			//ScriptExt::FollowFriendlyByGroup(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::RallyUnitWithSameGroup:
		{
			//ScriptExt::RallyUnitInMap(pTeam, argument); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::SetHouseAngerModifier:
		{
			//ScriptExt::SetHouseAngerModifier(pTeam, 0); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::OverrideOnlyTargetHouseEnemy:
		{
			//ScriptExt::OverrideOnlyTargetHouseEnemy(pTeam, -1); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ModifyHateHouseIndex:
		{
			//ScriptExt::ModifyHateHouse_Index(pTeam, -1); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ModifyHateHousesList:
		{
			//ScriptExt::ModifyHateHouses_List(pTeam, -1); //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ModifyHateHousesList1Random:
		{
			//ScriptExt::ModifyHateHouses_List1Random(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}

#pragma region SetTheMostHatedHouse
		case PhobosScripts::SetTheMostHatedHouseMinorNoRandom:
		{
			// <, no random
			//ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, false);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::SetTheMostHatedHouseMajorNoRandom:
		{
			// >, no random
		//	ScriptExt::SetTheMostHatedHouse(pTeam, 0, 1, false);
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::SetTheMostHatedHouseRandom:
		{
			// random
			//ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, true);
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
#pragma endregion

#pragma region AngerNodesMod
		case PhobosScripts::ResetAngerAgainstHouses:
		{
			//ScriptExt::ResetAngerAgainstHouses(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::AggroHouse:
		{
			//ScriptExt::AggroHouse(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::AbortActionAfterSuccessKill:
		{
			//ScriptExt::SetAbortActionAfterSuccessKill(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
#pragma endregion

#pragma region ConditionalJump
		case PhobosScripts::ConditionalJumpSetCounter:
		{
			//ScriptExt::ConditionalJump_SetCounter(pTeam, -100000000);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpSetComparatorMode:
		{
			//ScriptExt::ConditionalJump_SetComparatorMode(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpSetComparatorValue:
		{
			//ScriptExt::ConditionalJump_SetComparatorValue(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpSetIndex:
		{
			//ScriptExt::ConditionalJump_SetIndex(pTeam, -1000000);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpResetVariables:
		{
			//ScriptExt::ConditionalJump_ResetVariables(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpIfFalse:
		{
			//ScriptExt::ConditionalJumpIfFalse(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpIfTrue:
		{
			//ScriptExt::ConditionalJumpIfTrue(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpManageKillsCounter:
		{
			//ScriptExt::ConditionalJump_ManageKillsCounter(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpCheckAliveHumans:
		{
			//ScriptExt::ConditionalJump_CheckAliveHumans(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpCheckHumanIsMostHated:
		{
			//ScriptExt::ConditionalJump_CheckHumanIsMostHated(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpKillEvaluation:
		{
			//ScriptExt::ConditionalJump_KillEvaluation(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpCheckObjects:
		{
			//ScriptExt::ConditionalJump_CheckObjects(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpCheckCount:
		{
			//ScriptExt::ConditionalJump_CheckCount(pTeam, 0);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ConditionalJumpManageResetIfJump:
		{
			//ScriptExt::ConditionalJump_ManageResetIfJump(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
#pragma endregion

		case PhobosScripts::JumpBackToPreviousScript:
		{
			//ScriptExt::JumpBackToPreviousScript(pTeam);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::SetSideIdxForManagingTriggers:
		{
			//ScriptExt::SetSideIdxForManagingTriggers(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::SetHouseIdxForManagingTriggers:
		{
			//ScriptExt::SetHouseIdxForManagingTriggers(pTeam, 1000000);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::ManageAllAITriggers:
		{
			//ScriptExt::ManageAITriggers(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::EnableTriggersFromList:
		{
			//ScriptExt::ManageTriggersFromList(pTeam, -1, true);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::DisableTriggersFromList:
		{
			//ScriptExt::ManageTriggersFromList(pTeam, -1, false);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::EnableTriggersWithObjects:
		{
			//ScriptExt::ManageTriggersWithObjects(pTeam, -1, true);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::DisableTriggersWithObjects:
		{
			//ScriptExt::ManageTriggersWithObjects(pTeam, -1, false);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}
		case PhobosScripts::RepairDestroyedBridge:
		{
			// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
			//ScriptExt::RepairDestroyedBridge(pTeam, -1);  //which branch is this again ?
			Debug::Log("Team[%x - %s , Script [%x - %s] Action [%d] - No AttachedFunction\n", pTeam, pTeam->get_ID(), pTeam->CurrentScript, pTeam->CurrentScript->get_ID(), action);
			pTeam->StepCompleted = true;
			return;
		}

		case PhobosScripts::ChronoshiftToEnemyBase: //#1077
		{
			// Chronoshift to enemy base, argument is additional distance modifier
			ScriptExt::ChronoshiftToEnemyBase(pTeam, argument);
			return;
		}
		}

		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (IsExtVariableAction((int)action))
		{
			VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
			return;
		}

		// Unknown new action. This action finished
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
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto const pScript = pTeam->CurrentScript;
	auto const pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			pUnit->QueueMission(Mission::Area_Guard, true);

		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->CurrentMission].Argument);
	}

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
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
			transports.emplace_back(pUnit);
		}
	}

	if (transports.size() == 0)
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
						return;

					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

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

	auto const pExt = TeamExt::ExtMap.Find(pTeam);

	if (pExt)
	{
		FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);
		pExt->TeamLeader = pLeaderUnit;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->InLimbo && pUnit->Health > 0)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
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

						return;
					}
				}
				else if (pUnit->GetTechnoType()->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown = -1)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;
	bool gatherUnits = false;
	auto const pExt = TeamExt::ExtMap.Find(pTeam);

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

		if (!ScriptExt::IsUnitAvailable(pLeaderUnit, true))
		{
			pLeaderUnit = ScriptExt::FindTheTeamLeader(pTeam);
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
			closeEnough = pExt->CloseEnough;
			pExt->CloseEnough = -1; // This a one-time-use value
		}
		else
		{
			closeEnough = RulesClass::Instance->CloseEnough.ToCell();
		}

		// The leader should stay calm & be the group's center
		if (pLeaderUnit->Locomotor->Is_Moving_Now())
			pLeaderUnit->SetDestination(nullptr, false);

		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!ScriptExt::IsUnitAvailable(pUnit, true))
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

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;

	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

		if (pTeamType
			&& ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType)
				|| (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->GetItem(i);
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

void ScriptExt::WaitIfNoTarget(TeamClass* pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (attempts <= 0)
			pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
		else
			pTeamData->WaitNoTargetAttempts = attempts;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass* pTeam, double award = 0)
{
	if (award <= 0)
		award = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (award > 0)
			pTeamData->NextSuccessWeightAward = award;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
{
	if (idxScriptsList <= 0)
		idxScriptsList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	bool changeFailed = true;

	if (idxScriptsList >= 0)
	{
		if ((size_t)idxScriptsList < RulesExt::Global()->AIScriptsLists.size())
		{
			const auto& objectsList = RulesExt::Global()->AIScriptsLists[idxScriptsList];

			if (!objectsList.empty())
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

				ScriptTypeClass* pNewScript = objectsList[IdxSelectedObject];

				if (pNewScript->ActionsCount > 0)
				{
					changeFailed = false;
					pTeam->CurrentScript = nullptr;
					pTeam->CurrentScript = GameCreate<ScriptClass>(pNewScript);

					// Ready for jumping to the first line of the new script
					pTeam->CurrentScript->CurrentMission = -1;
					pTeam->StepCompleted = true;

					return;
				}
				else
				{
					pTeam->StepCompleted = true;
					ScriptExt::Log("AI Scripts - PickRandomScript: [%s] Aborting Script change because [%s] has 0 Action scripts!\n", pTeam->Type->ID, pNewScript->ID);

					return;
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - PickRandomScript: [%s] [%s] Failed to change the Team Script with a random one!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass* pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData) {
		pTeam->StepCompleted = true;
		return;
	}

	if (distance > 0)
		pTeamData->CloseEnough = distance;


	if (distance <= 0)
		pTeamData->CloseEnough = RulesClass::Instance->CloseEnough.ToCell();

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, int mode = 0)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (auto const pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		if (mode >= 0 && mode <= 2)
			pTeamData->MoveMissionEndMode = mode;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader = nullptr, int mode = 0)
{
	if (!pTeam || !pFocus || mode < 0)
		return false;

	if (mode != 2 && mode != 1 && !pLeader)
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough.ToCell();

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData && pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction = false;

	if (mode == 2)
		bForceNextAction = true;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (ScriptExt::IsUnitAvailable(pUnit, true)
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
					if (pLeader)
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

void ScriptExt::SkipNextAction(TeamClass* pTeam, int successPercentage = 0)
{
	// This team has no units! END
	if (!pTeam->FirstUnit)
	{
		// This action finished
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No team members alive)\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument,
			pTeam->CurrentScript->CurrentMission + 1, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Action,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Argument);

		return;
	}

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	int percentage = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (percentage <= successPercentage)
	{
		ScriptExt::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d = %d,%d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, pTeam->CurrentScript->CurrentMission + 2,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Argument);

		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
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
void ScriptExt::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	auto itr = ScenarioExt::GetVariables(IsGlobal)->find(nVariable);

	if (itr != ScenarioExt::GetVariables(IsGlobal)->end())
	{
		itr->second.Value = _Pr()(itr->second.Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}

	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExt::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	auto itr = ScenarioExt::GetVariables(IsGlobal)->find(nVarToOperate);

	if (itr != ScenarioExt::GetVariables(IsGlobal)->end())
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	const auto pTech = pTeam->FetchLeader();

	if(pTech && Is_Techno(pTech))
		return pTech;

	return nullptr;
}

bool ScriptExt::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine = false, int count = 0)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (count <= 0)
		count = 15 * pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

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
		pTeamData->ForceJump_Countdown = -1;
		pTeamData->ForceJump_RepeatMode = false;
	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - SetForceJumpCountdown: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_Countdown = -1;
	pTeamData->ForceJump_RepeatMode = false;

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - StopForceJumpCountdown: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);
}

void ScriptExt::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	auto pScript = pTeam->CurrentScript;
	auto const pLeader = ScriptExt::FindTheTeamLeader(pTeam);

	char logText[1024];
	sprintf_s(logText, "AI Scripts - ChronoshiftToEnemyBase: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: %s)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, "%s");

	if (!pLeader)
	{
		ScriptExt::Log(logText, "No team leader found");
		pTeam->StepCompleted = true;
		return;
	}

	int houseIndex = pLeader->Owner->EnemyHouseIndex;
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array->GetItem(houseIndex) : nullptr;

	if (!pEnemy)
	{
		ScriptExt::Log(logText, "No enemy house found");
		pTeam->StepCompleted = true;
		return;
	}

	auto const pTargetCell = HouseExt::GetEnemyBaseGatherCell(pEnemy, pLeader->Owner, pLeader->GetCoords(), pLeader->GetTechnoType()->SpeedType, extraDistance);

	if (!pTargetCell)
	{
		ScriptExt::Log(logText, "No target cell found");
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExt::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

void ScriptExt::ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget)
{
	if (!pTeam || !pTeamLeader || !pTarget)
		return;

	auto pScript = pTeam->CurrentScript;
	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperChronosphere = nullptr;
	SuperClass* pSuperChronowarp = nullptr;

	for (auto const pSuper : pOwner->Supers)
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

	sprintf_s(jump, "Jump to next line: %d = %d,%d -> (Reason: %s)", pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, "%s");
	sprintf_s(logTextBase, "AI Scripts - ChronoshiftTeamToTarget: [%s] [%s] (line: %d = %d,%d) %s\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, "%s");
	sprintf_s(logTextJump, logTextBase, jump);

	if (!pSuperChronosphere || !pSuperChronowarp)
	{
		ScriptExt::Log(logTextJump, "No Chronosphere or ChronoWarp superweapon found");
		pTeam->StepCompleted = true;
		return;
	}

	if (!pSuperChronosphere->IsCharged || (pSuperChronosphere->IsPowered() && !pOwner->Is_Powered()))
	{
		if (pSuperChronosphere->Granted)
		{
			int rechargeTime = pSuperChronosphere->GetRechargeTime();
			int timeLeft = pSuperChronosphere->RechargeTimer.GetTimeLeft();

			if (1.0 - RulesClass::Instance->AIMinorSuperReadyPercent < timeLeft / rechargeTime)
			{
				ScriptExt::Log(logTextBase, "Chronosphere superweapon charge not at AIMinorSuperReadyPercent yet, not jumping to next line yet");
				return;
			}
		}
		else
		{
			ScriptExt::Log(logTextJump, "Chronosphere superweapon is not available");
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
		ScriptExt::Log(logTextJump, "Finished successfully");
	}
	else
	{
		ScriptExt::Log(logTextJump, "No target cell found");
	}

	pTeam->StepCompleted = true;
	return;
}

bool ScriptExt::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno || !Is_Techno(pTechno))
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

std::pair<WeaponTypeClass*, WeaponTypeClass*> ScriptExt::GetWeapon(TechnoClass* pTechno)
{
	if (!pTechno)
		return { nullptr , nullptr };

	return { TechnoExt::GetCurrentWeapon(pTechno, false),TechnoExt::GetCurrentWeapon(pTechno, true) };
}

void ScriptExt::Log(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgs(pFormat, args);
	va_end(args);
}

//
//DEFINE_HOOK(0x6913F8, ScriptClass_CTOR, 0x5)
//{
//	GET(ScriptClass* const, pThis, ESI);
//	ScriptExt::ExtMap.FindOrAllocate(pThis);
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x691F06, ScriptClass_DTOR, 0x6)
//DEFINE_HOOK(0x691486, ScriptClass_DTOR, 0x6)
//{
//	GET(ScriptClass*, pThis, ESI);
//	ScriptExt::ExtMap.Remove(pThis);
//	return 0x0;
//}
//
//
//DEFINE_HOOK_AGAIN(0x691690, ScriptClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x691630, ScriptClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(ScriptClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	ScriptExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x69166F, ScriptClass_Load_Suffix, 0x9)
//{
//	GET(ScriptClass*, pThis, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Type);
//	TeamExt::ExtMap.LoadStatic();
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
//		TeamExt::ExtMap.SaveStatic();
//		return 0x6916A8;
//	}
//
//	return 0x6916AA;
//}