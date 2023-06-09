#include "Body.h"

#include <Ext/TechnoType/Body.h>

bool NOINLINE GroupAllowed(const std::string& nFirst, const std::string& nSecond)
{
	if (nFirst.empty() || nSecond.empty())
		return false;

	auto nNone = NONE_STR;
	auto nNone2 = NONE_STR2;
	if (IS_SAME_STR_(nFirst.c_str(), nNone)
		|| IS_SAME_STR_(nSecond.c_str(), nNone)
		|| IS_SAME_STR_(nFirst.c_str(), nNone2)
		|| IS_SAME_STR_(nSecond.c_str(), nNone2)
		)
	{
		return false;
	}

	return IS_SAME_STR_(nSecond.c_str(), nSecond.c_str());
}

bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat)
{
	if (pThis == pThat)
		return true;

	//InfantryClass doesnt support this ,..
   // if (Is_InfantryType(pThis))
	//	return false;

	const auto pThatTechExt = TechnoTypeExt::ExtMap.Find(pThat);
	const auto pThisTechExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (!pThatTechExt || !pThisTechExt || pThatTechExt->IsDummy.Get())
		return false;

	return GroupAllowed(pThatTechExt->GroupAs.data(), pThisTechExt->GroupAs.data());
}

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

void NOINLINE CompareGroup(int* pData, int nMax, TaskForceClass* pTask, TechnoTypeClass* pThat)
{
	int temp = 0;

	do
	{
		temp = *pData;
		if (pTask->Entries[temp].Type == pThat)
			break;

		*pData = temp + 1;

	}
	while ((temp + 1) < nMax);
}

DEFINE_HOOK(0x6EA6A5, TeamClass_CanAdd_ReplaceLoop, 0x6)
{
	GET(TeamClass*, pThis, EBP);
	GET(TechnoClass*, pThat, ESI);
	GET(int*, pMissMatchCount, EBX);

	*pMissMatchCount = 0;

	if (pThis->Type->TaskForce->CountEntries > 0) {
		CompareGroup(pMissMatchCount, pThis->Type->TaskForce->CountEntries, pThis->Type->TaskForce, pThat->GetTechnoType());
	}

	R->EBX(pMissMatchCount);
	return 0x6EA6F2;
}


#ifdef BROKE_
DEFINE_HOOK(0x6EA6CD, TeamClass_Recuits_CompareType_Convert_All, 0x6)
{
	enum
	{
		Break = 0x6EA6F2,
		ContinueLoop = 0x6EA6DC
	};

	GET(TechnoTypeClass*, pGoingToBeRecuited, EAX);
	GET(TeamTypeClass*, pTeam, ECX);
	GET(int, nMemberIdx, EDI);

	return GroupAllowed(pTeam->TaskForce->Entries[nMemberIdx].Type, pGoingToBeRecuited) ?
		Break : ContinueLoop;
}
#endif