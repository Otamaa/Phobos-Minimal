#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Conversions.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include "Header.h"

#include <InfantryClass.h>

//DEFINE_HOOK(0x489280, Dmage_Area_Caller, 0x6)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("DamageArea Called %x.\n", caller);
//
//	return 0x0;
//}
//

DEFINE_HOOK(0x5F53DB, ObjectClass_ReceiveDamage_Handled, 0xA)
{
	enum
	{
		ContinueChecks = 0x5F5456,
		DecideResult = 0x5F5498,
		SkipDecideResult = 0x5F546A,
		ReturnResultNone = 0x5F545C,
	};

	GET(ObjectClass*, pObject, ESI);
	REF_STACK(args_ReceiveDamage, args, STACK_OFFSET(0x24, 0x4));

	const auto pType = pObject->GetType();
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);
	const bool bIgnoreDefenses = R->BL();

	OwnFunc::ApplyHitAnim(pObject, &args);

	pWHExt->applyRelativeDamage(pObject, &args);

	if (!bIgnoreDefenses)
	{
		MapClass::GetTotalDamage(&args, TechnoExtData::GetTechnoArmor(pObject, args.WH));
		//this already calculate distance damage from epicenter
		pWHExt->ApplyRecalculateDistanceDamage(pObject, &args);
	}

	if (*args.Damage == 0 && pObject->WhatAmI() == BuildingClass::AbsID)
	{
		auto const pBld = static_cast<BuildingClass*>(pObject);

		if (!pBld->Type->CanC4)
		{
			auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

			if (!pTypeExt->CanC4_AllowZeroDamage)
				*args.Damage = 1;
		}
	}

	if (!bIgnoreDefenses && args.Attacker && *args.Damage > 0)
	{
		if (pWHExt->applyCulling(args.Attacker, pObject))
			*args.Damage = pObject->Health;
	}

	const int MaxStr = pType->Strength;
	const int CurStr = R->Stack<int>(0x14);
	const int nDamage = *args.Damage;
	R->EBP(MaxStr);
	R->Stack(0x38, MaxStr);
	R->ECX(nDamage);

	//if (IS_SAME_STR_(pType->ID , "HARV"))
	//	Debug::Log("Harv [%s]DamageResult %d , HP %d/%d.\n", args.WH->ID,nDamage, CurStr, MaxStr);

	if (!nDamage)
		return ReturnResultNone;

	return nDamage > 0 ? DecideResult : SkipDecideResult;
}

