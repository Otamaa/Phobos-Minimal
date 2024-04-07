#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <SlaveManagerClass.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include "Header.h"

#include <Conversions.h>

DEFINE_HOOK(0x6F47A0, TechnoClass_GetBuildTime, 5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto what = pThis->WhatAmI();
	const bool isNaval = what == UnitClass::AbsID && pType->Naval;
	int finalSpeed = 0;

	if (const auto pOwner = pThis->Owner)
	{
		const int cap = RulesExtData::Instance()->MultipleFactoryCap.Get(what, isNaval);
		const double nFactorySpeed = pTypeExt->BuildTime_MultipleFactory.Get(RulesClass::Instance->MultipleFactory);
		finalSpeed = (int)(pType->BuildTimeMultiplier * pOwner->GetBuildTimeMult(pType) * (double)pType->GetBuildSpeed());

		//Power
		const double nPowerPercentage = pOwner->GetPowerPercentage();

		//if the house dont have power at all disable all the penalties
		{

			const double nLowPowerPenalty = pTypeExt->BuildTime_LowPowerPenalty.Get(RulesClass::Instance->LowPowerPenaltyModifier);
			const double nMinLowPoweProductionSpeed = pTypeExt->BuildTime_MinLowPower.Get(RulesClass::Instance->MinLowPowerProductionSpeed);
			const double nMaxLowPowerProductionSpeed = pTypeExt->BuildTime_MaxLowPower.Get(RulesClass::Instance->MaxLowPowerProductionSpeed);
			double powerdivisor = 1.0 - nLowPowerPenalty * (1.0 - nPowerPercentage);

			if (powerdivisor <= nMinLowPoweProductionSpeed)
			{
				powerdivisor = nMinLowPoweProductionSpeed;
			}

			if (nPowerPercentage < 1.0 && powerdivisor >= nMaxLowPowerProductionSpeed)
			{
				powerdivisor = nMaxLowPowerProductionSpeed;
			}

			if (powerdivisor < 0.01) {
				powerdivisor = 0.01;
			}

			finalSpeed = int((double)finalSpeed / powerdivisor);
		}

		if (nFactorySpeed > 0.0)
		{//Multiple Factory

			const int factoryCount = pOwner->FactoryCount(what, isNaval);
			const int divisor = (cap > 0 && factoryCount >= cap) ? cap : factoryCount;

			for (int i = divisor - 1; i > 0 ; --i) {
				finalSpeed *= nFactorySpeed;
			}
		}

		const auto bonus = BuildingTypeExtData::GetExternalFactorySpeedBonus(pThis);
		if(bonus > 0.0)
			finalSpeed = int((double)finalSpeed * bonus);
	}

	{ //Exception
		if (what == BuildingClass::AbsID && !pTypeExt->BuildTime_Speed.isset() && static_cast<BuildingTypeClass*>(pType)->Wall)
			finalSpeed = int((double)finalSpeed * RulesClass::Instance->WallBuildSpeedCoefficient);
	}

	R->EAX(finalSpeed);
	return 0x6F4955;
}

//The stack is messed up here , idk
// i cant properly catch them , it is just return garbages
//DEFINE_HOOK(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(WeaponTypeClass*, pWeapon, EBX);
//
//	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
//
//	if(pWeaponExt->IsDetachedRailgun || (pWeapon->IsRailgun && !pThis->RailgunParticleSystem))
//	{
//		GET_BASE(AbstractClass* , pTarget, 0x8);
//		LEA_STACK(CoordStruct* , pTo, 0xB4 - 0x74);
//		LEA_STACK(CoordStruct* , pFrom, 0xB4 - 0x1C);
//		LEA_STACK(CoordStruct* , pBuffer, 0xB4 - 0x80);
//
//		Debug::Log("Railgun[%s]  From [%d %d %d] To [%d %d %d]\n", pWeapon->ID,
//			pFrom->X,
//			pFrom->Y,
//			pFrom->Z,
//			pTo->X,
//			pTo->Y,
//			pTo->Z
//		);
//
//		pBuffer = pThis->DealthParticleDamage(pFrom , pTo , pTarget , pWeapon);
//		const auto pParticle = GameCreate<ParticleSystemClass>(pWeapon->AttachedParticleSystem, pTo, nullptr, pThis, pBuffer, pThis->Owner);
//
//		if(!pWeaponExt->IsDetachedRailgun)
//			pThis->RailgunParticleSystem = pParticle;
//	}
//
//	return 0x6FF274;
//}
//
//DEFINE_DISABLE_HOOK(0x6FF26E, TechnoClass_Fire_DetachedRailgun2_ares)

DEFINE_HOOK(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	//const bool IsRailgun = pWeapon->IsRailgun || pWeaponExt->IsDetachedRailgun;

	//if (IsRailgun && Is_Aircraft(pThis))
	//{
		//Debug::Log("TechnoClass_FireAt Aircraft[%s] attempting to fire Railgun !\n", pThis->get_ID());
		//return 0x6FF274;
	//}

	return pWeaponExt->IsDetachedRailgun
		? 0x6FF20F : 0x0;
}

