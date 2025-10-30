#include "Body.h"

#include <Ext/TechnoType/Body.h>

// Take NewINIFormat into account just like the other classes does
// Author: secsome
ASMJIT_PATCH(0x6EC77B, TeamClass_AI_MoveToCell, 0x7)
{
	GET_STACK(ScriptActionNode*, pNode, 0x4);

	// if ( NewINIFormat < 4 ) then divide 128
	// in other times we divide 1000
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;

	R->EAX(MapClass::Instance->GetCellAt(
		CellStruct(static_cast<short>(pNode->Argument % nDivisor) , static_cast<short>(pNode->Argument / nDivisor))));

	return 0x6EC7B3;
}

//6EF57F
//ASMJIT_PATCH(0x6EF577, TeamClass_GetMissionMemberTypes, 0x6)
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

//this completely replaced by ares
//ASMJIT_PATCH(0x50968F, HouseClass_CreateTeam_recuitType, 0x6)
//{
//	GET(TechnoTypeClass*, pThis, EBX);
//	GET(FootClass* , pFoot ,ESI);
//	GET(HouseClass* , pThisOwner ,EBP);
//
//	return (pFoot->Owner == pThisOwner && GroupAllowed(pThis, pFoot->GetTechnoType())) ?
//
//	//GET(TechnoTypeClass*, pThat, EAX);
//	//return GroupAllowed(pThis, pThat) ?
//		0x5096A5 : 0x5096B4;
//}

#include <Ext/Script/Body.h>
#include <ThemeClass.h>
#include <TeamTypeClass.h>
#include <Unsorted.h>

ASMJIT_PATCH(0x723CA1, TeamMissionClass_FillIn_StringsSupport_and_id_masks, 0xB)
{
	enum { SkipCode = 0x723CD2 };

	GET(ScriptActionNode*, node, ECX);
	GET_STACK(char*, scriptActionLine, 0x8);

	int action = 0;
	int argument = 0;
	char* endptr;

	char buff[256];
	if (sscanf(scriptActionLine, "%d,%s", &action, buff) != 2)
	{
		node->Action = (TeamMissionType)action;
		node->Argument = argument;
		R->ECX(node);

		return SkipCode;
	}

	long val = strtol(buff, &endptr, 10);

	if (*endptr == '\0'
		&& val >= std::numeric_limits<int>::min()
		&& val <= std::numeric_limits<int>::max())
	{
		// Integer case (the classic).
		argument = static_cast<int>(val);
	}
	else
	{
		// New strings case
		// Action masks: These actions translate IDs into indices while preserving the original action values.
		// The reason for using these masks is that some ScriptType actions rely on fixed indices rather than ID labels.
		// When these lists change, there's a high probability of breaking the original index of the pointed element

		int index = 0;
		int prefixIndex = 0;

		switch (static_cast<PhobosScripts>(action))
		{
		case PhobosScripts::ChangeToScriptByID:
			action = 17;
			index = ScriptTypeClass::FindIndexById(buff);
			break;
		case PhobosScripts::ChangeToTeamTypeByID:
			action = 18;
			index = TeamTypeClass::FindIndexById(buff);
			break;
		case PhobosScripts::ChangeToHouseByID:
			action = 20;
			index = HouseTypeClass::FindIndexByIdAndName(buff);

			if (index < 0)
				Debug::Log("AI Scripts - TeamMissionClass_FillIn_StringsSupport: Invalid House [%s]\n", buff);

			break;
		case PhobosScripts::PlaySpeechByID: // Note: PR 1900 needs to be merged into develop
			action = static_cast<int>(TeamMissionType::Play_speech);
			index = VoxClass::FindIndexById(buff);
			break;
		case PhobosScripts::PlaySoundByID:
			action = 25;
			index = VocClass::FindIndexById(buff);
			break;
		case PhobosScripts::PlayMovieByID:
			// Note: action "26" is currently impossible without an expert Phobos developer declaring the Movies class... in that case I could code the right FindIndex(textArgument) so sadly I'll skip "26" for now :-(
			action = 26;
			for (int i = 0; i < MovieInfoArray->Count; ++i)
			{
				if (MovieInfoArray[i] == buff)
				{
					index = i;
					break;
				}
			}
			break;
		case PhobosScripts::PlayThemeByID:
			action = 27;
			index = ThemeClass::Instance->FindIndex(buff);
			break;
		case PhobosScripts::PlayAnimationByID:
			action = 51;
			index = AnimTypeClass::FindIndexById(buff);
			break;
		case PhobosScripts::AttackEnemyStructureByID:
		case PhobosScripts::MoveToEnemyStructureByID:
		case PhobosScripts::ChronoshiftTaskForceToStructureByID:
		case PhobosScripts::MoveToFriendlyStructureByID:
		{
			if (PhobosScripts::AttackEnemyStructureByID == static_cast<PhobosScripts>(action))
				action = 46;
			else if (PhobosScripts::MoveToEnemyStructureByID == static_cast<PhobosScripts>(action))
				action = 47;
			else if (PhobosScripts::ChronoshiftTaskForceToStructureByID == static_cast<PhobosScripts>(action))
				action = 56;
			else if (PhobosScripts::MoveToFriendlyStructureByID == static_cast<PhobosScripts>(action))
				action = 58;

			/* BwP check:
			Information from https://modenc.renegadeprojects.com/ScriptTypes/ScriptActions
			Computed Value                           Description
			-------------------------------------		-------------------------------------------------------
			0 (Hex 0x0) + Building Index          -> Index of the instance of the building with least threat
			65536 (Hex 0x10000) + Building Index  -> Index of the instance of the building with highest threat
			131072 (Hex 0x20000) + Building Index -> Index of the instance of the building which is nearest
			196608 (Hex 0x30000) + Building Index -> Index of the instance of the building which is farthest
			*/

			char id[sizeof(AbstractTypeClass::ID)] = { 0 };
			char bwp[20] = { 0 };

			if (sscanf(buff, "%s,%[^\n]", id, bwp) == 2)
			{
				index = BuildingTypeClass::FindIndexById(id);

				if (index >= 0)
				{
					if (IS_SAME_STR_(bwp, "highestthreat"))
						prefixIndex = static_cast<int>(BuildingWithProperty::HighestThreat);
					else if (IS_SAME_STR_(bwp, "nearest"))
						prefixIndex = static_cast<int>(BuildingWithProperty::Nearest);
					else if (IS_SAME_STR_(bwp, "farthest"))
						prefixIndex = static_cast<int>(BuildingWithProperty::Farthest);
				}
			}
			break;
		}
		default:
			index = 0;
			break;
		}

		if (index >= 0)
			argument = prefixIndex + index;
	}

	node->Action = (TeamMissionType)action;
	node->Argument = argument;
	R->ECX(node);

	return SkipCode;
}