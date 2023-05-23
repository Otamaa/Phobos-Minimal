#include "FighterAreaGuard.h"

#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>

bool FighterAreaGuard::IsAreaGuardRolling()
{
	switch (State)
	{
	case AircraftGuardState::GUARD:
	case AircraftGuardState::ROLLING:
		return true;
	}
	return false;

}

void FighterAreaGuard::OnUpdate()
{
	FootClass* pTechno = OwnerObject;
	auto  pHouse = pTechno->Owner;
	if (!pHouse || !pHouse->IsControlledByHuman())
	{
		return;
	}

	const auto pType = OwnerObject->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto locomotion = pTechno->Locomotor.get();

	switch (State)
	{
	case AircraftGuardState::READY:
	{

		if (bool inAir = pTechno->IsInAir())
		{
			if (pTypeExt->MyFighterData.AutoGuard)
			{
				switch (pTechno->CurrentMission)
				{
				case Mission::Attack:
				case Mission::Enter:
					CancelAreaGuard();
					return;
				case Mission::Guard:
				case Mission::Area_Guard:
				case Mission::Move:
					if (SetupDestination())
					{
						if (onStopCommand)
						{
							// 按下S，跳过一次目的地更改
							onStopCommand = false;
							// Logger.Log($"{Game.CurrentFrame} [{section}]{pTechno} 跳过巡航");
						}
						else
						{
							// Logger.Log($"{Game.CurrentFrame} [{section}]{pTechno} 开始巡航");
							this->State = AircraftGuardState::GUARD;
						}
					}
					return;
				}
			}
		}
		else
		{
			onStopCommand = false;
			CancelAreaGuard();

			if (pTypeExt->MyFighterData.DefaultToGuard &&
				pTechno->Ammo >= pTypeExt->MyFighterData.MaxAmmo)
			{
				CoordStruct sourcePos = pTechno->GetCoords();
				if (const auto pCell = MapClass::Instance->TryGetCellAt(sourcePos))
				{
					auto nPos = pCell->GetCoordsWithBridge();
					SetupDestination(nPos);
					this->State = AircraftGuardState::GUARD;
				}
			}
		}
	}
	break;
	case AircraftGuardState::STOP:
	{

		if (pTechno->GetHeight() <= 0)
		{
			State = AircraftGuardState::READY;
		}

		switch (pTechno->CurrentMission)
		{
		case Mission::Attack:
		case Mission::Enter:
		case Mission::Move:
			State = AircraftGuardState::READY;
			return;
		}
	}
	break;
	case AircraftGuardState::ATTACK:
	{
		const auto pTarget = pTechno->Target;

		if (!pTarget)
		{
			if (CoordStruct::Empty != destCenter && pTechno->Ammo > pTypeExt->MyFighterData.MinAmmo)
			{
				this->State = AircraftGuardState::GUARD;
				pTechno->ForceMission(Mission::Area_Guard);
			}
			else
			{
				BackToAirport();
				this->State = AircraftGuardState::RELOAD;
			}
		}
		else if (pTypeExt->MyFighterData.ChaseRange > 0)
		{
			int dist = pTypeExt->MyFighterData.ChaseRange * 256;
			CoordStruct targetPos = pTarget->GetCoords();
			if (targetPos.DistanceFrom(pTechno->GetCoords()) > dist)
			{
				pTechno->SetTarget(nullptr);
			}
		}
	}
	break;
	case AircraftGuardState::RELOAD:
	{

		if (!pTechno->IsInAir() && pTechno->Ammo >= pTypeExt->MyFighterData.MaxAmmo)
		{
			State = AircraftGuardState::GUARD;
			locomotion->Move_To(destCenter);
			pTechno->ForceMission(Mission::Area_Guard);
		}
	}
	break;
	case AircraftGuardState::GUARD:
	case AircraftGuardState::ROLLING:
	{
		switch (pTechno->CurrentMission)
		{
		case Mission::Guard:
		case Mission::Area_Guard:
			break;
		case Mission::Move:
			if (pTypeExt->MyFighterData.AutoGuard)
			{
				if (SetupDestination())
				{
					State = AircraftGuardState::GUARD;
				}
				break;
			}

			CancelAreaGuard();
			State = AircraftGuardState::STOP;
			return;
		case Mission::Enter:
			if (onStopCommand
				&& pTypeExt->MyFighterData.DefaultToGuard
				&& pTechno->Ammo >= pTypeExt->MyFighterData.MaxAmmo)
			{
				pTechno->ForceMission(Mission::Area_Guard);
			}
			else
			{
				State = AircraftGuardState::STOP;
			}
			return;
		default:
			State = AircraftGuardState::RELOAD;
			return;
		}

		if (pTechno->Ammo == 0)
		{
			BackToAirport();
			State = AircraftGuardState::RELOAD;
			return;
		}

		CoordStruct destNow = locomotion->Destination();
		CoordStruct location = pTechno->GetCoords();

		if (FoundAndAttack(location))
		{
			State = AircraftGuardState::ATTACK;
		}
		else
		{
			auto nDestList = Iterator(destList);

			if (destNow != destCenter
				&& !nDestList.contains(destNow)
				&& pTechno->CurrentMission != Mission::Area_Guard)
			{
				locomotion->Move_To(destList[destIndex]);
			}

			CoordStruct posA = location;
			posA.Z = 0;
			CoordStruct posB = destNow;
			posB.Z = 0;
			bool changeDest = posA.DistanceFrom(posB) <= 512;
			if (changeDest)
			{
				if (destIndex > 0)
				{
					State = AircraftGuardState::ROLLING;
				}

				pTechno->SetDestination(nullptr, false);
				locomotion->Move_To(destList[destIndex]);
				if (++destIndex >= (int)destList.size())
				{
					destIndex = 0;
				}
			}
		}
	}
	break;
	}
}

