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
#include <Misc/AresData.h>

#include <Notifications.h>
#include <strsafe.h>
#include <Ares_TechnoExt.h>

DEFINE_OVERRIDE_HOOK(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
{
	GET(FootClass*, pFoot, EBP);
	R->ECX(HouseExt::GetParadropPlane(pFoot->Owner));
	++Unsorted::ScenarioInit();
	return 0x65DBD0;
}

bool ScriptExt_Handle(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd)
{
	switch (pTeamMission->Action)
	{
	case TeamMissionType::Garrison_building:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = pTeam->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				pFirst->align_154->TakeVehicleMode = false;

				if (pFirst->GarrisonStructure())
					pTeam->RemoveMember(pFirst, -1, 1);

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		pTeam->StepCompleted = 1;
		return true;
	}
	}

	if (pTeamMission->Action >= TeamMissionType::count)
	{
		switch ((AresScripts)pTeamMission->Action)
		{
		case AresScripts::AuxilarryPower:
		{
			AuxPower(pTeam->Owner) += pTeamMission->Argument;
			pTeam->Owner->RecheckPower = 1;
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::KillDrivers:
		{
			const auto pToHouse = HouseExt::FindSpecial();
			FootClass* pCur = nullptr;
			if (auto pFirst = pTeam->FirstUnit)
			{
				auto pNext = pFirst->NextTeamMember;
				do
				{
					if (pFirst->Health > 0 && pFirst->IsAlive && pFirst->IsOnMap && !pFirst->InLimbo)
					{
						if (!pFirst->align_154->Is_DriverKilled && AresData::IsDriverKillable(pFirst, 1.0))
						{
							AresData::KillDriverCore(pFirst, pToHouse, nullptr, false);
						}
					}

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = 1;
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
					pFirst->align_154->TakeVehicleMode = true;

					if (pFirst->GarrisonStructure())
						pTeam->RemoveMember(pFirst, -1, 1);

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = 1;
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
					const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFirst->GetTechnoType());
					if (pTypeExt->Convert_Script)
					{
						AresData::ConvertTypeTo(pFirst, pTypeExt->Convert_Script);
					}

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::SonarReveal:
		{
			const auto nDur = pTeamMission->Argument;
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				auto& nSonarTime = pUnit->align_154->CloakSkipTimer;
				if (nDur > nSonarTime.GetTimeLeft())
				{
					nSonarTime.Start(nDur);
				}
				else if (nDur <= 0)
				{
					if (nDur == 0)
					{
						nSonarTime.Stop();
					}
				}
			}

			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::DisableWeapons:
		{
			const auto nDur = pTeamMission->Argument;
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				auto& nTimer = pUnit->align_154->DisableWeaponTimer;
				if (nDur > nTimer.GetTimeLeft())
				{
					nTimer.Start(nDur);
				}
				else if (nDur <= 0 && nDur == 0)
				{
					nTimer.Stop();
				}
			}

			pTeam->StepCompleted = 1;
			return true;
		}
		}
	}

	return false;
}

#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

DEFINE_OVERRIDE_HOOK(0x6E9443, TeamClass_AI_HandleAres, 8)
{
	enum { ReturnFunc = 0x6E95AB, Continue = 0x0 };
	GET(TeamClass*, pThis, ESI);
	GET(ScriptActionNode*, pTeamMission, EAX);
	GET_STACK(bool, bThirdArg, 0x10);

	if(ScriptExt_Handle(pThis, pTeamMission, bThirdArg))
		return ReturnFunc;

	auto pTeamData = TeamExt::ExtMap.Find(pThis);

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = pThis->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			pThis->Focus = nullptr;
			pThis->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Jump to the same line -> (Reason: Timed Jump loop)\n",
				pThis->Type->ID,
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
				pThis->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission, curAct, curArgs,
				pScript->CurrentMission + 1, nextAct, nextArgs
			);
		}

		for (auto pUnit = pThis->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExt::IsUnitAvailable(pUnit, true))
			{
				pUnit->EnterIdleMode(false, 1);
			}
		}

		pThis->StepCompleted = true;
	}
	else
	{
		ScriptExt::ProcessScriptActions(pThis);
	}

	return Continue;
}

DEFINE_OVERRIDE_HOOK(0x6EF8A1, TeamClass_GatherAtEnemyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x34));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//Debug::Log(std::format(__FUNCTION__ " Function With Type {} ! \n",pTeam->Type->ID));
	//R->EDX(pTeamExt->AI_SafeDIstance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	R->EDX(pRules->AISafeDistance + pTeamM->Argument);

	return 0x6EF8A7;
}

DEFINE_OVERRIDE_HOOK(0x6EFB69, TeamClass_GatherAtFriendlyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x4C, 0x2C));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//Debug::Log("%s", std::format("{} Function With Type {} ! \n", __FUNCTION__, pTeam->Type->ID).c_str());
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//R->EDX(pTeamExt->AI_FriendlyDistance.Get(RulesExt::Global()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance)) + pTeamM->Argument);
	R->EDX(RulesExt::Global()->AIFriendlyDistance.Get(pRules->AISafeDistance) + pTeamM->Argument);
	return 0x6EFB6F;
}

// #895225: make the AI smarter. this code was missing from YR.
// it clears the targets and assigns the attacker the team's current focus.
DEFINE_OVERRIDE_HOOK(0x6EB432, TeamClass_AttackedBy_Retaliate, 9)
{
	GET(TeamClass*, pThis, ESI);
	GET(AbstractClass*, pAttacker, EBP);

	// get ot if global option is off
	if (!RulesExt::Global()->TeamRetaliate)
	{
		return 0x6EB47A;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pThis->Focus);
	auto pSpawn = pThis->SpawnCell;

	if (!pFocus || !pFocus->IsArmed() || !pSpawn || pFocus->IsCloseEnoughToAttackCoords(pSpawn->GetCoords()))
	{
		// disallow aircraft, or units considered as aircraft, or stuff not on map like parasites
		if (pAttacker->WhatAmI() != AircraftClass::AbsID)
		{
			if (auto pAttackerFoot = abstract_cast<FootClass*>(pAttacker))
			{
				if (pAttackerFoot->InLimbo || pAttackerFoot->GetTechnoType()->ConsideredAircraft)
				{
					return 0x6EB47A;
				}
			}

			pThis->Focus = pAttacker;

			// this is the original code, but commented out because it's responsible for switching
			// targets when the team is attacked by two or more opponents. Now, the team should pick
			// the first target, and keep it. -AlexB
			//for(NextTeamMember i(pThis->FirstUnit); i; ++i) {
			//	if(i->IsAlive && i->Health && (Unsorted::ScenarioInit || !i->InLimbo)) {
			//		if(i->IsTeamLeader || i->WhatAmI() == AircraftClass::AbsID) {
			//			i->SetTarget(nullptr);
			//			i->SetDestination(nullptr, true);
			//		}
			//	}
			//}
		}
	}

	return 0x6EB47A;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
DEFINE_OVERRIDE_HOOK_AGAIN(0x65EC4A, TeamTypeClass_ValidateHouse, 6)
DEFINE_OVERRIDE_HOOK(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
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
}
