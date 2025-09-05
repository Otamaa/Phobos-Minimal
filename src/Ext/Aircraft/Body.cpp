#include "Body.h"
#include <Ext/WeaponType/Body.h>

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

#include <lib/gcem/gcem.hpp>

#include <Locomotor/FlyLocomotionClass.h>
#include "Body.h"

//todo Add the hooks
int FakeAircraftClass::_Mission_Attack()
{
	auto return_to_base = [this]() -> void
		{
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
		};

	auto get_default_mission_control_return = [this]() -> int
		{
			return int(this->GetCurrentMissionControl()->Rate * (double)TICKS_PER_MINUTE) +
				ScenarioClass::Instance->Random.RandomRanged(0, 2);
		};

	switch ((AirAttackStatus)this->MissionStatus)
	{
	case AirAttackStatus::ValidateAZ:
	{
		auto TarCom = this->Target;
		this->IsLocked = 0;
		this->MissionStatus = int(TarCom != 0
			? AirAttackStatus::PickAttackLocation : AirAttackStatus::ReturnToBase);

		return 1;
	}

	case AirAttackStatus::PickAttackLocation:
	{
		bool lose_ammo = this->loseammo_6c8;
		this->IsLocked = 0;

		if (lose_ammo) {
			auto Ammo = this->Ammo;
			this->loseammo_6c8 = 0;
			this->Ammo = Ammo - 1;
		}

		auto v7 = this->Target;

		if (v7 && this->Ammo)
		{
			this->SetDestination(this->GoodTargetLoc_(v7),true);

			this->MissionStatus = int(this->Destination != 0
				? AirAttackStatus::FlyToPosition : AirAttackStatus::ReturnToBase);
		} else {
			return_to_base();
		}

		return get_default_mission_control_return();
	}

	case AirAttackStatus::FlyToPosition:
	{
		if (this->loseammo_6c8) {
			auto v10 = this->Ammo;
			this->loseammo_6c8 = 0;
			this->Ammo = v10 - 1;
		}

		this->IsLocked = 0;

		if (!this->Target || !this->Ammo) {
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}

		if (this->Is_Strafe()) {
			auto WeaponType = this->GetWeapon(0)->WeaponType;

			if (this->DistanceFrom(this->Target) < WeaponType->Range) {
				this->MissionStatus = (int)AirAttackStatus::FireAtTarget;
				return 1;
			}

			this->SetDestination(this->Target, 1);
		}
		else
		{
			if (this->Is_Locked()) {
				this->MissionStatus = (int)AirAttackStatus::FireAtTarget;
				return 1;
			}

			if (!this->Locomotor.GetInterfacePtr()->Is_Moving_Now()) {
				this->MissionStatus = (int)AirAttackStatus::FireAtTarget;
				return 1;
			}
		}

		if (this->Destination)
		{
			auto v13 = this->DistanceFrom(this->Destination);

			if (v13 >= 512)
			{
				auto v16 = this->Destination->GetCenterCoords();
				CoordStruct v17;
				this->GetFLH(&v17, 0, CoordStruct::Empty);

				DirStruct fac {};

				if (v17.X == v16.X && v17.Y == v16.Y) {
					fac.Raw = 0;
				}

				auto v18 = Math::atan2(double(v17.Y - v16.Y), double(v16.X - v17.X));
				auto v19 = Math::DEG90_AS_RAD;
				auto v20 = v18 - v19;
				auto v21 = Math::BINARY_ANGLE_MAGIC;
				fac.Raw = (v20 * v21);
				this->SecondaryFacing.Set_Desired(fac);
				return 1;
			}
			else
			{
				this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));

				if (v13 >= 16)
				{
					return 1;
				}
				else
				{
					this->MissionStatus = (int)AirAttackStatus::FireAtTarget;
					this->SetDestination(0, 1);
					return 1;
				}
			}
		}
		else {
			this->MissionStatus = (int)AirAttackStatus::PickAttackLocation;
		}

		return 1;
	}

	case AirAttackStatus::FireAtTarget:
	{
		if (!this->Target || !this->Ammo)
		{
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}

		if (!this->Is_Strafe())
		{
			this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
		}

		switch (this->GetFireError(this->Target, this->SelectWeapon(this->Target), true))
		{
		case FireError::OK:
		{
			this->loseammo_6c8 = 1;

			int v28 = 0;

			if (this->GetWeapon(this->SelectWeapon(this->Target))->WeaponType->Burst > 0)
			{
				do
				{
					this->Fire(this->Target, this->SelectWeapon(this->Target));
					++v28;
				}
				while (v28 < this->GetWeapon(this->SelectWeapon(this->Target))->WeaponType->Burst);
			}

			MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);

			if (this->Is_Strafe())
			{
				this->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
				this->IsLocked = 1;
				return this->GetWeapon(0)->WeaponType->ROF;
			}

			if (!this->Is_Locked())
			{
				this->MissionStatus = (int)AirAttackStatus::FireAtTarget2;
			}
			else
			{
				auto v37 = this->Ammo;
				auto v38 = v37 == 0;
				auto v39 = v37 < 0;
				this->IsLocked = 1;
				this->MissionStatus = !v39 && !v38 ? (int)AirAttackStatus::PickAttackLocation : (int)AirAttackStatus::ReturnToBase;
				return this->GetWeapon(0)->WeaponType->ROF;
			}
			break;
		}
		case FireError::FACING:

		{
			if (!this->Ammo)
			{
				this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
				return 1;
			}

			if (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe())
			{
				this->MissionStatus = int(AirAttackStatus::PickAttackLocation);
			}
			else if (this->Is_Locked())
			{
				this->MissionStatus = int(AirAttackStatus::FireAtTarget);
			}
			else
			{
				this->MissionStatus = int(RulesClass::Instance->CurleyShuffle ?
					AirAttackStatus::PickAttackLocation : AirAttackStatus::FireAtTarget);
			}

			return this->Is_Strafe() ? 1 : 45;
		}
		case FireError::REARM:
			return 1;
		case FireError::CLOAKED:
			this->Uncloak(false);
			return 1;
		default:
		{
			if (!this->Ammo)
			{
				this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			}
			else if (this->Is_Strafe())
			{
				return 1;
			}
			else
			{
				this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			}
		}
		}
		return 1;
	}

	case AirAttackStatus::FireAtTarget2:
	{
		if (this->Target)
		{
			this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));

			switch (this->GetFireError(this->Target, this->SelectWeapon(this->Target), true))
			{
			case FireError::OK:
			{
				this->Fire(this->Target, this->SelectWeapon(this->Target));
				MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);

				if (!this->Ammo) {
					return_to_base();
				} else {
					this->MissionStatus = int(RulesClass::Instance->CurleyShuffle ? AirAttackStatus::PickAttackLocation : AirAttackStatus::FireAtTarget);
				}
				return get_default_mission_control_return();
			}
			case FireError::FACING:
			{
				if (!this->Ammo)
				{
					return_to_base();
				}
				else
				{
					if (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe()) {
						this->MissionStatus = (int)AirAttackStatus::PickAttackLocation;
					} else {
						this->MissionStatus = (int)(RulesClass::Instance->CurleyShuffle ? AirAttackStatus::PickAttackLocation : AirAttackStatus::FireAtTarget);
					}

					if (this->Is_Strafe()) {
						return 45;
					}
				}
				return get_default_mission_control_return();
			}
			case FireError::REARM:
			{
				return get_default_mission_control_return();
			}
			case FireError::CLOAKED:
			{
				this->Uncloak(0);
				return get_default_mission_control_return();
			}
			default: {
				if (!this->Ammo) {
					return_to_base();
				} else {
					this->MissionStatus = int(this->IsCloseEnoughToAttack(this->Target) ?
						RulesClass::Instance->CurleyShuffle
						? AirAttackStatus::PickAttackLocation : AirAttackStatus::FireAtTarget :
						AirAttackStatus::PickAttackLocation);
				}
				return get_default_mission_control_return();
			}
			}
		} else {
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget2_Strafe:
	{
		if (this->Target)
		{
			switch (this->GetFireError(this->Target, this->SelectWeapon(this->Target), true))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
				this->SetDestination(this->Target, 1);
				break;
			default:
				if (!this->Ammo)
				{
					this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
					this->IsLocked = 0;
				}
				return 1;
			}
			this->Fire(this->Target, this->SelectWeapon(this->Target));
			MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);
			this->SetDestination(this->Target, 1);
			this->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;
			return this->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget3_Strafe:
	{
		if (this->Target)
		{
			switch (this->GetFireError(this->Target, this->SelectWeapon(this->Target), true))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
				this->SetDestination(this->Target, 1);
				break;
			default:
				if (!this->Ammo)
				{
					this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
					this->IsLocked = 0;
				}
				return 1;
			}

			this->Fire(this->Target, this->SelectWeapon(this->Target));
			MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);
			this->SetDestination(this->Target, 1);
			this->MissionStatus = (int)AirAttackStatus::FireAtTarget4_Strafe;
			return this->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget4_Strafe:
	{
		if (this->Target)
		{
			switch (this->GetFireError(this->Target, this->SelectWeapon(this->Target), true))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
				this->SetDestination(this->Target, 1);
				break;
			default:
				if (!this->Ammo) {
					this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
					this->IsLocked = 0;
				}
				return 1;
			}
			this->Fire(this->Target, this->SelectWeapon(this->Target));
			MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);
			this->SetDestination(this->Target, 1);
			this->MissionStatus = (int)AirAttackStatus::FireAtTarget5_Strafe;
			return this->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget5_Strafe:
	{
		if (this->Target)
		{
			switch (this->GetFireError(this->Target,this->SelectWeapon(this->Target),true))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::RANGE:
			case FireError::CLOAKED:
				this->Fire(this->Target, this->SelectWeapon(this->Target));
				MapClass::Instance->GetCellAt(this->Target->GetCoords())->ScatterContent(this->Location, true, false, false);
				this->MissionStatus = (int)AirAttackStatus::FlyToPosition;
				return this->GetWeapon(0)->WeaponType->Range + 1024 / this->Type->Speed;
			default:
				if (!this->Ammo) {
					this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
					this->IsLocked = 0;
				}
				break;
			}
		}
		else {
			this->MissionStatus = (int)AirAttackStatus::ReturnToBase;
		}
		return 1;
	}

	case AirAttackStatus::ReturnToBase:
	{
		auto v84 = this->loseammo_6c8;
		this->IsLocked = 0;
		if (v84) {
			auto v85 = this->Ammo;
			this->loseammo_6c8 = 0;
			if (v85 > 0) {
				this->Ammo = v85 - 1;
			}
		}

		if (this->Ammo) {
			if (this->Target) {
				this->MissionStatus= (int)AirAttackStatus::PickAttackLocation;
				return 1;
			}
		} else if (this->Spawned || this->Owner->IsControlledByHuman()) {
			this->SetTarget(0);
		}

		this->IsLocked = 0;
		CellStruct edgeCell = MapClass::Instance->PickCellOnEdge(this->Owner->GetCurrentEdge()
			, CellStruct::Empty
			, CellStruct::Empty
			, SpeedType::Winged
			,true
			,MovementZone::Normal
		);

		this->SetDestination(MapClass::Instance->GetCellAt(edgeCell), 1);
		this->NumParadropsLeft = 0;
		(this->Airstrike && this->Ammo > 0 ? this->QueueMission(Mission::Retreat, false) : this->EnterIdleMode(false,1));
		this->NumParadropsLeft = 1;
		return 1;
	}

	default:
		return get_default_mission_control_return();
	}
}

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
}

