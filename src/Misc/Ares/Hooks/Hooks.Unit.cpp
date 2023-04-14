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

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <WWKeyboardClass.h>
#include <Conversions.h>

// TODO : require complete port of More IFV turrents
// 746B89 = UnitClass_GetUIName, 8

DEFINE_OVERRIDE_HOOK(0x73D219, UnitClass_Draw_OreGatherAnim, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	// disabled ore gatherers should not appear working.
	return (pTechno->IsWarpingIn() || pTechno->IsUnderEMP()) ?
		0x73D28E : 0x73D223;
}

DEFINE_HOOK(0x7461C5, UnitClass_BallooonHoverExplode_OverrideCheck, 0x6)
{
	GET(UnitClass*, pThis, EDI);
	GET(UnitTypeClass*, pType, EAX);

	R->CL(pType->BalloonHover || pType->Explodes || pThis->HasAbility(AbilityType::Explodes));
	return 0x7461CB;
}

DEFINE_OVERRIDE_HOOK(0x73F7DD, UnitClass_IsCellOccupied_Bib, 0x8)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(UnitClass*, pThis, EBX);

	return pThis && pBuilding->Owner->IsAlliedWith(pThis) ? 0x0 : 0x73F823;
}

// #1171643: keep the last passenger if this is a gunner, not just
// when it has multiple turrets. gattling and charge turret is no
// longer affected by this.
DEFINE_OVERRIDE_HOOK(0x73D81E, UnitClass_Mi_Unload_LastPassenger, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	R->EAX(pThis->GetTechnoType()->Gunner);
	return 0x73D823;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x7388EB, UnitClass_ActionOnObject_IvanBombs, 0x6, 7388FD)

DEFINE_OVERRIDE_HOOK(0x744745, UnitClass_RegisterDestruction_Trigger, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0x0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x73C733, UnitClass_DrawSHP_SkipTurretedShadow, 7, 73C7AC)

DEFINE_OVERRIDE_HOOK(0x741206, UnitClass_CanFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto Type = pThis->Type;

	if (!Type->TurretCount || Type->IsGattling) {
		return 0x741229;
	}

	const auto W = pThis->GetWeapon(pThis->SelectWeapon(nullptr));
	return (W->WeaponType && W->WeaponType->Warhead->Temporal)
		? 0x741210u
		: 0x741229u
		;
}

DEFINE_OVERRIDE_HOOK(0x741113, UnitClass_CanFire_Heal, 0xA)
{
	GET(ObjectClass*, pTarget, EDI);

	return RulesClass::Instance->ConditionGreen > pTarget->GetHealthPercentage() ?
		0x741121 : 0x74113A;
}

DEFINE_OVERRIDE_HOOK(0x73C613, UnitClass_DrawSHP_FacingsA, 0x7)
{
	GET(UnitClass*, pThis, EBP);

	unsigned int ret = 0;

	if (pThis->Type->Facings > 0)
	{
		auto highest = Conversions::Int2Highest(pThis->Type->Facings);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3 && !pThis->IsDisguised())
		{
			ret = pThis->PrimaryFacing.Current().GetValue(highest, 1u << (highest - 3));
		}
	}

	R->EBX(ret);
	return 0x73C64B;
}

DEFINE_OVERRIDE_HOOK(0x73CD01, UnitClass_DrawSHP_FacingsB, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);
	GET(int, facing, EAX);

	R->ECX(pThis);
	R->EAX(facing + pType->WalkFrames * pType->Facings);

	return 0x73CD06;
}

DEFINE_OVERRIDE_HOOK(0x739ADA, UnitClass_SimpleDeploy_Height, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Deployed)
		return 0x739CBF;

	if (!pThis->InAir && pThis->Type->DeployToLand && pThis->GetHeight() > 0)
		pThis->InAir = 1;

	R->EAX(true);
	return 0x739B14;
}

DEFINE_OVERRIDE_HOOK(0x736E8E, UnitClass_UpdateFiringState_Heal, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTargetTechno = generic_cast<TechnoClass*>(pThis->Target);

	if (!pTargetTechno || pTargetTechno->GetHealthPercentage() <= RulesClass::Instance()->ConditionGreen)
		pThis->SetTarget(nullptr);

	return 0x737063;
}

