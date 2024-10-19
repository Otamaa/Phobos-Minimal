#include "Body.h"
#include <Utilities/EnumFunctions.h>

#include <Utilities/Macro.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

#include <New/Entity/FlyingStrings.h>

void BuildingExtData::InitializeConstant()
{
	this->PrismForwarding.Owner = this->AttachedToObject;

	this->TechnoExt = TechnoExtContainer::Instance.Find(this->AttachedToObject);
	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);
	this->Type = pTypeExt;

	if (pTypeExt && !pTypeExt->DamageFire_Offs.empty())
	{
		this->DamageFireAnims.resize(pTypeExt->DamageFire_Offs.size());
	}

	this->StartupCashDelivered.resize(HouseClass::Array->Count);
}

const std::vector<CellStruct> BuildingExtData::GetFoundationCells(BuildingClass* const pThis, CellStruct const baseCoords, bool includeOccupyHeight)
{
	auto const pFoundation = pThis->GetFoundationData(false);

	int occupyHeight = includeOccupyHeight ? pThis->Type->OccupyHeight : 1;

	if (occupyHeight <= 0)
		occupyHeight = 1;

	auto pCellIterator = pFoundation;

	while ((*pCellIterator).DifferTo(CellStruct::EOL))
		++pCellIterator;

	HelperedVector<CellStruct> foundationCells;
	foundationCells.reserve(static_cast<int>(std::distance(pFoundation, pCellIterator + 1)) * occupyHeight);
	pCellIterator = pFoundation;

	while ((*pCellIterator).DifferTo(CellStruct::EOL))
	{
		auto actualCell = baseCoords + *pCellIterator;

		for (auto i = occupyHeight; i > 0; --i)
		{
			foundationCells.push_back(actualCell);
			--actualCell.X;
			--actualCell.Y;
		}
		++pCellIterator;
	}

	foundationCells.remove_all_duplicates([](const CellStruct& lhs, const CellStruct& rhs) -> bool {
		return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});

	return foundationCells;
}

// Assigns a secret production option to the building.
void BuildingExtData::UpdateSecretLab(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (!pOwner || pOwner->Type->MultiplayPassive) {
		return;
	}

	auto pExt = BuildingExtContainer::Instance.Find(pThis);
	auto pType = pThis->Type;

	// fixed item, no need to randomize
	if (pType->SecretInfantry || pType->SecretUnit || pType->SecretBuilding)
	{
		Debug::Log("[Secret Lab] %s has a fixed boon.\n", pType->ID);
		return;
	}

	auto pData = BuildingTypeExtContainer::Instance.Find(pType);

	// go on if not placed or always recalculate on capture
	if (pExt->SecretLab_Placed && !pData->Secret_RecalcOnCapture) {
		return;
	}

	std::vector<TechnoTypeClass*> Options;
	const DWORD OwnerBits = 1u << pOwner->Type->ArrayIndex;
	;
	auto AddToOptions = [OwnerBits , pOwner, &Options](const Iterator<TechnoTypeClass*>& items)
		{
			for (const auto& Option : items)
			{
				const auto pExt = TechnoTypeExtContainer::Instance.Find(Option);

				if ((pExt->Secret_RequiredHouses & OwnerBits) && !(pExt->Secret_ForbiddenHouses & OwnerBits))
				{
					switch (HouseExtData::RequirementsMet(pOwner, Option))
					{
					case RequirementStatus::Forbidden:
					case RequirementStatus::Incomplete:
						Options.emplace_back(Option);
						break;
					default:
						break;
					}
				}
			}
		};

	// generate a list of items
	if (pData->Secret_Boons.HasValue())
	{
		AddToOptions(pData->Secret_Boons);
	}
	else
	{
		AddToOptions(make_iterator(RulesClass::Instance->SecretInfantry));
		AddToOptions(make_iterator(RulesClass::Instance->SecretUnits));
		AddToOptions(make_iterator(RulesClass::Instance->SecretBuildings));
	}

	// pick one of all eligible items
	if (!Options.empty())
	{
		const auto Result = Options[ScenarioClass::Instance->Random.RandomFromMax(Options.size() - 1)];
		Debug::Log("[Secret Lab] rolled %s for %s\n", Result->ID, pType->ID);
		pThis->SecretProduction = Result;
		pExt->SecretLab_Placed = true;
	}
	else
	{
		Debug::Log("[Secret Lab] %s has no boons applicable to country [%s]!\n",
			pType->ID, pOwner->Type->ID);
	}
}

