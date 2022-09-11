#include "Body.h"
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;
void BuildingExt::ExtData::InitializeConstants()
{
	if (!Get() || !Get()->Type)
		return;

#ifdef ENABLE_NEWHOOKS
	if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(Get()->Type)) {
		if (pTypeExt->DamageFire_Offs.Count > 0)
			DamageFireAnims.reserve((size_t)pTypeExt->DamageFire_Offs.Count);
	}
#endif
}

void BuildingExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(CurrentAirFactory, ptr);
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
			if (auto const pExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
			{
				if (pExt->Refinery_UseStorage)
				{
					// Store Tiberium in structures
					depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
					pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
				}
			}
		}
	}
}

void BuildingExt::UpdatePrimaryFactoryAI(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto BuildingExt = BuildingExt::ExtMap[pThis];
	auto HouseExt = HouseExt::ExtMap[pOwner];

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
				BuildingExt->CurrentAirFactory= nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	std::for_each(pOwner->Buildings.begin(), pOwner->Buildings.end(), [&](BuildingClass* pBuilding) {
		if (!pBuilding || !pBuilding->Type)
			return;

		if (pBuilding->Type->Factory == AbstractType::AircraftType && Phobos::Config::ForbidParallelAIQueues_Aircraft)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			const auto It = std::find_if_not(std::begin(HouseExt->HouseAirFactory), std::end(HouseExt->HouseAirFactory), [&](const auto pData)
			{
				return pData == pBuilding;
			});

			if(It == std::end(HouseExt->HouseAirFactory))
			HouseExt->HouseAirFactory.push_back(pBuilding);
		}
	});

	if (BuildingExt->CurrentAirFactory)
	{
		std::for_each(HouseExt->HouseAirFactory.begin(), HouseExt->HouseAirFactory.end(), [&](BuildingClass* pBuilding) {
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

	std::for_each(HouseExt->HouseAirFactory.begin(), HouseExt->HouseAirFactory.end(), [&](BuildingClass* pBuilding) {
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

	if (!pBuilding || !pBuilding->Type || pBuilding->WhatAmI() != AbstractType::Building)
		return 0;

	if (pBuilding->RadioLinks.IsAllocated && pBuilding->RadioLinks.IsInitialized)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->GetNthLink(i))
				if(pLink->WhatAmI() == AbstractType::Aircraft)
					nOccupiedDocks++;
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
	if (!pBuilding->Type->Grinding)
		return false;

	if (pTechno->WhatAmI() != AbstractType::Infantry && pTechno->WhatAmI() != AbstractType::Unit)
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb) &&
		(pTechno->WhatAmI() == AbstractType::Infantry && !pBuilding->Type->InfantryAbsorb ||
			pTechno->WhatAmI() == AbstractType::Unit && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
	{
		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner.Get())
			return false;

		if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies.Get())
			return false;

		if (pExt->Grinding_AllowTypes.size() > 0 && !pExt->Grinding_AllowTypes.Contains(pTechno->GetTechnoType()))
			return false;

		if (pExt->Grinding_DisallowTypes.size() > 0 && pExt->Grinding_DisallowTypes.Contains(pTechno->GetTechnoType()))
			return false;
	}

	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	if (const auto pExt = BuildingExt::ExtMap.Find(pBuilding))
	{
		const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (!pTypeExt || !pTechno)
			return false;

		if (pTypeExt->Grinding_DisplayRefund &&	EnumFunctions::CanTargetHouse(pTypeExt->Grinding_DisplayRefund_Houses, pBuilding->Owner, HouseClass::Player))
		{
			pExt->AccumulatedGrindingRefund += pTechno->GetRefund();
		}

		if (pTypeExt->Grinding_Weapon.isset()
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon.Get()->ROF)
		{
			TechnoExt::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon.Get());
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
		}

		if (pTypeExt->Grinding_Sound.isset())
		{
			VocClass::PlayAt(pTypeExt->Grinding_Sound.Get(), pTechno->GetCoords());
			return true;
		}
	}

	return false;
}
// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedGrindingRefund)
		.Process(this->DamageFireAnims)
		.Process(this->AutoSellTimer)

		;
}

void BuildingExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void BuildingExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool BuildingExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BuildingExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

void BuildingExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }
// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }
BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);
#ifdef ENABLE_NEWEXT
	BuildingExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	BuildingExt::ExtMap.FindOrAllocate(pItem);
#endif
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

//#ifdef ENABLE_NEWEXT
DEFINE_JUMP(LJMP, 0x41D9FB, 0x41DA05);
//#endif

DEFINE_HOOK(0x44E940, BuildingClass_Detach, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(void*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pExt = BuildingExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->LightSource == target ? 0x44E948 : 0x44E94E;
}
