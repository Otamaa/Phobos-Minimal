#include "Body.h"
#include <TechnoClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <SpecificStructures.h>
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include <Utilities/GeneralUtils.h>


//TODO :
// Parasite Heal
// Parasite Able To target friendly
// Parasite not removed when heal
// Parasite Gain victim control instead of damaging

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x62A0D3, ParasiteClass_AI_Particle, 0x5)
{
	//GET(ParasiteClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x4C, 0x18));
	GET(WeaponTypeClass* const,  pWeapon, EDI);

	auto pParticle = RulesGlobal->DefaultSparkSystem;
	if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead))
		pParticle = pWarheadExt->Parasite_ParticleSys.Get(RulesGlobal->DefaultSparkSystem);

	if(pParticle)
		GameCreate<ParticleSystemClass>(pParticle, nCoord);

	return 0x62A108;
}

DEFINE_HOOK(0x62A13F, ParasiteClass_AI_WeaponAnim, 0x5)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pAnimType, EBP);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x4C, 0x18));

	if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord)) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr, pThis->Victim ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
	}

	return 0x62A16A;
}

DEFINE_HOOK(0x62A0B7, ParasiteClass_AI_InfantryAction, 0x5)
{
	enum
	{
		SkippAll = 0x62A24D,
		ReceiveDamage = 0x0,
		ReceiveDamage_LikeVehicle = 0x62A0D3 // not instant kill the infantry , but give it damage per second
	};

	//GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const , pWeapon, EDI);
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if(pWarheadExt && pWarheadExt->Parasite_TreatInfantryAsVehicle.Get()){
		return ReceiveDamage_LikeVehicle;
	}

	return ReceiveDamage;
}

DEFINE_HOOK(0x62A16A, ParasiteClass_AI_DisableRocking, 0x7)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);

	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	return (pWarheadExt && pWarheadExt->Parasite_DisableRocking.Get()) || pThis->Victim->WhatAmI() == AbstractType::Infantry
		? 0x62A222 : 0x0;
}
#endif

DEFINE_HOOK_AGAIN(0x62A399 , ParasiteClass_ExitUnit_ExitSound ,0x0) //ParasiteClass_Detach
DEFINE_HOOK(0x62A735, ParasiteClass_ExitUnit_ExitSound, 0x0) //ParasiteClass_Uninfect
{
	GET(ParasiteClass* const, pParasite, ESI);

	if (auto const pParasiteOwner = pParasite->Owner) {
		if (auto const pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pParasiteOwner->GetTechnoType())) {
			auto nCoord = pParasiteOwner->GetCoords();
			VoxClass::PlayAtPos(pOwnerTypeExt->ParasiteExit_Sound.Get(), &nCoord);
		}
	}

	return 0;
}

DEFINE_HOOK(0x629B50, ParasiteClass_SquiddyGrab_DeharcodeSplash, 0x7)
{
	enum { Handled = 0x629B9C, Continue = 0x0 };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFS(0x70, 0x4C));
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x70, 0xC));
	GET(ParasiteClass* const, pThis, ESI);

	if (auto const pWhExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead))
	{
		if (auto const AnimType = pWhExt->SquidSplash.GetElements(RulesClass::Instance->SplashList))
		{
			if (auto const pSplashType = AnimType.at(ScenarioClass::Instance->Random.RandomFromMax((AnimType.size() - 1))))
			{
				if (auto pAnim = GameCreate<AnimClass>(pSplashType, *pCoord))
				{
					auto const Invoker = (pThis->Owner) ? pThis->Owner->GetOwningHouse() : pThis->GetOwningHouse();
					AnimExt::SetAnimOwnerHouseKind(pAnim, Invoker, (pThis->Victim) ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
					return Handled;
				}
			}
		}
	}

	return Continue;
}