bool BuildingExtData::ReverseEngineer(BuildingClass* pBuilding, TechnoClass* Victim)
{
	const auto pReverseData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
	if (!pReverseData->ReverseEngineersVictims || !pBuilding->Owner) {
		return false;
	}

	auto VictimType = Victim->GetTechnoType();
	auto pVictimData = TechnoTypeExtContainer::Instance.Find(VictimType);
	auto VictimAs = pVictimData->ReversedAs.Get(VictimType);

	if (!pVictimData->CanBeReversed || !VictimAs)
		return false;

	const auto pBldOwner = pBuilding->Owner;
	auto pBldOwnerExt = HouseExtContainer::Instance.Find(pBldOwner);

	if (!pBldOwnerExt->Reversed.contains(VictimAs))
	{
		const bool WasBuildable =
			HouseExtData::PrereqValidate(pBldOwner, VictimType, false, true) == CanBuildResult::Buildable;

		pBldOwnerExt->Reversed.push_back(VictimAs);

		if (!WasBuildable) {

			if (HouseExtData::RequirementsMet(pBldOwner, VictimType) != RequirementStatus::Forbidden)
			{
				pBldOwner->RecheckTechTree = true;
				return true;
			}
		}
	}

	return false;
}

void BuildingExtData::ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse)
{
	if (!pAttackerHouse || !pTargetHouse || LimboIDs.empty())
		return;

	if (!EnumFunctions::CanTargetHouse(Affects.Get(), pAttackerHouse, pTargetHouse))
		return;

	HelperedVector<BuildingClass*> LimboedID {};
	for (const auto& pBuilding : pTargetHouse->Buildings) {
		const auto pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		if (pBuildingExt->LimboID <= -1 || !LimboIDs.Contains(pBuildingExt->LimboID))
			continue;

		LimboedID.push_back(pBuilding);
	}

	for(auto& pLimboBld : LimboedID){
		BuildingExtData::LimboKill(pLimboBld);
	}
}

int BuildingExtData::GetFirstSuperWeaponIndex(BuildingClass* pThis)
{
	if (const auto pExt = BuildingTypeExtContainer::Instance.TryFind(pThis->Type))
	{
		for (auto i = 0; i < pExt->GetSuperWeaponCount(); ++i)
		{
			const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);
			if (idxSW != -1)
			{
				return idxSW;
			}
		}
	}

	return -1;
}

SuperClass* BuildingExtData::GetFirstSuperWeapon(BuildingClass* pThis)
{
	const size_t idx = GetFirstSuperWeaponIndex(pThis);

	if (idx < (size_t)pThis->Owner->Supers.Count) {
		return pThis->Owner->Supers[idx];
	}

	return nullptr;
}

void BuildingExtData::DisplayIncomeString()
{
	if (this->Type->DisplayIncome.Get(RulesExtData::Instance()->DisplayIncome) &&
		this->AccumulatedIncome && Unsorted::CurrentFrame % 15 == 0)
	{
		if(!RulesExtData::Instance()->DisplayIncome_AllowAI && !this->AttachedToObject->Owner->IsControlledByHuman())
		{
			this->AccumulatedIncome = 0;
			return;
		}

		FlyingStrings::AddMoneyString(
			this->AccumulatedIncome,
			this->AccumulatedIncome,
			this->AttachedToObject,
			this->Type->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses),
			this->AttachedToObject->GetRenderCoords(),
			this->Type->DisplayIncome_Offset
		);

		this->AccumulatedIncome = 0;
	}
}

void BuildingExtData::UpdatePoweredKillSpawns() const
{
	auto const pThis = this->AttachedToObject;

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
					if (pItem->Unit && pItem->Unit->IsAlive)
						pItem->Unit->ReceiveDamage(&pItem->Unit->GetType()->Strength, 0,
							RulesClass::Instance()->C4Warhead, nullptr, true, true, nullptr);
				}
			}
		}
	}
}

void BuildingExtData::UpdateSpyEffecAnimDisplay()
{
	auto const pThis = this->AttachedToObject;
	auto const nMission = pThis->GetCurrentMission();

	if (pThis->InLimbo || !pThis->IsOnMap || this->LimboID != -1 || nMission == Mission::Selling)
		return;

	if (this->SpyEffectAnim)
	{
		if (HouseClass::IsCurrentPlayerObserver() ||  EnumFunctions::CanTargetHouse(
			this->Type->SpyEffect_Anim_DisplayHouses,
			SpyEffectAnim->Owner, HouseClass::CurrentPlayer))
		{
			SpyEffectAnim->Invisible = false;
		} else {
			SpyEffectAnim->Invisible = true;
		}

		if (SpyEffectAnimDuration > 0)
			SpyEffectAnimDuration--;
		else if (SpyEffectAnimDuration == 0)
		{
			SpyEffectAnim.release();
		}
	}
}

