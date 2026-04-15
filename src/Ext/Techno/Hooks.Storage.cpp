#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Infantry/Body.h>

#include <Misc/DamageArea.h>

#include <New/Entity/FlyingStrings.h>

#ifndef STORAGE_HOOKS

#ifndef BUILDING_STORAGE_HOOK
// spread tiberium on building destruction. replaces the
// original code, made faster and spilling is now optional.
ASMJIT_PATCH(0x441B30, BuildingClass_Destroy_Refinery, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto& store = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	auto& total = HouseExtContainer::Instance.Find(pThis->Owner)->TiberiumStorage;

	// remove the tiberium contained in this structure from the house's owned
	// tiberium. original code does this one bail at a time, we do bulk.
	if (store.GetAmounts() >= 1.0)
	{
		for (size_t i = 0u; i < (size_t)TiberiumClass::Array->Count; ++i)
		{
			auto const amount = std::ceil(store.GetAmount(i));

			if (amount > 0.0)
			{

				store.DecreaseLevel((float)amount, i);
				total.DecreaseLevel((float)amount, i);

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

	if (!TechnoTypeExtContainer::Instance.Find(pThis->Type)->DontShake.Get() && RulesClass::Instance->ShakeScreen)
	{
		int cost = pThis->Type->GetCost();
		TechnoExtData::ShakeScreen(pThis, cost, RulesClass::Instance->ShakeScreen);
	}

	return 0x441C39;
}

ASMJIT_PATCH(0x445FE4, BuildingClass_GrandOpening_GetStorageTotalAmount, 0x6)
{
	GET(FakeBuildingClass*, pThis, EBP);

	if (pThis->_GetTypeExtData()->Refinery_UseNormalActiveAnim)
		return 0x446183;

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x446016;
}

ASMJIT_PATCH(0x450CD7, BuildingClass_AnimAI_GetStorageTotalAmount_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts()))
		result = int(double(amount * TiberiumClass::Array->Count) / (pThis->Type->Storage + 0.5));

	R->EAX(result);
	R->EDX(pThis->Type);
	return 0x450D09;
}

ASMJIT_PATCH(0x450DAA, BuildingClass_AnimAI_GetStorageTotalAmount_B, 0x6)
{
	GET(FakeBuildingClass*, pThis, ESI);

	if (pThis->_GetTypeExtData()->Refinery_UseNormalActiveAnim)
		return 0x446183;

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450DDC;
}

ASMJIT_PATCH(0x450E12, BuildingClass_AnimAI_GetStorageTotalAmount_C, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450E3E;
}

ASMJIT_PATCH(0x4589C0, BuildingClass_storage_4589C0, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x4589DC;
}

ASMJIT_PATCH(0x44A232, BuildingClass_BuildingClass_Destruct_Storage, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto decreaase = storage->DecreaseLevel((float)storage->GetAmount(i), i);
		HouseExtContainer::Instance.Find(pThis->Owner)->TiberiumStorage.DecreaseLevel(decreaase, i);
	}
	return 0x44A287;
}
#endif

//TechnoClass
#ifndef TECHNO_STORAGE_HOOK
#endif

//UnitClass
#ifndef UNIT_STORAGE_HOOK
//UnitClass_CreditLoad
ASMJIT_PATCH(0x7438B0, UnitClass_CreditLoad_Handle, 0xA)
{
	GET(UnitClass*, pThis, ECX);
	int result = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetTotalTiberiumValue() * pThis->Owner->Type->IncomeMult);
	R->EAX((int)result);
	return 0x7438E1;
}

ASMJIT_PATCH(0x73D4A4, UnitClass_Harvest_IncludeWeeder, 0x6)
{
	enum { retFalse = 0x73D5FE, retTrue = 0x73D4DA };
	GET(UnitTypeClass*, pType, EDX);
	GET(UnitClass*, pThis, ESI);
	GET(CellClass*, pCell, EBP);
	const bool canharvest = (pType->Harvester && pCell->LandType == LandType::Tiberium) || (pType->Weeder && pCell->LandType == LandType::Weeds);
	const auto storagesPercent = pThis->GetStoragePercentage();
	const bool canStoreHarvest = storagesPercent < 1.0;

	return canharvest && canStoreHarvest ? retTrue : retFalse;
}

