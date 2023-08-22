#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

// TODO : revisit
// DEFINE_HOOK_AGAIN(0x6B73B9, SpawnManagerClass_AI_SpawnTimer, 0x5)
// DEFINE_HOOK(0x6B73A8, SpawnManagerClass_AI_SpawnTimer, 0x5)
// {
// 	GET(SpawnManagerClass* const, pThis, ESI);

// 	if (pThis->Owner)
// 	{
// 		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

// 		if (pTypeExt->Spawner_DelayFrames.isset())
// 			R->ECX(pTypeExt->Spawner_DelayFrames.Get());
// 	}

// 	return 0;
// }

// TODO : revisit
// DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
// {
// 	GET(SpawnManagerClass* const, pThis, ESI);

// 	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching
// 		&& pThis->CountDockedSpawns() != 0)
// 	{
// 		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
// 		R->EAX(pTypeExt->Spawner_DelayFrames.Get(10));
// 	}

// 	return 0;
// }

DEFINE_HOOK(0x6B743E, SpawnManagerClass_AI_SpawnOffsets, 0x6)
{
	GET(TechnoClass*, pOwner, ECX);
	//yes , i include the buffer just in case it used somewhere !
	LEA_STACK(CoordStruct*, pBuffer, STACK_OFFS(0x68, 0x18));
	LEA_STACK(CoordStruct*, pBuffer2, STACK_OFFS(0x68, 0xC));

	auto const pExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

	if (pExt->Spawner_SpawnOffsets.isset())
	{
		if (pExt->Spawner_SpawnOffsets_OverrideWeaponFLH)
		{
			auto const pRet = pExt->Spawner_SpawnOffsets.GetEx();
			pBuffer = pRet;
			pBuffer2 = pRet;
			R->EAX(pRet);
		}
		else
		{
			CoordStruct FLH = pExt->Spawner_SpawnOffsets.Get();
			if (pOwner->CurrentBurstIndex)
			{
				auto const pRet = pOwner->GetFLH(pBuffer, R->EBP<int>(), pExt->Get()->SecondSpawnOffset);
				pRet->X += FLH.X;
				pRet->Y += FLH.Y;
				pRet->Z += FLH.Z;
				R->EAX(pRet);
			}
			else
			{
				auto const pRet = pOwner->GetFLH(pBuffer2, R->EBP<int>(), CoordStruct::Empty);
				pRet->X += FLH.X;
				pRet->Y += FLH.Y;
				pRet->Z += FLH.Z;
				R->EAX(pRet);
			}
		}

		return 0x6B7498;
	}

	return 0x0;
}
