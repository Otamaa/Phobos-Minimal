#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));

	if (auto const pThisTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		pCoord->X += pThisTypeExt->XDrawOffset;

	return 0;
}

DEFINE_HOOK(0x423B95, AnimClass_AI_HideIfNoOre_Threshold, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pType, EDX);

	if (pType->HideIfNoOre)
	{
		int nThreshold = 0;
		if (auto const pExt = AnimTypeExt::ExtMap.Find(pType))
			nThreshold = abs(pExt->HideIfNoOre_Threshold.Get());

		auto const pCell = pThis->GetCell();

		pThis->Invisible = !pCell || pCell->GetContainedTiberiumValue() <= nThreshold;
	}

	return 0x423BBF;
} //was 8

//DEFINE_JUMP(VTABLE, 0x7E33CC, GET_OFFSET(AnimExt::GetLayer_patch));

DEFINE_HOOK(0x424CB0, AnimClass_InWhichLayer_Override, 0x6) //was 5
{
	GET(AnimClass*, pThis, ECX);

	enum {
		RetLayerGround = 0x424CBA,
		RetLayerAir = 0x0424CD1,
		RetTypeLayer = 0x424CCA,
		ReturnSetManualResult = 0x424CD6
	};

	if (pThis->OwnerObject) {
		const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

		if (!pExt || !pExt->Layer_UseObjectLayer.isset()) {
			return RetLayerGround;
		}

		if (pExt->Layer_UseObjectLayer.Get()) {
			Layer nRes = Layer::Ground;

			if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject)) {
				if (auto const pLocomotor = pFoot->Locomotor.get())
					nRes = pLocomotor->In_Which_Layer();
			}
			else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
				nRes = pBullet->InWhichLayer();
			else
				nRes = pThis->OwnerObject->ObjectClass::InWhichLayer();

			R->EAX(nRes);
			return ReturnSetManualResult;
		}
	}

	if (pThis->Type) {
		R->EAX(pThis->Type->Layer);
		return ReturnSetManualResult;
	} else {
		return RetLayerAir;
	}

}

DEFINE_HOOK(0x424C49, AnimClass_AttachTo_BuildingCoords, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);
	GET(CoordStruct*, pCoords, EAX);

	const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pExt && pExt->UseCenterCoordsIfAttached)
	{
		pCoords = pObject->GetRenderCoords(pCoords);
		pCoords->X += 128;
		pCoords->Y += 128;
	}

	return 0;
}

DEFINE_HOOK(0x424807, AnimClass_AI_Next, 0x6) //was 8
{
	GET(AnimClass*, pThis, ESI);

	if(const auto pExt = AnimExt::ExtMap.Find(pThis)) {
		if (const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type)) {
			if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
				pExt->DeleteAttachedSystem();

			if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
				pExt->CreateAttachedSystem(pTypeExt);
		}
	}

	return 0;
}

#ifdef ENABLE_PHOBOS_DAMAGEDELAYANIM
// Goes before and replaces Ares animation damage / weapon hook at 0x424538.
DEFINE_HOOK(0x424513, AnimClass_AI_Damage, 0x6)
{
	enum { SkipDamage = 0x424663, Continue = 0x42464C };

	GET(AnimClass*, pThis, ESI);

	auto Ret_SkipDamage = [R, pThis]()
	{
		R->EAX(pThis->Type);
		return 0x42465D;
	};

	if (pThis->Type->Damage <= 0.0 || pThis->HasExtras)
		return Ret_SkipDamage();

	auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int delay = pTypeExt->Damage_Delay.Get();
	TechnoClass* const pInvoker = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker);

	int damageMultiplier = 1;

	if (pThis->OwnerObject && pThis->OwnerObject->WhatAmI() == AbstractType::Terrain)
		damageMultiplier = 5;

	bool adjustAccum = false;
	double damage = 0;
	int appliedDamage = 0;

	if (pTypeExt->Damage_ApplyOnce.Get()) // If damage is to be applied only once per animation loop
	{
		if (pThis->Animation.Value == max(delay - 1, 1))
			appliedDamage = static_cast<int>(int_round(pThis->Type->Damage)) * damageMultiplier;
		else
			return Ret_SkipDamage();
	}
	else if (delay <= 0 || pThis->Type->Damage < 1.0) // If Damage.Delay is less than 1 or Damage is a fraction.
	{
		adjustAccum = true;
		damage = damageMultiplier * pThis->Type->Damage + pThis->Accum;
		pThis->Accum = damage;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (damage >= 1.0)
			appliedDamage = static_cast<int>(int_round(damage));
		else
			return Ret_SkipDamage();
	}
	else
	{
		// Accum here is used as a counter for Damage.Delay, which cannot deal fractional damage.
		damage = pThis->Accum + 1.0;
		pThis->Accum = damage;

		if (damage < delay)
			return Ret_SkipDamage();

		// Use Type->Damage as the actually dealt damage.
		appliedDamage = static_cast<int>((pThis->Type->Damage)) * damageMultiplier;
	}

	if (appliedDamage <= 0 || pThis->IsPlaying)
		return Ret_SkipDamage();

	// Store fractional damage if needed, or reset the accum if hit the Damage.Delay counter.
	if (adjustAccum)
		pThis->Accum = damage - appliedDamage;
	else
		pThis->Accum = 0.0;

	auto nCoord = pThis->GetCoords();
	auto const nDamageResult = static_cast<int>(appliedDamage * TechnoExt::GetDamageMult(pInvoker, !pTypeExt->Damage_ConsiderOwnerVeterancy.Get()));

	if (auto const pWeapon = pTypeExt->Weapon.Get(nullptr))
	{
		AbstractClass* pTarget = pTypeExt->Damage_TargetInvoker.Get() ? static_cast<AbstractClass*>(pInvoker) : pThis->GetCell();
		if (auto const pBullet = pWeapon->Projectile->CreateBullet(pTarget, pInvoker, nDamageResult, pWeapon->Warhead, 0, pWeapon->Bright || pWeapon->Warhead->Bright))
		{
			pBullet->SetWeaponType(pWeapon);
			pBullet->Limbo();
			pBullet->SetLocation(nCoord);
			pBullet->Explode(true);
			pBullet->UnInit();
		}
	}
	else
	{
		auto const pWarhead = pThis->Type->Warhead ? pThis->Type->Warhead :
			pTypeExt->IsInviso ? RulesGlobal->FlameDamage2 : RulesGlobal->C4Warhead;

		auto pOwner = pThis->Owner ? pThis->Owner : pInvoker ? pInvoker->GetOwningHouse() : nullptr;

		if (pTypeExt->Warhead_Detonate.Get())
			WarheadTypeExt::DetonateAt(pWarhead, nCoord, pInvoker, nDamageResult , !pTypeExt->Damage_TargetInvoker.Get());
		else
			MapClass::DamageArea(nCoord, nDamageResult, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
		//MapClass::FlashbangWarheadAt(nDamageResult, pWarhead, nCoord);
	}

	return Continue;
}
#endif