#include "Helpers.h"
#include <Ext/Anim/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

#include <New/Entity/FlyingStrings.h>

bool Helpers::Otamaa::LauchSW(const LauchSWData& nData,
	HouseClass* pOwner, const CoordStruct Where , TechnoClass* pFirer)
{
	const auto pOwnerResult = HouseExtData::GetHouseKind(nData.LauchhSW_Owner, true, HouseExtData::FindFirstCivilianHouse(), pOwner, nullptr);
	auto const HouseOwner = pOwnerResult->Defeated ? HouseExtData::FindFirstCivilianHouse() : pOwnerResult;

	if (HouseOwner)
	{
		if (auto pSelected = HouseOwner->Supers.get_or_default(nData.LaunchWhat))
		{
			auto const pSuper = pSelected;
			const auto pSWExt = SWTypeExtContainer::Instance.Find(pSelected->Type);
			const auto pHouseExt = HouseExtContainer::Instance.Find(HouseOwner);

			auto const nWhere = CellClass::Coord2Cell(Where);
			bool const bIsCurrentPlayer = HouseOwner->IsCurrentPlayer();

			if (nData.LaunchGrant || nData.LaunchSW_Manual) {
				if (pSuper->Grant(nData.LaunchGrant_OneTime, !bIsCurrentPlayer, nData.LaunchGrant_OnHold)) {
					if (!bIsCurrentPlayer && (nData.LaunchSW_Manual || nData.LaunchGrant_RepaintSidebar)) {
						if (MouseClass::Instance->AddCameo(AbstractType::Special, nData.LaunchWhat)) {
							MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}
			}

			// check these after grant
			bool const chargeStae = !nData.LaunchWaitcharge || pSuper->IsCharged;
			bool const MoneyEligible = nData.LauchSW_IgnoreMoney || HouseOwner->CanTransactMoney(pSWExt->Money_Amount.Get());
			bool const BattleDataEligible = nData.LauchSW_IgnoreBattleData || pHouseExt->CanTransactBattlePoints(pSWExt->BattlePoints_Amount);

			bool const InhibitorEligible = nData.LaunchSW_IgnoreInhibitors || !pSWExt->HasInhibitor(HouseOwner, nWhere);
			bool const DesignatorEligible = nData.LaunchSW_IgnoreDesignators || pSWExt->HasDesignator(HouseOwner, nWhere);

			auto const mostCheckPasses = !nData.LaunchSW_RealLauch || 
				pSuper->Granted && chargeStae && !pSuper->IsOnHold && MoneyEligible && BattleDataEligible && InhibitorEligible && DesignatorEligible;

			if (mostCheckPasses && !nData.LaunchSW_Manual) {

				if (!nData.LaunchWaitcharge)
					pSuper->SetReadiness(true);

				const int oldstart = pSuper->RechargeTimer.StartTime;
				const int oldleft = pSuper->RechargeTimer.TimeLeft;
				FlyingStrings::Instance.AddMoneyString(nData.LaunchSW_DisplayMoney && pSWExt->Money_Amount != 0 ,
					pSWExt->Money_Amount, HouseOwner,
					nData.LaunchSW_DisplayMoney_Houses, Where, nData.LaunchSW_DisplayMoney_Offset, ColorStruct::Empty);

				pSuper->Launch(nWhere, bIsCurrentPlayer);

				if (nData.LaunchResetCharge)
					pSuper->Reset();
				else if (!nData.LaunchSW_RealLauch)
				{
					pSuper->RechargeTimer.StartTime = oldstart;
					pSuper->RechargeTimer.TimeLeft = oldleft;
				}

				return true;
			}
		}
	}

	return false;
}