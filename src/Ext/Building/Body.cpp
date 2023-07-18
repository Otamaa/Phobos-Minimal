#include "Body.h"
#include <Utilities/EnumFunctions.h>

#include <Utilities/Macro.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

#include <New/Entity/FlyingStrings.h>

void BuildingExt::ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse)
{
	if (!pAttackerHouse || !pTargetHouse || LimboIDs.empty())
		return;

	if (!EnumFunctions::CanTargetHouse(Affects.Get(), pAttackerHouse, pTargetHouse))
		return;

	for (const auto& pBuilding : pTargetHouse->Buildings)
	{
		if (!Is_Building(pBuilding)) // this somehow reading out of bound array ,..
			break;

		const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

		if (pBuildingExt->LimboID <= -1 || !LimboIDs.Contains(pBuildingExt->LimboID))
			continue;

		BuildingExt::LimboKill(pBuilding);
	}
}

int BuildingExt::GetFirstSuperWeaponIndex(BuildingClass* pThis)
{
	const auto pExt = BuildingTypeExt::ExtMap.TryFind(pThis->Type);

	if (!pExt)
		return -1;

	for (auto i = 0; i < pExt->GetSuperWeaponCount(); ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);
		if (idxSW != -1)
		{
			return idxSW;
		}
	}

	return -1;
}

SuperClass* BuildingExt::GetFirstSuperWeapon(BuildingClass* pThis)
{
	const auto idxSW = GetFirstSuperWeaponIndex(pThis);
	return pThis->Owner->Supers.GetItemOrDefault(idxSW);
}

void BuildingExt::ExtData::DisplayIncomeString()
{
	if (Unsorted::CurrentFrame % 15 == 0)
	{
		FlyingStrings::AddMoneyString(
			this->AccumulatedIncome,
			this->AccumulatedIncome,
			this->Get(), AffectedHouse::All,
			this->Get()->GetRenderCoords(),
			this->Type->Refinery_DisplayRefund_Offset
		);

		this->AccumulatedIncome = 0;
	}
}

void BuildingExt::ExtData::UpdatePoweredKillSpawns()
{
	auto const pThis = this->Get();

	if (this->Type->Type->Powered_KillSpawns &&
		pThis->Type->Powered &&
		!pThis->IsPowerOnline())
	{
		if (const auto& pManager = pThis->SpawnManager)
		{
			pManager->ResetTarget();

			for (const auto& pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
				{
					if (pItem->Unit)
						pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
							RulesClass::Instance()->C4Warhead, nullptr, true, true, nullptr);
				}
			}
		}
	}
}

void BuildingExt::ExtData::UpdateAutoSellTimer()
{
	auto const pThis = this->Get();
	auto const nMission = pThis->GetCurrentMission();

	if (pThis->InLimbo || !pThis->IsOnMap || this->LimboID != -1 || nMission == Mission::Selling)
		return;

	if (!pThis->Type->Unsellable && pThis->Type->TechLevel != -1)
	{

		auto const pRulesExt = RulesExt::Global();

		if (this->Type->AutoSellTime.isset() && std::abs(this->Type->AutoSellTime.Get()) > 0.00f)
		{

			if (!AutoSellTimer.HasStarted())
				AutoSellTimer.Start(static_cast<int>(this->Type->AutoSellTime.Get() * 900.0));
			else
			{
				if (AutoSellTimer.Completed())
				{
					pThis->Sell(-1);
				}
			}
		}

		if (!pRulesExt->AI_AutoSellHealthRatio.empty() && pRulesExt->AI_AutoSellHealthRatio.size() >= 3)
		{
			if (!pThis->Owner || pThis->Occupants.Count || pThis->Owner->Type->MultiplayPassive)
				return;

			if (!pThis->Owner->IsCurrentPlayer())
				return;

			const double nValue = pRulesExt->AI_AutoSellHealthRatio[pThis->Owner->GetCorrectAIDifficultyIndex()];

			if (nValue > 0.0 && pThis->GetHealthPercentage() <= nValue)
				pThis->Sell(-1);
		}
	}
}

