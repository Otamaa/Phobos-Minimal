#include "Body.h"

#include <BuildingClass.h>
#include <FootClass.h>
#include <AirstrikeClass.h>
#include <SpawnManagerClass.h>

static void HandleTargeting(TechnoClass* pThis, AbstractClass* pTarget)
{
	pThis->ShouldLoseTargetNow = false;
	if (pThis->Target == pTarget)
		return;

	const auto pAirstrike = pThis->Airstrike;

	if (pAirstrike && pAirstrike->Target == pThis)
		pAirstrike->ResetTarget();

	AbstractClass* pTargetResult = nullptr;
	if (Is_Aircraft(pThis)) {
		if (pTarget) {
			const auto pType = pThis->GetTechnoType();
			if (pType->Spawned && pType->MissileSpawn) {
				if (pThis->Ammo) {
					if (pThis->CurrentMission == Mission::Attack) {
						auto nStatus = pThis->MissionStatus;
						if (nStatus > 4 && nStatus < 10) {
							pThis->Ammo = 0;
						}
					}
				}
			}
		}
	}

}