void FighterAreaGuard::OnStopCommand()
{
	FootClass* pTechno = OwnerObject;
	onStopCommand = true;
	CancelAreaGuard();
	State = AircraftGuardState::STOP;
	pTechno->ForceMission(Mission::Enter);
}

void FighterAreaGuard::StartAreaGuard()
{
	FootClass* pTechno = OwnerObject;

	switch (State)
	{
	case AircraftGuardState::READY:
	case AircraftGuardState::GUARD:
	case AircraftGuardState::ROLLING:
		CoordStruct dest = CoordStruct::Empty;
		auto pDest = pTechno->Destination;
		if (pDest)
		{
			dest = pDest->GetCoords();
		}
		else
		{
			dest = pTechno->Locomotor->Destination();
		}

		if (SetupDestination(dest))
		{
			State = AircraftGuardState::GUARD;
		}
		break;
	}
}

bool FighterAreaGuard::SetupDestination()
{
	auto nDest = OwnerObject->Locomotor->Destination();
	return SetupDestination(nDest);
}

bool FighterAreaGuard::SetupDestination(CoordStruct& dest)
{
	FootClass* pTechno = OwnerObject;
	const auto pType = OwnerObject->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto nDestList = Iterator(destList);

	if (CoordStruct::Empty != dest && dest != destCenter && !nDestList.contains(dest))
	{
		destCenter = dest;
		CoordStruct location = pTechno->GetCoords();
		DirStruct sourceDir = Helpers_DP::Point2Dir(location, dest);
		double sourceRad = sourceDir.GetRadian();
		CoordStruct flh = CoordStruct { pTypeExt->MyFighterData.GuardRadius * 256, 0, 0 };
		destList.clear();

		bool clockwise = pTypeExt->MyFighterData.Clockwise;
		if (pTypeExt->MyFighterData.Randomwise)
		{
			clockwise = ScenarioClass::Instance->Random.RandomFromMax(2) == 0;
		}

		Clockwise = clockwise;

		for (int i = 0; i < 16; i++)
		{
			DirStruct targetDir = Helpers_DP::DirNormalized(i, 16);
			double targetRad = targetDir.GetRadian();
			float angle = (float)(sourceRad - targetRad);
			targetDir = Helpers_DP::Radians2Dir(angle);
			CoordStruct newDest = Helpers_DP::GetFLHAbsoluteCoords(dest, flh, targetDir);

			if (i == 0)
			{
				destList.push_back(newDest);
			}
			else
			{
				if (clockwise)
				{
					destList.insert(destList.begin(), newDest);
				}
				else
				{
					destList.push_back(newDest);
				}
			}
		}

		destIndex = 0;
		return true;
	}
	return false;
}

void FighterAreaGuard::CancelAreaGuard()
{
	destCenter = CoordStruct::Empty;
	destList.clear();
	destIndex = 0;
}

void FighterAreaGuard::BackToAirport()
{
	OwnerObject->SetTarget(nullptr);
	OwnerObject->ForceMission(Mission::Enter);
}

