#include "Helpers.h"
#include <Ext/SWType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

bool Helpers::Otamaa::LauchSW(int LaunchWhat,
	HouseClass* pOwner,
	const CoordStruct Where,
	bool WaitForCharge,
	bool ResetChargeAfterLauch,
	bool Grant,
	bool GrantRepaitSidebar,
	bool GrantOneTime,
	bool GrantOnHold,
	bool Manual,
	bool IgnoreInhibitor,
	bool IgnoreDesignator,
	bool IgnoreMoney)
{
	auto const HouseOwner = !pOwner || pOwner->Defeated ? HouseExt::FindCivilianSide() : pOwner;

	if (HouseOwner)
	{
		//TODO : if not real lauch , use Ext SW array to handle
		if (auto pSelected = HouseOwner->Supers.GetItemOrDefault(LaunchWhat))
		{
			auto const pSuper = pSelected;
			const auto pSWExt = SWTypeExt::ExtMap.Find(pSelected->Type);

			auto const nWhere = CellClass::Coord2Cell(Where);
			bool const lauch = (WaitForCharge) && (!pSuper->IsCharged || (pSuper->IsPowered() && HouseOwner->HasLowPower())) ? false : true;
			bool const bIsObserver = HouseOwner->IsObserver();
			bool const MoneyEligible = IgnoreMoney ? true : HouseOwner->CanTransactMoney(pSWExt->Money_Amount.Get());
			bool const InhibitorEligible = IgnoreInhibitor ? true : !pSWExt->HasInhibitor(HouseOwner, nWhere);
			bool const DesignatorEligible = IgnoreDesignator ? true : !pSWExt->HasDesignator(HouseOwner, nWhere);

			if (Grant || Manual)
			{
				if (pSuper->Grant(GrantOneTime, !bIsObserver, GrantOnHold))
				{
					if (!bIsObserver && (Manual || GrantRepaitSidebar))
					{
						if (MouseClass::Instance->AddCameo(AbstractType::Special, LaunchWhat))
						{
							MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}
			}

			if (!WaitForCharge)
				pSuper->SetReadiness(true);

			if (!Manual && lauch && MoneyEligible && InhibitorEligible && DesignatorEligible && !pSuper->IsOnHold)
			{
				const int oldstart = pSuper->RechargeTimer.StartTime;
				const int oldleft = pSuper->RechargeTimer.TimeLeft;

				pSuper->Launch(nWhere, !bIsObserver);

				if (ResetChargeAfterLauch)
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