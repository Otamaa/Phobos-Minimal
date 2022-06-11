#include "Body.h"
#include <TeamClass.h>

//6EF8B0
DEFINE_HOOK(0x6EF8B0, TeamMission_GatherAt_Enemy, 0x8)
{
	GET(int, nDistance, EDX);
	GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);

	if (auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type))
		R->EDX(pTeamExt->AI_SafeDIstance.isset() ? (pTeamExt->AI_SafeDIstance.Get() + pTeamM->Argument) << 8 : nDistance);

	return 0x0;
}

DEFINE_HOOK(0x6EFB78, TeamMission_GatherAt_BaseTeam, 0x8)
{
	GET(int, nDistance, EDX);
	GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x4C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);

	if (auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type))
		R->EDX(pTeamExt->AI_FriendlyDistance.isset() ? (pTeamExt->AI_FriendlyDistance.Get() + pTeamM->Argument) << 8 : nDistance);

	return 0x0;
}

DEFINE_HOOK(0x472589, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(TeamTypeClass*, pTeamType, ECX);

	if (auto nTeamDecision = pTeamType->MindControlDecision) {
		if (nTeamDecision > 5)
			nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);

		R->EAX(nTeamDecision);
		return 0x472593;
	}

	return 0x4725B0;
}