void BuildingExtData::UpdateAutoSellTimer()
{
	auto const pThis = this->AttachedToObject;
	auto const nMission = pThis->GetCurrentMission();

	if (pThis->InLimbo || !pThis->IsOnMap || this->LimboID != -1 || nMission == Mission::Selling)
		return;

	if (!pThis->Type->Unsellable && pThis->Type->TechLevel != -1)
	{
		auto const pRulesExt = RulesExtData::Instance();

		if (this->Type->AutoSellTime.isset() && Math::abs(this->Type->AutoSellTime.Get()) > 0.00f)
		{
			if (!AutoSellTimer.HasStarted())
				AutoSellTimer.Start(static_cast<int>(this->Type->AutoSellTime.Get() * 900.0));
		}

		if (pRulesExt->AI_AutoSellHealthRatio.isset())
		{
			if (!pThis->Owner || pThis->Occupants.Count || pThis->Owner->Type->MultiplayPassive)
				return;

			if (pThis->Owner->IsControlledByHuman())
				return;

			const double nValue = pRulesExt->AI_AutoSellHealthRatio->at(pThis->Owner->GetCorrectAIDifficultyIndex());

			if (nValue > 0.0 && pThis->GetHealthPercentage() <= nValue){
				pThis->Sell(-1);
				return;
			}
		}

		if (AutoSellTimer.Completed()) {
			pThis->Sell(-1);
		}
	}
}

bool BuildingExtData::RubbleYell(bool beingRepaired) const
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
		auto pOwner = HouseExtData::GetHouseKind(owner, true, pBuilding->Owner);

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
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pBuilding->GetCoords()),
				pOwner,
				nullptr,
				false
			);
		}

		return true;
	};

	auto currentBuilding = AttachedToObject;
	auto pTypeData = BuildingTypeExtContainer::Instance.Find(currentBuilding->Type);
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

//unused ?
bool BuildingExtData::HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse)
{
	if (!BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->SpyEffect_Custom)
		return false;

	if (pInfiltratorHouse->IsAlliedWith(pBuilding->Owner))
		return true;

	return true;
}

bool BuildingExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->AttachedToObject;

	for (auto i = 0; i < this->Type->GetSuperWeaponCount(); ++i) {
		if (this->Type->GetSuperWeaponIndex(i, pThis->Owner) == index) {
			return true;
		}
	}

	if (withUpgrades) {
		for (auto const& pUpgrade : pThis->Upgrades) {
			if(const auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(pUpgrade)){
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

CoordStruct BuildingExtData::GetCenterCoords(BuildingClass* pBuilding, bool includeBib)
{
	CoordStruct ret = pBuilding->GetCoords();
	ret.X += pBuilding->Type->GetFoundationWidth() / 2;
	ret.Y += pBuilding->Type->GetFoundationHeight(includeBib) / 2;
	return ret;
}

void BuildingExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(this->CurrentAirFactory, ptr , bRemoved);
	AnnounceInvalidPointer<TechnoClass*>(this->RegisteredJammers, ptr, bRemoved);
	if (bRemoved && ptr == this->SpyEffectAnim.get()) {
		this->SpyEffectAnim.release();
	}

	this->PrismForwarding.InvalidatePointer(ptr, bRemoved);
}

bool BuildingExtData::InvalidateIgnorable(AbstractClass* ptr)
{
	switch (ptr->WhatAmI())
	{
	case BuildingClass::AbsID:
		return false;
	}

	return true;
}

void BuildingExtData::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array->Items[idxStorageTiberiumType];
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array->Items[idxTiberiumType];

	if (amount > 0.0)
	{
		if (auto pBuildingType = pThis->Type)
		{
			if (BuildingTypeExtContainer::Instance.Find(pBuildingType)->Refinery_UseStorage)
			{
				// Store Tiberium in structures
				depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
				pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
			}
		}
	}
}

static std::vector<BuildingClass*> airFactoryBuilding;

void BuildingExtData::UpdatePrimaryFactoryAI(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto BuildingExt = BuildingExtContainer::Instance.Find(pThis);
	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->Items[pOwner->ProducingAircraftTypeIndex];
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	airFactoryBuilding.clear();
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (BuildingExt->CurrentAirFactory)
	{
		int nDocks = 0;
		if (BuildingExt->CurrentAirFactory->Type)
			nDocks = BuildingExt->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = BuildingExtData::CountOccupiedDocks(BuildingExt->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = BuildingExt->CurrentAirFactory->Factory;
		else
			BuildingExt->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto& pBuilding : pOwner->Buildings)
	{
		if
		(
			pBuilding->Type->Factory == AbstractType::AircraftType
			&& pBuilding->IsAlive
			&& pBuilding->Health > 0
			&& !pBuilding->InLimbo
			&& !pBuilding->TemporalTargetingMe
			&& !BuildingExtContainer::Instance.Find(pBuilding)->AboutToChronoshift
			&& pBuilding->GetCurrentMission() != Mission::Selling
			&& pBuilding->QueuedMission != Mission::Selling
		)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.push_back(pBuilding);
		}
	}

	if (BuildingExt->CurrentAirFactory)
	{
		for (auto pBuilding : airFactoryBuilding)
		{
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
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto pBuilding : airFactoryBuilding)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExtData::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
		{
			if (!newBuilding)
			{
				newBuilding = pBuilding;
				newBuilding->Factory = currFactory;
				newBuilding->IsPrimaryFactory = true;
				BuildingExt->CurrentAirFactory = newBuilding;

				continue;
			}
		}

		pBuilding->IsPrimaryFactory = false;

		if (pBuilding->Factory)
			pBuilding->Factory->AbandonProduction();
	}

	return;
}

int BuildingExtData::CountOccupiedDocks(BuildingClass* pBuilding)
{
	int nOccupiedDocks = 0;

	if (pBuilding->RadioLinks.IsAllocated)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->RadioLinks[i])
			{
				nOccupiedDocks++;
			}
		}
	}

	return nOccupiedDocks;
}