DEFINE_HOOK(0x701A3B, TechnoClass_ReceiveDamage_Flash, 0xA)
{
	enum { NullifyDamage = 0x701A98, ContinueChecks = 0x701AAD };

	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(bool, forced, 0xD8);
	GET_STACK(bool, third, 0x13);
	GET(int*, pDamage, EBX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	//const auto pShield = pExt->GetShield();

	if (forced || third)
		return ContinueChecks;

	//if (pShield && pShield->IsActive() && pShield->GetType()->HitBright.isset())
	//{
	//	MapClass::FlashbangWarheadAt(2 * (*pDamage), pWh, pThis->Location, true, pShield->GetType()->HitBright);
	//} else
	 if (!WarheadTypeExtContainer::Instance.Find(pWh)->CanAffectInvulnerable(pThis))
	{
		if (pThis->ProtectType == ProtectTypes::ForceShield)
			MapClass::FlashbangWarheadAt(2 * (*pDamage), pWh, pThis->Location, true, SpotlightFlags::NoRed | SpotlightFlags::NoGreen);
		else if (WarheadTypeExtContainer::Instance.Find(pWh)->IC_Flash.Get(RulesExtData::Instance()->IC_Flash.Get()))
			MapClass::FlashbangWarheadAt(2 * (*pDamage), pWh, pThis->Location, true, SpotlightFlags::NoColor);

		return NullifyDamage; //nullify damage
	}

	return ContinueChecks;
}

DEFINE_HOOK(0x489968, Explosion_Damage_PenetratesIronCurtain, 0x5)
{
	enum { BypassInvulnerability = 0x48996D };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	if (WarheadTypeExtContainer::Instance.Find(pWarhead)->PenetratesIronCurtain)
		return BypassInvulnerability;

	return 0;
}

DEFINE_HOOK(0x702117,  TechnoClass_ReceiveDamage_OverrideSounds , 0xA){
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto pType = pThis->GetTechnoType();

	if(pType->VoiceDie.Count > 0 && pThis->Owner->ControlledByCurrentPlayer()){
		auto const& nSound = WarheadTypeExtContainer::Instance.Find(pWh)->DieSound_Override;

		if(nSound.isset()) {
			VocClass::PlayIndexAtPos(nSound, pThis->Location);
		} else {
			int rand = Random2Class::NonCriticalRandomNumber->Random();
			VocClass::PlayIndexAtPos(pType->VoiceDie[rand % pType->VoiceDie.Count], pThis->Location);
		}
	}

	if(pType->DieSound.Count > 0){
		auto const& nSound = WarheadTypeExtContainer::Instance.Find(pWh)->VoiceSound_Override;

		if(nSound.isset()) {
			VocClass::PlayIndexAtPos(nSound, pThis->Location);
		} else {
			int rand = Random2Class::NonCriticalRandomNumber->Random();
			VocClass::PlayIndexAtPos(pType->DieSound[rand % pType->DieSound.Count], pThis->Location);
		}
	}

	return 0x702200;
}

DEFINE_HOOK_AGAIN(0x702BFE , TechnoClass_ReceiveDamage_ScatterCheck,0x8)
DEFINE_HOOK(0x702B67, TechnoClass_ReceiveDamage_ScatterCheck, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	if(WarheadTypeExtContainer::Instance.Find(pWarhead)->PreventScatter)
		return 0x702D11;

	if (pThis->AbstractFlags & AbstractFlags::Foot) {

		auto pFoot = reinterpret_cast<FootClass*>(pThis);

		if (!pFoot->Target && !pFoot->Destination) {

			if (R->Origin() == 0x702B67 && (RulesClass::Instance->PlayerScatter || pFoot->HasAbility(AbilityType::Scatter))) {
				pFoot->Scatter(CoordStruct::Empty, true, false);
			}
			else if (!pFoot->IsTethered && pFoot->WhatAmI() != AircraftClass::AbsID && pThis->GetCurrentMissionControl()->Scatter) {
				const bool IsMoving = pFoot->Locomotor.GetInterfacePtr()->Is_Moving();

				if (!IsMoving) {
					const bool IsHuman = pFoot->Owner->IsControlledByHuman();
					const bool IsScatter = RulesClass::Instance->PlayerScatter;
					const bool IscatterAbility = pFoot->HasAbility(AbilityType::Scatter);
					if (!IsHuman || IsScatter || IscatterAbility) {
						pFoot->Scatter(CoordStruct::Empty, true, false);
					}
				}

			}

		}
	}

	return 0x702D11;
}

// #1283653: fix for jammed buildings and attackers in open topped transports
DEFINE_HOOK(0x702A38, TechnoClass_ReceiveDamage_OpenTopped, 0x7)
{
	REF_STACK(TechnoClass*, pAttacker, STACK_OFFS(0xC4, -0x10));

	// decide as if the transporter fired at this building
	if (pAttacker && pAttacker->InOpenToppedTransport && pAttacker->Transporter)
	{
		pAttacker = pAttacker->Transporter;
	}

	R->EDI(pAttacker);
	return 0x702A3F;
}

DEFINE_HOOK(0x702669, TechnoClass_ReceiveDamage_SuppressDeathWeapon, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (!WarheadTypeExtContainer::Instance.Find(pWarhead)->ApplySuppressDeathWeapon(pThis))
	{
		pThis->FireDeathWeapon(0);
	}

	return 0x702672;
}

DEFINE_HOOK(0x517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 0x6)
{

	GET(InfantryClass*, I, ESI);
	const bool IgnoreDefenses = R->BL() != 0;

	if (!I->IsDeployed() || IgnoreDefenses)
	{
		return 0;
	}

	GET(WarheadTypeClass*, pWH, EBP);
	GET(int*, pDamage, EDI);

	// yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
	if (pWH)
	{
		const auto nMult = WarheadTypeExtContainer::Instance.Find(pWH)->DeployedDamage.Get(I);
		*pDamage = static_cast<int>(*pDamage * nMult);
		return 0x517FF9u;
	}

	return 0x518016u;
}

#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

DEFINE_HOOK(0x702050, TechnoClass_ReceiveDamage_ResultDestroyed, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);

	const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	auto pTechExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto coords = pThis->GetCoords();
	auto const pOwner = pThis->Owner;

	if (pWarheadExt->Supress_LostEva.Get())
		pTechExt->SupressEVALost = true;

	GiftBoxFunctional::Destroy(pTechExt, pTypeExt);

	if(pThis->IsAlive) {
		std::set<PhobosAttachEffectTypeClass*> cumulativeTypes {};
		std::vector<WeaponTypeClass*> expireWeapons {};
		PhobosAEFunctions::ApplyExpireWeapon(expireWeapons, cumulativeTypes, pThis);

		for (auto const& pWeapon : expireWeapons) {
			TechnoClass* pTarget = pThis;
			if(!pThis->IsAlive)
				pTarget = nullptr;

			WeaponTypeExtData::DetonateAt(pWeapon, coords, pTarget, false, pOwner);
		}
	}

	return 0x0;
}