DEFINE_OVERRIDE_HOOK(0x7440BD, UnitClass_Remove, 0x6)
{
	GET(UnitClass*, U, ESI);

	if (auto Bld = specific_cast<BuildingClass*>(U->BunkerLinkedItem))
	{
		Bld->ClearBunker();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x739B8A, UnitClass_SimpleDeploy_Facing, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto const pType = pThis->Type;

	if (pType->DeployingAnim)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		// not sure what is the bitfrom or bitto so it generate this result
		// yes iam dum , iam sorry - otamaa
		const auto nRulesDeployDir = ((((RulesClass::Instance->DeployDir) >> 4) + 1) >> 1) & 7;
		const DirType8 nRaw = pTypeExt->DeployDir.isset() ? pTypeExt->DeployDir.Get() : (DirType8)nRulesDeployDir;
		const auto nCurrent = (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 7);

		if (nCurrent != (int)nRaw)
		{
			if (const auto pLoco = pThis->Locomotor.get())
			{
				if (!pLoco->Is_Moving_Now())
				{
					pLoco->Do_Turn(DirStruct { nRaw });
				}

				return 0x739C70;
			}
		}
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x74642C, UnitClass_ReceiveGunner, 6)
{
	GET(UnitClass*, Unit, ESI);
	TechnoExt::ExtMap.Find(Unit)->MyOriginalTemporal = Unit->TemporalImUsing;
	Unit->TemporalImUsing = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x74653C, UnitClass_RemoveGunner, 0xA)
{
	GET(UnitClass*, Unit, EDI);
	auto pData = TechnoExt::ExtMap.Find(Unit);
	Unit->TemporalImUsing = pData->MyOriginalTemporal;
	pData->MyOriginalTemporal = nullptr;
	return 0x746546;
}

DEFINE_OVERRIDE_HOOK(0x73769E, UnitClass_ReceivedRadioCommand_SpecificPassengers, 8)
{
	GET(UnitClass* const, pThis, ESI);
	GET(TechnoClass const* const, pSender, EDI);

	auto const pType = pThis->GetTechnoType();
	auto const pSenderType = pSender->GetTechnoType();

	return TechnoTypeExt::PassangersAllowed(pType, pSenderType) ? 0u : 0x73780Fu;
}

DEFINE_OVERRIDE_HOOK(0x73762B, UnitClass_ReceivedRadioCommand_BySize1, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x737677 : 0x73780F;
}

DEFINE_OVERRIDE_HOOK(0x73778F, UnitClass_ReceivedRadioCommand_BySize2, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers == pType->Passengers ?
		0x7377AA : 0x7377C9;
}

DEFINE_OVERRIDE_HOOK(0x73782F, UnitClass_ReceivedRadioCommand_BySize3, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x737877 : 0x73780F;
}

DEFINE_OVERRIDE_HOOK(0x737994, UnitClass_ReceivedRadioCommand_BySize4, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x7379E8 : 0x737AFC;
}

// TODO : port this
#define Is_DriverKilled(techno) \
(*(bool*)((char*)techno->align_154 + 0x9C))

#define GetDisableWeaponTimer(techno) \
(*(CDTimerClass*)((char*)techno->align_154 + 0x50))

DEFINE_OVERRIDE_HOOK(0x6FC0D3, TechnoClass_CanFire_DisableWeapons, 8)
{
	enum { FireRange = 0x6FC0DF, ContinueCheck = 0x0 };
	GET(TechnoClass*, pThis, ESI);
	return GetDisableWeaponTimer(pThis).InProgress()
		? FireRange : ContinueCheck;
}

// stop command would still affect units going berzerk
DEFINE_OVERRIDE_HOOK(0x730EE5, StopCommandClass_Execute_Berzerk, 6)
{
	GET(TechnoClass*, pTechno, ESI);
	return pTechno->Berzerk || Is_DriverKilled(pTechno) ? 0x730EF7 : 0;
}

DEFINE_OVERRIDE_HOOK(0x6F3283, TechnoClass_CanScatter_KillDriver, 8)
{
	// prevent units with killed drivers from scattering when attacked.
	GET(TechnoClass*, pThis, ESI);
	return (Is_DriverKilled(pThis) ? 0x6F32C5u : 0u);
}

DEFINE_OVERRIDE_HOOK(0x7091D6, TechnoClass_CanPassiveAquire_KillDriver, 6)
{
	// prevent units with killed drivers from looking for victims.
	GET(TechnoClass*, pThis, ESI);
	return (Is_DriverKilled(pThis) ? 0x70927Du : 0u);
}

DEFINE_OVERRIDE_HOOK(0x6F6A58, TechnoClass_DrawHealthBar_HidePips_KillDriver, 6)
{
	// prevent player from seeing pips on transports with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	return Is_DriverKilled(pThis) ? 0x6F6AB6u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x7087EB, TechnoClass_ShouldRetaliate_KillDriver, 6)
{
	// prevent units with killed drivers from retaliating.
	GET(TechnoClass*, pThis, ESI);
	return (Is_DriverKilled(pThis) ? 0x708B17u : 0u);
}

DEFINE_OVERRIDE_HOOK(0x73758A, UnitClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	return Is_DriverKilled(pThis) ? 0x73761Fu : 0u;
}

DEFINE_OVERRIDE_HOOK(0x70DEBA, TechnoClass_UpdateGattling_Cycle, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, lastStageValue, EAX);
	GET_STACK(int, a2, 0x24);

	auto pType = pThis->GetTechnoType();

	if (pThis->GattlingValue < lastStageValue)
	{
		// just increase the value
		pThis->GattlingValue += a2 * pType->RateUp;
	}
	else
	{
		// if max or higher, reset cyclic gattlings
		if (TechnoTypeExt::ExtMap.Find(pType)->GattlingCyclic)
		{
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			pThis->Audio4.AudioEventHandleStop();
			pThis->GattlingAudioPlayed = false;
		}
	}

	// recreate hooked instruction
	R->Stack<int>(0x10, pThis->GattlingValue);

	return 0x70DEEB;
}

// do not let deactivated teleporter units move, otherwise
// they could block a cell forever
DEFINE_OVERRIDE_HOOK(0x71810D, TeleportLocomotionClass_ILocomotion_MoveTo_Deactivated, 6)
{
	GET(FootClass*, pFoot, ECX);
	return (!pFoot->Deactivated && pFoot->Locomotor.get()->Is_Powered() && !Is_DriverKilled(pFoot))
		? 0 : 0x71820F;
}

// sink stuff that simply cannot exist on water
DEFINE_OVERRIDE_HOOK(0x7188F2, TeleportLocomotionClass_Unwarp_SinkJumpJets, 7)
{
	GET(CellClass*, pCell, EAX);
	GET(TechnoClass**, pTechno, ESI);

	if (pCell->Tile_Is_Wet() && !pCell->ContainsBridge())
	{
		if (UnitClass* pUnit = specific_cast<UnitClass*>(pTechno[3]))
		{
			if (pUnit->Deactivated || Is_DriverKilled(pUnit))
			{
				// this thing does not float
				R->BL(0);
			}

			// manually sink it
			if (pUnit->Type->SpeedType == SpeedType::Hover && pUnit->Type->JumpJet)
			{
				return 0x718A66;
			}
		}
	}

	return 0;
}

// iron curtained units would crush themselves
DEFINE_OVERRIDE_HOOK(0x7187DA, TeleportLocomotionClass_Unwarp_PreventSelfCrush, 6)
{
	GET(TechnoClass*, pTeleporter, EDI);
	GET(TechnoClass*, pContent, ECX);
	return (pTeleporter == pContent) ? 0x71880A : 0;
}

// bug 897
DEFINE_OVERRIDE_HOOK(0x718871, TeleportLocomotionClass_UnfreezeObject_SinkOrSwim, 7)
{
	GET(TechnoTypeClass*, Type, EAX);

	switch (Type->MovementZone)
	{
	case MovementZone::Amphibious:
	case MovementZone::AmphibiousCrusher:
	case MovementZone::AmphibiousDestroyer:
	case MovementZone::WaterBeach:
		R->BL(1);
		return 0x7188B1;
	}
	if (Type->SpeedType == SpeedType::Hover)
	{
		// will set BL to 1 , unless this is a powered unit with no power centers <-- what if we have a powered unit that's not a hover?
		return 0x71887A;
	}
	return 0x7188B1;
}

// #895584: ships not taking damage when repaired in a shipyard. bug
// was that the logic that prevented units from being damaged when
// exiting a war factory applied here, too. added the Naval check.
DEFINE_OVERRIDE_HOOK(0x737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
{
	GET(BuildingTypeClass*, pType, ECX);
	return (pType->WeaponsFactory && !pType->Naval) ? 0x737CEE : 0x737D31;
}

#include <Misc/AresData.h>

DEFINE_OVERRIDE_HOOK(0x737F86, UnitClass_ReceiveDamage_Survivours, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFS(0x44, -0x18));

	if (!ignoreDefenses)
	{
		// TODO : full port this
		AresData::SpawnSurvivors(pThis, pKiller, select, ignoreDefenses);
	}

	if (pType->OpenTopped)
	{
		pThis->MarkPassengersAsExited();
	}

	if (!preventPassangersEscape)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		// passenger escape chances
		auto const passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

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
				Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}
			if (trySpawn && TechnoExt::EjectRandomly(pPassenger, pThis->Location, 128, select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			TechnoExt::HandleRemove(pPassenger, pKiller, true);
		}
	}

	R->EBX(-1);
	return 0x737F97;
}

DEFINE_OVERRIDE_HOOK(0x41668B, AircraftClass_ReceiveDamage_Survivours, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x28);
	GET_STACK(int, ignoreDefenses, 0x20);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFS(0x14, -0x18));

	const bool bSelected = pThis->IsSelected && pThis->Owner && pThis->Owner->ControlledByPlayer();

	if (!ignoreDefenses)
	{
		// TODO : complete port this
		AresData::SpawnSurvivors(pThis, pKiller, bSelected, ignoreDefenses);
	}

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!preventPassangersEscape)
	{
		// passenger escape chances
		auto const passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

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
				Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}
			if (trySpawn && TechnoExt::EjectRandomly(pPassenger, pThis->Location, 128, bSelected))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			TechnoExt::HandleRemove(pPassenger, pKiller, true);
		}
	}

	// Crashable support for aircraft
	const auto& nCrashable = pTypeExt->Crashable;
	if (!nCrashable.Get(true))
	{
		R->EAX(0);
		return 0x41669A;
	}

	return 0x0;
}

