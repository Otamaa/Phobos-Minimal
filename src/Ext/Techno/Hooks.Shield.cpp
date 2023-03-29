#include "Body.h"
#include <SpecificStructures.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TEvent/Body.h>
#include <TunnelLocomotionClass.h>

namespace EvaluateObjectTemp
{
	WeaponTypeClass* PickedWeapon = nullptr;
}

DEFINE_HOOK(0x6F7E24, TechnoClass_EvaluateObject_SetContext, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBP);

	EvaluateObjectTemp::PickedWeapon = pWeapon;

	return 0;
}

// #issue 88 : shield logic
// TODO : Emp reset shield
DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);

	//if (pWHExt->IgnoreDefense)
	//	args->IgnoreDefenses = true;

	pWHExt->ApplyDamageMult(pThis, &args);

	if (!args.IgnoreDefenses) {
		if (auto pShieldData = TechnoExt::ExtMap.Find(pThis)->GetShield()) {
			pShieldData->OnReceiveDamage(&args);
		}
	}

	return 0;
}

DEFINE_HOOK(0x7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 0x5)
{
	enum { Continue = 0x0, SkipLowDamageCheck = 0x7019E3 };
	GET(TechnoClass*, pThis, ESI);
	GET(int*, pDamage, EBX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->SkipLowDamageCheck) {
		pExt->SkipLowDamageCheck = false;
	} else {

		// Restore overridden instructions
		if (*pDamage < 1)
			*pDamage = 1;
	}

	return SkipLowDamageCheck;
}

#define REPLACE_ARMOR(addr , regWP , regTech , name)\
DEFINE_HOOK(addr, name, 0x6) {\
GET(WeaponTypeClass*, pWeapon, regWP);\
GET(TechnoClass*, pTarget, regTech);\
	if (TechnoExt::ReplaceArmor(R, pTarget, pWeapon))\
		{ return R->Origin() + 6; } return 0; }

DEFINE_HOOK(0x70CF39, TechnoClass_EvalThreatRating_Shield, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(ObjectClass*, pTarget, ESI);

	if (auto pTechno = generic_cast<TechnoClass*>(pTarget)) {
		if(TechnoExt::ReplaceArmor(R,pTechno,pWeapon))
			return R->Origin() + 6;
	}

	return 0;
}

REPLACE_ARMOR(0x6F7D31, EBP, ESI, TechnoClass_CanAutoTargetObject_Shield) //
REPLACE_ARMOR(0x6FCB64, EBX, EBP, TechnoClass_CanFire_Shield) //
REPLACE_ARMOR(0x708AEB, ESI, EBP, TechnoClass_ShouldRetaliate_Shield) //

#undef REPLACE_ARMOR

// Ares-hook jmp to this offset
DEFINE_HOOK(0x71A88D, TemporalClass_AI_Shield, 0x8) //0
{
	GET(TemporalClass*, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);

		if (const auto pShieldData = pTargetExt->GetShield()) {
			if (pShieldData->IsAvailable())
				pShieldData->OnTemporalUpdate(pThis);
		}

		//pTargetExt->UpdateFireSelf();
		//pTargetExt->UpdateRevengeWeapons();
	}

	// Recovering vanilla instructions that were broken by a hook call
	return R->EAX<int>() <= 0 ? 0x71A895 : 0x71AB08;
}

DEFINE_HOOK(0x6F6AC4, TechnoClass_Remove_Shield, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	if (const auto pShieldData = TechnoExt::ExtMap.Find(pThis)->GetShield())
		pShieldData->OnRemove();

	return 0;
}

DEFINE_HOOK_AGAIN(0x44A03C, DeploysInto_UndeploysInto_SyncShieldStatus, 0x6) //BuildingClass_Mi_Selling_SyncShieldStatus
DEFINE_HOOK(0x739956, DeploysInto_UndeploysInto_SyncShieldStatus, 0x6) //UnitClass_Deploy_SyncShieldStatus
{
	GET(TechnoClass*, pFrom, EBP);
	GET(TechnoClass*, pTo, EBX);

	ShieldClass::SyncShieldToAnother(pFrom, pTo);
	TechnoExt::SyncIronCurtainStatus(pFrom, pTo);
	return 0;
}

DEFINE_HOOK(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (const auto pShieldData = TechnoExt::ExtMap.Find(pLoco->LinkedTo)->GetShield())
	{
		pShieldData->SetAnimationVisibility(false);
	}

	return 0;
}

DEFINE_HOOK(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PRE_DIG_OUT)
	{
		if (const auto pShieldData = TechnoExt::ExtMap.Find(pLoco->LinkedTo)->GetShield())
			pShieldData->SetAnimationVisibility(true);
	}

	return 0;
}

