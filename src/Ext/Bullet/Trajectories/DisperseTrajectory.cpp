#include "DisperseTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <ParticleSystemClass.h>
#include <ScenarioClass.h>
#include <Utilities/Helpers.h>

// https://github.com/Phobos-developers/Phobos/pull/1295
// TODO : Update

bool DisperseTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool DisperseTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	const_cast<DisperseTrajectoryType*>(this)->Serialize(Stm);
	return true;
}

bool DisperseTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);
	this->UniqueCurve.Read(exINI, pSection, "Trajectory.Disperse.UniqueCurve");
	this->PreAimCoord.Read(exINI, pSection, "Trajectory.Disperse.PreAimCoord");
	this->LaunchSpeed.Read(exINI, pSection, "Trajectory.Disperse.LaunchSpeed");
	this->Acceleration.Read(exINI, pSection, "Trajectory.Disperse.Acceleration");
	this->ROT.Read(exINI, pSection, "Trajectory.Disperse.ROT");
	this->LockDirection.Read(exINI, pSection, "Trajectory.Disperse.LockDirection");
	this->CruiseEnable.Read(exINI, pSection, "Trajectory.Disperse.CruiseEnable");
	this->CruiseUnableRange.Read(exINI, pSection, "Trajectory.Disperse.CruiseUnableRange");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Disperse.LeadTimeCalculate");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Disperse.TargetSnapDistance");
	this->RetargetAllies.Read(exINI, pSection, "Trajectory.Disperse.RetargetAllies");
	this->RetargetRadius.Read(exINI, pSection, "Trajectory.Disperse.RetargetRadius");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Disperse.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Disperse.SuicideIfNoWeapon");
	this->Weapon.Read(exINI, pSection, "Trajectory.Disperse.Weapons");
	this->WeaponBurst.Read(exINI, pSection, "Trajectory.Disperse.WeaponBurst");
	this->WeaponCount.Read(exINI, pSection, "Trajectory.Disperse.WeaponCount");
	this->WeaponDelay.Read(exINI, pSection, "Trajectory.Disperse.WeaponDelay");
	this->WeaponTimer.Read(exINI, pSection, "Trajectory.Disperse.WeaponTimer");
	this->WeaponScope.Read(exINI, pSection, "Trajectory.Disperse.WeaponScope");
	this->WeaponRetarget.Read(exINI, pSection, "Trajectory.Disperse.WeaponRetarget");
	this->WeaponLocation.Read(exINI, pSection, "Trajectory.Disperse.WeaponLocation");
	this->WeaponTendency.Read(exINI, pSection, "Trajectory.Disperse.WeaponTendency");
	this->WeaponToAllies.Read(exINI, pSection, "Trajectory.Disperse.WeaponToAllies");
	this->FacingCoord.Read(exINI, pSection, "Trajectory.Disperse.FacingCoord");

	this->Acceleration = this->Acceleration > 0.1 ? this->Acceleration : 0.1;
	this->ROT = this->ROT > 0.1 ? this->ROT : 0.1;

	return true;
}

bool DisperseTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool DisperseTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<DisperseTrajectory*>(this)->Serialize(Stm);
	return true;
}

void DisperseTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	this->LaunchSpeed = pType->LaunchSpeed;
	this->SuicideAboveRange = pType->SuicideAboveRange * 256;
	this->WeaponCount = pType->WeaponCount;
	this->WeaponTimer = pType->WeaponTimer;

	if (ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target))
		this->TargetInAir = (pTarget->GetHeight() > 0);

	this->FinalHeight = static_cast<int>(pBullet->TargetCoords.DistanceFrom(pBullet->SourceCoords));
	this->LastTargetCoord = pBullet->TargetCoords;
	this->FirepowerMult = 1.0;

	if (pBullet->Owner)
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

	if (pType->UniqueCurve || this->LaunchSpeed > 256.0)
		this->LaunchSpeed = 256.0;
	else if (this->LaunchSpeed < 0.001)
		this->LaunchSpeed = 0.001;

	if (pType->UniqueCurve)
	{
		pBullet->Velocity.X = 0;
		pBullet->Velocity.Y = 0;
		pBullet->Velocity.Z = 4.0;

		if (this->FinalHeight < 1280)
		{
			this->FinalHeight = static_cast<int>(this->FinalHeight * 1.2) + 512;
			this->SuicideAboveRange = 4 * this->FinalHeight;
		}
		else if (this->FinalHeight > 3840)
		{
			this->FinalHeight = static_cast<int>(this->FinalHeight * 0.4) + 512;
			this->SuicideAboveRange = 2 * this->FinalHeight;
		}
		else
		{
			this->FinalHeight = 2048;
			this->SuicideAboveRange = 3 * this->FinalHeight;
		}
	}
	else if (pType->PreAimCoord->X == 0 && pType->PreAimCoord->Y == 0 && pType->PreAimCoord->Z == 0)
	{
		this->InStraight = true;
		pBullet->Velocity.X = pBullet->TargetCoords.X - pBullet->SourceCoords.X;
		pBullet->Velocity.Y = pBullet->TargetCoords.Y - pBullet->SourceCoords.Y;
		pBullet->Velocity.Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;

		if (CalculateBulletVelocity(this->LaunchSpeed))
			this->SuicideAboveRange = 0.1;
	}
	else
	{
		double RotateAngle = 0.0;
		CoordStruct TheSource = pBullet->SourceCoords;

		if (pBullet->Owner)
			TheSource = pBullet->Owner->GetCoords();

		if (pBullet->Owner && (pType->FacingCoord || (pBullet->TargetCoords.Y == TheSource.Y && pBullet->TargetCoords.X == TheSource.X))) {
			if (pBullet->Owner->HasTurret())
				RotateAngle = -(pBullet->Owner->TurretFacing().GetRadian<32>());
			else
				RotateAngle = -(pBullet->Owner->PrimaryFacing.Current().GetRadian<32>());
		}
		else {
			RotateAngle = Math::atan2(float(pBullet->TargetCoords.Y - TheSource.Y), float(pBullet->TargetCoords.X - TheSource.X));
		}

		pBullet->Velocity.X = pType->PreAimCoord->X * Math::cos(RotateAngle) + pType->PreAimCoord->Y * Math::sin(RotateAngle);
		pBullet->Velocity.Y = pType->PreAimCoord->X * Math::sin(RotateAngle) - pType->PreAimCoord->Y * Math::cos(RotateAngle);
		pBullet->Velocity.Z = pType->PreAimCoord->Z;

		if (CalculateBulletVelocity(this->LaunchSpeed))
			this->SuicideAboveRange = 0.1;
	}
}

bool DisperseTrajectory::OnAI()
{
	if (BulletDetonatePreCheck())
		return true;

	auto pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();
	HouseClass* pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;
	bool VelocityUp = false;

	if (pType->WeaponScope.Get() <= 0 || pBullet->TargetCoords.DistanceFrom(pBullet->Location) <= pType->WeaponScope.Get())
	{
		if (this->WeaponCount > 0)
		{
			if (pOwner)
			{
				if (PrepareDisperseWeapon(pOwner))
					return true;
			}
			else
			{
				return true;
			}
		}
	}

	if (pType->UniqueCurve)
	{
		if (CurveVelocityChange())
			return true;

		return false;
	}

	if (this->Accelerate)
	{
		double StraightSpeed = this->GetTrajectorySpeed();
		this->LaunchSpeed += pType->Acceleration;

		if (StraightSpeed > 256.0)
		{
			if (this->LaunchSpeed >= 256.0)
			{
				this->LaunchSpeed = 256.0;
				this->Accelerate = false;
			}
		}
		else
		{
			if (this->LaunchSpeed >= StraightSpeed)
			{
				this->LaunchSpeed = StraightSpeed;
				this->Accelerate = false;
			}
		}

		VelocityUp = true;
	}

	if (!pType->LockDirection )
	{
		if (pType->RetargetRadius != 0 && BulletRetargetTechno(pOwner))
			return true;

		if (this->InStraight)
		{
			if (StandardVelocityChange())
				return true;
		}
		else if (pBullet->SourceCoords.DistanceFromSquared(pBullet->Location) >= pType->PreAimCoord->pow())
		{
			this->InStraight = true;

			if (StandardVelocityChange())
				return true;

			this->InStraight = true;
		}

		VelocityUp = true;
	}

	if (VelocityUp && CalculateBulletVelocity(this->LaunchSpeed))
		return true;

	return false;
}

void DisperseTrajectory::OnAIPreDetonate()
{
	auto pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= pType->TargetSnapDistance.Get())
	{
		BulletExtContainer::Instance.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void DisperseTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(this->AttachedTo->Type);
}

