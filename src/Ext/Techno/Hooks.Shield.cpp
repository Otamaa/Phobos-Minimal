#include "Body.h"
#include <SpecificStructures.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TEvent/Body.h>

// #issue 88 : shield logic
DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, 0x4);

	if (!args->IgnoreDefenses)
	{
		if(const auto pExt = TechnoExt::GetExtData(pThis))
		{
			if (auto pShieldData = pExt->GetShield())
			{
				if (!pShieldData->IsActive())
					return 0;

				pShieldData->OnReceiveDamage(args);

				if (auto const pTag = pThis->AttachedTag)
					pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::ShieldBroken, pThis,
						CellStruct::Empty, false, args->Attacker);//where is this? is this correct?

			}
		}
	}

	return 0;
}

/*
DEFINE_HOOK(0x5F5399, ObjectClass_ReceiveDamage_Shield, 0xB)
{
	GET(ObjectClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFS(0x24,-0x4));

	const auto pTechno = generic_cast<TechnoClass*>(pThis);

	if (!args->IgnoreDefenses && pTechno) {
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno)) {
			if (const auto pShieldData = pExt->GetShield())
			{
				if (pShieldData->IsActive()){

					pShieldData->OnReceiveDamage(args);

					if(!pShieldData->ReceiveDamageExecuted)
					if (auto const pTag = pTechno->AttachedTag)
						pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::ShieldBroken, pTechno,
							CellStruct::Empty, false, args->Attacker);//where is this? is this correct?
				}
			}
		}
	}

	return 0;
}
*/
DEFINE_HOOK(0x7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int*, Damage, EBX);

	if (const auto pExt = TechnoExt::GetExtData(pThis))
	{
		if (pExt->GetShield() && pExt->GetShield()->IsActive())
			return 0x7019E3;
	}

	//pre fix
	//return *Damage >= 0 ? 0x7019E3 : 0x7019DD;

	//after fix
	return *Damage >= 1 ? 0x7019E3 : 0x7019DD;
}

#define REPLACE_ARMOR(addr , regWP , regTech , name)\
DEFINE_HOOK(addr, name, 0x6) {\
GET(WeaponTypeClass*, pWeapon, regWP);\
GET(TechnoClass*, pTarget, regTech);\
if (auto pExt = TechnoExt::ExtMap.Find(pTarget)) {\
	if (auto pShieldData = pExt->Shield.get()) {\
	if (pShieldData->CanBePenetrated(pWeapon->Warhead)) return 0;\
	if (pShieldData->IsActive()) { R->EAX(pShieldData->GetType()->Armor);\
		return R->Origin() + 6; } } } return 0;}

DEFINE_HOOK(0x70CF39, TechnoClass_EvalThreatRating_Shield, 0x6) {
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_STACK(TechnoClass*, pTarget, STACK_OFFS(0x5C ,-0x8));

	if(pTarget->IsAlive){
		if (auto pExt = TechnoExt::ExtMap.Find(pTarget)) {
			if (auto pShieldData = pExt->Shield.get()) {
			if (pShieldData->CanBePenetrated(pWeapon->Warhead)) return 0;
				if (pShieldData->IsActive()) {
					R->EAX(pShieldData->GetType()->Armor);
					return R->Origin() + 6;
				}
			}
		}
	}

	return 0;
}

REPLACE_ARMOR(0x6F7D31, EBP, ESI, TechnoClass_CanAutoTargetObject_Shield) //
REPLACE_ARMOR(0x6FCB64, EBX, EBP, TechnoClass_CanFire_Shield) //
REPLACE_ARMOR(0x708AEB, ESI, EBP, TechnoClass_ShouldRetaliate_Shield) //

#undef REPLACE_ARMOR

DEFINE_HOOK(0x6F9E50, TechnoClass_AI_Shield, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt)
	{
		// Set current shield type if it is not set.
		if (!pExt->CurrentShieldType || (!pExt->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength))
			pExt->CurrentShieldType = pTypeExt->ShieldType;

		// Create shield class instance if it does not exist.
		if (pExt->CurrentShieldType && pExt->CurrentShieldType->Strength && !pExt->Shield){
			pExt->Shield = std::make_unique<ShieldClass>(pThis);
			pExt->Shield->OnInit();
		}

		if (const auto pShieldData = pExt->GetShield())
			pShieldData->OnUpdate();
	}

	return 0;
}

// Ares-hook jmp to this offset
DEFINE_HOOK(0x71A88D, TemporalClass_AI_Shield, 0x8) //0
{
	GET(TemporalClass*, pThis, ESI);

	if (auto const pTarget = pThis->Target) {
		if (const auto pExt = TechnoExt::GetExtData(pTarget)) {
			if (const auto pShieldData = pExt->GetShield()) {
				if (pShieldData->IsAvailable())
					pShieldData->OnTemporalUpdate(pThis);
			}
		}
	}

	// Recovering vanilla instructions that were broken by a hook call
	return R->EAX<int>() <= 0 ? 0x71A895 : 0x71AB08;
}

