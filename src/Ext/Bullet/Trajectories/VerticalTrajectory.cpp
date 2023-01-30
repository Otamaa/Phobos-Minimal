#include "VerticalTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool VerticalTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType::Load(Stm, false);
	Stm.Process(this->Height, false);
	return true;
}

bool VerticalTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	PhobosTrajectoryType::Save(Stm);
	Stm.Process(this->Height);
	return true;
}

bool VerticalTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);

	this->Height.Read(exINI, pSection, "Trajectory.Vertical.Height");

	return true;
}

bool VerticalTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;

	return true;
}

bool VerticalTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;

	return true;
}

void VerticalTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	PhobosTrajectory::SetInaccurate(pBullet);

	this->Height = this->GetTrajectoryType()->Height + pBullet->TargetCoords.Z;

	pBullet->Velocity.X = 0;
	pBullet->Velocity.Y = 0;
	pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool VerticalTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void VerticalTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void VerticalTrajectory::OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition)
{
	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);

		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			auto pExt = BulletExt::ExtMap.Find(pBullet);
			auto type = this->GetTrajectoryType();

			pExt->LaserTrails.clear();
#ifdef COMPILE_PORTED_DP_FEATURES
			pExt->Trails.clear();
#endif
			this->IsFalling = true;
			pSpeed->X = 0.0;
			pSpeed->Y = 0.0;
			pSpeed->Z = 0.0;
			CoordStruct target = pBullet->TargetCoords;
			target.Z += static_cast<int>(type->Height);
			pBullet->Limbo();
			pBullet->Unlimbo(target, static_cast<DirType>(0));

			pPosition->X = pBullet->TargetCoords.X;
			pPosition->Y = pBullet->TargetCoords.Y;
			pPosition->Z = pBullet->TargetCoords.Z + type->Height;

			pExt->InitializeLaserTrails();

#ifdef COMPILE_PORTED_DP_FEATURES
			TrailsManager::Construct(pBullet);
#endif

		}
	}
}

TrajectoryCheckReturnType VerticalTrajectory::OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType VerticalTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
