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
	INI_EX exINI { pINI };

	if (!this->PhobosTrajectoryType::ReadBase(exINI, pSection))
		return;

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

		const int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		const int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? static_cast<int>(pTypeExt->BallisticScatter_Max.Get() * 256.0) : ballisticScatter;
		const int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? static_cast<int>(pTypeExt->BallisticScatter_Min.Get() * 256.0) : (scatterMax / 2);

		const double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		const double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

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
	const int zDelta = this->InitialTargetLocation.Z - this->InitialSourceLocation.Z;
	const double maxHeight = this->GetTrajectoryType()->MaxHeight + static_cast<double>(zDelta);

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
	const int sinDecimalTrajectoryAngle = 90;
	const double sinRadTrajectoryAngle = Math::sin(Math::deg2rad(sinDecimalTrajectoryAngle));

	// Angle of the projectile in the current location
	const double angle = (currentBulletDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
	const double sinAngle = Math::sin(Math::deg2rad(angle));

	// Height of the flying projectile in the current location
	const double currHeight = (sinAngle * maxHeight) / sinRadTrajectoryAngle;

	if (currHeight != 0)
		pBullet->Location.Z = this->InitialSourceLocation.Z + static_cast<int>(currHeight);

	// If the projectile is close enough to the target then explode it
	const double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExt::GetExtData(pBullet);

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