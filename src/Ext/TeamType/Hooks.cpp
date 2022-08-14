#include "Body.h"
#include <TeamClass.h>

#ifdef ENABLE_NEWHOOKS
//6EF8B0
DEFINE_HOOK(0x6EF8B0, TeamMission_GatherAt_Enemy, 0x8)
{
	GET(int, nDistance, EDX);
	GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);

	if (const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type))
		R->EDX(pTeamExt->AI_SafeDIstance.isset() ? (pTeamExt->AI_SafeDIstance.Get() + pTeamM->Argument) << 8 : nDistance);

	return 0x0;
}

DEFINE_HOOK(0x6EFB78, TeamMission_GatherAt_BaseTeam, 0x8)
{
	GET(int, nDistance, EDX);
	GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x4C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);

	if (const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type))
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

static void __fastcall TeamClass_Assign_Mission_Target(TeamClass* pThis, void* _, AbstractClass* pNewTarget)
{
	if(pNewTarget && pNewTarget->WhatAmI() == AbstractType::Cell){
		if (const auto pExt = TeamTypeExt::ExtMap.Find(pThis->Type)){
			if (!pExt->AttackWaypoint_AllowCell.Get()){
				pThis->AssignMissionTarget(nullptr);
				return;
			}
		}
	}

	pThis->AssignMissionTarget(pNewTarget);
}

DEFINE_JUMP(CALL, 0x6ECA05, GET_OFFSET(TeamClass_Assign_Mission_Target));

DEFINE_HOOK(0x6EBB86, TeamClass_MoveToFocus_IsInStray, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TeamClass*, pThis, EBP);

	if (pFoot->GetHeight() > 0 && pFoot->WhatAmI() == AbstractType::Unit && pThis->SpawnCell){
		auto nCoord = pThis->SpawnCell->GetCoords();
		nCoord.Z = Map.GetCellFloorHeight(nCoord);
		R->EAX(static_cast<int>(pFoot->GetCoords().DistanceFrom(nCoord)));
	}
	else
		R->EAX(pFoot->DistanceFrom(pThis->SpawnCell));

	return 0x6EBB91;
}

DEFINE_HOOK(0x6EBE69, TeamClass_MoveToFocus_SetDestination, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == AbstractType::UnitType && static_cast<UnitTypeClass*>(pType)->JumpJet && static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?
		0x6EBE9C : 0x6EBE82;
}

DEFINE_HOOK(0x6EBEDB, TeamClass_MoveToFocus_BalloonHover, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == AbstractType::UnitType && static_cast<UnitTypeClass*>(pType)->JumpJet && static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?

		0x6EBEEF : 0x6EBEFF;
}

DEFINE_HOOK(0x65DF8D, TeamTypeClass_GenerateTeamMemembers_OpenTopped, 0x6)
{
	GET(FootClass* , pFoot , ESI);
	GET_STACK(TechnoClass*, pWho, 0x14);

	if (pFoot->GetTechnoType()->OpenTopped)
	{
		pFoot->EnteredOpenTopped(pWho);
		pWho->Transporter = pFoot;
	}

	return R->Stack<bool>(0x14) ? 0x65DF95 : 0x65E004;
}

DEFINE_HOOK(0x6ED492 ,TeamClass_ExecuteTransporterLoad_PassengerFix, 0x6)
{
	GET(FootClass*, pFoot, EDI);
	return (pFoot->GetTechnoType()->JumpJet && pFoot->GetHeight() > 0 || pFoot->CurrentMission == Mission::Unload) ?
		0x6ED429 :0x0;
}

DEFINE_HOOK(0x6EF23A,TeamClass_ExecuteTransportUnload_Transporter, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	auto const v2 = pFoot->Passengers.FirstPassenger == 0;
	return (!pFoot->Passengers.NumPassengers || !v2) ? 0x6EF2A7 :0x6EF244;
}
#endif