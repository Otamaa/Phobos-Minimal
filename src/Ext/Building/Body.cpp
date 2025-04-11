#include "Body.h"
#include <Utilities/EnumFunctions.h>

#include <Utilities/Macro.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

#include <New/Entity/FlyingStrings.h>

#include <Misc/Hooks.Otamaa.h>

void BuildingExtData::InitializeConstant()
{

	this->TechnoExt = TechnoExtContainer::Instance.Find(this->AttachedToObject);
	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);
	this->Type = pTypeExt;

	if (pTypeExt)
	{
		if (pTypeExt->IsPrism) {
			this->MyPrismForwarding = std::make_unique<PrismForwarding>();
			this->MyPrismForwarding->Owner = this->AttachedToObject;
		}

		if(!pTypeExt->DamageFire_Offs.empty())
			this->DamageFireAnims.resize(pTypeExt->DamageFire_Offs.size());
	}

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

#include <ExtraHeaders/StackVector.h>

static auto AddToOptions(DWORD OwnerBits, HouseClass* pOwner,
	StackVector<TechnoTypeClass*, 256>& Options,
	TechnoTypeClass** Data ,
	size_t size
) {

	for (size_t i = 0; i < size; ++i) {
		auto Option = *(Data + i);

		//Debug::LogInfo("Checking [%s - %s] option for [%s] " ,
		//	Option->ID,
		//	Option->GetThisClassName() ,
		//	pOwner->Type->ID
		//);

		const auto pExt = TechnoTypeExtContainer::Instance.Find(Option);
		const bool Eligible = (OwnerBits & pExt->Secret_RequiredHouses) != 0 && (OwnerBits & pExt->Secret_ForbiddenHouses) == 0 ;

		if (Eligible)
		{
			const auto result = HouseExtData::RequirementsMet(pOwner, Option);

			switch (result)
			{
			case RequirementStatus::Forbidden:
			case RequirementStatus::Incomplete:

				//Debug::LogInfo("[%s - %s] Is Avaible for [%s] ",
				//	Option->ID,
				//	Option->GetThisClassName(),
				//	pOwner->Type->ID
				//);
				Options->emplace_back(Option);
				break;
			default:

			//	Debug::LogInfo("[%s - %s] Is Unavaible[%d] for [%s] ",
			//		Option->ID,
			//		Option->GetThisClassName(),
			//		result,
			//		pOwner->Type->ID
			//	);
				break;
			}
		}
	}
};

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
		Debug::LogInfo("[Secret Lab] {} has a fixed boon.", pType->ID);
		return;
	}

	auto pData = BuildingTypeExtContainer::Instance.Find(pType);

	// go on if not placed or always recalculate on capture
	if (pExt->SecretLab_Placed && !pData->Secret_RecalcOnCapture) {
		return;
	}

	StackVector<TechnoTypeClass* , 256> Options {};
	const DWORD OwnerBits = 1u << pOwner->Type->ArrayIndex;

	TechnoTypeClass** vec_data = pData->Secret_Boons.HasValue() ?
		pData->Secret_Boons.data() : RulesExtData::Instance()->Secrets.data();
	size_t vec_size = pData->Secret_Boons.HasValue() ?
		pData->Secret_Boons.size() : RulesExtData::Instance()->Secrets.size();

	// generate a list of items
	AddToOptions(OwnerBits,pOwner, Options, vec_data, vec_size);

	// pick one of all eligible items
	if (!Options->empty())
	{
		const auto Result = Options[ScenarioClass::Instance->Random.RandomFromMax(Options->size() - 1)];
		Debug::LogInfo("[Secret Lab] rolled {} for {}", Result->ID, pType->ID);
		pThis->SecretProduction = Result;
		pExt->SecretLab_Placed = true;
	} else {
		Debug::LogInfo("[Secret Lab] {} has no boons applicable to country [{}]!",
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

#include <ExtraHeaders/StackVector.h>

void BuildingExtData::ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse)
{
	if (!pAttackerHouse || !pTargetHouse || LimboIDs.empty())
		return;

	if (!EnumFunctions::CanTargetHouse(Affects.Get(), pAttackerHouse, pTargetHouse))
		return;

	StackVector<BuildingClass* , 256> LimboedID {};
	for (const auto& pBuilding : pTargetHouse->Buildings) {
		const auto pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		if (pBuildingExt->LimboID <= -1 || !LimboIDs.Contains(pBuildingExt->LimboID))
			continue;

		LimboedID->push_back(pBuilding);
	}

	for(auto& pLimboBld : LimboedID.container()){
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
			Debug::LogInfo("Warning! Advanced Rubble was supposed to be reconstructed but"
				" Ares could not obtain its new BuildingType. Check if [{}]Rubble.{} is"
				" set (correctly).", pBuilding->Type->ID, pTagName);
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
				Debug::LogInfo("Advanced Rubble: Failed to place normal state on map!");
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

	if(this->MyPrismForwarding)
		this->MyPrismForwarding->InvalidatePointer(ptr, bRemoved);
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
				((FakeHouseClass*)(pThis->Owner))->_GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
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
		for (auto& pBuilding : airFactoryBuilding)
		{
			if (!pBuilding->IsAlive)
				continue;

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

	for (auto& pBuilding : airFactoryBuilding)
	{
		if (!pBuilding->IsAlive)
			continue;

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

bool BuildingExtData::CanGrindTechnoSimplified(BuildingClass* pBuilding, TechnoClass* pTechno)
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
		//auto const pOwnerExt = HouseExtContainer::Instance.Find(pOwner);

		// BuildLimit check goes before creation
		if (((BuildLimitStatus)HouseExtData::BuildLimitGroupCheck(pOwner, pType, true , false)) != BuildLimitStatus::NotReached && HouseExtData::CheckBuildLimit(pOwner, pType , true) != BuildLimitStatus::NotReached) {
			Debug::LogInfo("Fail to Create Limbo Object[{}] because of BuildLimit ! ", pType->get_ID());
			return;
		}

		BuildingClass* pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner));
		if (!pBuilding)
		{
			Debug::LogInfo("Fail to Create Limbo Object[{}] ! ", pType->get_ID());
			return;
		}

		//Debug::LogInfo("[0x%x - %s] Sending [%s] As Limbo Delivered ID [%d]", pOwner , pOwner->get_ID(), pType->ID, ID);
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

		//pOwner->AddTracking(pBuilding);
		pOwner->RegisterGain(pBuilding, false);
		pOwner->UpdatePower();
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->RecheckRadar = true;
		pOwner->Buildings.AddItem(pBuilding);

		pOwner->ActiveBuildingTypes.Increment(pBuilding->Type->ArrayIndex);
		pOwner->UpdateSuperWeaponsUnavailable();

		auto const pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		HouseExtData::UpdateFactoryPlans(pBuilding);

		if (BuildingTypeExtContainer::Instance.Find(pType)->Academy)
			HouseExtContainer::Instance.Find(pOwner)->UpdateAcademy(pBuilding, true);

		if (pType->SecretLab){
			pOwner->SecretLabs.AddItem(pBuilding);
			BuildingExtData::UpdateSecretLab(pBuilding);
		}

		pBuildingExt->LimboID = ID;
		pBuildingExt->MyPrismForwarding.reset();
		pBuildingExt->TechnoExt->Shield.release();
		pBuildingExt->TechnoExt->Trails.clear();
		pBuildingExt->TechnoExt->RevengeWeapons.clear();
		pBuildingExt->TechnoExt->DamageSelfState.release();
		pBuildingExt->TechnoExt->MyGiftBox.release();
		pBuildingExt->TechnoExt->PaintBallStates.clear();
		pBuildingExt->TechnoExt->ExtraWeaponTimers.clear();
		pBuildingExt->TechnoExt->MyWeaponManager.Clear();
		pBuildingExt->TechnoExt->MyWeaponManager.CWeaponManager.Clear();

		if (!HouseExtData::AutoDeathObjects.contains(pBuilding))
		{
			KillMethod nMethod = pBuildingExt->Type->Type->Death_Method.Get();

			if (nMethod != KillMethod::None
				&& pBuildingExt->Type->Type->Death_Countdown > 0
				&& !pBuildingExt->TechnoExt->Death_Countdown.HasStarted())
			{
				pBuildingExt->TechnoExt->Death_Countdown.Start(pBuildingExt->Type->Type->Death_Countdown);
				HouseExtData::AutoDeathObjects.emplace_unchecked(pBuilding, nMethod);
			}
		}
	}
}

void BuildingExtData::LimboKill(BuildingClass* pBuilding)
{
	if (!pBuilding->IsAlive)
		return;

	Debug::LogInfo("BuildingExtData::LimboKill -  Killing Building[{} - {}] ! ", (void*)pBuilding, pBuilding->get_ID());

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

	for (auto& pBaseNode : pTargetHouse->Base.BaseNodes) {
		if (pBaseNode.BuildingTypeIndex == pType->ArrayIndex)
				pBaseNode.Placed = false;
	}
#endif

	// Remove completely
	Debug::LogInfo(__FUNCTION__" Called ");
	TechnoExtData::HandleRemove(pBuilding, nullptr, true, false);
}

void FakeBuildingClass::_OnFireAI()
{
	const auto pType = this->Type;
	const auto pExt = this->_GetExtData();
	const auto pTypeext = pExt->Type;

	for (auto& nFires : pExt->DamageFireAnims) {
		if (nFires && nFires->Type) {
			nFires->TimeToDie = true;
			nFires->UnInit();
			nFires = nullptr;
		}
	}

	auto const& pFire = pTypeext->DamageFireTypes.GetElements(RulesClass::Instance->DamageFireTypes);

	if (!pFire.empty() &&
		!pTypeext->DamageFire_Offs.empty())
	{
		pExt->DamageFireAnims.resize(pTypeext->DamageFire_Offs.size());
		const auto render_coords = this->GetRenderCoords();
		const auto nBuildingHeight = pType->GetFoundationHeight(false);
		const auto nWidth = pType->GetFoundationWidth();

		for (int i = 0; i < (int)pTypeext->DamageFire_Offs.size(); ++i)
		{
			const auto& nFireOffs = pTypeext->DamageFire_Offs[i];
			const auto& [nPiX, nPiY] = TacticalClass::Instance->ApplyOffsetPixel(nFireOffs);

			CoordStruct nPixCoord { nPiX, nPiY, 0 };
			nPixCoord += render_coords;

			if (const auto pFireType = pFire[pFire.size() == 1 ?
				 0 : ScenarioClass::Instance->Random.RandomFromMax(pFire.size() - 1)])
			{
				auto pAnim = GameCreate<AnimClass>(pFireType, nPixCoord);
				const auto nAdjust = ((3 * (nFireOffs.Y - 15 * nWidth + (-15) * nBuildingHeight)) >> 1) - 10;
				pAnim->ZAdjust = nAdjust > 0 ? 0 : nAdjust; //ZAdjust always negative
				if (pAnim->Type->End > 0)
					pAnim->Animation.Value = ScenarioClass::Instance->Random.RandomFromMax(pAnim->Type->End - 1);

				pAnim->Owner = this->Owner;
				pExt->DamageFireAnims[i] = pAnim;
			}
		}
	}
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
		.Process(this->MyPrismForwarding)
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
		.Process(this->FactoryBuildingMe, true)
		;
}

// =============================
// container
BuildingExtContainer BuildingExtContainer::Instance;
// =============================
// container hooks

ASMJIT_PATCH(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Remove(pItem);
	return 0;
}

HRESULT __stdcall FakeBuildingClass::_Load(IStream* pStm)
{

	BuildingExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BuildingClass::Load(pStm);

	if (SUCCEEDED(res))
		BuildingExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeBuildingClass::_Save(IStream* pStm, bool clearDirty)
{

	BuildingExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BuildingClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		BuildingExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3ED0, FakeBuildingClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3ED4, FakeBuildingClass::_Save)

void FakeBuildingClass::_Detach(AbstractClass* target , bool all) {
	BuildingExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->BuildingClass::PointerExpired(target , all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3EE4, FakeBuildingClass::_Detach)

void FakeBuildingClass::_DetachAnim(AnimClass* pAnim)
{
	this->TechnoClass::AnimPointerExpired(pAnim);

	auto pExt = BuildingExtContainer::Instance.Find(this);

	if (pAnim == pExt->SpyEffectAnim.get()) {
		pExt->SpyEffectAnim.release();
	}
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3F1C, FakeBuildingClass::_DetachAnim)
//