#pragma region HealingWeapons

#pragma region TechnoClass__Evaluate_Object

double __fastcall HealthRatio_Wrapper(TechnoClass* pTechno, void* _)
{
	double result = pTechno->GetHealthPercentage();
	if (result >= 1.0)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pTechno);
		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsActive())
			{
				const auto pWeapon = EvaluateObjectTemp::PickedWeapon;
				if (!pShieldData->CanBePenetrated(pWeapon ? pWeapon->Warhead : nullptr))
					result = pExt->Shield->GetHealthRatio();
			}
		}
	}

	EvaluateObjectTemp::PickedWeapon = nullptr;
	return result;
}

DEFINE_JUMP(CALL, 0x6F7F51, GET_OFFSET(HealthRatio_Wrapper));

#pragma endregion TechnoClass__Evaluate_Object

class AresScheme
{
	static inline ObjectClass* LinkedObj = nullptr;
public:

	static void __cdecl Prefix(TechnoClass* pThis, ObjectClass* pObj, int nWeaponIndex)
	{
		if (LinkedObj)
			return;

		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			const auto pExt = TechnoExt::ExtMap.Find(pTechno);

			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive())
				{

					const auto pWeapon = pThis->GetWeapon(nWeaponIndex < 0 ? pThis->SelectWeapon(pObj) : nWeaponIndex);

					if (pWeapon && pWeapon->WeaponType && !pShieldData->CanBePenetrated(pWeapon->WeaponType->Warhead))
					{
						if (pExt->Shield->GetHealthRatio() < 1.0)
						{
							LinkedObj = pObj;
							--LinkedObj->Health;
						}
					}
				}
			}
		}
	}

	static void __cdecl Prefix(InfantryClass* pThis, ObjectClass* pObj, int nWeaponIndex, bool IsEngiiner)
	{
		if (LinkedObj)
			return;

		if (IsEngiiner && CanApplyEngineerActions(pThis, pObj))
			return;

		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			const auto pExt = TechnoExt::ExtMap.Find(pTechno);

			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive())
				{

					const auto pWeapon = pThis->GetWeapon(nWeaponIndex < 0 ? pThis->SelectWeapon(pObj) : nWeaponIndex);

					if (pWeapon && pWeapon->WeaponType && !pShieldData->CanBePenetrated(pWeapon->WeaponType->Warhead))
					{
						if (pExt->Shield->GetHealthRatio() < 1.0)
						{
							LinkedObj = pObj;
							--LinkedObj->Health;
						}
					}
				}
			}
		}
	}

	static void __cdecl Suffix()
	{
		if (LinkedObj)
		{
			++LinkedObj->Health;
			LinkedObj = nullptr;
		}
	}
private:
	static bool CanApplyEngineerActions(InfantryClass* pThis, ObjectClass* pTarget)
	{
		if (Is_Building(pTarget))
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTarget);
			if (HouseClass::CurrentPlayer->IsAlliedWith(pBuilding))
			{
				return pBuilding->Type->Repairable;
			}
			else
			{
				return pBuilding->Type->Capturable && (!pBuilding->Owner->Type->MultiplayPassive || !pBuilding->Type->CanBeOccupied || pBuilding->IsBeingWarpedOut());
			}
		}

		return false;
	}
};

#pragma region UnitClass_GetFireError_Heal

FireError __fastcall UnitClass__GetFireError_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
	auto const result = pThis->UnitClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE, 0x7F6030, GET_OFFSET(UnitClass__GetFireError_Wrapper));
#pragma endregion UnitClass_GetFireError_Heal

#pragma region InfantryClass_GetFireError_Heal

FireError __fastcall InfantryClass__GetFireError_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
	auto const result = pThis->InfantryClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}

DEFINE_JUMP(VTABLE, 0x7EB418, GET_OFFSET(InfantryClass__GetFireError_Wrapper));
#pragma endregion InfantryClass_GetFireError_Heal

#pragma region UnitClass__WhatAction

Action __fastcall UnitClass__WhatAction_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1);
	auto const result = pThis->UnitClass::MouseOverObject(pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE, 0x7F5CE4, GET_OFFSET(UnitClass__WhatAction_Wrapper));
#pragma endregion UnitClass__WhatAction

#pragma region InfantryClass__WhatAction
Action __fastcall InfantryClass__WhatAction_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1, pThis->Type->Engineer);
	auto const result = pThis->InfantryClass::MouseOverObject(pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE, 0x7EB0CC, GET_OFFSET(InfantryClass__WhatAction_Wrapper));
#pragma endregion InfantryClass__WhatAction
#pragma endregion HealingWeapons
