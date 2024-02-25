#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(TargetSnapDistance, false)
		.Process(this->PassThrough, false)
		;

}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(TargetSnapDistance , false)
		.Process(this->PassThrough, false)
		;

}

int StraightTrajectory::GetVelocityZ() const
{
	auto const pBullet = this->AttachedTo;

	int velocity = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

	if (!this->GetTrajectoryType()->PassThrough)
		return velocity;

	if (pBullet->Owner && pBullet->Owner->Location.Z == pBullet->TargetCoords.Z)
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
	return true;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->FirerZPosition, RegisterForChange)
		.Process(this->TargetZPosition, RegisterForChange)
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

	this->FirerZPosition = this->GetFirerZPosition();
	this->TargetZPosition = this->GetTargetPosition().Z;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = this->GetVelocityZ();
	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords);
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

	return (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance.ToDouble());

}

void StraightTrajectory::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (type->PassThrough)
		return;

	const auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	const auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= type->SnapThreshold.Get(type->TargetSnapDistance.Get()))
	{
		BulletExtContainer::Instance.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void StraightTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExtContainer::Instance.Find(this->AttachedTo->Type)->GetAdjustedGravity(); // We don't want to take the gravity into account
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

