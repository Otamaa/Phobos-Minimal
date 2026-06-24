#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>

#include <Utilities/Macro.h>

//Author : Otamaa
ASMJIT_PATCH(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	int weapon = pThis->Type->DeployFireWeapon;
	if (pThis->Type->DeployFireWeapon == -1) {
		if (pThis->Target && (pThis->Target->WhatAmI() == CellClass::AbsID || pThis->Target->AbstractFlags & AbstractFlags::Techno && ((TechnoClass*)pThis->Target)->IsAlive)) {
			weapon = pThis->SelectWeapon(pThis->Target);
		} else {
			weapon = 0;
		}
	}

	R->EDI(weapon);
	return 0x5223B9;
}

ASMJIT_PATCH(0x5218F3, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->DeployFireWeapon == -1)
		return 0x52194E;

	if (pThis->Type->IsGattling || TechnoTypeExtContainer::Instance.Find(pThis->Type)->MultiWeapon.Get())
		return !pThis->IsDeployed() ? 0x52194E : 0x52190D;

	if(pThis->IsDeployed())
		return 0x52190D;

	return 0x521917;
}


#ifndef DISABLEFORTESTINGS

ASMJIT_PATCH(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C, SetMissionGuard = 0x73DEBA };

	GET(UnitClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pExt->DeployFireTimer.InProgress())
	{
		auto const nWeapIdx = TechnoExtData::GetDeployFireWeapon(pThis);
		auto pTarget = pThis->GetCell();
		pThis->SetTarget(pTarget);

		if (pThis->GetFireError(pTarget, nWeapIdx, true) == FireError::OK)
		{
			auto pWeapon = pThis->GetWeapon(nWeapIdx);

			pThis->Fire(pThis->GetCell(), nWeapIdx);

			if (pWeapon->WeaponType->FireOnce) {
				R->EBX(0);
				return SetMissionGuard;
			}
		}
	}
	else
	{
		pThis->SetTarget(nullptr);
		R->EBX(0);
		return SetMissionGuard;
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x746CD0, UnitClass_WhatWeaponShouldIUse_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	//GET_STACK(uintptr_t, callerAddress, 0x0);

	if (pThis->Deployed && pThis->Type->DeployFire) {
		if (pThis->Type->DeployFireWeapon != -1) {
			R->EAX(pThis->Type->DeployFireWeapon);
			return 0x746CFD;
		}
	}

	//if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
	//	if(!pObj->IsAlive) {
	//		//Debug::LogInfo("[{}] {} {} Attempt to target death Object of {}!"
	//		//	, callerAddress ,(void*)pThis , pThis->get_ID() , (void*)pTarget);

	//		pTarget = nullptr;
	//	}
	//}

	R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	return 0x746CFD;
}
#endif

ASMJIT_PATCH(0x51ECC0, InfantryClass_MouseOverObject_IsAreaFire, 0xA)
{
	enum { IsAreaFire = 0x51ECE5, NotAreaFire = 0x51ECEC };

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	const int deployWeaponIdx = pThis->Type->DeployFireWeapon;
	const auto deployWeapon = pThis->GetWeapon(deployWeaponIdx >= 0 ? deployWeaponIdx : pThis->SelectWeapon(pObject))->WeaponType;

	return deployWeapon && deployWeapon->AreaFire ? IsAreaFire : NotAreaFire;
}

ASMJIT_PATCH(0x6F7666, TechnoClass_TriggersCellInset_DeployWeapon, 0x8)
{
	enum { NotAreaFire = 0x6F7776, ContinueIn = 0x6F7682 };

	GET(TechnoClass*, pThis, ESI);
	int weaponIdx;

	if (const auto pInfantry = cast_to<InfantryClass*>(pThis))
	{
		GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x28, 0x4));
		const int deployWeaponIdx = pInfantry->Type->DeployFireWeapon;
		weaponIdx = deployWeaponIdx >= 0 ? deployWeaponIdx : pThis->SelectWeapon(pTarget);
	}
	else
	{
		weaponIdx = pThis->IsNotSprayAttack();
	}

	const auto deployWeaponStruct = pThis->GetWeapon(weaponIdx);
	return deployWeaponStruct && deployWeaponStruct->WeaponType && deployWeaponStruct->WeaponType->AreaFire ? ContinueIn : NotAreaFire;
}