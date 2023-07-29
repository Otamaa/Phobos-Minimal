#include "Body.h"

#include <Ext/Script/Body.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x6E9443, TeamClass_AI, 0x8)
{
	GET(TeamClass*, pTeam, ESI);

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = pTeam->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			auto currentMission = std::exchange(pScript->CurrentMission , pScript->CurrentMission -1);
			pTeam->Focus = nullptr;
			pTeam->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Jump to the same line -> (Reason: Timed Jump loop)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				currentMission,
				nextAction.Action,
				nextAction.Argument);

			if (pTeamData->ForceJump_InitialCountdown > 0)
			{
				pTeamData->ForceJump_Countdown.Start(pTeamData->ForceJump_InitialCountdown);
				pTeamData->ForceJump_RepeatMode = true;
			}
		}
		else
		{
			pTeamData->ForceJump_InitialCountdown = -1;
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Jump to line: %d = %d,%d -> (Reason: Timed Jump)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->GetCurrentAction().Action, pScript->GetCurrentAction().Argument, pScript->CurrentMission + 1, pScript->GetNextAction().Action, pScript->GetNextAction().Argument);
		}

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit
				&& pUnit->IsAlive
				&& pUnit->Health > 0
				&& !pUnit->InLimbo)
			{
				pUnit->EnterIdleMode(false , 1);
			}
		}

		pTeam->StepCompleted = true;

	}
	else
	{
		ScriptExt::ProcessScriptActions(pTeam);
	}

	return 0;
}

// Take NewINIFormat into account just like the other classes does
// Author: secsome
DEFINE_HOOK(0x6E95B3, TeamClass_AI_MoveToCell, 0x6)
{
	if (!R->BL())
		return 0x6E95A4;

	GET(int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFS(0x38, 0x28));

	// if ( NewINIFormat < 4 ) then divide 128
	// in other times we divide 1000
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;
	cell.X = static_cast<short>(nCoord % nDivisor);
	cell.Y = static_cast<short>(nCoord / nDivisor);

	R->EAX(MapClass::Instance->GetCellAt(cell));
	return 0x6E959C;
}
bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat)
{
	if (pThis == pThat) //default comparison result
		return true;

	const auto pThatTechExt = TechnoTypeExt::ExtMap.TryFind(pThat);
	const auto pThisTechExt = TechnoTypeExt::ExtMap.TryFind(pThis);

	if (!pThatTechExt || !pThisTechExt)
		return false;

	if (GeneralUtils::IsValidString(pThatTechExt->GroupAs)) return IS_SAME_STR_(pThis->ID, pThatTechExt->GroupAs.c_str());
	else if (GeneralUtils::IsValidString(pThisTechExt->GroupAs)) return  IS_SAME_STR_(pThat->ID, pThisTechExt->GroupAs.c_str());

	return false;
}

//6EF57F
//DEFINE_HOOK(0x6EF577, TeamClass_GetMissionMemberTypes, 0x6)
//{
//	GET(TechnoTypeClass*, pCurMember, EAX);
//	GET(DynamicVectorClass<TechnoTypeClass*>*, OutVector , ESI);
//
//	return  OutVector->any_of([=](TechnoTypeClass* pMember) {
//		return pMember == pCurMember || GroupAllowed(pMember, pCurMember);
//	}) ?
//		//add member : // skip
//		0x6EF584 : 0x6EF5A5;
//}

DEFINE_HOOK(0x6EAD86, TeamClass_CanAdd_CompareType_Convert_UnitType, 0x7) //6
{
	enum
	{
		ContinueCheck = 0x6EAD8F,
		ContinueLoop = 0x6EADB3
	};

	//GET(UnitClass*, pGoingToBeRecuited, ESI);
	GET(UnitTypeClass*, pGoingToBeRecuitedType, EAX);
	GET(TaskForceClass*, pTeam, EDX);
	GET(int, nMemberIdx, EBP);

	return GroupAllowed(pTeam->Entries[nMemberIdx].Type, pGoingToBeRecuitedType) ?
		ContinueCheck : ContinueLoop;
}

DEFINE_HOOK(0x6EA6D3, TeamClass_CanAdd_ReplaceLoop, 0x7)
{
	GET(TaskForceClass*, pForce, EDX);
	GET(TechnoTypeClass*, pThat, EAX);
	GET(int, idx, EDI);

	return GroupAllowed(pForce->Entries[idx].Type , pThat) ? 0x6EA6F2 : 0x6EA6DC;
}

DEFINE_HOOK(0x50968F, HouseClass_CreateTeam_recuitType, 0x6)
{
	GET(TechnoTypeClass*, pThis, EBX);
	GET(FootClass* , pFoot ,ESI);
	GET(HouseClass* , pThisOwner ,EBP);

	return (pFoot->Owner == pThisOwner && GroupAllowed(pThis, pFoot->GetTechnoType())) ?

	//GET(TechnoTypeClass*, pThat, EAX);
	//return GroupAllowed(pThis, pThat) ?
		0x5096A5 : 0x5096B4;
}