//bool IsDamaging;
DEFINE_HOOK(0x701914, TechnoClass_ReceiveDamage_Damaging, 0x7)
{
	R->Stack(0xE, R->EAX() > 0);
	//IsDamaging = R->EAX() > 0;
	return 0;
}

//DEFINE_HOOK(0x4D7330, FootClass_ReceiveDamage_probe, 0x8) {
//	GET(FootClass*, pThis, ECX);
//
//	auto id = pThis->get_ID();
//	if (IS_SAME_STR_("MDUMMY7", id))
//		Debug::Log(__FUNCTION__" Executed\n");
//
//	return 0;
//}

//DEFINE_HOOK(0x737C90 , UnitClass_TakeDamage_probe ,0x5)
//{
//	GET(UnitClass*, pThis, ECX);
//	REF_STACK(args_ReceiveDamage, args, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (IS_SAME_STR_(pThis->Type->ID, "MDUMMY7"))
//		Debug::Log("%s [%s]DamageResult %d , HP %d/%d Called %x.\n", pThis->Type->ID, args.WH->ID, *args.Damage, pThis->Health, pThis->Type->Strength , caller);
//
//	return 0x0;
//}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Aftermath, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, 0xD0);
	GET_STACK(TechnoClass* const, pAttacker, 0xD4);
	GET_STACK(DamageState const, nDamageResult, 0x20);
	GET_STACK(int* const, pDamamge, 0xC8);
	GET_STACK(bool const, bIgnoreDamage, 0xD8);
	GET_STACK(HouseClass* const, pAttacker_House, STACK_OFFSET(0xC4, 0x18));
	GET_STACK(bool, IsDamaging, 0x12);

	bool bAffected = false;
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const bool IsAffected = nDamageResult != DamageState::Unaffected;

	if (IsAffected || bIgnoreDamage || !IsDamaging || *pDamamge)
	{
		if (IsAffected && IsDamaging)
		{
			const auto rank = pThis->Veterancy.GetRemainingLevel();
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			const auto pWHExt = WarheadTypeExtContainer::Instance.TryFind(pWarhead);
			const auto fromTechno = pTypeExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank);

			const int amount = pWHExt ? pWHExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank)
				->Get(fromTechno) : fromTechno;

			//the timer will always restart
			//not accumulated
			if (amount > 0)
			{
				pExt->SelfHealing_CombatDelay.Start(amount);
			}
		}

	}
	else { bAffected = true; }

	if (pWarhead)
	{
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		const auto pHouse = pAttacker ? pAttacker->Owner : pAttacker_House;

		if (IsAffected && pWHExt->DecloakDamagedTargets.Get())
			pThis->Reveal();

		const auto bCond1 = (!bAffected || !pWHExt->EffectsRequireDamage);
		const auto bCond2 = (!pWHExt->EffectsRequireVerses || (pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(pThis, pWarhead)).Verses >= 0.0001));

		if (bCond1 && bCond2)
		{
			AresWPWHExt::applyKillDriver(pWarhead, pAttacker, pThis);

			if (pWHExt->Sonar_Duration > 0)
			{
				auto& nSonarTime = TechnoExtContainer::Instance.Find(pThis)->CloakSkipTimer;
				if (pWHExt->Sonar_Duration > nSonarTime.GetTimeLeft())
				{
					nSonarTime.Start(pWHExt->Sonar_Duration);

					if (pThis->CloakState != CloakState::Uncloaked)
					{
						pThis->Uncloak(true);
						pThis->NeedsRedraw = true;
					}
				}
			}

			if (pWHExt->DisableWeapons_Duration > 0)
			{
				auto& nTimer = TechnoExtContainer::Instance.Find(pThis)->DisableWeaponTimer;
				if (pWHExt->DisableWeapons_Duration > nTimer.GetTimeLeft())
				{
					nTimer.Start(pWHExt->DisableWeapons_Duration);
				}
			}

			if (pWHExt->Flash_Duration > 0
				&& pWHExt->Flash_Duration > pThis->Flashing.DurationRemaining)
			{
				pThis->Flash(pWHExt->Flash_Duration);
			}

			if (pWHExt->RemoveDisguise)
			{
				pWHExt->ApplyRemoveDisguise(pHouse, pThis);
			}

			if (pWHExt->RemoveMindControl)
			{
				pWHExt->ApplyRemoveMindControl(pHouse, pThis);
			}
		}
	}

	return 0x702823;
}

