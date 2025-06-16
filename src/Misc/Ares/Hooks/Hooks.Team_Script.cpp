#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

#include "Header.h"

#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

ASMJIT_PATCH(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
{
	GET(FootClass*, pFoot, EBP);
	R->ECX(HouseExtData::GetParadropPlane(pFoot->Owner));
	++Unsorted::ScenarioInit();
	return 0x65DBD0;
}

ASMJIT_PATCH(0x6E9443, TeamClass_AI_Additionals, 8)
{
	enum { ReturnFunc = 0x6E95AB, Continue = 0x0 };
	GET(TeamClass*, pThis, ESI);
	GET(ScriptActionNode*, pTeamMission, EAX);
	GET_STACK(bool, bThirdArg, 0x10);

	if (AresScriptExt::Handle(pThis, pTeamMission, bThirdArg))
		return ReturnFunc;

	auto pTeamData = TeamExtContainer::Instance.Find(pThis);

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = pThis->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			pThis->ArchiveTarget = nullptr;
			pThis->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::LogInfo("DEBUG: [{}] {}](line: {} = {},{}): Jump to the same line -> (Reason: Timed Jump loop)",
				pThis->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission + 1,
				(int)nextAction.Action,
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
			pTeamData->ForceJump_Countdown.Stop();
			Debug::LogInfo("DEBUG: [{}] [{}](line: {} = {},{}): Jump to line: {} = {},{} -> (Reason: Timed Jump)",
				pThis->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission, (int)curAct, curArgs,
				pScript->CurrentMission + 1, (int)nextAct, nextArgs
			);
		}

		for (auto pUnit = pThis->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExtData::IsUnitAvailable(pUnit, true))
			{
				pUnit->EnterIdleMode(false, 1);
			}
		}

		pThis->StepCompleted = true;
		return ReturnFunc;
	}

	return ScriptExtData::ProcessScriptActions(pThis) ? ReturnFunc : Continue;
}

ASMJIT_PATCH(0x6EF8A1, TeamClass_GatherAtEnemyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x34));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//Debug::LogInfo(fmt::format(__FUNCTION__ " Function With Type {} ! ",pTeam->Type->ID));
	//R->EDX(pTeamExt->AI_SafeDIstance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	R->EDX(pRules->AISafeDistance + pTeamM->Argument);

	return 0x6EF8A7;
}

ASMJIT_PATCH(0x6EFB69, TeamClass_GatherAtFriendlyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, 0x48 - 0x2C);
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//if (IS_SAME_STR_(pTeam->Type->ID, "0100003I-G")) {
	//	Debug::LogInfo("Team %s with script %s, GatherAt friendlyBase.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	//	int i = 0;
	//	for (auto pCur = pTeam->FirstUnit; pCur; pCur = pCur->NextTeamMember) {
	//		Debug::LogInfo("	Team %s with script %s, GatherAt friendlyBase CurMember[%d - %s].",
	//			pTeam->Type->ID,
	//			pTeam->CurrentScript->Type->ID,
	//		i++,
	//		pCur->GetTechnoType()->ID
	//		);
	//	}
	//}

	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//R->EDX(pTeamExt->AI_FriendlyDistance.Get(RulesExtData::Instance()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance)) + pTeamM->Argument);
	const auto distanceresult = RulesExtData::Instance()->AIFriendlyDistance.Get(pRules->AISafeDistance) + pTeamM->Argument;
	R->EDX(distanceresult);
	return 0x6EFB6F;
}

//ASMJIT_PATCH(0x6EFC54, TeamClass_GatherAtFriendlyBase_TargetAssigned, 0x5)
//{
//	GET_STACK(TeamClass*, pTeam, 0x48 - 0x2C);
//	GET(CellClass*, pTarget, EAX);
//
//	if (IS_SAME_STR_(pTeam->Type->ID, "0100003I-G"))
//	{
//		Debug::LogInfo("Team %s with script %s, GatherAt friendlyBase.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
//		int i = 0;
//		for (auto pCur = pTeam->FirstUnit; pCur; pCur = pCur->NextTeamMember)
//		{
//			Debug::LogInfo("	Team %s with script %s, GatherAt friendlyBase CurMember[%d - %s].",
//				pTeam->Type->ID,
//				pTeam->CurrentScript->Type->ID,
//			i++,
//			pCur->GetTechnoType()->ID
//			);
//		}
//
//		const auto coord = pTarget->GetCoords();
//		Debug::LogInfo("Team %s with script %s, GatherAt { %d , %d , %d }.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, coord.X, coord.Y, coord.Z);
//	}
//
//	return 0x0;
//}

