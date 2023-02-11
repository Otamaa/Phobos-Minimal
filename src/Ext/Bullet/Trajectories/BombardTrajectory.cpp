#include "BombardTrajectory.h"

#include <Ext/BulletType/Body.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType::Load(Stm, RegisterForChange);
	Stm
		.Process(this->Height, false)
		.Process(this->Anti, false)
		;
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->Height)
		.Process(this->Anti)
		;
	return true;
}

bool BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };
	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->Anti.Read(exINI, pSection, "Trajectory.Bombard.Anti");
	
	return true;
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectory::Load(Stm, RegisterForChange);

	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;

	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectory::Save(Stm);
	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;

	return true;
}

void BombardTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	this->Height = type->Height + pBullet->TargetCoords.Z;
	this->SetInaccurate();

	if (!type->Anti.Get())
	{
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Magnitude();
	}
	else
	{
		pBullet->Velocity.X = 0.0;
		pBullet->Velocity.Y = 0.0;
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Magnitude();
	}
}

bool BombardTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;

	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) <  this->DetonationDistance) // This value maybe adjusted?
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate() { }

void BombardTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;

	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
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

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}