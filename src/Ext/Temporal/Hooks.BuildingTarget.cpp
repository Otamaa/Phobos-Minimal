#include "Body.h"


DEFINE_HOOK(0x71A9EE, TemporalClass_Update_RemoveBuildingTarget, 0x9)
{
	GET(TemporalClass* const, pThis, ESI);

	BuildingClass* BuildingTargetResult = nullptr;
	auto const pTarget = pThis->Target;

	{
		if (pTarget->IsSelected)
			pTarget->Deselect();

		if (auto const BuildingTarget = specific_cast<BuildingClass*>(pTarget))
		{
			BuildingTargetResult = BuildingTarget;

			if (BuildingTarget->BunkerLinkedItem)
				BuildingTarget->UnloadBunker();

			if (BuildingTarget->Type->Helipad
				&& BuildingTarget->RadioLinks.Items
				&& BuildingTarget->RadioLinks.IsAllocated
				&& BuildingTarget->RadioLinks.IsInitialized
				)
			{
				for (auto i = 0; i < BuildingTarget->RadioLinks.Capacity; ++i)
				{
					if (auto const pAir = specific_cast<AircraftClass*>(BuildingTarget->RadioLinks[i]))
					{
						if (pAir->IsAlive && !pAir->InLimbo)
						{
							if (pAir->IsInAir() && pAir->Type->Crashable)
							{
								pAir->Crash(pThis->Owner);
							}
							else
							{
								//Ask plane to fly
								BuildingTarget->SendCommand(RadioCommand::AnswerLeave, pAir);
								pAir->DockedTo = nullptr;
							}
						}
					}
				}
			}
		}
	}

	R->ECX(BuildingTargetResult);
	return 0x71AA1D;
}