bool BuildingExt::ExtData::RubbleYell(bool beingRepaired)
{
	auto CreateBuilding = [](BuildingClass* pBuilding, bool remove,
		BuildingTypeClass* pNewType, OwnerHouseKind owner, int strength,
		AnimTypeClass* pAnimType, const char* pTagName) -> bool
	{
		if (!pNewType && !remove)
		{
			Debug::Log("Warning! Advanced Rubble was supposed to be reconstructed but"
				" Ares could not obtain its new BuildingType. Check if [%s]Rubble.%s is"
				" set (correctly).\n", pBuilding->Type->ID, pTagName);
			return true;
		}

		pBuilding->Limbo(); // only takes it off the map
		pBuilding->DestroyNthAnim(BuildingAnimSlot::All);
		auto pOwner = HouseExt::GetHouseKind(owner, true, pBuilding->Owner);

		if (!remove)
		{
			auto pNew = static_cast<BuildingClass*>(pNewType->CreateObject(pOwner));

			if (strength <= -1 && strength >= -100)
			{
				// percentage of original health
				const auto nDecided = ((-strength * pNew->Type->Strength) / 100);
				pNew->Health = MaxImpl(nDecided, 1);
			}
			else if (strength > 0)
			{
				pNew->Health = MinImpl(strength, pNew->Type->Strength);
			} /* else Health = Strength*/

			// The building is created?
			if (!pNew->Unlimbo(pBuilding->Location, pBuilding->PrimaryFacing.Current().GetDir()))
			{
				Debug::Log("Advanced Rubble: Failed to place normal state on map!\n");
				GameDelete<true, false>(pNew);
				return false;
			}
		}

		if (pAnimType)
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, pBuilding->GetCoords()))
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
		}

		return true;
	};

	auto currentBuilding = Get();
	auto pTypeData = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	if (beingRepaired)
	{
		return CreateBuilding(currentBuilding, pTypeData->RubbleIntactRemove,
			pTypeData->RubbleIntact, pTypeData->RubbleIntactOwner,
			pTypeData->RubbleIntactStrength, pTypeData->RubbleIntactAnim, "Intact");
	}
	else
	{
		return CreateBuilding(currentBuilding, pTypeData->RubbleDestroyedRemove,
			pTypeData->RubbleDestroyed, pTypeData->RubbleDestroyedOwner,
			pTypeData->RubbleDestroyedStrength, pTypeData->RubbleDestroyedAnim,
			"Destroyed");
	}
}

bool BuildingExt::HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse)
{
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	const auto pVictimHouse = pBuilding->Owner;

	if (pInfiltratorHouse->IsAlliedWith(pVictimHouse))
		return true;


	return true;
}

