#include "Body.h"
#include <Utilities/EnumFunctions.h>

#include <Utilities/Macro.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Side/Body.h>

#include <New/Entity/FlyingStrings.h>

#include <Misc/Hooks.Otamaa.h>

void BuildingExtData::UpdateMainEvaVoice()
{
	auto const pTypeExt = this->Type;

	if (!pTypeExt->NewEvaVoice || !pTypeExt->NewEvaVoice_Index.isset())
		return;

	auto const pThis = This();

	auto const pHouse = pThis->Owner;
	int newPriority = -1;
	int newEvaIndex = -1;

	for (const auto pBuilding : pHouse->Buildings)
	{
		if (pBuilding->CurrentMission == Mission::Selling)
			continue;

		auto const pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
		if (!pBuildingTypeExt->NewEvaVoice_Index.isset())
			continue;

		// The first highest priority takes precedence over lower ones
		if (pBuildingTypeExt->NewEvaVoice && pBuildingTypeExt->NewEvaVoice_Priority > newPriority)
		{
			newPriority = pBuildingTypeExt->NewEvaVoice_Priority;
			newEvaIndex = pBuildingTypeExt->NewEvaVoice_Index;
		}
	}

	if (pThis->CurrentMission != Mission::Selling && pTypeExt->NewEvaVoice_Priority > newPriority)
	{
		newPriority = pTypeExt->NewEvaVoice_Priority;
		newEvaIndex = pTypeExt->NewEvaVoice_Index;
	}

	if (newPriority > 0 && VoxClass::EVAIndex != newEvaIndex) {
		// Note: if the index points to a nonexistant voice index then the player will hear no EVA voices
		VoxClass::EVAIndex = newEvaIndex;

		// Greeting of the new EVA voice
		VoxClass::PlayIndex(pTypeExt->NewEvaVoice_InitialMessage);

	} else if (newPriority < 0) {
		// Restore the original EVA voice of the owner's side
		VoxClass::EVAIndex = SideExtContainer::Instance.Find(
			SideClass::Array->Items[pHouse->SideIndex])->EVAIndex;
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
	if (!pReverseData->ReverseEngineersVictims) {
		return false;
	}

	const auto pBldOwner = pBuilding->Owner;
	auto pBldOwnerExt = HouseExtContainer::Instance.Find(pBldOwner);

	return pBldOwnerExt->ReverseEngineer(Victim);
}

void BuildingExtData::ApplyLimboKill(ValueableVector<int>& LimboIDs, Valueable<AffectedHouse>& Affects, HouseClass* pTargetHouse, HouseClass* pAttackerHouse)
{
	if (!pAttackerHouse || !pTargetHouse || LimboIDs.empty())
		return;

	if (!EnumFunctions::CanTargetHouse(Affects.Get(), pAttackerHouse, pTargetHouse))
		return;

	StackVector<BuildingClass* , 20> LimboedID {};
	for (const auto& pBuilding : pTargetHouse->Buildings) {
		const auto pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		if (pBuildingExt->LimboID <= -1 || !LimboIDs.Contains(pBuildingExt->LimboID))
			continue;

		LimboedID->push_back(pBuilding); // we cant do it immedietely since the array will me modified
		// we need to fetch the eligible building first before really killing it
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
		if(!RulesExtData::Instance()->DisplayIncome_AllowAI && !This()->Owner->IsControlledByHuman())
		{
			this->AccumulatedIncome = 0;
			return;
		}

		FlyingStrings::AddMoneyString(
			this->AccumulatedIncome,
			this->AccumulatedIncome,
			This(),
			this->Type->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses),
			This()->GetRenderCoords(),
			this->Type->DisplayIncome_Offset
		);

		this->AccumulatedIncome = 0;
	}
}

