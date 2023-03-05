#ifdef COMPILE_PORTED_DP_FEATURES
#include "GiftBoxFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include "GiftBox.h"
#include "GiftBoxData.h"

const bool OpenDisallowed(TechnoClass* const pTechno)
{
	if (pTechno)
	{

		if (pTechno->InLimbo)
			return false;

		const bool bIsOnWarfactory = TechnoExt::IsInWarfactory(pTechno);

		return pTechno->Absorbed ||
			pTechno->InOpenToppedTransport ||
			bIsOnWarfactory ||
			pTechno->TemporalTargetingMe;
	}

	return false;
}

void GiftBoxFunctional::Init(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->MyGiftBoxData.Enable)
		return;

	auto const nDelay = pTypeExt->MyGiftBoxData.DelayMax == 0 ?
		pTypeExt->MyGiftBoxData.Delay :
		ScenarioClass::Instance->Random(
			pTypeExt->MyGiftBoxData.DelayMin,
			pTypeExt->MyGiftBoxData.DelayMax);

	pExt->MyGiftBox = std::make_unique<GiftBox>(nDelay);

}

void GiftBoxFunctional::Destroy(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->MyGiftBox || OpenDisallowed(pExt->Get()))
		return;

	if (pTypeExt->MyGiftBoxData.OpenWhenDestoryed && !pExt->MyGiftBox->IsOpen)
	{
		pExt->MyGiftBox->Release(pExt->Get(), pTypeExt->MyGiftBoxData);
		pExt->MyGiftBox->IsOpen = true;
	}

}

void GiftBoxFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->MyGiftBox || OpenDisallowed(pExt->Get()))
		return;

	if (!pTypeExt->MyGiftBoxData.OpenWhenDestoryed &&
		!pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.isset() &&
		pExt->MyGiftBox->CanOpen())
	{

		pExt->MyGiftBox->Release(pExt->Get(), pTypeExt->MyGiftBoxData);
		pExt->MyGiftBox->IsOpen = true;
	}

	if (pExt->MyGiftBox->IsOpen)
	{
		if (pTypeExt->MyGiftBoxData.Remove)
		{
			pExt->Get()->Limbo();
			TechnoExt::HandleRemove(pExt->Get());
			return;
		}

		if (pTypeExt->MyGiftBoxData.Destroy)
		{
			auto nDamage = (pExt->Get()->Health + 1);
			pExt->Get()->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, nullptr, false,
				!pTypeExt->Get()->Crewed, nullptr);

			return;
		}

		{
			auto const nDelay = pTypeExt->MyGiftBoxData.DelayMax == 0 ?
				pTypeExt->MyGiftBoxData.Delay :
				ScenarioClass::Instance->Random(
					pTypeExt->MyGiftBoxData.DelayMin,
					pTypeExt->MyGiftBoxData.DelayMax);

			pExt->MyGiftBox->Reset(nDelay);
		}
	}

}

void GiftBoxFunctional::TakeDamage(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, WarheadTypeClass* pWH, DamageState nState)
{

	if (!pExt->MyGiftBox.get())
		return;

	if (nState != DamageState::NowDead &&
		(!OpenDisallowed(pExt->Get())) &&
		pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.isset())
	{
		// 计算血量百分比是否达到开启条件
		double healthPercent = pExt->Get()->GetHealthPercentage();
		if (healthPercent <= pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.Get())
		{
			pExt->MyGiftBox->Release(pExt->Get(), pTypeExt->MyGiftBoxData);
			pExt->MyGiftBox->IsOpen = true;
		}
	}
}

