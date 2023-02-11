#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType::Load(Stm, RegisterForChange);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}


bool StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->SnapOnTarget.Read(exINI, pSection, "Trajectory.Straight.SnapOnTarget");
	this->SnapThreshold.Read(exINI, pSection, "Trajectory.Straight.SnapThreshold");
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
	this->DetonationDistance = type->DetonationDistance.Get(Leptons());
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
		pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
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
		int maxTravelDistance = this->DetonationDistance > 0 ? this->DetonationDistance : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= (maxTravelDistance))
			return true;
	}

	return (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < (this->DetonationDistance.value));
}

void StraightTrajectory::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (type->SnapOnTarget.Get() && !type->PassThrough.Get())
	{
		auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
		auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

		if (pCoords.DistanceFrom(pBullet->Location) <= type->SnapThreshold.Get())
		{
			BulletExt::ExtMap.Find(pBullet)->SnappedToTarget = true;
			pBullet->SetLocation(pCoords);
		}
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
	const int bulletX = pBullet->Location.X / Unsorted::LeptonsPerCell;
	const int bulletY = pBullet->Location.Y / Unsorted::LeptonsPerCell;
	const int targetX = pBullet->TargetCoords.X / Unsorted::LeptonsPerCell;
	const int targetY = pBullet->TargetCoords.Y / Unsorted::LeptonsPerCell;

	if (bulletX == targetX && bulletY == targetY && pBullet->GetHeight() < 2 * Unsorted::LevelHeight)
		return TrajectoryCheckReturnType::Detonate; // Detonate projectile.

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