TrajectoryCheckReturnType DisperseTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType DisperseTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

bool DisperseTrajectory::CalculateBulletVelocity(double StraightSpeed) const
{
	auto pBullet = this->AttachedTo;
	double VelocityLength = pBullet->Velocity.Length();

	if (VelocityLength > 0)
		pBullet->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool DisperseTrajectory::BulletDetonatePreCheck()
{
	auto pBullet = this->AttachedTo;
	if (MapClass::Instance->GetCellFloorHeight(pBullet->Location) > pBullet->Location.Z)
		return true;

	auto const pType = this->GetTrajectoryType();
	double TargetDistance = pBullet->TargetCoords.DistanceFrom(pBullet->Location);

	if (pType->UniqueCurve)
	{
		if (TargetDistance > 128)
			return false;
		else
			return true;
	}

	if (this->SuicideAboveRange > 0)
	{
		double BulletSpeed = this->LaunchSpeed;

		if (pType->UniqueCurve)
			BulletSpeed = pBullet->Velocity.Length();

		this->SuicideAboveRange -= BulletSpeed;

		if (this->SuicideAboveRange <= 0)
			return true;
	}

	if (TargetDistance < pType->TargetSnapDistance.Get())
		return true;

	return false;
}

bool DisperseTrajectory::BulletRetargetTechno(HouseClass* pOwner)
{
	auto pBullet = this->AttachedTo;
	bool Check = false;

	if (!pBullet->Target)
	{
		Check = true;
	}
	else if (TechnoClass* pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
	{
		if (pTarget->IsDead() || pTarget->InLimbo)Check = true;
	}

	auto const pType = this->GetTrajectoryType();

	if (!Check)
		return false;

	if (pType->RetargetRadius.Get() < 0)
		return true;

	CoordStruct RetargetCoords = pBullet->TargetCoords;

	if (this->InStraight)
	{
		VelocityClass FutureVelocity = pBullet->Velocity * (pType->RetargetRadius.Get() * 256.0 / this->LaunchSpeed);
		RetargetCoords.X = pBullet->Location.X + static_cast<int>(FutureVelocity.X);
		RetargetCoords.Y = pBullet->Location.Y + static_cast<int>(FutureVelocity.Y);
		RetargetCoords.Z = pBullet->Location.Z;
	}

	std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(RetargetCoords, pType->RetargetRadius.Get(), this->TargetInAir);
	std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pBullet->WeaponType->Warhead, false);
	int ValidTechnoNums = ValidTechnos.size();

	if (ValidTechnoNums > 0)
	{
		int num = ScenarioClass::Instance->Random.RandomRanged(0, ValidTechnoNums - 1);
		TechnoClass* BulletTarget = ValidTechnos[num];

		pBullet->Target = BulletTarget;
		pBullet->TargetCoords = BulletTarget->GetCoords();

		this->LastTargetCoord = pBullet->TargetCoords;
	}

	return false;
}

bool DisperseTrajectory::CurveVelocityChange()
{
	auto pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TargetLocation = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
	pBullet->TargetCoords = TargetLocation;

	if (!this->InStraight)
	{
		int OffHeight = this->FinalHeight - 1600;

		if (this->FinalHeight < 3200)
			OffHeight = this->FinalHeight / 2;

		CoordStruct HorizonVelocity { TargetLocation.X - pBullet->Location.X, TargetLocation.Y - pBullet->Location.Y, 0 };
		double HorizonDistance = HorizonVelocity.Length();

		if (HorizonDistance > 0)
		{
			double HorizonMult = Math::abs(pBullet->Velocity.Z / 64.0) / HorizonDistance;
			pBullet->Velocity.X += HorizonMult * HorizonVelocity.X;
			pBullet->Velocity.Y += HorizonMult * HorizonVelocity.Y;
			double HorizonLength = std::sqrt(pBullet->Velocity.X * pBullet->Velocity.X + pBullet->Velocity.Y * pBullet->Velocity.Y);

			if (HorizonLength > 64)
			{
				HorizonMult = 64 / HorizonLength;
				pBullet->Velocity.X *= HorizonMult;
				pBullet->Velocity.Y *= HorizonMult;
			}
		}

		if ((pBullet->Location.Z - pBullet->SourceCoords.Z) < OffHeight && this->Accelerate)
		{
			if (pBullet->Velocity.Z < 160.0)
				pBullet->Velocity.Z += 4.0;
		}
		else
		{
			this->Accelerate = false;
			double FutureLocation = pBullet->Location.Z + 8 * pBullet->Velocity.Z;

			if (pBullet->Velocity.Z > -160.0)
				pBullet->Velocity.Z -= 4.0;

			if (FutureLocation <= TargetLocation.Z)
				this->InStraight = true;
			else if (FutureLocation <= pBullet->SourceCoords.Z)
				this->InStraight = true;
		}
	}
	else
	{
		double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / 192.0;

		TargetLocation.Z += static_cast<int>(TimeMult * 32);

		TargetLocation.X += static_cast<int>(TimeMult * (TargetLocation.X - this->LastTargetCoord.X));
		TargetLocation.Y += static_cast<int>(TimeMult * (TargetLocation.Y - this->LastTargetCoord.Y));

		this->LastTargetCoord = pBullet->TargetCoords;
		if (ChangeBulletVelocity(TargetLocation, 24.0, true))
			return true;
	}

	return false;
}

bool DisperseTrajectory::StandardVelocityChange()
{
	auto pBullet = this->AttachedTo;
	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	CoordStruct TargetLocation = pTarget ? pTarget->GetCoords() : pBullet->TargetCoords;
	pBullet->TargetCoords = TargetLocation;

	CoordStruct TargetHorizon { TargetLocation.X, TargetLocation.Y, 0 };
	CoordStruct BulletHorizon { pBullet->Location.X, pBullet->Location.Y, 0 };
	auto const pType = this->GetTrajectoryType();

	if (pType->CruiseEnable && TargetHorizon.DistanceFrom(BulletHorizon) > pType->CruiseUnableRange.Get())
		TargetLocation.Z = pBullet->Location.Z;

	if (pType->LeadTimeCalculate)
	{
		double LeadSpeed = (this->GetTrajectorySpeed() + this->LaunchSpeed) / 2.0;
		LeadSpeed = LeadSpeed > 0 ? LeadSpeed : 1;
		double TimeMult = TargetLocation.DistanceFrom(pBullet->Location) / LeadSpeed;
		TargetLocation.X += static_cast<int>(TimeMult * (TargetLocation.X - this->LastTargetCoord.X));
		TargetLocation.Y += static_cast<int>(TimeMult * (TargetLocation.Y - this->LastTargetCoord.Y));
	}

	this->LastTargetCoord = pBullet->TargetCoords;
	double TurningRadius = pType->ROT * this->LaunchSpeed * this->LaunchSpeed / 16384;

	if (ChangeBulletVelocity(TargetLocation, TurningRadius, false))
		return true;

	return false;
}

bool DisperseTrajectory::ChangeBulletVelocity(CoordStruct TargetLocation, double TurningRadius, bool Curve)
{
	auto pBullet = this->AttachedTo;
	CoordStruct TargetVelocity
	{
		TargetLocation.X - pBullet->Location.X,
		TargetLocation.Y - pBullet->Location.Y,
		TargetLocation.Z - pBullet->Location.Z
	};

	VelocityClass MoveToVelocity = pBullet->Velocity;

	CoordStruct FutureVelocity
	{
		static_cast<int>(TargetVelocity.X - MoveToVelocity.X),
		static_cast<int>(TargetVelocity.Y - MoveToVelocity.Y),
		static_cast<int>(TargetVelocity.Z - MoveToVelocity.Z)
	};

	VelocityClass ReviseVelocity = { 0,0,0 };
	VelocityClass DirectVelocity = { 0,0,0 };

	double TargetSquared = TargetVelocity.pow();
	double BulletSquared = MoveToVelocity.pow();
	double FutureSquared = FutureVelocity.pow();

	double TargetSide = std::sqrt(TargetSquared);
	double BulletSide = std::sqrt(BulletSquared);

	double ReviseMult = (TargetSquared + BulletSquared - FutureSquared);
	double ReviseBase = 2 * TargetSide * BulletSide;

	if (TargetSide > 0)
	{
		if (ReviseMult < 0.005 * ReviseBase && ReviseMult > -0.005 * ReviseBase)
		{
			double VelocityMult = TurningRadius / TargetSide;

			pBullet->Velocity.X += TargetVelocity.X * VelocityMult;
			pBullet->Velocity.Y += TargetVelocity.Y * VelocityMult;
			pBullet->Velocity.Z += TargetVelocity.Z * VelocityMult;
		}
		else
		{
			double DirectLength = ReviseBase * BulletSide / ReviseMult;
			double VelocityMult = DirectLength / TargetSide;

			DirectVelocity.X = TargetVelocity.X * VelocityMult;
			DirectVelocity.Y = TargetVelocity.Y * VelocityMult;
			DirectVelocity.Z = TargetVelocity.Z * VelocityMult;

			if (DirectVelocity.IsCollinearTo(MoveToVelocity))
			{
				if (ReviseMult < 0)
					ReviseVelocity.Z += TurningRadius;
			}
			else
			{
				if (ReviseMult > 0)
					ReviseVelocity = DirectVelocity - MoveToVelocity;
				else
					ReviseVelocity = MoveToVelocity - DirectVelocity;
			}

			double ReviseLength = ReviseVelocity.Length();

			if (!Curve && ReviseMult < 0 && this->LastReviseMult > 0 && this->LastTargetCoord == pBullet->TargetCoords)
				return true;

			if (TurningRadius < ReviseLength)
			{
				ReviseVelocity *= TurningRadius / ReviseLength;

				pBullet->Velocity.X += ReviseVelocity.X;
				pBullet->Velocity.Y += ReviseVelocity.Y;
				pBullet->Velocity.Z += ReviseVelocity.Z;
			}
			else
			{
				pBullet->Velocity.X = TargetVelocity.X;
				pBullet->Velocity.Y = TargetVelocity.Y;
				pBullet->Velocity.Z = TargetVelocity.Z;

				if (!Curve)
					this->InStraight = true;
			}
		}
	}
	this->LastReviseMult = ReviseMult;
	this->LastTargetCoord = pBullet->TargetCoords;

	if (Curve)
	{
		if (BulletSide < 192)
			BulletSide += 4;

		if (BulletSide > 192)
			BulletSide = 192;

		if (CalculateBulletVelocity(BulletSide))
			return true;
	}

	return false;
}

bool DisperseTrajectory::PrepareDisperseWeapon(HouseClass* pOwner)
{
	auto pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	if (this->WeaponTimer == 0)
	{
		int ValidWeapons = static_cast<int>(MaxImpl(pType->Weapon.size(), pType->WeaponBurst.size()));

		AbstractClass* BulletTarget = pBullet->Target ? pBullet->Target
			: MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

		if (pType->WeaponRetarget)
		{
			for (int i = 0; i < ValidWeapons; i++)
			{
				auto const pWeapon = pType->Weapon[i];
				double Spread = pWeapon->Range / 256.0;
				CoordStruct CenterCoords = pType->WeaponLocation ? pBullet->Location : pBullet->TargetCoords;
				bool IncludeInAir = (this->TargetInAir && pWeapon->Projectile->AA);

				std::vector<TechnoClass*> Technos = Helpers::Alex::getCellSpreadItems(CenterCoords, Spread, IncludeInAir);
				std::vector<TechnoClass*> ValidTechnos = GetValidTechnosInSame(Technos, pOwner, pWeapon->Warhead, true);
				int ValidTechnoNums = ValidTechnos.size();

				for (int j = 0; j < pType->WeaponBurst[i]; j++)
				{
					if (ValidTechnoNums > 0)
					{
						int k = ScenarioClass::Instance->Random.RandomRanged(0, ValidTechnoNums - 1);
						BulletTarget = ValidTechnos[k];
					}
					else
					{
						CellClass* RandomCell = nullptr;
						int RandomRange = ScenarioClass::Instance->Random.RandomRanged(0, pWeapon->Range);
						CoordStruct RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);

						while (!RandomCell)
						{
							RandomCell = MapClass::Instance->TryGetCellAt(RandomCoords);
							RandomRange = (RandomRange > 256) ? RandomRange / 2 : 1;
							RandomCoords = MapClass::GetRandomCoordsNear(CenterCoords, RandomRange, false);
						}

						BulletTarget = RandomCell;
					}

					if (pType->WeaponTendency && i == 0 && j == 0)
						BulletTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->TryGetCellAt(pBullet->TargetCoords);

					CreateDisperseBullets(pWeapon, BulletTarget, pOwner);
				}
			}
		}
		else
		{
			for (int i = 0; i < ValidWeapons; i++)
			{
				auto const pWeapon = pType->Weapon[i];

				for (int j = 0; j < pType->WeaponBurst[i]; j++)
					CreateDisperseBullets(pWeapon, BulletTarget, pOwner);
			}
		}

		this->WeaponCount -= 1;
	}

	this->WeaponTimer += 1;
	if (this->WeaponTimer > 0)
		this->WeaponTimer %= pType->WeaponDelay;

	if (pType->SuicideIfNoWeapon && this->WeaponCount == 0)
		return true;

	return false;
}