void BuildingExtData::UpdatePoweredKillSpawns() const
{
	auto const pThis = (BuildingClass*)this->AttachedToObject;

	if (this->Type->Powered_KillSpawns &&
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
	auto const pThis = This();
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
	auto const pThis = (BuildingClass*)this->AttachedToObject;
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

	auto currentBuilding = (BuildingClass*)AttachedToObject;
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
	const auto pThis = (BuildingClass*)this->AttachedToObject;

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
	this->TechnoExtData::InvalidatePointer(ptr, bRemoved);

	AnnounceInvalidPointer(this->CurrentAirFactory, ptr , bRemoved);
	AnnounceInvalidPointer<TechnoClass*>(this->RegisteredJammers, ptr, bRemoved);

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

	auto pBldExt = BuildingExtContainer::Instance.Find(pThis);
	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->Items[pOwner->ProducingAircraftTypeIndex];
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	pBldExt->airFactoryBuilding.clear();
	pBldExt->airFactoryBuilding.reserve(pOwner->Buildings.Count);
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (pBldExt->CurrentAirFactory)
	{
		int nDocks = 0;
		if (pBldExt->CurrentAirFactory->Type)
			nDocks = pBldExt->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = BuildingExtData::CountOccupiedDocks(pBldExt->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = pBldExt->CurrentAirFactory->Factory;
		else
			pBldExt->CurrentAirFactory = nullptr;
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

			pBldExt->airFactoryBuilding.push_back(pBuilding);
		}
	}

	if (pBldExt->CurrentAirFactory)
	{
		for (auto& pBuilding : pBldExt->airFactoryBuilding)
		{
			if (!pBuilding->IsAlive)
				continue;

			if (pBuilding == pBldExt->CurrentAirFactory)
			{
				pBldExt->CurrentAirFactory->Factory = currFactory;
				pBldExt->CurrentAirFactory->IsPrimaryFactory = true;
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

	for (auto& pBuilding : pBldExt->airFactoryBuilding)
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
				pBldExt->CurrentAirFactory = newBuilding;

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
			VocClass::SafeImmedietelyPlayAt(pTypeExt->Grinding_Sound.Get(), &pTechno->GetCoords());
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
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
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
		pBuildingExt->Shield.release();
		pBuildingExt->Trails.clear();
		pBuildingExt->RevengeWeapons.clear();
		pBuildingExt->DamageSelfState.release();
		pBuildingExt->MyGiftBox.release();
		pBuildingExt->PaintBallStates.clear();
		pBuildingExt->ExtraWeaponTimers.clear();
		pBuildingExt->MyWeaponManager.Clear();
		pBuildingExt->MyWeaponManager.CWeaponManager.Clear();

		if (!HouseExtData::AutoDeathObjects.contains(pBuilding))
		{
			KillMethod nMethod = pBuildingExt->Type->Death_Method.Get();

			if (nMethod != KillMethod::None) {

				if(pBuildingExt->Type->Death_Countdown > 0)
					pBuildingExt->Death_Countdown.Start(pBuildingExt->Type->Death_Countdown);

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
	//Debug::LogInfo(__FUNCTION__" Called ");
	TechnoExtData::HandleRemove(pBuilding, nullptr, true, false);
}

constexpr int GetRepairValue(BuildingClass* pTarget, int repair)
{

	int repairAmount = pTarget->Type->Strength;

	if (repair > 0)
	{
		repairAmount = std::clamp(pTarget->Health + repair, 0, pTarget->Type->Strength);
	}
	else if (repair < 0)
	{
		const double percentage = std::clamp(pTarget->GetHealthPercentage() - (static_cast<double>(repair) / 100), 0.0, 1.0);
		repairAmount = static_cast<int>(std::round(pTarget->Type->Strength * percentage));
	}

	return repairAmount;
};

void FakeBuildingClass::_OnFinishRepairB(InfantryClass* pEngineer)
{
	const bool wasDamaged = this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	this->Mark(MarkType::Change);

	const int repairBuilding = TechnoTypeExtContainer::Instance.Find(this->Type)->EngineerRepairAmount;
	const int repairEngineer = TechnoTypeExtContainer::Instance.Find(pEngineer->Type)->EngineerRepairAmount;
	this->Health = MinImpl(GetRepairValue(this, repairBuilding), GetRepairValue(this, repairEngineer));
	this->EstimatedHealth = this->Health;
	this->SetRepairState(0);

	if ((this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow) != wasDamaged)
	{
		this->ToggleDamagedAnims(!wasDamaged);

		if (wasDamaged && this->DamageParticleSystem)
			this->DamageParticleSystem->UnInit();
	}

	const auto sound = this->_GetTypeExtData()->BuildingRepairedSound.Get(RulesClass::Instance->BuildingRepairedSound);
	VocClass::SafeImmedietelyPlayAt(sound, & this->GetCoords());
}

void FakeBuildingClass::_OnFinishRepair()
{
	const bool wasDamaged = this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	this->Mark(MarkType::Change);
	this->Health = this->Type->Strength;
	this->EstimatedHealth = this->Health;
	this->SetRepairState(0);

	if ((this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow) != wasDamaged)
	{
		this->ToggleDamagedAnims(!wasDamaged);

		if (wasDamaged && this->DamageParticleSystem)
			this->DamageParticleSystem->UnInit();
	}

	const auto sound = this->_GetTypeExtData()->BuildingRepairedSound.Get(RulesClass::Instance->BuildingRepairedSound);
	VocClass::SafeImmedietelyPlayAt(sound, & this->GetCoords());
}

void FakeBuildingClass::UnloadOccupants(bool assignMission, bool killIfStuck) {

	this->FiringOccupantIndex = 0;

	if (!this->Occupants.Count)
		return;

	this->Mark(MarkType::Change);
	CoordStruct defaultCoord = CoordStruct::Empty;
	CellStruct originCell = this->GetMapCoords();
	CoordStruct scatterCoord{};
	CellStruct fallbackCell{};
	bool foundValidCell = false;

	const int width = this->Type->GetFoundationWidth();
	const int height = this->Type->GetFoundationHeight(false);
	const int startX = originCell.X;
	const int startY = originCell.Y;
	const int endX = startX + width;
	const int endY = startY + height;

	InfantryClass* firstOccupant = this->Occupants.Items[0];
	// Define directional priority: up, left, right, down
	static std::array<CellStruct, 4u> directions = {
		CellStruct{0, -1},  // up
		CellStruct{-1, 0},  // left
		CellStruct{1, 0},   // right
		CellStruct{0, 1}   // down
	};

	// Try to find a valid adjacent cell to unload into
	for (const auto& [dx, dy] : directions)
	{
		CellStruct tryCell = { endX, endY };

		while (true)
		{
			tryCell.X += dx;
			tryCell.Y += dy;

			// Stop if out of building bounds
			if (tryCell.X < startX || tryCell.X >= endX ||
				tryCell.Y < startY || tryCell.Y >= endY)
				break;

			CellClass* mapCell = MapClass::Instance->GetCellAt(tryCell);
			if (firstOccupant->IsCellOccupied(mapCell, FacingType::None, -1, false, true) == Move::OK)
			{
				fallbackCell = tryCell;
				foundValidCell = true;
				break;
			}
		}

		if (foundValidCell)
			break;
	}

	if (!foundValidCell)
	{
		if (killIfStuck)
		{
			this->KillOccupants(nullptr);
			return;
		}

		fallbackCell.X = originCell.X + width - 1;
		fallbackCell.Y = originCell.Y + height - 1;
	}

	// Unload logic begins
	CellClass* unloadCell = MapClass::Instance->GetCellAt(fallbackCell);
	scatterCoord = unloadCell->GetCoords();

	++Unsorted::ScenarioInit;

	for (int i = this->Occupants.Count - 1; i >= 0; --i)
	{
		FootClass* foot = this->Occupants.Items[i];

		if (foot->Unlimbo(scatterCoord, DirType::North))
		{
			if (!foot->Owner) {
				Debug::FatalErrorAndExit("BuildingClass::KickAllOccupants for [%x(%s)] Missing Occupier [%x(%s)] House Pointer !",
					this,
					this->get_ID(),
					foot,
					foot->get_ID()
				);
			}

			if (foot->Owner->IsInPlayerControl) {
				foot->ShouldGarrisonStructure = 0;
				foot->ShouldEnterOccupiable = 0;
			}

			foot->SetTarget(nullptr);
			CoordStruct originCenter = this->GetCoords();
			foot->Scatter(originCenter, true, true);

			if (assignMission)
			{
				if (TeamClass* team = foot->Team)
					team->RemoveMember(foot, -1, false);

				foot->QueueMission(Mission::Hunt, 0);
			}
		}
		else
		{
			foot->UnInit();
		}
	}

	--Unsorted::ScenarioInit;

	// Reset Occupants vector while keeping capacity
	this->Occupants.Reset();

	// Update threat map
	this->UpdateThreatInCell(this->GetCell());
}

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

int ProcessNukeSilo(BuildingClass* pThis, SuperClass* pLinked , SWTypeExtData* pLinkedTypeExt) {
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	enum class NukeFiringState : int {
		PsiWarn, Aux1, SentWeaponPayload, Aux2, Idle
	};
	switch ((NukeFiringState)pThis->MissionStatus)
	{
	case NukeFiringState::PsiWarn:
	{
		pThis->IsReadyToCommence = false;
		pThis->BeginMode(BStateType::Active);

		HouseClass* house = pThis->Owner;
		pThis->MissionStatus = 1;

		const auto targetCell = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->NukeTarget;
		CellClass* v3 = MapClass::Instance->GetCellAt(targetCell);

		if(AnimClass* anim =
			BulletClass::CreateDamagingBulletAnim(house,
			v3,
			nullptr,
			SWTypeExtContainer::Instance.Find(pLinked->Type)->Nuke_PsiWarning
			)){

			anim->SetBullet(nullptr);
			anim->SetHouse(house);
			anim->Invisible = true;
			pThis->PsiWarnAnim = anim;
		}

		// fall through to case 1
	}
	case NukeFiringState::Aux1:
	{
		if (pThis->IsReadyToCommence) {
			pThis->BeginMode(BStateType::Aux1);
			pThis->MissionStatus = 2;
		}
		// fall through to case 2
	}
	case NukeFiringState::SentWeaponPayload:
	{
		//HouseClass* house = pThis->Owner;
		const auto targetCell = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->NukeTarget;
		CellClass* v3 = MapClass::Instance->GetCellAt(targetCell);

		if (WeaponTypeClass* pWeapon = pLinked->Type->WeaponType)
		{
			//speed harcoded to 255
			if (auto pCreated = pWeapon->Projectile->CreateBullet(v3, pThis, pWeapon->Damage, pWeapon->Warhead, 255, pWeapon->Bright || pWeapon->Warhead->Bright))
			{
				BulletExtContainer::Instance.Find(pCreated)->NukeSW = pLinkedTypeExt->AttachedToObject;
				pCreated->Range = WeaponTypeExtContainer::Instance.Find(pWeapon)->GetProjectileRange();
				pCreated->SetWeaponType(pWeapon);

				if (pThis->PsiWarnAnim)
				{
					pThis->PsiWarnAnim->SetBullet(pCreated);
					pThis->PsiWarnAnim = nullptr;
				}

				//Limbo-in the bullet will remove the `TechnoClass` owner from the bullet !
				//pThis->Limbo();

				CoordStruct nFLH;
				pThis->GetFLH(&nFLH, 0, CoordStruct::Empty);

				// Otamaa : the original calculation seems causing missile to be invisible
				//auto nCos = 0.00004793836;
				//auto nCos = Math::cos(1.570748388432313); // Accuracy is different from the game
				//auto nSin = 0.99999999885;
				//auto nSin = Math::sin(1.570748388432313);// Accuracy is different from the game

				const auto nMult = pCreated->Type->Vertical ? 10.0 : 100.0;
				//const auto nX = nCos * nCos * nMult;
				//const auto nY = nCos * nSin * nMult;
				//const auto nZ = nSin * nMult;

				if (!pCreated->MoveTo(nFLH, { 0.0, 0.0 , nMult })) {
					GameDelete<true, false>(pCreated);
				} else {
					if (auto const pAnimType = SWTypeExtContainer::Instance.Find(pLinked->Type)->Nuke_TakeOff.Get(RulesClass::Instance->NukeTakeOff))
					{
						auto pAnim = GameCreate<AnimClass>(pAnimType, nFLH);
						if (!pAnim->ZAdjust)
							pAnim->ZAdjust = -100;

						pAnim->SetHouse(pThis->GetOwningHouse());
						((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
					}
				}

				pThis->MissionStatus = 3;
			}
		}

#ifdef _OriginalGameCode
		WeaponTypeClass* WeaponType = SuperWeaponTypeClass::Array->Items[pThis->FiringSWType]->WeaponType;

		if (BulletClass* Bullet = WeaponType->Projectile->CreateBullet(v3,
			pThis ,
			WeaponType->Damage ,
			WeaponType->Warhead ,
			255,
			true)
			) {
			Bullet->SetWeaponType(WeaponType);

			if (auto pPSIWarn = pThis->PsiWarnAnim) {
				pPSIWarn->SetBullet(Bullet);
				pThis->PsiWarnAnim=  nullptr;
			}

			Bullet->Limbo();

			CoordStruct coord {};
			pThis->GetFLH(&coord, 0, CoordStruct::Empty);

			VelocityClass velocity{};
			double angle = 1.570748388432313; // ~90 degrees
			double speed = 10.0;

			double sinA = Math::sin(angle);
			double cosA = Math::cos(angle);

			velocity.X = cosA * sinA * speed;
			velocity.Y = sinA * speed;
			velocity.Z = cosA * cosA * speed;

			if (Bullet->MoveTo(coord, velocity)) {
				CoordStruct coord = v3->GetCoords();
				AnimClass* anim = GameCreate<AnimClass>(RulesClass::Instance->NukeTakeOff, coord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				anim->ZAdjust = -100;
				pThis->MissionStatus = 3;
			}
			else {
				GameDelete<true, false>(Bullet);
				pThis->MissionStatus = 3;
			}
		}
#endif

		return 1;
	}
	case  NukeFiringState::Aux2:
		pThis->BeginMode(BStateType::Aux2);
		pThis->MissionStatus = 4;
		return TIMER_SECOND / 10;
	case  NukeFiringState::Idle:
		pThis->BeginMode(BStateType::Idle);
		pThis->QueueMission(Mission::Guard, false);
		return TIMER_SECOND;
	default:
		//apply the missioncontrol rate as delay
		return int(pThis->GetCurrentMissionControl()->Rate * TICKS_PER_MINUTE);
	}
}

int ProcessEMPUlseCannon(BuildingClass* pThis, SuperClass* pLinked, SWTypeExtData* pLinkedTypeExt) 	{

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	enum class EMPulseFiringState : int {
		Preparing , PlayPulseBall , SentWeaponPayload , RestoreFacing
	};

	switch ((EMPulseFiringState)pThis->MissionStatus)
	{
	case EMPulseFiringState::Preparing: //sync the facing and preparing the weapon
	{
		//HouseClass* house = pThis->Owner;

		const auto celltarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->EMPTarget;

		CellClass* v21 = MapClass::Instance->GetCellAt(celltarget);
		DirStruct dirPrimary = pThis->FireAngleTo(v21);

		auto& prim = pThis->PrimaryFacing;

		if (Math::abs(prim.Current().Raw - dirPrimary.Raw)) {
			prim.Set_Desired(dirPrimary);
		} else {
			DirStruct dirBarrel{};
			pThis->GetFacingAgainst(&dirBarrel, v21);
			auto& barr = pThis->BarrelFacing;

			if (Math::abs(barr.Current().Raw - dirBarrel.Raw)) {
				barr.Set_Desired(dirBarrel);
			} else {
				pThis->MissionStatus = 1;
			}
		}

		return 1;
	}
	case EMPulseFiringState::PlayPulseBall:
	{
		if (auto pPulseBall = pLinkedTypeExt->EMPulse_PulseBall)
		{
			CoordStruct flh {};
			pThis->GetFLH(&flh, pExt->idxSlot_EMPulse, CoordStruct::Empty);
			auto pAnim = GameCreate<AnimClass>(pPulseBall, flh);
			pAnim->Owner = pThis->GetOwningHouse();
			((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
		}

		pThis->MissionStatus = 2;

		if(pLinkedTypeExt->EMPulse_PulseDelay >= 0) //if negative value are used , just fallthru the function
			return pLinkedTypeExt->EMPulse_PulseDelay;
	}
	case EMPulseFiringState::SentWeaponPayload:
	{
		//HouseClass* house = pThis->Owner;
		const auto celltarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->EMPTarget;

		// If no valid target or destination, reset to idle
		if (Unsorted::ArmageddonMode() || !celltarget.IsValid()) {
			pThis->BeginMode(BStateType::Idle);
			pThis->QueueMission(Mission::Guard, false);
			return 60;
		}
		WeaponTypeClass* weaponType = pThis->GetWeapon(pExt->idxSlot_EMPulse)->WeaponType;
		AbstractClass* target = MapClass::Instance->GetCellAt(celltarget);

		// Aim the barrel
		DirStruct dirBarrel{};
		pThis->GetFacingAgainst(&dirBarrel, target);
		pThis->BarrelFacing.Set_Desired(dirBarrel);

		// Prepare bullet trajectory
		CoordStruct flhCoord{};
		pThis->GetFLH(&flhCoord, pExt->idxSlot_EMPulse, CoordStruct::Empty);

		CoordStruct targetCoord = CellClass::Cell2Coord(celltarget);
		targetCoord.Z = MapClass::Instance->GetZPos(&targetCoord);

		// Compute angles and bullet speed
		int range = pThis->DistanceFromSquared(target);
		const auto gravity = BulletTypeExtData::GetAdjustedGravity(weaponType->Projectile);
		int speed = Game::AdjustRangeWithGravity(range, gravity);

		BulletClass* bullet = weaponType->Projectile->CreateBullet(target,
			pThis,
			weaponType->Damage,
			weaponType->Warhead,
			(3 * speed) / 4,
			weaponType->Bright);

		if (!bullet) {
			// No bullet created, return to idle
			pThis->BeginMode(BStateType::Idle);
			pThis->QueueMission(Mission::Guard, false);
			return 60;
		}

		bullet->SetWeaponType(weaponType);

		// --- Compute horizontal angle ---
		double targetRadX = (targetCoord.X);
		double targetRadY = (targetCoord.Y);

		double angleToTarget = Math::atan2(
			((double)flhCoord.Y - targetRadY),
			(targetRadX - (double)flhCoord.X)
		);

		// Offset by -90 degrees
		angleToTarget -= Math::DEG90_AS_RAD;

		// Convert to binary angle
		int binaryAngle = (int)(angleToTarget * Math::BINARY_ANGLE_MAGIC);

		// --- Base velocity calculation ---
		double magnitude = Math::sqrt(10000.0);
		double radians = (double)(binaryAngle - 16383) * -0.00009587672516830327;

		VelocityClass vel{ Math::cos(radians) * magnitude  , -(Math::sin(radians) * magnitude)  , 0.0 };
		vel.SetIfZeroXYZ();

		// Normalize and scale to projectile speed
		double length = vel.Length();
		double scale = speed / length;

		vel *= scale;

		// --- Adjust for weapon direction ---
		CoordStruct barrelDir{};
		pThis->vt_entry_300(&barrelDir, pExt->idxSlot_EMPulse);
		CoordStruct bulletPos = bullet->GetCoords();
		double dz = bulletPos.Z - barrelDir.Z;
		double xyDistSq = (bulletPos.X - barrelDir.X) * (bulletPos.X - barrelDir.X) +
			(bulletPos.Y - barrelDir.Y) * (bulletPos.Y - barrelDir.Y);

		double xyDist = Math::sqrt(xyDistSq);

		// Recalculate direction if needed
		DirStruct legal;
		const bool canReach = pThis->CanReachTarget(pExt->idxSlot_EMPulse);
		if (!Game::func_48A8D0_Legal(canReach, speed, xyDist, dz, gravity, &legal)) {
			if (!Game::func_48A8D0_Legal(canReach, (10 * speed) / 8, xyDist, dz, gravity, &legal)) {
				legal = DirStruct((unsigned short)-1536);
			}
		}

		// --- Pitch adjustment ---
		double pitch = Math::atan2(vel.Z, vel.LengthXY()) - Math::DEG90_AS_RAD;
		int pitchBinary = (int)(pitch * Math::BINARY_ANGLE_MAGIC) - 0x3FFF;
		double pitchRad = (double)pitchBinary * -0.00009587672516830327;

		if (pitchRad != 0.0)
		{
			vel.X /= Math::cos(pitchRad);
			vel.Y /= Math::cos(pitchRad);
		}

		// --- Facing adjustment ---
		double dirRad = ((double)legal.Raw - 0x3FFF) * -0.00009587672516830327;
		vel.X *= Math::cos(dirRad);
		vel.Y *= Math::cos(dirRad);
		vel.Z = Math::sin(dirRad) * speed;


		auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(bullet->Type);
		auto pBulletExt = BulletExtContainer::Instance.Find(bullet);

		if (bullet->Type->Arcing && !pBulletTypeExt->Arcing_AllowElevationInaccuracy) {
			pBulletExt->ApplyArcingFix(flhCoord, targetCoord, vel);
		}

		BulletExtData::SimulatedFiringEffects(bullet, pThis->Owner, pThis, true, true);

		// Fire the bullet
		if (!bullet->MoveTo(flhCoord, vel))
		{
			GameDelete<true, false>(bullet);
			pThis->BeginMode(BStateType::Idle);
			pThis->QueueMission(Mission::Guard, false);
			return TIMER_SECOND;
		}

		// Turret recoil if applicable
		if (pThis->HasTurret() &&
			pThis->Type->TurretRecoil)
		{
			pThis->TurretRecoil.Update();
			pThis->BarrelRecoil.Update();
		}

		// Play sound if not gattling , this will played twice
		if (!pThis->Type->IsGattling && weaponType->Report.Count > 0) {
			const int soundIdx = weaponType->Report.Count > 1 ? ScenarioClass::Instance->Random.RandomFromMax(weaponType->Report.Count - 1) : 0;
			VocClass::SafeImmedietelyPlayAt(soundIdx, flhCoord);
		}

		if (pThis->Type->Ammo > 0 && pThis->Ammo > 0)
		{
			int ammo = WeaponTypeExtContainer::Instance.Find(weaponType)->Ammo.Get();
			pThis->Ammo -= ammo;
			pThis->Ammo = pThis->Ammo < 0 ? 0 : pThis->Ammo;

			if (pThis->Ammo >= ammo)
				pThis->MissionStatus = 0;

			if (!pThis->ReloadTimer.InProgress())
				pThis->ReloadTimer.Start(pThis->Type->Reload);

			if (pThis->Ammo == 0 && pThis->Type->EmptyReload >= 0 && pThis->ReloadTimer.GetTimeLeft() > pThis->Type->EmptyReload)
				pThis->ReloadTimer.Start(pThis->Type->EmptyReload);

			return 1;
		}

		pThis->MissionStatus = 3;
		return 1;
	}
	case EMPulseFiringState::RestoreFacing:
	{
		DirStruct resetFacing((int)0x4000);
		pThis->BarrelFacing.Set_Desired(resetFacing);

		// Return to idle
		pThis->BeginMode(BStateType::Idle);
		pThis->QueueMission(Mission::Guard, 0);
		return TIMER_SECOND;
	}
	default:
		//apply the missioncontrol rate as delay
		return int(pThis->GetCurrentMissionControl()->Rate * TICKS_PER_MINUTE);
	}
}

int ProcessMissionMissile(BuildingClass* pThis, SuperClass* pLinked, SWTypeExtData* pLinkedTypeExt) 	{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pThis->Type->EMPulseCannon) {
		const auto pTarget = pExt->SuperTarget.IsValid()
				? pExt->SuperTarget : pThis->Owner->NukeTarget;

		pThis->Fire(MapClass::Instance->GetCellAt(pTarget), 0);
		pThis->QueueMission(Mission::Guard, false);

		return 1; //ares
	} else {
		return ProcessEMPUlseCannon(pThis, pLinked, pLinkedTypeExt);
	}

}

int FakeBuildingClass::_Mission_Missile() {

	if (this->FiringSWType < 0) {
		Debug::LogInfo("Building[{}] with Mission::Missile Missing Important Linked SW data !", this->get_ID());
		//apply the missioncontrol rate as delay
		return int(this->GetCurrentMissionControl()->Rate * TICKS_PER_MINUTE);
	}

	auto pSW = this->Owner->Supers.Items[this->FiringSWType];
	auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSW->Type);

	if (this->Type->NukeSilo) {
		return ProcessNukeSilo(this,pSW,pSWTypeExt);
	}

	return ProcessMissionMissile(this, pSW, pSWTypeExt);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E410C, FakeBuildingClass::_Mission_Missile);
DEFINE_FUNCTION_JUMP(LJMP, 0x44C980, FakeBuildingClass::_Mission_Missile);

DEFINE_FUNCTION_JUMP(LJMP, 0x457DE0, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x44263B, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x44A5CA, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x44D89C, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x458229, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x501509, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x6DF77F, FakeBuildingClass::UnloadOccupants);
DEFINE_FUNCTION_JUMP(CALL, 0x6E40F2, FakeBuildingClass::UnloadOccupants);

int FakeBuildingClass::_GetAirstrikeInvulnerabilityIntensity(int currentIntensity) const
{
	int newIntensity = this->GetFlashingIntensity(currentIntensity);

	if (this->IsIronCurtained() || this->_GetTechnoExtData()->AirstrikeTargetingMe)
		newIntensity = this->GetEffectTintIntensity(newIntensity);

	return newIntensity;
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
					pAnim->Animation.Stage = ScenarioClass::Instance->Random.RandomFromMax(pAnim->Type->End - 1);

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
		.Process(this->Type)
		.Process(this->MyPrismForwarding)
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory)
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
		.Process(this->SecretLab_Placed)
		.Process(this->AboutToChronoshift)
		.Process(this->IsFromSW)
		.Process(this->RegisteredJammers)
		.Process(this->GrindingWeapon_AccumulatedCredits)
		.Process(this->BeignMCEd)
		.Process(this->LastFlameSpawnFrame)
		.Process(this->SpyEffectAnim)
		.Process(this->SpyEffectAnimDuration)
		.Process(this->PoweredUpToLevel)
		.Process(this->FactoryBuildingMe)
		.Process(this->airFactoryBuilding)
		;
}

// =============================
// container
BuildingExtContainer BuildingExtContainer::Instance;
// =============================
// container hooks

ASMJIT_PATCH(0x43BAD6, BuildingClass_CTOR, 0x5)
{
	GET(BuildingClass*, pItem, ESI);
	BuildingExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x43B733, BuildingClass_CTOR, 0x7)
{
	GET(BuildingClass*, pItem, ESI);
	BuildingExtContainer::Instance.AllocateNoInit(pItem);
	return 0;
}

ASMJIT_PATCH(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	FakeHouseClass* pOwner = (FakeHouseClass*)pItem->Owner;
	auto pOwnerExt = pOwner->_GetExtData();

	pOwnerExt->TunnelsBuildings.erase(pItem);
	pOwnerExt->Academies.erase(pItem);
	pOwnerExt->RestrictedFactoryPlants.erase(pItem);

	BuildingExtContainer::Instance.Remove(pItem);
	return 0;
}

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