#include "Body.h"

#include <Ext/AircraftType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/TerrainType/Body.h>

#include <AircraftClass.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>

#include <Utilities/Macro.h>

#include <Locomotor/FlyLocomotionClass.h>
#include <Phobos.SaveGame.h>

COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon)
{
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1
			&& !pWeapon->Projectile->Inviso)
		&& !BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->TrajectoryType;
}

bool AircraftExtData::FireWeapon(AircraftClass* pAir, AbstractClass* pTarget)
{
	const auto pExt = AircraftExtContainer::Instance.Find(pAir);
	const int weaponIndex = pExt->CurrentAircraftWeaponIndex;
	const bool Scatter = TechnoTypeExtContainer::Instance.Find(pAir->Type)->FiringForceScatter;
	auto pDecideTarget = (pExt->Strafe_TargetCell ? pExt->Strafe_TargetCell : pTarget);

	if (const auto pWeaponStruct = pAir->GetWeapon(weaponIndex))
	{
		if (const auto weaponType = pWeaponStruct->WeaponType)
		{
			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(weaponType);
			bool isStrafe = pAir->Is_Strafe();

			if (weaponType->Burst > 0)
			{
				for (int i = 0; i < weaponType->Burst; i++)
				{
					if (isStrafe && weaponType->Burst < 2 && pWeaponExt->Strafing_SimulateBurst)
						pAir->CurrentBurstIndex = pExt->Strafe_BombsDroppedThisRound % 2 == 0;

					pAir->Fire(pDecideTarget, weaponIndex);
				}

				if (isStrafe)
				{
					pExt->Strafe_BombsDroppedThisRound++;

					if (pWeaponExt->Strafing_UseAmmoPerShot)
					{
						pAir->Ammo--;
						pAir->loseammo_6c8 = false;

						if (!pAir->Ammo)
						{
							pAir->SetTarget(nullptr);
							pAir->SetDestination(nullptr, true);
						}
					}
				}
			}

		}
	}

	if (pDecideTarget)
	{
		if (Scatter)
		{

			auto coord = pDecideTarget->GetCoords();

			if (auto pCell = MapClass::Instance->TryGetCellAt(coord))
			{
				pCell->ScatterContent(coord, true, false, false);
			}
		}

		return true;
	}

	return false;
}

int AircraftExtData::GetDelay(AircraftClass* pThis, bool isLastShot)
{
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);
	auto const pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int delay = pWeapon->ROF;

	if (isLastShot || pExt->Strafe_BombsDroppedThisRound == pWeaponExt->Strafing_Shots.Get(5) || (pWeaponExt->Strafing_UseAmmoPerShot && !pThis->Ammo))
	{
		pExt->Strafe_TargetCell = nullptr;
		pThis->MissionStatus = (int)AirAttackStatus::FlyToPosition;
		delay = pWeaponExt->Strafing_EndDelay.Get((pWeapon->Range + 1024) / pThis->Type->Speed);
	}

	return delay;
}

long __stdcall AircraftClass_IFlyControl_IsStrafe(IFlyControl* ifly)
{
	auto pThis = static_cast<AircraftClass*>(ifly);
	WeaponTypeClass* pWeapon = nullptr;
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);

	pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;

	if (pWeapon)
		return (long)AircraftCanStrafeWithWeapon(pWeapon);

	return false;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2268, AircraftClass_IFlyControl_IsStrafe);

