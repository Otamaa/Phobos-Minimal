#include "MeteorTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool MeteorTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectoryType::Load(Stm, false) &&
	Stm
		.Process(this->Height, false)
		.Process(this->Range, false)
		;

}

bool MeteorTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectoryType::Save(Stm) &&
	Stm
		.Process(this->Height)
		.Process(this->Range)
		;
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
	return this->PhobosTrajectory::Load(Stm, false);
}

bool MeteorTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectory::Save(Stm);
}

void MeteorTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const type = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	int range = static_cast<int>(type->Range.Get() * Unsorted::d_LeptonsPerCell);
	double angel = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;
	double length = ScenarioClass::Instance()->Random.RandomRanged(-range, range);

	CoordStruct SourceLocation {
		pBullet->TargetCoords.X + static_cast<int>(length * Math::cos(angel))
	    ,pBullet->TargetCoords.Y + static_cast<int>(length * Math::sin(angel))
		,pBullet->TargetCoords.Z + static_cast<int>(type->Height)
	};

	this->SetInaccurate();

	pBullet->Limbo();
	pBullet->Unlimbo(SourceLocation, static_cast<DirType>(0));

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - SourceLocation.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - SourceLocation.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - SourceLocation.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
}

bool MeteorTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;

	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void MeteorTrajectory::OnAIPreDetonate() { }

void MeteorTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(pBullet->Type);
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
