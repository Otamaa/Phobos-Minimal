#ifdef COMPILE_PORTED_DP_FEATURES
#include "SpawnSupportFunctional.h"
#include "SpawnSupportData.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include "../../Helpers/Helpers.h"

void SpawnSupportFunctional::Construct(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::GetExtData(pType);
	auto supportWeapon = pTypeExt->MySpawnSupportDatas.SupportWeapon;

	if (supportWeapon.Get()) {
		if (!pThis->SpawnManager) {
			pThis->SpawnManager = GameCreate<SpawnManagerClass>(pThis, pType->Spawns, pType->SpawnsNumber, pType->SpawnRegenRate, pType->SpawnReloadRate);
		}
	}
}

void SpawnSupportFunctional::FireSupportWeaponToSpawn(TechnoClass* pThis ,bool InUpdateFunc , bool useROF)
{
	if (!pThis)
		return;

	auto const pSpawnOwner = pThis->SpawnOwner;

	if (!pSpawnOwner)
		return;

	auto const pExt = TechnoExt::GetExtData(pSpawnOwner);
	auto const pTypeExt = TechnoTypeExt::GetExtData(pSpawnOwner->GetTechnoType());

	if (!pExt || !pTypeExt)
		return;

	if (pTypeExt->MySpawnSupportDatas.FireOnce)
	{
		if (!pExt->MySpawnSuport.spawnFireFlag)
		{
			pExt->MySpawnSuport.spawnFireOnceDelay.Start(pTypeExt->MySpawnSupportDatas.Delay);
			pExt->MySpawnSuport.spawnFireFlag = true;
		}
		else
		{
			if (!pExt->MySpawnSuport.spawnFireOnceDelay.InProgress())
			{
				pSpawnOwner->SpawnManager->Target = nullptr;
				pSpawnOwner->SpawnManager->SetTarget(nullptr);
			}
		}
	}

	if (auto pSpawn = pSpawnOwner->SpawnManager)
	{
		if (!pSpawn->Target && !pSpawn->NewTarget)
		{
			pExt->MySpawnSuport.spawnFireFlag = false;
		}
	}

	if (pTypeExt->MySpawnSupportDatas.Enable && pTypeExt->MySpawnSupportDatas.SupportWeapon)
	{
		if (InUpdateFunc && !pTypeExt->MySpawnSupportDatas.Always.Get())
			return;

		if (!InUpdateFunc && pTypeExt->MySpawnSupportDatas.Always.Get())
			return;

		CoordStruct nFLH { 0,0,0 };

		SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;

		if (auto const pTransporter = pSpawnOwner->Transporter)
		{
			if(auto const pTransportExt = TechnoTypeExt::GetExtData(pTransporter->GetTechnoType()))
			{
				nFLHData = pTransportExt->MySpawnSupportFLH;
			}
		}

		auto const pSupportWeapon = pSpawnOwner->Veterancy.IsElite() ?
			pTypeExt->MySpawnSupportDatas.EliteSupportWeapon : pTypeExt->MySpawnSupportDatas.SupportWeapon;

		nFLH = pSpawnOwner->Veterancy.IsElite() ? nFLHData.EliteSpawnSupportFLH : nFLHData.SpawnSupportFLH;

		if (pTypeExt->MySpawnSupportDatas.SwitchFLH)
		{
			nFLH.Y *= pExt->MySpawnSuport.supportFLHMult;
			pExt->MySpawnSuport.supportFLHMult *= -1;
		}

		if (useROF && pExt->MySpawnSuport.supportFireROF.InProgress())
			return;

		auto nSourcePos = Helpers_DP::GetFLHAbsoluteCoords(pSpawnOwner, nFLH, true);
		auto nTargetPos = Helpers_DP::GetFLHAbsoluteCoords(pThis, nFLH, true);
		VelocityClass nVel = Helpers_DP::GetBulletVelocity(nSourcePos, nTargetPos);

		Helpers_DP::FireBulletTo(pSpawnOwner, pThis, pSupportWeapon, nSourcePos, nTargetPos, nVel);

		if (useROF)
			pExt->MySpawnSuport.supportFireROF.Start(pSupportWeapon->ROF);

	}
}

void SpawnSupportFunctional::AI(TechnoClass* pThis)
{
	SpawnSupportFunctional::FireSupportWeaponToSpawn(pThis,true,true);
}

void SpawnSupportFunctional::OnFire(TechnoClass* pThis)
{
	SpawnSupportFunctional::FireSupportWeaponToSpawn(pThis);
}
#endif