bool BuildingExtData::HasFreeDocks(BuildingClass* pBuilding)
{
	return BuildingExtData::CountOccupiedDocks(pBuilding) < pBuilding->Type->NumberOfDocks;
}

bool BuildingExtData::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	const auto what = pTechno->WhatAmI();

	if (what != InfantryClass::AbsID && what != UnitClass::AbsID)
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb)
		&& (what == InfantryClass::AbsID && !pBuilding->Type->InfantryAbsorb ||
			what == UnitClass::AbsID && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	if(pBuilding->Owner->IsAlliedWith(pTechno))
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner.Get())
			return false;

		if (pBuilding->Owner != pTechno->Owner && !pExt->Grinding_AllowAllies.Get())
			return false;

		auto const pTechnoType = pTechno->GetTechnoType();

		if (!pExt->Grinding_AllowTypes.empty() && !pExt->Grinding_AllowTypes.Contains(pTechnoType))
			return false;

		if (!pExt->Grinding_DisallowTypes.empty() && pExt->Grinding_DisallowTypes.Contains(pTechnoType))
			return false;

		return true;
	}

	return false;
}

bool BuildingExtData::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int nRefundAmounts)
{
	const auto pExt = BuildingExtContainer::Instance.Find(pBuilding);
	const auto pTypeExt = pExt->Type;

	{
		if (!pTechno)
			return false;

		pExt->AccumulatedIncome += nRefundAmounts;
		pExt->GrindingWeapon_AccumulatedCredits += nRefundAmounts;

		if (pTypeExt->Grinding_Weapon
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon->ROF
			&& pExt->GrindingWeapon_AccumulatedCredits >= pTypeExt->Grinding_Weapon_RequiredCredits)
		{
			TechnoExtData::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon);
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
			pExt->GrindingWeapon_AccumulatedCredits = 0;
		}

		if (pTypeExt->Grinding_Sound.isset())
		{
			VocClass::PlayIndexAtPos(pTypeExt->Grinding_Sound.Get(), pTechno->GetCoords());
			return true;
		}
	}

	return false;
}

