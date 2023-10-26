#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Strsafe.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/SWType/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/HoverLocomotionClass.h>

#include "Header.h"

//DEFINE_HOOK(0x7079A1, TechnoClass_Detach_HunterSeeker, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(void*, comparator, EBP);
//
//	if (!pThis->GetTechnoType()->HunterSeeker || (pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
//		return 0x0;
//
//	if (pThis->Target == comparator)
//		((FootClass*)pThis)->Locomotor->Acquire_Hunter_Seeker_Target();
//
//	return 0x0;
//}

DEFINE_DISABLE_HOOK(0x4B5F9E, DropPodLocomotionClass_ILocomotion_Process_Report_ares)
DEFINE_DISABLE_HOOK(0x4B619F, DropPodLocomotionClass_ILocomotion_MoveTo_AtmosphereEntry_ares)

DEFINE_OVERRIDE_HOOK(0x718275 ,TeleportLocomotionClass_MakeRoom, 9)
{
	LEA_STACK(CoordStruct*, pCoord, 0x3C);
	GET(TeleportLocomotionClass*, pLoco, EBP);

	const auto pCell = MapClass::Instance->GetCellAt(pCoord);

	R->Stack(0x48 , false);
	const bool bLinkedIsInfantry = pLoco->LinkedTo->WhatAmI() == AbstractType::Infantry;
	R->EBX(pCell->OverlayTypeIndex);
	R->EDI(false);

	for (auto pObj = pCell->GetContentB(); pObj; pObj = pObj->NextObject)
	{
		const auto bObjIsInfantry = pObj->WhatAmI() == AbstractType::Infantry;
		bool bIsImmune = pObj->IsIronCurtained();
		auto pType = pObj->GetTechnoType();
		const auto pTypeExt = TechnoTypeExtContainer::Instance.TryFind(pType);

		if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
			bIsImmune = 1;

		if (!RulesExtData::Instance()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry) {
			pLoco->LinkedTo->ReceiveDamage(&pLoco->LinkedTo->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
			break;
		}

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
				break;
			}
		} else {
			pObj->ReceiveDamage(&pObj->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
		}
	}

	if ((pCell->Flags & CellFlags(0x300)) == CellFlags(100))
		R->Stack(0x48, true);

	R->Stack(0x20 , pLoco->LinkedTo->GetCellAgain());
	R->EAX(true);
	return 0x7184CE;
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

DEFINE_DISABLE_HOOK(0x513EAA, HoverLocomotionClass_UpdateHover_DeployToLand_ares)

DEFINE_OVERRIDE_HOOK(0x4CD9C8, FlyLocomotionClass_sub_4CD600_HunterSeeker_UpdateTarget, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	const auto pObject = pThis->LinkedTo;
	const auto pType = pObject->GetTechnoType();

	if (pType->HunterSeeker) {

		if (const auto pTarget = pObject->Target) {

			// update the target's position, considering units in tunnels
			auto crd = pTarget->GetCoords();

			const auto abs = pTarget->WhatAmI();
			if (abs == UnitClass::AbsID || abs == InfantryClass::AbsID) {
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
		else
		{
			pThis->Acquire_Hunter_Seeker_Target();
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
	const auto pObject = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo;
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

DEFINE_OVERRIDE_HOOK(0x4CFE80, FlyLocomotionClass_ILocomotion_AcquireHunterSeekerTarget, 5)
{
	GET_STACK(ILocomotion* const, pThis, 0x4);

	const auto pFly = static_cast<FlyLocomotionClass*>(pThis);

	// replace the entire function
	TechnoExt_ExtData::AcquireHunterSeekerTarget(pFly->Owner ? pFly->Owner : pFly->LinkedTo);

	return 0x4D016F;
}

DEFINE_OVERRIDE_HOOK(0x4B99A2, DropshipLoadout_WriteUnit, 0xA)
{
	GET(TechnoTypeClass*, pType, ESI);

	GET_STACK(bool, Available, STACK_OFFS(0x164, -0x8));

	LEA_STACK(Point2D*, BaseCoords, STACK_OFFS(0x164, 0x14C));
	LEA_STACK(Point2D*, AltCoords, STACK_OFFS(0x164, 0x144));

	constexpr size_t StringLen = 256;

	wchar_t pName[StringLen];
	wchar_t pArmor[StringLen];
	wchar_t pArmament[StringLen];
	wchar_t pCost[StringLen];

	StringCchPrintfW(pName, StringLen, L"Name: %hs", pType->Name);

	if (Available)
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: %d", pType->GetCost());
	}
	else
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: N/A");
	}

	if (auto pPrimary = pType->Weapon[0].WeaponType)
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: %hs", pPrimary->Name);
	}
	else
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: NONE");
	}

	if (const auto& pArmorType = ArmorTypeClass::Array[static_cast<unsigned int>(pType->Armor)])
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: %hs", pArmorType->Name.data());
	}
	else
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: UNKNOWN");
	}

	auto Color = ColorScheme::Find(Available ? GameStrings::Green() : GameStrings::Red(), 1);

	auto pSurface = DSurface::Hidden();
	RectangleStruct pSurfaceRect = pSurface->Get_Rect();
	Point2D Coords = *BaseCoords;
	Coords.X += 450;
	Coords.Y += 300;

	Drawing::PrintUnicode(AltCoords, pName, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmament, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmor, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pCost, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	return 0x4B9BBF;
}

DEFINE_OVERRIDE_HOOK(0x4B9A52, DropshipLoadout_PrintArmor, 5)
{
	R->Stack(0x4, ArmorTypeClass::Array[R->EDX()].get());
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CF3D0, FlyLocomotionClass_sub_4CEFB0_HunterSeeker, 7)
{
	GET_STACK(FlyLocomotionClass* const, pThis, 0x20);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->HunterSeeker)
	{
		//target was invalidated , use the moving dest
		if (const auto pTarget =
			pObject->Target ? pObject->Target : MapClass::Instance->TryGetCellAt(pThis->MovingDestination)
			)
		{
			auto const DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExtData::Instance()->HunterSeekerDetonateProximity);
			auto const DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExtData::Instance()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			auto crd = pObject->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			auto const dist = int(crd.Length());

			if (dist >= DetonateProximity)
			{
				// not close enough to detonate, but we might start the decent
				if (dist < DescendProximity)
				{
					// the target's current height
					auto const z = pTarget->GetCoords().Z;

					// the hunter seeker's default flight level
					crd = pObject->GetCoords();
					auto floor = MapClass::Instance->GetCellFloorHeight(crd);
					auto const height = floor + pType->GetFlightLevel();

					// linear interpolation between target's Z and normal flight level
					auto const ratio = dist / static_cast<double>(DescendProximity);
					auto const lerp = z * (1.0 - ratio) + height * ratio;

					// set the descending flight level
					auto level = int(lerp) - floor;
					if (level < 10) {
						level = 10;
					}

					pThis->FlightLevel = level;

					return 0x4CF4D2;
				}

				// project the next steps using the current speed
				// and facing. if there's a height difference, use
				// the highest value as the new flight level.
				auto const speed = pThis->Apparent_Speed();
				if (speed > 0)
				{
					double const value = pObject->PrimaryFacing.Current().GetRadian();
					double const cos = Math::cos(value);
					double const sin = Math::sin(value);

					int maxHeight = 0;
					int currentHeight = 0;
					auto crd2 = pObject->GetCoords();
					for (int i = 0; i < 11; ++i)
					{
						auto const pCell = MapClass::Instance->GetCellAt(crd2);
						auto const z = pCell->GetCoordsWithBridge().Z;

						if (z > maxHeight)
						{
							maxHeight = z;
						}

						if (!i)
						{
							currentHeight = z;
						}

						// advance one step
						crd2.X += int(cos * speed);
						crd2.Y -= int(sin * speed);

						// result is never used in TS, but a break sounds
						// like a good idea.
						auto const cell = CellClass::Coord2Cell(crd2);
						if (!MapClass::Instance->CoordinatesLegal(cell))
						{
							break;
						}
					}

					// pull the old lady up
					if (maxHeight > currentHeight)
					{
						pThis->FlightLevel = pType->GetFlightLevel();
						return 0x4CF4D2;
					}
				}

			}
			else
			{
				auto const pWeapon = pObject->GetPrimaryWeapon()->WeaponType;

				// damage
				auto damage = pWeapon->Damage;

				//if the target exist , damage the target
				if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget)) {
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pObject, true, true, pObject->Owner);
				}

				// damage the hunter seeker regardless the target state
				damage = pWeapon->Damage;
				pObject->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

				// damage the map
				auto const crd2 = pObject->GetCoords();
				//WeaponTypeExtData::DetonateAt(pWeapon, crd2, pObject, true, pObject->Owner);
				MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd2);
				MapClass::DamageArea(crd2, pWeapon->Damage, pObject, pWeapon->Warhead, true, pObject->Owner);

				R->EBX(0);
				return 0x4CF5F2;
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CDE64, FlyLocomotionClass_sub_4CD600_HunterSeeker_Ascent, 6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, unk, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto ret = pThis->FlightLevel - unk;
	auto max = 16;

	if (!pType->IsDropship)
	{
		if (!pType->HunterSeeker)
		{
			// ordinary aircraft
			max = (R->BL() != 0) ? 10 : 20;

		}
		else
		{
			// is hunter seeker
			if (pThis->IsTakingOff)
			{
				max = pExt->HunterSeekerEmergeSpeed.Get(RulesExtData::Instance()->HunterSeekerEmergeSpeed);
			}
			else
			{
				max = pExt->HunterSeekerAscentSpeed.Get(RulesExtData::Instance()->HunterSeekerAscentSpeed);
			}
		}
	}

	if (ret > max)
	{
		ret = max;
	}

	R->EAX(ret);
	return 0x4CDE8F;
}