DEFINE_HOOK(0x701BFE, TechnoClass_ReceiveDamage_Abilities, 0x6)
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
	GET_STACK(TechnoClass*, pAttacker, 0xD4);
	GET_STACK(HouseClass*, pAttacker_House, 0xE0);
	GET(int*, pDamage, EBX);

	const auto nRank = pThis->Veterancy.GetRemainingLevel();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	if (pWHExt->ImmunityType.isset() && TechnoExtData::HasImmunity(nRank, pThis, pWHExt->ImmunityType))
		return RetNullify;

	if (pWH->Radiation && TechnoExtData::IsRadImmune(nRank, pThis))
		return RetNullify;

	if (pWH->PsychicDamage && TechnoExtData::IsPsionicsWeaponImmune(nRank, pThis))
		return RetNullify;

	if (pWH->Poison && TechnoExtData::IsPoisonImmune(nRank, pThis))
		return RetNullify;

	const auto pSourceHouse = pAttacker ? pAttacker->Owner : pAttacker_House;

	if (!pWHExt->CanAffectHouse(pThis->Owner, pSourceHouse))
		return RetNullifyB;

	if (pWH->Psychedelic)
	{
		if (pThis->WhatAmI() == BuildingClass::AbsID)
			return RetUnaffected;

		//This thing does ally check twice
		if (pSourceHouse && pSourceHouse->IsAlliedWith(pThis))
			return RetUnaffected;

		if (TechnoExtData::IsPsionicsImmune(nRank, pThis) || TechnoExtData::IsBerserkImmune(nRank, pThis))
			return RetUnaffected;

		// there is no building involved
		// More customizeable berzerk appying - Otamaa
		// return boolean to decide receive damage after apply berzerk or just retun function result
		//  appylying  damage will cause  the tehno to retaliate !
		if (!pWHExt->GoBerzerkFor(static_cast<FootClass*>(pThis), pDamage))
			return RetResultLight;
	}
	else
	{
		// restoring TS berzerk cyborg
		//this will happen regardless the immunity i guess
		if (auto pInf = specific_cast<InfantryClass*>(pThis))
		{
			if (RulesClass::Instance->BerzerkAllowed && pInf->Type->Cyborg && pThis->IsYellowHP())
			{
				if (!pInf->Berzerk)
				{
					pInf->Berzerk = true;
					pInf->GoBerzerkFor(10);
				}
			}
		}
	}

	PhobosAEFunctions::ApplyReflectDamage(pThis, pDamage, pAttacker, pAttacker_House, pWH);

	return RetObjectClassRcvDamage;
}

//DEFINE_HOOK(0x4D7431, FootClass_TakeDamage_ProbeResult, 0x5)
//{
//	GET(DamageState, result, EAX);
//	GET(WarheadTypeClass*, pWH, EBP);
//	GET(FootClass*, pThis, ESI);
//
//	if (IS_SAME_STR_("EradiationWH", pWH->ID) && IS_SAME_STR_("PENTGENX", pThis->get_ID())) {
//		Debug::Log("Affected [%d] by[%s]\n", (int)result, pWH->ID);
//	}
//
//	return 0x0;
//}