DEFINE_HOOK(0x6FF26E, TechnoClass_Fire_DetachedRailgun2, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBX);

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->IsDetachedRailgun
		? 0x6FF274 : 0x0;
}

DEFINE_HOOK(0x6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	return (pThis->WhatAmI() == AbstractType::Aircraft) ? 0x6FA4D1 : 0;
}

DEFINE_HOOK(0x70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const nAmount = TechnoExt_ExtData::GetSelfHealAmount(pThis);
	R->EAX(nAmount > 0 || nAmount != 0);
	return 0x70BF46;
}

// DEFINE_HOOK(0x6FA743, TechnoClass_Update_SelfHeal, 0xA)
// {
// 	enum
// 	{
// 		ContineCheckUpdateSelfHeal = 0x6FA75A,
// 		SkipAnySelfHeal = 0x6FA941,
// 	};

// 	GET(TechnoClass* const, pThis, ESI);


// 	//handle everything
// 	return SkipAnySelfHeal;
// }

// spark particle systems created at random intervals
DEFINE_HOOK(0x6FAD49, TechnoClass_Update_SparkParticles, 8) // breaks the loop
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x60);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems))
	{
		auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

		for (auto pSystem : it)
		{
			if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Spark)
			{
				Systems.AddItem(pSystem);
			}
		}
	}

	return 0x6FADB3;
}

// customizable cloaking stages
DEFINE_HOOK(0x7036EB, TechnoClass_Uncloak_CloakingStages, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	R->ECX(pTypeExt->CloakStages.Get(RulesClass::Instance->CloakingStages));
	return 0x7036F1;
}

DEFINE_HOOK(0x703A79, TechnoClass_VisualCharacter_CloakingStages, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	int stages = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->CloakStages.Get(RulesClass::Instance->CloakingStages);
	R->EAX(int(pThis->CloakProgress.Value * 256.0 / stages));
	return 0x703A94;
}

// make damage sparks customizable, using game setting as default.
DEFINE_HOOK(0x6FACD9, TechnoClass_Update_DamageSparks, 6)
{
	GET(TechnoClass*, pThis, ESI);

	if (!pThis->SparkParticleSystem)
		return 0x6FAF01;

	GET(TechnoTypeClass*, pType, EBX);

	if (pThis->GetHealthPercentage() >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10)
		return 0x6FAF01;

	return TechnoTypeExtContainer::Instance.Find(pType)->DamageSparks.Get(pType->DamageSparks) ?
		0x6FAD17 : 0x6FAF01;
}

DEFINE_HOOK(0x70380A, TechnoClass_Cloak_CloakSound, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	R->ECX(pExt->CloakSound.Get(RulesClass::Instance->CloakSound));

	if (const auto pAnimType = pExt->CloakAnim.Get(RulesExtData::Instance()->CloakAnim)) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->GetCoords()),
			pThis->Owner,
			nullptr,
			false
		);
	}
	return 0x703810;
}

DEFINE_HOOK(0x70375B, TechnoClass_Uncloak_DecloakSound, 6)
{
	GET(int, ptr, ESI);
	const TechnoClass* pThis = reinterpret_cast<TechnoClass*>(ptr - 0x9C);
	const int nDefault = RulesExtData::Instance()->DecloakSound.Get(RulesClass::Instance->CloakSound);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	R->ECX(pTypeExt->DecloakSound.Get(nDefault));

	if (const auto pAnimType = pTypeExt->DecloakAnim.Get(RulesExtData::Instance()->DecloakAnim)) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pThis->GetCoords()),
			pThis->Owner,
			nullptr,
			false
		);
	}

	return 0x703761;
}

// linking units for type selection
DEFINE_HOOK(0x732C30, TechnoClass_IDMatches, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET(DynamicVectorClass<const char*>*, pNames, EDX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	const char* id = TechnoTypeExtContainer::Instance.Find(pType)
		->GetSelectionGroupID();
	bool match = false;
	const auto what = pThis->WhatAmI();

	// find any match
	for (auto i = pNames->begin(); i < pNames->end(); ++i)
	{
		if (IS_SAME_STR_(*i, id))
		{
			if (pThis->CanBeSelectedNow())
			{
				match = true;
				break;
			}

			// buildings are exempt if they can't undeploy
			if (what == BuildingClass::AbsID && pType->UndeploysInto)
			{
				match = true;
				break;
			}
		}
	}

	R->EAX(match);
	return 0x732C97;
}

DEFINE_HOOK(0x6F3950, TechnoClass_GetCrewCount, 8)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();

	// previous default for crew count was -1
	int count = TechnoTypeExtContainer::Instance.Find(pType)->Survivors_PilotCount.Get();
	// default to original formula
	if (count < 0)
	{
		count = pType->Crewed ? 1 : 0;
	}

	R->EAX(count);
	return 0x6F3967;
}