#include <Ext/Techno/Body.h>

std::vector<TechnoClass*> DisperseTrajectory::GetValidTechnosInSame(std::vector<TechnoClass*>& Technos,
	HouseClass* pOwner, WarheadTypeClass* pWH, bool Mode) const
{
	auto pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();
	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(Technos.size());
	bool CheckAllies = Mode ? !pType->WeaponToAllies : !pType->RetargetAllies;

	for (auto const& pTechno : Technos)
	{
		if (this->TargetInAir != pTechno->GetHeight() > 0)
			continue;

		if (pTechno->IsDead() || pTechno->InLimbo)
			continue;

		if (CheckAllies)
		{
			if (pOwner->IsAlliedWith(pTechno->Owner))
				continue;

			if (pTechno->WhatAmI() == AbstractType::Infantry && pTechno->IsDisguisedAs(pOwner))
				continue;
		}

		if (pTechno->WhatAmI() == AbstractType::Unit && pTechno->IsDisguised())
			continue;

		if (pTechno->CloakState == CloakState::Cloaked)
			continue;

		auto const armor_ = TechnoExtData::GetArmor(pTechno);
		if (MapClass::GetTotalDamage(100, pWH, armor_, 0) == 0)
			continue;

		ValidTechnos.push_back(pTechno);
	}

	return ValidTechnos;
}

