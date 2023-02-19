#include "Body.h"
#include <Ext/TechnoType/Body.h>

//Author : Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

DEFINE_HOOK(0x52190D, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6) //7
{
	GET(InfantryClass*, pThis, ESI);
	return pThis->Type->DeployFireWeapon == -1 ? 0x52194E : 0x0;
}

DEFINE_HOOK(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C };
	GET(UnitClass*, pThis, ESI);

	const auto pCell = pThis->GetCell();

	if(!pCell)
		return SkipGameCode;

	pThis->SetTarget(pCell);
	auto const nWeapIdx = pThis->SelectWeapon(pCell);
	auto const pWs = pThis->GetWeapon(nWeapIdx);

	if (!pWs->WeaponType)
	{
		pThis->QueueMission(Mission::Guard, true);
		return SkipGameCode;
	}

	if (pThis->GetFireError(pCell, nWeapIdx, true) == FireError::OK)
	{
		pThis->Fire(pThis->Target, nWeapIdx);
		const auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->DeployFireTimer.Start(pWs->WeaponType->ROF);

		if (pWs->WeaponType->FireOnce)
		{
			pThis->SetTarget(nullptr);
			pThis->QueueMission(Mission::Guard, true);
			const auto& missionControl = MissionControlClass::Controls[(int)Mission::Unload];
			int delay = Game::F2I(missionControl.Rate * 900 + ScenarioClass::Instance->Random(0, 2));
			pExt->DeployFireTimer.Start(Math::min(pWs->WeaponType->ROF, delay));
		}
	}

	return 0x73DD3C;
}

DEFINE_HOOK(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = specific_cast<UnitClass*>(pThis);

	/// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit && pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer)
	{
		const auto pCell = pThis->GetCell();
		pThis->SetTarget(pCell);
		auto const nWeapIdx = pThis->SelectWeapon(pCell);
		auto const pWs = pThis->GetWeapon(nWeapIdx);

		if (!pWs->WeaponType)
			return DoNotExecute;

		if (pWs->WeaponType->FireOnce)
		{
			if (TechnoExt::ExtMap.Find(pThis)->DeployFireTimer.HasTimeLeft())
				return DoNotExecute;
		}
	}

	return 0;
}

DEFINE_HOOK(0x746CD0, UnitClass_SelectWeapon_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->Type;

	if (pThis->Deployed && pType->DeployFire)
	{
		int weaponIndex = pType->DeployFireWeapon;
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				if (pThis->GetFireError(pTarget, weaponIndex, true) != FireError::OK)
					weaponIndex = 1;
			}
		}

		if (weaponIndex != -1)
		{
			R->EAX(weaponIndex);
			return 0x746CFD;
		}
	}

	R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	return 0x746CFD;
}