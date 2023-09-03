#include "ArcingTrajectory.h"

#include <Ext/BulletType/Body.h>

bool ArcingTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectoryType::Load(Stm, false) && Stm
		.Process(this->Elevation, false)
		.Process(this->Lobber, false)
		.Success()
		;
}

bool ArcingTrajectoryType::Save(PhobosStreamWriter& Stm) const
{

	return this->PhobosTrajectoryType::Save(Stm) && Stm
		.Process(this->Elevation)
		.Process(this->Lobber)
		.Success()
		;
}

bool ArcingTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);

	this->Elevation.Read(exINI, pSection, "Trajectory.Arcing.Elevation");
	this->Lobber.Read(exINI, pSection, "Trajectory.Arcing.Lobber");

	return true;
}

bool ArcingTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectory::Load(Stm, false) && Stm
		.Process(this->OverRange, false)
		.Success()
		;
}

bool ArcingTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectory::Save(Stm) && Stm
		.Process(this->OverRange)
		.Success()
		;
}

void ArcingTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto pBullet = this->AttachedTo;

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

		int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? (int)(pTypeExt->BallisticScatter_Max.Get()) : ballisticScatter;
		int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? (int)(pTypeExt->BallisticScatter_Min.Get()) : (scatterMax / 2);

		double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		pBullet->TargetCoords += offset;
	}

	CoordStruct InitialSourceLocation = pBullet->SourceCoords;
	InitialSourceLocation.Z = 0;
	CoordStruct InitialTargetLocation = pBullet->TargetCoords;
	InitialTargetLocation.Z = 0;
	double FullDistance = InitialTargetLocation.DistanceFrom(InitialSourceLocation);

	double Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
	double g = BulletTypeExt::GetAdjustedGravity(pBullet->Type);

	double Elevation = this->GetTrajectoryType()->Elevation;
	if (Elevation > DBL_EPSILON)
	{
		double LifeTime = Math::sqrt(2 / g * (Elevation * FullDistance - Z));

		double Velocity_XY = FullDistance / LifeTime;
		double ratio = Velocity_XY / FullDistance;
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
		pBullet->Velocity.Z = Elevation * Velocity_XY;
	}
	else
	{
		double S = this->GetTrajectorySpeed();
		double A = g * g / 4;
		double B = g * Z - S * S;
		double C = Z * Z + FullDistance * FullDistance;

		double delta = B * B - 4 * A * C;
		if (delta < 0)
		{
			double Velocity_XY = S / Math::Sqrt2;
			double ratio = Velocity_XY / FullDistance;
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
			pBullet->Velocity.Z = Velocity_XY;

			this->OverRange = true;
		}
		else
		{
			int isLobber = this->GetTrajectoryType()->Lobber ? 1 : -1;
			double LifeTimeSquare = (-B + isLobber * Math::sqrt(delta)) / (2 * A);
			double LifeTime = Math::sqrt(LifeTimeSquare);

			double Velocity_XY = FullDistance / LifeTime;
			double ratio = Velocity_XY / FullDistance;
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;

			pBullet->Velocity.Z = Math::sqrt(S * S - Velocity_XY * Velocity_XY);
		}
	}
}

bool ArcingTrajectory::OnAI()
{
	auto pBullet = this->AttachedTo;

	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (this->OverRange)
	{
		if (pBullet->GetHeight() + pBullet->Velocity.Z < 0)
			return true;
	}

	return false;
}

void ArcingTrajectory::OnAIPreDetonate()
{
}

void ArcingTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
}

TrajectoryCheckReturnType ArcingTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArcingTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}