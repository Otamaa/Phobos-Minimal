#include "Body.h"

#include <Ext/TechnoType/Body.h>

// Take NewINIFormat into account just like the other classes does
// Author: secsome
DEFINE_HOOK(0x6E95B3, TeamClass_AI_MoveToCell, 0x6)
{
	if (!R->BL())
		return 0x6E95A4;

	GET(int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFS(0x38, 0x28));

	// if ( NewINIFormat < 4 ) then divide 128
	// in other times we divide 1000
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;
	cell.X = static_cast<short>(nCoord % nDivisor);
	cell.Y = static_cast<short>(nCoord / nDivisor);

	R->EAX(MapClass::Instance->GetCellAt(cell));
	return 0x6E959C;
}

//6EF57F
//DEFINE_HOOK(0x6EF577, TeamClass_GetMissionMemberTypes, 0x6)
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
//DEFINE_HOOK(0x50968F, HouseClass_CreateTeam_recuitType, 0x6)
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
#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x509697, HouseClass_CanInstansiateTeam_CompareType_Convert, 0xA)
{
	enum
	{
		ContinueCheck = 0x5096A5,
		ContinueLoop = 0x5096B4
	};
	GET(FootClass*, pGoingToBeRecuited, ESI);
	GET(UnitTypeClass*, pTaskForceTeam, EBX);

	const auto pGoingToBeRecuitedType = pGoingToBeRecuited->GetTechnoType();
	return pGoingToBeRecuitedType == pTaskForceTeam
		|| TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type == pTaskForceTeam
		//|| TeamExtData::GroupAllowed(pTaskForceTeam, pGoingToBeRecuitedType)
		//|| TeamExtData::GroupAllowed(pTaskForceTeam, TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type)
		?
		ContinueCheck : ContinueLoop;
}

bool IsActuallySameType(UnitClass* pGoing, TechnoTypeClass* pTaskforce) {

	if (pGoing->Type == pTaskforce)
		return true;

	//if (IS_SAME_STR_("SURVEY", pTaskforce->ID) && IS_SAME_STR_("SURVEYUP", pGoing->Type->ID)) {
	//	Debug::Log("Recuiting SURVEY for [%s]\n", pGoing->Type->ID);
	//}

	if (TechnoExtContainer::Instance.Find(pGoing)->Type == pTaskforce)
		return true;

	return false;
}

DEFINE_HOOK(0x6EA8FA, TeamClass_Remove_CompareType_Convert, 0x6)
{
	enum {
		retSame = 0x6EA91B,
		retNotSame = 0x6EA905
	};

	GET(FootClass*, pTeam, EBP);
	GET(DWORD, val1, EBX);
	GET(DWORD, val2, ESI);

	TechnoTypeClass* pTaskForceTeam = reinterpret_cast<TechnoTypeClass*>(val1 + val2);

	return pTeam->GetTechnoType() == pTaskForceTeam
		|| TechnoExtContainer::Instance.Find(pTeam)->Type == pTaskForceTeam
		? retSame : retNotSame;
}

DEFINE_HOOK(0x6EAD86, TeamClass_CanAdd_CompareType_Convert_UnitType, 0x7) //6
{
	enum
	{
		ContinueCheck = 0x6EAD8F,
		ContinueLoop = 0x6EADB3
	};

	GET(UnitClass*, pGoingToBeRecuited, ESI);
	//GET(UnitTypeClass*, pGoingToBeRecuitedType, EAX);
	GET(TaskForceClass*, pTeam, EDX);
	GET(int, nMemberIdx, EBP);

	return IsActuallySameType(pGoingToBeRecuited , pTeam->Entries[nMemberIdx].Type)
		//pGoingToBeRecuitedType == pTeam->Entries[nMemberIdx].Type
		//|| TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type == pTeam->Entries[nMemberIdx].Type
		//|| TeamExtData::GroupAllowed(pTeam->Entries[nMemberIdx].Type, pGoingToBeRecuitedType)
		//|| TeamExtData::GroupAllowed(pTeam->Entries[nMemberIdx].Type, TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type)
		?
		ContinueCheck : ContinueLoop;
}

DEFINE_HOOK(0x6EA6D3, TeamClass_CanAdd_ReplaceLoop, 0x7)
{
	GET(TechnoClass*, pGoingToBeRecuited, ESI);
	GET(TaskForceClass*, pForce, EDX);
	GET(TechnoTypeClass*, pThat, EAX);
	GET(int, idx, EDI);
	enum
	{
		ContinueCheck = 0x6EA6F2,
		ContinueLoop = 0x6EA6DC
	};

	return pThat == pForce->Entries[idx].Type
		|| TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type == pForce->Entries[idx].Type
		//|| TeamExtData::GroupAllowed(pForce->Entries[idx].Type, pThat)
		//|| TeamExtData::GroupAllowed(pForce->Entries[idx].Type, TechnoExtContainer::Instance.Find(pGoingToBeRecuited)->Type)
		? ContinueCheck : ContinueLoop;
}