DEFINE_HOOK(0x737F97, UnitClass_ReceiveDamage_Survivours, 0xA)
{
	//GET(UnitTypeClass*, pType, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFSET(0x44, 0x18));

	if (pThis && pThis->Passengers.NumPassengers > 0)
	{
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
		{
			auto pPassenger = pThis->Passengers.GetFirstPassenger();
			auto pExt = TechnoExtContainer::Instance.Find(pPassenger);

			if (pExt->OriginalPassengerOwner)
				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);

			while (pPassenger->NextObject)
			{
				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
				pExt = TechnoExtContainer::Instance.Find(pPassenger);

				if (pExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
			}
		}
	}

	TechnoExt_ExtData::SpawnSurvivors(pThis, pKiller, select, ignoreDefenses, preventPassangersEscape);

	R->EBX(-1);
	return 0x73838A;
}

DEFINE_HOOK(0x41660C, AircraftClass_ReceiveDamage_destroyed, 0x5)
{
	GET(AircraftClass*, pThis, ESI);
	REF_STACK(const args_ReceiveDamage, args, 0x14 + 0x4);

	pThis->Destroyed(args.Attacker);

	if (pThis->Type->Explosion.Count > 0)
	{
		if (auto pExp = pThis->Type->Explosion
			[ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1)])
		{
			auto nCoord = pThis->GetTargetCoords();
			// if (pInvoker && !Is_House(pInvoker))
			// 	pInvoker = nullptr;

			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExp, nCoord),
				args.Attacker ? args.Attacker->Owner : (args.SourceHouse ? args.SourceHouse : nullptr),
				pThis->Owner,
				true
			);
		}
	}

	// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
	const bool bSelected = pThis->IsSelected && pThis->Owner->ControlledByCurrentPlayer();
	TechnoExt_ExtData::SpawnSurvivors(pThis,
		args.Attacker,
		bSelected,
		args.IgnoreDefenses,
		args.PreventsPassengerEscape);

	const auto& crashable = TechnoTypeExtContainer::Instance.Find(pThis->Type)->Crashable;
	if ((crashable.isset() && !crashable.Get()) || !pThis->Crash(args.Attacker))
		pThis->UnInit();

	return 0x4166A9;
}

// spawn tiberium when a unit dies. this is a minor part of the
// tiberium heal feature. the actual healing happens in FootClass_Update.
DEFINE_HOOK(0x702216, TechnoClass_ReceiveDamage_TiberiumHeal_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (TechnoTypeExtContainer::Instance.Find(pType)->TiberiumRemains
		.Get(pType->TiberiumHeal && RulesExtData::Instance()->Tiberium_HealEnabled))
	{
		int nIdx = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetHighestStorageIdx();
		const CellClass* pCenter = MapClass::Instance->GetCellAt(pThis->Location);

		// increase the tiberium for the four neighbours and center.
		// center is retrieved by getting a neighbour cell index >= 8
		for (int i = 0; i < 8; i += 2)
		{
			pCenter->GetNeighbourCell((FacingType)i)
				->IncreaseTiberium(nIdx, ScenarioClass::Instance->Random.RandomFromMax(2));
		}
	}

	return 0;
}

// smoke particle systems created when a techno is damaged
DEFINE_HOOK(0x702894, TechnoClass_ReceiveDamage_SmokeParticles, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x30);

	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	const auto it = pExt->ParticleSystems_DamageSmoke.GetElements(pType->DamageParticleSystems);
	const auto allowAny = pExt->ParticleSystems_DamageSmoke.HasValue();

	for (const auto pSystem : it)
	{
		if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
		{
			Systems.AddItem(pSystem);
		}
	}

	return 0x702938;
}

// spill the stored tiberium on destruction
DEFINE_HOOK(0x702200, TechnoClass_ReceiveDamage_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (TechnoTypeExtContainer::Instance.Find(pType)->TiberiumSpill)
	{
		const auto pUnit = specific_cast<UnitClass*>(pThis);
		const auto pBld = specific_cast<BuildingClass*>(pThis);

		if (pUnit && pUnit->Type->Weeder || pBld && pBld->Type->Weeder)
			return 0x0;

		auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

		double stored = storage->GetAmounts();

		if (!pBld
			&& stored > 0.0
			&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune)
		{
			// don't spill more than we can hold
			double max = 9.0;
			if (max > pType->Storage)
			{
				max = pType->Storage;
			}

			const int nIdx = storage->GetHighestStorageIdx();

			// assume about half full, recalc if possible
			int value = static_cast<int>(max / 2);
			if (pType->Storage > 0)
			{
				value = int(stored / pType->Storage * max);
			}

			// get the spill center
			TechnoClass::SpillTiberium(value, nIdx, MapClass::Instance->GetCellAt(pThis->GetCoords()), { 0,2 });
		}
	}

	return 0;
}

