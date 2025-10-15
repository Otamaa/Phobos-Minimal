#include "SpawnSupportFunctional.h"
#include "SpawnSupportData.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include "../../Helpers/Helpers.h"

#include <SpawnManagerClass.h>

void SpawnSupportFunctional::Construct(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
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
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->MySpawnSupportDatas.Enable)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		auto pSpawn = pExt->Get_SpawnSupport();
		if(!pSpawn)
			pSpawn = &Phobos::gEntt->emplace<SpawnSupport>(pExt->MyEntity);

		//if (pTypeExt->MySpawnSupportDatas.Always.Get())
		//	return;

		//CoordStruct nFLH { 0,0,0 };

		//SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;

		//if (auto const pTransporter = pThis->Transporter)
		//{
		//	if(auto const pTransportExt = TechnoTypeExtContainer::Instance.Find<false>(pTransporter->GetTechnoType()))
		//	{
		//		nFLHData = pTransportExt->MySpawnSupportFLH;
		//	}
		//}

		auto const& pSupportWeapon = pThis->Veterancy.IsElite() ?
			pTypeExt->MySpawnSupportDatas.EliteSupportWeapon : pTypeExt->MySpawnSupportDatas.SupportWeapon;

		if ((useROF && pSpawn->supportFireROF.InProgress()) || !pSupportWeapon.Get())
			return;

		//nFLH = pThis->Veterancy.IsElite() ? nFLHData.EliteSpawnSupportFLH : nFLHData.SpawnSupportFLH;

		//if (pTypeExt->MySpawnSupportDatas.SwitchFLH)
		//{
		//	nFLH.Y *= pExt->MySpawnSuport.supportFLHMult;
		//	pExt->MySpawnSuport.supportFLHMult *= -1;
		//}

		//if(!InUpdateFunc) {
		//	auto nSourcePos = TechnoExtData::GetFLHAbsoluteCoords(pThis, nFLH, true);
		//	auto nTargetPos = TechnoExtData::GetFLHAbsoluteCoords(pThis, nFLH, true);
		//	VelocityClass nVel = Helpers_DP::GetBulletVelocity(nSourcePos, nTargetPos);

		//	Helpers_DP::FireBulletTo(pSpawnOwner, pThis, pSupportWeapon, nSourcePos, nTargetPos, nVel);
		//}

		pThis->SpawnManager->SetTarget(pTarget);

		if (useROF)
			pSpawn->supportFireROF.Start(pSupportWeapon->ROF);

	}
}

void SpawnSupportFunctional::AI(TechnoClass* pThis)
{
	//auto const pSpawnOwner = pThis->SpawnOwner;

	//if (!pSpawnOwner)
	//	return;

	//auto const pExt = TechnoExtContainer::Instance.Find<false>(pSpawnOwner);
	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find<false>(pSpawnOwner->GetTechnoType());

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