int FakeAircraftClass::_Mission_Attack()
{
	auto* pExt = AircraftExtContainer::Instance.Find(this);

	// 0x417FF1 - Top-of-function: weapon re-eval and strafe state bookkeeping
	{
		AirAttackStatus const state = static_cast<AirAttackStatus>(this->MissionStatus);

		if (state > AirAttackStatus::ValidateAZ && state < AirAttackStatus::FireAtTarget)
			pExt->CurrentAircraftWeaponIndex = MaxImpl(this->SelectWeapon(this->Target), 0);

		if (this->MissionStatus < static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe)
			|| this->MissionStatus > static_cast<int>(AirAttackStatus::FireAtTarget5_Strafe))
		{
			pExt->Strafe_BombsDroppedThisRound = 0;
		}

		if (pExt->Strafe_BombsDroppedThisRound)
		{
			auto const pWeapon = this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			int const  count = pWeaponExt->Strafing_Shots.Get(5);

			if (count > 5
				&& this->MissionStatus == static_cast<int>(AirAttackStatus::FireAtTarget3_Strafe)
				&& (count - 3 - pExt->Strafe_BombsDroppedThisRound) > 0)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe);
			}
		}
	}

	// --- Helpers ---

	auto MissionRate = [this]() -> int
		{
			auto ctrl = this->GetCurrentMissionControl();
			return static_cast<int>(ctrl->Rate * TICKS_PER_MINUTE)
				+ ScenarioClass::Instance->Random.RandomRanged(0, 2);
		};

	auto CurleyShuffle = [this]() -> bool
		{
			return TechnoTypeExtContainer::Instance.Find(this->Type)
				->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle);
		};

	auto ReturnToBaseNow = [this]() -> int
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
			return 1;
		};

	auto HandleOutOfRange = [&]() -> int
		{
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				this->IsLocked = 0;
			}
			return 1;
		};

	auto SelectWeaponBeforeFiring = [&]() -> int
		{
			if (!pExt->Strafe_BombsDroppedThisRound)
				pExt->CurrentAircraftWeaponIndex = MaxImpl(this->SelectWeapon(this->Target), 0);
			return pExt->CurrentAircraftWeaponIndex;
		};

	// CurleyShuffle A/B/C/D: pick re-approach or continue attacking
	auto SetCurleyStatus = [&]()
		{
			this->MissionStatus = CurleyShuffle()
				? static_cast<int>(AirAttackStatus::PickAttackLocation)
				: static_cast<int>(AirAttackStatus::FireAtTarget);
		};

	// Used in default cases: if in range defer to CurleyShuffle, otherwise always re-approach
	auto SetRangeBasedStatus = [&]()
		{
			if (this->IsCloseEnoughToAttack(this->Target))
				SetCurleyStatus();
			else
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
		};

	// Shared logic for FACING and default in FireAtTarget2:
	//   checkStrafe45 = true  -> FACING case (also returns 45 if strafing)
	//   checkStrafe45 = false -> default case (no strafe-45 path)
	auto HandleCantFire = [&](bool checkStrafe45) -> int
		{
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				return MissionRate();
			}
			if (checkStrafe45 && (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe()))
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			}
			else if (!checkStrafe45)
			{
				SetRangeBasedStatus();
			}
			else
			{
				SetCurleyStatus();
			}
			if (checkStrafe45 && this->Is_Strafe())
				return 45;
			return MissionRate();
		};

	auto SetupReturnFlight = [&]() -> int
		{
			this->IsLocked = 0;
			CellStruct edgeCell = MapClass::Instance->PickCellOnEdge(
				this->Owner->GetCurrentEdge(),
				CellStruct::Empty, CellStruct::Empty,
				SpeedType::Winged, true, MovementZone::Normal);
			this->SetDestination(MapClass::Instance->GetCellAt(edgeCell), true);
			this->NumParadropsLeft = 0;
			if (this->Airstrike && this->Ammo > 0)
				this->QueueMission(Mission::Retreat, false);
			else
				this->EnterIdleMode(false, true);
			this->NumParadropsLeft = true;
			return 1;
		};

	// ---

	switch (static_cast<AirAttackStatus>(this->MissionStatus))
	{
	case AirAttackStatus::ValidateAZ:
	{
		this->IsLocked = 0;
		this->MissionStatus = this->Target
			? static_cast<int>(AirAttackStatus::PickAttackLocation)
			: static_cast<int>(AirAttackStatus::ReturnToBase);
		return 1;
	}

	case AirAttackStatus::PickAttackLocation:
	{
		this->IsLocked = 0;
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			this->Ammo--;
		}
		if (this->Target && this->Ammo)
		{
			this->SetDestination(this->GoodTargetLoc_(this->Target), true);
			this->MissionStatus = this->Destination
				? static_cast<int>(AirAttackStatus::FlyToPosition)
				: static_cast<int>(AirAttackStatus::ReturnToBase);
		}
		else
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
		}
		return MissionRate();
	}

	case AirAttackStatus::FlyToPosition:
	{
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			this->Ammo--;
		}
		this->IsLocked = 0;

		if (!this->Target || !this->Ammo)
			return ReturnToBaseNow();

		if (this->Is_Strafe())
		{
			// 0x4180F4 - use CurrentAircraftWeaponIndex instead of slot 0 for range check
			auto const* wt = this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
			if (this->DistanceFrom(this->Target) < wt->Range)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				return 1;
			}
			this->SetDestination(this->Target, true);
		}
		else
		{
			if (this->Is_Locked())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				return 1;
			}
			if (!this->Locomotor.GetInterfacePtr()->Is_Moving_Now())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				return 1;
			}
		}

		if (!this->Destination)
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			return 1;
		}

		int const dist = this->DistanceFrom(this->Destination);
		if (dist >= 512)
		{
			CoordStruct myPos;
			CoordStruct navCenter = this->Destination->GetCenterCoords();
			FakeTechnoClass::__Get_FLH(this, discard_t(), &myPos, 0, {});

			DirStruct dir;
			if (myPos.X == navCenter.X && myPos.Y == navCenter.Y)
			{
				dir.Raw = 0;
			}
			else
			{
				double angle = Math::atan2(float(myPos.Y - navCenter.Y), navCenter.X - myPos.X);
				dir.Raw = static_cast<short>((angle - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC);
			}
			this->SecondaryFacing.Set_Desired(dir);
		}
		else
		{
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
			if (dist < 16)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				this->SetDestination(nullptr, true);
			}
		}
		return 1;
	}

	case AirAttackStatus::FireAtTarget:
	{
		if (!this->Target || !this->Ammo)
			return ReturnToBaseNow();

		if (!this->Is_Strafe())
		{
			this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
		}

		int const weapSlot = SelectWeaponBeforeFiring(); // 0x41831E

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		{
			this->loseammo_6c8 = true; // 0x418403
			AircraftExtData::FireWeapon(this, this->Target);

			if (this->Is_Strafe())
			{
				// 0x4184CC - Delay1A
				auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(
					this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType);
				if (pWeaponExt->Strafing_TargetCell)
					pExt->Strafe_TargetCell = MapClass::Instance->GetCellAt(this->Target->GetCoords());
				this->IsLocked = true;
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe);
				return AircraftExtData::GetDelay(this, false);
			}

			if (!this->Is_Locked())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2);
				return 1;
			}

			// 0x418506 - Delay1B
			this->IsLocked = true;
			this->MissionStatus = this->Ammo > 0
				? static_cast<int>(AirAttackStatus::PickAttackLocation)
				: static_cast<int>(AirAttackStatus::ReturnToBase);
			return AircraftExtData::GetDelay(this, false);
		}
		case FireError::FACING:
		{
			if (!this->Ammo)
				return ReturnToBaseNow();

			if (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			}
			else if (this->Is_Locked())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
			}
			else
			{
				SetCurleyStatus(); // 0x4183C3 CurleyShuffle_A
			}
			if (this->Is_Strafe())
				return 45;
			return 1;
		}
		case FireError::REARM:
			return 1;

		case FireError::CLOAKED:
			this->Uncloak(false);
			return 1;

		case FireError::RANGE:
			if (this->Is_Strafe()) // 0x418544 StrafingDestinationFix
				this->SetDestination(this->Target, true);
			[[fallthrough]];

		default:
			if (!this->Ammo)
				return ReturnToBaseNow();
			if (this->Is_Strafe())
				return 1;
			this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget2:
	{
		if (!this->Target)
			return ReturnToBaseNow();

		this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
		this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));

		int const weapSlot = SelectWeaponBeforeFiring(); // 0x4185F5

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		{
			AircraftExtData::FireWeapon(this, this->Target); // 0x4186B6
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				return MissionRate();
			}
			SetCurleyStatus(); // 0x418733 CurleyShuffle_C
			return MissionRate();
		}
		case FireError::FACING:
			return HandleCantFire(true);  // 0x418671 CurleyShuffle_B, strafe-45 applies

		case FireError::REARM:
			return MissionRate();

		case FireError::CLOAKED:
			this->Uncloak(false);
			return MissionRate();

		case FireError::RANGE:
			if (this->Is_Strafe()) // 0x41874E StrafingDestinationFix
				this->SetDestination(this->Target, true);
			[[fallthrough]];

		default:
			return HandleCantFire(false); // 0x418782 CurleyShuffle_D, no strafe-45
		}
	}

	// Strafe states 2-4: SelectWeaponBeforeFiring -> Can_Fire -> FireWeapon
	// -> if fired: SetDestination -> GetDelay(false) to next strafe state