// #895584: ships not taking damage when repaired in a shipyard. bug
// was that the logic that prevented units from being damaged when
// exiting a war factory applied here, too. added the Naval check.
DEFINE_HOOK(0x737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
{
	GET(BuildingTypeClass*, pType, ECX);
	return (pType->WeaponsFactory && !pType->Naval)
		? 0x737CEE : 0x737D31;
}

DEFINE_HOOK(0x51849A, InfantryClass_ReceiveDamage_DeathAnim, 5)
{
	GET(InfantryClass*, I, ESI);
	LEA_STACK(args_ReceiveDamage*, Arguments, 0xD4);
	GET(DWORD, InfDeath, EDI);

	// if you got here, a valid DeathAnim for this InfDeath has been defined, and the game has already checked the preconditions
	// just allocate the anim and set its owner/remap

	AnimClass* Anim = GameCreate<AnimClass>(I->Type->DeathAnims[InfDeath], I->Location);

	HouseClass* Invoker = (Arguments->Attacker)
		? Arguments->Attacker->Owner
		: Arguments->SourceHouse
		;

	AnimExtData::SetAnimOwnerHouseKind(Anim, Invoker, I->Owner, Arguments->Attacker, false, true);

	R->EAX<AnimClass*>(Anim);
	return 0x5184F2;
}

DEFINE_HOOK_AGAIN(0x518575, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
DEFINE_HOOK(0x5183DE, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, true);

	// bonus: don't require SpawnsParticle to be present

	if (ParticleSystemClass::Array->ValidIndex(pAnim->Type->SpawnsParticle))
	{
		return 0;
	}

	return (R->Origin() == 0x5183DE) ? 0x518422 : 0x5185B9;
}

DEFINE_HOOK_AGAIN(0x518B93, InfantryClass_ReceiveDamage_Anims, 5) // InfantryBrute
DEFINE_HOOK_AGAIN(0x518821, InfantryClass_ReceiveDamage_Anims, 5) // InfantryNuked
DEFINE_HOOK_AGAIN(0x5187BB, InfantryClass_ReceiveDamage_Anims, 5) // InfantryHeadPop
DEFINE_HOOK_AGAIN(0x518755, InfantryClass_ReceiveDamage_Anims, 5) // InfantryElectrocuted
DEFINE_HOOK_AGAIN(0x5186F2, InfantryClass_ReceiveDamage_Anims, 5) // FlamingInfantry
DEFINE_HOOK(0x518698, InfantryClass_ReceiveDamage_Anims, 5) // InfantryExplode
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// animation has been created. set the owner and color.
	const auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);

	return 0x5185F1;
}

DEFINE_HOOK(0x51887B, InfantryClass_ReceiveDamage_InfantryVirus2, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner, but
	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	const auto& [bChanged, result] =
		AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);

	// reset the color for default (invoker).
	if (bChanged && result != OwnerHouseKind::Default)
	{
		pAnim->LightConvert = nullptr;
	}

	return 0x5185F1;
}

DEFINE_HOOK(0x518A96, InfantryClass_ReceiveDamage_InfantryMutate, 7)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryMutate animation has been created. set the owner and color.
	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);

	return 0x518AFF;
}

DEFINE_HOOK(0x518CB3, InfantryClass_ReceiveDamage_Doggie, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	// hurt doggie gets more panic
	if (pThis->Type->Doggie && pThis->IsRedHP()) {
		R->EDI(RulesExtData::Instance()->DoggiePanicMax);
	}

	return 0;
}

