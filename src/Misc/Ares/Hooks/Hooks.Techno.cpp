#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Conversions.h>

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
DEFINE_OVERRIDE_HOOK(0x6F7561, TechnoClass_Targeting_Arcing_Aircraft, 0x5)
{
	GET(AbstractType, pTarget, EAX);
	GET(CoordStruct*, pCoord, ESI);
	R->EAX(pCoord->X);
	return pTarget == AbstractType::Aircraft ? 0x6F75B2 : 0x6F7568;
}

// No data found on .inj for this
//DEFINE_OVERRIDE_HOOK(0x5F7933, TechnoTypeClass_FindFactory_ExcludeDisabled, 0x6)
//{
//	GET(BuildingClass*, pBld, ESI);
//
//	 //add the EMP check to the limbo check
//	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
//		0x5F7A57 : 0x5F7941;
//}

DEFINE_OVERRIDE_HOOK(0x6F90F8, TechnoClass_SelectAutoTarget_Demacroize, 0x6)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x6F9116;
}

DEFINE_OVERRIDE_HOOK(0x70133E, TechnoClass_GetWeaponRange_Demacroize, 0x5)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EBX);

	R->EAX(nVal1 >= nVal2 ? nVal2 : nVal1);
	return 0x701388;
}

DEFINE_OVERRIDE_HOOK(0x707EEA, TechnoClass_GetGuardRange_Demacroize, 0x6)
{
	GET(int, nVal1, EBX);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x707F08;
}

DEFINE_OVERRIDE_HOOK(0x707A47, TechnoClass_PointerGotInvalid_LastTarget, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->LastTarget == ptr)
		pThis->LastTarget = nullptr;

	return 0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6FCF53, TechnoClass_SetTarget_Burst, 0x6, 6FCF61)

DEFINE_OVERRIDE_HOOK_AGAIN(0x717855, TechnoTypeClass_UpdatePalette_Reset, 0x6)
DEFINE_OVERRIDE_HOOK(0x717823, TechnoTypeClass_UpdatePalette_Reset, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->Palette = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71136F, TechnoTypeClass_CTOR_Initialize, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->WeaponCount = 0; //default
	pThis->Bunkerable = false;
	pThis->Parasiteable = false;
	pThis->ImmuneToPoison = false;
	pThis->ConsideredAircraft = false;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7119D5, TechnoTypeClass_CTOR_NoInit_Particles, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI)

	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DamageParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();
	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DestroyParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();

	return 0x711A00;
}

// destroying a building (no health left) resulted in a single green pip shown
// in the health bar for a split second. this makes the last pip red.
DEFINE_OVERRIDE_HOOK(0x6F661D, TechnoClass_DrawHealthBar_DestroyedBuilding_RedPip, 0x7)
{
	GET(BuildingClass*, pBld, ESI);
	return (pBld->Health <= 0 || pBld->IsRedHP()) ? 0x6F6628 : 0x6F6630;
}

// issues 1002020, 896263, 895954: clear stale mind control pointer to prevent
// crashes when accessing properties of the destroyed controllers.
DEFINE_OVERRIDE_HOOK(0x7077EE, TechnoClass_PointerGotInvalid_ResetMindControl, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->MindControlledBy == ptr)
	{
		pThis->MindControlledBy = nullptr;
	}

	return 0;
}

// #1415844: units in open-topped transports show behind anim
DEFINE_OVERRIDE_HOOK(0x6FA2C7, TechnoClass_Update_DrawHidden, 0x8)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const disallowed = pThis->InOpenToppedTransport || pThis->GetTechnoType()->Invisible;
	return !disallowed ? 0u : 0x6FA30Cu;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6FFF9E, TechnoClass_GetActionOnObject_IvanBombsB, 0x8, 700006)

