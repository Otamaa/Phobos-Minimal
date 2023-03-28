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
#include <Ext/WeaponType/Body.h>

#include <Utilities/GeneralUtils.h>


//TODO :
// Parasite Heal
// Parasite Able To target friendly
// Parasite not removed when heal
// Parasite Gain victim control instead of damaging

DEFINE_HOOK(0x629FE4, ParasiteClass_IsGrapplingAttack, 0x5)
{
	enum { DoGrapple = 0x62A00E , DoParasite = 0x62A01D };
	GET(ParasiteClass*, pThis, ESI);

	auto const pOwnerType = pThis->Owner->GetTechnoType();
	auto const pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwnerType);

	return pOwnerTypeExt->GrapplingAttack.Get(pOwnerType->Naval && pOwnerType->Organic) ?
		DoGrapple : DoParasite;
}

//#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x62A0D3, ParasiteClass_AI_Particle, 0x5)
{
	GET(ParasiteClass* const, pThis, ESI);
	LEA_STACK(CoordStruct* const, pCoord, STACK_OFFS(0x4C, 0x18));
	GET(WeaponTypeClass* const, pWeapon, EDI);

	if (auto pParticle = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem))
	{
		auto nLocHere = *pCoord;
		if(pParticle->BehavesLike == BehavesLike::Smoke)
			nLocHere.Z += 100;

		GameCreate<ParticleSystemClass>(pParticle, nLocHere, pThis->Victim , pThis->Owner ,CoordStruct::Empty , pThis->Owner->Owner);
	}

	return 0x62A108;
}

DEFINE_HOOK(0x62A13F, ParasiteClass_AI_WeaponAnim, 0x5)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(AnimTypeClass* const, pAnimType, EBP);
	LEA_STACK(CoordStruct*, pStack, STACK_OFFS(0x4C, 0x18));

	if (auto pAnim = GameCreate<AnimClass>(pAnimType, pStack,0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 , 0 , 0)) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr, pThis->Victim ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
	}

	return 0x62A16A;
}

DEFINE_HOOK(0x62A074, ParasiteClass_AI_DamagingAction, 0x6)
{
	enum {
		SkippAll = 0x62A24D,
		ReceiveDamage = 0x62A0B7,
		ReceiveDamage_LikeVehicle = 0x62A0D3 // not instant kill the infantry , but gave it damage per second
	};

	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);
	GET(WarheadTypeClass* const, pWarhead, EBP);

	pThis->DamageDeliveryTimer.Start(pWeapon->ROF);
	pThis->SuppressionTimer.Start(pWarhead->Paralyzes);

	if (pThis->Victim->WhatAmI() == AbstractType::Infantry 
		&& !WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_TreatInfantryAsVehicle
		.Get(static_cast<InfantryClass*>(pThis->Victim)->Type->Cyborg)) {
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
	enum { DealDamage = 0x62A222 , Continue = 0x0 };
	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);
	
	if (pThis->Victim->IsAttackedByLocomotor)
		return DealDamage;

	return (WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->Parasite_DisableRocking.Get()) || pThis->Victim->WhatAmI() == AbstractType::Infantry
		? DealDamage : Continue;
}

DEFINE_HOOK(0x62A222, ParasiteClass_AI_DealDamage, 0x6)
{
	enum { SkipDamaging = 0x62A24D , VictimTakeDamage = 0x0 };

	GET(ParasiteClass*, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);

	auto const pWarheadTypeExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWarheadTypeExt->Parasite_Damaging_Chance.isset()
		&& ScenarioClass::Instance->Random.RandomDouble() >= abs(pWarheadTypeExt->Parasite_Damaging_Chance.Get())
		) {
		return SkipDamaging;
	}

	if (auto const pInvestationWP = pWarheadTypeExt->Parasite_InvestationWP.Get(nullptr)) {
		WeaponTypeExt::DetonateAt(pInvestationWP,pThis->Victim, pThis->Owner);
		return SkipDamaging;
	}

	return VictimTakeDamage;
}

DEFINE_HOOK_AGAIN(0x62A399, ParasiteClass_ExitUnit_ExitSound, 0x9) //ParasiteClass_Detach
DEFINE_HOOK(0x62A735, ParasiteClass_ExitUnit_ExitSound, 0xA) //ParasiteClass_Uninfect
{
	GET(ParasiteClass* const, pParasite, ESI);

	if (auto const pParasiteOwner = pParasite->Owner) {
		auto nCoord = pParasiteOwner->GetCoords();
		VoxClass::PlayAtPos(TechnoTypeExt::ExtMap.Find(pParasiteOwner->GetTechnoType())->ParasiteExit_Sound.Get(), &nCoord);
	}

	return 0;
}