ASMJIT_PATCH(0x73E3BF, UnitClass_Mission_Unload_replace, 0x6)
{
	GET(BuildingClass* const, pBld, EDI);
	GET(UnitClass*, pThis, ESI);

	auto unit_storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	if (!pBld->Type->Weeder)
		HouseExtContainer::Instance.LastHarvesterBalance = pBld->GetOwningHouse()->Available_Money();// Available_Money takes silos into account

	const  auto pType = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	const int idxTiberium = unit_storage->GetFirstSlotUsed();
	float dumpAmount = pType->HarvesterDumpAmount.Get(RulesExtData::Instance()->HarvesterDumpAmount.Get());
	const float amountCanBeRemoved = idxTiberium != -1 ?
		Math::abs((float)unit_storage->GetAmount(idxTiberium)) : 0.0f;//after decreased

	if (dumpAmount > 0.0f)
		dumpAmount = std::min(dumpAmount, amountCanBeRemoved);
	else
		dumpAmount = amountCanBeRemoved;

	if (idxTiberium == -1 || unit_storage->DecreaseLevel((float)dumpAmount, idxTiberium) <= 0.0)
	{
		if (pBld->Type->Refinery)
		{ //weed ???
			pBld->Game_PlayNthAnim(BuildingAnimSlot::Production,
				pBld->GetHealthPercentage_() <= RulesClass::Instance->ConditionYellow,
				false, false);
		}
		pThis->MissionStatus = 4;

		if (pBld->Anims[10])
		{
			pBld->DestroyNthAnim(BuildingAnimSlot::Special);
		}
	}
	else
		if (pBld->Type->Weeder)
		{
			pBld->Owner->GiveWeed((int)dumpAmount, idxTiberium);
			pThis->Animation.Stage = 0;
		}
		else
		{
			TechnoExtData::DepositTiberium(pBld, pBld->Owner,
			(float)dumpAmount,
			(float)(BuildingTypeExtData::GetPurifierBonusses(pBld->Owner) * dumpAmount),
			idxTiberium
			);
			pThis->Animation.Stage = 0;

			BuildingExtContainer::Instance.Find(pBld)->AccumulatedIncome +=
				pBld->Owner->Available_Money() - HouseExtContainer::Instance.LastHarvesterBalance;

		}

	return 0x73E539;
}

ASMJIT_PATCH(0x708BC0, TechnoClass_GetStoragePercentage_GetTotalAmounts, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pType = GET_TECHNOTYPE(pThis);

	double result = 0.0;
	if (pType->Storage > 0)
	{
		result = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() / pType->Storage;
	}

	__asm fld result;
	return 0x708C0A;
}

ASMJIT_PATCH(0x7414A0, UnitClass_GetStoragePercentage_GetTotalAmounts, 0x9)
{
	GET(UnitClass*, pThis, ECX);
	double result = pThis->Type->Harvester || pThis->Type->Weeder ?
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() : 0.0f;

	result /= (double)pThis->Type->Storage;
	__asm fld result;
	return 0x7414DD;
}

