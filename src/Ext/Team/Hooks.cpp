#include "Body.h"

#include <Ext/TechnoType/Body.h>

static bool GroupAllowed(const std::string& nFirst, const std::string& nSecond)
{
	if (IS_SAME_STR_(nFirst.c_str(), NONE_STR) || IS_SAME_STR_(nSecond.c_str(), NONE_STR))
		return false;

	return IS_SAME_STR_(nFirst.c_str(), nSecond.c_str());
}

DEFINE_HOOK(0x6EAD80, TeamClass_Recuits_CompareType_Convert_UnitType, 0xD)
{
	enum { AllowAdd = 0x6EAD8F ,
		   ContinueLoop = 0x6EADB3
	};
	GET(UnitClass*, pGoingToBeRecuited, ESI);
	GET(TaskForceClass* const , pTeam, EDX);
	GET(int, nMemberIdx, EBP);


	auto pThatTech = TechnoTypeExt::ExtMap.Find(pGoingToBeRecuited->Type);
	auto pThisTech = TechnoTypeExt::ExtMap.Find(pTeam->Entries[nMemberIdx].Type);

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

	auto pThatTech = TechnoTypeExt::ExtMap.Find(pGoingToBeRecuited);
	auto pThisTech = TechnoTypeExt::ExtMap.Find(pTeam->TaskForce->Entries[nMemberIdx].Type);

	return pGoingToBeRecuited == pTeam->TaskForce->Entries[nMemberIdx].Type
		|| GroupAllowed(pThatTech->GroupAs.data(), pThisTech->GroupAs.data()) ?
		AllowAdd : ContinueLoop;
}