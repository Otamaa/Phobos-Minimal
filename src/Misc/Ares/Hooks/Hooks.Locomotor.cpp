#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

DEFINE_OVERRIDE_HOOK(0x4B5F9E, DropPodLocomotionClass_ILocomotion_Process_Report, 0x6)
{
	// do not divide by zero
	GET(int, count, EBP);
	return count ? 0 : 0x4B5FAD;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x514F60, HoverLocomotionClass_ILocomotion_MoveTo, 0x7)
DEFINE_OVERRIDE_HOOK(0x514E97, HoverLocomotionClass_ILocomotion_MoveTo, 0x7)
{
	GET(HoverLocomotionClass const* const, hLoco, ESI);
	FootClass* pFoot = hLoco->Owner ? hLoco->Owner : hLoco->LinkedTo;

	if (!pFoot->Destination)
		pFoot->SetSpeedPercentage(0.0);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x516305, HoverLocomotionClass_sub_515ED0, 0x9)
{
	GET(HoverLocomotionClass const* const, hLoco, ESI);
	hLoco->sub_514F70(true);
	auto pFoot = hLoco->LinkedTo;
	if (!pFoot->Destination)
		pFoot->SetSpeedPercentage(0.0);

	return 0x51630E;
}

DEFINE_OVERRIDE_HOOK(0x514DFE, HoverLocomotionClass_ILocomotion_MoveTo_DeployToLand, 0x7)
{
	GET(HoverLocomotionClass const* const, pLoco, ESI);
	const auto pFoot = !pLoco->Owner ? pLoco->LinkedTo : pLoco->Owner;

	if (pFoot->GetTechnoType()->DeployToLand)
		pFoot->NeedsRedraw = true;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x513EAA, HoverLocomotionClass_UpdateHover_DeployToLand, 0x5)
{
	GET(HoverLocomotionClass const* const, pLoco, ESI);
	return pLoco->LinkedTo->InAir ? 0x513ECD : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4B619F, DropPodLocomotionClass_ILocomotion_MoveTo_AtmosphereEntry, 0x5)
{
	return RulesClass::Instance->AtmosphereEntry ? 0x0 : 0x4B61D6;
}

DEFINE_OVERRIDE_HOOK(0x4CD9C8, FlyLocomotionClass_sub_4CD600_HunterSeeker_UpdateTarget, 0x6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if (pType->HunterSeeker) {
		if (auto const pTarget = pObject->Target) {

			// update the target's position, considering units in tunnels
			auto crd = pTarget->GetCoords();

			auto const abs = GetVtableAddr(pTarget);
			if (abs == UnitClass::vtable || abs == InfantryClass::vtable) {
				auto const pFoot = static_cast<FootClass*>(pObject);
				if (pFoot->TubeIndex >= 0) {
					crd = pFoot->CurrentMechPos;
				}
			}

			auto const height = MapClass::Instance->GetCellFloorHeight(crd);

			if (crd.Z < height) {
				crd.Z = height;
			}

			pThis->MovingDestination = crd;

			// update the facing
			auto const crdSource = pObject->GetCoords();

			DirStruct const tmp(double(crdSource.Y - crd.Y), double(crd.X - crdSource.X));
			pObject->PrimaryFacing.Set_Current(tmp);
			pObject->SecondaryFacing.Set_Current(tmp);
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CE85A, FlyLocomotionClass_UpdateLanding, 0x8)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if (pType->HunterSeeker) {
		if (!pObject->Target) {

			pThis->Acquire_Hunter_Seeker_Target();

			if (pObject->Target) {
				pThis->IsLanding = false;
				pThis->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}

		// return 0
		R->EAX(0);
		return 0x4CE852;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CCB84, FlyLocomotionClass_ILocomotion_Process_HunterSeeker, 0x6)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<FlyLocomotionClass*>(pThis);
	auto const pObject = pLoco->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if (pType->HunterSeeker) {
		if (!pObject->Target) {

			pLoco->Acquire_Hunter_Seeker_Target();

			if (pObject->Target) {

				pLoco->IsLanding = false;
				pLoco->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}
	}

	return 0;
}