// Support per unit modification of Iron Curtain effect duration
DEFINE_HOOK(0x70E2B0, TechnoClass_IronCurtain, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, duration, STACK_OFFS(0x0, -0x4));
	//GET_STACK(HouseClass*, source, STACK_OFFS(0x0, -0x8));
	GET_STACK(bool, force, STACK_OFFS(0x0, -0xC));

	// if it's no force shield then it's the iron curtain.
	const auto pData = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const auto modifier = (force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier).Get();

	pThis->IronCurtainTimer.Start(int(duration * modifier));
	pThis->IronTintStage = 0;
	pThis->ProtectType = force ? ProtectTypes::ForceShield : ProtectTypes::IronCurtain;

	R->EAX(DamageState::Unaffected);
	return 0x70E2FD;
}

DEFINE_HOOK(0x7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	R->EAX<int>(TechnoTypeExtData::HasSelectionGroupID(pThis->GetTechnoType(), pID));
	return 0x7327B2;
}

// #912875: respect the remove flag for invalidating SpawnManager owners
DEFINE_HOOK(0x707B19, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EBP);
	GET_STACK(bool, remove, 0x28);

	if(!pThis->SpawnManager || !remove &&  pThis->Owner == ptr)
		return 0x707B29;

	R->ECX(pThis->SpawnManager);
	return 0x707B23;
}

DEFINE_HOOK(0x70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto PlayEva = [](const char* pEva, CDTimerClass& nTimer, double nRate)
		{
			if (!nTimer.GetTimeLeft())
			{
				nTimer.Start(GameOptionsClass::Instance->GetAnimSpeed(static_cast<int>(nRate * 900.0)));
				VoxClass::Play(pEva);
			}
		};

	if (detect && pTypeExt->SensorArray_Warn)
	{
		switch (detect)
		{
		case 1:
			PlayEva("EVA_CloakedUnitDetected", HouseExtData::CloakEVASpeak, RulesExtData::Instance()->StealthSpeakDelay);
			break;
		case 2:
			PlayEva("EVA_SubterraneanUnitDetected", HouseExtData::SubTerraneanEVASpeak, RulesExtData::Instance()->SubterraneanSpeakDelay);
			break;
		}

		CellStruct cell = CellClass::Coord2Cell(pThis->Location);
		RadarEventClass::Create(RadarEventType::EnemySensed, cell);
	}

	return 0x70DADC;
}

DEFINE_HOOK(0x70CBB0, TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (!pWeapon->AmbientDamage)
		return 0x70CC3E;

	R->EDI(pWeapon);
	R->ESI(0);
	return (!(R->EAX<int>() <= 0)) ? 0x70CBB9 : 0x70CBF7;
}

// the fuck , game calling `MapClass[]` multiple times , fixed it
DEFINE_HOOK(0x6FB5F0, TechnoClass_DeleteGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	const int nDecrease = pCell->GapsCoveringThisCell - 1;
	pCell->GapsCoveringThisCell = nDecrease;

	if (!HouseClass::CurrentPlayer->SpySatActive || nDecrease > 0)
		return 0x6FB69E;

	--pCell->ShroudCounter;

	if (pCell->ShroudCounter <= 0)
		pCell->AltFlags |= (AltCellFlags::NoFog | AltCellFlags::Mapped);

	return 0x6FB69E;
}

DEFINE_HOOK(0x6FB306, TechnoClass_CreateGap_Optimize, 6)
{
	GET(CellClass*, pCell, EAX);

	int nCounter = pCell->ShroudCounter;
	int nCounter_b = nCounter;
	if (nCounter >= 0 && nCounter != 1)
	{
		nCounter_b = nCounter + 1;
		pCell->ShroudCounter = nCounter + 1;
	}
	++pCell->GapsCoveringThisCell;
	if (nCounter_b >= 1)
		pCell->UINTAltFlags &= 0xFFFFFFE7;

	return 0x6FB3BD;
}

DEFINE_HOOK(0x6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	return !TechnoExt_ExtData::CloakDisallowed(pThis, false) ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_HOOK(0x6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	R->EAX(TechnoExt_ExtData::CloakDisallowed(pThis, true));
	return 0x6FBDBC;
}

DEFINE_HOOK(0x6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	R->EAX(TechnoExt_ExtData::CloakAllowed(pThis));
	return 0x6FBF93;
}

DEFINE_HOOK(0x6F6AC9, TechnoClass_Remove_Early, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// if the removed object is a radar jammer, unjam all jammed radars
	TechnoExtContainer::Instance.Find(pThis)->RadarJammer.reset(nullptr);
	// #617 powered units
	TechnoExtContainer::Instance.Find(pThis)->PoweredUnit.reset(nullptr);


	//#1573, #1623, #255 attached effects
	AresAE::Remove(&TechnoExtContainer::Instance.Find(pThis)->AeData , pThis);

	if (TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount != 0) {
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, true);
	}

	return pThis->InLimbo ? 0x6F6C93u : 0x6F6AD5u;
}

DEFINE_HOOK_AGAIN(0x6F6D0E, TechnoClass_Put_BuildingLight, 7)
DEFINE_HOOK(0x6F6F20, TechnoClass_Put_BuildingLight, 6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	//only update the SW if really needed it
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsUnavailable();

	if (pTypeExt->HasSpotlight)
	{
		TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
	}

	return 0x0;
}

