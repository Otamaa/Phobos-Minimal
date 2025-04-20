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
#include <GameOptionsClass.h>
#include <TacticalClass.h>
#include <RadarEventClass.h>
#include <SpawnManagerClass.h>
#include <AirstrikeClass.h>


ASMJIT_PATCH(0x702DD6, TechnoClass_RegisterDestruction_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			// 85
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0;
}

ASMJIT_PATCH(0x7032B0, TechnoClass_RegisterLoss_Trigger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(HouseClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6F47A0, TechnoClass_GetBuildTime, 5)
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

		if (nFactorySpeed > 0)
		{//Multiple Factory

			const int factoryCount = pOwner->FactoryCount(what, isNaval);
			const int divisor = (cap > 0 && factoryCount >= cap) ? cap : factoryCount;

			for (int i = divisor - 1; i > 0 ; --i) {
				finalSpeed = int(finalSpeed * nFactorySpeed);
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
//ASMJIT_PATCH(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
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
//		Debug::LogInfo("Railgun[%s]  From [%d %d %d] To [%d %d %d]", pWeapon->ID,
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

// ASMJIT_PATCH(0x6FF1FB, TechnoClass_Fire_DetachedRailgun, 0x6)
// {
// 	//GET(TechnoClass*, pThis, ESI);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
// 	//const bool IsRailgun = pWeapon->IsRailgun || pWeaponExt->IsDetachedRailgun;
//
// 	//if (IsRailgun && Is_Aircraft(pThis))
// 	//{
// 		//Debug::LogInfo("TechnoClass_FireAt Aircraft[%s] attempting to fire Railgun !", pThis->get_ID());
// 		//return 0x6FF274;
// /	//}
//
// 	//return pWeaponExt->IsDetachedRailgun
// 		? 0x6FF20F : 0x0;
// }
//
// ASMJIT_PATCH(0x6FF26E, TechnoClass_Fire_DetachedRailgun2, 0x6)
// {
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	return WeaponTypeExtContainer::Instance.Find(pWeapon)->IsDetachedRailgun
// 		? 0x6FF274 : 0x0;
// }

ASMJIT_PATCH(0x6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	return (pThis->WhatAmI() == AbstractType::Aircraft) ? 0x6FA4D1 : 0;
}

ASMJIT_PATCH(0x70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const nAmount = TechnoExt_ExtData::GetSelfHealAmount(pThis);
	R->EAX(nAmount > 0 || nAmount != 0);
	return 0x70BF46;
}

// customizable cloaking stages
ASMJIT_PATCH(0x7036EB, TechnoClass_Uncloak_CloakingStages, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	R->ECX(pTypeExt->CloakStages.Get(RulesClass::Instance->CloakingStages));
	return 0x7036F1;
}

ASMJIT_PATCH(0x703A79, TechnoClass_VisualCharacter_CloakingStages, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	int stages = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->CloakStages.Get(RulesClass::Instance->CloakingStages);
	R->EAX(int(pThis->CloakProgress.Value * 256.0 / stages));
	return 0x703A94;
}

#include <ExtraHeaders/StackVector.h>

// make damage sparks customizable, using game setting as default.
ASMJIT_PATCH(0x6FACD9 , TechnoClass_AI_DamageSparks , 6)
{
	GET(TechnoClass*, pThis, ESI);

    if (!pThis->SparkParticleSystem) {
        auto _HPRatio = pThis->GetHealthPercentage();

        if (!(_HPRatio >= RulesClass::Instance->ConditionYellow || pThis->GetHeight() <= -10)) {

            auto pType = pThis->GetTechnoType();
			const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

            if(pExt->DamageSparks.Get(pType->DamageSparks)) {

                StackVector<ParticleSystemTypeClass*, 0x25> Systems {};

                if (auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems)) {
                    auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

                    for (auto pSystem : it) {
                        if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Spark) {
                            Systems->push_back(pSystem);
                        }
                    }
                }

                if(!Systems->empty()) {

                    const double _probability = _HPRatio >= RulesClass::Instance->ConditionRed ?
                        RulesClass::Instance->ConditionYellowSparkingProbability : RulesClass::Instance->ConditionRedSparkingProbability;
                    const auto _rand = ScenarioClass::Instance->Random.RandomDouble();

                    if (_rand < _probability ) {
                        CoordStruct _offs = pThis->Location + pType->GetParticleSysOffset();
                        pThis->SparkParticleSystem =
                        GameCreate<ParticleSystemClass>(Systems[ScenarioClass::Instance->Random.RandomFromMax(Systems->size() - 1)], _offs, nullptr, pThis);
                    }
                }
            }
        }
    }

   return 0x6FAF01;
}

ASMJIT_PATCH(0x70380A, TechnoClass_Cloak_CloakSound, 6)
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

ASMJIT_PATCH(0x70375B, TechnoClass_Uncloak_DecloakSound, 6)
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
ASMJIT_PATCH(0x732C30, TechnoClass_IDMatches, 5)
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
			if (what == BuildingClass::AbsID && (pType->UndeploysInto || RulesExtData::Instance()->BuildingTypeSelectable))
			{
				match = true;
				break;
			}
		}
	}

	R->EAX(match);
	return 0x732C97;
}

