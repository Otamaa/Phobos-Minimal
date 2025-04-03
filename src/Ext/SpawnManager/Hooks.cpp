#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

// TODO : revisit
// ASMJIT_PATCH_AGAIN(0x6B73B9, SpawnManagerClass_AI_SpawnTimer, 0x5)
// ASMJIT_PATCH(0x6B73A8, SpawnManagerClass_AI_SpawnTimer, 0x5)
// {
// 	GET(SpawnManagerClass* const, pThis, ESI);
//
// 	if (pThis->Owner)
// 	{
// 		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());
//
// 		if (pTypeExt->Spawner_DelayFrames.isset())
// 			R->ECX(pTypeExt->Spawner_DelayFrames.Get());
// 	}
//
// 	return 0;
// }

// TODO : revisit
// ASMJIT_PATCH(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
// {
// 	GET(SpawnManagerClass* const, pThis, ESI);
//
// 	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching
// 		&& pThis->CountDockedSpawns() != 0)
// 	{
// 		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());
// 		R->EAX(pTypeExt->Spawner_DelayFrames.Get(10));
// 	}
//
// 	return 0;
// }

//ASMJIT_PATCH(0x6B743E, SpawnManagerClass_AI_SpawnOffsets, 0x6)
//{
//	GET(TechnoClass*, pOwner, ECX);
//	//yes , i include the buffer just in case it used somewhere !
//	LEA_STACK(CoordStruct*, pBuffer, STACK_OFFS(0x68, 0x18));
//	LEA_STACK(CoordStruct*, pBuffer2, STACK_OFFS(0x68, 0xC));
//
//	auto const pExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());
//
//	if (pExt->Spawner_SpawnOffsets.isset())
//	{
//		if (pExt->Spawner_SpawnOffsets_OverrideWeaponFLH)
//		{
//			auto const pRet = pExt->Spawner_SpawnOffsets.GetEx();
//			pBuffer = pRet;
//			pBuffer2 = pRet;
//			R->EAX(pRet);
//		}
//		else
//		{
//			CoordStruct FLH = pExt->Spawner_SpawnOffsets.Get();
//			if (pOwner->CurrentBurstIndex)
//			{
//				auto const pRet = pOwner->GetFLH(pBuffer, R->EBP<int>(), pExt->Get()->SecondSpawnOffset);
//				pRet->X += FLH.X;
//				pRet->Y += FLH.Y;
//				pRet->Z += FLH.Z;
//				R->EAX(pRet);
//			}
//			else
//			{
//				auto const pRet = pOwner->GetFLH(pBuffer2, R->EBP<int>(), CoordStruct::Empty);
//				pRet->X += FLH.X;
//				pRet->Y += FLH.Y;
//				pRet->Z += FLH.Z;
//				R->EAX(pRet);
//			}
//		}
//
//		return 0x6B7498;
//	}
//
//	return 0x0;
//}


ASMJIT_PATCH(0x6B6D44, SpawnManagerClass_Init_Spawns, 0x5)
{
	enum { Jump = 0x6B6DF0, Change = 0x6B6D53, Continue = 0 };

	GET(SpawnManagerClass*, pThis, ESI);


	GET_STACK(size_t, i, STACK_OFFSET(0x1C, 0x4));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());

	if ((int)i >= pTypeExt->InitialSpawnsNumber.Get(pThis->SpawnCount)) {

		GET(SpawnNode*, pControl, EBP);

		pControl->Unit = nullptr;
		pControl->NodeSpawnTimer.Start(pThis->RegenRate);
		pControl->Status = SpawnNodeStatus::Dead;
		pThis->SpawnedNodes.AddItem(pControl);
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
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());

	if (pTypeExt->Spawns_Queue.empty())
		return 0;

	std::vector<AircraftTypeClass*> vec = pTypeExt->Spawns_Queue;

	for (auto& pNode : pThis->SpawnedNodes) {
		if (pNode->Unit) {
			fast_remove_if(vec, [=](auto pType) { return pType == pNode->Unit->GetTechnoType(); });
		}
	}

	if (vec.empty() || !vec[0])
		return 0;


	R->EAX(vec[0]->CreateObject(pThis->Owner->GetOwningHouse()));
	return 0x6B78EA;
}
