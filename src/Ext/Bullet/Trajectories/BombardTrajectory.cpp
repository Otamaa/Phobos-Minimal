#include "BombardTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
		Stm
		.Process(this->Height, false)
		.Process(this->FallPercent, false)
		.Process(this->FallPercentShift, false)
		.Process(this->FallScatterRange, false)
		.Process(this->FallSpeed, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->FreeFallOnTarget, false)
		.Process(this->NoLaunch, false)
		.Process(this->TurningPointAnim, true)
		;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
		Stm
		.Process(this->Height, false)
		.Process(this->FallPercent, false)
		.Process(this->FallPercentShift, false)
		.Process(this->FallScatterRange, false)
		.Process(this->FallSpeed, false)
		.Process(this->TargetSnapDistance, false)
		.Process(this->FreeFallOnTarget, false)
		.Process(this->NoLaunch, false)
		.Process(this->TurningPointAnim, false)
		;
}

bool BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->FallPercent.Read(exINI, pSection, "Trajectory.Bombard.FallPercent");
	this->FallPercentShift.Read(exINI, pSection, "Trajectory.Bombard.FallPercentShift");
	this->FallScatterRange.Read(exINI, pSection, "Trajectory.Bombard.FallScatterRange");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Bombard.TargetSnapDistance");
	this->FreeFallOnTarget.Read(exINI, pSection, "Trajectory.Bombard.FreeFallOnTarget");
	this->NoLaunch.Read(exINI, pSection, "Trajectory.Bombard.NoLaunch");
	this->TurningPointAnim.Read(exINI, pSection, "Trajectory.Bombard.TurningPointAnim");
	return true;
}

void BombardTrajectory::ApplyTurningPointAnim(CoordStruct& Position)
{
	auto const pType = this->GetTrajectoryType();

	if (pType->TurningPointAnim) {
		if (auto const pAnim = GameCreate<AnimClass>(pType->TurningPointAnim, Position)) {
			auto pExt = BulletExtContainer::Instance.Find(this->AttachedTo);
			auto pTechno = this->AttachedTo->Owner ? this->AttachedTo->Owner : nullptr;
			auto pOwner = pTechno && pTechno->Owner ? pTechno->Owner : pExt->Owner;
			pAnim->SetOwnerObject(pTechno);
			pAnim->Owner = pOwner;
		}
	}
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) &&
	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;

}

void BombardTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->SetInaccurate();

	// use scaling since RandomRanged only support int
	double fallPercentShift = ScenarioClass::Instance()->Random.RandomRanged(0, int(200 * pType->FallPercentShift)) / 100.0;
	double fallPercent = pType->FallPercent - pType->FallPercentShift + fallPercentShift;
	this->Height = pType->Height + pBullet->TargetCoords.Z;

	if (!pType->NoLaunch)
	{
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * fallPercent;
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * fallPercent;
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed() / pBullet->Velocity.Length();
	}
	else
	{
		this->IsFalling = true;
		CoordStruct SourceLocation {};
		SourceLocation.Z = (int)this->Height - pBullet->SourceCoords.Z;
		int scatterRange = static_cast<int>(pType->FallScatterRange.Get());
		double angel = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;
		double length = ScenarioClass::Instance()->Random.RandomRanged(-scatterRange, scatterRange);
		int scatterX = static_cast<int>(length * Math::cos(angel));
		int scatterY = static_cast<int>(length * Math::sin(angel));

		if (!pType->FreeFallOnTarget)
		{
			SourceLocation.X = pBullet->SourceCoords.X + static_cast<int>((pBullet->TargetCoords.X - pBullet->SourceCoords.X) * fallPercent) + scatterX;
			SourceLocation.Y = pBullet->SourceCoords.Y + static_cast<int>((pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * fallPercent) + scatterY;
			pBullet->Limbo();
			pBullet->Unlimbo(SourceLocation, static_cast<DirType>(0));
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - SourceLocation.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - SourceLocation.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - SourceLocation.Z);
			pBullet->Velocity *= pType->FallSpeed / pBullet->Velocity.Length();
		}
		else
		{
			SourceLocation.X = pBullet->TargetCoords.X + scatterX;
			SourceLocation.Y = pBullet->TargetCoords.Y + scatterY;
			pBullet->Limbo();
			pBullet->Unlimbo(SourceLocation, static_cast<DirType>(0));
			pBullet->Velocity.X = 0.0;
			pBullet->Velocity.Y = 0.0;
			pBullet->Velocity.Z = 0.0;
		}

		this->ApplyTurningPointAnim(SourceLocation);
	}
}

bool BombardTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;

	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) <  this->DetonationDistance) // This value maybe adjusted?
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;

	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= this->GetTrajectoryType()->TargetSnapDistance.Get())
	{
		auto const pExt = BulletExtContainer::Instance.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void BombardTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (!this->IsFalling)
	{
		pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(pBullet->Type);
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			this->IsFalling = true;
			if (!pType->FreeFallOnTarget)
			{
				pSpeed->X = static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X - pBullet->Velocity.X);
				pSpeed->Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y - pBullet->Velocity.Y);
				pSpeed->Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->Location.Z - pBullet->Velocity.Z);
				(*pSpeed) *= pType->FallSpeed / pSpeed->Length();
				pPosition->X = pBullet->Location.X + pBullet->Velocity.X;
				pPosition->Y = pBullet->Location.Y + pBullet->Velocity.Y;
				pPosition->Z = pBullet->Location.Z + pBullet->Velocity.Z;
			}
			else
			{
				pSpeed->X = 0.0;
				pSpeed->Y = 0.0;
				pSpeed->Z = 0.0;
				if (pType->FallPercent != 1.0) // change position and recreate laser trail
				{
					auto pExt = BulletExtContainer::Instance.Find(pBullet);
					pExt->LaserTrails.clear();
					CoordStruct target = pBullet->TargetCoords;
					target.Z += (int)pType->Height;
					pBullet->Limbo();
					pBullet->Unlimbo(target, static_cast<DirType>(0));
					pPosition->X = pBullet->TargetCoords.X;
					pPosition->Y = pBullet->TargetCoords.Y;
					pPosition->Z = pBullet->TargetCoords.Z + pType->Height;

					pExt->LaserTrails.clear();
					pExt->InitializeLaserTrails();
					TrailsManager::CleanUp(pExt->AttachedToObject);
					TrailsManager::Construct(pExt->AttachedToObject);
				}
				else
				{
					pPosition->X = pBullet->TargetCoords.X;
					pPosition->Y = pBullet->TargetCoords.Y;
				}
			}

			CoordStruct BulletLocation {
				static_cast<int>(pPosition->X),
				static_cast<int>(pPosition->Y),
				static_cast<int>(pPosition->Z)
			};

			this->ApplyTurningPointAnim(BulletLocation);
		}
	}
	else if (!pType->FreeFallOnTarget)
	{
		pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(pBullet->Type);
	}
}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}