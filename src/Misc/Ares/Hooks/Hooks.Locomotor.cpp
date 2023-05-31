#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Locomotor/HoverLocomotionClass.h>

DEFINE_OVERRIDE_HOOK(0x718275 ,TeleportLocomotionClass_MakeRoom, 9)
{
	LEA_STACK(CoordStruct*, pCoord, 0x3C);
	GET(TeleportLocomotionClass*, pLoco, EBP);

	auto pCell = MapClass::Instance->TryGetCellAt(*pCoord);
	R->Stack(0x48 , false);
	const bool bLinkedIsInfantry = pLoco->LinkedTo->WhatAmI() == AbstractType::Infantry;
	R->EBX(pCell->OverlayTypeIndex);
	R->EDI(false);

	for (auto pObj = pCell->GetContent(); pObj; pObj = pObj->NextObject)
	{
		auto bObjIsInfantry = pObj->WhatAmI() == AbstractType::Infantry;
		bool bIsImmune = pObj->IsIronCurtained();
		auto pType = pObj->GetTechnoType();
		const auto pTypeExt = TechnoTypeExt::ExtMap.TryFind(pType);

		if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
			bIsImmune = 1;

		if (!RulesExt::Global()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry)
			break;

		if (!bIsImmune && bObjIsInfantry && bLinkedIsInfantry)
		{
			auto nCoord = pObj->GetCoords();
			if (nCoord.X == pCoord->X && nCoord.Y == pCoord->Y && nCoord.Z == pCoord->Z) {
				pObj->ReceiveDamage(&pObj->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
			}
		}
		else if (bIsImmune || ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		{
			if ((pObj->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None) {
				R->Stack(0x48, true);
			} else if(bIsImmune) {
				pLoco->LinkedTo->ReceiveDamage(&pLoco->LinkedTo->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
			}
		} else {
			pObj->ReceiveDamage(&pObj->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
		}
	}

	if ((pCell->Flags & CellFlags(0x100)) != CellFlags(0) && (pCell->Flags & CellFlags(0x200)) == CellFlags(100))
		R->Stack(0x48, true);

	R->Stack(0x20 , pLoco->LinkedTo->GetCellAgain());
	R->EAX(true);
	return 0x7184CE;
}

DEFINE_OVERRIDE_HOOK(0x4B5F9E, DropPodLocomotionClass_ILocomotion_Process_Report, 0x6)
{
	// do not divide by zero
	GET(int const, count, EBP);
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

	FootClass* pFoot = hLoco->LinkedTo ? hLoco->LinkedTo : hLoco->Owner;

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
	GET(FlyLocomotionClass*, pThis, ESI);
	const auto pObject = pThis->LinkedTo;
	const auto pType = pObject->GetTechnoType();

	if (pType->HunterSeeker) {
		if (const auto pTarget = pObject->Target) {

			// update the target's position, considering units in tunnels
			auto crd = pTarget->GetCoords();

			const auto abs = GetVtableAddr(pTarget);
			if (abs == UnitClass::vtable || abs == InfantryClass::vtable) {
				const auto pFoot = static_cast<FootClass* const>(pObject);
				if (pFoot->TubeIndex >= 0) {
					crd = pFoot->CurrentMechPos;
				}
			}

			const auto  height = MapClass::Instance->GetCellFloorHeight(crd);

			if (crd.Z < height) {
				crd.Z = height;
			}

			pThis->MovingDestination = crd;

			// update the facing
			const auto crdSource = pObject->GetCoords();

			DirStruct const tmp(double(crdSource.Y - crd.Y), double(crd.X - crdSource.X));
			pObject->PrimaryFacing.Set_Current(tmp);
			pObject->SecondaryFacing.Set_Current(tmp);
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CE85A, FlyLocomotionClass_UpdateLanding, 0x8)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	const auto pObject = pThis->LinkedTo;
	const auto pType = pObject->GetTechnoType();

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
	const auto pObject = pLoco->LinkedTo;
	const auto pType = pObject->GetTechnoType();

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
