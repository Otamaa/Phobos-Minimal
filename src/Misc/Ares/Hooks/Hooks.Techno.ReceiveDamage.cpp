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

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Misc/AresData.h>

#include <Conversions.h>
#include <New/Type/ArmorTypeClass.h>

void ApplyHitAnim(ObjectClass* pTarget, args_ReceiveDamage* args)
{
	if (Unsorted::CurrentFrame % 15)
		return;

	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(args->WH);
	auto const pTechno = generic_cast<TechnoClass*>(pTarget);
	auto const pType = pTarget->GetType();
	auto const bIgnoreDefense = args->IgnoreDefenses;
	bool bImmune_pt2 = false;
	bool const bImmune_pt1 =
		(pTarget->IsIronCurtained() && !bIgnoreDefense) ||
		(pType->Immune && !bIgnoreDefense) || pTarget->InLimbo
		;

	if (pTechno)
	{
		const auto pShield = TechnoExt::ExtMap.Find(pTechno)->GetShield();
		bImmune_pt2 = (pShield && pShield->IsActive())
			|| pTechno->TemporalTargetingMe
			|| (pTechno->ForceShielded && !bIgnoreDefense)
			|| pTechno->BeingWarpedOut
			|| pTechno->IsSinking
			;

	}

	if (!bImmune_pt1 && !bImmune_pt2)
	{
		const int nArmor = (int)pType->Armor;

#ifdef COMPILE_PORTED_DP_FEATURES_
		TechnoClass_ReceiveDamage2_DamageText(pTechno, pDamage, pWarheadExt->DamageTextPerArmor[(int)nArmor]);
#endif

		if (const auto pAnimTypeDecided = pWarheadExt->GetArmorHitAnim(nArmor))
		{
			CoordStruct nBuffer { 0, 0 , 0 };

			if (pTechno)
			{
				auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

				if (!pTechnoTypeExt->HitCoordOffset.empty())
				{
					if ((pTechnoTypeExt->HitCoordOffset.size() > 1) && pTechnoTypeExt->HitCoordOffset_Random.Get())
						nBuffer = pTechnoTypeExt->HitCoordOffset[ScenarioClass::Instance->Random.RandomFromMax(pTechnoTypeExt->HitCoordOffset.size() - 1)];
					else
						nBuffer = pTechnoTypeExt->HitCoordOffset[0];
				}
			}

			auto const nCoord = pTarget->GetCenterCoords() + nBuffer;
			if (auto pAnimPlayed = GameCreate<AnimClass>(pAnimTypeDecided, nCoord))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnimPlayed, args->Attacker ? args->Attacker->GetOwningHouse() : args->SourceHouse, pTarget->GetOwningHouse(), args->Attacker, false);
			}
		}
	}
}

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

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
	const bool bIgnoreDefenses = R->BL();

	ApplyHitAnim(pObject, &args);

	pWHExt->ApplyRelativeDamage(pObject, &args);

	if (!bIgnoreDefenses)
	{
		MapClass::GetTotalDamage(&args, pObject->GetType()->Armor);
		//this already calculate distance damage from epicenter
		pWHExt->ApplyRecalculateDistanceDamage(pObject, &args);
	}

	if (*args.Damage == 0 && Is_Building(pObject))
	{
		auto const pBld = static_cast<BuildingClass*>(pObject);

		if (!pBld->Type->CanC4)
		{
			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (!pTypeExt->CanC4_AllowZeroDamage)
				*args.Damage = 1;
		}
	}

	if (!bIgnoreDefenses && args.Attacker && *args.Damage > 0)
	{
		if (pWHExt->ApplyCulling(args.Attacker, pObject))
			*args.Damage = pObject->Health;
	}

	const int pTypeStr = pObject->GetType()->Strength;
	const int nDamage = *args.Damage;
	R->EBP(pTypeStr);
	R->Stack(0x38, pTypeStr);
	R->ECX(nDamage);

	if (!nDamage)
		return ReturnResultNone;

	return nDamage > 0 ? DecideResult : SkipDecideResult;
}

DEFINE_OVERRIDE_HOOK(0x701A5C, TechnoClass_ReceiveDamage_IronCurtainFlash, 0x7)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	if (!WarheadTypeExt::ExtMap.Find(pWh)->IC_Flash.Get(RulesExt::Global()->IC_Flash.Get()))
		return 0x701A98;

	return (pThis->ForceShielded == 1) ? 0x701A65 : 0x701A69;
}

