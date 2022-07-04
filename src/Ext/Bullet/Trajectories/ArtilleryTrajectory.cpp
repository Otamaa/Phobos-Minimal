#include "ArtilleryTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool ArtilleryTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->LoadBase(Stm , RegisterForChange);
	Stm.Process(this->MaxHeight, false);
	return true;
}

bool ArtilleryTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->SaveBase(Stm);
	Stm.Process(this->MaxHeight, false);
	return true;
}

void ArtilleryTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::ReadBase(pINI, pSection))
		return;

	INI_EX exINI(pINI);

	this->MaxHeight.Read(exINI,pSection, "Trajectory.Artillery.MaxHeight");
}

bool ArtilleryTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->LoadBase(Stm, RegisterForChange);

	Stm
		.Process(this->MaxHeight, false) // Creo que esto no hace falta aquí porque no se actualiza....
		;

	return true;
}

bool ArtilleryTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->SaveBase(Stm);

	Stm
		.Process(this->MaxHeight, false) // Creo que esto no hace falta aquí porque no se actualiza....
		;

	return true;
}

void ArtilleryTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	this->InitialTargetLocation = pBullet->TargetCoords;

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::GetExtData(pBullet->Type);

		int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? (int)(pTypeExt->BallisticScatter_Max.Get() * 256.0) : ballisticScatter;
		int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? (int)(pTypeExt->BallisticScatter_Min.Get() * 256.0) : (scatterMax / 2);

		double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		const CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		this->InitialTargetLocation += offset;
	}

	this->InitialSourceLocation = pBullet->SourceCoords;

	CoordStruct initialSourceLocation = this->InitialSourceLocation; // Obsolete
	initialSourceLocation.Z = 0;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= (this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude());
}

void ArtilleryTrajectory::OnAIPreDetonate(BulletClass* pBullet) { }

bool ArtilleryTrajectory::OnAI(BulletClass* pBullet)
{
	int zDelta = this->InitialTargetLocation.Z - this->InitialSourceLocation.Z;
	double maxHeight = this->GetTrajectoryType()->MaxHeight + (double)zDelta;

	CoordStruct bulletCoords = pBullet->Location;
	bulletCoords.Z = 0;
	CoordStruct initialTargetLocation = this->InitialTargetLocation;
	initialTargetLocation.Z = 0;
	CoordStruct initialSourceLocation = this->InitialSourceLocation;
	initialSourceLocation.Z = 0;

	double fullInitialDistance = initialSourceLocation.DistanceFrom(initialTargetLocation);
	double halfInitialDistance = fullInitialDistance / 2;
	double currentBulletDistance = initialSourceLocation.DistanceFrom(bulletCoords);

	// Trajectory angle
	int sinDecimalTrajectoryAngle = 90;
	double sinRadTrajectoryAngle = Math::sin(Math::deg2rad(sinDecimalTrajectoryAngle));

	// Angle of the projectile in the current location
	double angle = (currentBulletDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
	double sinAngle = Math::sin(Math::deg2rad(angle));

	// Height of the flying projectile in the current location
	double currHeight = (sinAngle * maxHeight) / sinRadTrajectoryAngle;

	if (currHeight != 0)
		pBullet->Location.Z = this->InitialSourceLocation.Z + (int)currHeight;

	// If the projectile is close enough to the target then explode it
	double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExt::GetExtData(pBullet);

		if (pBulletExt->LaserTrails.size())
			pBulletExt->LaserTrails.clear();

		if (pBulletExt->Trails.size())
			pBulletExt->Trails.clear();

		return true;
	}

	return false;
}

void ArtilleryTrajectory::OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}