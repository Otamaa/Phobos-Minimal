#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Aircraft/Body.h>

ASMJIT_PATCH(0x6B7867, SpawnManagerClass_AI_MoveTo7ifDies, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(TechnoClass*, pSpawnee, EDI);
	GET(int, idx, EBX);

	if (!pSpawnee)
	{
		pThis->SpawnedNodes.Items[idx]->Status = SpawnNodeStatus::Dead;
		return 0x6B727F;
	}

	return 0x0;
}

ASMJIT_PATCH(0x6B71E7, SpawnManagerClass_Manage_AlreadyNull, 0xA)
{
	GET(SpawnNode*, pNode, EDX);

	if (pNode->Unit && pNode->Unit->IsAlive)
	{
		pNode->Unit->UnInit(); // call detach function for everyone
	}

	return 0x6B71F1;
}

ASMJIT_PATCH(0x6B7759, SpawnManagerClass_AI_State4And3_DeadTechno, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(int, idx, EBX);

	if (!pThis->SpawnedNodes.Items[idx]->Unit || !pThis->SpawnedNodes.Items[idx]->Unit->IsAlive)
	{
		pThis->SpawnedNodes.Items[idx]->Status = SpawnNodeStatus::Dead;
		pThis->SpawnedNodes.Items[idx]->Unit = nullptr;

		if (!pThis->SpawnedNodes.Items[idx]->Unit->IsAlive)
			pThis->SpawnedNodes.Items[idx]->NodeSpawnTimer.Start(pThis->RegenRate);

		return 0x6B727F;
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6B770D, SpawnManagerClass_AI_State4And3_DeadTechno, 0x7)

ASMJIT_PATCH(0x6B6D44, SpawnManagerClass_Init_Spawns, 0x5)
{
	enum { Jump = 0x6B6DF0, Change = 0x6B6D53, Continue = 0 };

	GET(SpawnManagerClass*, pThis, ESI);


	GET_STACK(size_t, i, STACK_OFFSET(0x1C, 0x4));

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis->Owner);

	if ((int)i >= pTypeExt->InitialSpawnsNumber.Get(pThis->SpawnCount)) {

		GET(SpawnNode*, pControl, EBP);

		pControl->Unit = nullptr;
		pControl->NodeSpawnTimer.Start(pThis->RegenRate);
		pControl->Status = SpawnNodeStatus::Dead;
		pThis->SpawnedNodes.push_back(pControl);
		return Jump;
	}

	if (pTypeExt->Spawns_Queue.size() <= i || !pTypeExt->Spawns_Queue[i])
		return Continue;

	R->EAX(pTypeExt->Spawns_Queue[i]->CreateObject(pThis->Owner->GetOwningHouse()));
	return Change;

}

ASMJIT_PATCH(0x6B78D3, SpawnManagerClass_Update_Spawns, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis->Owner);

	if (pTypeExt->Spawns_Queue.empty())
		return 0;

	std::vector<AircraftTypeClass*> vec = pTypeExt->Spawns_Queue;

	for (auto& pNode : pThis->SpawnedNodes) {
		if (pNode->Unit) {
			fast_remove_if(vec, [=](auto pType) { return pType == pNode->Unit->Type; });
		}
	}

	if (vec.empty() || !vec[0])
		return 0;

	R->EAX(vec[0]->CreateObject(pThis->Owner->GetOwningHouse()));
	return 0x6B78EA;
}