#define STRAFE_FIRE_CASE(CaseName, NextState)                               \
    case AirAttackStatus::CaseName:                                         \
    {                                                                       \
        if (!this->Target)                                                  \
            return ReturnToBaseNow();                                       \
        int const weapSlot = SelectWeaponBeforeFiring();                    \
        switch (this->GetFireError(this->Target, weapSlot, true))           \
        {                                                                   \
        case FireError::OK:                                                 \
        case FireError::FACING:                                             \
        case FireError::CLOAKED:                                            \
            break;                                                          \
        case FireError::RANGE:                                              \
            this->SetDestination(this->Target, true);                      \
            break;                                                          \
        default:                                                            \
            return HandleOutOfRange();                                      \
        }                                                                   \
        if (AircraftExtData::FireWeapon(this, this->Target))                                 \
            this->SetDestination(this->Target, true);                      \
        this->MissionStatus = static_cast<int>(AirAttackStatus::NextState);\
        return AircraftExtData::GetDelay(this, false);                                       \
    }

	STRAFE_FIRE_CASE(FireAtTarget2_Strafe, FireAtTarget3_Strafe) // 0x418805 + 0x418883
		STRAFE_FIRE_CASE(FireAtTarget3_Strafe, FireAtTarget4_Strafe) // 0x418914 + 0x418992
		STRAFE_FIRE_CASE(FireAtTarget4_Strafe, FireAtTarget5_Strafe) // 0x418A23 + 0x418AA1