DEFINE_HOOK(0x6F6AC4, TechnoClass_Remove_Shield, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	if (const auto pExt = TechnoExt::GetExtData(pThis)) {
		if (const auto pShieldData = pExt->GetShield())
			 pShieldData->OnRemove();
	}

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

#pragma region HealingWeapons

#pragma region TechnoClass__Evaluate_Object

double __fastcall HealthRatio_Wrapper(TechnoClass* pTechno , void* _)
{
	double result = pTechno->GetHealthPercentage();
	if (result >= 1.0)
	{
		if (const auto pExt = TechnoExt::GetExtData(pTechno))
		{
			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive())
					result = pShieldData->GetHealthRatio();
			}
		}
	}

	return result;
}

DEFINE_JUMP(CALL,0x6F7F51, GET_OFFSET(HealthRatio_Wrapper));

#pragma endregion TechnoClass__Evaluate_Object

class AresScheme
{
	static inline ObjectClass* LinkedObj = nullptr;
public:
	static void __cdecl Prefix(ObjectClass* pObj)
	{
		if (LinkedObj)
			return;

		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			if (const auto pExt = TechnoExt::GetExtData(pTechno))
			{
				if (const auto pShieldData = pExt->Shield.get())
				{
					if (pShieldData->IsActive())
					{
						const auto shieldRatio = pExt->Shield->GetHealthRatio();
						if (shieldRatio < 1.0)
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

};

static inline int ReplaceThreadPosed(TechnoClass* pThis, TechnoTypeClass* pType)
{
	if (auto pExt = TechnoExt::ExtMap.Find(pThis)){
		if (const auto pShieldData = pExt->Shield.get()){
			if (pShieldData->IsAvailable()){
				auto const pShiedType = pShieldData->GetType();
				if (pShiedType->ThreadPosed.isset())
					return pShiedType->ThreadPosed.Get();
			}
		}
	}

	return pType->ThreatPosed;
}

DEFINE_HOOK(0x708BA3, TechnoClass_ThreatPosed_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EDI);
	R->EAX(ReplaceThreadPosed(pThis, pType));
	return 0x708BA9;
}

DEFINE_HOOK(0x708B9A, TechnoClass_ThreadPosed_BunkerLinked_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET(TechnoTypeClass*, pType, EAX);
	R->EAX(ReplaceThreadPosed(pThis, pType));
	return 0x708BA0;
}

DEFINE_HOOK(0x4F6B97, HouseClass_Apparent_Category_Power_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	R->ECX(ReplaceThreadPosed(pThis, pType));
	return 0x4F6B9D;
}

DEFINE_HOOK(0x4F6B27, HouseClass_Category_Power_Shield, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	R->ECX(ReplaceThreadPosed(pThis, pType));
	return 0x4F6B2D;
}

#pragma region UnitClass_GetFireError_Heal

FireError __fastcall UnitClass__GetFireError(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	JMP_THIS(0x740FD0);
}

FireError __fastcall UnitClass__GetFireError_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pObj);
	auto const result = UnitClass__GetFireError(pThis, _, pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE,0x7F6030, GET_OFFSET(UnitClass__GetFireError_Wrapper));
#pragma endregion UnitClass_GetFireError_Heal

#pragma region InfantryClass_GetFireError_Heal
FireError __fastcall InfantryClass__GetFireError(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	JMP_THIS(0x51C8B0);
}
FireError __fastcall InfantryClass__GetFireError_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pObj);
	auto const result = InfantryClass__GetFireError(pThis, _, pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE,0x7EB418, GET_OFFSET(InfantryClass__GetFireError_Wrapper));
#pragma endregion InfantryClass_GetFireError_Heal

#pragma region UnitClass__WhatAction
Action __fastcall UnitClass__WhatAction(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	JMP_THIS(0x73FD50);
}

Action __fastcall UnitClass__WhatAction_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pObj);
	auto const result = UnitClass__WhatAction(pThis, _, pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE,0x7F5CE4, GET_OFFSET(UnitClass__WhatAction_Wrapper));
#pragma endregion UnitClass__WhatAction

#pragma region InfantryClass__WhatAction
Action __fastcall InfantryClass__WhatAction(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	JMP_THIS(0x51E3B0);
}

Action __fastcall InfantryClass__WhatAction_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pObj);
	auto const result = InfantryClass__WhatAction(pThis, _, pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE,0x7EB0CC, GET_OFFSET(InfantryClass__WhatAction_Wrapper));
#pragma endregion InfantryClass__WhatAction
#pragma endregion HealingWeapons