void DepositTiberium(TechnoClass* pThis ,float const amount, float const bonus, int const idxType) {
	const auto pHouse = pThis->GetOwningHouse();
	const auto pTiberium = TiberiumClass::Array->GetItem(idxType);
	auto value = 0;

	// always put the purified money on the bank account. otherwise ore purifiers
	// would fill up storage with tiberium that doesn't exist. this is consistent with
	// the original YR, because old GiveTiberium put it on the bank anyhow, despite its name.
	if (bonus > 0.0) {
		value += Game::F2I(bonus * pTiberium->Value * pHouse->Type->IncomeMult);
	}

	// also add the normal tiberium to the global account?
	if (amount > 0.0) {
		auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		if (!pExt->Refinery_UseStorage) {
			value += Game::F2I(amount * pTiberium->Value * pHouse->Type->IncomeMult);
		}
		else {
			pHouse->GiveTiberium(amount, idxType);
		}
	}

	// deposit
	if (value > 0) {
		pHouse->GiveMoney(value);
	}
}

void RefineTiberium(TechnoClass* pThis , float const amount, int const idxType) {

	auto const pHouse = pThis->GetOwningHouse();

	// get the number of applicable purifiers
	auto purifiers = pHouse->NumOrePurifiers;
	if (!pHouse->CurrentPlayer && SessionClass::Instance->GameMode != GameMode::Campaign) {
		purifiers += RulesClass::Instance->AIVirtualPurifiers.GetItem(pHouse->GetAIDifficultyIndex());
	}

	// bonus amount (in tiberium)
	auto const purified = purifiers * RulesClass::Instance->PurifierBonus * amount;

	// add the tiberium to the house's credits
	DepositTiberium(pThis , amount, purified, idxType);
}

DEFINE_OVERRIDE_HOOK(0x73E4A2, UnitClass_Mi_Unload_Storage, 0x6)
{
	// because a value gets pushed to the stack in an inconvenient
	// location, we do our stuff and then mess with the stack so
	// the original functions do nothing any more.
	GET(BuildingClass* const, pBld, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amountRaw, 0x1C);
	REF_STACK(float, amountPurified, 0x34);

	DepositTiberium(pBld , amountRaw , amountPurified, idxTiberium);
	//AresData::TechnoExt_ExtData_DepositTiberium(pBld, amountRaw, amountPurified, idxTiberium);
	amountPurified = amountRaw = 0.0f;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x522D75, InfantryClass_Slave_UnloadAt_Storage, 6)
{
	GET(TechnoClass* const, pBld, EAX);
	GET(int const, idxTiberium, ESI);
	GET(StorageClass* const, pTiberium, EBP);

	// replaces the inner loop and stores
	// one tiberium type at a time
	auto const amount = pTiberium->GetAmount(idxTiberium);
	pTiberium->RemoveAmount(amount, idxTiberium);

	if(amount > 0.0) {
		RefineTiberium(pBld , amount, idxTiberium);

		// register for refinery smoke
		R->BL(1);
	}

	return 0x522E38;
}

DEFINE_OVERRIDE_HOOK(0x73DE90, UnitClass_Mi_Unload_SimpleDeployer, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Deployed
		&& pTypeExt->Convert_Deploy
		&& AresData::ConvertTypeTo(pThis, pTypeExt->Convert_Deploy))
	{
		pThis->Deployed = false;
	}

	return pThis->Locomotor.get()->Is_Moving_Now() ? 0x73E5B1 : 0x0;
}

// do not order deactivated units to move
DEFINE_OVERRIDE_HOOK(0x73DBF9, UnitClass_Mi_Unload_Decactivated, 5)
{
	GET(FootClass*, pUnloadee, EDI);
	LEA_STACK(CellStruct**, ppCell, 0x0);
	LEA_STACK(CellStruct*, pPosition, 0x1C);

	const auto pLoco = pUnloadee->Locomotor.get();

	if (pUnloadee->Deactivated)
	{
		pLoco->Power_Off();
	}

	if (!pLoco->Is_Powered())
	{
		*ppCell = pPosition;
	}

	return 0;
}

#include <SlaveManagerClass.h>
DEFINE_OVERRIDE_HOOK(0x73E66D, UnitClass_Mi_Harvest_SkipDock, 6) { return 0x73E6CF; }

DEFINE_OVERRIDE_HOOK(0x6AF748, SlaveManagerClass_UpdateSlaves_SlaveScan, 6)
{
	GET(InfantryClass*, pSlave, ESI);
	//GET(SlaveManagerClass*, pThis, EDI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->Type);
	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerSlaveScan));
	return 0x6AF74E;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6B026C, SlaveManagerClass_UpdateMiner_ShortScan, 6)
DEFINE_OVERRIDE_HOOK(0x6B006D, SlaveManagerClass_UpdateMiner_ShortScan, 6)
{
	GET(TechnoClass*, pSlaveOwner, ECX);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlaveOwner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerShortScan));
	return R->Origin() + 0x6;
}

DEFINE_OVERRIDE_HOOK(0x6B01A3, SlaveManagerClass_UpdateMiner_ScanCorrection, 6)
{
	GET(SlaveManagerClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_ScanCorrection.Get(RulesClass::Instance->SlaveMinerScanCorrection));
	return 0x6B01A9;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6B02CC, SlaveManagerClass_UpdateMiner_LongScan, 6)
DEFINE_OVERRIDE_HOOK_AGAIN(0x6B00BD, SlaveManagerClass_UpdateMiner_LongScan, 6)
DEFINE_OVERRIDE_HOOK(0x6AFDFC, SlaveManagerClass_UpdateMiner_LongScan, 6)
{
	GET(TechnoClass*, pSlaveOwner, ECX);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlaveOwner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_LongScan.Get(RulesClass::Instance->SlaveMinerLongScan));
	return R->Origin() + 0x6;
}

DEFINE_OVERRIDE_HOOK(0x6B1065, SlaveManagerClass_ShouldWakeUp_ShortScan, 5)
{
	GET(SlaveManagerClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());


	const auto nKickFrameDelay = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	if (nKickFrameDelay < 0 ||
		nKickFrameDelay + pThis->LastScanFrame >= Unsorted::CurrentFrame
		)
		return 0x6B10C6;

	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerShortScan));
	return 0x6B1085;
}

