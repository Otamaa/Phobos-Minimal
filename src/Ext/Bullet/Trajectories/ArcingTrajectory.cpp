#include "ArcingTrajectory.h"

#include <Ext/BulletType/Body.h>

#include <BulletClass.h>

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

void ArcingTrajectory::CalculateVelocity(BulletClass* pBullet, double elevation, bool lobber, ArcingTrajectory* pTraj)
{
	CoordStruct InitialSourceLocation = pBullet->SourceCoords;
	InitialSourceLocation.Z = 0;
	CoordStruct InitialTargetLocation = pBullet->TargetCoords;
	InitialTargetLocation.Z = 0;
	double FullDistance = InitialTargetLocation.DistanceFrom(InitialSourceLocation);

	double Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
	double g = BulletTypeExtData::GetAdjustedGravity(pBullet->Type);

	// Handle edge case where source and target are at the same XY position (FullDistance == 0)
	// In this case, just fire straight up/down
	if (FullDistance < 1e-10)
	{
		pBullet->Velocity.X = 0.0;
		pBullet->Velocity.Y = 0.0;
		pBullet->Velocity.Z = (Z >= 0) ? 1.0 : -1.0;
		return;
	}

	if (elevation > DBL_EPSILON)
	{
		double LifeTime = Math::sqrt(2 / g * (elevation * FullDistance - Z));

		// Guard against zero or invalid LifeTime
		if (LifeTime < 1e-10)
		{
			pBullet->Velocity.X = 0.0;
			pBullet->Velocity.Y = 0.0;
			pBullet->Velocity.Z = 1.0;
			return;
		}

		double Velocity_XY = FullDistance / LifeTime;
		double ratio = Velocity_XY / FullDistance;
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
		pBullet->Velocity.Z = elevation * Velocity_XY;
	}
	else // 不指定发射仰角，则读取Speed设定作为出膛速率，每次攻击时自动计算发射仰角
	{
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
		double S = pBulletTypeExt->Trajectory_Speed;
		double A = g * g / 4;
		double B = g * Z - S * S;
		double C = Z * Z + FullDistance * FullDistance;
		double delta = B * B - 4 * A * C;

		if (delta < 0)
		{
			double Velocity_XY = S / Math::SQRT_TWO;
			double ratio = Velocity_XY / FullDistance;
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
			pBullet->Velocity.Z = Velocity_XY;

			if (pTraj != nullptr)
				pTraj->OverRange = true;
		}
		else
		{
			int isLobber = lobber ? 1 : -1;
			double LifeTimeSquare = (-B + isLobber * Math::sqrt(delta)) / (2 * A);
			double LifeTime = Math::sqrt(LifeTimeSquare);

			// Guard against zero LifeTime (can happen if A is very small or LifeTimeSquare is zero)
			if (LifeTime < 1e-10)
			{
				pBullet->Velocity.X = 0.0;
				pBullet->Velocity.Y = 0.0;
				pBullet->Velocity.Z = S;
				return;
			}

			double Velocity_XY = FullDistance / LifeTime;
			double ratio = Velocity_XY / FullDistance;
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;

			pBullet->Velocity.Z = Math::sqrt(S * S - Velocity_XY * Velocity_XY);
		}
	}
}

void ArcingTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	this->SetInaccurate();
	const auto pType = this->GetTrajectoryType();
	CalculateVelocity(this->AttachedTo, pType->Elevation, pType->Lobber, this);
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