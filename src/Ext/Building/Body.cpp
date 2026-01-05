#include "Body.h"
#include <Utilities/EnumFunctions.h>

#include <Utilities/Macro.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Tactical/Body.h>
#include <New/Entity/FlyingStrings.h>

#include <TextDrawing.h>

#include <Misc/Hooks.Otamaa.h>

#include <Phobos.SaveGame.h>

BuildingExtData::~BuildingExtData()
{

	auto pThis = This();
	FakeHouseClass* pOwner = (FakeHouseClass*)pThis->Owner;
	auto pOwnerExt = pOwner->_GetExtData();

	pOwnerExt->TunnelsBuildings.erase(pThis);
	pOwnerExt->Academies.erase(pThis);
	pOwnerExt->RestrictedFactoryPlants.erase(pThis);
}

//use IsPoweredOnline ?
bool BuildingExtData::BuildingHasPower(BuildingClass* pThis)
{
	//if (pThis->HasPower)
	//{
	//	if (pThis->IsDeactivated()) {
	//		return false;
	//	}

	//	if (pThis->IsUnderEMP()) {
	//		return false;
	//	}

	//	if (!(TechnoExtContainer::Instance.Find(pThis)->Is_Operated || TechnoExt_ExtData::IsOperated(pBld))) {
	//		return false;
	//	}

	//	return true;
	//}

	return false;
}


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

	if (newPriority > 0 && VoxClass::EVAIndex != newEvaIndex)
	{
		// Note: if the index points to a nonexistant voice index then the player will hear no EVA voices
		VoxClass::EVAIndex = newEvaIndex;

		// Greeting of the new EVA voice
		VoxClass::PlayIndex(pTypeExt->NewEvaVoice_InitialMessage);

	}
	else if (newPriority < 0)
	{
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

	foundationCells.remove_all_duplicates([](const CellStruct& lhs, const CellStruct& rhs) -> bool
 {
	 return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});

	return foundationCells;
}

#include <ExtraHeaders/StackVector.h>

