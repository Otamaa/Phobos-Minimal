#include "ArtilleryTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool ArtilleryTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType::Load(Stm, RegisterForChange);
	Stm
		.Process(this->MaxHeight)
		.Process(this->DistanceToHeight)
		.Process(this->DistanceToHeight_Multiplier)
		;

	return true;
}

bool ArtilleryTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->MaxHeight)
		.Process(this->DistanceToHeight)
		.Process(this->DistanceToHeight_Multiplier)
		;

	return true;
}

bool ArtilleryTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->MaxHeight.Read(exINI, pSection, "Trajectory.Artillery.MaxHeight");
	this->DistanceToHeight.Read(exINI, pSection, "Trajectory.Artillery.DistanceToHeight");
	this->DistanceToHeight_Multiplier.Read(exINI, pSection, "Trajectory.Artillery.Multiplier");

	return true;
}

bool ArtilleryTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectory::Load(Stm, RegisterForChange);

	Stm
		.Process(this->InitialTargetLocation)
		.Process(this->InitialSourceLocation)
		.Process(this->CenterLocation)
		.Process(this->Height)
		.Process(this->Init)
		;

	return true;
}

bool ArtilleryTrajectory::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->InitialTargetLocation)
		.Process(this->InitialSourceLocation)
		.Process(this->CenterLocation)
		.Process(this->Height)
		.Process(this->Init)
		;

	return true;
}

void ArtilleryTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	this->SetInaccurate();
	auto const pBullet = AttachedTo;
	this->InitialTargetLocation = pBullet->TargetCoords;
	this->InitialSourceLocation = pBullet->SourceCoords;
	this->CenterLocation = pBullet->Location;

	this->Height = this->GetTrajectoryType()->MaxHeight.Get() + this->InitialTargetLocation.Z;

	if (this->Height < pBullet->SourceCoords.Z)
		this->Height = pBullet->SourceCoords.Z;

	CoordStruct TempTargetLocation = pBullet->TargetCoords;
	TempTargetLocation.Z = 0;
	CoordStruct TempSourceLocation = pBullet->SourceCoords;
	TempSourceLocation.Z = 0;
	double distance = TempTargetLocation.DistanceFrom(TempSourceLocation);
	double fix = (((this->Height - pBullet->TargetCoords.Z) * distance) / (2 * this->Height - pBullet->SourceCoords.Z - pBullet->TargetCoords.Z)) / (this->Height - pBullet->TargetCoords.Z);

	double DirectionAngel = Math::atan2((double)(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y), (double)(pBullet->TargetCoords.X - pBullet->SourceCoords.X)) + (Math::Pi / 2);

	this->InitialTargetLocation.X += static_cast<int>((pBullet->TargetCoords.Z * fix * Math::cos(DirectionAngel)));
	this->InitialTargetLocation.Y += static_cast<int>((pBullet->TargetCoords.Z * fix * Math::sin(DirectionAngel)));
	this->InitialTargetLocation.Z = 0;

	this->InitialSourceLocation.X -= static_cast<int>((pBullet->SourceCoords.Z * fix * Math::cos(DirectionAngel)));
	this->InitialSourceLocation.Y -= static_cast<int>((pBullet->SourceCoords.Z * fix * Math::sin(DirectionAngel)));
	this->InitialSourceLocation.Z = 0;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= (this->GetTrajectorySpeed() / pBullet->Velocity.Magnitude());
}

void ArtilleryTrajectory::OnAIPreDetonate() { }

bool ArtilleryTrajectory::OnAI()
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = AttachedTo;

	if (type->DistanceToHeight)
	{
		CoordStruct TempTargetLocation = pBullet->TargetCoords;
		TempTargetLocation.Z = 0;
		CoordStruct TempSourceLocation = pBullet->SourceCoords;
		TempSourceLocation.Z = 0;
		double distance = TempTargetLocation.DistanceFrom(TempSourceLocation);
		double height = (type->DistanceToHeight_Multiplier * distance) + pBullet->TargetCoords.Z;

		if (type->MaxHeight > 0 && height > (type->MaxHeight + pBullet->TargetCoords.Z))
			height = this->GetTrajectoryType()->MaxHeight + pBullet->TargetCoords.Z;

		if (height > pBullet->SourceCoords.Z)
			this->Height = height;
	}

	CoordStruct bulletCoords = pBullet->Location;
	bulletCoords.Z = 0;
	CoordStruct initialTargetLocation = this->InitialTargetLocation;
	CoordStruct initialSourceLocation = this->InitialSourceLocation;

	double fullInitialDistance = initialSourceLocation.DistanceFrom(initialTargetLocation);
	double halfInitialDistance = fullInitialDistance / 2;
	double currentBulletDistance = initialSourceLocation.DistanceFrom(bulletCoords);

	// Trajectory angle
	const int sinDecimalTrajectoryAngle = 90;
	const double sinRadTrajectoryAngle = Math::sin(Math::deg2rad(sinDecimalTrajectoryAngle));

	// Angle of the projectile in the current location
	const double angle = (currentBulletDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
	const double sinAngle = Math::sin(Math::deg2rad(angle));

	// Height of the flying projectile in the current location
	const double currHeight = (sinAngle * this->Height) / sinRadTrajectoryAngle;

	CoordStruct source = pBullet->SourceCoords;
	source.Z = 0;
	double SourceDistance = this->InitialSourceLocation.DistanceFrom(source);
	double fixangle = (SourceDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
	double fixsinAngle = Math::sin(Math::deg2rad(fixangle));
	double heightfix = pBullet->SourceCoords.Z - (fixsinAngle * this->Height) / sinRadTrajectoryAngle;

	pBullet->Location.Z = pBullet->TargetCoords.Z + static_cast<int>(currHeight + heightfix);

	if (!this->Init)
	{
		const auto pExt = BulletExt::ExtMap.Find(pBullet);

		pExt->LaserTrails.clear();
#ifdef COMPILE_PORTED_DP_FEATURES
		pExt->Trails.clear();
#endif
		pBullet->Limbo();
		pBullet->Unlimbo(pBullet->SourceCoords, static_cast<DirType>(0));

		pExt->InitializeLaserTrails();

#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(pBullet);
#endif

		this->Init = true;
	}
	else
	{
		this->CenterLocation.X += static_cast<int>(pBullet->Velocity.X);
		this->CenterLocation.Y += static_cast<int>(pBullet->Velocity.Y);
		this->CenterLocation.Z += static_cast<int>(pBullet->Velocity.Z);
	}

	if (this->CenterLocation.DistanceFrom(pBullet->TargetCoords) < 100)
		pBullet->Location = this->CenterLocation;

	// If the projectile is close enough to the target then explode it
	const double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		if (pBulletExt->LaserTrails.size())
			pBulletExt->LaserTrails.clear();
#ifdef COMPILE_PORTED_DP_FEATURES
		if (pBulletExt->Trails.size())
			pBulletExt->Trails.clear();
#endif
		return true;
	}

	return false;
}

void ArtilleryTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = AttachedTo;
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}