bool IsDamaging;
DEFINE_OVERRIDE_HOOK(0x701914, TechnoClass_ReceiveDamage_Damaging, 0x7)
{
	//R->Stack(0xE, R->EAX() > 0);
	IsDamaging = R->EAX() > 0;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7021F5, TechnoClass_ReceiveDamage_OverrideDieSound, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->DieSound_Override;

	if (nSound.isset())
	{
		VocClass::PlayIndexAtPos(nSound, pThis->Location);
		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702185, TechnoClass_ReceiveDamage_OverrideVoiceDie, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->VoiceSound_Override;

	if (nSound.isset())
	{
		VocClass::PlayIndexAtPos(nSound, pThis->Location);
		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702CFE, TechnoClass_ReceiveDamage_PreventScatter, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	const auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	// only allow to scatter if not prevented
	if (!pExt->PreventScatter)
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
	}

	return 0x702D11;
}

// #1283653: fix for jammed buildings and attackers in open topped transports
DEFINE_OVERRIDE_HOOK(0x702A38, TechnoClass_ReceiveDamage_OpenTopped, 0x7)
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

DEFINE_OVERRIDE_HOOK(0x702669, TechnoClass_ReceiveDamage_SuppressDeathWeapon, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (!WarheadTypeExt::ExtMap.Find(pWarhead)->ApplySuppressDeathWeapon(pThis))
	{
		pThis->FireDeathWeapon(0);
	}

	return 0x702672;
}

DEFINE_OVERRIDE_HOOK(0x517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 0x6)
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
		*pDamage = static_cast<int>(*pDamage * WarheadTypeExt::ExtMap.Find(pWH)->DeployedDamage);
		return 0x517FF9u;
	}

	return 0x518016u;
}

DEFINE_OVERRIDE_HOOK(0x702050, TechnoClass_ReceiveDamage_SuppressUnitLost, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);

	const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	auto pTechExt = TechnoExt::ExtMap.Find(pThis);

	if (pWarheadExt->Supress_LostEva.Get())
		pTechExt->SupressEVALost = true;

	return 0x0;
}

// not this
/*
 * Fixing issue #722
 */
DEFINE_OVERRIDE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, WH, STACK_OFFS(0x44, -0xC));
	return WH && !Is_MaliciousWH(WH) ? 0x738535u : 0u;
}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Aftermath, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, 0xD0);
	GET_STACK(TechnoClass* const, pAttacker, 0xD4);
	GET_STACK(DamageState, nDamageResult, 0x20);
	GET_STACK(int* const, pDamamge, 0xC8);
	GET_STACK(bool, bIgnoreDamage, 0xD8);
	GET_STACK(HouseClass*, pAttacker_House, STACK_OFFSET(0xC4, 0x18));
	//GET_STACK(void*, bSomething, 0x12); //

	const bool bResult = nDamageResult == DamageState::Unaffected ? 0x702823 : 0x0;
	bool bAffected = false;
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if ((int)nDamageResult || bIgnoreDamage || !IsDamaging || *pDamamge)
	{
		if ((int)nDamageResult && IsDamaging)
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			if (pTypeExt->SelfHealing_CombatDelay > 0)
			{
				GetSelfHealingCombatTimer(pThis).Start(pTypeExt->SelfHealing_CombatDelay);
			}
		}
	}
	else
	{
		bAffected = true;
	}

	if (pWarhead)
	{
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		const auto pHouse = pAttacker ? pAttacker->Owner : pAttacker_House;

		if (pWHExt->DecloakDamagedTargets.Get())
			pThis->Uncloak(false);

		if ((!bAffected || !pWHExt->EffectsRequireDamage) &&
			(!pWHExt->EffectsRequireVerses || (pWHExt->GetVerses(pType->Armor).Verses >= 0.0001)))
		{

			AresData::WarheadTypeExt_ExtData_ApplyKillDriver(pWarhead, pAttacker, pThis);

			if (pWHExt->Sonar_Duration > 0)
			{
				auto& nSonarTime = GetSonarTimer(pThis);
				if (pWHExt->Sonar_Duration > nSonarTime.GetTimeLeft())
				{
					nSonarTime.Start(pWHExt->Sonar_Duration);
				}
			}

			if (pWHExt->DisableWeapons_Duration > 0)
			{
				if (pWHExt->DisableWeapons_Duration > pExt->DisableWeaponTimer.GetTimeLeft())
				{
					pExt->DisableWeaponTimer.Start(pWHExt->DisableWeapons_Duration);
				}
			}

			if (pWHExt->Flash_Duration > 0 && pWHExt->Flash_Duration > pThis->Flashing.DurationRemaining)
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

	return bResult;
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
	GET_STACK(TechnoClass*, pAttacker, 0xD4);
	GET_STACK(HouseClass*, pAttacker_House, 0xE0);
	GET(int*, pDamage, EBX);

	if (pWH->Radiation && TechnoExt::IsRadImmune(pThis))
		return RetNullify;

	if (pWH->PsychicDamage && TechnoExt::IsPsionicsWeaponImmune(pThis))
		return RetNullify;

	if (pWH->Poison && TechnoExt::IsPoisonImmune(pThis))
		return RetNullify;

	const auto pSourceHouse = pAttacker ? pAttacker->Owner : pAttacker_House;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt->CanAffectHouse(pThis->Owner, pSourceHouse))
		return RetNullifyB;

	if (pWH->Psychedelic)
	{
		//This thing does ally check twice
		if (pSourceHouse && pSourceHouse->IsAlliedWith_(pThis))
			return RetUnaffected;

		if (Is_Building(pThis))
			return RetUnaffected;

		if (TechnoExt::IsPsionicsImmune(pThis) || TechnoExt::IsBerserkImmune(pThis))
			return RetUnaffected;

		// there is no building involved
		// More customizeable berzerk appying - Otamaa
		// return boolean to decide receive damage after apply berzerk or just retun function result
		if (!pWHExt->GoBerzerkFor(static_cast<FootClass*>(pThis), pDamage))
			return RetResultLight;
	}

	return RetObjectClassRcvDamage;
}

