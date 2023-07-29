#include "Body.h"

//TODO , there is some inlined array acces
// also the `ScriptTypeClass` detach is difficult to read ,..
DEFINE_HOOK(0x691518, ScriptClass_GetCurrentAction_extra, 0x6)
{
	GET(ScriptClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pNode, 0x4);

	if (pThis->CurrentMission < ScriptTypeClass::MaxActions) {
		*pNode = pThis->Type->ScriptActions[pThis->CurrentMission];
	} else {
		const int nCurIdx = MinImpl(pThis->CurrentMission , pThis->Type->ActionsCount);
		const auto pTypeExt = ScriptTypeExt::ExtMap.Find(pThis->Type);
		size_t const nIdx = size_t(nCurIdx - ScriptTypeClass::MaxActions);
		constexpr auto const nMax = ScriptTypeClass::MaxActions - 1;

		if (!pTypeExt->PhobosNode.empty() && !(nIdx > (int)pTypeExt->PhobosNode.size())) {
			*pNode = pTypeExt->PhobosNode[nIdx];
		} else {
			*pNode = pThis->Type->ScriptActions[nMax];
		}
	}

	R->EAX(pNode);
	return 0x691534;
}

DEFINE_HOOK(0x691540, ScriptClass_GetNextAction_extra, 0x6)
{
	GET(ScriptClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pNode, 0x4);

	auto nCurIndex = pThis->CurrentMission + 1;

	if (nCurIndex < ScriptTypeClass::MaxActions) {
		*pNode = pThis->Type->ScriptActions[nCurIndex];
	} else {

		nCurIndex = MinImpl(nCurIndex, pThis->Type->ActionsCount);
		const auto pTypeExt = ScriptTypeExt::ExtMap.Find(pThis->Type);
		size_t const nIdx = size_t(nCurIndex - ScriptTypeClass::MaxActions);
		constexpr auto const nMax = ScriptTypeClass::MaxActions - 1;

		if (!pTypeExt->PhobosNode.empty() && !(nIdx > (int)pTypeExt->PhobosNode.size())) {
			*pNode = pTypeExt->PhobosNode[nIdx];
		}else {
			*pNode = { TeamMissionType::none , 0 };
		}
	}

	R->EAX(pNode);
	return 0x69157E;
}

DEFINE_HOOK(0x6918A0, ScriptTypeClass_LoadFromINI, 0x5)
{
	GET(ScriptTypeClass*, pThis, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	pINI->Reset();
	if (!pThis->AbstractTypeClass::LoadFromINI(pINI)){
		R->EAX(false);
		return 0x69196C;
	}

	auto pExt = ScriptTypeExt::ExtMap.Find(pThis);

	//script consist of
	// Name=
	// 0 - 5=
	pThis->ActionsCount = 0;
	const auto nCount = pINI->GetKeyCount(pThis->ID);

	if (nCount > 0 && nCount < std::numeric_limits<int>::max()) {

		const auto nActionCount = nCount - 1;
		Debug::Log("Reading [%x - %s][%s] with %d ActionsCount!\n", pThis, pThis->ID, pThis->Name, nActionCount);
		pThis->ActionsCount = nActionCount;

		char buffer[0x10];
		for (int i = 0; i < nActionCount; ++i)
		{
			CRT::sprintf(buffer, "%d", i);
			CRT::strtrim(buffer);

			if (pINI->ReadString(pThis->ID, buffer, "", Phobos::readBuffer)) {

				ScriptActionNode nBuffer { Phobos::readBuffer };

				if (i < ScriptTypeClass::MaxActions) {
					pThis->ScriptActions[i].Action = nBuffer.Action;
					pThis->ScriptActions[i].Argument = nBuffer.Argument;
				}else{
					pExt->PhobosNode.push_back(nBuffer);
				}
			}
		}
	}

	R->EAX(true);
	return 0x69196C;
}

DEFINE_HOOK(0x6917F0, ScriptTypeClass_WriteToINI, 0x6)
{
	GET(ScriptTypeClass*, pThis, ECX);
	GET_STACK(CCINIClass*, pRules, 0x4);

	if (!pThis->AbstractTypeClass::SaveToINI(pRules)) {
		R->EAX(false);
		return 0x69188E;
	}

	char buffer[0x10];
	Debug::Log("Writing [%x - %s][%s] with %d ActionsCount!\n", pThis, pThis->ID, pThis->Name, pThis->ActionsCount);
	for (int i = 0; i < pThis->ActionsCount; ++i)
	{
		CRT::sprintf(buffer, "%d", i);
		CRT::strtrim(buffer);

		char bufferdata[0x80];
		if (i < ScriptTypeClass::MaxActions){
			pThis->ScriptActions[i].BuildINIEntry(bufferdata);
			pRules->WriteString(pThis->ID, buffer, bufferdata);
		} else {
			ScriptTypeExt::ExtMap.Find(pThis)->PhobosNode[i - ScriptTypeClass::MaxActions].BuildINIEntry(bufferdata);
			pRules->WriteString(pThis->ID, buffer, bufferdata);
		}
	}

	R->EAX(true);
	return 0x69188E;
}

DEFINE_HOOK(0x691E30, ScriptTypeClass_Detach, 0x6)
{
	GET(ScriptTypeClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(bool, remove, 0x8);

	if (auto pTeam = specific_cast<TeamTypeClass*>(pTarget))
	{
		if (pThis->ActionsCount > 0)
		{
			auto TeamTypeCount = TeamTypeClass::Array->Count;
			int data = -1;
			for (int i = pThis->ActionsCount; i > 0; --i)
			{
				ScriptActionNode* Action = i < ScriptTypeClass::MaxActions ?
					&pThis->ScriptActions[i] : &ScriptTypeExt::ExtMap.Find(pThis)->PhobosNode[i - ScriptTypeClass::MaxActions];

				if (Action->Action == TeamMissionType::Change_team)
				{
					if (data == 1)
					{
						int idx = -1;
						if (TeamTypeCount < 0)
							return 0x691EBD;
						else
						{
							for (int a = 0; a < TeamTypeCount; a++) {
								if (TeamTypeClass::Array->Items[a] == pTeam) {
									idx = a;
								}
							}
						}

						if(idx == -1)
							return 0x691EBD;

						data = idx;
					}

					if (Action->Argument > data) {
						--Action->Argument;
						continue;
					}

					if (Action->Argument == data)
					{
						Action->Argument = 0;
						continue;
					}
				}
			}
		}
	}

	return 0x691EBD;
}

//65ECA6