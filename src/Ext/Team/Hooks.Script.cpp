#include "Body.h"

#include <Ext/Script/Body.h>

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
			pScript->CurrentMission--;
			pTeam->Focus = nullptr;
			pTeam->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Jump to the same line -> (Reason: Timed Jump loop)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission + 1,
				nextAction.Action,
				nextAction.Argument
			);

			if (pTeamData->ForceJump_InitialCountdown > 0)
			{
				pTeamData->ForceJump_Countdown.Start(pTeamData->ForceJump_InitialCountdown);
				pTeamData->ForceJump_RepeatMode = true;
			}
		}
		else
		{
			const auto& [curAct, curArgs] = pScript->GetCurrentAction();
			const auto& [nextAct, nextArgs] = pScript->GetNextAction();

			pTeamData->ForceJump_InitialCountdown = -1;
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Jump to line: %d = %d,%d -> (Reason: Timed Jump)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission, curAct, curArgs,
				pScript->CurrentMission + 1, nextAct, nextArgs
			);
		}

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExt::IsUnitAvailable(pUnit, true))
			{
				pUnit->EnterIdleMode(false, 1);
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