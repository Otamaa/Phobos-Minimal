#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

namespace UnitDeployConvertHelpers
{
	void RemoveDeploying(REGISTERS* R);
}

void UnitDeployConvertHelpers::RemoveDeploying(REGISTERS* R)
{
	GET(UnitClass*, pThis, ESI);

	auto const pThisType = TechnoTypeExt::ExtMap.Find(pThis->Type);
	const bool canDeploy = pThis->CanDeploySlashUnload();

	R->AL(canDeploy);
	if (!canDeploy)
		return;

	const bool skipMinimum = pThisType->Ammo_DeployUnlockMinimumAmount < 0;
	const bool skipMaximum = pThisType->Ammo_DeployUnlockMaximumAmount < 0;

	if (skipMinimum && skipMaximum)
		return;


	const bool moreThanMinimum = pThis->Ammo >= pThisType->Ammo_DeployUnlockMinimumAmount;
	const bool lessThanMaximum = pThis->Ammo <= pThisType->Ammo_DeployUnlockMaximumAmount;

	if ((skipMinimum || moreThanMinimum) && (skipMaximum || lessThanMaximum))
		return;

	R->AL(false);
}


DEFINE_HOOK(0x73FFE6, UnitClass_WhatAction_RemoveDeploying, 0xA)
{
	UnitDeployConvertHelpers::RemoveDeploying(R);
	return 0x73FFF0;
}

DEFINE_HOOK(0x730C70, DeployClass_Execute_RemoveDeploying, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	if (specific_cast<UnitClass*>(pThis)) {
		UnitDeployConvertHelpers::RemoveDeploying(R);
		return 0x730C7A;
	}

	return 0x0;
}

DEFINE_HOOK(0x739C70, UnitClass_ToggleDeployState_ChangeAmmo, 0xA) // deploying
{
	GET(UnitClass*, pThis, ESI);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Deployed && !pThis->Deploying && pThisExt->Ammo_AddOnDeploy)
	{
		const int ammoCalc = MaxImpl(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = MinImpl(pThis->Type->Ammo, ammoCalc);
	}
	return 0x0;
}

DEFINE_HOOK(0x739E6E, UnitClass_ToggleSimpleDeploy_ChangeAmmo, 0xA) // undeploying
{
	GET(UnitClass*, pThis, ESI);
	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Deployed && !pThis->Deploying && pThisExt->Ammo_AddOnDeploy)
	{
		const int ammoCalc = MaxImpl(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = MinImpl(pThis->Type->Ammo, ammoCalc);
	}

	return 0x0;
}

DEFINE_HOOK(0x73DE78, UnitClass_Unload_ChangeAmmo, 0x6) // converters
{
	GET(UnitClass*, pThis, ESI);

	auto const pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Type->IsSimpleDeployer && pThisExt->Ammo_AddOnDeploy && (pThis->Type->UnloadingClass == nullptr))
	{
		const int ammoCalc = MaxImpl(pThis->Ammo + pThisExt->Ammo_AddOnDeploy, 0);
		pThis->Ammo = MinImpl(pThis->Type->Ammo, ammoCalc);
	}

	R->AL(pThis->Deployed);
	return 0x73DE7E;
}