DEFINE_OVERRIDE_HOOK(0x73EC0E, UnitClass_Mi_Harvest_TooFarDistance1, 6)
{
	GET(UnitClass*, pThis, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EDX(pTypeExt->Harvester_TooFarDistance.Get(RulesClass::Instance->HarvesterTooFarDistance));
	return 0x73EC14;
}

DEFINE_OVERRIDE_HOOK(0x73EE40, UnitClass_Mi_Harvest_TooFarDistance2, 6)
{
	GET(UnitClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	R->EDX(pTypeExt->Harvester_TooFarDistance.Get(RulesClass::Instance->ChronoHarvTooFarDistance));
	return 0x73EE46;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x73EAC6, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_OVERRIDE_HOOK_AGAIN(0x73EAA6, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_OVERRIDE_HOOK_AGAIN(0x73EA17, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_OVERRIDE_HOOK(0x73E9F1, UnitClass_Mi_Harvest_ShortScan, 6)
{
	GET(UnitClass*, pThis, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->TiberiumShortScan));
	return R->Origin() + 6;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x73E772, UnitClass_Mi_Harvest_LongScan, 6)
DEFINE_OVERRIDE_HOOK(0x73E851, UnitClass_Mi_Harvest_LongScan, 6)
{
	GET(UnitClass*, pThis, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->TiberiumLongScan));
	return R->Origin() + 6;
}

DEFINE_OVERRIDE_HOOK(0x74081F, UnitClass_Mi_Guard_KickFrameDelay, 5)
{
	GET(UnitClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	const auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	return(nFrame < 0 || nFrame + pThis->CurrentMissionStartTime >= Unsorted::CurrentFrame) ?
		0x740854 : 0x74083B;
}

DEFINE_OVERRIDE_HOOK(0x74410D, UnitClass_Mi_AreaGuard_KickFrameDelay, 5)
{
	GET(UnitClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	const auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	return(nFrame < 0 || nFrame + pThis->CurrentMissionStartTime >= Unsorted::CurrentFrame) ?
		0x74416C : 0x744129;
}

#define Is_NavalYardSpied(var) \
(*(bool*)((char*)GetAresHouseExt(var) + 0x48))

DEFINE_OVERRIDE_HOOK_AGAIN(0x735678, UnitClass_Init_Academy, 6) // inlined in CTOR
DEFINE_OVERRIDE_HOOK(0x74689B, UnitClass_Init_Academy, 6)
{
	GET(UnitClass*, pThis, ESI);

	if (!pThis->Owner)
		return 0x0;

	const auto pType = pThis->Type;
	// TODO :  complete port of these
	if (pType->Trainable && pType->Naval && Is_NavalYardSpied(pThis->Owner)) {
		pThis->Veterancy.SetVeteran();
	}

	if (pType->ConsideredAircraft)
	{
		AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner , pThis, AbstractType::Aircraft);
	}
	else if (pType->Organic)
	{
		AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner , pThis, AbstractType::Infantry);
	}
	else
	{
		AresData::HouseExt_ExtData_ApplyAcademy(pThis->Owner, pThis, AbstractType::Unit);
	}

	return 0;
}

// make the space between gunner name segment and ifv
// name smart. it disappears if one of them is empty,
// eliminating leading and trailing spaces.
DEFINE_OVERRIDE_HOOK(0x746C55, UnitClass_GetUIName_Space, 6)
{
	GET(UnitClass*, pThis, ESI);
	GET(wchar_t*, pGunnerName, EAX);

	auto pName = pThis->Type->UIName;

	auto pSpace = L"";
	if (pName && *pName && pGunnerName && *pGunnerName) {
		pSpace = L" ";
	}

	_snwprintf_s(pThis->ToolTipText, sizeof(pThis->ToolTipText), L"%s%s%s", pGunnerName, pSpace, pName);

	R->EAX(pThis->ToolTipText);
	return 0x746C76;
}

DEFINE_OVERRIDE_HOOK(0x740031, UnitClass_GetActionOnObject_NoManualUnload, 6)
{
	GET(UnitClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualUnload ? 0x740115u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x417DD2, AircraftClass_GetActionOnObject_NoManualUnload, 6)
{
	enum { RetDefault = 0, NoUnload = 0x417DF4 };

	GET(AircraftClass const* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualUnload ? 0x417DF4u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x700EEC, TechnoClass_CanDeploySlashUnload_NoManualUnload, 6)
{
	// this techno is known to be a unit
	GET(UnitClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualUnload || pThis->BunkerLinkedItem 
	? 0x700DCEu : 0u;
}

// #908369, #1100953: units are still deployable when warping or falling
DEFINE_OVERRIDE_HOOK(0x700E47, TechnoClass_CanDeploySlashUnload_Immobile, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	CellClass* pCell = pThis->GetCell();
	CoordStruct crd = pCell->GetCoordsWithBridge();

	// recreate replaced check, and also disallow if unit is still warping or dropping in.
	return (pThis->IsUnderEMP()
	|| pThis->IsWarpingIn()
	|| pThis->IsFallingDown
	|| Is_DriverKilled(pThis)
	)

	? 0x700DCE : 0x700E59;
}

DEFINE_OVERRIDE_HOOK(0x53C450, TechnoClass_CanBePermaMC, 5)
{
	// complete rewrite. used by psychic dominator, ai targeting, etc.
	GET(TechnoClass*, pThis, ECX);
	BYTE bDisaalow = 0;

	if (pThis && pThis->WhatAmI() != AbstractType::Building
		&& !pThis->IsIronCurtained() && !pThis->IsInAir()) {

		const auto TechnoExt = TechnoExt::ExtMap.Find(pThis);
		if (!TechnoExt::IsPsionicsImmune(pThis) && !TechnoExt->Type->BalloonHover) {
			// KillDriver check
			if (!Is_DriverKilled(pThis))
			{
				bDisaalow = 1;
			}
		}
	}

	R->AL(bDisaalow);
	return 0x53C4BA;
}

DEFINE_OVERRIDE_HOOK(0x700536, TechnoClass_GetActionOnObject_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualFire ? 0x70056Cu : 0u;
}

DEFINE_OVERRIDE_HOOK(0x7008D4, TechnoClass_GetCursorOverCell_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualFire ? 0x700AB7u : 0u;
}

DEFINE_OVERRIDE_HOOK(0x74031A, UnitClass_GetActionOnObject_NoManualEnter, 6)
{
	//GET(UnitClass const* const, pThis, ESI);
	GET(TechnoTypeClass*, pTargetType, EAX);
	const bool enterable = pTargetType->Passengers > 0 && !TechnoTypeExt::ExtMap.Find(pTargetType)->NoManualEnter;
	return enterable ? 0x740324u : 0x74037Au;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x6F7FC5, TechnoClass_CanAutoTargetObject_Heal, 7, 6F7FDF)

DEFINE_OVERRIDE_HOOK(0x51C913, InfantryClass_CanFire_Heal, 7)
{
	GET(ObjectClass*, pTarget, EDI);

	bool bCond = false;

	if (const auto pTech = generic_cast<TechnoClass*>(pTarget)) {
		bCond = RulesClass::Instance->ConditionGreen > pTarget->GetHealthPercentage_();
	}

	return bCond ? 0x51C947 : 0x51C939;
}

DEFINE_OVERRIDE_HOOK(0x6FA361, TechnoClass_Update_LoseTarget, 5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);

	const HouseClass* pOwner = R->BL() ? pHouse : pThis->Owner;
	bool IsAlly = false;
	if (const auto pTechTarget = generic_cast<TechnoClass*>(pThis->Target)) {
		IsAlly = pTechTarget->Owner->IsAlliedWith(pOwner);
	}

	return IsAlly == (pThis->CombatDamage() <= 0)  ? 0x6FA472 : 0x6FA39D;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F8F1F, TechnoClass_FindTargetType_Heal, 6)
DEFINE_OVERRIDE_HOOK(0x6F8EE3, TechnoClass_FindTargetType_Heal, 6)
{
	GET(unsigned int, nVal, EBX);

	nVal |= 0x403Cu;

	R->EBX(nVal);
	return 0x6F8F25;
}

DEFINE_OVERRIDE_HOOK(0x51E710, InfantryClass_GetActionOnObject_Heal, 7)
{
	enum
	{
		ActionGueardArea = 0x51E748,
		NextCheck = 0x51E757,
		NextCheck2 = 0x51E7A6,
		DoActionHeal = 0x51E739
	};

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pThat, ESI);

	if (pThis == pThat)
		return ActionGueardArea;

	const auto pThatTechno = generic_cast<TechnoClass*>(pThat);
	if (!pThatTechno)
		return NextCheck;

	const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThatTechno))->WeaponType;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
	const auto pType = pThatTechno->GetTechnoType();
	const auto& Verses = pWHExt->GetVerses(pType->Armor).Verses;

	if (pThatTechno->IsFullHP() || WWKeyboardClass::Instance->IsForceMoveKeyPressed() || Verses <= 0.0) {
		if (Is_Building(pThatTechno)) {
			const auto pBuilding = static_cast<BuildingClass*>(pThatTechno);
			if (pBuilding->Type->Grinding){
				return NextCheck2;
			}
		}

		return NextCheck;
	}

	const size_t nCursor = (pType->Organic || Is_Infantry(pThat)) ? 90 : 91;
	//TODO : properly port this
	AresData::SetMouseCursorAction(nCursor, Action::Heal, false);
	return DoActionHeal;
}

DEFINE_OVERRIDE_HOOK(0x73FDBD, UnitClass_GetActionOnObject_Heal, 5)
{
	GET(UnitClass*, pThis, EDI);
	GET(ObjectClass*, pThat, ESI);
	GET(Action, nAct, EBX);

	if (nAct == Action::GuardArea)
		return 0x73FE48;

	const auto pThatTechno = generic_cast<TechnoClass*>(pThat);

	if (WWKeyboardClass::Instance->IsForceMoveKeyPressed() ||
		!pThatTechno ||
		pThatTechno->IsFullHP() ||
		!pThat->IsSurfaced()) {
		return 0x73FE3B;
	}

	if (Is_Aircraft(pThat)) {
		const auto pAir = static_cast<AircraftClass*>(pThat);
		if (pAir->GetCell()->GetBuilding()) {
			return 0x73FE3B;
		}
	}

	const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThat))->WeaponType;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
	const auto pType = pThat->GetTechnoType();
	const auto& Verses = pWHExt->GetVerses(pType->Armor).Verses;

	if (Verses <= 0.0)
		return 0x73FE3B;

	const size_t nCursor = (pType->Organic || Is_Infantry(pThat)) ? 90 : 91;
	//TODO : properly port this
	AresData::SetMouseCursorAction(nCursor, Action::GRepair, false);

	return 0x73FE08;
}