COMPILETIMEEVAL FORCEDINLINE bool IsFlyLoco(const ILocomotion* pLoco) {
	return (((DWORD*)pLoco)[0] == FlyLocomotionClass::ILoco_vtable);
}

COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon) {
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
}

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
		const int facingOffset = dir.Raw - 0x3FFF;
		const double yawRad = facingOffset * -0.00009587672516830327;
		const double mag = velocity->Length();

		if (yawRad != 0.0)
		{
			velocity->X /= Math::cos(yawRad);
			velocity->Y /= Math::cos(yawRad);
		}

		const double pitchRad = -0.00009587672516830327;
		velocity->X *= Math::cos(pitchRad);
		velocity->Y *= Math::cos(pitchRad);
		velocity->Z = Math::sin(pitchRad) * mag;

		const DirStruct newFacingDir = pThis->SecondaryFacing.Current();

		velocity->SetIfZeroXY();

		const double dist2D = velocity->LengthXY();
		const int newFacing = newFacingDir.Raw - 0x3FFF;
		const double newRad = newFacing * -0.00009587672516830327;

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
			int adjustedYaw = yawBinaryAngle - 0x3FFF;
			double adjustedYawRad = adjustedYaw * -0.00009587672516830327;

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
			int adjustedPitch = pitchBinaryAngle - 0x3FFF;
			double adjustedPitchRad = adjustedPitch * -0.00009587672516830327;

			// Re-calculate current yaw from bullet velocity
			DirStruct currentFacing = velocity->GetDirectionFromXY();
			int currentFacingOffset = currentFacing.Raw - 0x3FFF;
			double currentYawRad = currentFacingOffset * -0.00009587672516830327;

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
			TechnoExtContainer::Instance.Find(this)->ShootCount++;

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
	TechnoExtContainer::Instance.Find(this)->CurrentAircraftWeaponIndex = -1;
}

void FakeAircraftClass::_Destroyed(int mult)
{
	AircraftExtData::TriggerCrashWeapon(this, mult);
}

HRESULT __stdcall FakeAircraftClass::_Load(IStream* pStm)
{
	auto hr = this->AircraftClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		hr = AircraftExtContainer::Instance.ReadDataFromTheByteStream(this,
			AircraftExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeAircraftClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->AircraftClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = AircraftExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E22B8, FakeAircraftClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E22BC, FakeAircraftClass::_Save)

WeaponStruct* FakeAircraftClass::_GetWeapon(int weaponIndex)
{
	auto const pExt = TechnoExtContainer::Instance.Find(this);

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
	const auto pType = pThis->GetTechnoType();
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

			return pDestCell->IsClearToMove(pPassanger->GetTechnoType()->SpeedType, false, false, ZoneType::None, pPassanger->GetTechnoType()->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1;
		}
	}

	return false;

}

std::vector<AircraftExtData*> Container<AircraftExtData>::Array;
AircraftExtContainer AircraftExtContainer::Instance;

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

ASMJIT_PATCH(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AbstractClass*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	AircraftExtContainer::Instance.InvalidatePointerFor(pThis, target, all);

	return 0x0;
}