// If available, creates an InfantryClass instance and removes the hijacker from the victim.
NOINLINE InfantryClass* RecoverHijacker(FootClass* const pThis)
{
	if (auto const pType = InfantryTypeClass::Array->GetItemOrDefault(
		pThis->HijackerInfantryType))
	{
		const auto pOwner = HijackerOwner(pThis) ?
			HijackerOwner(pThis) : pThis->Owner;

		pThis->HijackerInfantryType = -1;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (!pTypeExt->HijackerOneTime && pOwner && !pOwner->Defeated)
		{
			if(auto const pHijacker = static_cast<InfantryClass*>(pType->CreateObject(pOwner))) {
				pHijacker->Health = MaxImpl(HijackerHealth(pThis), 10) / 2;
				pHijacker->Veterancy.Veterancy = HijackerVeterancy(pThis);
				return pHijacker;
			}
		}
	}

	return nullptr;
}

void NOINLINE SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses, const bool PreventPassengersEscape)
{
	auto const pType = pThis->GetTechnoType();
	auto const pOwner = pThis->Owner;
	auto const location = pThis->Location;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// do not ever do this again for this unit
	if (!Is_SurvivorsDone(pThis))
	{
		Is_SurvivorsDone(pThis) = true;
	}
	else
	{
		return;
	}

	// always eject passengers, but passengers only if not supressed.
	if (!Is_DriverKilled(pThis) && !IgnoreDefenses)
	{
		// save this, because the hijacker can kill people
		auto pilotCount = pThis->GetCrewCount();

		// process the hijacker
		if (auto const pHijacker = RecoverHijacker(pThis))
		{
			auto const pHijackerTypeExt = TechnoTypeExt::ExtMap.Find(pHijacker->Type);

			if (!TechnoExt::EjectRandomly(pHijacker, location, 144, Select))
			{
				pHijacker->RegisterDestruction(pKiller);

				if (pHijacker->IsAlive) {
					pHijacker->UnInit();
				}
			}
			else
			{
				// the hijacker will now be controlled instead of the unit
				if (auto const pController = pThis->MindControlledBy)
				{
					++Unsorted::IKnowWhatImDoing; // disables sound effects
					pController->CaptureManager->FreeUnit(pThis);
					pController->CaptureManager->CaptureUnit(pHijacker); // does the immunetopsionics check for us
					--Unsorted::IKnowWhatImDoing;
					pHijacker->QueueMission(Mission::Guard, true); // override the fate the AI decided upon
				}

				VocClass::PlayAt(pHijackerTypeExt->HijackerLeaveSound, location, nullptr);

				// lower than 0: kill all, otherwise, kill n pilots
				pilotCount = ((pHijackerTypeExt->HijackerKillPilots < 0) ? 0 :
					(pilotCount - pHijackerTypeExt->HijackerKillPilots));
			}
		}

		int pilotChance = pTypeExt->Survivors_PilotChance.Get(pThis);
		if (pilotChance < 0)
		{
			pilotChance = static_cast<int>(RulesClass::Instance->CrewEscape * 100);
		}

		// possibly eject up to pilotCount crew members
		if (pType->Crewed && pilotChance > 0)
		{
			for (int i = 0; i < pilotCount; ++i)
			{
				if (auto pPilotType = pThis->GetCrew())
				{
					if (ScenarioClass::Instance->Random.RandomRanged(1, 100) <= pilotChance)
					{
						auto const pPilot = static_cast<InfantryClass*>(pPilotType->CreateObject(pOwner));
						pPilot->Health /= 2;
						pPilot->Veterancy.Veterancy = pThis->Veterancy.Veterancy;

						if (!TechnoExt::EjectRandomly(pPilot, location, 144, Select))
						{
							pPilot->RegisterDestruction(pKiller);

							if (pPilot->IsAlive) {
								pPilot->UnInit();
							}
						}
						else if (auto const pTag = pThis->AttachedTag)
						{
							if (pTag->ShouldReplace())
							{
								pPilot->ReplaceTag(pTag);
							}
						}
					}
				}
			}
		}
	}

	if (!PreventPassengersEscape)
	{
		// passenger escape chances
		const auto passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

		// eject or kill all passengers
		while (pThis->Passengers.GetFirstPassenger())
		{
			auto const pPassenger = pThis->RemoveFirstPassenger();
			bool trySpawn = false;
			if (passengerChance > 0)
			{
				trySpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= passengerChance;
			}
			else if (passengerChance == -1 && Is_Unit(pThis))
			{
				const Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}

			if (trySpawn && TechnoExt::EjectRandomly(pPassenger, location, 128, Select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);

			if (pPassenger->IsAlive) {
				pPassenger->UnInit();
			}
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x737F97, UnitClass_ReceiveDamage_Survivours, 0xA)
{
	//GET(UnitTypeClass*, pType, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFSET(0x44, 0x18));

	SpawnSurvivors(pThis, pKiller, select, ignoreDefenses, preventPassangersEscape);

	R->EBX(-1);
	return 0x73838A;
}

DEFINE_OVERRIDE_HOOK(0x41668B, AircraftClass_ReceiveDamage_Survivours, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x28);
	GET_STACK(int, ignoreDefenses, 0x20);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFSET(0x14, 0x18));

	const bool bSelected = pThis->IsSelected && pThis->Owner && pThis->Owner->ControlledByPlayer();

	SpawnSurvivors(pThis, pKiller, bSelected, ignoreDefenses, preventPassangersEscape);

	return 0x0;
}

bool __fastcall FootClass_Crash_(FootClass* pThis , DWORD , ObjectClass* pSource)
{
	// Crashable support for aircraft
	const auto& nCrashable = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->Crashable;
	if (nCrashable.isset() && !nCrashable.Get()) {
		return false;
	}

	// call the address direcly instead of vtable
	return pThis->FootClass::Crash(pSource);
}

//replace an vtable call
DEFINE_JUMP(CALL6, 0x416694, GET_OFFSET(FootClass_Crash_));

// spawn tiberium when a unit dies. this is a minor part of the
// tiberium heal feature. the actual healing happens in FootClass_Update.
DEFINE_OVERRIDE_HOOK(0x702216, TechnoClass_ReceiveDamage_TiberiumHeal_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (TechnoTypeExt::ExtMap.Find(pType)->TiberiumRemains
		.Get(pType->TiberiumHeal && RulesExt::Global()->Tiberium_HealEnabled))
	{
		const CellClass* pCenter = MapClass::Instance->GetCellAt(pThis->Location);

		// increase the tiberium for the four neighbours and center.
		// center is retrieved by getting a neighbour cell index >= 8
		for (auto i = 0u; i < 5u; ++i)
		{
			pCenter->GetNeighbourCell(2 * i)
				->IncreaseTiberium(0, ScenarioClass::Instance->Random.RandomFromMax(2));
		}
	}

	return 0;
}

// smoke particle systems created when a techno is damaged
//#pragma optimize("", off )
DEFINE_OVERRIDE_HOOK(0x702894, TechnoClass_ReceiveDamage_SmokeParticles, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x30);

	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	const auto it = pExt->ParticleSystems_DamageSmoke.GetElements(pType->DamageParticleSystems);
	const auto allowAny = pExt->ParticleSystems_DamageSmoke.HasValue();

	for (const auto pSystem : it)
	{
		if (allowAny || pSystem->BehavesLike == BehavesLike::Smoke)
		{
			Systems.AddItem(pSystem);
		}
	}

	return 0x702938;
}
//#pragma optimize("", on)

