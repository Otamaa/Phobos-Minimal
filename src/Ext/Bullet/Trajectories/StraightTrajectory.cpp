#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->SDetonationDistance, false)
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
		.Process(this->SDetonationDistance,false)
		.Process(TargetSnapDistance , false)
		.Process(this->PassThrough, false)
		;

}

int StraightTrajectory::GetVelocityZ()
{
	auto const pBullet = this->AttachedTo;

	int velocity = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

	if (!this->GetTrajectoryType()->PassThrough)
		return velocity;

	if (pBullet->Owner && pBullet->Owner->Location.Z == pBullet->TargetCoords.Z)
		return 0;

	return velocity;
}

bool StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->SnapOnTarget.Read(exINI, pSection, "Trajectory.Straight.SnapOnTarget");
	this->SnapThreshold.Read(exINI, pSection, "Trajectory.Straight.SnapThreshold");
	this->SDetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	return true;
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange);
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm);
}

void StraightTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	this->DetonationDistance = type->DetonationDistance.Get(type->SDetonationDistance.Get());
	
	this->SetInaccurate();

	if (type->PassThrough.Get())
	{
		pBullet->TargetCoords.X = INT_MAX;
		pBullet->TargetCoords.Y = INT_MAX;
		pBullet->TargetCoords.Z = INT_MAX;
	}
	else
	{
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = this->GetVelocityZ();
	}

	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Magnitude();
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
		BulletExt::ExtMap.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void StraightTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
	pSpeed->Z += pTypeExt->GetAdjustedGravity(); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
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

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