#undef STRAFE_FIRE_CASE

	case AirAttackStatus::FireAtTarget5_Strafe:
	{
		if (!this->Target)
			return ReturnToBaseNow();

		int const weapSlot = SelectWeaponBeforeFiring();

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		case FireError::FACING:
		case FireError::RANGE:
		case FireError::CLOAKED:
			AircraftExtData::FireWeapon(this, this->Target); // 0x418B1F
			return AircraftExtData::GetDelay(this, true);    // 0x418B8A isLastShot=true
		default:
			return HandleOutOfRange();
		}
	}

	case AirAttackStatus::ReturnToBase:
	{
		this->IsLocked = 0;
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			if (this->Ammo > 0)
				this->Ammo--;
		}

		if (this->Ammo)
		{
			// 0x418CD1 - ContinueFlyToDestination
			if (this->Target)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
				return 1;
			}
			if (RulesExtData::Instance()->ExpandAircraftMission
				&& this->MegaMissionIsAttackMove()
				&& this->MegaDestination)
			{
				this->SetDestination(reinterpret_cast<AbstractClass*>(this->MegaDestination), false);
				this->QueueMission(Mission::Move, true);
				this->QueueMission(Mission::Move, true);
				this->HaveAttackMoveTarget = false;
				return 1;
			}
			// No target and no attack-move: fly out even with ammo remaining
			return SetupReturnFlight();
		}

		if (this->Spawned || this->Owner->IsControlledByHuman())
			this->SetTarget(nullptr);

		return SetupReturnFlight();
	}

	default:
		return MissionRate();
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x417FE0, FakeAircraftClass::_Mission_Attack);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E24B4, FakeAircraftClass::_Mission_Attack);

