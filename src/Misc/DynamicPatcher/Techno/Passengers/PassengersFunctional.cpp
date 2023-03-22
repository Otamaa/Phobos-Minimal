#ifdef COMPILE_PORTED_DP_FEATURES
#include "PassengersFunctional.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

void PassengersFunctional::AI(TechnoClass* pThis)
{
	if (auto const pTranporter = pThis->Transporter)
	{
		if (pTranporter->GetTechnoType()->OpenTopped)
		{
			if (auto const pTransportExt = TechnoTypeExt::ExtMap.Find(pTranporter->GetTechnoType()))
			{
				if (!pTransportExt->MyPassangersData.PassiveAcquire)
				{
					auto nMission = pTranporter->GetCurrentMission();
					if (nMission != Mission::Attack)
						pThis->Override_Mission(Mission::Sleep, nullptr,nullptr);
				}

				if (pTransportExt->MyPassangersData.ForceFire)
					pThis->Override_Mission(pTranporter->CurrentMission, pTranporter->Target);
			}
		}
	}
}

bool PassengersFunctional::CanFire(TechnoClass* pThis)
{
	if (auto const pTranporter = pThis->Transporter)
	{
		auto const pType = pTranporter->GetTechnoType();
		if (pType->OpenTopped) {
			auto const pTransportExt = TechnoTypeExt::ExtMap.Find(pType);

			switch (pTranporter->GetCurrentMission())
			{
			case Mission::Attack:
				return !pTransportExt->MyPassangersData.SameFire;
			case Mission::Move:
			case Mission::AttackMove:
				return !pTransportExt->MyPassangersData.MobileFire;
			}
		}
	}

	return false;
}
#endif