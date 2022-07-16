#include "Body.h"

#include <Ext/TechnoType/Body.h>

static bool GroupAllowed(const std::string& nFirst, const std::string& nSecond)
{
	if (nFirst.empty() || nSecond.empty())
		return false;

	auto const None = NONE_STR;
	if (nFirst == None || nSecond == None)
		return false;

	return (nFirst == nSecond);
}

DEFINE_HOOK(0x6EAD80, TeamClass_Recuits_CompareType_Convert_UnitType, 0x6)
{
	enum { ContinueCheck = 0x6EAD8F ,
		   ContinueLoop = 0x6EADB3
	};

	GET(const UnitClass*, pGoingToBeRecuited, ESI);
	GET(const TaskForceClass* const , pTeam, EDX);
	GET(int, nMemberIdx, EBP);

	const auto pThisTech = pTeam->Entries[nMemberIdx].Type;

	if (pGoingToBeRecuited->Type == pThisTech)
		return ContinueCheck;

	const auto pThatTechExt = TechnoTypeExt::ExtMap.Find(pGoingToBeRecuited->Type);
	const auto pThisTechExt = TechnoTypeExt::ExtMap.Find(pThisTech);

	if (!pThatTechExt || !pThisTechExt)
		return ContinueLoop;

	return GroupAllowed(pThatTechExt->GroupAs.data() , pThisTechExt->GroupAs.data()) ?
		ContinueCheck : ContinueLoop;
}

DEFINE_HOOK(0x6EA6CD, TeamClass_Recuits_CompareType_Convert_All, 0x6)
{
	enum {
		Break = 0x6EA6F2,
		ContinueLoop = 0x6EA6DC
	};

	GET(const TechnoTypeClass*, pGoingToBeRecuited, EAX);
	GET(const TeamTypeClass*, pTeam, ECX);
	GET(int, nMemberIdx, EDI);

	const auto pThisTechn = pTeam->TaskForce->Entries[nMemberIdx].Type;

	if (pThisTechn == pGoingToBeRecuited)
		return Break;

	const auto pThatTechExt = TechnoTypeExt::ExtMap.Find(pGoingToBeRecuited);
	const auto pThisTechExt = TechnoTypeExt::ExtMap.Find(pThisTechn);

	if (!pThatTechExt || !pThisTechExt)
		return ContinueLoop;

	return GroupAllowed(pThatTechExt->GroupAs.data(), pThisTechExt->GroupAs.data()) ?
		Break : ContinueLoop;
}