constexpr unsigned int Neighbours[] = {
	9, 0, 2, 0, 0,
	0, 7, 0, 0, 0,
	1, 0, 0, 0, 4,
	0, 0, 3, 0, 0,
	0, 0, 5, 0, 0,
	0, 6, 0, 0, 0
};

// spill the stored tiberium on destruction
DEFINE_OVERRIDE_HOOK(0x702200, TechnoClass_ReceiveDamage_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	if (TechnoTypeExt::ExtMap.Find(pType)->TiberiumSpill)
	{
		double stored = pThis->Tiberium.GetTotalAmount();
		if (pThis->WhatAmI() != BuildingClass::AbsID
			&& stored > 0.0
			&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune)
		{
			// don't spill more than we can hold
			double max = 9.0;
			if (max > pType->Storage)
			{
				max = pType->Storage;
			}

			// assume about half full, recalc if possible
			int value = static_cast<int>(max / 2);
			if (pType->Storage > 0)
			{
				value = int(stored / pType->Storage * max);
			}

			// get the spill center
			CoordStruct crd = pThis->GetCoords();
			CellClass* pCenter = MapClass::Instance->GetCellAt(crd);

			for (auto neighbour : Neighbours)
			{
				// spill random amount
				int amount = ScenarioClass::Instance->Random.RandomFromMax(2);
				CellClass* pCell = pCenter->GetNeighbourCell(neighbour);
				pCell->IncreaseTiberium(0, amount);
				value -= amount;

				// stop if value is reached
				if (value <= 0)
				{
					break;
				}
			}
		}
	}

	return 0;
}