ASMJIT_PATCH(0x6F3950, TechnoClass_GetCrewCount, 8)
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
ASMJIT_PATCH(0x70E2B0, TechnoClass_IronCurtain, 5)
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

ASMJIT_PATCH(0x7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	R->EAX<int>(TechnoTypeExtData::HasSelectionGroupID(pThis->GetTechnoType(), pID));
	return 0x7327B2;
}

#include <CaptureManagerClass.h>

ASMJIT_PATCH(0x707B09, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EBP);
	GET_STACK(bool, remove, 0x28);

	// issues 1002020, 896263, 895954: clear stale mind control pointer to prevent
	// crashes when accessing properties of the destroyed controllers.
	if (pThis->MindControlledBy == ptr) {
		pThis->MindControlledBy = nullptr;
	}

	if(pThis->CaptureManager) {
		pThis->CaptureManager->DetachTarget(ptr);
	}

	// #912875: respect the remove flag for invalidating SpawnManager owners
	if(pThis->SpawnManager && (pThis->Owner != ptr || !(!remove && pThis->Owner == ptr))){
		pThis->SpawnManager->UnlinkPointer(ptr);
	}

	return 0x707B29;
}

void PlayEva(const char* pEva, CDTimerClass& nTimer, double nRate) {
	if (!nTimer.GetTimeLeft()) {
		nTimer.Start(GameOptionsClass::Instance->GetAnimSpeed(static_cast<int>(nRate * 900.0)));
		VoxClass::Play(pEva);
	}
}

ASMJIT_PATCH(0x70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

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

ASMJIT_PATCH(0x70CBB0, TechnoClass_DealParticleDamage_AmbientDamage, 6)
{
	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	if (!pWeapon->AmbientDamage)
		return 0x70CC3E;

	R->EDI(pWeapon);
	R->ESI(0);
	return (!(R->EAX<int>() <= 0)) ? 0x70CBB9 : 0x70CBF7;
}

// the fuck , game calling `MapClass[]` multiple times , fixed it
ASMJIT_PATCH(0x6FB5F0, TechnoClass_DeleteGap_Optimize, 6)
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

ASMJIT_PATCH(0x6FB306, TechnoClass_CreateGap_Optimize, 6)
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

ASMJIT_PATCH(0x6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	return !TechnoExt_ExtData::CloakDisallowed(pThis, false) ? 0x6FB7FD : 0x6FB75F;
}

ASMJIT_PATCH(0x6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	R->EAX(TechnoExt_ExtData::CloakDisallowed(pThis, true));
	return 0x6FBDBC;
}

ASMJIT_PATCH(0x6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	R->EAX(TechnoExt_ExtData::CloakAllowed(pThis));
	return 0x6FBF93;
}

ASMJIT_PATCH(0x6F6AC9, TechnoClass_Remove_Early, 6)
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

ASMJIT_PATCH(0x6F6F20, TechnoClass_Put_BuildingLight, 6)
{
	GET(TechnoClass*, pThis, ESI);

	//const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if(R->Origin() == 0x6F6F20)
		HugeBar::InitializeHugeBar(pThis);

	//only update the SW if really needed it
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsUnavailable();

	if (pTypeExt->HasSpotlight)
	{
		TechnoExt_ExtData::SetSpotlight(pThis, GameCreate<BuildingLightClass>(pThis));
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6F6D0E, TechnoClass_Put_BuildingLight, 7)


ASMJIT_PATCH(0x707D20, TechnoClass_GetCrew, 5)
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
ASMJIT_PATCH(0x70FBE0, TechnoClass_Activate_AresReplace, 6)
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

		if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
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
			if (auto const pBld = cast_to<BuildingClass*, false>(pThis))
			{
				TechnoExt_ExtData::UpdateFactoryQueues(pBld);
			}
		}
	}

	return 0x70FC85;
}


