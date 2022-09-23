#include "Helpers.h"
#include <Ext/SWType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

bool Helpers::Otamaa::LauchSW(SuperWeaponTypeClass* LaunchWhat,
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
	auto const HouseOwner = !pOwner  || pOwner->Defeated ? HouseExt::FindCivilianSide():pOwner;

	if (HouseOwner)
	{
		if (LaunchWhat)
		{
			SuperClass* pSelected = nullptr;
			for (auto const pSuper : HouseOwner->Supers)
			{
				if (pSuper->Type->ArrayIndex == LaunchWhat->ArrayIndex)
				{
					pSelected = pSuper;
					break;
				}
			}

			const auto pSWExt = SWTypeExt::ExtMap.Find(LaunchWhat);

			if (auto const pSuper = pSelected)
			{
				auto const nWhere = Map[Where]->MapCoords;
				bool const lauch = (WaitForCharge) && (!pSuper->IsCharged || (pSuper->IsPowered() && HouseOwner->HasLowPower())) ? false : true;
				bool const bIsObserver = HouseOwner->IsObserver() || HouseOwner->IsCurrentPlayerObserver();
				bool const MoneyEligible = IgnoreMoney ? true : HouseOwner->CanTransactMoney(pSWExt->Money_Amount.Get());
				bool const InhibitorEligible = IgnoreInhibitor ? true : !pSWExt->HasInhibitor(HouseOwner, nWhere);
				bool const DesignatorEligible = IgnoreDesignator ? true : !pSWExt->HasDesignator(HouseOwner, nWhere);

				if (Grant || Manual)
				{
					if (pSuper->Grant(GrantOneTime, !bIsObserver, GrantOnHold))
					{
						if (!bIsObserver && (Manual || GrantRepaitSidebar))
						{
							if (MouseClass::Instance->AddCameo(AbstractType::Special, LaunchWhat->ArrayIndex))
								MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}

				if (!WaitForCharge)
					pSuper->SetReadiness(true);

				if (!Manual && lauch && MoneyEligible && InhibitorEligible && DesignatorEligible)
				{
					CellStruct nCell { nWhere.X ,nWhere.Y };
					pSuper->Launch(nCell, !bIsObserver);

					if (ResetChargeAfterLauch)
						pSuper->Reset();

					return true;
				}
			}
		}
	}

	return false;
}