AbstractClass* FakeAircraftClass::_GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy)
{
	if (RulesExtData::Instance()->ExpandAircraftMission && !this->Team && this->Ammo && !this->Airstrike && !this->Spawned)
	{
		if (WeaponTypeClass* const pPrimaryWeapon = this->GetWeapon(0)->WeaponType)
			threatType |= pPrimaryWeapon->AllowedThreats();

		if (WeaponTypeClass* const pSecondaryWeapon = this->GetWeapon(1)->WeaponType)
			threatType |= pSecondaryWeapon->AllowedThreats();
	}

	return this->FootClass::GreatestThreat(threatType, pSelectCoords, onlyTargetHouseEnemy); // FootClass_GreatestThreat (Prevent circular calls)
}

void FakeAircraftClass::_FootClass_Update_Wrapper()
{
	auto pExt = TechnoExtContainer::Instance.Find(this);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);


	//pExt->UpdateAircraftOpentopped();
	AircraftPutDataFunctional::AI(pExt, pTypeExt);
	AircraftDiveFunctional::AI(pExt, pTypeExt);
	//FighterAreaGuardFunctional::AI(pExt, pTypeExt);

	//if (pThis->IsAlive && pThis->SpawnOwner != nullptr)
	//{
	//
	//	/**
	//	 *  If we are close enough to our owner, delete us and return true
	//	 *  to signal to the challer that we were deleted.
	//	 */
	//	if (Spawned_Check_Destruction(pThis))
	//	{
	//		pThis->UnInit();
	//		return 0x414F99;
	//	}
	//}

	this->FootClass::Update();

	if (this->IsAlive && this->Type->AirportBound && !this->Airstrike && !this->Spawned)
	{
		const bool extendedMissions = RulesExtData::Instance()->ExpandAircraftMission;

		if (extendedMissions)
		{
			// Check area guard range
			if (const auto pArchive = this->ArchiveTarget)
			{
				if (this->Target && !this->IsFiring && !this->IsLocked
					&& this->DistanceFromSquared(pArchive) > static_cast<int>(this->GetGuardRange(1) * 1.1))
				{
					this->SetTarget(nullptr);
					this->SetDestination(pArchive, true);
				}
			}

			// Check dock building
			this->FindDockingBayInVector(reinterpret_cast<TypeList<TechnoTypeClass*>*>(&this->Type->Dock), 0, 0);
		}

		if (this->DockedTo)
		{
			// Exit the aimless hovering state and return to the new airport
			if (this->GetCurrentMission() == Mission::Area_Guard && this->MissionStatus)
			{
				this->SetArchiveTarget(nullptr);
				this->EnterIdleMode(false, true);
			}
		}
		else if (this->IsInAir())
		{
			int damage = AircraftTypeExtContainer::Instance.Find(this->Type)->ExtendedAircraftMissions_UnlandDamage.Get(RulesExtData::Instance()->ExtendedAircraftMissions_UnlandDamage);

			if (damage > 0)
			{
				if (!extendedMissions && !this->IsCrushingSomething && this->FindDockingBayInVector(reinterpret_cast<TypeList<TechnoTypeClass*>*>(&this->Type->Dock), 0, 0))
					return;

				// Injury every four frames
				if (!((Unsorted::CurrentFrame - this->LastFireBulletFrame + this->UniqueID) & 0x3))
					this->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
			else if (damage < 0)
			{
				// Avoid using circular movement paths to prevent the aircraft from crashing
				if (extendedMissions)
					this->Crash(nullptr);
			}
		}
	}
}

COMPILETIMEEVAL FORCEDINLINE bool IsFlyLoco(const ILocomotion* pLoco) {
	return (((DWORD*)pLoco)[0] == FlyLocomotionClass::ILoco_vtable);
}

//COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon) {
//	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
//		.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
//}