ASMJIT_PATCH(0x6FD438, TechnoClass_FireLaser, 6)
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

ASMJIT_PATCH(0x6f526c, TechnoClass_DrawExtras_PowerOff, 5)
{
	GET(TechnoClass*, pTechno, EBP);

	if(!pTechno->IsAlive)
		return 0x6F5347;

	GET_STACK(RectangleStruct*, pRect, 0xA0);

	if (auto pBld = cast_to<BuildingClass*, false>(pTechno))
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
				Point2D point = TacticalClass::Instance->CoordsToClient(crd);

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

ASMJIT_PATCH(0x70AA60, TechnoClass_DrawExtraInfo, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);
	//GET_STACK(Point2D*, pOriginalPoint, 0x8);
	//	GET_STACK(unsigned int , nFrame, 0x4);
	GET_STACK(RectangleStruct*, pRect, 0xC);

	if (!HouseClass::CurrentPlayer)
		return 0x70AD4C;

	if (auto pBuilding = cast_to<BuildingClass*, false>(pThis))
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
				auto nIntersect = RectangleStruct::Intersect(nTextDimension, *pRect, nullptr, nullptr);
				auto nColorInt = pOwner->Color.ToInit();//0x63DAD0

				DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
				DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
				Point2D nRet;
				Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pRect, &DrawLoca, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
				DrawLoca.Y += (nTextDimension.Height) + 2; //extra number for the background
			};

		if (pOwner->IsAlliedWith(HouseClass::CurrentPlayer)
			|| HouseClass::CurrentPlayer->IsObserver()
			|| pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer)
		) {

			if (pTypeExt->Fake_Of)
				DrawTheStuff(Phobos::UI::BuidingFakeLabel);

			if (pType->PowerBonus > 0 && BuildingTypeExtContainer::Instance.Find(pType)->ShowPower)
			{
				wchar_t pOutDrainFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				//foundating check ,...
				//can be optimized using stored bool instead checking them each frames
				if(pType->GetFoundationWidth() > 2 && pType->GetFoundationHeight(false) > 2) {
					swprintf_s(pOutDrainFormat, StringTable::FetchString(GameStrings::TXT_POWER_DRAIN2()), pOutput, pDrain);
				} else {
					swprintf_s(pOutDrainFormat, Phobos::UI::Power_Label, pOutput);
					DrawTheStuff(pOutDrainFormat);
					swprintf_s(pOutDrainFormat, Phobos::UI::Drain_Label, pDrain);
				}

				DrawTheStuff(pOutDrainFormat);
			}

			const bool hasStorage = pType->Storage > 0;
			bool HasSpySat = false;
			for(auto& _pType : pBuilding->GetTypes()) {
				if(_pType && _pType->SpySat) {
					HasSpySat = true;
					break;
				}
			}

			if (hasStorage) {

				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_1()), nMoney);
				DrawTheStuff(pOutMoneyFormat);

				if (BuildingTypeExtContainer::Instance.Find(pType)->Refinery_UseStorage) {
					wchar_t pOutStorageFormat[0x80];
					auto nStorage = pBuilding->GetStoragePercentage();
					swprintf_s(pOutStorageFormat, Phobos::UI::Storage_Label, nStorage);
					DrawTheStuff(pOutStorageFormat);
				}
			}

			if (pThis->IsPrimaryFactory)
			{
				if(SHPStruct* pImage = RulesExtData::Instance()->PrimaryFactoryIndicator) {
						ConvertClass* pPalette = FileSystem::PALETTE_PAL();
						if(RulesExtData::Instance()->PrimaryFactoryIndicator_Palette)
							pPalette =  RulesExtData::Instance()->PrimaryFactoryIndicator_Palette->GetOrDefaultConvert<PaletteManager::Mode::Default>(pPalette);

						int const cellsToAdjust = pType->GetFoundationHeight(false) - 1;
						Point2D pPosition = TacticalClass::Instance->CoordsToClient(pThis->GetCell()->GetCoords());
						pPosition.X -= Unsorted::CellWidthInPixels / 2 * cellsToAdjust;
						pPosition.Y += Unsorted::CellHeightInPixels / 2 * cellsToAdjust - 4;
						DSurface::Temp->DrawSHP(pPalette, pImage, 0, &pPosition, pRect, BlitterFlags(0x600), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				} else {
					DrawTheStuff(StringTable::FetchString((pType->GetFoundationWidth() != 1) ?
						GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
				}
			}

			if(pType->Radar || HasSpySat) {

				if(pType->Radar) {
					DrawTheStuff(Phobos::UI::Radar_Label);
				}

				if(HasSpySat) {
					DrawTheStuff(Phobos::UI::Spysat_Label);
				}

				if(!BuildingExtContainer::Instance.Find(pBuilding)->RegisteredJammers.empty())
					DrawTheStuff(Phobos::UI::BuidingRadarJammedLabel);

			}
		}
	}

	return 0x70AD4C;
}