void BuildingExtData::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	// LimboKill init
	if (ID != -1)
	{
		auto const pOwnerExt = HouseExtContainer::Instance.Find(pOwner);

		// BuildLimit check goes before creation
		if (((BuildLimitStatus)HouseExtData::BuildLimitGroupCheck(pOwner, pType, true , false)) != BuildLimitStatus::NotReached && HouseExtData::CheckBuildLimit(pOwner, pType , true) != BuildLimitStatus::NotReached) {
			Debug::Log("Fail to Create Limbo Object[%s] because of BuildLimit ! \n", pType->get_ID());
			return;
		}

		BuildingClass* pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner));
		if (!pBuilding)
		{
			Debug::Log("Fail to Create Limbo Object[%s] ! \n", pType->get_ID());
			return;
		}

		//Debug::Log("[0x%x - %s] Sending [%s] As Limbo Delivered ID [%d]\n", pOwner , pOwner->get_ID(), pType->ID, ID);
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

		// For reasons beyond my comprehension, the discovery logic is checked for certain logics like power drain/output in campaign only.
		// Normally on unlimbo the buildings are revealed to current player if unshrouded or if game is a campaign and to non-player houses always.
		// Because of the unique nature of LimboDelivered buildings, this has been adjusted to always reveal to the current player in singleplayer
		// and to the owner of the building regardless, removing the shroud check from the equation since they don't physically exist - Starkku
		if (SessionClass::IsCampaign())
			pBuilding->DiscoveredBy(HouseClass::CurrentPlayer);

		pBuilding->DiscoveredBy(pOwner);

		pOwnerExt->LimboTechno.insert(pBuilding);
		//pOwner->AddTracking(pBuilding);
		pOwner->RegisterGain(pBuilding, false);
		pOwner->UpdatePower();
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->RecheckRadar = true;
		pOwner->Buildings.AddItem(pBuilding);

		pOwner->ActiveBuildingTypes.Increment(pBuilding->Type->ArrayIndex);

		if (pType->SecretLab)
			pOwner->SecretLabs.AddItem(pBuilding);

		//if (pType->FactoryPlant)
		//{
		//	pOwner->FactoryPlants.AddItem(pBuilding);
		//	pOwner->CalculateCostMultipliers();
		//}

		//if (pType->OrePurifier)
		//	pOwner->NumOrePurifiers++;

		//if (!pOwner->Type->MultiplayPassive)
		//{
		//	if (auto const pInfantrySelfHeal = pType->InfantryGainSelfHeal)
		//		pOwner->InfantrySelfHeal += pInfantrySelfHeal;
		//
		//	if (auto const pUnitSelfHeal = pType->UnitsGainSelfHeal)
		//		pOwner->UnitsSelfHeal += pUnitSelfHeal;
		//}

		pOwner->UpdateSuperWeaponsUnavailable();

		auto const pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		HouseExtData::UpdateFactoryPlans(pBuilding);

		if (BuildingTypeExtContainer::Instance.Find(pType)->Academy)
			HouseExtContainer::Instance.Find(pOwner)->UpdateAcademy(pBuilding, true);

		if (pType->SecretLab)
			BuildingExtData::UpdateSecretLab(pBuilding);

		pBuildingExt->LimboID = ID;
		pBuildingExt->TechnoExt->Shield.release();
		pBuildingExt->TechnoExt->Trails.clear();
		pBuildingExt->TechnoExt->RevengeWeapons.clear();
		pBuildingExt->TechnoExt->DamageSelfState.release();
		pBuildingExt->TechnoExt->MyGiftBox.release();
		pBuildingExt->TechnoExt->PaintBallStates.clear();
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
				pOwnerExt->AutoDeathObjects.emplace_unchecked(pBuilding, nMethod);
			}
		}
	}
}

void BuildingExtData::LimboKill(BuildingClass* pBuilding)
{
	if (!pBuilding->IsAlive)
		return;

	Debug::Log("BuildingExtData::LimboKill -  Killing Building[%x - %s] ! \n", pBuilding, pBuilding->get_ID());

#ifdef SIMPLIFY_CAUSECRASH
	pBuilding->Stun();
	pBuilding->Limbo();
	pBuilding->RegisterDestruction(nullptr);
#else
	auto const pType = pBuilding->Type;
	auto const pTargetHouse = pBuilding->Owner;

	// Mandatory
	pBuilding->InLimbo = true;
	//pBuilding->IsAlive = false;
	pBuilding->IsOnMap = false;
	pTargetHouse->UpdatePower();

	if (!pTargetHouse->RecheckTechTree)
		pTargetHouse->RecheckTechTree = true;

	auto pOwnerExt = HouseExtContainer::Instance.Find(pTargetHouse);

	if (BuildingTypeExtContainer::Instance.Find(pType)->Academy)
		pOwnerExt->UpdateAcademy(pBuilding, false);

	pTargetHouse->RecheckPower = true;
	pTargetHouse->RecheckRadar = true;
	pTargetHouse->Buildings.Remove(pBuilding);
	static_assert(offsetof(HouseClass, Buildings) == 0x68, "ClassMember Shifted !");
	pOwnerExt->LimboTechno.erase(pBuilding);
	pTargetHouse->RegisterLoss(pBuilding, false);
	//pTargetHouse->RemoveTracking(pBuilding);

	pTargetHouse->ActiveBuildingTypes.Decrement(pBuilding->Type->ArrayIndex);

	// Building logics
	if (pType->ConstructionYard)
		pTargetHouse->ConYards.Remove(pBuilding);

	if (pType->SecretLab)
		pTargetHouse->SecretLabs.Remove(pBuilding);

	//if (pType->FactoryPlant)
	//{
	//
	//	pTargetHouse->FactoryPlants.Remove(pBuilding);
	//	pTargetHouse->CalculateCostMultipliers();
	//}

	//if (pType->OrePurifier)
	//	pTargetHouse->NumOrePurifiers--;

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
	pBuilding->Stun();
	pBuilding->Limbo();
#endif

	// Remove completely
	Debug::Log(__FUNCTION__" Called \n");
	TechnoExtData::HandleRemove(pBuilding, nullptr, true, false);
}

// Check whether can call the occupiers leave
bool BuildingExtData::CheckOccupierCanLeave(HouseClass* pBuildingHouse, HouseClass* pOccupierHouse)
{
	if (!pOccupierHouse)
		return false;
	else if (pBuildingHouse == pOccupierHouse)
		return true;
	else if (SessionClass::Instance->GameMode == GameMode::Campaign && pOccupierHouse->IsInPlayerControl)
		return true;
	else if (!pOccupierHouse->IsControlledByHuman() && pOccupierHouse->IsAlliedWith(pBuildingHouse))
		return true;

	return false;
}