DEFINE_HOOK(0x6FD0BF, TechnoClass_GetROF_AttachEffect, 6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto nRof = TechnoExtContainer::Instance.Find(pThis)->AE_ROF;
	__asm { fmul nRof };
	return 0x0;
}

DEFINE_HOOK(0x707D20, TechnoClass_GetCrew, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto pHouse = pThis->Owner;
	InfantryTypeClass* pCrewType = nullptr;

	// YR defaults to 15 for armed objects,
	// Ares < 0.5 defaulted to 0 for non-buildings.
	const int TechnicianChance = pExt->Crew_TechnicianChance.Get(
		(pThis->AbstractFlags & AbstractFlags::Foot) ?
		0 :
		pThis->IsArmed() ? 15 : 0
	);

	if (pType->Crewed)
	{
		// for civilian houses always technicians. random for others
		const bool isTechnician = pHouse->Type->SideIndex == -1 ? true :
			TechnicianChance > 0 && ScenarioClass::Instance->Random.RandomFromMax(99) < TechnicianChance
			? true : false;

		// chose the appropriate type
		if (!isTechnician)
		{
			// customize with this techno's pilot type
			// only use it if non-null, as documented

			const auto& nVec = pExt->Survivors_Pilots;

			if ((size_t)pHouse->SideIndex >= nVec.size())
			{
				pCrewType = HouseExtData::GetCrew(pHouse);
			}
			else if (auto pPilotType = nVec[pHouse->SideIndex])
			{
				pCrewType = pPilotType;
			}
			else
			{
				pCrewType = HouseExtData::GetCrew(pHouse);
			}
		}
		else
		{
			// either civilian side or chance
			pCrewType = HouseExtData::GetTechnician(pHouse);
		}
	}

	R->EAX(pCrewType);
	return 0x707DCF;
}

// complete replacement
DEFINE_HOOK(0x70FBE0, TechnoClass_Activate_AresReplace, 6)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pType = pThis->GetTechnoType();

	if (pType->PoweredUnit && pThis->Owner)
	{
		pThis->Owner->RecheckPower = true;
	}

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
		If any of the above conditions, bail out and don't activate the object.
	*/

	if (pThis->IsUnderEMP() || !TechnoExt_ExtData::IsPowered(pThis))
	{
		return 0x70FC85;
	}

	if (TechnoExt_ExtData::IsOperatedB(pThis))
	{
		pThis->Guard();

		if (auto const pFoot = abstract_cast<FootClass*>(pThis))
		{
			pFoot->Locomotor.GetInterfacePtr()->Power_On();
		}

		if (auto const wasDeactivated = std::exchange(pThis->Deactivated, false))
		{
			// change: don't play sound when mutex active
			if (!Unsorted::ScenarioInit && pType->ActivateSound != -1)
			{
				VocClass::PlayAt(pType->ActivateSound, pThis->Location, nullptr);
			}

			// change: add spotlight
			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
			if (pTypeExt->HasSpotlight)
			{
				++Unsorted::ScenarioInit;
				TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
				--Unsorted::ScenarioInit;
			}

			// change: update factories
			if (auto const pBld = specific_cast<BuildingClass*>(pThis))
			{
				TechnoExt_ExtData::UpdateFactoryQueues(pBld);
			}
		}
	}

	return 0x70FC85;
}


DEFINE_HOOK(0x6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(LaserDrawClass*, pBeam, EAX);

	auto const pData = WeaponTypeExtContainer::Instance.Find(pWeapon);
	if (!pBeam->IsHouseColor && WeaponTypeExtContainer::Instance.Find(pWeapon)->Laser_IsSingleColor)
		pBeam->IsHouseColor = true;

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	if (pData->Laser_Thickness > 1) {
		pBeam->Thickness = pData->Laser_Thickness;
	}

	pBeam->IsSupported = pBeam->Thickness > 3;

	return 0;
}