UnitTypeClass* GetUnitTypeImage(UnitClass* const pThis)
{
	const auto pData = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pData->WaterImage && !pThis->OnBridge && pThis->GetCell()->LandType == LandType::Water)
	{
		return pData->WaterImage;
	}

	return nullptr;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x73C69D, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_OVERRIDE_HOOK_AGAIN(0x73C702, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_OVERRIDE_HOOK(0x73C655, UnitClass_DrawSHP_ChangeType1, 6)
{
	GET(UnitClass*, U, EBP);

	if (UnitTypeClass* pCustomType = GetUnitTypeImage(U)) {
		R->ECX<UnitTypeClass*>(pCustomType);
		return R->Origin() + 6;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73C5FC, UnitClass_DrawSHP_WaterType, 6)
{
	GET(UnitClass*, U, EBP);

	SHPStruct* Image = U->GetImage();

	if (UnitTypeClass* pCustomType = GetUnitTypeImage(U)) {
		Image = pCustomType->GetImage();
	}

	if (Image) {
		R->EAX<SHPStruct*>(Image);
		return 0x73C602;
	}

	return 0x73CE00;
}

DEFINE_OVERRIDE_HOOK(0x73B4A0, UnitClass_DrawVXL_WaterType, 9)
{
	R->ESI(0);
	GET(UnitClass*, U, EBP);

	ObjectTypeClass* Image = U->Type;

	if (!U->IsClearlyVisibleTo(HouseClass::CurrentPlayer)) {
		Image = U->GetDisguise(true);
	}

	if (U->Deployed) {
		if (UnitTypeClass* const Unloader = U->Type->UnloadingClass) {
			Image = Unloader;
		}
	}

	if (UnitTypeClass* const pCustomType = GetUnitTypeImage(U)) {
		Image = pCustomType;
	}

	R->EBX<ObjectTypeClass*>(Image);
	return 0x73B4DA;
}

DEFINE_OVERRIDE_HOOK(0x715320, TechnoTypeClass_LoadFromINI_EarlyReader, 6)
{
	GET(CCINIClass*, pINI, EDI);
	GET(TechnoTypeClass*, pType, EBP);

	INI_EX exINI(pINI);
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pType);

	pData->WaterImage.Read(exINI, pType->ID, "WaterImage");

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);

	const auto pTechnoType = pThis->GetTechnoType();
	const bool bAnimAttached = pTechnoType ? pThis->Parachute != 0 : pThis->HasParachute;

	int nFallRate = 1;
	int nMaxFallRate = bAnimAttached ? RulesClass::Instance->ParachuteMaxFallRate : RulesClass::Instance->NoParachuteMaxFallRate;

	if (pTechnoType)
	{
		const auto pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		nFallRate = !bAnimAttached ? pExt->FallRate_NoParachute.Get() : pExt->FallRate_Parachute.Get();
		auto& nCustomMaxFallRate = !bAnimAttached ? pExt->FallRate_NoParachuteMax : pExt->FallRate_ParachuteMax;

		if (nCustomMaxFallRate.isset())
			nMaxFallRate = nCustomMaxFallRate.Get();
	}

	if (pThis->FallRate - nFallRate >= nMaxFallRate)
		nMaxFallRate = pThis->FallRate - nFallRate;

	pThis->FallRate = nMaxFallRate;
	return 0x5F3FFD;
}

struct TurrentDummy
{
	static NOINLINE int ObjectTypeClass_Load3DArt_Barrels(REGISTERS* R)
	{
		GET(TechnoTypeClass*, pThis, ESI);

		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

		if (pThis->TurretCount <= 0)
			return 0x5F8A60;

		auto const nRemaining = pThis->TurretCount - 17;
		if (nRemaining > 0)
		{
			TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);
			if (pTypeExt->BarrelImageData.size() <= (size_t)(nRemaining)) {
				pTypeExt->BarrelImageData.resize((size_t)nRemaining);
			}
		}

		char Buffer[0x40];
		for (int i = 0; ; ++i)
		{
			auto pKey = !i ? "%sBARL" : "%sBARL%d";
			IMPL_SNPRNINTF(Buffer, sizeof(Buffer), pKey, pThis->ImageFile, i);

			auto& nArr = i >= TechnoTypeClass::MaxWeapons ?
				pTypeExt->BarrelImageData[i - TechnoTypeClass::MaxWeapons] : pThis->ChargerBarrels[i];

			ImageStatusses nPairStatus {};
			ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

			if (nArr.VXL != nPairStatus.Images.VXL) {
				std::swap(nArr.VXL, nPairStatus.Images.VXL);
			}

			if (nArr.HVA != nPairStatus.Images.HVA) {
				std::swap(nArr.HVA, nPairStatus.Images.HVA);
			}

			//clean up
			GameDelete<true, true>(nPairStatus.Images.HVA);
			GameDelete<true, true>(nPairStatus.Images.VXL);

			if (!nPairStatus.Loaded) {
				Debug::Log("%s Techno Barrel [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, Buffer, i);
				break;
			}

			if (i >= pThis->TurretCount)
				return 0x5F8A60;
		}

		return 0x5F8A6A;
	}

	static NOINLINE int ObjectTypeClass_Load3DArt_Turrets(REGISTERS* R)
	{
		GET(TechnoTypeClass*, pThis, ESI);

		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);

		if (pThis->TurretCount <= 0)
			return 0x5F8844;

