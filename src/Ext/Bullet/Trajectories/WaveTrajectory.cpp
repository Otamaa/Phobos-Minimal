#include "WaveTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <ScenarioClass.h>

bool WaveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return  PhobosTrajectoryType::Load(Stm, false) &&
	Stm
		.Process(this->MaxHeight, false)
		.Process(this->MinHeight, false)
		;

}

bool WaveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
	Stm
		.Process(this->MaxHeight)
		.Process(this->MinHeight)
		;
}


bool WaveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);

	this->MaxHeight.Read(exINI, pSection, "Trajectory.Wave.MaxHeight");
	this->MinHeight.Read(exINI, pSection, "Trajectory.Wave.MinHeight");

	return true;
}

bool WaveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, false) &&
	Stm
		.Process(this->Fallen, false)
		;
}

bool WaveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) &&
	Stm
		.Process(this->Fallen)
		;
}

void WaveTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pBullet = this->AttachedTo;
	this->SetInaccurate();

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
}

bool WaveTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();

	if (!this->Fallen)
	{
		int zDelta = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
		int maxHeight = type->MaxHeight + zDelta;
		int minHeight = type->MinHeight + zDelta;

		int currHeight = ScenarioClass::Instance->Random.RandomRanged(minHeight, maxHeight);

		if (currHeight != 0)
			pBullet->Location.Z = pBullet->SourceCoords.Z + currHeight;

		CoordStruct bullet = pBullet->Location;
		bullet.Z = 0;
		CoordStruct target = pBullet->TargetCoords;
		target.Z = 0;

		if (bullet.DistanceFrom(target) < 100)
		{
			CoordStruct end = { pBullet->TargetCoords.X, pBullet->TargetCoords.Y,pBullet->Location.Z };
			pBullet->SetLocation(end);
			this->Fallen = true;
		}
	}
	else
	{
		pBullet->SetLocation(pBullet->TargetCoords);
	}

	// If the projectile is close enough to the target then explode it
	double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		pBulletExt->LaserTrails.clear();
		pBulletExt->Trails.clear();

		return true;
	}
	return false;
}

void WaveTrajectory::OnAIPreDetonate() { }

void WaveTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType WaveTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType WaveTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}