bool BuildingExt::ExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->Get();
	for (auto i = 0; i < this->Type->GetSuperWeaponCount(); ++i) {
		if (this->Type->GetSuperWeaponIndex(i, pThis->Owner) == index) {
			return true;
		}
	}

	if (withUpgrades) {
		for (auto const& pUpgrade : pThis->Upgrades) {
			if(const auto pUpgradeExt = BuildingTypeExt::ExtMap.TryFind(pUpgrade)){
				for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i) {
					if (pUpgradeExt->GetSuperWeaponIndex(i, pThis->Owner) == index) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

CoordStruct BuildingExt::GetCenterCoords(BuildingClass* pBuilding, bool includeBib)
{
	CoordStruct ret = pBuilding->GetCoords();
	ret.X += pBuilding->Type->GetFoundationWidth() / 2;
	ret.Y += pBuilding->Type->GetFoundationHeight(includeBib) / 2;
	return ret;
}

void BuildingExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(CurrentAirFactory, ptr);
}

bool BuildingExt::ExtData::InvalidateIgnorable(void* ptr) const
{
	switch (VTable::Get(ptr))
	{
	case BuildingClass::vtable:
		return false;
	}

	return true;
}

void BuildingExt::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array->GetItem(idxStorageTiberiumType);
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array->GetItem(idxTiberiumType);

	if (amount > 0.0)
	{
		if (auto pBuildingType = pThis->Type)
		{
			if (BuildingTypeExt::ExtMap.Find(pBuildingType)->Refinery_UseStorage)
			{
				// Store Tiberium in structures
				depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
				pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
			}
		}
	}
}

void BuildingExt::UpdatePrimaryFactoryAI(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto BuildingExt = BuildingExt::ExtMap.Find(pThis);
	auto HouseExt = HouseExt::ExtMap.Find(pOwner);

	if (!BuildingExt || !HouseExt)
		return;

	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->GetItem(pOwner->ProducingAircraftTypeIndex);
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (BuildingExt->CurrentAirFactory)
	{
		int nDocks = 0;
		if (BuildingExt->CurrentAirFactory->Type)
			nDocks = BuildingExt->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = CountOccupiedDocks(BuildingExt->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = BuildingExt->CurrentAirFactory->Factory;
		else
			BuildingExt->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	std::for_each(
		pOwner->Buildings.begin(),
		pOwner->Buildings.end(),
		[&](BuildingClass* pBuilding)
 {

	 if (!pBuilding || !pBuilding->Type)
		 return;

	 if (pBuilding->Type->Factory == AbstractType::AircraftType &&
		 Phobos::Config::ForbidParallelAIQueues_Aircraft)
	 {

		 if (!currFactory && pBuilding->Factory)
			 currFactory = pBuilding->Factory;

		 if (!std::any_of(HouseExt->HouseAirFactory.begin(),
			 HouseExt->HouseAirFactory.end(),
			 [pBuilding](auto const pData)
			 { return pData == pBuilding; }))
		 {
			 HouseExt->HouseAirFactory.push_back(pBuilding);
		 }
	 }
		});

	if (BuildingExt->CurrentAirFactory)
	{
		std::for_each(
			HouseExt->HouseAirFactory.begin(),
			HouseExt->HouseAirFactory.end(),
			[&](BuildingClass* pBuilding)
 {

	 if (!pBuilding || !pBuilding->Type)
		 return;

	 if (pBuilding == BuildingExt->CurrentAirFactory)
	 {
		 BuildingExt->CurrentAirFactory->Factory = currFactory;
		 BuildingExt->CurrentAirFactory->IsPrimaryFactory = true;
	 }
	 else
	 {
		 pBuilding->IsPrimaryFactory = false;

		 if (pBuilding->Factory)
			 pBuilding->Factory->AbandonProduction();
	 }
			});

		return;
	}

	if (!currFactory)
		return;

	std::for_each(
		HouseExt->HouseAirFactory.begin(),
		HouseExt->HouseAirFactory.end(),
		[&](BuildingClass* pBuilding)
 {

	 if (!pBuilding || !pBuilding->Type)
		 return;

	 int nDocks = pBuilding->Type->NumberOfDocks;
	 int nOccupiedDocks = CountOccupiedDocks(pBuilding);

	 if (nOccupiedDocks < nDocks)
	 {
		 if (!newBuilding)
		 {
			 newBuilding = pBuilding;
			 newBuilding->Factory = currFactory;
			 newBuilding->IsPrimaryFactory = true;
			 BuildingExt->CurrentAirFactory = newBuilding;

			 return;
		 }
	 }

	 pBuilding->IsPrimaryFactory = false;

	 if (pBuilding->Factory)
		 pBuilding->Factory->AbandonProduction();

		});

	return;
}

int BuildingExt::CountOccupiedDocks(BuildingClass* pBuilding)
{
	int nOccupiedDocks = 0;

	if (!pBuilding || !pBuilding->Type)
		return 0;

	if (pBuilding->RadioLinks.IsAllocated && pBuilding->RadioLinks.IsInitialized)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->RadioLinks[i])
			{
				if (Is_Aircraft(pLink))
				{
					nOccupiedDocks++;
				}
			}
		}
	}

	return nOccupiedDocks;
}

bool BuildingExt::HasFreeDocks(BuildingClass* pBuilding)
{
	return BuildingExt::CountOccupiedDocks(pBuilding) < pBuilding->Type->NumberOfDocks;
}

bool BuildingExt::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	if (!Is_Infantry(pTechno) && !Is_Unit(pTechno))
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb)
		&& (Is_Infantry(pTechno) && !pBuilding->Type->InfantryAbsorb ||
			Is_Unit(pTechno) && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner.Get())
			return false;

		if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies.Get())
			return false;

		auto const pTechnoType = pTechno->GetTechnoType();

		if (!pExt->Grinding_AllowTypes.empty() && !pExt->Grinding_AllowTypes.Contains(pTechnoType))
			return false;

		if (!pExt->Grinding_DisallowTypes.empty() && pExt->Grinding_DisallowTypes.Contains(pTechnoType))
			return false;
	}

	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int nRefundAmounts)
{
	const auto pExt = BuildingExt::ExtMap.Find(pBuilding);
	const auto pTypeExt = pExt->Type;

	{
		if (!pTechno)
			return false;

		if (nRefundAmounts
			&& pTypeExt->Grinding_DisplayRefund
			&& EnumFunctions::CanTargetHouse(pTypeExt->Grinding_DisplayRefund_Houses, pBuilding->Owner, HouseClass::CurrentPlayer))
		{
			pExt->AccumulatedGrindingRefund += nRefundAmounts;
		}

		if (pTypeExt->Grinding_Weapon.isset()
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon.Get()->ROF)
		{
			TechnoExt::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon.Get());
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
		}

		if (pTypeExt->Grinding_Sound.isset())
		{
			VocClass::PlayIndexAtPos(pTypeExt->Grinding_Sound.Get(), pTechno->GetCoords());
			return true;
		}
	}

	return false;
}

