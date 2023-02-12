#ifdef COMPILE_PORTED_DP_FEATURES
#include "SpawnSupportFunctional.h"
#include "SpawnSupportData.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include "../../Helpers/Helpers.h"

void SpawnSupportFunctional::Construct(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const& pSupportWeapon = pThis->Veterancy.IsElite() ?
		pTypeExt->MySpawnSupportDatas.EliteSupportWeapon : pTypeExt->MySpawnSupportDatas.SupportWeapon;


	if (pTypeExt->MySpawnSupportDatas.Enable && pSupportWeapon.Get())
	{
		if (!pThis->SpawnManager) {
			pThis->SpawnManager = GameCreate<SpawnManagerClass>(pThis, pType->Spawns, pType->SpawnsNumber, pType->SpawnRegenRate, pType->SpawnReloadRate);
		}
	}
}

void SpawnSupportFunctional::FireSupportWeaponToSpawn(TechnoClass* pThis, AbstractClass* pTarget, bool useROF)
{
	//auto const pSpawnOwner = pThis->SpawnOwner;

	//if (!pSpawnOwner)
	//	return;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->MySpawnSupportDatas.Enable)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		//if (pTypeExt->MySpawnSupportDatas.Always.Get())
		//	return;

		//CoordStruct nFLH { 0,0,0 };

		//SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;

		//if (auto const pTransporter = pThis->Transporter)
		//{
		//	if(auto const pTransportExt = TechnoTypeExt::ExtMap.Find<false>(pTransporter->GetTechnoType()))
		//	{
		//		nFLHData = pTransportExt->MySpawnSupportFLH;
		//	}
		//}

		auto const& pSupportWeapon = pThis->Veterancy.IsElite() ?
			pTypeExt->MySpawnSupportDatas.EliteSupportWeapon : pTypeExt->MySpawnSupportDatas.SupportWeapon;

		if ((useROF && pExt->MySpawnSuport.supportFireROF.InProgress()) || !pSupportWeapon.Get())
			return;

		//nFLH = pThis->Veterancy.IsElite() ? nFLHData.EliteSpawnSupportFLH : nFLHData.SpawnSupportFLH;

		//if (pTypeExt->MySpawnSupportDatas.SwitchFLH)
		//{
		//	nFLH.Y *= pExt->MySpawnSuport.supportFLHMult;
		//	pExt->MySpawnSuport.supportFLHMult *= -1;
		//}

		//if(!InUpdateFunc) {
		//	auto nSourcePos = TechnoExt::GetFLHAbsoluteCoords(pThis, nFLH, true);
		//	auto nTargetPos = TechnoExt::GetFLHAbsoluteCoords(pThis, nFLH, true);
		//	VelocityClass nVel = Helpers_DP::GetBulletVelocity(nSourcePos, nTargetPos);

		//	Helpers_DP::FireBulletTo(pSpawnOwner, pThis, pSupportWeapon, nSourcePos, nTargetPos, nVel);
		//}

		pThis->SpawnManager->SetTarget(pTarget);

		if (useROF)
			pExt->MySpawnSuport.supportFireROF.Start(pSupportWeapon->ROF);

	}
}

DEFINE_HOOK(0x6B743E , SpawnManagerAI_SpawnSupportFLH, 0x8)
{
	GET(SpawnManagerClass*, pSpawn, ESI);
	//GET_STACK(int, nArrIdx, STACK_OFFS(0x68, 0x54));

	if (auto pOwner = pSpawn->Owner)
	{
		//if ((*pFLH) == CoordStruct::Empty)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

			if (pTypeExt->MySpawnSupportDatas.Enable)
			{
				//CoordStruct nFLH = CoordStruct::Empty;

				SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;
				if (auto const pTransporter = pOwner->Transporter)
				{
					if (auto const pTransportExt = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType()))
					{
						nFLHData = pTransportExt->MySpawnSupportFLH;
					}
				}

				CoordStruct nFLH = pOwner->Veterancy.IsElite() ? nFLHData.EliteSpawnSupportFLH : nFLHData.SpawnSupportFLH;

				if (nFLH == CoordStruct::Empty)
					return 0x0;

				if(auto pSpawnExt = TechnoExt::ExtMap.Find(pOwner)){
					if (pTypeExt->MySpawnSupportDatas.SwitchFLH)
					{
						nFLH.Y *= pSpawnExt->MySpawnSuport.supportFLHMult;
						pSpawnExt->MySpawnSuport.supportFLHMult *= -1;
					}
				}

				R->EAX(&nFLH);
				return 0x6B7498;
			}
		}
	}


	return 0x0;
}
void SpawnSupportFunctional::AI(TechnoClass* pThis)
{
	//auto const pSpawnOwner = pThis->SpawnOwner;

	//if (!pSpawnOwner)
	//	return;

	//auto const pExt = TechnoExt::ExtMap.Find<false>(pSpawnOwner);
	//auto const pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pSpawnOwner->GetTechnoType());

	//if (!pExt || !pTypeExt)
	//	return;

	//if (pTypeExt->MySpawnSupportDatas.FireOnce)
	//{
	//	if (!pExt->MySpawnSuport.spawnFireFlag)
	//	{
	//		pExt->MySpawnSuport.spawnFireOnceDelay.Start(pTypeExt->MySpawnSupportDatas.Delay);
	//		pExt->MySpawnSuport.spawnFireFlag = true;
	//	}
	//	else
	//	{
	//		if (!pExt->MySpawnSuport.spawnFireOnceDelay.InProgress())
	//		{
	//			pSpawnOwner->SpawnManager->Target = nullptr;
	//			pSpawnOwner->SpawnManager->SetTarget(nullptr);
	//		}
	//	}
	//}

	//if (auto pSpawn = pSpawnOwner->SpawnManager)
	//{
	//	if (!pSpawn->Target && !pSpawn->NewTarget)
	//	{
	//		pExt->MySpawnSuport.spawnFireFlag = false;
	//	}
	//}

}

void SpawnSupportFunctional::OnFire(TechnoClass* pThis, AbstractClass* pTarget)
{
	SpawnSupportFunctional::FireSupportWeaponToSpawn(pThis, pTarget,true);
}
#endif