void DisperseTrajectory::CreateDisperseBullets(WeaponTypeClass* pWeapon, AbstractClass* BulletTarget, HouseClass* pOwner) const
{
	auto pBullet = this->AttachedTo;

	int FinalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);

	if (BulletClass* pCreateBullet = pWeapon->Projectile->CreateBullet(BulletTarget, pBullet->Owner,
		FinalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		pCreateBullet->WeaponType = pWeapon;
		auto const pBulletExt = BulletExtContainer::Instance.Find(pCreateBullet);
		pBulletExt->Owner = BulletExtContainer::Instance.Find(pBullet)->Owner;
		pCreateBullet->MoveTo(pBullet->Location, {});
	}

	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser;
		if (pWeapon->IsHouseColor)
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
				pOwner->LaserColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else if (WeaponTypeExtContainer::Instance.Find(pWeapon)->Laser_IsSingleColor)
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
				pWeapon->LaserInnerColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pWeapon->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else
		{
			pLaser = GameCreate<LaserDrawClass>(pBullet->Location, BulletTarget->GetCoords(),
				pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);
			pLaser->IsHouseColor = false;
		}

		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}

	if (pWeapon->IsElectricBolt)
	{
		if (EBolt* pEBolt = GameCreate<EBolt>())
		{
			if (pWeapon->IsAlternateColor)
				pEBolt->AlternateColor = true;
			else
				pEBolt->AlternateColor = false;

			pEBolt->Fire(pBullet->Location, BulletTarget->GetCoords(), 0);
		}
	}

	if (pWeapon->IsRadBeam)
	{
		RadBeamType pRadBeamType;
		if (pWeapon->Warhead->Temporal)
			pRadBeamType = RadBeamType::Temporal;
		else
			pRadBeamType = RadBeamType::RadBeam;

		if (RadBeam* pRadBeam = RadBeam::Allocate(pRadBeamType))
		{
			pRadBeam->SetCoordsSource(pBullet->Location);
			pRadBeam->SetCoordsTarget(BulletTarget->GetCoords());
			pRadBeam->Color = pWeapon->Warhead->Temporal ? ColorStruct { 255, 100, 0 } : ColorStruct { 128, 200, 255 };
		}
	}

	if (ParticleSystemTypeClass* pPSType = pWeapon->AttachedParticleSystem)
		GameCreate<ParticleSystemClass>(pPSType, pBullet->Location, BulletTarget, pBullet->Owner, BulletTarget->GetCoords(), pOwner);
}