DEFINE_HOOK(0x6f526c, TechnoClass_DrawExtras_PowerOff, 5)
{
	GET(TechnoClass*, pTechno, EBP);

	if(!pTechno->IsAlive)
		return 0x6F5347;

	GET_STACK(RectangleStruct*, pRect, 0xA0);

	if (auto pBld = specific_cast<BuildingClass*>(pTechno))
	{
		const auto pBldExt = BuildingExtContainer::Instance.Find(pBld);
		const auto isObserver = HouseClass::IsCurrentPlayerObserver();

		// allies and observers can always see by default
		const bool canSeeRepair = HouseClass::CurrentPlayer->IsAlliedWith(pBld->Owner)
			|| isObserver;

		const bool showRepair = FileSystem::WRENCH_SHP
			&& pBld->IsBeingRepaired
			// fixes the wrench playing over a temporally challenged building
			&& !pBld->IsBeingWarpedOut()
			&& !pBld->WarpingOut
			// never show to enemies when cloaked, and only if allowed
			&& (canSeeRepair || (pBld->CloakState == CloakState::Uncloaked
				&& RulesExtData::Instance()->EnemyWrench));

		// display power off marker only for current player's buildings
		const bool showPower = FileSystem::POWEROFF_SHP
			&& (!pBldExt->TogglePower_HasPower)
			// only for owned buildings, but observers got magic eyes
			&& ((pBld->GetCurrentMission() != Mission::Selling) && (pBld->GetCurrentMission() != Mission::Construction))
			&& (pBld->Owner->ControlledByCurrentPlayer() || isObserver);

		// display any?
		if (showPower || showRepair)
		{
			auto cell = pBld->GetMapCoords();

			if (!MapClass::Instance->GetCellAt(cell)->IsShrouded())
			{
				CoordStruct crd = pBld->GetCenterCoords();

				Point2D point {};
				TacticalClass::Instance->CoordsToClient(&crd, &point);

				// offset the markers
				Point2D ptRepair = point;
				if (showPower)
				{
					ptRepair.X -= 7;
					ptRepair.Y -= 7;
				}

				Point2D ptPower = point;
				if (showRepair)
				{
					ptPower.X += 18;
					ptPower.Y += 18;
				}

				// animation display speed
				// original frame calculation: ((currentframe%speed)*6)/(speed-1)
				const int speed = MaxImpl(GameOptionsClass::Instance->GetAnimSpeed(14) / 4, 2);

				// draw the markers
				if (showRepair)
				{
					int frame = (FileSystem::WRENCH_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::WRENCH_SHP,
						frame, &ptRepair, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}

				if (showPower)
				{
					int frame = (FileSystem::POWEROFF_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Temp->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::POWEROFF_SHP,
						frame, &ptPower, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0x6F5347;
}

DEFINE_HOOK(0x70AA60, TechnoClass_DrawExtraInfo, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);
	//GET_STACK(Point2D*, pOriginalPoint, 0x8);
	//	GET_STACK(unsigned int , nFrame, 0x4);
	GET_STACK(RectangleStruct*, pRect, 0xC);

	if (!HouseClass::CurrentPlayer)
		return 0x70AD4C;

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pType = pBuilding->Type;
		auto const pOwner = pBuilding->Owner;
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (!pType || !pOwner)
			return 0x70AD4C;

		Point2D DrawLoca = *pPoint;
		auto DrawTheStuff = [&](const wchar_t* pFormat)
			{
				//DrawingPart
				RectangleStruct nTextDimension;
				Drawing::GetTextDimensions(&nTextDimension, pFormat, DrawLoca, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
				auto nIntersect = Drawing::Intersect(nTextDimension, *pRect);
				auto nColorInt = pOwner->Color.ToInit();//0x63DAD0

				DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
				DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
				Point2D nRet;
				Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pRect, &DrawLoca, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
				DrawLoca.Y += (nTextDimension.Height) + 2; //extra number for the background
			};

		const bool IsAlly = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
		const bool isFake = pTypeExt->Fake_Of.Get();
		const bool bReveal = pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

		if (IsAlly || IsObserver || bReveal)
		{
			if (isFake)
				DrawTheStuff(StringTable::LoadString("TXT_FAKE"));

			if (pType->PowerBonus > 0)
			{
				auto pDrainFormat = StringTable::LoadString(GameStrings::TXT_POWER_DRAIN2());
				wchar_t pOutDraimFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				swprintf_s(pOutDraimFormat, pDrainFormat, pOutput, pDrain);
				DrawTheStuff(pOutDraimFormat);
			}

			if (pType->Storage > 0)
			{
				auto pMoneyFormat = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1());
				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, pMoneyFormat, nMoney);
				DrawTheStuff(pOutMoneyFormat);
			}

			if (pThis->IsPrimaryFactory)
			{
				DrawTheStuff(StringTable::LoadString((pType->GetFoundationWidth() != 1) ?
					GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
			}

			if(!BuildingExtContainer::Instance.Find(pBuilding)->RegisteredJammers.empty())
				DrawTheStuff(Phobos::UI::BuidingRadarJammedLabel);
		}
	}

	return 0x70AD4C;
}

// complete replacement
DEFINE_HOOK(0x70FC90, TechnoClass_Deactivate_AresReplace, 6)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pType = pThis->GetTechnoType();

	if (pType->PoweredUnit && pThis->Owner)
	{
		pThis->Owner->RecheckPower = true;
	}

	// don't deactivate when inside/on the linked building
	if (pThis->IsTethered)
	{
		auto const pLink = pThis->GetNthLink(0);

		if (pLink && pThis->GetCell()->GetBuilding() == pLink)
		{
			return 0x70FD6E;
		}
	}

	pThis->Guard();
	pThis->Deselect();

	if (auto const pFoot = abstract_cast<FootClass*>(pThis))
	{
		pFoot->Locomotor.GetInterfacePtr()->Power_Off();
	}

	auto const wasDeactivated = std::exchange(pThis->Deactivated, true);

	if (!wasDeactivated)
	{
		// change: don't play sound when mutex active
		if (!Unsorted::ScenarioInit && pType->DeactivateSound != -1)
		{
			VocClass::PlayAt(pType->DeactivateSound, pThis->Location, nullptr);
		}

		// change: remove spotlight
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		if (pTypeExt->HasSpotlight)
		{
			TechnoExt_ExtData::SetSpotlight(pThis, nullptr);
		}

		// change: update factories
		if (auto const pBld = specific_cast<BuildingClass*>(pThis))
		{
			TechnoExt_ExtData::UpdateFactoryQueues(pBld);
		}
	}

	return 0x70FD6E;
}

DEFINE_HOOK_AGAIN(0x6FB4A3, TechnoClass_CreateGap_LargeGap, 7)
DEFINE_HOOK(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	pThis->GapRadius = TechnoTypeExtContainer::Instance.Find(pType)->GapRadiusInCells;
	return R->Origin() + 0xD;
}

// Radar Jammers (#305) unjam all on owner change
DEFINE_HOOK(0x7014D5, TechnoClass_ChangeOwnership_Additional, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	//Debug::Log("ChangeOwnershipFor [%s]\n" , pThis->get_ID());

	if (auto& pJammer = TechnoExtContainer::Instance.Find(pThis)->RadarJammer)
	{
		pJammer->UnjamAll();
	}

	if (TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount != 0)
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, true);

	return 0;
}

DEFINE_HOOK(0x702E64, TechnoClass_RegisterDestruction_Bounty, 6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);

	TechnoExt_ExtData::GiveBounty(pVictim, pKiller);

	return 0x0;
}

DEFINE_HOOK(0x6FAF0D, TechnoClass_Update_EMPLock, 6)
{
	GET(TechnoClass*, pThis, ESI);

	// original code.
	const auto was = pThis->EMPLockRemaining;
	if (was > 0)
	{
		pThis->EMPLockRemaining = was - 1;
		if (was == 1)
		{
			// the forced vacation just ended. we use our own
			// function here that is quicker in retrieving the
			// EMP animation and does more stuff.
			AresEMPulse::DisableEMPEffect(pThis);
		}
		else
		{
			// deactivate units that were unloading afterwards
			if (!pThis->Deactivated && AresEMPulse::IsDeactivationAdvisable(pThis))
			{
				// update the current mission
				TechnoExtContainer::Instance.Find(pThis)->EMPLastMission = pThis->CurrentMission;
				pThis->Deactivate();
			}
		}
	}

	return 0x6FAFFD;
}

DEFINE_HOOK(0x6F3F88, TechnoClass_Init_1, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();

	CaptureManagerClass* pCapturer = nullptr;
	ParasiteClass* pParasite = nullptr;
	TemporalClass* pTemporal = nullptr;
	SpawnManagerClass* pSpawnManager = nullptr;
	SlaveManagerClass* pSlaveManager = nullptr;
	AirstrikeClass* pAirstrike = nullptr;

	if (pType->Spawns) {
		pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pType->Spawns, pType->SpawnsNumber, pType->SpawnRegenRate, pType->SpawnReloadRate);
	}

	if (pType->Enslaves) {
		pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pType->Enslaves, pType->SlavesNumber, pType->SlaveRegenRate, pType->SlaveReloadRate);
	}

	if (pType->AirstrikeTeam > 0 && pType->AirstrikeTeamType) {
		pAirstrike = GameCreate<AirstrikeClass>(pThis);
	}

	const bool IsFoot = pThis->WhatAmI() != BuildingClass::AbsID;

	const int WeaponCount = pType->TurretCount <= 0 ? 2 : pType->WeaponCount;

	for (auto i = 0; i < WeaponCount; ++i)
	{

		if (auto const pWeapon = pType->GetWeapon(i)->WeaponType)
		{
			TechnoExt_ExtData::InitWeapon(pThis, pType, pWeapon, i, pCapturer, pParasite, pTemporal, "Weapon", IsFoot);
		}

		if (auto const pWeaponE = pType->GetEliteWeapon(i)->WeaponType)
		{
			TechnoExt_ExtData::InitWeapon(pThis, pType, pWeaponE, i, pCapturer, pParasite, pTemporal, "EliteWeapon", IsFoot);
		}
	}

	pThis->CaptureManager = pCapturer;
	pThis->TemporalImUsing = pTemporal;
	if (IsFoot)
	{
		((FootClass*)pThis)->ParasiteImUsing = pParasite;
	}
	pThis->SpawnManager = pSpawnManager;
	pThis->SlaveManager = pSlaveManager;
	pThis->Airstrike = pAirstrike;

	if (auto pOwner = pThis->Owner)
	{
		const auto pHouseType = pOwner->Type;
		const auto pParentHouseType = pHouseType->FindParentCountry();
		TechnoExtContainer::Instance.Find(pThis)->OriginalHouseType = pParentHouseType ? pParentHouseType : pHouseType;
	}
	else
	{
		Debug::Log("Techno[%s] Init Without any ownership!\n", pType->ID);
	}
	// if override is in effect, do not create initial payload.
	// this object might have been deployed, undeployed, ...
	if (Unsorted::ScenarioInit && Unsorted::CurrentFrame)
	{
		TechnoExtContainer::Instance.Find(pThis)->PayloadCreated = true;
	}

	R->EAX(pType);
	return 0x6F4212;
}

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
DEFINE_HOOK(0x6F7561, TechnoClass_Targeting_Arcing_Aircraft, 0x5)
{
	GET(AbstractType, pTarget, EAX);
	GET(CoordStruct*, pCoord, ESI);
	R->EAX(pCoord->X);
	return pTarget == AbstractType::Aircraft ? 0x6F75B2 : 0x6F7568;
}