static auto AddToOptions(DWORD OwnerBits, HouseClass* pOwner,
	StackVector<TechnoTypeClass*, 256>& Options,
	TechnoTypeClass** Data,
	size_t size
)
{

	for (size_t i = 0; i < size; ++i)
	{
		auto Option = *(Data + i);

		//Debug::LogInfo("Checking [%s - %s] option for [%s] " ,
		//	Option->ID,
		//	Option->GetThisClassName() ,
		//	pOwner->Type->ID
		//);

		const auto pExt = TechnoTypeExtContainer::Instance.Find(Option);
		const bool Eligible = (OwnerBits & pExt->Secret_RequiredHouses) != 0 && (OwnerBits & pExt->Secret_ForbiddenHouses) == 0;

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

	if (!pOwner || pOwner->Type->MultiplayPassive)
	{
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
	if (pExt->SecretLab_Placed && !pData->Secret_RecalcOnCapture)
	{
		return;
	}

	StackVector<TechnoTypeClass*, 256> Options {};
	const DWORD OwnerBits = 1u << pOwner->Type->ArrayIndex;

	TechnoTypeClass** vec_data = pData->Secret_Boons.HasValue() ?
		pData->Secret_Boons.data() : RulesExtData::Instance()->Secrets.data();
	size_t vec_size = pData->Secret_Boons.HasValue() ?
		pData->Secret_Boons.size() : RulesExtData::Instance()->Secrets.size();

	// generate a list of items
	AddToOptions(OwnerBits, pOwner, Options, vec_data, vec_size);

	// pick one of all eligible items
	if (!Options->empty())
	{
		const auto Result = Options[ScenarioClass::Instance->Random.RandomFromMax(Options->size() - 1)];
		Debug::LogInfo("[Secret Lab] rolled {} for {}", Result->ID, pType->ID);
		pThis->SecretProduction = Result;
		pExt->SecretLab_Placed = true;
	}
	else
	{
		Debug::LogInfo("[Secret Lab] {} has no boons applicable to country [{}]!",
			pType->ID, pOwner->Type->ID);
	}
}

bool BuildingExtData::ReverseEngineer(BuildingClass* pBuilding, TechnoClass* Victim)
{
	const auto pReverseData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
	if (!pReverseData->ReverseEngineersVictims)
	{
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

	StackVector<BuildingClass*, 20> LimboedID {};
	for (const auto& pBuilding : pTargetHouse->Buildings)
	{
		const auto pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		if (pBuildingExt->LimboID <= -1 || !LimboIDs.Contains(pBuildingExt->LimboID))
			continue;

		LimboedID->push_back(pBuilding); // we cant do it immedietely since the array will me modified
		// we need to fetch the eligible building first before really killing it
	}

	for (auto& pLimboBld : LimboedID.container())
	{
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

	if (idx < (size_t)pThis->Owner->Supers.Count)
	{
		return pThis->Owner->Supers[idx];
	}

	return nullptr;
}

void BuildingExtData::DisplayIncomeString()
{
	if (this->Type->DisplayIncome.Get(RulesExtData::Instance()->DisplayIncome) &&
		this->AccumulatedIncome && Unsorted::CurrentFrame % 15 == 0)
	{
		if (!RulesExtData::Instance()->DisplayIncome_AllowAI && !This()->Owner->IsControlledByHuman())
		{
			this->AccumulatedIncome = 0;
			return;
		}

		FlyingStrings::Instance.AddMoneyString(
			this->AccumulatedIncome,
			this->AccumulatedIncome,
			This(),
			this->Type->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses),
			This()->GetRenderCoords(),
			this->Type->DisplayIncome_Offset, ColorStruct::Empty);

		this->AccumulatedIncome = 0;
	}
}

void BuildingExtData::UpdatePoweredKillSpawns() const
{
	auto const pThis = (BuildingClass*)this->This();

	if (this->Type->Powered_KillSpawns && !pThis->IsPowerOnline()) {
		if (const auto& pManager = pThis->SpawnManager) {
			pManager->ResetTarget();

			for (const auto& pItem : pManager->SpawnedNodes) {
				if (pItem->Status == SpawnNodeStatus::Attacking 
					|| pItem->Status == SpawnNodeStatus::Returning) {
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
		if (HouseClass::IsCurrentPlayerObserver() || EnumFunctions::CanTargetHouse(
			this->Type->SpyEffect_Anim_DisplayHouses,
			SpyEffectAnim->Owner, HouseClass::CurrentPlayer))
		{
			SpyEffectAnim->Invisible = false;
		}
		else
		{
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
	auto const pThis = (BuildingClass*)this->This();
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

			if (nValue > 0.0 && pThis->GetHealthPercentage() <= nValue)
			{
				pThis->Sell(-1);
				return;
			}
		}

		if (AutoSellTimer.Completed())
		{
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

	auto currentBuilding = (BuildingClass*)This();
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
	const auto pThis = (BuildingClass*)this->This();

	for (auto i = 0; i < this->Type->GetSuperWeaponCount(); ++i)
	{
		if (this->Type->GetSuperWeaponIndex(i, pThis->Owner) == index)
		{
			return true;
		}
	}

	if (withUpgrades)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(pUpgrade))
			{
				for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i)
				{
					if (pUpgradeExt->GetSuperWeaponIndex(i, pThis->Owner) == index)
					{
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

	AnnounceInvalidPointer(this->CurrentAirFactory, ptr, bRemoved);
	AnnounceInvalidPointer<TechnoClass*>(this->RegisteredJammers, ptr, bRemoved);

	this->MyPrismForwarding->InvalidatePointer(ptr, bRemoved);
}

void BuildingExtData::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array->Items[idxStorageTiberiumType];
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array->Items[idxTiberiumType];

	if (amount > 0.0) {
		if (auto pBuildingType = pThis->Type) {
			if (BuildingTypeExtContainer::Instance.Find(pBuildingType)->Refinery_UseStorage) {
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

	if (pBuilding->Owner->IsAlliedWith(pTechno))
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner.Get())
			return false;

		if (pBuilding->Owner != pTechno->Owner && !pExt->Grinding_AllowAllies.Get())
			return false;

		auto const pTechnoType = GET_TECHNOTYPE(pTechno);

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

	auto const pTechnoType = GET_TECHNOTYPE(pTechno);

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
			auto coord = pTechno->GetCoords();
			VocClass::SafeImmedietelyPlayAt(pTypeExt->Grinding_Sound.Get(), &coord);
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
		if (((BuildLimitStatus)HouseExtData::BuildLimitGroupCheck(pOwner, pType, true, false)) != BuildLimitStatus::NotReached && HouseExtData::CheckBuildLimit(pOwner, pType, true) != BuildLimitStatus::NotReached)
		{
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
		pOwner->Buildings.push_back(pBuilding);

		pOwner->ActiveBuildingTypes.increment(pBuilding->Type->ArrayIndex);
		pOwner->UpdateSuperWeaponsUnavailable();

		auto const pBuildingExt = BuildingExtContainer::Instance.Find(pBuilding);

		HouseExtData::UpdateFactoryPlans(pBuilding);

		if (BuildingTypeExtContainer::Instance.Find(pType)->Academy)
			HouseExtContainer::Instance.Find(pOwner)->UpdateAcademy(pBuilding, true);

		if (pType->SecretLab)
		{
			pOwner->SecretLabs.push_back(pBuilding);
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

		if (!HouseExtContainer::Instance.AutoDeathObjects.contains(pBuilding))
		{
			KillMethod nMethod = pBuildingExt->Type->Death_Method.Get();

			if (nMethod != KillMethod::None)
			{

				if (pBuildingExt->Type->Death_Countdown > 0)
					pBuildingExt->Death_Countdown.Start(pBuildingExt->Type->Death_Countdown);

				HouseExtContainer::Instance.AutoDeathObjects.emplace_unchecked(pBuilding, nMethod);
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
	pTargetHouse->Buildings.erase(pBuilding);
	pTargetHouse->RegisterLoss(pBuilding, false);
	//pTargetHouse->RemoveTracking(pBuilding);

	pTargetHouse->ActiveBuildingTypes.decrement(pBuilding->Type->ArrayIndex);

	// Building logics
	if (pType->ConstructionYard)
		pTargetHouse->ConYards.erase(pBuilding);

	if (pType->SecretLab)
		pTargetHouse->SecretLabs.erase(pBuilding);

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

	for (auto& pBaseNode : pTargetHouse->Base.BaseNodes)
	{
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

		if (wasDamaged && this->Sys.Damage)
			this->Sys.Damage->UnInit();
	}

	const auto sound = this->_GetTypeExtData()->BuildingRepairedSound.Get(RulesClass::Instance->BuildingRepairedSound);
	auto coord = this->GetCoords();
	VocClass::SafeImmedietelyPlayAt(sound, &coord);
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

		if (wasDamaged && this->Sys.Damage)
			this->Sys.Damage->UnInit();
	}

	const auto sound = this->_GetTypeExtData()->BuildingRepairedSound.Get(RulesClass::Instance->BuildingRepairedSound);
	auto coord = this->GetCoords();
	VocClass::SafeImmedietelyPlayAt(sound, &coord);
}

void FakeBuildingClass::UnloadOccupants(bool assignMission, bool killIfStuck)
{

	this->FiringOccupantIndex = 0;

	if (!this->Occupants.Count)
		return;

	CoordStruct defaultCoord = CoordStruct::Empty;
	CellStruct originCell = this->GetMapCoords();
	CoordStruct scatterCoord {};
	CellStruct fallbackCell {};
	bool foundValidCell = false;

	const int width = this->Type->GetFoundationWidth();
	const int height = this->Type->GetFoundationHeight(false);
	const int startX = originCell.X;
	const int startY = originCell.Y;
	const int endX = startX + width;
	const int endY = startY + height;

	InfantryClass* firstOccupant = this->Occupants.Items[0];
	// Define directional priority: up, left, right, down
	static COMPILETIMEEVAL std::array<CellStruct, 4u> directions = { {
		{0, -1},  // up
		{-1, 0},  // left
		{1, 0},   // right
		{0, 1}   // down
	} };

	// Try to find a valid adjacent cell to unload into
	for (const auto& [dx, dy] : directions)
	{
		CellStruct tryCell = { (short)endX, (short)endY };

		while (true)
		{
			tryCell.X += dx;
			tryCell.Y += dy;

			// Stop if out of building bounds
			if (tryCell.X < startX || tryCell.X >= endX ||
				tryCell.Y < startY || tryCell.Y >= endY)
				break;

			CellClass* mapCell = MapClass::Instance->GetCellAt(tryCell);
			if (firstOccupant->IsCellOccupied(mapCell, FacingType::None, -1, nullptr, true) == Move::OK)
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
			this->Mark(MarkType::Change);
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
			if (!foot->Owner)
			{
				Debug::FatalErrorAndExit("BuildingClass::KickAllOccupants for [%x(%s)] Missing Occupier [%x(%s)] House Pointer !",
					this,
					this->get_ID(),
					foot,
					foot->get_ID()
				);
			}

			if (foot->Owner->IsInPlayerControl)
			{
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
	this->Occupants.reset();
	this->Mark(MarkType::Change);
	// Update threat map
	this->UpdateThreatInCell(this->GetCell());
}

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

int ProcessNukeSilo(BuildingClass* pThis, SuperClass* pLinked, SWTypeExtData* pLinkedTypeExt)
{
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

		if (AnimClass* anim =
			BulletClass::CreateDamagingBulletAnim(house,
				v3,
				nullptr,
				SWTypeExtContainer::Instance.Find(pLinked->Type)->Nuke_PsiWarning
			))
		{

			anim->SetBullet(nullptr);
			anim->SetHouse(house);
			anim->Invisible = true;
			pThis->PsiWarnAnim = anim;
		}

		// fall through to case 1
	}
	case NukeFiringState::Aux1:
	{
		if (pThis->IsReadyToCommence)
		{
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
				BulletExtContainer::Instance.Find(pCreated)->NukeSW = pLinkedTypeExt->This();
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
				//auto nCos = Math::cos(Math::Math::PI_BY_TWO_ACCURATE);
				//auto nSin = Math::sin(Math::Math::PI_BY_TWO_ACCURATE);

				const auto nMult = pCreated->Type->Vertical ? 10.0 : 100.0;
				//const auto nX = nCos * nCos * nMult;
				//const auto nY = nCos * nSin * nMult;
				//const auto nZ = nSin * nMult;

				if (!pCreated->MoveTo(nFLH, { 0.0, 0.0 , nMult }))
				{
					GameDelete<true, false>(pCreated);
				}
				else
				{
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
			pThis,
			WeaponType->Damage,
			WeaponType->Warhead,
			255,
			true)
			)
		{
			Bullet->SetWeaponType(WeaponType);

			if (auto pPSIWarn = pThis->PsiWarnAnim)
			{
				pPSIWarn->SetBullet(Bullet);
				pThis->PsiWarnAnim = nullptr;
			}

			Bullet->Limbo();

			CoordStruct coord {};
			pThis->GetFLH(&coord, 0, CoordStruct::Empty);

			VelocityClass velocity {};
			double angle = Math::Math::PI_BY_TWO_ACCURATE;
			double speed = 10.0;

			double sinA = Math::sin(angle);
			double cosA = Math::cos(angle);

			velocity.X = cosA * sinA * speed;
			velocity.Y = sinA * speed;
			velocity.Z = cosA * cosA * speed;

			if (Bullet->MoveTo(coord, velocity))
			{
				CoordStruct coord = v3->GetCoords();
				AnimClass* anim = GameCreate<AnimClass>(RulesClass::Instance->NukeTakeOff, coord, 0, 1, AnimFlag::AnimFlag_600, 0, 0);
				anim->ZAdjust = -100;
				pThis->MissionStatus = 3;
			}
			else
			{
				GameDelete<true, false>(Bullet);
				pThis->MissionStatus = 3;
			}
		}
#endif

		return 1;
	}
	case  NukeFiringState::Aux2:
	{
		pThis->BeginMode(BStateType::Aux2);
		pThis->MissionStatus = 4;
		return TIMER_SECOND / 10;
	}
	case  NukeFiringState::Idle:
	{
		pThis->BeginMode(BStateType::Idle);
		pThis->QueueMission(Mission::Guard, false);
		return TIMER_SECOND;
	}
	default:
		//apply the missioncontrol rate as delay
		return int(pThis->GetCurrentMissionControl()->Rate * TICKS_PER_MINUTE);
	}
}

int ProcessEMPUlseCannon(BuildingClass* pThis, SuperClass* pLinked, SWTypeExtData* pLinkedTypeExt)
{

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	enum class EMPulseFiringState : int {
		Preparing, PlayPulseBall, SentWeaponPayload, RestoreFacing
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

		if (Math::abs(prim.Current().Raw - dirPrimary.Raw))
		{
			prim.Set_Desired(dirPrimary);
		}
		else
		{
			DirStruct dirBarrel {};
			pThis->GetFacingAgainst(&dirBarrel, v21);
			auto& barr = pThis->BarrelFacing;

			if (Math::abs(barr.Current().Raw - dirBarrel.Raw))
			{
				barr.Set_Desired(dirBarrel);
			}
			else
			{
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

		if (pLinkedTypeExt->EMPulse_PulseDelay >= 0) //if negative value are used , just fallthru the function
			return pLinkedTypeExt->EMPulse_PulseDelay;
	}
	case EMPulseFiringState::SentWeaponPayload:
	{
		//HouseClass* house = pThis->Owner;
		const auto celltarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->EMPTarget;

		// If no valid target or destination, reset to idle
		if (Unsorted::ArmageddonMode() || !celltarget.IsValid())
		{
			pThis->BeginMode(BStateType::Idle);
			pThis->QueueMission(Mission::Guard, false);
			return 60;
		}
		WeaponTypeClass* weaponType = pThis->GetWeapon(pExt->idxSlot_EMPulse)->WeaponType;
		AbstractClass* target = MapClass::Instance->GetCellAt(celltarget);

		// Aim the barrel
		DirStruct dirBarrel {};
		pThis->GetFacingAgainst(&dirBarrel, target);
		pThis->BarrelFacing.Set_Desired(dirBarrel);

		// Prepare bullet trajectory
		CoordStruct flhCoord {};
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

		if (!bullet)
		{
			// No bullet created, return to idle
			pThis->BeginMode(BStateType::Idle);
			pThis->QueueMission(Mission::Guard, false);
			return 60;
		}

		bullet->SetWeaponType(weaponType);

		// --- Compute horizontal angle ---
		double targetRadX = (targetCoord.X);
		double targetRadY = (targetCoord.Y);

		double angleToTarget = std::atan2(
			((double)flhCoord.Y - targetRadY),
			(targetRadX - (double)flhCoord.X)
		);

		// Offset by -90 degrees
		angleToTarget -= Math::DEG90_AS_RAD;

		// Convert to binary angle
		int binaryAngle = (int)(angleToTarget * Math::BINARY_ANGLE_MAGIC);

		// --- Base velocity calculation ---
		double radians = (double)(binaryAngle - 16383) * Math::DIRECTION_FIXED_MAGIC;

		VelocityClass vel { Math::cos(radians) * Math::SQRT_TENTOUSAND  , -(Math::sin(radians) * Math::SQRT_TENTOUSAND)  , 0.0 };
		vel.SetIfZeroXYZ();

		// Normalize and scale to projectile speed
		double length = vel.Length();
		double scale = speed / length;

		vel *= scale;

		// --- Adjust for weapon direction ---
		CoordStruct barrelDir {};
		pThis->vt_entry_300(&barrelDir, pExt->idxSlot_EMPulse);
		CoordStruct bulletPos = bullet->GetCoords();
		CoordStruct xyDiff = bulletPos - barrelDir;
		double dz = xyDiff.Z;
		double xyDist = xyDiff.Length();

		// Recalculate direction if needed
		DirStruct legal;
		const bool canReach = pThis->CanReachTarget(pExt->idxSlot_EMPulse);
		if (!Game::func_48A8D0_Legal(canReach, speed, xyDist, dz, gravity, &legal))
		{
			if (!Game::func_48A8D0_Legal(canReach, (10 * speed) / 8, xyDist, dz, gravity, &legal))
			{
				legal = DirStruct((unsigned short)-1536);
			}
		}

		// --- Pitch adjustment ---
		double pitch = std::atan2(vel.Z, vel.LengthXY()) - Math::DEG90_AS_RAD;
		int pitchBinary = (int)(pitch * Math::BINARY_ANGLE_MAGIC) - Math::BINARY_ANGLE_MASK;
		double pitchRad = (double)pitchBinary * Math::DIRECTION_FIXED_MAGIC;

		if (pitchRad != 0.0)
		{
			vel.X /= Math::cos(pitchRad);
			vel.Y /= Math::cos(pitchRad);
		}

		// --- Facing adjustment ---
		double dirRad = ((double)legal.Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;
		vel.X *= Math::cos(dirRad);
		vel.Y *= Math::cos(dirRad);
		vel.Z = Math::sin(dirRad) * speed;


		auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(bullet->Type);
		auto pBulletExt = BulletExtContainer::Instance.Find(bullet);

		if (bullet->Type->Arcing && !pBulletTypeExt->Arcing_AllowElevationInaccuracy)
		{
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
		if (!pThis->Type->IsGattling && weaponType->Report.Count > 0)
		{
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

int ProcessMissionMissile(BuildingClass* pThis, SuperClass* pLinked, SWTypeExtData* pLinkedTypeExt)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pThis->Type->EMPulseCannon)
	{
		const auto pTarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->NukeTarget;

		pThis->Fire(MapClass::Instance->GetCellAt(pTarget), 0);
		pThis->QueueMission(Mission::Guard, false);

		return 1; //ares
	}
	else
	{
		return ProcessEMPUlseCannon(pThis, pLinked, pLinkedTypeExt);
	}

}

int FakeBuildingClass::_Mission_Missile()
{

	if (this->FiringSWType < 0)
	{
		Debug::LogInfo("Building[{}] with Mission::Missile Missing Important Linked SW data !", this->get_ID());
		//apply the missioncontrol rate as delay
		return int(this->GetCurrentMissionControl()->Rate * TICKS_PER_MINUTE);
	}

	auto pSW = this->Owner->Supers.Items[this->FiringSWType];
	auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pSW->Type);

	if (this->Type->NukeSilo)
	{
		return ProcessNukeSilo(this, pSW, pSWTypeExt);
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

	for (auto& nFires : pExt->DamageFireAnims)
	{
		if (nFires && nFires->Type)
		{
			nFires->TimeToDie = true;
			nFires->UnInit();
			nFires = nullptr;
		}
	}

	auto const& pFire = pTypeext->DamageFireTypes.GetElements(RulesClass::Instance->DamageFireTypes);

	if (!pFire.empty())
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

class NOVTABLE FakeTacticalClassB : public TacticalClass {
public:

	static COMPILETIMEEVAL reference<FakeTacticalClassB*, 0x887324u> const Instance {};

	Point2D* XY_To_Screen_Pixels(Point2D* pBuffer, Point2D* a2)
	{ JMP_THIS(0x6D1FE0); }

	//static int __fastcall ZDepth_Adjust_For_Height(int z) {
	//	JMP_FAST(0x6D20E0);
	//}
};

#include <Misc/Ares/Hooks/Header.h>

// Calculate the mask once at initialization (assuming you know ColorStruct at startup)
COMPILETIMEEVAL WORD BuildPcxMask()
{
	return (0xFFu >> ColorStruct::BlueShiftRight << ColorStruct::BlueShiftLeft)
		| (0xFFu >> ColorStruct::RedShiftRight << ColorStruct::RedShiftLeft);
}

void FakeBuildingClass::_DrawVisible(Point2D* pLocation, RectangleStruct* pBounds)
{
	auto pType = this->Type;

	if (!this->IsSelected || !HouseClass::CurrentPlayer)
		return;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

	//DrawExtraInfo has internal checking to determine what can or cannot be drawn
	//we follow those check instead of check below
	Point2D DrawExtraLoc = { pLocation->X , pLocation->Y };
	this->DrawExtraInfo(&DrawExtraLoc, pLocation, pBounds);

	// helpers (with support for the new spy effect)
	const bool bAllied = this->Owner->IsAlliedWith(HouseClass::CurrentPlayer);
	const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
	const bool bReveal = pTypeExt->SpyEffect_RevealProduction && this->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

	// show building or house state
	if (bAllied || IsObserver || bReveal)
	{

		// display production cameo
		if (IsObserver || bReveal)
		{
			const auto pFactory = this->Owner->IsControlledByHuman() ?
				this->Owner->GetPrimaryFactory(pType->Factory, pType->Naval, BuildCat::DontCare)
				: this->Factory;

			if (pFactory && pFactory->Object)
			{
				auto pProdType = GET_TECHNOTYPE(pFactory->Object);
				//const int nTotal = pFactory->CountTotal(pProdType);
				Point2D DrawCameoLoc = { pLocation->X , pLocation->Y + 45 };
				const auto pProdTypeExt = TechnoTypeExtContainer::Instance.Find(pProdType);
				RectangleStruct cameoRect {};

				// support for pcx cameos
				if (auto pPCX = TechnoTypeExt_ExtData::GetPCXSurface(pProdType, this->Owner))
				{
					const int cameoWidth = 60;
					const int cameoHeight = 48;

					RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
					RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
					RectangleStruct destRect = { DrawCameoLoc.X - cameoWidth / 2, DrawCameoLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

					if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
					{
						cameoRect = destRect;
						if (!StaticVars::InitEd)
						{
							StaticVars::GlobalPcxBlitter = AresPcxBlit<WORD>(BuildPcxMask(), 60, 48, 2);
							StaticVars::InitEd = true;
						}

						Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds, &StaticVars::GlobalPcxBlitter, 0, 3, 1000, 0);

					}
				}
				else
				{
					// old shp cameos, fixed palette
					if (auto pCameo = pProdType->GetCameo())
					{
						cameoRect = { DrawCameoLoc.X, DrawCameoLoc.Y, pCameo->Width, pCameo->Height };

						ConvertClass* pPal = FileSystem::CAMEO_PAL();
						if (auto pManager = pProdTypeExt->CameoPal.GetConvert())
							pPal = pManager;

						DSurface::Temp->DrawSHP(pPal, pCameo, 0, &DrawCameoLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
					}
				}

				int prog = pFactory->GetProgress();
				{
					Point2D textLoc = { cameoRect.X + cameoRect.Width / 2, cameoRect.Y };
					const auto percent = int(((double)prog / 54.0) * 100.0);
					static fmt::basic_memory_buffer<wchar_t> text_;
					text_.clear();
					fmt::format_to(std::back_inserter(text_), L"{}", percent);
					text_.push_back(L'\0');
					RectangleStruct nTextDimension {};
					COMPILETIMEEVAL TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;
					Drawing::GetTextDimensions(&nTextDimension, text_.data(), textLoc, printType, 4, 2);
					auto nIntersect = RectangleStruct::Intersect(nTextDimension, *pBounds, nullptr, nullptr);
					const COLORREF foreColor = this->Owner->Color.ToInit();
					DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
					DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)foreColor);
					DSurface::Temp->DrawText_Old(text_.data(), pBounds, &textLoc, (DWORD)foreColor, 0, (DWORD)printType);
				}

			}
			else if (pType->SuperWeapon != -1)
			{
				SuperClass* const pSuper = this->Owner->Supers.Items[pType->SuperWeapon];

				if (pSuper->RechargeTimer.TimeLeft > 0 && SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_ShowCameo)
				{
					RectangleStruct cameoRect {};
					Point2D DrawCameoLoc = { pLocation->X , pLocation->Y + 45 };

					// support for pcx cameos
					if (auto pPCX = SWTypeExtContainer::Instance.Find(pSuper->Type)->SidebarPCX.GetSurface())
					{
						const int cameoWidth = 60;
						const int cameoHeight = 48;

						RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
						RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
						RectangleStruct destRect = { DrawCameoLoc.X - cameoWidth / 2, DrawCameoLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

						if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
						{
							cameoRect = destRect;
							if (!StaticVars::InitEd)
							{
								StaticVars::GlobalPcxBlitter = AresPcxBlit<WORD>(BuildPcxMask(), 60, 48, 2);
								StaticVars::InitEd = true;
							}

							Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds, &StaticVars::GlobalPcxBlitter, 0, 3, 1000, 0);
						}

					}
					else
					{
						// old shp cameos, fixed palette
						if (auto pCameo = pSuper->Type->SidebarImage)
						{
							cameoRect = { DrawCameoLoc.X, DrawCameoLoc.Y, pCameo->Width, pCameo->Height };

							ConvertClass* pPal = FileSystem::CAMEO_PAL();
							if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper->Type)->SidebarPalette.GetConvert())
								pPal = pManager;

							DSurface::Temp->DrawSHP(pPal, pCameo, 0, &DrawCameoLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
						}
					}
				}
			}
		}
	}

	return;
}

void FakeBuildingClass::_DrawExtras(Point2D* pLocation, RectangleStruct* pBounds)
{
	if (this->IsSelected && this->IsOnMap && this->_GetExtData()->LimboID <= -1) {
		const int foundationHeight = this->Type->GetFoundationHeight(0);
		const int typeHeight = this->Type->Height;
		const int yOffest = (Unsorted::CellHeightInPixels * (foundationHeight + typeHeight)) >> 2;

		Point2D centeredPoint = { pLocation->X, pLocation->Y - yOffest };
		this->_DrawVisible(&centeredPoint, pBounds);
		//this->DrawInfoTipAndSpiedSelection(&centeredPoint, &DSurface::ViewBounds);
	}

	this->DrawExtras(pLocation, pBounds);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x6D9789, FakeBuildingClass::_DrawExtras);
void FakeBuildingClass::_DrawStuffsWhenSelected(Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect)
{
	if (!HouseClass::CurrentPlayer)
		return;

	{
		auto const pType = this->Type;
		auto const pOwner = this->Owner;
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

		if (!pType || !pOwner)
			return;

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
				TextDrawing::Simple_Text_Print_Wide(pFormat, DSurface::Temp.get(), pRect, &DrawLoca, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
				DrawLoca.Y += (nTextDimension.Height) + 2; //extra number for the background
			};

		//everyone can see this regardless
		if ((pType->TechLevel <= 0 || pType->TechLevel > 10) && this->Type->NeedsEngineer && this->Type->Capturable)
		{
			DrawTheStuff(Phobos::UI::Tech_Label);
		}

		// helpers (with support for the new spy effect)
		const bool bAllied = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
		const bool bReveal = pTypeExt->SpyEffect_RevealProduction && this->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);


		if (bAllied || IsObserver || bReveal
			)
		{

			if (pTypeExt->Fake_Of)
				DrawTheStuff(Phobos::UI::BuidingFakeLabel);

			if (pType->PowerBonus > 0 && BuildingTypeExtContainer::Instance.Find(pType)->ShowPower)
			{
				wchar_t pOutDrainFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				//foundating check ,...
				//can be optimized using stored bool instead checking them each frames
				if (pType->GetFoundationWidth() > 2 && pType->GetFoundationHeight(false) > 2)
				{
					swprintf_s(pOutDrainFormat, StringTable::FetchString(GameStrings::TXT_POWER_DRAIN2()), pOutput, pDrain);
				}
				else
				{
					swprintf_s(pOutDrainFormat, Phobos::UI::Power_Label, pOutput);
					DrawTheStuff(pOutDrainFormat);
					swprintf_s(pOutDrainFormat, Phobos::UI::Drain_Label, pDrain);
				}

				DrawTheStuff(pOutDrainFormat);
			}

			const bool hasStorage = pType->Storage > 0;
			bool HasSpySat = false;
			for (auto& _pType : this->GetTypes())
			{
				if (_pType && _pType->SpySat)
				{
					HasSpySat = true;
					break;
				}
			}

			if (hasStorage)
			{

				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, StringTable::FetchString(GameStrings::TXT_MONEY_FORMAT_1()), nMoney);
				DrawTheStuff(pOutMoneyFormat);

				if (BuildingTypeExtContainer::Instance.Find(pType)->Refinery_UseStorage)
				{
					wchar_t pOutStorageFormat[0x80];
					auto nStorage = this->GetStoragePercentage();
					swprintf_s(pOutStorageFormat, Phobos::UI::Storage_Label, nStorage);
					DrawTheStuff(pOutStorageFormat);
				}
			}

			if (this->IsPrimaryFactory)
			{
				if (SHPStruct* pImage = RulesExtData::Instance()->PrimaryFactoryIndicator)
				{
					ConvertClass* pPalette = FileSystem::PALETTE_PAL();
					if (auto pPall_c = RulesExtData::Instance()->PrimaryFactoryIndicator_Palette.GetConvert())
						pPalette = pPall_c;

					int const cellsToAdjust = pType->GetFoundationHeight(false) - 1;
					Point2D pPosition = TacticalClass::Instance->CoordsToClient(this->GetCell()->GetCoords());
					pPosition.X -= Unsorted::CellWidthInPixels / 2 * cellsToAdjust;
					pPosition.Y += Unsorted::CellHeightInPixels / 2 * cellsToAdjust - 4;
					DSurface::Temp->DrawSHP(pPalette, pImage, 0, &pPosition, pRect, BlitterFlags(0x600), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				}
				else
				{
					DrawTheStuff(StringTable::FetchString((pType->GetFoundationWidth() != 1) ?
						GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
				}
			}

			if (pType->Radar || HasSpySat)
			{

				if (pType->Radar)
				{
					DrawTheStuff(Phobos::UI::Radar_Label);
				}

				if (HasSpySat)
				{
					DrawTheStuff(Phobos::UI::Spysat_Label);
				}

				if (!this->_GetExtData()->RegisteredJammers.empty())
					DrawTheStuff(Phobos::UI::BuidingRadarJammedLabel);

			}
		}
	}
}

void FakeBuildingClass::_Draw_It(Point2D* screenPos, RectangleStruct* clipRect)
{
	if (SHPStruct* mainShape = this->GetImage()) {

		BuildingTypeClass* buildingType = this->Type;

		if (buildingType->InvisibleInGame) {
			return;
		}

		auto curMission = this->GetCurrentMission();

		if (this->BState == BStateType::Construction && curMission == Mission::Selling) {
			for (auto& anim : this->Anims) {
				if (anim) {
					anim->Invisible = true;
				}
			}
		}

		CellStruct cellPos = this->GetMapCoords();

		auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->Type);

		if (pTypeExt->IsHideDuringSpecialAnim &&
			(this->Anims[(int)BuildingAnimSlot::Special] ||
			this->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
			this->Anims[(int)BuildingAnimSlot::SpecialThree]))
			return;

		bool isUnloadingAirUnit = false;
		if (curMission == Mission::Unload) {
			if (TechnoClass* contactUnit = this->GetRadioContact()) {
				TechnoTypeClass* unitType = GET_TECHNOTYPE(contactUnit);
				if (unitType->JumpJet || unitType->BalloonHover)
				{
					isUnloadingAirUnit = true;
				}
			}
		}

		auto pCell = MapClass::Instance->GetCellAt(cellPos);
		auto pCenterCell = MapClass::Instance->GetCellAt(this->GetCoords());

		int tintLevel = 0;
		if (!pCenterCell->IsShrouded()) {
			tintLevel = TechnoExtData::ApplyTintColor(this, true, true, false);
			TechnoExtData::ApplyCustomTint(this, &tintLevel, nullptr);
		}

		int NormalZAdjust = buildingType->NormalZAdjust;
		auto& door = this->UnloadTimer;

		// Handle gate/door animations
		if (curMission == Mission::Open && (door.IsOpening() || door.IsClosing() || door.IsOpen()))
		{

			// Calculate gate frame based on door state
			int gateFrame = (int)(door.GetCompletePercent()
						* this->Type->GateStages);

			if (door.IsClosing())
			{
				gateFrame = this->Type->GateStages - gateFrame;
			}

			if (door.IsClosed())
			{
				gateFrame = 0;
			}

			if (door.IsOpen())
			{
				gateFrame = this->Type->GateStages - 1;
			}

			// Clamp frame to valid range
			gateFrame = MaxImpl(0, MinImpl(gateFrame, this->Type->GateStages - 1));

			// Add damage frame offset if building is damaged
			if (this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
			{
				gateFrame += this->Type->GateStages + 1;
			}

			ZGradient zgrad = ZGradient::Ground;

			if (gateFrame < this->Type->GateStages / 2 || pTypeExt->IsBarGate)
			{
				zgrad = ZGradient::Deg90;
			}

			const int lightLevel = pCell->Color1.Red;
			const int depthAdjust = Game::AdjustHeight(this->GetZ());

			this->Draw_Object(mainShape,
				gateFrame,
				screenPos,
				clipRect,
				DirType::North,  // rotation
				256,  // scale
				NormalZAdjust - depthAdjust,  // height adjust
				zgrad,  // ZGradient
				1,  // useZBuffer
				lightLevel,
				tintLevel,
				0, 0, 0, 0, BlitterFlags::None  // z-shape params
			);

			return;
		}

		// Handle special unload animations
		if (curMission == Mission::Unload)
		{
			if (isUnloadingAirUnit)
			{
				if (this->Type->RoofDeployingAnim)
				{
					mainShape = this->Type->RoofDeployingAnim;
					NormalZAdjust = -40;
				}
			}
			else
			{
				if (this->Type->DeployingAnim)
				{
					mainShape = this->Type->DeployingAnim;
					NormalZAdjust = -20;
				}
			}
		}

		// Calculate shape positioning
		// Default shape offset values (198 = 0xC6, 446 = 0x1BE)
		Point2D shapeOffset = { 198, 446 };

		if ((curMission != Mission::Construction && curMission != Mission::Selling) || pTypeExt->ZShapePointMove_OnBuildup) {
			shapeOffset += this->Type->ZShapePointMove;
		}

		const auto foundationWidth = this->Type->GetFoundationWidth();
		const auto foundationHeiht = this->Type->GetFoundationHeight(0);

		// Calculate screen position adjustment
		const Point2D buildingSize {
			.X = (foundationWidth * Unsorted::LeptonsPerCell) - Unsorted::LeptonsPerCell
			,
			.Y = (foundationHeiht * Unsorted::LeptonsPerCell) - Unsorted::LeptonsPerCell
		};

		Point2D screenOffset = TacticalClass::Instance->AdjustForZShapeMove(buildingSize.X, buildingSize.Y);
				shapeOffset -= screenOffset;

		// Draw main building if clipping rect has height
		if (clipRect->Height > 0) {
			SHPStruct* zShape = foundationWidth < 8 ? FileSystem::BUILDINGZ_SHA() : nullptr;
			const int lightLevel = (int16)this->Type->ExtraLight + pCell->Color1.Red;
			const int depthAdjust = Game::AdjustHeight(this->GetZ());

			if (!pTypeExt->Firestorm_Wall) {
				const int frameCount = mainShape->Frames;
				const auto currentFrame = this->GetShapeNumber();
				const int shapeFrame = currentFrame < frameCount / 2 ?
					currentFrame : frameCount / 2;

				this->Draw_Object(mainShape,
						shapeFrame,
						screenPos,
						clipRect,
						DirType::North,  // rotation
						256,  // scale
						NormalZAdjust - depthAdjust,  // height adjust
						ZGradient::Ground,  // ZGradient
						1,  // useZBuffer
						lightLevel,
						tintLevel,
						zShape,
						0,
						shapeOffset.X ,
						shapeOffset.Y ,
						BlitterFlags::None  // flags
				);
			} else {
				this->Draw_Object(mainShape,
					this->GetShapeNumber(),
					screenPos,
					clipRect,
					DirType::North,  // rotation
					256,  // scale
					-1 - depthAdjust,  // height adjust
					ZGradient::Deg90,  // ZGradient
					1,  // useZBuffer
					lightLevel,
					tintLevel,
					zShape,
					0,
					shapeOffset.X ,
					shapeOffset.Y ,
					BlitterFlags::None  // flags
				);
			}
		}

		//// Draw bib (foundation) if present
		if (this->Type->BibShape && this->BState != BStateType::Construction && this->ActuallyPlacedOnMap) {
			const int lightLevel = (int)(this->Type->ExtraLight + pCell->Color1.Red);
			const int heightZ = this->GetZ();

			this->Draw_Object(
				this->Type->BibShape,
				this->GetShapeNumber(),
				screenPos,
				clipRect,
				DirType::North,  // rotation
				256,  // scale
				-1 - Game::AdjustHeight(heightZ),  // height adjust
				ZGradient::Ground,  // ZGradient
				1,  // useZBuffer
				lightLevel,
				tintLevel,
				0, 0, 0, 0, BlitterFlags::None  // z-shape params
			);
		}

		// Draw special door animations for unload mission
		if (curMission == Mission::Unload) {
			if (const auto RoofAnim = isUnloadingAirUnit ? this->Type->UnderRoofDoorAnim : this->Type->UnderDoorAnim) {
				this->Draw_Object(
					RoofAnim,
					this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow,
					screenPos,
					clipRect,
					DirType::North, 256,  // rotation, scale
					-Game::AdjustHeight(this->GetZ()),  // height adjust
					ZGradient::Ground, 1,  // ZGradient, useZBuffer
					 (int16)this->Type->ExtraLight + pCell->Color1.Red,
					tintLevel,
					0, 0, 0, 0, BlitterFlags::None  // z-shape params
				);
			}
		}
	}
}

//DEFINE_HOOK(0x43D290,BuildingClass_Draw_It, 0x5)
//{
//	GET(FakeBuildingClass*, pThis, ECX);
//	GET_STACK(Point2D*, pPoint, 0x4);
//	GET_STACK(RectangleStruct*, pRect, 0x8);
//	DWORD stored = R->EDI();
//	pThis->_Draw_It(pPoint, pRect);
//	R->EDI(stored);
//	return 0x43DA73;
//}
//
//DEFINE_FUNCTION_JUMP(CALL6, 0x43D005 , FakeBuildingClass::_Draw_It)

int FakeBuildingClass::_BuildingClass_GetRangeOfRadial()
{
	BuildingTypeClass* pType = this->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->RadialIndicatorRadius.isset())
		return pTypeExt->RadialIndicatorRadius.Get();

	if (pType->PsychicDetectionRadius <= 0) {
		if (pType->GapGenerator) {
			if (this->GapSuperCharged) {
				return pTypeExt->SuperGapRadiusInCells;
			} else {
				return pTypeExt->GapRadiusInCells;
			}
		} else if (pType->CloakGenerator) {
			return pType->CloakRadiusInCells;
		} else if (pType->SensorArray) {
			return pType->SensorsSight;
		} else {
			auto pWeapon = this->GetPrimaryWeapon();
			if (pWeapon && pWeapon->WeaponType) {
				const int range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon->WeaponType, this);
				if(range > 0)
					return range / 256;
			} else {
				return 0;
			}
		}
	}

	return pType->PsychicDetectionRadius;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6DBEA5, FakeBuildingClass::_BuildingClass_GetRangeOfRadial)
DEFINE_FUNCTION_JUMP(CALL, 0x6DBF76, FakeBuildingClass::_BuildingClass_GetRangeOfRadial)

bool CanDrawRadialIndicator(FakeBuildingClass* pThis) {
	auto pBldTypeExt = pThis->_GetTypeExtData();

	if (HouseExtData::IsObserverPlayer())
		return true;

	if (!pBldTypeExt->AlwayDrawRadialIndicator.Get(pThis->HasPower))
		return false;

	if (pBldTypeExt->RadialIndicator_Visibility.isset()) {
		if (EnumFunctions::CanTargetHouse(pBldTypeExt->RadialIndicator_Visibility.Get(), pThis->Owner, HouseClass::CurrentPlayer()))
			return true;
	} else {
		if (pThis->Owner == HouseClass::CurrentPlayer())
			return true;
	}

	return false;
}

void FakeBuildingClass::_DrawRadialIndicator(int val)
{
	if (!CanDrawRadialIndicator(this))
		return;

	const int radius = this->_BuildingClass_GetRangeOfRadial();

	if (radius <= 0)
		return;

	const bool concentricMode = this->Type->ConcentricRadialIndicator;
	const ColorStruct laserColor = this->Owner->LaserColor;
	CoordStruct center = this->GetCoords();

	// ------------------------------------------------------------------------------------
	// CASE 1: Concentric radial indicator
	// ------------------------------------------------------------------------------------
	if (concentricMode) {
		// Color modulated by time-based sine-wave rhythm
		DWORD timeMs = timeGetTime();
		DWORD t = (timeMs >> 1);
		int wave = t & 0x3FF;     // 0..1023 pattern

		// Copy color into a working variable
		ColorStruct colorMod = laserColor;

		// If inside first 512 frames…
		if ((t & 0x200) == 0) {
			if ((t & 0x100) == 0) {
				// Adjust color depending on inverted time mask (Ares algorithm)
				colorMod.Adjust(~t, ColorStruct::Empty);
			}

			FakeTacticalClass::__DrawRadialIndicator(
				/*unknown_a*/ 0,
				/*unknown_b*/ 0,
				center,
				colorMod,
				static_cast<float>(radius),
				/*drawBack?*/ 0,
				/*something*/ 1);
		}

		// second ring: scaled pulse effect
		const float amplitude =
			(static_cast<float>(radius) + 0.5f) / Math::SQRT_TWO * 60.0f;

		unsigned scaled = (wave * static_cast<unsigned>(amplitude)) >> 10;

		if (scaled > 0x20)
		{
			FakeTacticalClass::__DrawRadialIndicator(
				0,
				0,
				center,
				laserColor,
				static_cast<float>(scaled),
				1,
				1);
		}
	}
	// ------------------------------------------------------------------------------------
	// CASE 2: Simple radial indicator (no concentric animation)
	// ------------------------------------------------------------------------------------
	else
	{
		FakeTacticalClass::__DrawRadialIndicator(
			/*overrideColor*/ val,
			/*unknown_b*/ 1,
			center,
			laserColor,
			static_cast<float>(radius),
			/*drawBack*/ 0,
			/*something*/ 1
		);
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FEC , FakeBuildingClass::_DrawRadialIndicator)
DEFINE_FUNCTION_JUMP(LJMP, 0x456750, FakeBuildingClass::_DrawRadialIndicator)


InfantryTypeClass* FakeBuildingClass::__GetCrew()
{
	// YR defaults to 25 for buildings producing buildings
	return TechnoExt_ExtData::GetBuildingCrew(this, TechnoTypeExtContainer::Instance.Find(this->Type)->
		Crew_EngineerChance.Get((this->Type->Factory == BuildingTypeClass::AbsID) ? 25 : 0));
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E41C8, FakeBuildingClass::__GetCrew)
DEFINE_FUNCTION_JUMP(LJMP, 0x44EB10, FakeBuildingClass::__GetCrew)

int FakeBuildingClass::__GetCrewCount()
{
	int count = 0;

	if (!this->NoCrew && this->Type->Crewed)
	{
		auto pHouse = this->Owner;

		// get the divisor
		int divisor = HouseExtData::GetSurvivorDivisor(pHouse);

		if (divisor > 0)
		{
			// if captured, less survivors
			if (this->HasBeenCaptured)
			{
				divisor *= 2;
			}

			// value divided by "cost per survivor"
			// clamp between 1 and 5
			count = std::clamp(this->Type->GetRefund(pHouse, 0) / divisor, 1, 5);
		}
	}

	return count;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E418C, FakeBuildingClass::__GetCrewCount)
DEFINE_FUNCTION_JUMP(LJMP, 0x451330, FakeBuildingClass::__GetCrewCount)

const wchar_t* FakeBuildingClass::__GetUIName()
{
	if (HouseClass::CurrentPlayer) {
		const auto pBldOWner = this->Owner;
		if (HouseClass::CurrentPlayer->IsObserver()
			|| HouseClass::CurrentPlayer == pBldOWner
			|| HouseClass::CurrentPlayer->IsAlliedWith(pBldOWner)
			|| this->DisplayProductionTo.Contains(HouseClass::CurrentPlayer->ArrayIndex))
		{
			return this->Type->UIName;
		}
	}

	auto Type = this->Type;
	if (TechnoTypeExtContainer::Instance.Find(this->Type)->Fake_Of)
		Type = (BuildingTypeClass*)TechnoTypeExtContainer::Instance.Find(this->Type)->Fake_Of.Get();

	return Type->UIName;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3F4C, FakeBuildingClass::__GetUIName)
DEFINE_FUNCTION_JUMP(LJMP, 0x459ED0, FakeBuildingClass::__GetUIName)

void FakeBuildingClass::_TechnoClass_Draw_Object(SHPStruct* shapefile,
	int shapenum,
	Point2D* xy,
	RectangleStruct* rect,
	DirType rotation,  //unused
	int scale, //unused
	int height_adjust,
	ZGradient a8,
	bool useZBuffer,
	int lightLevel,
	int tintLevel,
	SHPStruct* z_shape,
	int z_shape_framenum,
	int z_shape_offs_x,
	int z_shape_offs_y,
	BlitterFlags flags)
{
	ZGradient zgrad = ZGradient::Ground;
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->Type);
	if (shapenum < this->Type->GateStages / 2 || pTypeExt->IsBarGate) {
		zgrad = ZGradient::Deg90;
	}

	return this->Draw_Object(shapefile, shapenum, xy, rect, rotation, scale, height_adjust, zgrad, useZBuffer, lightLevel, tintLevel, z_shape, z_shape_framenum, z_shape_offs_x, z_shape_offs_y, flags);
}
DEFINE_FUNCTION_JUMP(CALL, 0x43D683, FakeBuildingClass::_TechnoClass_Draw_Object)

ASMJIT_PATCH(0x43D386, BuildingClass_Draw_TintColor, 0x6)
{
	enum { SkipGameCode = 0x43D4EB };

	GET(BuildingClass*, pThis, ESI);

	int color = TechnoExtData::ApplyTintColor(pThis, true, true, false);
	TechnoExtData::ApplyCustomTint(pThis, &color, nullptr);
	R->EDI(color);

	return SkipGameCode;
}

ASMJIT_PATCH(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
{
	enum { DontDrawBib = 0x43D8EE };

	GET(BuildingClass* const, pThis, ESI);
	return !pThis->ActuallyPlacedOnMap ? DontDrawBib : 0x0;
}

 ASMJIT_PATCH(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
 {
 	enum { Apply = 0x43D6EF, Skip = 0x43D712 };

 	GET(FakeBuildingClass*, pThis, ESI);
 	GET(Mission, mission, EAX);

 	if (
 		(mission != Mission::Selling && mission != Mission::Construction) ||
 			pThis->_GetTypeExtData()->ZShapePointMove_OnBuildup
 		)
 		return Apply;

 	return Skip;
 }

  ASMJIT_PATCH(0x43D290, BuildingClass_Draw_LimboDelivered, 0x5)
 {
 	GET(BuildingClass* const, pBuilding, ECX);

	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (pTypeExt->IsHideDuringSpecialAnim &&
		(pBuilding->Anims[(int)BuildingAnimSlot::Special] ||
			pBuilding->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
			pBuilding->Anims[(int)BuildingAnimSlot::SpecialThree]))
		return 0x43D9D5;

 	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ? 0x43D9D5 : 0x0;
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
		.Process(this->C4Owner)
		.Process(this->C4Warhead)
		.Process(this->Silent)
		.Process(this->DockReloadTimers)
		.Process(this->OwnerBeforeRaid)
		.Process(this->CashUpgradeTimers)
		.Process(this->SensorArrayActiveCounter)
		.Process(this->SecretLab_Placed)
		.Process(this->AboutToChronoshift)
		.Process(this->IsFromSW)
		.Process(this->RegisteredJammers)
		.Process(this->GrindingWeapon_AccumulatedCredits)
		.Process(this->LastFlameSpawnFrame)
		.Process(this->SpyEffectAnim)
		.Process(this->SpyEffectAnimDuration)
		.Process(this->PoweredUpToLevel)
		.Process(this->FactoryBuildingMe)
		.Process(this->airFactoryBuilding)
		.Process(this->FreeUnitDone)
		.Process(this->SeparateRepair)
		;
}

// =============================
// container
BuildingExtContainer BuildingExtContainer::Instance;

bool BuildingExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(BuildingExtContainer::ClassName))
	{
		auto& container = root[BuildingExtContainer::ClassName];

		for (auto& entry : container[BuildingExtData::ClassName])
		{

			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, BuildingExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool BuildingExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[BuildingExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : BuildingExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer); // write all data to stream

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[BuildingExtData::ClassName] = std::move(_extRoot);
	return true;
}
// =============================
// container hooks

ASMJIT_PATCH(0x43B75C, BuildingClass_CTOR, 0x6)
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

void FakeBuildingClass::_Detach(AbstractClass* target, bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	this->BuildingClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3EE4, FakeBuildingClass::_Detach)

void FakeBuildingClass::_DetachAnim(AnimClass* pAnim)
{
	this->TechnoClass::AnimPointerExpired(pAnim);

	auto pExt = BuildingExtContainer::Instance.Find(this);

	if (pAnim == pExt->SpyEffectAnim.get())
	{
		pExt->SpyEffectAnim.release();
	}
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3F1C, FakeBuildingClass::_DetachAnim)
