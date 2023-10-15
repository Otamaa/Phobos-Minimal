#include "Body.h"

NOINLINE FacingType GetPoseDir(AircraftClass* pAir , BuildingClass* pBld)
{
	FacingType ret = (FacingType)TechnoTypeExtContainer::Instance.Find(pAir->Type)->LandingDir.Get(RulesClass::Instance->PoseDir);

	if (pBld || pAir->HasAnyLink())
	{
		if (!pBld){

			for (auto i = 0; i < pAir->RadioLinks.Capacity; ++i) {
				if (auto possiblebld = specific_cast<BuildingClass*>(pAir->RadioLinks[i])) {
					pBld = possiblebld;
				}
			}

			if(!pBld && pAir->RadioLinks[0] && ret < FacingType::Min) { //spawner
				return FacingType((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
			}
		}

		if(pBld) {
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
			const int nIdx = pBld->FindLinkIndex(pAir);
			const auto dir = &pBldTypeExt->DockPoseDir;

			if (nIdx <= -1 || (size_t)nIdx >= dir->size() || ret < FacingType::Min)
			{
				return pBldTypeExt->LandingDir.Get(FacingType((((pBld->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7));

			}
			else
				return (*dir)[nIdx];
		}
	}

	if (!pAir->Type->AirportBound && ret < FacingType::Min) {
		return FacingType((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
	}

	return FacingType((((((int)ret) >> 4) + 1) >> 1) & 7);
}

// replace the entire function
DEFINE_HOOK(0x41B760, IFlyControl_LandDirection, 0x6)
{
	GET_STACK(IFlyControl*, pThis, 0x4);

	const FacingType result = GetPoseDir(static_cast<AircraftClass*>(pThis), nullptr);
	R->EAX(result);
	return 0x41B7C1;
}

// request radio contact then get land dir
DEFINE_HOOK(0x446FA2, BuildingClass_GrandOpening_PoseDir, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAir, ESI);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	const DirStruct dir { GetPoseDir(pAir, pThis) };
	pAir->SecondaryFacing.Set_Current(dir);

//	if (pThis->GetHeight() > 0)
//		AircraftTrackerClass::Instance->Add(pThis);

	return 0x446FB0;
}

// request radio contact then get land dir
DEFINE_HOOK(0x444014, BuildingClass_ExitObject_PoseDir_AirportBound, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, ECX);

	pThis->SendCommand(RadioCommand::RequestLink, pAir);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SetLocation(pThis->GetDockCoords(pAir));
	pAir->DockedTo = pThis;
	FacingType result = GetPoseDir(pAir, pThis);
	const DirStruct dir { result };
	pAir->SecondaryFacing.Set_Current(dir);

	//if (pAir->GetHeight() > 0)
	//	AircraftTrackerClass::Instance->Add(pAir);

	return 0x444053;
}

// there no radio contact happening here
// so the result mostlikely building facing
DEFINE_HOOK(0x443FD8, BuildingClass_ExitObject_PoseDir_NotAirportBound, 0x8)
{
	enum { RetCreationFail = 0x444EDE, RetCreationSucceeded = 0x443FE0 };

	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);

	if (R->AL())
	{
		pAir->DockedTo = pThis;
		const DirStruct dir { ((int)GetPoseDir(pAir, pThis) << 13) };
		pAir->SecondaryFacing.Set_Current(dir);

		//if (pAir->GetHeight() > 0)
		//	AircraftClass::AircraftTracker_4134A0(pAir);

		return RetCreationSucceeded;
	}

	return RetCreationFail;
}