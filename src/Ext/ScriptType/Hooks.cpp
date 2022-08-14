#include "Body.h"

#ifdef ENABLE_NEWHOOKS_SCIPT
DEFINE_HOOK(0x691518, ScriptClass_GetCurrentAction_extra, 0x7)
{
	GET(ScriptClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pNode, 0x4);
	GET(int, nCurIndex, EDX);

	if (nCurIndex > pThis->Type->ActionsCount)
		nCurIndex = pThis->Type->ActionsCount;

	if (nCurIndex <= 50) {
		pNode->Action = pThis->Type->ScriptActions[nCurIndex].Action;
		pNode->Argument = pThis->Type->ScriptActions[nCurIndex].Argument;
	} else {
		const auto pTypeExt = ScriptTypeExt::ExtMap.Find(pThis->Type);
		if(pTypeExt && !pTypeExt->PhobosNode.empty()){
			pNode->Action = pTypeExt->PhobosNode[nCurIndex - 50].Action;
			pNode->Argument = pTypeExt->PhobosNode[nCurIndex - 50].Argument;
		} else {
			pNode->Action = pThis->Type->ScriptActions[50].Action;
			pNode->Argument = pThis->Type->ScriptActions[50].Argument;
		}
	}

	R->EAX(pNode);
	return 0x691534;
}

DEFINE_HOOK(0x691566, ScriptClass_GetNextAction_extra, 0xB)
{
	GET(ScriptTypeClass*, pType, EDX);
	GET_STACK(ScriptActionNode*, pNode, STACK_OFFS(0x4, -0x4));
	GET(int, nCurIndex, ECX);

	nCurIndex += 1;

	if (nCurIndex > pType->ActionsCount)
		nCurIndex = pType->ActionsCount;

	if (nCurIndex <= 50)
	{
		pNode->Action = pType->ScriptActions[nCurIndex].Action;
		pNode->Argument = pType->ScriptActions[nCurIndex].Argument;
	} else {
		const auto pTypeExt = ScriptTypeExt::ExtMap.Find(pType);
		if (pTypeExt && !pTypeExt->PhobosNode.empty()) {
			pNode->Action = pTypeExt->PhobosNode[nCurIndex - 50].Action;
			pNode->Argument = pTypeExt->PhobosNode[nCurIndex - 50].Argument;
		}else {
			pNode->Action = pType->ScriptActions[50].Action;
			pNode->Argument = pType->ScriptActions[50].Argument;
		}
	}

	R->EAX(pNode);
	return 0x69157D;
}

DEFINE_HOOK(0x6918CA, ScriptTypeClass_LoadFromINI, 0x5)
{
	GET(ScriptTypeClass*, pThis, ESI);
	LEA_STACK(char*, pBuffer, STACK_OFFS(0xA0, 0x90));
	GET(CCINIClass*, pRules, EBP);

	auto pExt = ScriptTypeExt::ExtMap.Find(pThis);

	pThis->ActionsCount = 0;
	auto nCount = pRules->GetKeyCount(pThis->ID);

	if (nCount > 0)
		nCount -= 1; //name

	if (nCount > 0) {
		for (int i = 0; i < nCount; ++i) {
			CRT::sprintf(pBuffer, "%d", i);
			CRT::strtrim(pBuffer);
			if (pRules->ReadString(pThis->ID, pBuffer, "", Phobos::readBuffer)) {
				ScriptActionNode nBuffer;
				nBuffer.FillIn(Phobos::readBuffer);
				if (i <= 50) {
					pThis->ScriptActions[i] = std::move(nBuffer);
				}else{
					pExt->PhobosNode.push_back(std::move(nBuffer));
				}

				if (++pThis->ActionsCount >= std::numeric_limits<int>::max())
					break;
			}
		}
	}

	pExt->LoadFromINI(pRules);

	return 0x691953;
}
#endif
