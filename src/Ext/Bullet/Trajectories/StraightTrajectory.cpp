#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <Ext/WeaponType/Body.h>

#pragma region BaseStraight

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->SnapOnTarget)
		.Process(this->SnapThreshold)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
		.Process(this->DetonationDistance_ApplyRangeModifiers)
		;

}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
	Stm
		.Process(this->SnapOnTarget)
		.Process(this->SnapThreshold)
		.Process(this->TargetSnapDistance)
		.Process(this->PassThrough)
		.Process(this->DetonationDistance_ApplyRangeModifiers)
		;

}

int StraightTrajectory::GetVelocityZ(CoordStruct& source) const
{
	auto const pBullet = this->AttachedTo;

	int velocity = pBullet->TargetCoords.Z - source.Z;

	if (!this->GetTrajectoryType()->PassThrough)
		return velocity;

	if (this->FirerZPosition == this->TargetZPosition)
		return 0;

	return velocity;
}

int StraightTrajectory::GetFirerZPosition() const
{
	CoordStruct coords = AttachedTo->SourceCoords;

	if (AttachedTo->Owner) {
		if (auto const pCell = AttachedTo->Owner->GetCell())
			coords = pCell->GetCoordsWithBridge();
	}

	return coords.Z;
}

CoordStruct StraightTrajectory::GetTargetPosition() const
{
	CoordStruct coords = AttachedTo->TargetCoords;

	if (auto pTarget = AttachedTo->Target) {
		if (auto const pCell = MapClass::Instance()->TryGetCellAt(pTarget->GetCoords()))
			coords = pCell->GetCoordsWithBridge();
	}

	return coords;
}

// Should bullet detonate based on elevation conditions.
bool StraightTrajectory::ElevationDetonationCheck() const
{
	//auto const location = &AttachedTo->Location;
	//auto const target = &AttachedTo->TargetCoords;

	// Special case - detonate if it is on same cell as target and lower or at same level as it and beneath the cell floor.
	if (AttachedTo->GetCell() == MapClass::Instance->TryGetCellAt(AttachedTo->TargetCoords)
		&& AttachedTo->Location.Z <= AttachedTo->TargetCoords.Z
		&& AttachedTo->Location.Z < MapClass::Instance->GetCellFloorHeight(AttachedTo->TargetCoords))
	{
		return true;
	}

	bool sourceObjectAboveTarget = this->FirerZPosition > this->TargetZPosition;
	bool sourceCoordAboveTarget = AttachedTo->SourceCoords.Z > AttachedTo->TargetCoords.Z;

	// If it is not coming from above then no.
	if (!sourceObjectAboveTarget || !sourceCoordAboveTarget)
		return false;

	// If it is not currently above or at target then no.
	if (AttachedTo->Location.Z >= AttachedTo->TargetCoords.Z)
		return false;

	return true;
}

bool StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->SnapOnTarget.Read(exINI, pSection, "Trajectory.Straight.SnapOnTarget");
	this->SnapThreshold.Read(exINI, pSection, "Trajectory.Straight.SnapThreshold");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->DetonationDistance_ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	return true;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->FirerZPosition)
		.Process(this->TargetZPosition)
		;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm)  &&
	Stm
		.Process(this->FirerZPosition)
		.Process(this->TargetZPosition)
		;
}

void StraightTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	this->DetonationDistance = type->DetonationDistance.Get(Leptons(102));

	if (type->DetonationDistance_ApplyRangeModifiers)
		this->DetonationDistance = Leptons(WeaponTypeExtData::GetRangeWithModifiers(pBullet->WeaponType, pBullet->Owner, this->DetonationDistance));

	this->FirerZPosition = this->GetFirerZPosition();
	this->TargetZPosition = this->GetTargetPosition().Z;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = this->GetVelocityZ(pBullet->SourceCoords) //pBullet->Owner && pBullet->Owner->IsInAir() ?  0 : this->GetVelocityZ(pBullet->SourceCoords)
		;
	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
}

