#include "Body.h"

#include <AircraftClass.h>
#include <AircraftTrackerClass.h>

#include <Locomotor/FlyLocomotionClass.h>

#include <Misc/DamageArea.h>

#include <Ext/Building/Body.h>
#include <Ext/AircraftType/Body.h>

DirType NOINLINE BuildingExtData::GetPoseDir(AircraftClass* pAir, BuildingClass* pBld, bool isProduction)
{
	const bool hasDockOrLink = (pBld || pAir->HasAnyLink());

	if (auto const pOwner = pAir->SpawnOwner)
		return pOwner->PrimaryFacing.Current().GetDir();

	auto pRules = FakeRulesClass::Instance();

	if (hasDockOrLink) {
		if (!pBld) {
			for (auto i = 0; i < pAir->RadioLinks.Capacity; ++i) {
				if (auto possiblebld = cast_to<BuildingClass*>(pAir->RadioLinks[i])) {
					pBld = possiblebld;
				}
			}
		}

		if (pBld) {
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
			const int nIdx = pBld->FindLinkIndex(pAir);
			const auto dir = &pBldTypeExt->DockPoseDir;

			// Use DockPoseDir if valid index exists, otherwise fall back to building's LandingDir
			if ((size_t)nIdx < dir->size()) {
				return (*dir)[nIdx];
			}

			if (pBldTypeExt->LandingDir.isset())
				return pBldTypeExt->LandingDir.Fetch();

			if (!pBldTypeExt->AircraftDockingDir_DefaultToPoseDir.Get(pRules->AircraftDockingDir_DefaultToPoseDir)) {
				if (!isProduction) {
					return pBld->PrimaryFacing.Current().GetDir();
				}
			}
		}
	}

	auto const pTypeExt = AircraftTypeExtContainer::Instance.Find(pAir->Type);

	if (pTypeExt->LandingDir.isset()) {
		int landingDir = pTypeExt->LandingDir.Fetch();
		if (pAir->Type->AirportBound)
			return static_cast<DirType>(landingDir & 0xFF);
		else if (landingDir < 0)
			return pAir->PrimaryFacing.Current().GetDir();
		else
			return static_cast<DirType>(landingDir & 0xFF);
	}

	if (isProduction)
		return static_cast<DirType>(pRules->PoseDir_Production.Get(RulesClass::Instance->PoseDir) & 0xFF);

	if (hasDockOrLink)
		return pRules->_PoseDir;

	int fieldDir = pRules->PoseDir_Field.Get(RulesClass::Instance->PoseDir * 32);
	return static_cast<DirType>(fieldDir & 0xFF);
}

// replace the entire function
ASMJIT_PATCH(0x41B760, IFlyControl_LandDirection, 0x6)
{
	GET_STACK(IFlyControl*, pThis, 0x4);
	R->EAX(BuildingExtData::GetPoseDir(static_cast<AircraftClass*>(pThis), nullptr, false));
	return 0x41B7C1;
}

// request radio contact then get land dir
ASMJIT_PATCH(0x446FA2, BuildingClass_GrandOpening_PoseDir, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAir, ESI);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	const DirStruct dir {  BuildingExtData::GetPoseDir(pAir, pThis, true) };

	if (AircraftTypeExtData::ExtendedAircraftMissionsEnabled(pAir))
		pAir->PrimaryFacing.Set_Current(dir);

	pAir->SecondaryFacing.Set_Current(dir);

//	if (pThis->GetHeight() > 0)
//		AircraftTrackerClass::Instance->Add(pThis);

	return 0x446FB0;
}

ASMJIT_PATCH(0x687AF4, CCINIClass_InitializeStuffOnMap_AdjustAircrafts, 0x5)
{
	AircraftClass::Array->for_each([](AircraftClass* const pThis) {
		if (pThis && pThis->Type->AirportBound) {
			if (auto pCell = pThis->GetCell()) {
				if (auto pBuilding = pCell->GetBuilding()) {
					if (pBuilding->Type->Helipad && pThis->Type->Dock.contains(pBuilding->Type)) {
						pBuilding->SendCommand(RadioCommand::RequestLink, pThis);
						pBuilding->SendCommand(RadioCommand::RequestTether, pThis);
						pThis->SetLocation(pBuilding->GetDockCoords(pThis));
						pThis->DockedTo = pBuilding;
						const DirStruct dir { ((int) BuildingExtData::GetPoseDir(pThis, pBuilding, false) << 13) };
						pThis->SecondaryFacing.Set_Current(dir);

						if (pThis->GetHeight() > 0)
							AircraftTrackerClass::Instance->Add(pThis);
					}
				}
			}
		}
	});

	return 0x0;
}

ASMJIT_PATCH(0x4CF31C, FlyLocomotionClass_FlightUpdate_LandingDir, 0x9)
{
	enum { SkipGameCode = 0x4CF3D0, SetSecondaryFacing = 0x4CF351 };

	GET(FootClass** const, pFootPtr, ESI);
	GET_STACK(IFlyControl* const, iFly, STACK_OFFSET(0x48, -0x38));
	REF_STACK(unsigned int, dir, STACK_OFFSET(0x48, 0x8));

	const auto pFoot = *pFootPtr;
	dir = 0;

	if (iFly) {

		if (iFly->Is_Locked())
			return SkipGameCode;

		if (const auto pAircraft = cast_to<AircraftClass*, true>(pFoot))
			dir = DirStruct(BuildingExtData::GetPoseDir(pAircraft, nullptr, false)).Raw;
		else
			dir = (iFly->Landing_Direction() << 13);
	}

	return SetSecondaryFacing;
}


