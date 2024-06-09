#include "FireSWFunctional.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

void FireSWFunctional::OnFire(TechnoClass* pThis, AbstractClass* pTarget, int nWeaponIDx)
{
	if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType()))
	{
		if (!pTypeExt->SWFireData.SuperWeapons.empty() && (pTypeExt->SWFireData.AnyWeapon.Get()
			|| pTypeExt->SWFireData.WeaponIndex.Get() == nWeaponIDx))
		{
			if (auto const pHouse = pThis->Owner)
			{
				for (auto const& pSuperType : pTypeExt->SWFireData.SuperWeapons)
				{
					if (pSuperType)
					{
						if (SuperClass* pSuperDecided = pHouse->FindSuperWeapon(pSuperType->Type))
						{
							if (pSuperDecided->IsCharged || !pTypeExt->SWFireData.RealLaunch)
							{
								CoordStruct targetPos = pTarget && pTypeExt->SWFireData.ToTarget.Get() ? pTarget->GetCoords() : pThis->GetCoords();
								CellStruct cell = CellClass::Coord2Cell(targetPos);
								int oldstart = pSuperDecided->RechargeTimer.StartTime;
								int oldleft = pSuperDecided->RechargeTimer.TimeLeft;
								pSuperDecided->SetReadiness(true);
								pSuperDecided->Launch(cell, true);
								pSuperDecided->Reset();

								if (!pTypeExt->SWFireData.RealLaunch) {
									pSuperDecided->RechargeTimer.StartTime = oldstart;
									pSuperDecided->RechargeTimer.TimeLeft = oldleft;
								}
							}
						}
					}
				}
			}
		}
	}
}