// #895584: ships not taking damage when repaired in a shipyard. bug
// was that the logic that prevented units from being damaged when
// exiting a war factory applied here, too. added the Naval check.
DEFINE_OVERRIDE_HOOK(0x737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
{
	GET(BuildingTypeClass*, pType, ECX);
	return (pType->WeaponsFactory && !pType->Naval)
		? 0x737CEE : 0x737D31;
}

DEFINE_OVERRIDE_HOOK(0x51849A, InfantryClass_ReceiveDamage_DeathAnim, 5)
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

	AnimTypeExt::SetMakeInfOwner(Anim, Invoker, I->Owner);

	R->EAX<AnimClass*>(Anim);
	return 0x5184F2;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x518575, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
DEFINE_OVERRIDE_HOOK(0x5183DE, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner);

	// bonus: don't require SpawnsParticle to be present

	if (ParticleSystemClass::Array->ValidIndex(pAnim->Type->SpawnsParticle))
	{
		return 0;
	}

	return (R->Origin() == 0x5183DE) ? 0x518422 : 0x5185B9;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x518B93, InfantryClass_ReceiveDamage_Anims, 5) // InfantryBrute
DEFINE_OVERRIDE_HOOK_AGAIN(0x518821, InfantryClass_ReceiveDamage_Anims, 5) // InfantryNuked
DEFINE_OVERRIDE_HOOK_AGAIN(0x5187BB, InfantryClass_ReceiveDamage_Anims, 5) // InfantryHeadPop
DEFINE_OVERRIDE_HOOK_AGAIN(0x518755, InfantryClass_ReceiveDamage_Anims, 5) // InfantryElectrocuted
DEFINE_OVERRIDE_HOOK_AGAIN(0x5186F2, InfantryClass_ReceiveDamage_Anims, 5) // FlamingInfantry
DEFINE_OVERRIDE_HOOK(0x518698, InfantryClass_ReceiveDamage_Anims, 5) // InfantryExplode
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner);

	return 0x5185F1;
}

DEFINE_OVERRIDE_HOOK(0x51887B, InfantryClass_ReceiveDamage_InfantryVirus2, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryVirus animation has been created. set the owner, but
	// reset the color for default (invoker).

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	auto res = AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner);
	if (res == OwnerHouseKind::Invoker)
	{
		pAnim->LightConvert = nullptr;
	}

	return 0x5185F1;
}

DEFINE_OVERRIDE_HOOK(0x518A96, InfantryClass_ReceiveDamage_InfantryMutate, 7)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));

	// Rules->InfantryMutate animation has been created. set the owner and color.

	auto pInvoker = Arguments.Attacker
		? Arguments.Attacker->Owner
		: Arguments.SourceHouse;

	AnimTypeExt::SetMakeInfOwner(pAnim, pInvoker, pThis->Owner);

	return 0x518AFF;
}

DEFINE_OVERRIDE_HOOK(0x518CB3, InfantryClass_ReceiveDamage_Doggie, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	// hurt doggie gets more panic
	if (pThis->Type->Doggie && pThis->IsRedHP())
	{
		R->EDI(RulesExt::Global()->DoggiePanicMax);
	}

	return 0;
}