ASMJIT_PATCH(0x738749, UnitClass_Destroy_TiberiumExplosive, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

	if (RulesClass::Instance->TiberiumExplosive
		&& !pThis->Type->Weeder
		&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& storage->GetAmounts() > 0.0f)
	{
		// multiply the amounts with their powers and sum them up
		int morePower = 0;

		for (int i = 0; i < TiberiumClass::Array->Count; ++i)
		{
			morePower += int(storage->m_values[i] * TiberiumClass::Array->Items[i]->Power);
		}

		if (morePower > 0)
		{

			CoordStruct crd = pThis->GetCoords();
			if (auto pWH = RulesExtData::Instance()->Tiberium_ExplosiveWarhead)
			{
				DamageArea::Apply(&crd, morePower, const_cast<UnitClass*>(pThis), pWH, pWH->Tiberium, pThis->Owner);
			}

			if (auto pAnim = RulesExtData::Instance()->Tiberium_ExplosiveAnim)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}

	return 0x7387C4;
}
#endif

//IfantryClass
#ifndef INFANTRY_STROAGE_HOOK

ASMJIT_PATCH(0x522E70, InfantryClass_Mission_Harvest_Handle, 0x5)
{
	GET(InfantryClass*, pThis, ECX);

	if (pThis->Type->Storage)
	{
		const auto v4 = (FakeCellClass*)pThis->GetCell();
		const auto val = pThis->GetStoragePercentage();

		if (v4->HasTiberium() && val < 1.0)
		{
			if (pThis->SequenceAnim != DoType::Shovel)
			{
				pThis->PlayAnim(DoType::Shovel);
			}

			auto tibType = v4->_GetTiberiumType();
			auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
			const auto amount = storage->GetAmount(tibType);
			double result = 1.0;

			if (((double)pThis->Type->Storage - amount) <= 1.0)
			{
				result = (double)pThis->Type->Storage - amount;
			}

			auto v10 = v4->ReduceTiberium((int)result);

			if (v10 > 0)
			{
				storage->IncreaseAmount((float)v10, tibType);
			}

			R->EAX(pThis->Type->HarvestRate);
			return 0x522EAB;
		}
	}

	pThis->PlayAnim(DoType::Ready);
	pThis->QueueMission(Mission::Guard, false);
	R->EAX(1);

	return 0x522EAB;
}

ASMJIT_PATCH(0x522D50, InfantryClass_StorageAI_Handle, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(TechnoClass* const, pDest, 0x4);

	//be carefull , that slave sometime do unload with different owner
	//this can become troublesome later ,..

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	bool updateSmoke = false;
	int balanceBefore = pDest->Owner->Available_Money();

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto const amountRemoved = storage->DecreaseLevel((float)storage->GetAmount(i), i);

		if (amountRemoved > 0.0)
		{
			TechnoExtData::DepositTiberium(pThis, pDest->Owner, amountRemoved,
				BuildingTypeExtData::GetPurifierBonusses(pDest->Owner) * amountRemoved,
				i);

			// register for refinery smoke
			updateSmoke = true;
		}
	}

	if (updateSmoke)
	{
		pDest->UpdateRefinerySmokeSystems();

		int money = pDest->Owner->Available_Money() - balanceBefore;
		const auto what = pDest->WhatAmI();

		if (what == BuildingClass::AbsID)
		{
			BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pDest))->AccumulatedIncome += money;
		}
		else if (what == UnitClass::AbsID && money)
		{
			auto pUnit = static_cast<UnitClass*>(pDest);

			if (pUnit->Type->DeploysInto)
			{
				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pUnit->Type->DeploysInto);

				if (pTypeExt->DisplayIncome.Get(RulesExtData::Instance()->DisplayIncome))
				{
					if (pThis->Owner->IsControlledByHuman() || RulesExtData::Instance()->DisplayIncome_AllowAI)
					{
						FlyingStrings::Instance.AddMoneyString(
						money,
						money,
						pThis,
						pTypeExt->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses),
						pThis->GetCoords(),
						pTypeExt->DisplayIncome_Offset, ColorStruct::Empty);
					}
				}
			}
		}
	}

	return 0x522E61;
}
#endif

//HouseClass
#ifndef HOUSE_STORAGE_HOOK
ASMJIT_PATCH(0x4F69D0, HouseClass_AvaibleStorage_GetStorageTotalAmounts, 0x5)
{
	GET(IHouse*, pThis, ESI);
	const int value = *reinterpret_cast<int*>(((DWORD)pThis) + 0x2EC);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	auto pExt = HouseExtContainer::Instance.Find(pHouse);

	R->EAX(value - (int)pExt->TiberiumStorage.GetAmounts());
	return 0x4F69F0;
}