bool StraightTrajectory::OnAI()
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;

	if (type->PassThrough.Get())
	{
		pBullet->Data.Distance = INT_MAX;
		int maxTravelDistance = this->DetonationDistance.value > 0 ? this->DetonationDistance.value : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= (maxTravelDistance))
			return true;
	}

	return (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < (double)this->DetonationDistance.value);

}

void StraightTrajectory::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (type->PassThrough)
		return;

	const auto pTarget = flag_cast_to<ObjectClass*>(pBullet->Target);
	CoordStruct coords = (pTarget ? pTarget->GetCoords() : pBullet->Data.Location);

	if (coords.DistanceFrom(pBullet->Location) <= type->SnapThreshold.Get(type->TargetSnapDistance.Get()))
	{
		BulletExtContainer::Instance.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}
}

void StraightTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	//pSpeed->Z += BulletTypeExtContainer::Instance.Find(this->AttachedTo->Type)->GetAdjustedGravity(); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (type->PassThrough)
	{
		if (this->FirerZPosition > this->TargetZPosition && pBullet->Location.Z <= pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}
	else if (this->ElevationDetonationCheck())
	{
		return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

#pragma endregion BaseStraight

#pragma region StraightVariantB

int StraightTrajectoryVarianB::GetVelocityZ(CoordStruct& source) const
{
	auto const pBullet = this->AttachedTo;

	int velocity = pBullet->TargetCoords.Z - source.Z;

	if (!this->GetTrajectoryType()->PassThrough)
		return velocity;

	if (pBullet->Owner && pBullet->Owner->Location.Z == pBullet->TargetCoords.Z)
		return 0;

	return velocity;
}

void StraightTrajectoryVarianB::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	this->DetonationDistance = type->DetonationDistance.Get(Leptons(102));
	this->SetInaccurate();

	if (type->PassThrough.Get())
	{
		pBullet->TargetCoords.X = INT_MAX;
		pBullet->TargetCoords.Y = INT_MAX;
		pBullet->TargetCoords.Z = INT_MAX;
	}
	else
	{
		this->SetInaccurate();

		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = this->GetVelocityZ(pBullet->SourceCoords);
	}

	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
}

bool StraightTrajectoryVarianB::OnAI()
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;

	if (type->PassThrough.Get())
	{
		pBullet->Data.Distance = INT_MAX;
		int maxTravelDistance = this->DetonationDistance.value > 0 ? this->DetonationDistance.value : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= (maxTravelDistance))
			return true;
	}

	return (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance.ToDouble());

}

void StraightTrajectoryVarianB::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (type->PassThrough)
		return;

	const auto pTarget = flag_cast_to<ObjectClass*>(pBullet->Target);
	const auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= type->SnapThreshold.Get(type->TargetSnapDistance.Get()))
	{
		BulletExtContainer::Instance.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void StraightTrajectoryVarianB::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
	pSpeed->Z += pTypeExt->GetAdjustedGravity(); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectoryVarianB::OnAITargetCoordCheck(CoordStruct& coords)
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (!type->PassThrough)
	{
		int bulletX = pBullet->Location.X / Unsorted::LeptonsPerCell;
		int bulletY = pBullet->Location.Y / Unsorted::LeptonsPerCell;
		int targetX = pBullet->TargetCoords.X / Unsorted::LeptonsPerCell;
		int targetY = pBullet->TargetCoords.Y / Unsorted::LeptonsPerCell;

		if (bulletX == targetX && bulletY == targetY && pBullet->GetHeight() < 2 * Unsorted::LevelHeight)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.

		if (pBullet->Location.Z < pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.
	}
	else
	{
		bool isAboveTarget = false;

		if ((pBullet->Owner && pBullet->Owner->Location.Z > pBullet->TargetCoords.Z) ||
			(!pBullet->Owner && pBullet->SourceCoords.Z > pBullet->TargetCoords.Z))
			isAboveTarget = true;

		if (isAboveTarget && pBullet->Location.Z <= pBullet->TargetCoords.Z)
			return TrajectoryCheckReturnType::Detonate; // Detonate projectile.*/
	}

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

#pragma endregion StraightVariantB