// #895225: make the AI smarter. this code was missing from YR.
// it clears the targets and assigns the attacker the team's current focus.
ASMJIT_PATCH(0x6EB432, TeamClass_AttackedBy_Retaliate, 9)
{
	GET(TeamClass*, pThis, ESI);
	GET(AbstractClass*, pAttacker, EBP);

	if (RulesExtData::Instance()->TeamRetaliate)
	{
		auto pFocus = flag_cast_to<TechnoClass*>(pThis->ArchiveTarget);
		CellClass* SpawnCell = pThis->SpawnCell;

		if (!pFocus
		  || !pFocus->IsArmed()
		  || !SpawnCell
		  || pFocus->IsCloseEnoughToAttackCoords(SpawnCell->GetCoords()))
		{
			if (pAttacker->WhatAmI() != AircraftClass::AbsID)
			{
				auto pAttackerTechno = flag_cast_to<TechnoClass*, false>(pAttacker);

				auto Owner = pThis->Owner;
				if (pAttackerTechno && Owner->IsAlliedWith(pAttackerTechno->GetOwningHouse())) {
					return 0x6EB47A;
				}

				if (auto pAttackerFoot = flag_cast_to<FootClass*, false>(pAttacker)) {
					if(pAttackerFoot->InLimbo
					|| pAttackerFoot->GetTechnoType()->ConsideredAircraft) {
						return 0x6EB47A;
					}
				}

				pThis->ArchiveTarget = pAttacker;
			}
		}
	}

#ifdef CUSTOM
	// get ot if global option is off
	if (!RulesExtData::Instance()->TeamRetaliate)
	{
		return 0x6EB47A;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pThis->ArchiveTarget);
	auto pSpawn = pThis->SpawnCell;

	if (!pFocus || !pFocus->IsArmed() || !pSpawn || pFocus->IsCloseEnoughToAttackCoords(pSpawn->GetCoords())) {
		// disallow aircraft, or units considered as aircraft, or stuff not on map like parasites
		if (pAttacker->WhatAmI() != AircraftClass::AbsID) {
			if (pFocus) {
				if (auto pFocusOwner = pFocus->GetOwningHouse()) {
					if (pFocusOwner->IsAlliedWith(pAttacker))
						return 0x6EB47A;
				}
			}

			if (auto pAttackerFoot = abstract_cast<FootClass*>(pAttacker)) {
				auto IsInTransporter = pAttackerFoot->Transporter && pAttackerFoot->Transporter->GetTechnoType()->OpenTopped;

				if (pAttackerFoot->InLimbo && !IsInTransporter) {
					return 0x6EB47A;
				}

                if(IsInTransporter)
				   pAttacker = pAttackerFoot->Transporter;

				if (((TechnoClass*)pAttacker)->GetTechnoType()->ConsideredAircraft || pAttacker->WhatAmI() == AircraftClass::AbsID)
					return 0x6EB47A;

				auto first = pThis->FirstUnit;
				if (first) {
					auto next = first->NextTeamMember;
					while (!first->IsAlive
						|| !first->Health
						|| !first->IsArmed()
						|| !first->IsTeamLeader && first->WhatAmI() != AircraftClass::AbsID
					) {
						first = next;
						if (!next)
							return 0x6EB47A;

						next = next->NextTeamMember;
					}

					pThis->AssignMissionTarget(pAttacker);
				}
			}
		}
	}
#endif

	return 0x6EB47A;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
ASMJIT_PATCH(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if (pHouse &&
		(pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated))
	{
		return 0;
	}

	// no.
	return (R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
}ASMJIT_PATCH_AGAIN(0x65EC4A, TeamTypeClass_ValidateHouse, 6)