// complete replacement
ASMJIT_PATCH(0x70FC90, TechnoClass_Deactivate_AresReplace, 6)
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

	if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
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
		if (auto const pBld = cast_to<BuildingClass*, false>(pThis))
		{
			TechnoExt_ExtData::UpdateFactoryQueues(pBld);
		}
	}

	return 0x70FD6E;
}

ASMJIT_PATCH(0x6FB1B5, TechnoClass_CreateGap_LargeGap, 7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	pThis->GapRadius = TechnoTypeExtContainer::Instance.Find(pType)->GapRadiusInCells;
	return R->Origin() + 0xD;
}ASMJIT_PATCH_AGAIN(0x6FB4A3, TechnoClass_CreateGap_LargeGap, 7)


// Radar Jammers (#305) unjam all on owner change
ASMJIT_PATCH(0x7014D5, TechnoClass_ChangeOwnership_Additional, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	//Debug::LogInfo("ChangeOwnershipFor [%s]" , pThis->get_ID());

	if (auto& pJammer = TechnoExtContainer::Instance.Find(pThis)->RadarJammer)
	{
		pJammer->UnjamAll();
	}

	if (auto pBuilding = cast_to<BuildingClass*, false>(pThis)) {

		const auto nTunnelVec = HouseExtData::GetTunnelVector(pBuilding->Type, pThis->Owner);

		if (!nTunnelVec || TunnelFuncs::FindSameTunnel(pBuilding))
			return 0x0;

		for (auto nPos = nTunnelVec->Vector.begin();
			nPos != nTunnelVec->Vector.end(); ++nPos) {
			TunnelFuncs::KillFootClass(*nPos, nullptr);
		}

		nTunnelVec->Vector.clear();
	}

	if (TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount != 0)
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, true);

	return 0;
}

ASMJIT_PATCH(0x702E64, TechnoClass_RegisterDestruction_Bounty, 6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);

	TechnoExt_ExtData::GiveBounty(pVictim, pKiller);

	return 0x0;
}

ASMJIT_PATCH(0x6FAF0D, TechnoClass_Update_EMPLock, 6)
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

