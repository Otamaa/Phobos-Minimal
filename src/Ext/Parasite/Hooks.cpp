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

void TechnoExt::DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	#ifdef PARASITE_PIPS
	{
		//bool IsHost = false;
		//bool IsSelected = false;					//Red         //Green           //White
		//ColorScheme Color = IsSelected ? (IsHost ? {255, 0, 0} : {0, 255, 0}) : {255,255,255};
		int xOffset = 0;
		int yOffset = 0;

		int nBracket = pThis->GetTechnoType()->PixelSelectionBracketDelta;
		if (auto pFoot = generic_cast<FootClass*>(pThis->Disguise))
			if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::Player))
				nBracket = pFoot->GetTechnoType()->PixelSelectionBracketDelta;

		switch (pThis->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case AbstractType::Infantry:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		}

		int pipFrame = 4;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
	#endif
}

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

	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const , pWeapon, EDI);
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
	R->ECX(pThis->Victim);
	return pWarheadExt && pWarheadExt->Parasite_TreatInfantryAsVehicle.Get() ? ReceiveDamage_LikeVehicle : ReceiveDamage;
}

DEFINE_HOOK(0x62A16A, ParasiteClass_AI_DisableRocking, 0x7)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);

	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	return (pWarheadExt && pWarheadExt->Parasite_DisableRocking.Get()) || pThis->Victim->WhatAmI() == AbstractType::Infantry
		? 0x62A222 : 0x0;
}

DEFINE_HOOK(0x62A71C, ParasiteClass_ExitUnit_ExitSound, 0x6)
{
	GET(TechnoClass* const, pParasiteOwner, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x3C, 0x18));

	if (pParasiteOwner)
		if (auto const pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pParasiteOwner->GetTechnoType()))
			VoxClass::PlayAtPos(pOwnerTypeExt->ParasiteExit_Sound.Get(), &nCoord);

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