		auto const nRemaining = pThis->TurretCount - 17;
		if (nRemaining > 0)
		{
			TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);
			if (pTypeExt->TurretImageData.size() <= (size_t)(nRemaining)) {
				pTypeExt->TurretImageData.resize((size_t)nRemaining);
			}
		}

		char Buffer[0x40];
		for (int i = 0; ; ++i)
		{
			auto pKey = !i ? "%sTUR" : "%sTUR%d";
			IMPL_SNPRNINTF(Buffer, sizeof(Buffer), pKey, pThis->ImageFile, i);

			auto& nArr = i >= TechnoTypeClass::MaxWeapons ? pTypeExt->TurretImageData[i - TechnoTypeClass::MaxWeapons] : pThis->ChargerTurrets[i];

			ImageStatusses nPairStatus {};
			ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

			if (nArr.VXL != nPairStatus.Images.VXL) {
				std::swap(nArr.VXL, nPairStatus.Images.VXL);
			}

			if (nArr.HVA != nPairStatus.Images.HVA) {
				std::swap(nArr.HVA, nPairStatus.Images.HVA);
			}

			//clean up
			GameDelete<true, true>(nPairStatus.Images.HVA);
			GameDelete<true, true>(nPairStatus.Images.VXL);

			if (!nPairStatus.Loaded)
			{
				Debug::Log("%s Techno Turret [%s] at[%d] cannot be loaded , breaking the loop ! \n", pThis->ID, Buffer, i);
				break;
			}

			if (i >= pThis->TurretCount)
				return 0x5F8844;
		}

		return 0x5F868C;
	}
};

DEFINE_OVERRIDE_HOOK(0x5F887B, ObjectTypeClass_Load3DArt_Barrels, 6)
{
	return TurrentDummy::ObjectTypeClass_Load3DArt_Barrels(R);
}

DEFINE_OVERRIDE_HOOK(0x5F865F, ObjectTypeClass_Load3DArt_Turrets, 6)
{
	return TurrentDummy::ObjectTypeClass_Load3DArt_Turrets(R);
}

DEFINE_OVERRIDE_HOOK(0x5F8277, ObjectTypeClass_Load3DArt_NoSpawnAlt1, 7)
{
	REF_STACK(bool, bLoadFailed, 0x13);
	GET(ObjectTypeClass*, pThis, ESI);

	if (!pThis || (!Is_AircraftType(pThis) && !Is_UnitType(pThis) && !Is_InfantryType(pThis) && !Is_BuildingType(pThis)))
		return 0x5F8640;

	TechnoTypeClass* const pType = reinterpret_cast<TechnoTypeClass*>(pThis);

	if (pType->NoSpawnAlt)
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		char Buffer[0x40];
		auto pKey = "%sWO";
		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), pKey, pThis->ImageFile);
		ImageStatusses nPairStatus {};
		ImageStatusses::ReadVoxel(nPairStatus, Buffer, 1);

		if (pTypeExt->SpawnAltData.VXL != nPairStatus.Images.VXL) {
			std::swap(pTypeExt->SpawnAltData.VXL, nPairStatus.Images.VXL);
		}

		if (pTypeExt->SpawnAltData.HVA != nPairStatus.Images.HVA) {
			std::swap(pTypeExt->SpawnAltData.HVA, nPairStatus.Images.HVA);
		}

		//clean up
		GameDelete<true, true>(nPairStatus.Images.HVA);
		GameDelete<true, true>(nPairStatus.Images.VXL);

		if (!nPairStatus.Loaded)
		{
			Debug::Log("%s Techno NoSpawnAlt Image[%s] cannot be loaded ,returning load failed ! \n", pThis->ID, Buffer);
			bLoadFailed = true;
		}
	}

	return 0x5F8287;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x5F848C, ObjectTypeClass_Load3DArt_NoSpawnAlt2, 6, 5F8844)

DEFINE_OVERRIDE_HOOK(0x5F8084, ObjectTypeClass_UnloadTurretArt, 6)
{
	GET(ObjectTypeClass*, pThis, ESI);

	if (!pThis || (!Is_AircraftType(pThis) && !Is_UnitType(pThis) && !Is_InfantryType(pThis) && !Is_BuildingType(pThis)))
		return 0;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(reinterpret_cast<TechnoTypeClass*>(pThis));
	TechnoTypeExt::ClearImageData(pTypeExt->BarrelImageData);
	TechnoTypeExt::ClearImageData(pTypeExt->TurretImageData);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73B6E3, UnitClass_DrawVXL_NoSpawnAlt, 6)
{
	GET(UnitTypeClass*, pType, EBX);
	R->EDX(&TechnoTypeExt::ExtMap.Find(pType)->SpawnAltData);
	return 0x73B6E9;
}

DEFINE_OVERRIDE_HOOK(0x73C485, UnitClass_DrawVXL_NoSpawnAlt_SkipShadow, 8)
{
	enum { DoNotDrawShadow = 0x73C5C9, ShadowAlreadyDrawn = 0x0 };

	GET(UnitClass*, pThis, EBP);
	auto const pSpawnManager = pThis->SpawnManager;

	if (pThis->Type->NoSpawnAlt 
		&& pSpawnManager 
		&& pSpawnManager->CountDockedSpawns() < pSpawnManager->SpawnCount
		)
	{
		if (TechnoTypeExt::ExtMap.Find(pThis->Type)->NoShadowSpawnAlt.Get())
			return DoNotDrawShadow;
	}

	return ShadowAlreadyDrawn;
}

DEFINE_OVERRIDE_HOOK(0x73B90E, UnitClass_DrawVXL_Barrels1, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, EAX);

	R->Stack(0x2C, R->ESI());
	auto const pData = TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx);
	if (!pData->VXL || !pData->HVA)
		return 0x73B94A;

	return 0x73B928;
}

DEFINE_OVERRIDE_HOOK(0x73BCCD, UnitClass_DrawVXL_Barrels2, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ECX);
	R->EDX(TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx));
	return 0x73BCD4;
}

DEFINE_OVERRIDE_HOOK(0x73BD6A, UnitClass_DrawVXL_Barrels3, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExt::GetBarrelsVoxelData(pUnit, nIdx));
	return 0x73BD71;
}

DEFINE_OVERRIDE_HOOK(0x73BD15, UnitClass_DrawVXL_Turrets, 7)
{
	GET(UnitTypeClass*, pUnit, EBX);
	GET(int, nIdx, ESI);
	R->ECX(TechnoTypeExt::GetTurretVoxelData(pUnit, nIdx));
	return 0x73BD1C;
}

static bool ShadowAlreadyDrawn;

