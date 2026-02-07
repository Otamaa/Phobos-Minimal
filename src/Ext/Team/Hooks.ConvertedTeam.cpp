#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Team/Body.h>

#include <TaskForceClass.h>

ASMJIT_PATCH(0x509697, HouseClass_CanInstansiateTeam_CompareType_Convert, 0xA)
{
	enum
	{
		ContinueCheck = 0x5096A5,
		ContinueLoop = 0x5096B4
	};
	GET(FootClass*, pGoingToBeRecuited, ESI);
	GET(TechnoTypeClass*, pTaskForceTeam, EBX);

	const auto pGoingToBeRecuitedType = pGoingToBeRecuited->GetTechnoType();
	return pGoingToBeRecuitedType == pTaskForceTeam
		|| TeamExtData::IsEligible(pGoingToBeRecuited, pTaskForceTeam)
		?
		ContinueCheck : ContinueLoop;
}

// Dead code: _Remove is LJMP-replaced at 0x6EA870 by FakeTeamClass::_Remove
// The IsEligible check is integrated into the backported _Remove function
#if 0
ASMJIT_PATCH(0x6EA8FA, TeamClass_Remove_CompareType_Convert, 0x6)
{
	enum
	{
		jz_ = 0x6EA91B,
		advance = 0x6EA905
	};

	GET(FootClass*, pTeam, EBP);
	GET(DWORD, val1, EBX);
	GET(DWORD, val2, ESI);

	TechnoTypeClass* pTaskForceTeam = reinterpret_cast<TechnoTypeClass*>(val1 + val2);

	return pTeam->GetTechnoType() == pTaskForceTeam
		|| TeamExtData::IsEligible(pTeam, pTaskForceTeam)
		? jz_ : advance;
}
#endif

// Dead code: _Can_Add is LJMP-replaced at 0x6EA610 by FakeTeamClass::_Can_Add
// The IsEligible check is integrated into the backported _Can_Add function
#if 0
ASMJIT_PATCH(0x6EAD86, TeamClass_CanAdd_CompareType_Convert_UnitType, 0x7) //6
{
	enum
	{
		ContinueCheck = 0x6EAD8F,
		ContinueLoop = 0x6EADB3
	};

	GET(UnitClass*, pGoingToBeRecuited, ESI);
	GET(UnitTypeClass*, pGoingToBeRecuitedType, EAX);
	GET(TaskForceClass*, pTeam, EDX);
	GET(int, nMemberIdx, EBP);

	return
		pGoingToBeRecuitedType == pTeam->Entries[nMemberIdx].Type
		|| TeamExtData::IsEligible(pGoingToBeRecuited, pTeam->Entries[nMemberIdx].Type)
		//|| TeamExtData::GroupAllowed(pTeam->Entries[nMemberIdx].Type, pGoingToBeRecuitedType)
		//|| TeamExtData::GroupAllowed(pTeam->Entries[nMemberIdx].Type, TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type)
		?
		ContinueCheck : ContinueLoop;
}
#endif

// Dead code: _Can_Add is LJMP-replaced at 0x6EA610 by FakeTeamClass::_Can_Add
// The IsEligible check is integrated into the backported _Can_Add function
#if 0
ASMJIT_PATCH(0x6EA6D3, TeamClass_CanAdd_ReplaceLoop, 0x7)
{
	GET(TechnoClass*, pGoingToBeRecuited, ESI);
	GET(TaskForceClass*, pForce, EDX);
	GET(TechnoTypeClass*, pThat, EAX);
	GET(int, idx, EDI);
	enum
	{
		Conditionmet = 0x6EA6F2,
		ContinueLoop = 0x6EA6DC
	};

	return pThat == pForce->Entries[idx].Type
		|| TeamExtData::IsEligible(pGoingToBeRecuited, pForce->Entries[idx].Type)
		//|| TeamExtData::GroupAllowed(pForce->Entries[idx].Type, pThat)
		//|| TeamExtData::GroupAllowed(pForce->Entries[idx].Type, TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type)
		? Conditionmet : ContinueLoop;
}
#endif
