#include "Helpers.h"
#include <Ext/SWType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

#include <New/Entity/FlyingStrings.h>

bool Helpers::Otamaa::LauchSW(const LauchSWData& nData,
	HouseClass* pOwner, const CoordStruct Where)
{
	auto const HouseOwner = !pOwner || pOwner->Defeated ? HouseExt::FindCivilianSide() : pOwner;

	if (HouseOwner)
	{
		//TODO : if not real lauch , use Ext SW array to handle
		if (auto pSelected = HouseOwner->Supers.GetItemOrDefault(nData.LaunchWhat))
		{
			auto const pSuper = pSelected;
			const auto pSWExt = SWTypeExt::ExtMap.Find(pSelected->Type);

			auto const nWhere = CellClass::Coord2Cell(Where);
			bool const lauch = (nData.LaunchWaitcharge) && (!pSuper->IsCharged || (pSuper->IsPowered() && HouseOwner->HasLowPower())) ? false : true;
			bool const bIsObserver = HouseOwner->IsObserver();
			bool const MoneyEligible = nData.LauchSW_IgnoreMoney ? true : HouseOwner->CanTransactMoney(pSWExt->Money_Amount.Get());
			bool const InhibitorEligible = nData.LaunchSW_IgnoreInhibitors ? true : !pSWExt->HasInhibitor(HouseOwner, nWhere);
			bool const DesignatorEligible = nData.LaunchSW_IgnoreDesignators ? true : !pSWExt->HasDesignator(HouseOwner, nWhere);

			if (nData.LaunchGrant || nData.LaunchSW_Manual)
			{
				if (pSuper->Grant(nData.LaunchGrant_OneTime, !bIsObserver, nData.LaunchGrant_OnHold))
				{
					if (!bIsObserver && (nData.LaunchSW_Manual || nData.LaunchGrant_RepaintSidebar))
					{
						if (MouseClass::Instance->AddCameo(AbstractType::Special, nData.LaunchWhat))
						{
							MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}
			}

			if (!nData.LaunchWaitcharge)
				pSuper->SetReadiness(true);

			if (!nData.LaunchSW_Manual &&
				lauch &&
				MoneyEligible &&
				InhibitorEligible &&
				DesignatorEligible &&
				!pSuper->IsOnHold)
			{
				const int oldstart = pSuper->RechargeTimer.StartTime;
				const int oldleft = pSuper->RechargeTimer.TimeLeft;
				FlyingStrings::AddMoneyString(nData.LaunchSW_DisplayMoney && pSWExt->Money_Amount != 0 ,
					pSWExt->Money_Amount, HouseOwner, 
					nData.LaunchSW_DisplayMoney_Houses, Where, nData.LaunchSW_DisplayMoney_Offset);

				pSuper->Launch(nWhere, !bIsObserver);

				if (nData.LaunchResetCharge)
					pSuper->Reset();
				else
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