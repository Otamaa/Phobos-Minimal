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

static constexpr std::array<CoordStruct, 6> areaGuardCoords
{
	{
		{-300, -300, 0}
		, { -300 ,0,0 }
			, { 0,0,0 }
			, { 300,0,0 }
			, { 300,300,0 }
			, { 0 , 300 ,0 }
	}
};

bool contains(std::list<CoordStruct>& List, const CoordStruct& data)
{
	for (auto const& coord : List)
	{
		if (coord == data)
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
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto NewUpdate = [&]()
	{

		auto locomotion = pTechno->Locomotor.GetInterfacePtr();

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

				if (destNow != destCenter
					&& !contains(destList, destNow)
					&& pTechno->CurrentMission != Mission::Area_Guard)
				{
					locomotion->Move_To((*std::next(destList.begin(), destIndex)));
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
					locomotion->Move_To((*std::next(destList.begin(), destIndex)));
					if (++destIndex >= (int)destList.size())
					{
						destIndex = 0;
					}
				}
			}
		}
		break;
		}
	};

	auto OldUpdate = [&]()
	{
		if (OwnerObject->Spawned)
			return;

		if (!pTypeExt->MyFighterData.AreaGuard)
			return;

		auto& nDataType = pTypeExt->MyFighterData;

		if (OwnerObject->CurrentMission == Mission::Move)
		{
			this->isAreaProtecting = false;
			this->isAreaGuardReloading = false;
			return;
		}

		if (auto pFoot = (FootClass*)(OwnerObject))
		{
			if (!this->isAreaProtecting)
			{
				if (OwnerObject->CurrentMission == Mission::Area_Guard)
				{
					this->isAreaProtecting = true;
					CoordStruct dest = pFoot->Locomotor.GetInterfacePtr()->Destination();
					this->areaProtectTo = dest;
				}
			}

			if (this->isAreaProtecting)
			{
				//没弹药的情况下返回机场
				if (OwnerObject->Ammo == 0 && !this->isAreaGuardReloading)
				{
					OwnerObject->SetTarget(nullptr);
					OwnerObject->SetDestination(nullptr, false);
					OwnerObject->ForceMission(Mission::Stop);
					this->isAreaGuardReloading = true;
					return;
				}

				//填弹完毕后继续巡航
				if (this->isAreaGuardReloading)
				{
					if (OwnerObject->Ammo >= nDataType.MaxAmmo)
					{
						this->isAreaGuardReloading = false;
						OwnerObject->ForceMission(Mission::Area_Guard);
					}
					else
					{
						if (OwnerObject->CurrentMission != Mission::Sleep &&
							OwnerObject->CurrentMission != Mission::Enter)
						{
							if (OwnerObject->CurrentMission == Mission::Guard)
							{
								OwnerObject->ForceMission(Mission::Sleep);
							}
							else
							{
								OwnerObject->ForceMission(Mission::Enter);
							}
							return;
						}
					}
				}

				if (OwnerObject->CurrentMission == Mission::Move)
				{
					this->isAreaProtecting = false;
					return;
				}
				else if (OwnerObject->CurrentMission == Mission::Attack)
				{
					return;
				}
				else if (OwnerObject->CurrentMission == Mission::Enter)
				{
					if (this->isAreaGuardReloading)
					{
						return;
					}
					else
					{
						OwnerObject->ForceMission(Mission::Stop);
					}
				}
				else if (OwnerObject->CurrentMission == Mission::Sleep)
				{
					if (this->isAreaGuardReloading)
					{
						return;
					}
				}

				if (this->areaProtectTo)
				{
					auto dest = this->areaProtectTo;
					auto house = OwnerObject->Owner;

					if (pTypeExt->MyFighterData.AutoFire)
					{
						if (this->areaProtectTo.DistanceFrom(OwnerObject->GetCoords()) <= 2000)
						{
							if (this->areaGuardTargetCheckRof-- <= 0)
							{
								this->areaGuardTargetCheckRof = 20;

								auto FindOneTechno = [&](HouseClass* pOwner)
								{
									TechnoClass* pDummy = nullptr;
									for (const auto pTech : *TechnoClass::Array())
									{
										if (!pTech
											|| pTech->InLimbo
											|| !pTech->IsAlive
											|| !pTech->Health
											|| pTech->IsSinking
											|| pTech->IsCrashing
											|| pTech->TemporalTargetingMe
											|| pTech->BeingWarpedOut)
											continue;

										bool IsBuilding = false;
										if (const auto pBuilding = specific_cast<BuildingClass*>(pTech))
										{
											IsBuilding = true;
											if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1)
												continue;

										}else if(TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTech)))
											continue;

										const auto pOwnerHouse = pTech->GetOwningHouse();
										const auto pTechTypeHere = pTech->GetTechnoType();

										if (!pOwnerHouse ||
											pOwnerHouse->IsAlliedWith_(pOwner) ||
											pOwnerHouse == pOwner ||
											pOwnerHouse->IsNeutral() ||
											pTech == OwnerObject ||
											pTechTypeHere->Immune ||
											pTechTypeHere->Invisible
											)
											continue;

										auto coords = pTech->GetCoords();
										auto height = pTech->GetHeight();
										//auto type = pTech->WhatAmI();

										if (pTech->InLimbo)
											continue;

										auto bounsRange = 0;
										if (pTech->GetHeight() > 10)
											bounsRange = nDataType.GuardRange;

										const auto nDummy = CoordStruct { 0, 0, height };
										if ((coords - nDummy).DistanceFrom(dest)
										<= (nDataType.GuardRange * 256 + bounsRange) &&
											 !IsBuilding)
										{
											pDummy = pTech;
											break;
										}
									}

									return pDummy;
								};

								if (TechnoClass* pTarget = FindOneTechno(house))
								{
									OwnerObject->SetTarget(pTarget);
									OwnerObject->ForceMission(Mission::Stop);
									OwnerObject->ForceMission(Mission::Attack);
									return;
								}
							}
						}
					}

					if (this->areaProtectTo.DistanceFrom(OwnerObject->GetCoords()) <= 2000)
					{
						if (this->currentAreaProtectedIndex > (int)(areaGuardCoords.size() - 1))
						{
							this->currentAreaProtectedIndex = 0;
						}
						dest += areaGuardCoords[this->currentAreaProtectedIndex];
						this->currentAreaProtectedIndex++;
					}

					pFoot->Locomotor.GetInterfacePtr()->Move_To(dest);
					if (auto const pCell = MapClass::Instance->GetCellAt(dest))
					{
						OwnerObject->SetDestination(pCell, false);
					}
				}
			}
		}
	};

	OldUpdate();
}

