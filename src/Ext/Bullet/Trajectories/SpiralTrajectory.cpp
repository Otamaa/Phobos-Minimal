#include "SpiralTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <ScenarioClass.h>

bool SpiralTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return  this->PhobosTrajectoryType::Load(Stm, false) &&
	Stm
		.Process(this->MaxRadius, false)
		.Process(this->Length, false)
		.Process(this->Angel, false)
		;
}

bool SpiralTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectoryType::Save(Stm) &&
	Stm
		.Process(this->MaxRadius)
		.Process(this->Length)
		.Process(this->Angel)
		;
}


bool SpiralTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->MaxRadius.Read(exINI, pSection, "Trajectory.Spiral.Radius");
	this->Length.Read(exINI, pSection, "Trajectory.Spiral.Length");
	this->Angel.Read(exINI, pSection, "Trajectory.Spiral.Angel");

	return true;
}

bool SpiralTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return  this->PhobosTrajectory::Load(Stm, false) &&
	Stm
		.Process(this->CenterLocation, false)
		.Process(this->DirectionAngel, false)
		.Process(this->CurrentRadius, false)
		.Process(this->CurrentAngel, false)
		.Process(this->close, false)
		;
}

bool SpiralTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectory::Save(Stm) &&
	Stm
		.Process(this->CenterLocation)
		.Process(this->DirectionAngel)
		.Process(this->CurrentRadius)
		.Process(this->CurrentAngel)
		.Process(this->close)
		;
}

void SpiralTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pBullet = this->AttachedTo;
	this->SetInaccurate();
	this->CenterLocation = pBullet->Location;

	COMPILETIMEEVAL auto halfpi = (Math::GAME_PI / 2);

	this->DirectionAngel = std::atan2((double)(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y),
		(double)(pBullet->TargetCoords.X - pBullet->SourceCoords.X)) + halfpi;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
}

bool SpiralTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;

	if (!this->close)
	{
		double height = std::sin(Math::deg2rad(this->CurrentAngel)) * this->CurrentRadius;
		double width = std::cos(Math::deg2rad(this->CurrentAngel)) * this->CurrentRadius;

		pBullet->Location.X = static_cast<int>((width * std::cos(this->DirectionAngel)) + this->CenterLocation.X);
		pBullet->Location.Y = static_cast<int>((width * std::sin(this->DirectionAngel)) + this->CenterLocation.Y);
		pBullet->Location.Z = static_cast<int>(height + this->CenterLocation.Z);

		this->CurrentAngel += this->GetTrajectoryType()->Angel;
	}

	this->CenterLocation.X += static_cast<int>(pBullet->Velocity.X);
	this->CenterLocation.Y += static_cast<int>(pBullet->Velocity.Y);
	this->CenterLocation.Z += static_cast<int>(pBullet->Velocity.Z);

	// If the projectile is close enough to the target then explode it
	double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

		pBulletExt->LaserTrails.clear();

		pBulletExt->Trails.clear();

		return true;
	}

	return false;
}

void SpiralTrajectory::OnAIPreDetonate() { }

void SpiralTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	auto const type = this->GetTrajectoryType();
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account

	CoordStruct center = this->CenterLocation;
	center.Z = 0;

	CoordStruct target = pBullet->TargetCoords;
	target.Z = 0;

	if (center.DistanceFrom(target) > type->Length)
	{
		if (this->CurrentRadius < type->MaxRadius)
		{
			double speed = std::sqrt(pow(pSpeed->X, 2) + pow(pSpeed->Y, 2));
			this->CurrentRadius += type->MaxRadius / (type->Length / speed);
		}
	}
	else
	{
		if (this->CurrentRadius > 0)
		{
			double speed = std::sqrt(pow(pSpeed->X, 2) + pow(pSpeed->Y, 2));
			this->CurrentRadius -= type->MaxRadius / (type->Length / speed);
		}
		else if (!this->close)
		{
			this->close = true;
			pBullet->Location.X = this->CenterLocation.X;
			pBullet->Location.Y = this->CenterLocation.Y;
			pBullet->Location.Z = this->CenterLocation.Z;
		}
	}
}

TrajectoryCheckReturnType SpiralTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType SpiralTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}