NOINLINE void CalculateVelocity(AircraftClass* pThis , BulletClass* pBullet , AbstractClass* pTarget) {
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (pBullet->HasParachute ||(pBullet->Type->Vertical && pBulletTypeExt->Vertical_AircraftFix)) {
		return;
	}

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	const auto pLoco = pThis->Locomotor.GetInterfacePtr();

	if (pBullet->Type->ROT == 0 && !PhobosTrajectory::IgnoreAircraftROT0(pBulletExt->Trajectory))
	{
		const auto pLocomotor = static_cast<LocomotionClass*>(pLoco);
		double aircraftSpeed = !pBullet->Type->Cluster && IsFlyLoco(pLoco) ?
			pThis->Type->Speed * static_cast<FlyLocomotionClass*>(pLoco)->CurrentSpeed * TechnoExtData::GetCurrentSpeedMultiplier(pThis)
			: pLocomotor->Apparent_Speed();

		VelocityClass* velocity = &pBullet->Velocity;

		velocity->SetIfZeroXYZ();

		const double dist = velocity->Length();
		const double scale = aircraftSpeed / dist;

		velocity->X *= scale;
		velocity->Y *= scale;
		velocity->Z *= scale;

		DirStruct dir = velocity->GetDirectionFromXY();
		const int facingOffset = dir.Raw - Math::BINARY_ANGLE_MASK;
		const double yawRad = facingOffset * Math::DIRECTION_FIXED_MAGIC;
		const double mag = velocity->Length();

		if (yawRad != 0.0)
		{
			velocity->X /= Math::cos(yawRad);
			velocity->Y /= Math::cos(yawRad);
		}

		velocity->X *= Math::COS_DIRECTION_FIXED_MAGIC;
		velocity->Y *= Math::SIN_DIRECTION_FIXED_MAGIC;
		velocity->Z = Math::SIN_DIRECTION_FIXED_MAGIC * mag;

		const DirStruct newFacingDir = pThis->SecondaryFacing.Current();

		velocity->SetIfZeroXY();

		const double dist2D = velocity->LengthXY();
		const int newFacing = newFacingDir.Raw - Math::BINARY_ANGLE_MASK;
		const double newRad = newFacing * Math::DIRECTION_FIXED_MAGIC;

		velocity->X = Math::cos(newRad) * dist2D;
		velocity->Y = -Math::sin(newRad) * dist2D;

	} else if (pBullet->Type->ROT == 1)
		{

			// Homing weapon: calculate angle and scale
			CoordStruct src = pThis->GetCoords();
			CoordStruct tgt = pTarget->GetCoords();

			CoordStruct offset = tgt - src;

			// Copy offset components into double vector for math
			Vector3D aimVector = {
				static_cast<double>(offset.X),
				static_cast<double>(offset.Y),
				static_cast<double>(offset.Z)
			};

			// Calculate yaw angle to face the target in XY plane
			double yawRadians = Math::atan2(-aimVector.Y, aimVector.X) - Math::DEG90_AS_RAD;
			int yawBinaryAngle = static_cast<int>(yawRadians * Math::BINARY_ANGLE_MAGIC);
			int adjustedYaw = yawBinaryAngle - Math::BINARY_ANGLE_MASK;
			double adjustedYawRad = adjustedYaw * Math::DIRECTION_FIXED_MAGIC;

			// Prepare bullet velocity (set if all-zero)
			VelocityClass* velocity = &pBullet->Velocity;

			velocity->SetIfZeroXY();

			double originalSpeed2D = velocity->LengthXY();

			// Set initial XY velocity facing target yaw
			velocity->X = Math::cos(adjustedYawRad) * originalSpeed2D;
			velocity->Y = -Math::sin(adjustedYawRad) * originalSpeed2D;

			// Calculate pitch angle from aim vector
			double horizontalDistance = aimVector.LengthXY();
			double pitchRadians = Math::atan2(aimVector.Z, horizontalDistance) - Math::DEG90_AS_RAD;
			int pitchBinaryAngle = static_cast<int>(pitchRadians * Math::BINARY_ANGLE_MAGIC);
			int adjustedPitch = pitchBinaryAngle - Math::BINARY_ANGLE_MASK;
			double adjustedPitchRad = adjustedPitch * Math::DIRECTION_FIXED_MAGIC;

			// Re-calculate current yaw from bullet velocity
			DirStruct currentFacing = velocity->GetDirectionFromXY();
			int currentFacingOffset = currentFacing.Raw - Math::BINARY_ANGLE_MASK;
			double currentYawRad = currentFacingOffset * Math::DIRECTION_FIXED_MAGIC;

			double currentSpeed3D = velocity->Length();

			// If yaw was altered, rescale velocity
			if (currentYawRad != 0.0)
			{
				double cosYaw = Math::cos(currentYawRad);
				velocity->X /= cosYaw;
				velocity->Y /= Math::cos(currentYawRad); // redundant, matches original logic
			}

			// Apply pitch to Z velocity
			velocity->X *= Math::cos(adjustedPitchRad);
			velocity->Y *= Math::cos(adjustedPitchRad);
			velocity->Z = Math::sin(adjustedPitchRad) * currentSpeed3D;

			// Normalize speed to weapon's max speed
			WeaponTypeClass* weapon = pThis->GetPrimaryWeapon()->WeaponType;
			double maxBulletSpeed = static_cast<double>(weapon->Speed);

			velocity->SetIfZeroXYZ();

			double finalSpeed = velocity->Length();
			double speedScale = maxBulletSpeed / finalSpeed;

			velocity->X *= speedScale;
			velocity->Y *= speedScale;
			velocity->Z *= speedScale;
	}
}