void BuildingExt::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);

	// BuildLimit check goes before creation
	if (pType->BuildLimit > 0)
	{
		int sum = pOwner->CountOwnedNow(pType);

		// copy Ares' deployable units x build limit fix
		if (auto const pUndeploy = pType->UndeploysInto)
			sum += pOwner->CountOwnedNow(pUndeploy);

		if (sum >= pType->BuildLimit)
			return;
	}

	BuildingClass* pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner));
	if (!pBuilding)
	{
		Debug::Log("Fail to Create Limbo Object[%s] ! \n", pType->get_ID());
		return;
	}

	// All of these are mandatory
	pBuilding->InLimbo = false;
	pBuilding->IsAlive = true;
	pBuilding->IsOnMap = true;

	//const auto pCell = MapClass::Instance->TryGetCellAt(pBuilding->Location);

	// For reasons beyond my comprehension, the discovery logic is checked for certain logics like power drain/output in singleplayer only.
	// This code replicates how DiscoveredBy() is called in BuildingClass::Unlimbo() - Starkku
	//if (!pBuilding->DiscoveredByCurrentPlayer && (pCell && (pCell->AltFlags & AltCellFlags::NoFog) != AltCellFlags(0)) || SessionClass::Instance->GameMode == GameMode::Campaign)
	//	pBuilding->DiscoveredBy(HouseClass::CurrentPlayer);

	//if (!pOwner->IsControlledByHuman())
	//	pBuilding->DiscoveredBy(pOwner);

	pOwner->RegisterGain(pBuilding, false);
	pOwner->UpdatePower();
	pOwner->RecheckTechTree = true;
	pOwner->RecheckPower = true;
	pOwner->RecheckRadar = true;
	pOwner->Buildings.AddItem(pBuilding);
	pOwner->ActiveBuildingTypes.Increment(pBuilding->Type->ArrayIndex);

	// Different types of building logics
	if (pType->ConstructionYard)
		pOwner->ConYards.AddItem(pBuilding); // why would you do that????

	if (pType->SecretLab)
		pOwner->SecretLabs.AddItem(pBuilding);

	if (pType->FactoryPlant)
	{
		pOwner->FactoryPlants.AddItem(pBuilding);
		pOwner->CalculateCostMultipliers();
	}

	if (pType->OrePurifier)
		pOwner->NumOrePurifiers++;

	if (!pOwner->Type->MultiplayPassive)
	{
		if (auto const pInfantrySelfHeal = pType->InfantryGainSelfHeal)
			pOwner->InfantrySelfHeal += pInfantrySelfHeal;

		if (auto const pUnitSelfHeal = pType->UnitsGainSelfHeal)
			pOwner->UnitsSelfHeal += pUnitSelfHeal;
	}

	// BuildingClass::Place is where Ares hooks secret lab expansion
	// pTechnoBuilding->Place(false);
	// even with it no bueno yet, plus new issues
	// probably should just port it from Ares 0.A and be done

	pOwner->UpdateSuperWeaponsUnavailable();

	// LimboKill init
	if (ID != -1)
	{
		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

		pBuildingExt->LimboID = ID;
		pBuildingExt->TechnoExt->Shield.release();
		pBuildingExt->TechnoExt->Trails.clear();
		pBuildingExt->TechnoExt->RevengeWeapons.clear();
		pBuildingExt->TechnoExt->DamageSelfState.release();
		pBuildingExt->TechnoExt->MyGiftBox.release();
		pBuildingExt->TechnoExt->PaintBallState.release();
		pBuildingExt->TechnoExt->ExtraWeaponTimers.clear();
		pBuildingExt->TechnoExt->MyWeaponManager.Clear();
		pBuildingExt->TechnoExt->MyWeaponManager.CWeaponManager.Clear();

		if (!pOwnerExt->AutoDeathObjects.contains(pBuilding))
		{
			KillMethod nMethod = pBuildingExt->Type->Type->Death_Method.Get();

			if (nMethod != KillMethod::None
				&& pBuildingExt->Type->Type->Death_Countdown > 0
				&& !pBuildingExt->TechnoExt->Death_Countdown.HasStarted())
			{
				pBuildingExt->TechnoExt->Death_Countdown.Start(pBuildingExt->Type->Type->Death_Countdown);
				pOwnerExt->AutoDeathObjects.empalace_unchecked(pBuilding, nMethod);
			}
		}
	}
}

