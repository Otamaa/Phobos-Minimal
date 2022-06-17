#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>

bool StraightTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->LoadBase(Stm, RegisterForChange);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}

bool StraightTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->SaveBase(Stm);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}


void StraightTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::ReadBase(pINI, pSection))
		return;

	INI_EX exINI(pINI);

	this->SnapOnTarget.Read(exINI, pSection, "Trajectory.Straight.SnapOnTarget");
	this->SnapThreshold.Read(exINI, pSection, "Trajectory.Straight.SnapThreshold");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
}

bool StraightTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->LoadBase(Stm, RegisterForChange);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}

bool StraightTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->SaveBase(Stm);
	Stm
		.Process(this->SnapOnTarget, false)
		.Process(this->SnapThreshold, false)
		.Process(this->PassThrough, false)
		;

	return true;
}

void StraightTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto type = this->GetTrajectoryType();

	this->DetonationDistance = type->DetonationDistance.Get(Leptons());
	this->SnapOnTarget = type->SnapOnTarget.Get();
	this->SnapThreshold = type->SnapThreshold.Get();
	this->PassThrough = type->PassThrough.Get();

	if (this->PassThrough) {
		pBullet->TargetCoords.X = INT_MAX;
		pBullet->TargetCoords.Y = INT_MAX;
		pBullet->TargetCoords.Z = INT_MAX;
	} else {
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	}
}

bool StraightTrajectory::OnAI(BulletClass* pBullet)
{
	if (this->PassThrough)
	{
		pBullet->Data.Distance = INT_MAX;
		int maxTravelDistance = this->DetonationDistance > 0 ? this->DetonationDistance : INT_MAX;

		if (pBullet->SourceCoords.DistanceFrom(pBullet->Location) >= (maxTravelDistance))
			return true;
	}
	else if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < (this->DetonationDistance.value))
	{
		return true;
	}

	return false;
}

void StraightTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (this->SnapOnTarget && !this->PassThrough)
	{
		auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
		auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

		if (pCoords.DistanceFrom(pBullet->Location) <= this->SnapThreshold)
		{
			BulletExt::GetExtData(pBullet)->SnappedToTarget = true;
			pBullet->SetLocation(pCoords);
		}
	}
}

void StraightTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletExt::GetExtData(pBullet)->TypeExt->GetAdjustedGravity(); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectory::OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct coords)
{
	int bulletX = pBullet->Location.X / Unsorted::LeptonsPerCell;
	int bulletY = pBullet->Location.Y / Unsorted::LeptonsPerCell;
	int targetX = pBullet->TargetCoords.X / Unsorted::LeptonsPerCell;
	int targetY = pBullet->TargetCoords.Y / Unsorted::LeptonsPerCell;

	if (bulletX == targetX && bulletY == targetY && pBullet->GetHeight() < 2 * Unsorted::LevelHeight)
		return TrajectoryCheckReturnType::Detonate; // Detonate projectile.

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

TrajectoryCheckReturnType StraightTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