BulletClass* FakeAircraftClass::_FireAt(AbstractClass* pTarget, int nWeaponIdx) {

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);
	bool DropPassengers = pTypeExt->Paradrop_DropPassangers;

	if (this->Passengers.FirstPassenger)
	{
		if (auto pWewapons = this->GetWeapon(nWeaponIdx))
		{
			if (pWewapons->WeaponType)
			{
				const auto pExt = WeaponTypeExtContainer::Instance.Find(pWewapons->WeaponType);
				if (pExt->KickOutPassenger.isset())
					DropPassengers = pExt->KickOutPassenger; //#1151
			}
		}

		if (DropPassengers)
		{
			this->DropOffParadropCargo();
			return nullptr;
		}
	}

	BulletClass* pBullet = this->TechnoClass::Fire(pTarget, nWeaponIdx);

	if(pBullet) {

		if (AircraftCanStrafeWithWeapon(pBullet->WeaponType))
		{
			AircraftExtContainer::Instance.Find(this)->Strafe_BombsDroppedThisRound++;

			if (WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType)->Strafing_UseAmmoPerShot)
			{
				this->loseammo_6c8 = false;
				this->Ammo--;
			}
		}

		if(!pTypeExt->Firing_IgnoreGravity)
			CalculateVelocity(this, pBullet, pTarget);
	}
	// Reveal map for attacking aircraft if controlled by player

	if (this->Owner->ControlledByCurrentPlayer())
	{
		CoordStruct coord = this->Location;

		if (!MapClass::Instance->IsLocationShrouded(coord)) {
			bool mapped = false;
			constexpr CoordStruct offsets[4] = {
				{512, 512 , 0}, {-512, -512 , 0}, {512, -512 , 0}, {-512, 512 , 0}
			};

			for (auto& off : offsets) {
				CoordStruct probe = off + coord;

				if (MapClass::Instance->IsLocationShrouded(probe)) {
					mapped = true;
					break;
				}
			}

			if (!mapped) {
				CoordStruct tgtCenter = pTarget->GetCoords();
				mapped = MapClass::Instance->IsLocationShrouded(tgtCenter);
			}

			if (mapped) {
				const int sightRange = TechnoTypeExtContainer::Instance.Find(this->Type)->AttackingAircraftSightRange.Get(RulesClass::Instance->AttackingAircraftSightRange);
				MapClass::Instance->RevealArea2(&coord, sightRange, this->Owner, 0, 0, 0, 1, 0);
				MapClass::Instance->RevealArea2(&coord, sightRange, this->Owner, 0, 0, 0, 1, 1);
			}
		}
	}

	if (this->IsKamikaze) {
		this->UnInit();
	}

	return pBullet;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x415EE0, FakeAircraftClass::_FireAt);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2670, FakeAircraftClass::_FireAt);

void FakeAircraftClass::_SetTarget(AbstractClass* pTarget)
{
	this->TechnoClass::SetTarget(pTarget);
	AircraftExtContainer::Instance.Find(this)->CurrentAircraftWeaponIndex = -1;
}

