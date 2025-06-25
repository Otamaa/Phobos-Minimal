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