bool FighterAreaGuard::FoundAndAttack(CoordStruct location)
{
	FootClass* pTechno = OwnerObject;
	const auto pType = OwnerObject->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->MyFighterData.AutoFire)
	{
		auto pPrimary = pTechno->GetWeapon(0);
		bool hasPrimary = pPrimary && pPrimary->WeaponType && !pPrimary->WeaponType->NeverUse;
		auto pSecondary = pTechno->GetWeapon(1);
		bool hasSecondary = pSecondary && pSecondary->WeaponType && !pSecondary->WeaponType->NeverUse;

		if (hasPrimary || hasSecondary)
		{
			CoordStruct sourcePos = location;
			if (!pTypeExt->MyFighterData.FindRangeAroundSelf)
			{
				sourcePos = destCenter;
			}

			int cellSpread = pTypeExt->MyFighterData.GuardRange;

			AbstractClass* pTarget = nullptr;
			const bool canAA = (hasPrimary && pPrimary->WeaponType->Projectile->AA) || (hasSecondary && pSecondary->WeaponType->Projectile->AA);

			double dist = (cellSpread <= 0 ? 1 : cellSpread) * 255;
			for (auto const pTargetTechno : Helpers::Alex::getCellSpreadItems(sourcePos, dist, canAA))
			{
				if (CheckTarget(pTargetTechno))
				{
					pTarget = pTargetTechno;
					break;
				}
			}

			if (pTarget)
			{
				pTechno->SetTarget(pTarget);
				pTechno->QueueMission(Mission::Attack, true);
				return true;
			}
		}
	}
	return false;
}

bool FighterAreaGuard::CanAttack(TechnoClass* pTarget, bool isPassiveAcquire)
{
	TechnoClass* pThis = OwnerObject;
	bool canAttack = false;
	const auto WeaponIdx = pThis->SelectWeapon(pTarget);
	const auto pWeapon = pThis->GetWeapon(WeaponIdx);

	if (pWeapon && pWeapon->WeaponType)
	{
		const auto pWH = pWeapon->WeaponType->Warhead;
		const auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWH);
		const auto versus = &pWarheadExt->GetVerses(pTarget->GetType()->Armor);

		if (isPassiveAcquire)
		{
			canAttack = versus->Verses > 0.2 || versus->Flags.PassiveAcquire;
		}
		else
		{
			canAttack = versus->Verses != 0.0;
		}

		if (canAttack && pTarget->IsInAir())
		{
			canAttack = pWeapon->WeaponType->Projectile->AA;
		}
	}

	return canAttack;
}

bool FighterAreaGuard::CheckTarget(TechnoClass* pTarget)
{
	auto pThis = OwnerObject;
	if (!pTarget
				|| pTarget->InLimbo
				|| !pTarget->IsAlive
				|| !pTarget->Health
				|| pTarget->IsSinking
				|| pTarget->IsCrashing
				|| pTarget->TemporalTargetingMe
				|| pTarget->BeingWarpedOut
		)

		false;

	const auto IsBuilding = !Is_Building(pTarget);
	if (!IsBuilding && TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTarget)))
		return false;
	else if (IsBuilding)
	{
		auto const pBldExt = BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pTarget));
		if (pBldExt->LimboID != -1)
			return false;
	}

	const auto pTechTypeHere = pTarget->GetTechnoType();
	if (pTechTypeHere->Insignificant)
		return false;

	const auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTechTypeHere);

	if (pTargetTypeExt->IsDummy)
		return false;

	auto pOwnerHouse = pTarget->GetOwningHouse();

	if (pTarget->IsDisguised() && !pTarget->IsClearlyVisibleTo(pThis->Owner))
		pOwnerHouse = pTarget->GetDisguiseHouse(true);

	if (!pOwnerHouse)
		return false;

	if (pOwnerHouse->IsAlliedWith(pThis->Owner))
		return false;

	if (pOwnerHouse == HouseExt::FindCivilianSide())
	{

		auto AutoRepel = [](HouseClass* pHouse)
		{
			return
				(pHouse->IsControlledByHuman() ?
					RulesExt::Global()->AutoRepelAI : RulesExt::Global()->AutoRepelPlayer).Get();
		};

		if (!pTargetTypeExt->CivilianEnemy && AutoRepel(pThis->Owner) && pTarget->Target)
		{
			if (auto pTargetTargetTech = generic_cast<TechnoClass*>(pTarget->Target))
				if (pThis->Owner->IsAlliedWith(pTargetTargetTech))
					pTarget = pTargetTargetTech;
		}
	}

	return CanAttack(pTarget, true);
}
