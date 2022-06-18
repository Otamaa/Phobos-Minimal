#ifdef COMPILE_PORTED_DP_FEATURES
#include "GiftBoxFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include "GiftBox.h"
#include "GiftBoxData.h"

const bool OpenDisallowed(TechnoClass* const pTechno)
{
	if (pTechno) {

		if (pTechno->InLimbo)
			return false;

		bool bIsOnWarfactory = false;
		if (pTechno->WhatAmI() == AbstractType::Unit) {
			if (auto const pCell = pTechno->GetCell()) {
				if (auto const pBuildingBelow = pCell->GetBuilding()) {
					if (pTechno->RadioLinks.IsAllocated) {
						if (auto const pLinkedBuilding = specific_cast<BuildingClass*>(*pTechno->RadioLinks.Items)) {
							bIsOnWarfactory = pLinkedBuilding->Type->WeaponsFactory && !pLinkedBuilding->Type->Naval && pBuildingBelow == pLinkedBuilding;
						}
					}
				}
			}
		}

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
	if (!pExt->MyGiftBox || OpenDisallowed(pExt->OwnerObject()))
		return;

	if (pTypeExt->MyGiftBoxData.OpenWhenDestoryed && !pExt->MyGiftBox->IsOpen) {
		pExt->MyGiftBox->Release(pExt->OwnerObject() ,pTypeExt->MyGiftBoxData);
		pExt->MyGiftBox->IsOpen = true;
	}

}

void GiftBoxFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->MyGiftBox || OpenDisallowed(pExt->OwnerObject()))
		return;

	if (!pTypeExt->MyGiftBoxData.OpenWhenDestoryed &&
		!pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.isset() &&
		pExt->MyGiftBox->CanOpen()) {

		pExt->MyGiftBox->Release(pExt->OwnerObject(),pTypeExt->MyGiftBoxData);
		pExt->MyGiftBox->IsOpen = true;
	}

	if (pExt->MyGiftBox->IsOpen) {
		if (pTypeExt->MyGiftBoxData.Remove) {
			pExt->OwnerObject()->Limbo();
			if (auto pOwner = pExt->OwnerObject()->GetOwningHouse())
			{
				if (!pOwner->IsNeutral() && !pExt->OwnerObject()->GetTechnoType()->Insignificant)
				{
					pOwner->RegisterLoss(pExt->OwnerObject(), false);
					pOwner->RemoveTracking(pExt->OwnerObject());
					pOwner->RecheckTechTree = true;
				}
			}

			pExt->OwnerObject()->RemoveFromTargetingAndTeam();

			for (auto const& pBullet : *BulletClass::Array)
				if (pBullet && pBullet->Target == pExt->OwnerObject())
					pBullet->LoseTarget();

			pExt->OwnerObject()->UnInit();

			return;
		}

		if (pTypeExt->MyGiftBoxData.Destroy) {
			auto nDamage = (pExt->OwnerObject()->Health + 1);
			pExt->OwnerObject()->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, nullptr, true,
				!pTypeExt->OwnerObject()->Crewed, nullptr);

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

void GiftBoxFunctional::TakeDamage(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, WarheadTypeClass* pWH, DamageState nState) {

	if (!pExt->MyGiftBox)
		return;

	if (nState != DamageState::NowDead &&
		(!OpenDisallowed(pExt->OwnerObject())) &&
		pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.isset()) {
		// 计算血量百分比是否达到开启条件
		double healthPercent = pExt->OwnerObject()->GetHealthPercentage();
		if (healthPercent <= pTypeExt->MyGiftBoxData.OpenWhenHealthPercent.Get()) {
			pExt->MyGiftBox->Release(pExt->OwnerObject(), pTypeExt->MyGiftBoxData);
			pExt->MyGiftBox->IsOpen = true;
		}
	}
}

CellClass* GiftBox::GetCell(CellClass* pIn, CoordStruct& InOut, size_t nSpread, bool EmptyCell)
{
	CellStruct cell = CellClass::Coord2Cell(InOut);
	auto const nDummy = GeneralUtils::AdjacentCellsInRange(nSpread);

	size_t const max = nDummy.size();
	for (size_t i = 0; i < max; i++) {
		int index = ScenarioGlobal->Random(0, max - 1);
		CellStruct const offset = nDummy[index];

		if (offset == CellStruct::Empty)
			continue;

		if (auto pNewCell = MapClass::Instance->TryGetCellAt(cell + offset)) {
			if (pNewCell->LandType != pIn->LandType || (EmptyCell && pNewCell->GetContent()))
				continue;

			InOut = pNewCell->GetCoordsWithBridge();
			return pNewCell;
			break;
		}
	}

	return nullptr;
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
		auto const GetGifts = [&nData , &nOut]() {
			auto giftCount = nData.Gifts.size();

			if (nData.UseChancesAndWeight.Get()) {
				int numsCount = nData.Nums.Count;

				if (nData.RandomType.Get()) {
					int times = 1;
					if (numsCount > 0) {
						times = 0;
						for (auto const& num : nData.Nums) {
							times += num;
						}
					}

					auto weightCount = nData.RandomWeights.size();
					std::map<Point2D, int> targetPad;
					int flag = 0;
					Point2D targets = Point2D::Empty;

					for (size_t index = 0; index < giftCount; index++) {
						targets.X = flag;
						int weight = 1;
						if (weightCount > 0 && index < weightCount) {
							int w = nData.RandomWeights[index];
							if (w > 0) {
								weight = w;
							}
						}
						flag += weight;
						targets.Y = flag;
						targetPad.insert({ targets, index });
					}

					for (int i = 0; i < times; i++) {
						// 选出类型的序号
						int index = 0;
						// 产生标靶
						int p = ScenarioGlobal->Random.RandomRanged(0, flag);
						// 检查命中
						for (auto const& [point,idx] : targetPad) {
							Point2D tKey = point;
							if (p >= tKey.X && p < tKey.Y) {
								// 中
								index = idx;
							}
						}
						// 计算概率
						if (Helpers_DP::Bingo(nData.Chances, (index))) {
							nOut.push_back(nData.Gifts[index]);
						}
					}
				} else {
					for (size_t index = 0; index < giftCount; index++) {
						auto id = nData.Gifts[index];
						int times = 1;
						if (numsCount > 0 && index < (size_t)numsCount) {
							times = nData.Nums[index];
						}

						for (int i = 0; i < times; i++) {
							// 计算概率
							if (Helpers_DP::Bingo(nData.Chances, (index))){
								nOut.push_back(id);
							}
						}
					}
				}
			} else {
				if (nData.RandomType) {
					auto const nIdx = ScenarioGlobal->Random.RandomRanged(0, giftCount - 1);
					for (int i = 0; i < nData.Nums[nIdx]; ++i)
						nOut.push_back(nData.Gifts[nIdx]);
				} else {
					for (size_t i = 0; i < (giftCount); ++i) {
						for (int a = 0; a < nData.Nums[i]; ++a) {
							nOut.push_back(nData.Gifts[i]);
						}
					}
				}
			}
		};

		GetGifts();

		if (!nOut.empty()) {
			for (auto const& pTech : nOut) {

				if (nData.RandomRange > 0)
					if (auto const pNewCell = GetCell(pCell, location, (size_t)(nData.RandomRange.Get()), nData.EmptyCell))
						pCell = pNewCell;

				if (auto const pGift = Helpers_DP::CreateAndPutTechno(pTech, pHouse, location, pCell)) {

					if (auto pOwnerHouse = pGift->GetOwningHouse())
					{
						if (!pOwnerHouse->IsNeutral() && !pGift->GetTechnoType()->Insignificant)
						{
							pOwnerHouse->RegisterGain(pGift, false);
							pOwnerHouse->AddTracking(pGift);
							pOwnerHouse->RecheckTechTree = true;
						}
					}
					if (auto pAir = specific_cast<AircraftClass*>(pGift)) {
						if (pAir->IsInAir()) {
							pAir->Tracker_4134A0();
						}
					}

					auto pExt = TechnoExt::GetExtData(pGift);
					if (pExt && pOwner->IsSelected) {
						pExt->SkipVoice = true;
						pGift->Select();
						pExt->SkipVoice = false;
					}

					if (!pDest && !pFocus) {
						pGift->Scatter(CoordStruct::Empty, true, false);
					} else {

						if (pGift->WhatAmI() != AbstractType::Building) {
							CoordStruct des = pDest ? pDest->GetCoords() : location;

							if (pFocus) {
								pGift->SetFocus(pFocus);
								if (pGift->WhatAmI() == AbstractType::Unit) {
									des = pFocus->GetCoords();
								}
							}

							if (auto pTargetCell = MapClass::Instance->TryGetCellAt(des)) {
								pGift->SetDestination(pTargetCell, true);
								pGift->QueueMission(Mission::Move, false);
							}
						}
					}
				} else {
					Debug::Log("Gift box release gift failed ,pType [%s]", pTech->get_ID());
				}
			}
		}
	}

}
#endif