// No data found on .inj for this
//DEFINE_HOOK(0x5F7933, TechnoTypeClass_FindFactory_ExcludeDisabled, 0x6)
//{
//	GET(BuildingClass*, pBld, ESI);
//
//	 //add the EMP check to the limbo check
//	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
//		0x5F7A57 : 0x5F7941;
//}

DEFINE_HOOK(0x6F90F8, TechnoClass_SelectAutoTarget_Demacroize, 0x6)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x6F9116;
}

DEFINE_HOOK(0x70133E, TechnoClass_GetWeaponRange_Demacroize, 0x5)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EBX);

	R->EAX(nVal1 >= nVal2 ? nVal2 : nVal1);
	return 0x701388;
}

DEFINE_HOOK(0x707EEA, TechnoClass_GetGuardRange_Demacroize, 0x6)
{
	GET(int, nVal1, EBX);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x707F08;
}

// customizable berserk fire rate modification
DEFINE_HOOK(0x6FF28F, TechnoClass_Fire_BerserkROFMultiplier, 6)
{
	enum { SkipROF = 0x6FF2BE , SetROF = 0x6FF29E };
	GET(TechnoClass*, pThis, ESI);
	GET(int, ROF, EAX);

	if (pThis->Berzerk) {
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		double multiplier = pExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
		ROF = static_cast<int>(ROF * multiplier);
	}

	R->EAX(ROF);
	return SetROF;
}

