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

//#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x62A0D3, ParasiteClass_AI_Particle, 0x5)
{
	//GET(ParasiteClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x4C, 0x18));
	GET(WeaponTypeClass* const, pWeapon, EDI);

	if (auto pParticle = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_ParticleSys.Get(RulesGlobal->DefaultSparkSystem))
		GameCreate<ParticleSystemClass>(pParticle, nCoord);

	return 0x62A108;
}

DEFINE_HOOK(0x62A13F, ParasiteClass_AI_WeaponAnim, 0x5)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pAnimType, EBP);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x4C, 0x18));

	if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord))
	{
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr, pThis->Victim ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
	}

	return 0x62A16A;
}

DEFINE_HOOK(0x62A074, ParasiteClass_AI_DamagingAction, 0x6)
{
	enum
	{
		SkippAll = 0x62A24D,
		ReceiveDamage = 0x62A0B7,
		ReceiveDamage_LikeVehicle = 0x62A0D3 // not instant kill the infantry , but gave it damage per second
	};

	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);
	GET(WarheadTypeClass* const, pWarhead, EBP);

	pThis->DamageDeliveryTimer.Start(pWeapon->ROF);
	pThis->SuppressionTimer.Start(pWarhead->Paralyzes);

	if (pThis->Victim->WhatAmI() == AbstractType::Infantry)
	{
		if (!WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_TreatInfantryAsVehicle.Get(static_cast<InfantryClass*>(pThis->Victim)->Type->Cyborg))
			return ReceiveDamage;
	}

	return ReceiveDamage_LikeVehicle;
}

//DEFINE_HOOK(0x62A0B7, ParasiteClass_AI_InfantryAction, 0x5)
//{
//	enum
//	{
//		SkippAll = 0x62A24D,
//		ReceiveDamage = 0x62A0BA,
//		ReceiveDamage_LikeVehicle = 0x62A0D3 // not instant kill the infantry , but give it damage per second
//	};
//
//	GET(ParasiteClass* const, pThis, ESI);
//	GET(WeaponTypeClass* const , pWeapon, EDI);
//
//	if(pWarheadExt->Parasite_TreatInfantryAsVehicle.Get(static_cast<InfantryClass*>(pThis->Victim)->Type->Cyborg)) {
//		return ReceiveDamage_LikeVehicle;
//	}
//
//	R->ECX(pThis->Victim);
//	return ReceiveDamage;
//}

DEFINE_HOOK(0x62A16A, ParasiteClass_AI_DisableRocking, 0x5)
{
	enum { Skip = 0x62A222 , Continue = 0x0 };
	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);
	
	if (pThis->Victim->IsAttackedByLocomotor)
		return Skip;

	return (WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_DisableRocking.Get()) || pThis->Victim->WhatAmI() == AbstractType::Infantry
		? Skip : Continue;
}
//#endif

DEFINE_HOOK_AGAIN(0x62A399, ParasiteClass_ExitUnit_ExitSound, 0x9) //ParasiteClass_Detach
DEFINE_HOOK(0x62A735, ParasiteClass_ExitUnit_ExitSound, 0xA) //ParasiteClass_Uninfect
{
	GET(ParasiteClass* const, pParasite, ESI);

	if (auto const pParasiteOwner = pParasite->Owner)
	{
		auto nCoord = pParasiteOwner->GetCoords();
		VoxClass::PlayAtPos(TechnoTypeExt::ExtMap.Find(pParasiteOwner->GetTechnoType())->ParasiteExit_Sound.Get(), &nCoord);
	}

	return 0;
}


//
DEFINE_HOOK(0x629B3F, ParasiteClass_SquiddyGrab_DeharcodeSplash, 0x5) // 7
{
	enum { Handled = 0x629B9C, Continue = 0x0 };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFS(0x6C, 0x4C));
	GET(int, nX, ECX);
	GET(int, nY, EAX);
	GET(int, nZ, EDX);
	GET(ParasiteClass* const, pThis, ESI);

	if (auto const AnimType = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->SquidSplash.GetElements(RulesClass::Instance->SplashList))
	{
		if (auto const pSplashType = AnimType.at(ScenarioClass::Instance->Random.RandomFromMax((AnimType.size() - 1))))
		{
			CoordStruct nCoord { nX , nY , nZ };
			if (auto pAnim = GameCreate<AnimClass>(pSplashType, nCoord))
			{
				auto const Invoker = (pThis->Owner) ? pThis->Owner->GetOwningHouse() : nullptr;
				AnimExt::SetAnimOwnerHouseKind(pAnim, Invoker, (pThis->Victim) ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
				return Handled;
			}
		}
	}

	return Continue;
}

