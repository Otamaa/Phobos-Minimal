#include "BombardTrajectory.h"

#include <Ext/BulletType/Body.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm.Process(this->Height, false);
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm.Process(this->Height);
	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	this->Height = pINI->ReadDouble(pSection, "Trajectory.Bombard.Height", 0.0);
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;

	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;

	return true;
}

void BombardTrajectory::OnUnlimbo(CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	auto pBullet = BulletExtData->OwnerObject();
	this->Height = this->GetTrajectoryType<BombardTrajectoryType>()->Height + pBullet->TargetCoords.Z;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool BombardTrajectory::OnAI()
{
	auto pBullet = BulletExtData->OwnerObject();
	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < 100) // This value maybe adjusted?
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate()
{
}

void BombardTrajectory::OnAIVelocity(BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	if (!this->IsFalling)
	{
		auto pBullet = BulletExtData->OwnerObject();
		pSpeed->Z += BulletExtData->TypeExt->GetAdjustedGravity();
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			this->IsFalling = true;
			pSpeed->X = 0.0;
			pSpeed->Y = 0.0;
			pSpeed->Z = 0.0;
			pPosition->X = pBullet->TargetCoords.X;
			pPosition->Y = pBullet->TargetCoords.Y;
		}
	}

}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(CoordStruct coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}