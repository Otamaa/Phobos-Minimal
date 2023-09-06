#include "Body.h"

NOINLINE DirType8 GetPoseDir(AircraftClass* pAir , BuildingClass* pBld)
{
	DirType8 ret = (DirType8)TechnoTypeExt::ExtMap.Find(pAir->Type)->LandingDir.Get(RulesClass::Instance->PoseDir);

	if (pBld || pAir->HasAnyLink())
	{
		if (!pBld){

			for (auto i = 0; i < pAir->RadioLinks.Capacity; ++i) {
				if (auto possiblebld = specific_cast<BuildingClass*>(pAir->RadioLinks[i])) {
					pBld = possiblebld;
				}
			}

			if(!pBld && pAir->RadioLinks[0] && ret < DirType8::Min) { //spawner
				return DirType8((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
			}
		}

		if(pBld) {
			const auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
			const int nIdx = pBld->FindLinkIndex(pAir);
			const auto dir = &pBldTypeExt->DockPoseDir;

			if (nIdx <= -1 || (size_t)nIdx >= dir->size() || ret < DirType8::Min)
			{
				return pBldTypeExt->LandingDir.Get(DirType8((((pBld->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7));

			}
			else
				return (*dir)[nIdx];
		}
	}

	if (!pAir->Type->AirportBound && ret < DirType8::Min) {
		return DirType8((((pAir->PrimaryFacing.Current().Raw >> 12) + 1) >> 1) & 7);
	}

	return std::clamp(ret, DirType8::Min, DirType8::Max);
}

// replace the entire function
DEFINE_HOOK(0x41B760, IFlyControl_LandDirection, 0x6)
{
	GET_STACK(IFlyControl*, pThis, 0x4);

	const DirType8 result = GetPoseDir(static_cast<AircraftClass*>(pThis), nullptr);
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
	DirType8 result = GetPoseDir(pAir, pThis);
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