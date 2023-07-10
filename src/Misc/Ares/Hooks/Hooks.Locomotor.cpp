#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Misc/AresData.h>
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
		auto bObjIsInfantry = pObj->WhatAmI() == AbstractType::Infantry;
		bool bIsImmune = pObj->IsIronCurtained();
		auto pType = pObj->GetTechnoType();
		const auto pTypeExt = TechnoTypeExt::ExtMap.TryFind(pType);

		if (pTypeExt && !pTypeExt->Chronoshift_Crushable)
			bIsImmune = 1;

		if (!RulesExt::Global()->ChronoInfantryCrush && bLinkedIsInfantry && !bObjIsInfantry) {
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

bool NOINLINE AcquireHunterSeekerTarget(TechnoClass* pThis)  {

	if (!pThis->Target) {
		std::vector<TechnoClass*> preferredTargets;
		std::vector<TechnoClass*> randomTargets;

		// defaults if SW isn't set
		auto pOwner = pThis->GetOwningHouse();
		SWTypeExt::ExtData* pSWExt = nullptr;
		auto canPrefer = true;

		// check the hunter seeker SW
		if (auto const pSuper =
#ifndef Replace_SW
			AttachedSuperWeapon(pThis)
#else
			TechnoExt::ExtMap.Find(pThis)->LinkedSW
#endif
			) {
			pOwner = pSuper->Owner;
			pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
			canPrefer = !pSWExt->HunterSeeker_RandomOnly;
		}

		auto const isHumanControlled = pOwner->IsControlledByHuman_();
		auto const mode = SessionClass::Instance->GameMode;

		// the AI in multiplayer games only attacks its favourite enemy
		auto const pFavouriteEnemy = HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex);
		auto const favouriteEnemyOnly = (mode != GameMode::Campaign
			&& pFavouriteEnemy && !isHumanControlled);

		for (auto const& i : *TechnoClass::Array) {

			// is the house ok?
			if (favouriteEnemyOnly) {
				if (i->Owner != pFavouriteEnemy) {
					continue;
				}
			}
			else if (!pSWExt && pOwner->IsAlliedWith(i->Owner)) {
				// default without SW
				continue;
			}
			else if (pSWExt && !pSWExt->IsHouseAffected(pOwner, i->Owner)) {
				// use SW
				continue;
			}

			// techno ineligible
			if (i->Health < 0 || i->InLimbo || !i->IsAlive) {
				continue;
			}

			if (i->IsIronCurtained())
				continue;

			if(auto pBuilding = specific_cast<BuildingClass*>(i)) {
				const auto pExt = BuildingExt::ExtMap.Find(pBuilding);
				if(pExt->LimboID != -1)
				   continue;
			}

			// type prevents this being a target
			auto const pType = i->GetTechnoType();

			if (pType->Invisible || !pType->LegalTarget) {
				continue;
			}

			// is type to be ignored?
			auto const pExt = TechnoTypeExt::ExtMap.Find(pType);
			if (pExt->HunterSeekerIgnore) {
				continue;
			}

			// harvester truce
			if (ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune) {
				if (auto const pUnitType = abstract_cast<UnitTypeClass*>(pType)) {
					if (pUnitType->Harvester) {
						continue;
					}
				}
			}

			// allow to exclude certain techno types
			if (pSWExt && !pSWExt->IsTechnoAffected(i)) {
				continue;
			}

			// in multiplayer games, non-civilian targets are preferred
			// for human players
			auto const isPreferred = mode != GameMode::Campaign && isHumanControlled
				&& !i->Owner->Type->MultiplayPassive && canPrefer;

			// add to the right list
			if (isPreferred) {
				preferredTargets.push_back(i);
			}
			else {
				randomTargets.push_back(i);
			}
		}

		auto const& targets = !preferredTargets.empty() ? preferredTargets : randomTargets;

		if (auto const count = static_cast<int>(targets.size())) {
			auto const index = ScenarioClass::Instance->Random.RandomFromMax(count - 1);
			auto const& pTarget = targets[index];

			// that's our target
			pThis->SetTarget(pTarget);
			return true;
		}
	}

	return false;
}

DEFINE_OVERRIDE_HOOK(0x4CFE80, FlyLocomotionClass_ILocomotion_AcquireHunterSeekerTarget, 5)
{
	GET_STACK(ILocomotion* const, pThis, 0x4);
	auto const pLoco = static_cast<FlyLocomotionClass*>(pThis);

	// replace the entire function
	AcquireHunterSeekerTarget(pLoco->LinkedTo);

	return 0x4D016F;
}

DEFINE_OVERRIDE_HOOK(0x4B5EB0, DropPodLocomotionClass_ILocomotion_Process_Smoke, 6)
{
	GET(FootClass*, pFoot, ESI);
	REF_STACK(const CoordStruct, Coords, 0x34);

	// create trailer even without weapon, but only if it is set
	if (!(Unsorted::CurrentFrame % 6))
	{
		if (AnimTypeClass* pType = RulesExt::Global()->DropPodTrailer)
		{
			if (auto pAnim = GameCreate<AnimClass>(pType, Coords))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
			}
		}
	}

	if (const auto pWeapon = RulesClass::Instance->DropPodWeapon)
	{
		R->ESI(pWeapon);
		return 0x4B5F14;
	}

	return 0x4B602D;
}

DEFINE_OVERRIDE_HOOK(0x4B99A2, DropshipLoadout_WriteUnit, 0xA)
{
	GET(TechnoTypeClass*, pType, ESI);

	GET_STACK(bool, Available, STACK_OFFS(0x164, -0x8));

	LEA_STACK(Point2D*, BaseCoords, STACK_OFFS(0x164, 0x14C));
	LEA_STACK(Point2D*, AltCoords, STACK_OFFS(0x164, 0x144));

	const size_t StringLen = 256;

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
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pType->HunterSeeker)
	{
		if (auto const pTarget = pObject->Target)
		{
			auto const DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExt::Global()->HunterSeekerDetonateProximity);
			auto const DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExt::Global()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			auto crd = pObject->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			auto const dist = int(crd.Magnitude());

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
					if (level < 10)
					{
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
					double const cos = std::cos(value);
					double const sin = std::sin(value);

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
				// close enough to detonate
				if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
				{
					auto const pWeapon = pObject->GetPrimaryWeapon()->WeaponType;

					// damage the target
					auto damage = pWeapon->Damage;
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pObject, true, true, nullptr);

					// damage the hunter seeker
					damage = pWeapon->Damage;
					pObject->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

					// damage the map
					auto const crd2 = pObject->GetCoords();
					MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd2);
					MapClass::DamageArea(crd2, pWeapon->Damage, pObject, pWeapon->Warhead, true, nullptr);

					// return 0
					R->EBX(0);
					return 0x4CF5F2;
				}
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
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

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
				max = pExt->HunterSeekerEmergeSpeed.Get(RulesExt::Global()->HunterSeekerEmergeSpeed);
			}
			else
			{
				max = pExt->HunterSeekerAscentSpeed.Get(RulesExt::Global()->HunterSeekerAscentSpeed);
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
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pType->HunterSeeker)
	{
		auto ret = pExt->HunterSeekerDescentSpeed.Get(RulesExt::Global()->HunterSeekerDescentSpeed);
		if (max < ret)
		{
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
				auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
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
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
	const int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;

	DirStruct nDeployDir(nDeployDirVal);

	if (pLoco->Facing.Current() != nDeployDir)
		pLoco->Facing.Set_Desired(nDeployDir);

	return 0x54C7A3;
}