ASMJIT_PATCH(0x6F3F88, TechnoClass_Init_1, 5)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	CaptureManagerClass* pCapturer = nullptr;
	ParasiteClass* pParasite = nullptr;
	TemporalClass* pTemporal = nullptr;
	SpawnManagerClass* pSpawnManager = nullptr;
	SlaveManagerClass* pSlaveManager = nullptr;
	AirstrikeClass* pAirstrike = nullptr;

	//AircraftDiveFunctional::Init(pExt, pTypeExt);

	if (pTypeExt->AttachtoType == AircraftTypeClass::AbsID) {
		if (pTypeExt->MyFighterData.Enable) {
			pExt->MyFighterData = std::make_unique<FighterAreaGuard>();
			pExt->MyFighterData->OwnerObject = (AircraftClass*)pThis;
		}
	}

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

	for (auto i = 0; i < WeaponCount; ++i) {

		if (auto const pWeapon = pType->GetWeapon(i)->WeaponType) {
			TechnoExt_ExtData::InitWeapon(pThis, pType, pWeapon, i, pCapturer, pParasite, pTemporal, "Weapon", IsFoot);
		}

		if (auto const pWeaponE = pType->GetEliteWeapon(i)->WeaponType) {
			TechnoExt_ExtData::InitWeapon(pThis, pType, pWeaponE, i, pCapturer, pParasite, pTemporal, "EliteWeapon", IsFoot);
		}
	}

	pThis->CaptureManager = pCapturer;
	pThis->TemporalImUsing = pTemporal;
	if (IsFoot) {
		((FootClass*)pThis)->ParasiteImUsing = pParasite;
	}

	pThis->SpawnManager = pSpawnManager;
	pThis->SlaveManager = pSlaveManager;
	pThis->Airstrike = pAirstrike;

	if (auto pOwner = pThis->Owner) {
		const auto pHouseType = pOwner->Type;
		const auto pParentHouseType = pHouseType->FindParentCountry();
		TechnoExtContainer::Instance.Find(pThis)->OriginalHouseType = pParentHouseType ? pParentHouseType : pHouseType;
	}
	else {
		Debug::LogInfo("Techno[{}] Init Without any ownership!", pType->ID);
	}

	// if override is in effect, do not create initial payload.
	// this object might have been deployed, undeployed, ...
	if (Unsorted::ScenarioInit && Unsorted::CurrentFrame) {
		TechnoExtContainer::Instance.Find(pThis)->PayloadCreated = true;
	}

	TechnoExtData::InitializeItems(pThis, pType);
	TechnoExtData::InitializeAttachEffects(pThis, pType);
	TechnoExtData::InitializeUnitIdleAction(pThis, pType);

	R->EAX(pType);
	return 0x6F4212;
}

// westwood does firingUnit->WhatAmI() == abs_AircraftType
// which naturally never works
// let's see what this change does
// ASMJIT_PATCH(0x6F7561, TechnoClass_Targeting_Arcing_Aircraft, 0x5)
// {
// 	GET(AbstractType, pTarget, EAX);
// 	GET(CoordStruct*, pCoord, ESI);
// 	R->EAX(pCoord->X);
// 	return pTarget == AbstractType::Aircraft ? 0x6F75B2 : 0x6F7568;
// }
DEFINE_PATCH(0x6F7563,0x2);

// No data found on .inj for this
//ASMJIT_PATCH(0x5F7933, TechnoTypeClass_FindFactory_ExcludeDisabled, 0x6)
//{
//	GET(BuildingClass*, pBld, ESI);
//
//	 //add the EMP check to the limbo check
//	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
//		0x5F7A57 : 0x5F7941;
//}

ASMJIT_PATCH(0x6F90F8, TechnoClass_SelectAutoTarget_Demacroize, 0x6)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x6F9116;
}

ASMJIT_PATCH(0x70133E, TechnoClass_GetWeaponRange_Demacroize, 0x5)
{
	GET(int, nVal1, EDI);
	GET(int, nVal2, EBX);

	R->EAX(nVal1 >= nVal2 ? nVal2 : nVal1);
	return 0x701388;
}

ASMJIT_PATCH(0x707EEA, TechnoClass_GetGuardRange_Demacroize, 0x6)
{
	GET(int, nVal1, EBX);
	GET(int, nVal2, EAX);

	R->EAX(nVal2 >= nVal1 ? nVal2 : nVal1);
	return 0x707F08;
}

// customizable berserk fire rate modification
// ASMJIT_PATCH(0x6FF28F, TechnoClass_Fire_BerserkROFMultiplier, 6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(int, ROF, EAX);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	if (pThis->Berzerk) {
// 		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
// 		const double multiplier = pExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
// 		ROF = static_cast<int>(ROF * multiplier);
// 	}
//
// 	TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);
//
// 	R->EAX(ROF);
// 	return 0x6FF2A4;
// }

ASMJIT_PATCH(0x6FE709, TechnoClass_Fire_BallisticScatter1, 6)
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

ASMJIT_PATCH(0x6FE7FE, TechnoClass_Fire_BallisticScatter2, 5)
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

ASMJIT_PATCH(0x707A47, TechnoClass_PointerGotInvalid_LastTarget, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);

	if (pThis->LastTarget == ptr)
		pThis->LastTarget = nullptr;

	return 0;
}

