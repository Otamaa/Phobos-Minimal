#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>


DEFINE_HOOK(0x71A9F1, TemporalClass_Update_RemoveBuildingTarget, 0x6)
{
	GET(TemporalClass* const, pThis, ESI);
	GET(BuildingClass*, pTarget, EDI);

	if (!pTarget)
		return 0x71AAD5;
	{
		if (pTarget->BunkerLinkedItem)
			pTarget->UnloadBunker();

		if (pTarget->Type->Helipad
			&& pTarget->RadioLinks.Items
			&& pTarget->RadioLinks.IsAllocated
			&& pTarget->RadioLinks.IsInitialized
			)
		{
			for (auto i = 0; i < pTarget->RadioLinks.Capacity; ++i)
			{
				if (auto const pAir = cast_to<AircraftClass*>(pTarget->RadioLinks[i]))
				{
					if (pAir->IsAlive && !pAir->InLimbo && !pAir->TemporalTargetingMe)
					{
						const auto pExt = TechnoTypeExtContainer::Instance.Find(pAir->Type);
						if (pAir->IsInAir())
						{
							if ((pExt->Crashable.isset() && !pExt->Crashable) || !pAir->Crash(pThis->Owner))
							{
								Debug::LogInfo(__FUNCTION__" Called ");
								TechnoExtData::HandleRemove(pAir, pThis->Owner, false, false);
							}
						}
						else
						{
							//Ask plane to fly
							pTarget->SendCommand(RadioCommand::AnswerLeave, pAir);
							pAir->DockedTo = nullptr;
						}
					}
				}
			}
		}
	}

	return 0x71AA1A;
}