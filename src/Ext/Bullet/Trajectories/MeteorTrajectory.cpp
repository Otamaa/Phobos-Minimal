#include "MeteorTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool MeteorTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->Height, false)
		.Process(this->Range, false)
		;

	return true;
}

bool MeteorTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->Height)
		.Process(this->Range)
		;
	return true;
}

bool MeteorTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false ;

	INI_EX exINI(pINI);
	this->Height.Read(exINI, pSection, "Trajectory.Meteor.Height");
	this->Range.Read(exINI, pSection, "Trajectory.Meteor.Range");

	return true;
}

bool MeteorTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		;

	return true;
}

bool MeteorTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		;

	return true;
}

void MeteorTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto type = this->GetTrajectoryType();
	int range = static_cast<int>(type->Range.Get() * Unsorted::d_LeptonsPerCell);
	double angel = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;
	double length = ScenarioClass::Instance()->Random.RandomRanged(-range, range);

	CoordStruct SourceLocation {
		pBullet->TargetCoords.X + static_cast<int>(length * Math::cos(angel))
	    ,pBullet->TargetCoords.Y + static_cast<int>(length * Math::sin(angel))
		,pBullet->TargetCoords.Z + static_cast<int>(type->Height)
	};

	PhobosTrajectory::SetInaccurate(pBullet);

	pBullet->Limbo();
	pBullet->Unlimbo(SourceLocation, static_cast<DirType>(0));

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - SourceLocation.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - SourceLocation.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - SourceLocation.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool MeteorTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void MeteorTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void MeteorTrajectory::OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