DEFINE_HOOK(0x6FE709, TechnoClass_Fire_BallisticScatter1, 6)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for FlakScatter && !Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(0));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE71C;
}

DEFINE_HOOK(0x6FE7FE, TechnoClass_Fire_BallisticScatter2, 5)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for !FlakScatter || Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(RulesClass::Instance->BallisticScatter / 2));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE821;
}

DEFINE_HOOK(0x707A47, TechnoClass_PointerGotInvalid_LastTarget, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->LastTarget == ptr)
		pThis->LastTarget = nullptr;

	return 0;
}

//TechnoClass_SetTarget_Burst
DEFINE_JUMP(LJMP, 0x6FCF53, 0x6FCF61);

DEFINE_HOOK_AGAIN(0x717855, TechnoTypeClass_UpdatePalette_Reset, 0x6)
DEFINE_HOOK(0x717823, TechnoTypeClass_UpdatePalette_Reset, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->Palette = nullptr;

	return 0;
}

DEFINE_HOOK(0x71136F, TechnoTypeClass_CTOR_Initialize, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->WeaponCount = 0; //default
	pThis->Bunkerable = false;
	pThis->Parasiteable = false;
	pThis->ImmuneToPoison = false;
	pThis->ConsideredAircraft = false;

	return 0;
}

DEFINE_HOOK(0x7119D5, TechnoTypeClass_CTOR_NoInit_Particles, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI)

	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DamageParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();
	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DestroyParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();

	return 0x711A00;
}

// destroying a building (no health left) resulted in a single green pip shown
// in the health bar for a split second. this makes the last pip red.
DEFINE_HOOK(0x6F661D, TechnoClass_DrawHealthBar_DestroyedBuilding_RedPip, 0x7)
{
	GET(BuildingClass*, pBld, ESI);
	return (pBld->Health <= 0 || pBld->IsRedHP()) ? 0x6F6628 : 0x6F6630;
}

// issues 1002020, 896263, 895954: clear stale mind control pointer to prevent
// crashes when accessing properties of the destroyed controllers.
DEFINE_HOOK(0x707B09, TechnoClass_PointerGotInvalid_ResetMindControl, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->MindControlledBy == ptr)
	{
		pThis->MindControlledBy = nullptr;
	}

	return 0;
}

//TechnoClass_GetActionOnObject_IvanBombsB
DEFINE_JUMP(LJMP, 0x6FFF9E, 0x700006);

DEFINE_HOOK(0x6FF2D1, TechnoClass_FireAt_Facings, 0x6)
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

DEFINE_HOOK(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto pBulletExt = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	const auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	R->EAX(ret);
	return 0x6FE562;
}

DEFINE_HOOK(0x6F826E, TechnoClass_CanAutoTargetObject_CivilianEnemy, 0x5)
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

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	// always consider this an enemy
	if (pExt->CivilianEnemy) {
		return ConsiderEnemy;
	}

	// if the potential target is attacking an allied object, consider it an enemy
	// to not allow civilians to overrun a player
	if (const auto pTargetTarget = abstract_cast<TechnoClass*>(pTarget->Target)) {
		if (pThis->Owner->IsAlliedWith(pTargetTarget)) {
			const auto pData = RulesExtData::Instance();

			if (pThis->Owner->IsControlledByHuman() ?
				pData->AutoRepelPlayer : pData->AutoRepelAI) {
				return ConsiderEnemy;
			}
		}
	}

	return Undecided;
}