void GetGifts(const GiftBoxData& nData, std::vector<TechnoTypeClass*>& nOut)
{
	const auto giftCount = nData.Gifts.size();

	if (nData.UseChancesAndWeight.Get())
	{
		int numsCount = nData.Nums.size();

		if (nData.RandomType.Get())
		{
			int times = 1;
			if (numsCount > 0)
			{
				times = 0;
				for (auto const& num : nData.Nums)
				{
					times += num;
				}
			}

			auto weightCount = nData.RandomWeights.size();
			std::map<Point2D, int> targetPad;
			int flag = 0;
			Point2D targets = Point2D::Empty;

			for (size_t index = 0; index < giftCount; index++)
			{
				targets.X = flag;
				int weight = 1;
				if (weightCount > 0 && index < weightCount)
				{
					int w = nData.RandomWeights[index];
					if (w > 0)
					{
						weight = w;
					}
				}
				flag += weight;
				targets.Y = flag;
				targetPad.emplace(targets, index);
			}

			for (int i = 0; i < times; i++)
			{
				// 选出类型的序号
				int index = 0;
				// 产生标靶
				const int p = ScenarioClass::Instance->Random.RandomFromMax(flag);
				// 检查命中
				for (auto const& [point, idx] : targetPad)
				{
					Point2D tKey = point;
					if (p >= tKey.X && p < tKey.Y)
					{
						// 中
						index = idx;
					}
				}
				// 计算概率
				if (Helpers_DP::Bingo(nData.Chances, index))
				{
					nOut.push_back(nData.Gifts[index]);
				}
			}
		}
		else
		{
			for (size_t index = 0; index < giftCount; index++)
			{
				auto id = nData.Gifts[index];
				int times = 1;
				if (numsCount > 0 && index < (size_t)numsCount)
				{
					times = nData.Nums[index];
				}

				for (int i = 0; i < times; i++)
				{
					// 计算概率
					if (Helpers_DP::Bingo(nData.Chances, (index)))
					{
						nOut.push_back(id);
					}
				}
			}
		}
	}
	else
	{
		if (nData.RandomType)
		{
			auto const nIdx = ScenarioClass::Instance->Random.RandomFromMax(giftCount - 1);
			for (int i = 0; i < nData.Nums[nIdx]; ++i)
				nOut.push_back(nData.Gifts[nIdx]);
		}
		else
		{
			for (size_t i = 0; i < (giftCount); ++i)
			{
				for (int a = 0; a < nData.Nums[i]; ++a)
				{
					nOut.push_back(nData.Gifts[i]);
				}
			}
		}
	}
}

void GiftBox::Release(TechnoClass* pOwner, GiftBoxData& nData)
{
	auto const pHouse = pOwner->GetOwningHouse();
	CoordStruct location = pOwner->GetCoords();

	if (auto pCell = MapClass::Instance->TryGetCellAt(location)) {
		AbstractClass* pDest = nullptr;
		AbstractClass* pFocus = nullptr;

		if (pOwner->WhatAmI() != AbstractType::Building) {
			if (auto pFoot = generic_cast<FootClass*>(pOwner))
				pDest = pFoot->Destination;

			pFocus = pOwner->Focus;
		}

		std::vector<TechnoTypeClass*> nOut;
		GetGifts(nData, nOut);

		for (auto const& pTech : nOut) {
			if (nData.RandomRange > 0) { 
				if (auto const pNewCell = GeneralUtils::GetCell(pCell, location, (size_t)(nData.RandomRange.Get()), nData.EmptyCell))
					pCell = pNewCell;
			}

			if (auto const pGift = Helpers_DP::CreateAndPutTechno(pTech, pHouse, location, pCell, nData.CheckPathfind))
			{
				if (auto pOwnerHouse = pGift->GetOwningHouse())
				{
					if (!pOwnerHouse->IsNeutral() && !pGift->GetTechnoType()->Insignificant)
					{
						pOwnerHouse->RegisterGain(pGift, false);
						pOwnerHouse->AddTracking(pGift);
						pOwnerHouse->RecheckTechTree = true;
					}
				}

				if (auto pAir = specific_cast<AircraftClass*>(pGift))
				{
					if (pAir->IsInAir())
					{
						pAir->Tracker_4134A0();
					}
				}

				if (pOwner->IsSelected)
				{
					auto const feedback = Unsorted::MoveFeedback();
					Unsorted::MoveFeedback() = false;
					pGift->Select();
					Unsorted::MoveFeedback() = feedback;
				}

				if (!pDest && !pFocus)
				{
					pGift->Scatter(CoordStruct::Empty, true, false);
				}
				else
				{
					auto const nGiftWhat = pGift->WhatAmI();

					if (nGiftWhat != AbstractType::Building)
					{
						CoordStruct des = pDest ? pDest->GetCoords() : location;

						if (pFocus)
						{
							pGift->SetFocus(pFocus);
							if (nGiftWhat == AbstractType::Unit)
							{
								des = pFocus->GetCoords();
							}
						}

						if (auto pTargetCell = MapClass::Instance->TryGetCellAt(des))
						{
							pGift->SetDestination(pTargetCell, true);
							pGift->QueueMission(Mission::Move, false);
						}
					}
				}

				if (pTech->WhatAmI() == AbstractType::BuildingType)
				{
					Debug::Log("[%s] Gift box release BuildingType as an gift ,pType [%s] \n", pOwner->get_ID(), pTech->get_ID());
				}
			}
			else
			{
				Debug::Log("Gift box release gift failed ,pType [%s] \ns", pTech->get_ID());
			}
		}
	}
}
#endif