DEFINE_OVERRIDE_HOOK(0x73C725, UnitClass_DrawSHP_DrawShadowEarlier, 6)
{
	GET(UnitClass*, U, EBP);

	DWORD retAddr = (U->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
		? 0
		: 0x73CE0D
		;

	// TODO: other conditions where it would not make sense to draw shadow
	switch (U->VisualCharacter(VARIANT_FALSE, nullptr))
	{
	case VisualType::Normal:
	case VisualType::Indistinct:
		break;
	default:
		return retAddr;
	}

	if (U->CloakState != CloakState::Uncloaked || U->Type->Underwater || U->Type->SmallVisceroid || U->Type->LargeVisceroid)
	{
		return retAddr;
	}

	GET(SHPStruct*, Image, EDI);

	if (Image)
	{ // bug #960
		GET(int, FrameToDraw, EBX);
		GET_STACK(Point2D, coords, 0x12C);
		LEA_STACK(RectangleStruct*, BoundingRect, 0x134);

		if (U->IsOnCarryall)
		{
			coords.Y -= 14;
		}

		Point2D XYAdjust = U->Locomotor->Shadow_Point();
		coords += XYAdjust;

		int ZAdjust = U->GetZAdjustment() - 2;

		FrameToDraw += Image->Frames / 2;

		DSurface::Hidden_2->DrawSHP(FileSystem::THEATER_PAL, Image, FrameToDraw, &coords, BoundingRect, BlitterFlags(0x2E01),
				0, ZAdjust, 0, 1000, 0, 0, 0, 0, 0);

		ShadowAlreadyDrawn = true;
	}

	return retAddr;
}

DEFINE_OVERRIDE_HOOK(0x705FF3, TechnoClass_Draw_A_SHP_File_SkipUnitShadow, 6)
{

	if (ShadowAlreadyDrawn) {
		ShadowAlreadyDrawn = false;
		return 0x706007;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x739956, UnitClass_Deploy_TransferStatusses, 6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	AresData::TechnoTransferAffects(pUnit, pStructure);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4140EB, AircraftClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, EDI);

	if (AresData::IsGenericPrerequisite(pThis->Type)) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x517DF2, InfantryClass_DTOR_Prereqs, 6)
{
	GET(InfantryClass* const, pThis, ESI);

	if (AresData::IsGenericPrerequisite(pThis->Type)) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7357F6, UnitClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, ESI);

	if (AresData::IsGenericPrerequisite(pThis->Type)) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D7221, FootClass_Put_Prereqs, 6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (AresData::IsGenericPrerequisite(pType)) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x6F4A37, TechnoClass_DiscoveredBy_Prereqs, 5)
DEFINE_OVERRIDE_HOOK(0x6F4A1D, TechnoClass_DiscoveredBy_Prereqs, 6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!Is_Building(pThis) && AresData::IsGenericPrerequisite(pType)) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7015EB, TechnoClass_ChangeOwnership_Prereqs, 7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();

	if (!Is_Building(pThis) && AresData::IsGenericPrerequisite(pType))
	{
		pThis->Owner->RecheckTechTree = true;
		pNewOwner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x741613, UnitClass_ApproachTarget_OmniCrusher, 6)
{
	GET(UnitClass* const, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	return pExt->OmniCrusher_Aggressive ? 0u : 0x741685u;
}

DEFINE_OVERRIDE_HOOK(0x7418AA, UnitClass_CrushCell_CrushDamage, 6)
{
	GET(UnitClass* const, pThis, EDI);
	GET(ObjectClass* const, pVictim, ESI);

	if (auto const pTechno = abstract_cast<TechnoClass*>(pVictim))
	{
		auto pExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());

		auto const pWarhead = pExt->CrushDamageWarhead.Get(
			RulesClass::Instance->C4Warhead);

		auto damage = pExt->CrushDamage.Get(pTechno);
		bool isDead = false;

		if (pWarhead && damage)
		{
			isDead = pThis->ReceiveDamage(
				&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead;
		}

		if (!isDead) {
			if (!WarheadTypeExt::ExtMap.Find(pWarhead)->ApplySuppressDeathWeapon(pTechno))
				pTechno->FireDeathWeapon(0);

		} else {
			return 0x741916; // continue the loop
		}
	}

	return 0;// continue crush function
}

DEFINE_OVERRIDE_HOOK(0x735584, UnitClass_CTOR_TurretROT, 6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->EDX(TechnoTypeExt::ExtMap.Find(pType)->TurretRot.Get(pType->ROT));
	return 0x73558A;
}

#include <TunnelLocomotionClass.h>
#include <Ext/Anim/Body.h>


namespace AresHadleTunnelLocoStuffs
{
	void Handle(FootClass* pOwner, bool DugIN = false, bool PlayAnim = false)
	{
		const auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		const auto pRules = RulesClass::Instance();
		const auto nSound =  (DugIN ? pExt->DigInSound : pExt->DigOutSound).Get(pRules->DigSound);

		VocClass::PlayAt(nSound, pOwner->Location);

		if (PlayAnim) {
			if (const auto pAnimType = (DugIN ? pExt->DigInAnim : pExt->DigOutAnim).Get(pRules->Dig)) {
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, pOwner->Location)) {
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, nullptr, false);
				}
			}
		}
	}

}

DEFINE_OVERRIDE_HOOK(0x728EF0, TunnelLocomotionClass_ILocomotion_Process_Dig, 5)
{
	GET(FootClass*, pFoot, EAX);

	AresHadleTunnelLocoStuffs::Handle(pFoot, true, true);
	return 0x728F74;
}

DEFINE_OVERRIDE_HOOK(0x7292CF, TunnelLocomotionClass_sub_7291F0_Dig, 8)
{
	GET(RepeatableTimerStruct*, pTimer, EDX);
	GET(TunnelLocomotionClass*, pThis, ESI);
	GET(int, nTimeLeft, EAX);

	pTimer->Start(nTimeLeft);
	AresHadleTunnelLocoStuffs::Handle(pThis->LinkedTo, true, true);
	return 0x729365;

}

DEFINE_OVERRIDE_HOOK(0x7293DA, TunnelLocomotionClass_sub_729370_Dig, 6)
{
	GET(FootClass*, pFoot, ECX);

	AresHadleTunnelLocoStuffs::Handle(pFoot, true, true);
	return 0x72945E;
}

DEFINE_OVERRIDE_HOOK(0x7297C4, TunnelLocomotionClass_sub_729580_Dig, 6)
{
	GET(FootClass*, pFoot, EAX);

	AresHadleTunnelLocoStuffs::Handle(pFoot, false, false);
	return 0x7297F3;
}

DEFINE_OVERRIDE_HOOK(0x7299A9, TunnelLocomotionClass_sub_7298F0_Dig, 5)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	AresHadleTunnelLocoStuffs::Handle(pThis->LinkedTo, false, true);
	return 0x729A34;
}

DEFINE_OVERRIDE_HOOK(0x72920C, TunnelLocomotionClass_Turning, 9)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	if (pThis->_CoordsNow.X || pThis->_CoordsNow.Y || pThis->_CoordsNow.Z)
		return 0;

	pThis->State = TunnelLocomotionClass::State::DUG_OUT;
	return 0x729369;
}

// spawn tiberium when a unit dies. this is a minor part of the
// tiberium heal feature. the actual healing happens in FootClass_Update.
DEFINE_OVERRIDE_HOOK(0x702216, TechnoClass_ReceiveDamage_TiberiumHeal_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (TechnoTypeExt::ExtMap.Find(pType)->TiberiumRemains
		.Get(pType->TiberiumHeal && RulesExt::Global()->Tiberium_HealEnabled))
	{
		CoordStruct crd = pThis->GetCoords();
		CellClass* pCenter = MapClass::Instance->GetCellAt(crd);

		// increase the tiberium for the four neighbours and center.
		// center is retrieved by getting a neighbour cell index >= 8
		for (auto i = 0u; i < 5u; ++i)
		{
			CellClass* pCell = pCenter->GetNeighbourCell(2 * i);
			int value = ScenarioClass::Instance->Random.RandomRanged(0, 2);
			pCell->IncreaseTiberium(0, value);
		}
	}

	return 0;
}