void BuildingExt::LimboKill(BuildingClass* pBuilding)
{
	if (!pBuilding->IsAlive)
		return;

	Debug::Log("BuildingExt::LimboKill -  Killing Building[%x - %s] ! \n", pBuilding, pBuilding->get_ID());

	auto const pType = pBuilding->Type;
	auto const pTargetHouse = pBuilding->Owner;

	// Mandatory
	pBuilding->InLimbo = true;
	pBuilding->IsAlive = false;
	pBuilding->IsOnMap = false;
	pTargetHouse->UpdatePower();

	if (!pTargetHouse->RecheckTechTree)
		pTargetHouse->RecheckTechTree = true;

	pTargetHouse->RecheckPower = true;
	pTargetHouse->RecheckRadar = true;
	pTargetHouse->Buildings.Remove(pBuilding);

	pTargetHouse->RegisterLoss(pBuilding, false);
	pTargetHouse->RemoveTracking(pBuilding);

	pTargetHouse->ActiveBuildingTypes.Decrement(pBuilding->Type->ArrayIndex);

	// Building logics
	if (pType->ConstructionYard)
		pTargetHouse->ConYards.Remove(pBuilding);

	if (pType->SecretLab)
		pTargetHouse->SecretLabs.Remove(pBuilding);

	if (pType->FactoryPlant)
	{
		pTargetHouse->FactoryPlants.Remove(pBuilding);
		pTargetHouse->CalculateCostMultipliers();
	}

	if (pType->OrePurifier)
		pTargetHouse->NumOrePurifiers--;

	if (auto const pInfantrySelfHeal = pType->InfantryGainSelfHeal)
	{
		pTargetHouse->InfantrySelfHeal -= pInfantrySelfHeal;
		if (pTargetHouse->InfantrySelfHeal < 0)
			pTargetHouse->InfantrySelfHeal = 0;
	}

	if (auto const pUnitSelfHeal = pType->UnitsGainSelfHeal)
	{
		pTargetHouse->UnitsSelfHeal -= pUnitSelfHeal;
		if (pTargetHouse->UnitsSelfHeal < 0)
			pTargetHouse->UnitsSelfHeal = 0;
	}

	pTargetHouse->UpdateSuperWeaponsUnavailable();

	// Remove completely
	TechnoExt::HandleRemove(pBuilding, nullptr, true, true);
}

// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->OwnerObject()->LightSource)
		.Process(this->Type)
		.Process(this->TechnoExt)
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedGrindingRefund)
		.Process(this->AccumulatedIncome)
		.Process(this->IsCreatedFromMapFile)
		.Process(this->DamageFireAnims)
		.Process(this->AutoSellTimer)
		.Process(this->LighningNeedUpdate)
		.Process(this->TogglePower_HasPower)
		.Process(this->C4Damage)
		.Process(this->C4Owner)
		.Process(this->C4Warhead)
		.Process(this->Silent)
		.Process(this->ReceiveDamageWarhead)
		.Process(this->DockReloadTimers)
		.Process(this->OwnerBeforeRaid)
		.Process(this->CashUpgradeTimers)
		.Process(this->SensorArrayActiveCounter)
		;
}

// =============================
// container
BuildingExt::ExtContainer BuildingExt::ExtMap;
BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }
BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	if (auto pExt = BuildingExt::ExtMap.Allocate(pItem))
	{
		pExt->TechnoExt = TechnoExt::ExtMap.Find(pItem);

		auto const pTypeExt = BuildingTypeExt::ExtMap.TryFind(pItem->Type);
		pExt->Type = pTypeExt;

		if (pTypeExt && !pTypeExt->DamageFire_Offs.empty()) {
			pExt->DamageFireAnims.resize(pTypeExt->DamageFire_Offs.size());
		}
	}

	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x45417E, BuildingClass_Load_Suffix, 0x5)
{
	BuildingExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x454244, BuildingClass_Save_Suffix, 0x7)
{
	BuildingExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x44E940, BuildingClass_Detach, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(void*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	BuildingExt::ExtMap.InvalidatePointerFor(pThis, target, all);

	return pThis->LightSource == target ? 0x44E948 : 0x44E94E;
}