//TechnoClass_SetTarget_Burst
DEFINE_JUMP(LJMP, 0x6FCF53, 0x6FCF61);

ASMJIT_PATCH(0x717823, TechnoTypeClass_UpdatePalette_Reset, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->Palette = nullptr;

	return 0;
}ASMJIT_PATCH_AGAIN(0x717855, TechnoTypeClass_UpdatePalette_Reset, 0x6)


ASMJIT_PATCH(0x71136F, TechnoTypeClass_CTOR_Initialize, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	pThis->WeaponCount = 0; //default
	pThis->Bunkerable = false;
	pThis->Parasiteable = false;
	pThis->ImmuneToPoison = false;
	pThis->ConsideredAircraft = false;

	return 0;
}

ASMJIT_PATCH(0x7119D5, TechnoTypeClass_CTOR_NoInit_Particles, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI)

	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DamageParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();
	(*(uintptr_t*)((char*)pThis + offsetof(TechnoTypeClass, DestroyParticleSystems))) = ParticleSystemTypeClass::TypeListArray.getAddrs();

	return 0x711A00;
}

//TechnoClass_GetActionOnObject_IvanBombsB
DEFINE_JUMP(LJMP, 0x6FFF9E, 0x700006);

// ASMJIT_PATCH(0x6FF2D1, TechnoClass_FireAt_Facings, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
//
// 	int nIdx = 0;
//
// 	if (pWeapon->Anim.Count > 1) { //only execute if the anim count is more than 1
// 		const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);
//
// 		// 2^highest is the frame count, 3 means 8 frames
// 		if (highest >= 3) {
// 			nIdx = pThis->GetRealFacing().GetValue(highest, 1u << (highest - 3));
// 		}
// 	}
//
// 	R->EDI(pWeapon->Anim.GetItemOrDefault(nIdx , nullptr));
// 	return 0x6FF31B;
// }

ASMJIT_PATCH(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
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

ASMJIT_PATCH(0x6F826E, TechnoClass_CanAutoTargetObject_CivilianEnemy, 0x5)
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
	if (const auto pTargetTarget = flag_cast_to<TechnoClass*>(pTarget->Target)) {
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

ASMJIT_PATCH(0x7162B0, TechnoTypeClass_GetPipMax_MindControl, 0x6)
{
	GET(TechnoTypeClass* const, pThis, ECX);

	int count = 0;
	for (int i = 0; i < 3; ++i) {
		if (auto pWeapon = pThis->GetWeapon(i)->WeaponType) {
			if (pWeapon->Warhead->MindControl && pWeapon->Damage > 0) {
				count = pWeapon->Damage;
				break;
			}
		}
	}

	R->EAX(count);
	return 0x7162BC;
}

ASMJIT_PATCH(0x6FE31C, TechnoClass_Fire_AllowDamage, 8)
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
ASMJIT_PATCH(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
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

// ASMJIT_PATCH(0x70CBC3, TechnoClass_DealParticleDamage_FixArgs, 0x6)
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
ASMJIT_PATCH(0x6FA361, TechnoClass_Update_LoseTarget, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pHouse, EDI);

	enum { ForceAttack = 0x6FA472, ContinueCheck = 0x6FA39D };

	const bool BLRes = R->BL();
	const HouseClass* pOwner = !BLRes ? pThis->Owner : pHouse;

	bool IsAlly = false;

	if (const auto pTechTarget = flag_cast_to<ObjectClass*>(pThis->Target)) {
		if (const auto pTargetHouse = pTechTarget->GetOwningHouse()) {
			if (pOwner->IsAlliedWith(pTargetHouse)) {
				IsAlly = true;
			}
		}
	}

	auto pType = pThis->GetTechnoType();

	if (!pThis->Berzerk && pType->AttackFriendlies && IsAlly && TechnoTypeExtContainer::Instance.Find(pType)->AttackFriendlies_AutoAttack) {
		return ForceAttack;
	}

	//if(pThis->Berzerk && IsAlly) {
	//	return ForceAttack; // dont clear target
	//}

	const bool IsNegDamage = (pThis->CombatDamage() < 0);

	return IsAlly == IsNegDamage ? ForceAttack : ContinueCheck;
}