DEFINE_HOOK(0x629B3F, ParasiteClass_SquiddyGrab_DeharcodeSplash, 0x5) // 7
{
	enum { Handled = 0x629B9C, Continue = 0x0 };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFS(0x6C, 0x4C));
	GET(int, nX, ECX);
	GET(int, nY, EAX);
	GET(int, nZ, EDX);
	GET(ParasiteClass* const, pThis, ESI);
	CoordStruct nCoord { nX , nY , nZ };

	if(!MapClass::Instance->GetCellAt(nCoord)->Tile_Is_Water())
		return Handled;

	if (auto const AnimType = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)->SquidSplash.GetElements(RulesClass::Instance->SplashList))
	{
		if (auto const pSplashType = AnimType[ScenarioClass::Instance->Random.RandomFromMax(AnimType.size() - 1)])
		{
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

DEFINE_HOOK(0x62991C, ParasiteClass_GrappleAI_GrappleAnimCreated, 0x8)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(AnimClass*, pGrapple, EAX);
	auto const Invoker = (pThis->Owner) ? pThis->Owner->GetOwningHouse() : nullptr;
	AnimExt::SetAnimOwnerHouseKind(pGrapple, Invoker, (pThis->Victim) ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
	return 0x0;
}

DEFINE_HOOK(0x6298CC, ParasiteClass_GrappleAI_AnimType, 0x5)
{
	enum{ NoAnim = 0x629972 , CreateAnim = 0x6298F0 };

	//GET(ParasiteClass*, pThis, ESI);
	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFS(0x6C, 0x4C));

	auto const pWeaponExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
	auto const pAnimType = pWeaponExt->Parasite_GrappleAnim.Get(RulesExt::Global()->DefaultSquidAnim.Get());

	if (!pAnimType)
		return NoAnim;

	R->EDI(pAnimType);
	return CreateAnim;
}

DEFINE_JUMP(LJMP, 0x629852, 0x629890);

DEFINE_HOOK(0x629E90, FootClass_WakeAnim_OnlyWater, 0x6)
{
	GET(FootClass*, pThis, ECX);

	return pThis->GetCell()->Tile_Is_Water() ? 0x0 : 0x629FC6;
}

//#include <DriveLocomotionClass.h>
//
//#pragma optimize("", off )
//DEFINE_HOOK(0x62A2EC, ParasiteClass_PointerGotInvalid, 0x6)
//{
//	GET(ParasiteClass*, pThis, ESI);
//	LEA_STACK(CoordStruct*, pCoordBuffer, STACK_OFFS(0x24, 0xC));
//
//	auto pResult = pThis->DetachFromVictim(pCoordBuffer);
//
//	auto Owner = pThis->Owner;
//	auto const pWhat = pThis->WhatAmI();
//
//	bool allowed = false;
//	if (pWhat == AbstractType::Unit)
//	{
//		allowed = !Owner->GetTechnoType()->Naval;
//	}
//	else if (pWhat == AbstractType::Infantry)
//	{
//		allowed = true;
//	}
//
//	if (allowed && Owner->GetHeight() > 200)
//	{
//		*pResult = Owner->Location;
//		Owner->IsFallingDown = Owner->IsABomb = true;
//	}
//
//	++Unsorted::IKnowWhatImDoing();
//	if (!*pResult) {
//		--Unsorted::IKnowWhatImDoing();
//		pThis->Owner->Health = 0;
//		pThis->Owner->UnInit();
//		pThis->Victim = nullptr;
//		return 0x62A453;
//	}
//
//	if (!pThis->Owner->Unlimbo(*pResult, pThis->Victim->PrimaryFacing.Current().Get_Dir()))
//	{
//		--Unsorted::IKnowWhatImDoing();
//		pThis->Owner->Health = 0;
//		TechnoExt::HandleRemove(pThis->Owner, nullptr, true);
//		//pThis->Owner->UnInit();
//		pThis->Victim = nullptr;
//		return 0x62A453;
//	}
//
//	pThis->Owner->unknown_abstract_array_588.Clear();
//	pThis->Owner->unknown_abstract_array_5AC.Clear();
//	auto pDrive = static_cast<DriveLocomotionClass*>(pThis->Owner->Locomotor.get());
//	pDrive->Stop_Drive();
//
//	return 0x62A372;
//}
//#pragma optimize("", on)