void FighterAreaGuard::OnStopCommand()
{
	//FootClass* pTechno = OwnerObject;
	//onStopCommand = true;
	//CancelAreaGuard();
	//State = AircraftGuardState::STOP;
	//pTechno->ForceMission(Mission::Enter);
}

void FighterAreaGuard::StartAreaGuard()
{
	FootClass* pTechno = OwnerObject;

	switch (State)
	{
	case AircraftGuardState::READY:
	case AircraftGuardState::GUARD:
	case AircraftGuardState::ROLLING:
		auto nCoord = pTechno->Destination
			? pTechno->Destination->GetCoords() : pTechno->Locomotor->Destination();

		if (SetupDestination(nCoord))
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

void PopulateDestList(std::list<CoordStruct>& destList, double sourceRad, CoordStruct flh, CoordStruct dest, bool clockwise)
{
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
				destList.push_front(newDest);
			}
			else
			{
				destList.push_back(newDest);
			}
		}
	}
}

bool FighterAreaGuard::SetupDestination(CoordStruct& dest)
{
	FootClass* pTechno = OwnerObject;
	const auto pType = OwnerObject->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (CoordStruct::Empty != dest && dest != destCenter && !contains(destList, dest))
	{
		destCenter = dest;
		CoordStruct location = pTechno->GetCoords();
		DirStruct sourceDir = Helpers_DP::Point2Dir(location, dest);
		double sourceRad = sourceDir.GetRadian();
		CoordStruct flh = CoordStruct { pTypeExt->MyFighterData.GuardRadius * 256, 0, 0 };
		destList.clear();

		const bool clockwise = pTypeExt->MyFighterData.Randomwise
			? ScenarioClass::Instance->Random.RandomFromMax(2) == 0 : pTypeExt->MyFighterData.Clockwise;

		Clockwise = clockwise;
		PopulateDestList(destList, sourceRad, flh, dest, clockwise);
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
	OwnerObject->EnterIdleMode(true, true);
}

bool FighterAreaGuard::FoundAndAttack(CoordStruct location)
{
	FootClass* pTechno = OwnerObject;
	const auto pType = OwnerObject->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->MyFighterData.AutoFire)
	{
		auto pPrimary = pTechno->GetWeapon(0);
		bool hasPrimary = pPrimary && pPrimary->WeaponType && !pPrimary->WeaponType->NeverUse;
		auto pSecondary = pTechno->GetWeapon(1);
		bool hasSecondary = pSecondary && pSecondary->WeaponType && !pSecondary->WeaponType->NeverUse;

		if (hasPrimary || hasSecondary)
		{
			const CoordStruct sourcePos = !pTypeExt->MyFighterData.FindRangeAroundSelf ? destCenter : location;
			AbstractClass* pTarget = nullptr;
			const bool canAA = (hasPrimary && pPrimary->WeaponType->Projectile->AA) || (hasSecondary && pSecondary->WeaponType->Projectile->AA);

			const double dist = (pTypeExt->MyFighterData.GuardRange <= 0 ? 1 : pTypeExt->MyFighterData.GuardRange) * 255;
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
		const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pWH);
		const auto versus = &pWarheadExt->GetVerses(TechnoExtData::GetTechnoArmor(pTarget, pWH));

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
	//TODO : there is some manual check for `CivilianEnemy` stuffs , idk how that really work
	//		 so remove it atm
	//auto pThis = OwnerObject;
	//TODO : using TechnoClass::GetFireError instead ,..

	if (!pTarget)
		return false;

	if (pTarget->InLimbo
				|| !pTarget->IsAlive
				|| !pTarget->Health
				|| pTarget->IsSinking
				|| pTarget->IsCrashing
				|| pTarget->TemporalTargetingMe
				|| pTarget->BeingWarpedOut
		)
	{
		return false;
	}

	if (pTarget->Owner && this->OwnerObject->Owner)
	{
		if (this->OwnerObject->Owner->IsAlliedWith_(pTarget))
			return false;
	}

	if (const auto pBuildingTarget = specific_cast<BuildingClass*>(pTarget))
	{
		if (BuildingExtContainer::Instance.Find(pBuildingTarget)->LimboID != -1)
			return false;
	}
	else if (pTarget->AbstractFlags & AbstractFlags::Foot)
	{
		if (TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTarget)))
			return false;
	}

	const auto pTechTypeHere = pTarget->GetTechnoType();
	if (pTechTypeHere->Insignificant)
		return false;

	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTechTypeHere);

	if (pTargetTypeExt->IsDummy)
		return false;

	return CanAttack(pTarget, true);
}