void FakeAircraftClass::_Destroyed(int mult)
{
	AircraftExtData::TriggerCrashWeapon(this, mult);
}

WeaponStruct* FakeAircraftClass::_GetWeapon(int weaponIndex)
{
	auto const pExt = AircraftExtContainer::Instance.Find(this);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		return this->TechnoClass::GetWeapon(pExt->CurrentAircraftWeaponIndex);
	else
		return this->TechnoClass::GetWeapon(this->SelectWeapon(this->Target));
}

// Spy plane, airstrike etc.
bool AircraftExtData::PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto coords = CellClass::Cell2Coord(edgeCell);
	AbstractClass* pTarget = nullptr;

	if (pTypeExt->SpawnDistanceFromTarget.isset())
	{
		pTarget = pThis->Target ? pThis->Target : pThis->Destination;

		if (pTarget)
			coords = GeneralUtils::CalculateCoordsFromDistance(CellClass::Cell2Coord(edgeCell), pTarget->GetCoords(), pTypeExt->SpawnDistanceFromTarget.Get());
	}

	++Unsorted::ScenarioInit;
	const bool result = pThis->Unlimbo(coords, DirType::North);
	--Unsorted::ScenarioInit;

	pThis->SetHeight(pTypeExt->SpawnHeight.Get(pThis->Type->GetFlightLevel()));

	if (pTarget)
		pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pTarget));

	return result;
}

void AircraftExtData::TriggerCrashWeapon(AircraftClass* pThis, int nMult)
{
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pCrashWeapon = pTypeExt->CrashWeapon.Get(pThis);
	if (!pCrashWeapon)
		pCrashWeapon = pTypeExt->CrashWeapon_s.Get();

	if (!TechnoExtData::FireWeaponAtSelf(pThis, pCrashWeapon))
		pThis->FireDeathWeapon(nMult);

	AnimTypeExtData::ProcessDestroyAnims(pThis, nullptr);
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	if (!pTarget)
		return;

	AircraftExtData::FireBurst(pThis, pTarget, shotNumber, pThis->SelectWeapon(pTarget));
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	const auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	const auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	AircraftExtData::FireBurst(pThis , pTarget, shotNumber, WeaponIdx, weaponType);
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
{
	if (!pWeapon->Burst)
		return;

	for (int i = 0; i < pWeapon->Burst; i++)
	{
		if (pWeapon->Burst < 2 && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing_SimulateBurst)
			pThis->CurrentBurstIndex = (int)shotNumber;

		pThis->Fire(pTarget, WeaponIdx);
	}
}


bool AircraftExtData::IsValidLandingZone(AircraftClass* pThis)
{
	if (const auto pPassanger = pThis->Passengers.GetFirstPassenger())
	{
		if (const auto pDest = pThis->Destination)
		{
			const auto pDestCell = MapClass::Instance->GetCellAt(pDest->GetCoords());

			return pDestCell->IsClearToMove(GET_TECHNOTYPE(pPassanger)->SpeedType,
			false, false, ZoneType::None, GET_TECHNOTYPE(pPassanger)->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1;
		}
	}

	return false;

}

AircraftExtContainer AircraftExtContainer::Instance;

bool AircraftExtContainer::LoadAll(const json& root)
{
	this->Clear();

	//first layer
	if (root.contains(AircraftExtContainer::ClassName))
	{
		auto& container = root[AircraftExtContainer::ClassName];

		for (auto& entry : container[AircraftExtData::ClassName]) {

			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			AircraftExtData* buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, AircraftExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool AircraftExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[AircraftExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : AircraftExtContainer::Array) {
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[AircraftExtData::ClassName] = std::move(_extRoot);
	return true;
}

ASMJIT_PATCH(0x413DB1, AircraftClass_CTOR, 0x6)
{
	GET(AircraftClass*, pItem, ESI);
	AircraftExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);
	AircraftExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeAircraftClass::_Detach(AbstractClass* target, bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	//will detach type pointer
	this->AircraftClass::PointerExpired(target, all);
}
DEFINE_FUNCTION_JUMP(VTABLE , 0x7E22CC , FakeAircraftClass::_Detach)
