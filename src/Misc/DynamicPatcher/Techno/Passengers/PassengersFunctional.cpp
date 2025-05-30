#include "PassengersFunctional.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

void PassengersFunctional::AI(TechnoClass* pThis)
{
	if (auto const pTranporter = pThis->Transporter)
	{
		if (pThis->InOpenToppedTransport)
		{
			if (!pTranporter->IsAlive) {
				//if(auto pThisFoot = flag_cast_to<FootClass*>(pThis))
				//	pTranporter->MarkPassengersAsExited();

				//this not doing anything, just removing the transporter flag
				//there is some cases where transport is already dead, but the transporter pointer is still valid
				pThis->InOpenToppedTransport = false;
				pThis->Transporter = nullptr;

				if(pThis->InLimbo) // only delete limboed techno , assume it fail to survive
					TechnoExtData::HandleRemove(pThis, nullptr, false, false);

				return;
			}

			if (auto const pTransportExt = TechnoTypeExtContainer::Instance.Find(pTranporter->GetTechnoType()))
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
		if (pThis->InOpenToppedTransport) {
			auto const pTransportExt = TechnoTypeExtContainer::Instance.Find(pType);

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