// smoke particle systems created when a techno is damaged
DEFINE_OVERRIDE_HOOK(0x702894, TechnoClass_ReceiveDamage_SmokeParticles, 6)
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const*>, Systems, 0x30);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto it = pExt->ParticleSystems_DamageSmoke.GetElements(pType->DamageParticleSystems);
	auto allowAny = pExt->ParticleSystems_DamageSmoke.HasValue();

	for (auto pSystem : it)
	{
		if (allowAny || pSystem->BehavesLike == BehavesLike::Smoke)
		{
			Systems.AddItem(pSystem);
		}
	}

	return 0x702938;
}

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
				value = Game::F2I(stored / pType->Storage * max);
			}

			// get the spill center
			CoordStruct crd = pThis->GetCoords();
			CellClass* pCenter = MapClass::Instance->GetCellAt(crd);

			for (auto neighbour : Neighbours)
			{
				// spill random amount
				int amount = ScenarioClass::Instance->Random.RandomRanged(0, 2);
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

// spread tiberium on building destruction. replaces the
// original code, made faster and spilling is now optional.
DEFINE_OVERRIDE_HOOK(0x441B30, BuildingClass_Destroy_Refinery, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	auto& store = pThis->Tiberium;
	auto& total = pThis->Owner->OwnedTiberium;

	// remove the tiberium contained in this structure from the house's owned
	// tiberium. original code does this one bail at a time, we do bulk.
	if (store.GetTotalAmount() >= 1.0)
	{
		for (auto i = 0u; i < 4; ++i)
		{
			auto const amount = std::ceil(store.GetAmount(i));
			if (amount > 0.0)
			{
				store.RemoveAmount(amount, i);
				total.RemoveAmount(amount, i);

				// spread bail by bail
				if (pExt->TiberiumSpill)
				{
					for (auto j = static_cast<int>(amount); j; --j)
					{
						auto const dist = ScenarioClass::Instance->Random.RandomRanged(256, 768);
						auto const crd = MapClass::GetRandomCoordsNear(pThis->Location, dist, true);

						auto const pCell = MapClass::Instance->GetCellAt(crd);
						pCell->IncreaseTiberium(i, 1);
					}
				}
			}
		}
	}

	return 0x441C0C;
}


DEFINE_OVERRIDE_HOOK(0x6F3410, TechnoClass_SelectWeapon_NoAmmoWeapon, 5)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();

	if (pType->Ammo < 0)
		return 0x0;

	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->NoAmmoWeapon < 0 || pThis->Ammo > pExt->NoAmmoAmount)
		return 0x0;

	R->EAX(pExt->NoAmmoWeapon);
	return 0x6F3406;
}

// select the most appropriate firing voice and also account
// for undefined flags, so you actually won't lose functionality
// when a unit becomes elite.
DEFINE_OVERRIDE_HOOK(0x7090A8, TechnoClass_SelectFiringVoice, 5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, ECX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pType);

	int idxVoice = -1;

	int idxWeapon = pThis->SelectWeapon(pTarget);
	WeaponTypeClass* pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;

	// repair
	if (pWeapon && pWeapon->Damage < 0)
	{
		idxVoice = pData->VoiceRepair;
		if (idxVoice < 0)
		{
			if (!(IS_SAME_STR_(pType->ID, "FV")))
			{
				idxVoice = RulesClass::Instance->VoiceIFVRepair;
			}
		}
	}

	// don't mix them up, but fall back to rookie voice if there
	// is no elite voice.
	if (idxVoice < 0)
	{
		if (idxWeapon)
		{
			// secondary
			if (pThis->Veterancy.IsElite())
			{
				idxVoice = pType->VoiceSecondaryEliteWeaponAttack;
			}

			if (idxVoice < 0)
			{
				idxVoice = pType->VoiceSecondaryWeaponAttack;
			}
		}
		else
		{
			// primary
			if (pThis->Veterancy.IsElite())
			{
				idxVoice = pType->VoicePrimaryEliteWeaponAttack;
			}

			if (idxVoice < 0)
			{
				idxVoice = pType->VoicePrimaryWeaponAttack;
			}
		}
	}

	// generic attack voice
	if (idxVoice < 0 && pType->VoiceAttack.Count)
	{
		unsigned int idxRandom = Random2Class::Global->Random();
		idxVoice = pType->VoiceAttack.GetItem(idxRandom % pType->VoiceAttack.Count);
	}

	// play voice
	if (idxVoice > -1)
	{
		pThis->QueueVoice(idxVoice);
	}

	return 0x7091C5;
}

// variable amounts of rounds to reload
DEFINE_OVERRIDE_HOOK(0x6FB05B, TechnoClass_Reload_ReloadAmount, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	int amount = pExt->ReloadAmount;
	if (!pThis->Ammo)
	{
		amount = pExt->EmptyReloadAmount.Get(amount);
	}

	// clamping to support negative values
	auto const ammo = pThis->Ammo + amount;
	pThis->Ammo = std::clamp(ammo, 0, pType->Ammo);

	return 0x6FB061;
}


static constexpr std::array<const char* const, 17> SubName { {
	"Normal", "Repair" ,"MachineGun", "Flak" , "Pistol" , "Sniper" , "Shock",
	"Explode" , "BrainBlast" , "RadCannon" , "Chrono" , "TerroristExplode" ,
	"Cow" , "Initiate" , "Virus" , "YuriPrime" , "Guardian"

} };

//DEFINE_HOOK(747BBD, UnitTypeClass_LoadFromINI, 6)
//{
//	GET(UnitTypeClass*, pItem, ESI);
//	GET(CCINIClass* const, pINI, EBX);
//	GET(const char* const, pSection, EBP);
//
//	pItem->AlphaImage = R->EAX<SHPStruct*>();
//	auto pExt = TechnoTypeExt::ExtMap.Find(pItem);
//
//	{
//		pExt->WeaponUINameX.reserve(pItem->WeaponCount);
//		pExt->LoadFromINI(pINI);
//
//		if (pItem->Gunner)
//		{
//			INI_EX exINI(pINI);
//			char Buffer[0x100];
//			char BufferB[0x100];
//
//			for (int i = 0; i < pItem->WeaponCount; ++i)
//			{
//				Valueable<CSFText> nDummy { };
//				IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "WeaponUIName%d", i + 1);
//				nDummy.Read(exINI, pSection, Buffer);
//				pExt->WeaponUINameX.push_back(nDummy.Get());
//			}
//
//			for (int b = 0; b < pItem->TurretCount; ++b)
//			{
//				IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "WeaponTurretIndex%d", b + 1);
//				Nullable<int> nReader {};
//				nReader.Read(exINI, pSection, Buffer);
//
//				if (!nReader.isset())
//					break;
//
//				auto const nResult = std::clamp(nReader.Get(), 0, pItem->TurretCount);
//				pItem->SetTurretWeapon(nResult, b);
//			}
//
//			if (pExt->AdditionalTurrentWeapon.empty())
//			{
//				/*return 0x747BD7;*/
//
//				for (auto const& nData : SubName)
//				{
//					IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sTurretIndex", nData);
//					auto nTurIdx = pINI->ReadInteger(pSection, Buffer, 0);
//					IMPL_SNPRNINTF(BufferB, sizeof(BufferB), "%sTurretWeapon", nData);
//					auto nTurWeaponIdx = pINI->ReadInteger(pSection, BufferB, 0);
//					//Debug::Log("%s Setting [%s = %d ] and [%s = %d ] !\n", pSection, Buffer, nTurIdx, BufferB, nTurWeaponIdx);
//					pItem->SetTurretWeapon(nTurIdx, nTurWeaponIdx);
//				}
//			}
//		}
//
//		return 0x747E90;
//	}
//
//	return pItem->Gunner ? 0x747BD7 : 0x747E90;
//}