DEFINE_HOOK(0x7162B0, TechnoTypeClass_GetPipMax_MindControl, 0x6)
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

DEFINE_HOOK(0x6FC3FE, TechnoClass_CanFire_Immunities, 0x6)
{
	enum { FireIllegal = 0x6FC86A, ContinueCheck = 0x6FC425 };

	GET(WarheadTypeClass* , pWarhead , EAX);
	GET(TechnoClass*, pTarget, EBP);

	if(pTarget)	{
		//const auto nRank = pTarget->Veterancy.GetRemainingLevel();

		//const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		//if(pWHExt->ImmunityType.isset() &&
		//	 TechnoExtData::HasImmunity(nRank, pTarget , pWHExt->ImmunityType.Get()))
		//	return FireIllegal;

		if(pWarhead->Psychedelic && TechnoExtData::IsPsionicsImmune(pTarget))
			return FireIllegal;
	}

	return ContinueCheck;
}

// issue #895788: cells' high occupation flags are marked only if they
// actually contains a bridge while unmarking depends solely on object
// height above ground. this mismatch causes the cell to become blocked.
DEFINE_HOOK(0x7441B0, UnitClass_MarkOccupationBits, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pCrd, 0x4);

	CellClass* pCell = MapClass::Instance->GetCellAt(pCrd);
	int height = MapClass::Instance->GetCellFloorHeight(pCrd) + Unsorted::BridgeHeight;
	bool alt = (pCrd->Z >= height && pCell->ContainsBridge());

	// remember which occupation bit we set
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->AltOccupation = alt;

	if (alt)
	{
		pCell->AltOccupationFlags |= 0x20;
	}
	else
	{
		pCell->OccupationFlags |= 0x20;
	}

	return 0x74420B;
}

DEFINE_HOOK(0x744210, UnitClass_UnmarkOccupationBits, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pCrd, 0x4);

	enum { obNormal = 1, obAlt = 2 };

	CellClass* pCell = MapClass::Instance->GetCellAt(pCrd);
	int height = MapClass::Instance->GetCellFloorHeight(pCrd) + Unsorted::BridgeHeight;
	int alt = (pCrd->Z >= height) ? obAlt : obNormal;

	// also clear the last occupation bit, if set
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	if (!pExt->AltOccupation.empty())
	{
		int lastAlt = pExt->AltOccupation ? obAlt : obNormal;
		alt |= lastAlt;
		pExt->AltOccupation.clear();
	}

	if (alt & obAlt)
	{
		pCell->AltOccupationFlags &= ~0x20;
	}

	if (alt & obNormal)
	{
		pCell->OccupationFlags &= ~0x20;
	}

	return 0x744260;
}

DEFINE_HOOK(0x6FE31C, TechnoClass_Fire_AllowDamage, 8)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	// whether conventional damage should be used
	const bool applyDamage =
		WeaponTypeExtContainer::Instance.Find(pWeapon)->ApplyDamage.Get(!pWeapon->IsSonic && !pWeapon->UseFireParticles);

	if (!applyDamage)
	{
		// clear damage
		R->EDI(0);
		return 0x6FE3DFu;
	}

	return 0x6FE32Fu;
}

// health bar for detected submerged units
DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388  , CheckDrawHealthAllowed = 0x6F538E};

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x98, -0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExtData::DrawInsignia(pThis, pLocation, pBounds);

	bool drawHealth = pThis->IsSelected;
	if (!drawHealth)
	{
		// sensed submerged units
		drawHealth = !pThis->IsSurfaced()
			&& pThis->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
	}

	R->EAX(drawHealth);
	return CheckDrawHealthAllowed;
}

// DEFINE_HOOK(0x70CBC3, TechnoClass_DealParticleDamage_FixArgs, 0x6)
// {
// 	GET(WeaponTypeClass*, pWeapon, EDI);
// 	GET(float, nDamage , ECX);
// 	GET(ObjectClass**, pVec, EDX);
// 	GET(int, nIdx, ESI);
// 	GET_STACK(TechnoClass*, pThis, 0xC0 - 0x44);
//
// 	int iDamage = (int)nDamage;
// 	pVec[nIdx]->ReceiveDamage(&iDamage, 0, pWeapon->Warhead, pThis, false, false, pThis->Owner);
//
// 	return 0x70CBEE;
// }

// this code somewhat broke targeting
// it created identically like ares but not working as expected , duh
DEFINE_HOOK(0x6FA361, TechnoClass_Update_LoseTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pHouse, EDI);

	const bool BLRes = R->BL();
	const HouseClass* pOwner = !BLRes ? pThis->Owner : pHouse;
	bool IsAlly = false;
	if (const auto pTechTarget = generic_cast<TechnoClass*>(pThis->Target))
	{
		if (const auto pTargetHouse = pTechTarget->GetOwningHouse())
		{
			if (pOwner->IsAlliedWith(pTargetHouse))
				IsAlly = true;
		}
	}

	enum { RetNotAlly = 0x6FA472, RetAlly = 0x6FA39D };
	const bool IsNegDamage = (pThis->CombatDamage() < 0);

	return IsAlly == IsNegDamage ? RetNotAlly : RetAlly;
}