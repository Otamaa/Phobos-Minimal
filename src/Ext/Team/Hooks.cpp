#include "Body.h"

#include <Ext/TechnoType/Body.h>

static bool GroupAllowed(const std::string_view nFirst, const std::string_view nSecond)
{
	if (IS_SAME_STR_(nFirst.data(), NONE_STR) || IS_SAME_STR_(nSecond.data(), NONE_STR))
		return false;

	return IS_SAME_STR_(nFirst.data(), nSecond.data());
}

DEFINE_HOOK(0x6EAD80, TeamClass_Recuits_CompareType_Convert_UnitType, 0xD)
{
	enum { AllowAdd = 0x6EAD8F ,
		   ContinueLoop = 0x6EADB3
	};
	GET(UnitClass*, pGoingToBeRecuited, ESI);
	GET(TaskForceClass* const , pTeam, EDX);
	GET(int, nMemberIdx, EBP);


	auto pThatTech = TechnoTypeExt::GetExtData(pGoingToBeRecuited->Type);
	auto pThisTech = TechnoTypeExt::GetExtData(pTeam->Entries[nMemberIdx].Type);

	return pGoingToBeRecuited->Type == pTeam->Entries[nMemberIdx].Type
		|| GroupAllowed(pThatTech->GroupAs.data() , pThisTech->GroupAs.data()) ?
		AllowAdd : ContinueLoop;
}

DEFINE_HOOK(0x6EA6CD, TeamClass_Recuits_CompareType_Convert_All, 0xD)
{
	enum {
		AllowAdd = 0x6EA6F2,
		ContinueLoop = 0x6EA6DC
	};
	GET(TechnoTypeClass*, pGoingToBeRecuited, EAX);
	GET(TeamTypeClass*, pTeam, ECX);
	GET(int, nMemberIdx, EDI);

	auto pThatTech = TechnoTypeExt::GetExtData(pGoingToBeRecuited);
	auto pThisTech = TechnoTypeExt::GetExtData(pTeam->TaskForce->Entries[nMemberIdx].Type);

	return pGoingToBeRecuited == pTeam->TaskForce->Entries[nMemberIdx].Type
		|| GroupAllowed(pThatTech->GroupAs.data(), pThisTech->GroupAs.data()) ?
		AllowAdd : ContinueLoop;
}