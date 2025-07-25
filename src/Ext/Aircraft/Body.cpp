#include "Body.h"
#include <Ext/WeaponType/Body.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <AircraftClass.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>

#include <Utilities/Macro.h>

#include <lib/gcem/gcem.hpp>

#include <Locomotor/FlyLocomotionClass.h>

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
		pBullet->Velocity = { 0, 0, pBullet->Velocity.Z };
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
	AircraftExt::TriggerCrashWeapon(this, mult);
}

WeaponStruct* FakeAircraftClass::_GetWeapon(int weaponIndex)
{
	auto const pExt = TechnoExtContainer::Instance.Find(this);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		return this->TechnoClass::GetWeapon(pExt->CurrentAircraftWeaponIndex);
	else
		return this->TechnoClass::GetWeapon(this->SelectWeapon(this->Target));
}

// Spy plane, airstrike etc.
bool AircraftExt::PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell)
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

void AircraftExt::TriggerCrashWeapon(AircraftClass* pThis, int nMult)
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

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	if (!pTarget)
		return;

	AircraftExt::FireBurst(pThis, pTarget, shotNumber, pThis->SelectWeapon(pTarget));
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	const auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	const auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	AircraftExt::FireBurst(pThis , pTarget, shotNumber, WeaponIdx, weaponType);
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
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

#include <Ext/TerrainType/Body.h>

bool AircraftExt::IsValidLandingZone(AircraftClass* pThis)
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

#ifdef ENABLE_NEWHOOKS
ASMJIT_PATCH(0x413F6A, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);

	AircraftExt::ExtMap.JustAllocate(pItem, !pItem, "Invalid !");

	return 0;
}

ASMJIT_PATCH(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);

	AircraftExt::ExtMap.Remove(pItem);

	return 0;
}

ASMJIT_PATCH_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
ASMJIT_PATCH(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AircraftExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

ASMJIT_PATCH(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
{
	AircraftExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
{
	AircraftExt::ExtMap.SaveStatic();

	return 0;
}

ASMJIT_PATCH(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	if (const auto pExt = AircraftExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return 0x0;
}
#endif

//ASMJIT_PATCH(0x418478, AircraftClass_Mi_Attack_Untarget1, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x4184C2
//		;
//}
//
//ASMJIT_PATCH(0x4186D7, AircraftClass_Mi_Attack_Untarget2, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418720
//		;
//}
//
//ASMJIT_PATCH(0x418826, AircraftClass_Mi_Attack_Untarget3, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418883
//		;
//}
//
//ASMJIT_PATCH(0x418935, AircraftClass_Mi_Attack_Untarget4, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418992
//		;
//}
//
//ASMJIT_PATCH(0x418A44, AircraftClass_Mi_Attack_Untarget5, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418AA1
//		;
//}
//
//ASMJIT_PATCH(0x418B40, AircraftClass_Mi_Attack_Untarget6, 6)
//{
//	GET(AircraftClass*, A, ESI);
//	return A->Target
//		? 0
//		: 0x418B8A
//		;
//}