DEFINE_HOOK(0x5F57B5, ObjectClass_ReceiveDamage_Trigger, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	GET(ObjectClass*, pAttacker, EDI);
	GET(DamageState, state, EBP);
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFSET(0x24, 0xC));

	if (state != DamageState::NowDead && !WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative)
	{
		if (pObject->IsAlive)
		{
			if (auto pFirstTag = pObject->AttachedTag)
			{
				pFirstTag->RaiseEvent(
					TriggerEvent::AttackedByAnybody,
					pObject,
					CellStruct::Empty,
					false,
					pAttacker
				);
			}
		}

		if (pObject->IsAlive)
		{
			if (auto pSecondTag = pObject->AttachedTag)
			{
				pSecondTag->RaiseEvent(
					TriggerEvent::AttackedByHouse,
					pObject,
					CellStruct::Empty,
					false,
					pAttacker
				);
			}
		}
	}

	if (pObject->IsAlive)
	{
		if (auto pFirstTag = pObject->AttachedTag)
		{
			pFirstTag->RaiseEvent(
			(TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByHouse,
			pObject,
			CellStruct::Empty,
			false,
			pAttacker
			);
		}
	}

	if (pObject->IsAlive)
	{
		if (auto pSecondTag = pObject->AttachedTag)
		{
			pSecondTag->RaiseEvent(
				(TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByAnybody,
				pObject,
				CellStruct::Empty,
				false,
				pAttacker
			);

		}
	}

	return 0x5F580C;
}

DEFINE_HOOK(0x744745, UnitClass_RegisterDestruction_Trigger, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x702DD6, TechnoClass_RegisterDestruction_Trigger, 0x6)
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

DEFINE_HOOK(0x7032B0, TechnoClass_RegisterLoss_Trigger, 0x6)
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

// Do not consider Whiner=true team members hit by weapon as having been attacked e.g provoking response from AI.
DEFINE_HOOK(0x4D7493, FootClass_ReceiveDamage_Nonprovocative, 0x5)
{
	enum { SkipChecks = 0x4D74CD, SkipEvents = 0x4D74A3 };

	GET(TechnoClass*, pSource, EBX);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x1C, 0xC));

	if (!pSource)
		return SkipChecks;

	auto const pTypeExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Do not consider ToProtect technos hit by weapon as having been attacked e.g provoking response from AI.
DEFINE_HOOK(0x7027E6, TechnoClass_ReceiveDamage_Nonprovocative, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EAX);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	auto const pTypeExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	if (!pTypeExt->Nonprovocative)
	{
		pThis->BaseIsAttacked(pSource);
	}

	return 0x7027EE;
}

//DEFINE_HOOK(0x489A01, MapClass_DamageArea_LoopDamageGroups, 0x6)
//{
//	enum { AdvanceLoop = 0x489AC1 , SetStack1FTrue = 0x489ABC , Continue = 0x0};
//
//	GET(ObjectClass*, pTarget, ESI);
//	GET(int , nDistance , EDI);
//	GET_BASE(TechnoClass*, pSource, 0x8);
//	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
//	GET_BASE(HouseClass*, pHouse, 0x14);
//	GET_STACK(bool , IsIroncurtained , 0x17);
//	GET_STACK(int , cellSpread , 0x68);
//	GET_STACK(int , damage , 0x24);
//
//
//	char dummy[0x70];
//	if(pSource) {
//		IMPL_SNPRNINTF(dummy,sizeof(dummy),"%x - %s" , (size_t)pSource , pSource->get_ID());
//	}else{
//		std::strcpy(dummy,"Unknown");
//	}
//
//	if(!pTarget->IsAlive)
//		return AdvanceLoop;
//
//	const auto pWhat = pTarget->WhatAmI();
//
//	if(pWhat == BulletClass::AbsID)
//		Debug::Log("Bullet[%x - %s] Getting hit by [%s] Warhead [%s] !\n" , pTarget,pTarget->get_ID() ,dummy , pWarhead->ID);
//
//	{
//
//		if(pWhat == BuildingClass::AbsID && static_cast<BuildingClass*>(pTarget)->Type->InvisibleInGame)
//			return AdvanceLoop;
//
//		if(!IsIroncurtained || pTarget->IsIronCurtained())
//		{
//			if(pWhat == AircraftClass::AbsID && pTarget->IsInAir())
//				nDistance /= 2;
//
//			if(pTarget->Health > 0 && pTarget->IsOnMap && !pTarget->InLimbo && nDistance <= cellSpread)
//			{
//				pTarget->ReceiveDamage(&damage ,nDistance ,pWarhead ,pSource , false ,false ,pHouse);
//				return SetStack1FTrue;
//			}
//		}
//	}
//
//	return AdvanceLoop;
//	//return Continue;
//}