ASMJIT_PATCH(0x4F69A3, HouseClass_AvaibleMoney_GetStorageTotalAmounts, 0x6)
{
	GET(IHouse*, pThis, ESI);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	auto pExt = HouseExtContainer::Instance.Find(pHouse);

	R->EAX(pExt->TiberiumStorage.GetTotalTiberiumValue());
	return 0x4F69AE;
}

ASMJIT_PATCH(0x4F6E70, HouseClass_GetTiberiumStorageAmounts, 0xA)
{
	GET(HouseClass*, pThis, ESI);
	auto pExt = HouseExtContainer::Instance.Find(pThis);

	double result = 0.0;
	const double amount = pExt->TiberiumStorage.GetAmounts();

	if ((int)amount)
	{
		result = amount / double(pThis->TotalStorage);
	}

	__asm fld result;
	return 0x4F6EA2;
}

ASMJIT_PATCH(0x4F8C05, HouseeClass_AI_StorageSpeak_GetTiberiumStorageAmounts, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX((int)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F8C15;
}

ASMJIT_PATCH(0x4F96BF, HouseClass_FindBestStorage_GetStorageTotalAmounts, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX((float)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F96C4;
}

ASMJIT_PATCH(0x4F9790, HouseClass_SpendMoney_Handle, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(int, money, 0x4);

	auto storage = &HouseExtContainer::Instance.Find(pThis)->TiberiumStorage;

	int total = (int)storage->GetAmounts();
	int total_balance = pThis->TotalStorage;
	int credits = pThis->Balance;
	int blance_before = money;

	if (money <= credits)
	{
		pThis->Balance = credits - money;
	}
	else
	{
		blance_before = pThis->Balance;
		int deduced = money - credits;
		pThis->Balance = 0;
		if (deduced > 0 && total > 0.0)
		{
			for (auto& pBld : pThis->Buildings)
			{
				if (pBld)
				{
					auto bldStorage = &TechnoExtContainer::Instance.Find(pBld)->TiberiumStorage;

					if (bldStorage->GetAmounts() > 0.0)
					{
						while (bldStorage->GetAmounts() > 0.0)
						{
							if (deduced <= 0)
								break;

							int used = bldStorage->GetFirstSlotUsed();
							while (bldStorage->GetAmount(used) > 0.0)
							{
								auto decrease_str = bldStorage->DecreaseLevel(1.0, used);
								storage->DecreaseLevel(decrease_str, used);
								int mult = int(TiberiumClass::Array->Items[used]->Value * pThis->Type->IncomeMult * decrease_str);
								deduced -= mult;
								blance_before += mult;
								if (deduced < 0)
								{
									pThis->Balance -= deduced;
									blance_before += deduced;
									deduced = 0;
									break;
								}

								if (deduced <= 0)
									break;
							}
						}
					}
				}

				if (deduced == 0)
					break;
			}
		}
	}

	pThis->UpdateAllSilos(total, total_balance);
	pThis->CreditsSpent += blance_before;
	return 0x4F9941;
}

ASMJIT_PATCH(0x4F99A6, HouseClass_UpdateAllSilos_GetStorageTotalAmounts, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	R->EAX((float)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F99B1;
}

ASMJIT_PATCH(0x502821, HouseClass_RegisterLoss_TiberiumStorage, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	auto storage = &HouseExtContainer::Instance.Find(pThis)->TiberiumStorage;

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto decreaase = storage->DecreaseLevel(2147483600.0f, i);
		pThis->SiloMoney += int(decreaase * 5.0);
		pThis->TotalStorage += int(TiberiumClass::Array->Items[i]->Value * pThis->Type->IncomeMult * decreaase);
	}

	return 0x5028A7;
}

#endif

ASMJIT_PATCH(0x65DE6B, TeamTypeClass_CreateGroup_IncreaseStorage, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TechnoTypeClass*, pFootType, EDI);
	TechnoExtContainer::Instance.Find(pFoot)->TiberiumStorage.DecreaseLevel((float)pFootType->Storage, 0);
	return 0x65DE82;
}

#endif