DEFINE_OVERRIDE_HOOK(0x702DD6, TechnoClass_RegisterDestruction_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent(static_cast<TriggerEvent>(0x55), pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7032B0, TechnoClass_RegisterLoss_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(HouseClass*, pAttacker, EDI);

	if (pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent(static_cast<TriggerEvent>(0x55), pThis, CellStruct::Empty, false, pAttacker);
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6FF2D1, TechnoClass_FireAt_Facings, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	int nIdx = 0;

	if (pWeapon->Anim.Count > 1) { //only execute if the anim count is more than 1
		const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3) {
			nIdx = pThis->GetRealFacing().GetValue(highest, 1u << (highest - 3));
		} 
	}

	R->EDI(pWeapon->Anim.GetItemOrDefault(nIdx , nullptr));
	return 0x6FF31B;
}

DEFINE_OVERRIDE_HOOK(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pBulletExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	const auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	R->EAX(ret);
	return 0x6FE562;
}

DEFINE_OVERRIDE_HOOK(0x6F826E, TechnoClass_CanAutoTargetObject_CivilianEnemy, 0x5)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EBP);

	enum { 
		Undecided = 0, 
		ConsiderEnemy = 0x6F8483, 
		ConsiderCivilian = 0x6F83B1, 
		Ignore = 0x6F894F
	};

	const auto pExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	// always consider this an enemy
	if (pExt->CivilianEnemy) {
		return ConsiderEnemy;
	}

	// if the potential target is attacking an allied object, consider it an enemy
	// to not allow civilians to overrun a player
	if (const auto pTargetTarget = abstract_cast<TechnoClass*>(pTarget->Target)) {
		const auto pOwner = pThis->Owner;
		if (pOwner->IsAlliedWith(pTargetTarget)) {
			const auto pData = RulesExt::Global();

			if (pOwner->IsControlledByHuman() ? 
				pData->AutoRepelPlayer : pData->AutoRepelAI) {
				return ConsiderEnemy;
			}
		}
	}

	return Undecided;
}

DEFINE_OVERRIDE_HOOK(0x7162B0, TechnoTypeClass_GetPipMax_MindControl, 0x6)
{
	GET(TechnoTypeClass* const, pThis, ECX);

	auto const GetMindDamage = [](WeaponTypeClass const* const pWeapon) {
		return (pWeapon && pWeapon->Warhead->MindControl) ? pWeapon->Damage : 0;
	};

	auto count = GetMindDamage(pThis->GetWeapon(0)->WeaponType);
	if (count <= 0) {
		count = GetMindDamage(pThis->GetWeapon(1)->WeaponType);
	}

	R->EAX(count);
	return 0x7162BC;
}

DEFINE_HOOK(0x6FC40C, TechnoClass_CanFire_PsionicsImmune, 0x6)
{
	enum { FireIllegal = 0x6FC86A, ContinueCheck = 0x6FC425 };
	GET(TechnoClass*, pTarget, EBP);
	return TechnoExt::IsPsionicsImmune(pTarget)
		? FireIllegal : ContinueCheck;
}

DEFINE_OVERRIDE_HOOK(0x701BFE, TechnoClass_ReceiveDamage_Abilities, 0x6)
{
	enum
	{
		RetNullify = 0x701C1C,
		RetNullifyB = 0x701CC2,
		RetObjectClassRcvDamage = 0x701DCC,
		RetUnaffected = 0x701CFC,
		RetCheckBuilding = 0x701D2E,
		RetResultLight = 0x701DBA
	};

	GET(WarheadTypeClass*, pWH, EBP);
	GET(TechnoClass*, pThis, ESI);
	LEA_STACK(args_ReceiveDamage*, Arguments, 0xC8);

	if (pWH->Radiation && TechnoExt::IsRadImmune(pThis))
		return RetNullify;

	if (pWH->PsychicDamage && TechnoExt::IsPsionicsWeaponImmune(pThis))
		return RetNullify;

	if (pWH->Poison && TechnoExt::IsPoisonImmune(pThis))
		return RetNullify;

	const auto pSourceHouse = Arguments->Attacker ? Arguments->Attacker->Owner : Arguments->SourceHouse;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt->CanAffectHouse(pThis->Owner, pSourceHouse))
		return RetNullifyB;

	if (pWH->Psychedelic) {

		//This thing does ally check twice
		//if (pSourceHouse && pSourceHouse->IsAlliedWith(pThis) && !pWHExt->Berzerk_AffectAlly)
		//	return RetUnaffected;
		if (pThis->WhatAmI() == AbstractType::Building)
			return RetUnaffected;

		if (TechnoExt::IsPsionicsImmune(pThis) || TechnoExt::IsBerserkImmune(pThis))
			return RetUnaffected;

		// there is no building involved
		// More customizeable berzerk appying - Otamaa
		// return boolean to decide receive damage after apply berzerk or just retun function result
		if (!pWHExt->GoBerzerkFor(static_cast<FootClass*>(pThis), Arguments->Damage))
			return RetResultLight;
	}

	return RetObjectClassRcvDamage;
}