#include <Locomotor/Cast.h>

// Force occupiers leave, return: whether it should stop right now
bool BuildingExtData::CleanUpBuildingSpace(BuildingTypeClass* pBuildingType, CellStruct topLeftCell, HouseClass* pHouse, TechnoClass* pExceptTechno)
{
	// Step 1: Find the technos inside of the building place grid.
	CellStruct infantryCount { 0, 0 };
	std::vector<TechnoClass*> checkedTechnos;
	checkedTechnos.reserve(24);
	std::vector<CellClass*> checkedCells;
	checkedCells.reserve(24);

	for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct::EOL; ++pFoundation)
	{
		CellStruct currentCoord = topLeftCell + *pFoundation;

		if (CellClass* const pCell = MapClass::Instance->GetCellAt(currentCoord))
		{
			ObjectClass* pObject = pCell->FirstObject;

			while (pObject)
			{
				AbstractType const absType = pObject->WhatAmI();

				if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
				{
					TechnoClass* const pCellTechno = static_cast<TechnoClass*>(pObject);
					auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pCellTechno->GetTechnoType());

					if ((!pTypeExt || !pTypeExt->CanBeBuiltOn) && pCellTechno != pExceptTechno) // No need to check house
					{
						const FootClass* pFoot = static_cast<FootClass*>(pCellTechno);

						if (pFoot->GetCurrentSpeed() <= 0 || (locomotion_cast<TunnelLocomotionClass*>(pFoot->Locomotor) && !pFoot->Locomotor->Is_Moving()))
						{
							if (absType == AbstractType::Infantry)
								++infantryCount.X;

							checkedTechnos.push_back(pCellTechno);
						}
					}
				}

				pObject = pObject->NextObject;
			}

			checkedCells.push_back(pCell);
		}
	}

	if (checkedTechnos.empty()) // All in moving
		return false;

	// Step 2: Find the cells around the building.
	std::vector<CellClass*> optionalCells;
	optionalCells.reserve(24);

	for (auto pFoundation = pBuildingType->FoundationOutside; *pFoundation != CellStruct::EOL ; ++pFoundation)
	{
		CellStruct searchCell = topLeftCell + *pFoundation;

		if (CellClass* const pSearchCell = MapClass::Instance->GetCellAt(searchCell))
		{
			if (std::find(checkedCells.begin(), checkedCells.end(), pSearchCell) == checkedCells.end() // TODO If there is a cellflag that can be used …
				&& !pSearchCell->GetBuilding()
				&& pSearchCell->IsClearToMove(SpeedType::Amphibious, true, true, ZoneType::None, MovementZone::Amphibious, -1, false))
			{
				optionalCells.push_back(pSearchCell);
			}
		}
	}

	if (optionalCells.empty()) // There is no place for scattering
		return true;

	// Step 3: Sort the technos by the distance out of the foundation.
	std::sort(&checkedTechnos[0], &checkedTechnos[checkedTechnos.size()], [optionalCells](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
	{
		int minA = INT_MAX;
		int minB = INT_MAX;

		for (auto const& pOptionalCell : optionalCells) // If there are many valid cells at start, it means most of occupiers will near to the edge
		{
			if (minA > 65536) // If distance squared is lower or equal to 256^2, then no need to calculate any more because it is on the edge
			{
				int curA = static_cast<int>(pTechnoA->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

				if (curA < minA)
					minA = curA;
			}

			if (minB > 65536)
			{
				int curB = static_cast<int>(pTechnoB->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

				if (curB < minB)
					minB = curB;
			}
		}

		return minA > minB;
	});

	// Step 4: Core, successively find the farthest techno and its closest valid destination.
	std::vector<TechnoClass*> reCheckedTechnos;
	reCheckedTechnos.reserve(12);

	struct InfantryCountInCell // Temporary struct
	{
		CellClass* position;
		int count;
	};
	std::vector<InfantryCountInCell> infantryCells;
	infantryCells.reserve(4);

	struct TechnoWithDestination // Also temporary struct
	{
		TechnoClass* techno;
		CellClass* destination;
	};
	std::vector<TechnoWithDestination> finalOrder;
	finalOrder.reserve(24);

	do
	{
		// Step 4.1: Push the technos discovered just now back to the vector.
		for (auto const& pRecheckedTechno : reCheckedTechnos)
		{
			if (pRecheckedTechno->WhatAmI() == AbstractType::Infantry)
				++infantryCount.X;

			checkedTechnos.push_back(pRecheckedTechno);
		}

		reCheckedTechnos.clear();

		// Step 4.2: Check the techno vector.
		for (auto const& pCheckedTechno : checkedTechnos)
		{
			CellClass* pDestinationCell = nullptr;

			// Step 4.2.1: Search the closest valid cell to be the destination.
			do
			{
				const CellStruct location = pCheckedTechno->GetMapCoords();
				const bool isInfantry = pCheckedTechno->WhatAmI() == AbstractType::Infantry;
				TechnoTypeClass* const pCheckedType = pCheckedTechno->GetTechnoType();

				if (isInfantry) // Try to maximizing cells utilization
				{
					if (infantryCells.size() && infantryCount.Y >= (infantryCount.X / 3 + (infantryCount.X % 3 ? 1 : 0)))
					{
						std::sort(&infantryCells[0], &infantryCells[infantryCells.size()], [location](InfantryCountInCell cellA, InfantryCountInCell cellB)
{
	return cellA.position->MapCoords.DistanceFromSquared(location) < cellB.position->MapCoords.DistanceFromSquared(location);
						});

						for (auto& infantryCell : infantryCells)
						{
							if (static_cast<InfantryClass*>(pCheckedTechno)->Destination == infantryCell.position)
							{
								infantryCell.count = 3;
							}
							else if (infantryCell.count < 3 && infantryCell.position->IsClearToMove(pCheckedType->SpeedType, true, true, ZoneType::None, pCheckedType->MovementZone, -1, false))
							{
								pDestinationCell = infantryCell.position;
								++infantryCell.count;

								break;
							}
						}

						if (pDestinationCell)
							break; // Complete
					}
				}

				std::sort(&optionalCells[0], &optionalCells[optionalCells.size()], [location](CellClass* pCellA, CellClass* pCellB) {
	return			pCellA->MapCoords.DistanceFromSquared(location) < pCellB->MapCoords.DistanceFromSquared(location);
				});

				const double minDistanceSquared = optionalCells[0]->MapCoords.DistanceFromSquared(location);

				for (auto const& pOptionalCell : optionalCells) // Prioritize selecting empty cells
				{
					if (!pOptionalCell->FirstObject && pOptionalCell->IsClearToMove(pCheckedType->SpeedType, true, true, ZoneType::None, pCheckedType->MovementZone, -1, false))
					{
						if (isInfantry) // Not need to remove it now
						{
							infantryCells.push_back(InfantryCountInCell { pOptionalCell, 1 });
							++infantryCount.Y;
						}

						if (pOptionalCell->MapCoords.DistanceFromSquared(location) < (minDistanceSquared * 4)) // Empty cell is not too far
							pDestinationCell = pOptionalCell;

						break;
					}
				}

				if (!pDestinationCell)
				{
					std::vector<CellClass*> deleteCells;
					deleteCells.reserve(8);

					for (auto const& pOptionalCell : optionalCells)
					{
						ObjectClass* pCurObject = pOptionalCell->FirstObject;
						std::vector<TechnoClass*> optionalTechnos;
						optionalTechnos.reserve(4);
						bool valid = true;

						while (pCurObject)
						{
							AbstractType const absType = pCurObject->WhatAmI();

							if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
							{
								TechnoClass* const pCurTechno = static_cast<TechnoClass*>(pCurObject);

								if (!BuildingExtData::CheckOccupierCanLeave(pHouse, pCurTechno->Owner)) // Means invalid for all
								{
									deleteCells.push_back(pOptionalCell);
									valid = false;
									break;
								}

								optionalTechnos.push_back(pCurTechno);
							}

							pCurObject = pCurObject->NextObject;
						}

						if (valid && pOptionalCell->IsClearToMove(pCheckedType->SpeedType, true, true, ZoneType::None, pCheckedType->MovementZone, -1, false))
						{
							for (auto const& pOptionalTechno : optionalTechnos)
								reCheckedTechnos.push_back(pOptionalTechno);

							if (isInfantry) // Not need to remove it now
							{
								infantryCells.push_back(InfantryCountInCell { pOptionalCell, 1 });
								++infantryCount.Y;
							}

							pDestinationCell = pOptionalCell;
							break;
						}
					}

					for (auto const& pDeleteCell : deleteCells) // Mark the invalid cells
					{
						checkedCells.push_back(pDeleteCell);
						optionalCells.erase(std::remove(optionalCells.begin(), optionalCells.end(), pDeleteCell), optionalCells.end());
					}
				}
			}
			while (false);

			// Step 4.2.2: Mark the cell and push back its surrounded cells, then prepare for the command.
			if (pDestinationCell)
			{
				if (std::find(checkedCells.begin(), checkedCells.end(), pDestinationCell) == checkedCells.end())
					checkedCells.push_back(pDestinationCell);

				if (std::find(optionalCells.begin(), optionalCells.end(), pDestinationCell) != optionalCells.end())
				{
					optionalCells.erase(std::remove(optionalCells.begin(), optionalCells.end(), pDestinationCell), optionalCells.end());
					CellStruct searchCell = pDestinationCell->MapCoords - CellStruct { 1, 1 };

					for (int i = 0; i < 4; ++i)
					{
						for (int j = 0; j < 2; ++j)
						{
							if (CellClass* const pSearchCell = MapClass::Instance->GetCellAt(searchCell))
							{
								if (std::find(checkedCells.begin(), checkedCells.end(), pSearchCell) == checkedCells.end()
									&& std::find(optionalCells.begin(), optionalCells.end(), pSearchCell) == optionalCells.end()
									&& !pSearchCell->GetBuilding()
									&& pSearchCell->IsClearToMove(SpeedType::Amphibious, true, true, ZoneType::None , MovementZone::Amphibious, -1, false))
								{
									optionalCells.push_back(pSearchCell);
								}
							}

							if (i % 2)
								searchCell.Y += static_cast<short>((i / 2) ? -1 : 1);
							else
								searchCell.X += static_cast<short>((i / 2) ? -1 : 1);
						}
					}
				}

				const TechnoWithDestination thisOrder { pCheckedTechno, pDestinationCell };
				finalOrder.push_back(thisOrder);
			}
			else // Can not build
			{
				return true;
			}
		}

		checkedTechnos.clear();
	}
	while (reCheckedTechnos.size());

	// Step 5: Confirm command execution.
	for (auto const& pThisOrder : finalOrder)
	{
		TechnoClass* const pCheckedTechno = pThisOrder.techno;
		CellClass* const pDestinationCell = pThisOrder.destination;
		AbstractType const absType = pCheckedTechno->WhatAmI();
		pCheckedTechno->ForceMission(Mission::Guard);

		if (absType == AbstractType::Infantry)
		{
			InfantryClass* const pInfantry = static_cast<InfantryClass*>(pCheckedTechno);

			if (pInfantry->IsDeployed())
				pInfantry->PlayAnim(DoType::Undeploy, true);

			pInfantry->SetDestination(pDestinationCell, false);
			pInfantry->QueueMission(Mission::QMove, false); // To force every three infantries gather together, it should be QMove
		}
		else if (absType == AbstractType::Unit)
		{
			UnitClass* const pUnit = static_cast<UnitClass*>(pCheckedTechno);

			if (pUnit->Deployed)
				pUnit->Undeploy();

			pUnit->SetDestination(pDestinationCell, false);
			pUnit->QueueMission(Mission::Move, false);
		}
	}

	return false;
}

// =============================
// load / save

template <typename T>
void BuildingExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Type, true)
		.Process(this->TechnoExt, true)
		.Process(this->PrismForwarding)
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory, true)
		.Process(this->AccumulatedIncome)
		.Process(this->IsCreatedFromMapFile)
		.Process(this->DamageFireAnims)
		.Process(this->AutoSellTimer)
		.Process(this->LighningNeedUpdate)
		.Process(this->TogglePower_HasPower)
		.Process(this->C4Damage)
		.Process(this->C4Owner, true)
		.Process(this->C4Warhead, true)
		.Process(this->Silent)
		.Process(this->ReceiveDamageWarhead, true)
		.Process(this->DockReloadTimers)
		.Process(this->OwnerBeforeRaid, true)
		.Process(this->CashUpgradeTimers)
		.Process(this->SensorArrayActiveCounter)
		.Process(this->StartupCashDelivered)
		.Process(this->SecretLab_Placed)
		.Process(this->AboutToChronoshift)
		.Process(this->IsFromSW)
		.Process(this->RegisteredJammers, true)
		.Process(this->GrindingWeapon_AccumulatedCredits)
		.Process(this->BeignMCEd)
		.Process(this->LastFlameSpawnFrame)
		.Process(this->SpyEffectAnim, true)
		.Process(this->SpyEffectAnimDuration)
		.Process(this->PoweredUpToLevel)
		;
}

// =============================
// container
BuildingExtContainer BuildingExtContainer::Instance;
std::vector<BuildingExtData*> BuildingExtContainer::Pool;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Allocate(pItem);


	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x453ED4, BuildingClass_Load_Suffix, 0x6)
{
	BuildingExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4541B2, BuildingClass_Save_Suffix, 0x6)
{
	BuildingExtContainer::Instance.SaveStatic();
	return 0;
}

#include <Misc/Hooks.Otamaa.h>

void FakeBuildingClass::_Detach(AbstractClass* target , bool all) {
	BuildingExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->BuildingClass::PointerExpired(target , all);
}

DEFINE_JUMP(VTABLE, 0x7E3EE4, MiscTools::to_DWORD(&FakeBuildingClass::_Detach))