DEFINE_OVERRIDE_HOOK(0x4CDF54, FlyLocomotionClass_sub_4CD600_HunterSeeker_Descent, 5)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, max, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->HunterSeeker) {
		auto ret = pExt->HunterSeekerDescentSpeed.Get(RulesExtData::Instance()->HunterSeekerDescentSpeed);
		if (max < ret) {
			ret = max;
		}

		R->ECX(ret);
		return 0x4CDF81;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x514A21, HoverLocomotionClass_ILocomotion_Process_DeployToLand, 9)
{
	GET(ILocomotion*, ILoco, ESI);

	auto const bIsMovingNow = ILoco->Is_Moving_Now();
	R->AL(bIsMovingNow);
	auto const pOwner = static_cast<HoverLocomotionClass*>(ILoco)->Owner;

	if (pOwner->InAir)
	{
		auto const pType = pOwner->GetTechnoType();
		if (pType->DeployToLand)
		{
			auto pCell = pOwner->GetCell();
			auto nLand = pCell->LandType;
			if ((nLand == LandType::Beach || nLand == LandType::Water) && !pCell->ContainsBridge())
			{
				pOwner->InAir = false;
				pOwner->QueueMission(Mission::Guard, true);
			}

			if (bIsMovingNow)
			{
				ILoco->Stop_Moving();
				pOwner->SetDestination(nullptr, true);
			}

			if (pType->DeployingAnim)
			{
				auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
				const int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;
				DirStruct nDeployDir(nDeployDirVal);

				if (pOwner->PrimaryFacing.Current() != nDeployDir) {
					pOwner->PrimaryFacing.Set_Desired(nDeployDir);
				}
			}

			if (pOwner->GetHeight() <= 0)
			{
				pOwner->InAir = false;
				ILoco->Mark_All_Occupation_Bits(0);
			}
		}
	}

	return 0x514A2A;
}

DEFINE_OVERRIDE_HOOK(0x54C767, JumpjetLocomotionClass_State4_54C550_DeployDir, 6)
{
	GET(JumpjetLocomotionClass*, pLoco, ESI);
	auto const pOwner = pLoco->LinkedTo;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());
	const int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;

	DirStruct nDeployDir(nDeployDirVal);

	if (pLoco->Facing.Current() != nDeployDir)
		pLoco->Facing.Set_Desired(nDeployDir);

	return 0x54C7A3;
}
