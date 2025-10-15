#include "StraightTrajectoryVarianC.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>

#include <AircraftTrackerClass.h>

// https://github.com/Phobos-developers/Phobos/pull/1294
// TODO : update

bool StraightVariantCTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->ApplyRangeModifiers.Read(exINI, pSection, "Trajectory.Straight.ApplyRangeModifiers");
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Straight.DetonationDistance");

	if (!this->DetonationDistance.isset())
		this->DetonationDistance = Leptons(102);

	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Straight.TargetSnapDistance");
	this->PassThrough.Read(exINI, pSection, "Trajectory.Straight.PassThrough");
	this->PassDetonate.Read(exINI, pSection, "Trajectory.Straight.PassDetonate");
	this->PassDetonateWarhead.Read(exINI, pSection, "Trajectory.Straight.PassDetonateWarhead");
	this->PassDetonateDamage.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDamage");
	this->PassDetonateDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDelay");
	this->PassDetonateInitialDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateInitialDelay");
	this->PassDetonateLocal.Read(exINI, pSection, "Trajectory.Straight.PassDetonateLocal");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Straight.LeadTimeCalculate");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Straight.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Straight.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Straight.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Straight.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Straight.AxisOfRotation");
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.Straight.ProximityImpact");
	this->ProximityWarhead.Read(exINI, pSection, "Trajectory.Straight.ProximityWarhead");
	this->ProximityDamage.Read(exINI, pSection, "Trajectory.Straight.ProximityDamage");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityDirect.Read(exINI, pSection, "Trajectory.Straight.ProximityDirect");
	this->ProximityMedial.Read(exINI, pSection, "Trajectory.Straight.ProximityMedial");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ProximityFlight.Read(exINI, pSection, "Trajectory.Straight.ProximityFlight");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Straight.SubjectToGround");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
	this->EdgeAttenuation.Read(exINI, pSection, "Trajectory.Straight.EdgeAttenuation");

	if (this->EdgeAttenuation < 0.0)
		this->EdgeAttenuation = 0.0;

	this->CountAttenuation.Read(exINI, pSection, "Trajectory.Straight.CountAttenuation");

	if (this->CountAttenuation < 0.0)
		this->CountAttenuation = 0.0;

	return true;
}

bool StraightVariantCTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
		Stm
		.Process(this->TargetSnapDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
		.Process(this->PassDetonateWarhead)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateInitialDelay)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CountAttenuation)
		;

}

bool StraightVariantCTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
		Stm
		.Process(this->TargetSnapDistance)
		.Process(this->ApplyRangeModifiers)
		.Process(this->PassThrough)
		.Process(this->PassDetonate)
		.Process(this->PassDetonateWarhead)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateInitialDelay)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		.Process(this->ProximityImpact)
		.Process(this->ProximityWarhead)
		.Process(this->ProximityDamage)
		.Process(this->ProximityRadius)
		.Process(this->ProximityDirect)
		.Process(this->ProximityMedial)
		.Process(this->ProximityAllies)
		.Process(this->ProximityFlight)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		.Process(this->CountAttenuation)
		;

}

bool StraightTrajectoryVarianC::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
		Stm
		.Process(this->TrajectorySpeed)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->RemainingDistance)
		.Process(this->ExtraCheck)
		.Process(this->TheCasualty)
		.Process(this->FirepowerMult)
		.Process(this->AttenuationRange)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		;
}

bool StraightTrajectoryVarianC::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) &&
		Stm
		.Process(this->TrajectorySpeed)
		.Process(this->PassDetonateDamage)
		.Process(this->PassDetonateTimer)
		.Process(this->OffsetCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->ProximityImpact)
		.Process(this->ProximityDamage)
		.Process(this->RemainingDistance)
		.Process(this->ExtraCheck)
		.Process(this->TheCasualty)
		.Process(this->FirepowerMult)
		.Process(this->AttenuationRange)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		;
}

void StraightTrajectoryVarianC::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->DetonationDistance = pType->DetonationDistance;
	this->PassDetonateDamage = pType->PassDetonateDamage;
	this->OffsetCoord = pType->OffsetCoord.Get();
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->ProximityImpact = pType->ProximityImpact;
	this->ProximityDamage = pType->ProximityDamage;
	this->TrajectorySpeed = this->GetTrajectorySpeed();
	this->PassDetonateTimer.Start(pType->PassDetonateInitialDelay > 0 ? pType->PassDetonateInitialDelay : 0);

	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = VelocityClass::Empty;
	const auto pFirer = pBullet->Owner;

	if (const auto pWeapon = pBullet->WeaponType)
	{
		this->AttenuationRange = pWeapon->Range;
		this->CountOfBurst = pWeapon->Burst;

		if (pType->ApplyRangeModifiers && pFirer)
		{
			if (this->DetonationDistance >= 0)
				this->DetonationDistance = Leptons(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pFirer, this->DetonationDistance));
			else
				this->DetonationDistance = Leptons(-WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pFirer, -this->DetonationDistance));

			this->AttenuationRange = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pFirer);
		}
	}

	if (pFirer)
	{
		this->CurrentBurst = pFirer->CurrentBurstIndex;
		this->FirepowerMult = pFirer->FirepowerMultiplier;

		this->FirepowerMult *= TechnoExtContainer::Instance.Find(pFirer)->Get_AEProperties()->FirepowerMultiplier;

		if (pType->MirrorCoord && pFirer->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->LeadTimeCalculate || !flag_cast_to<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire();
	else
		this->WaitOneFrame = 2;
}

bool StraightTrajectoryVarianC::OnAI()
{
	auto const pType = this->GetTrajectoryType();

	if (this->WaitOneFrame && this->BulletPrepareCheck())
		return false;

	const auto pOwner = this->AttachedTo->Owner ? this->AttachedTo->Owner->Owner : BulletExtContainer::Instance.Find(this->AttachedTo)->Owner;

	if (this->BulletDetonatePreCheck())
		return true;

	this->BulletDetonateVelocityCheck(pOwner);

	if (pType->PassDetonate)
		this->PassWithDetonateAt(pOwner);

	if (this->ProximityImpact != 0 && pType->ProximityRadius.Get() > 0)
		this->PrepareForDetonateAt(pOwner);

	if (this->TrajectorySpeed < 256.0 && pType->ConfineAtHeight > 0 && this->PassAndConfineAtHeight())
		return true;

	this->BulletDetonateLastCheck(pOwner);

	return false;
}

void StraightTrajectoryVarianC::OnAIPreDetonate()
{
	const auto pType = this->GetTrajectoryType();
	const auto pBullet = this->AttachedTo;
	const auto pTechno = flag_cast_to<TechnoClass*>(pBullet->Target);
	pBullet->Health = this->GetTheTrueDamage(pBullet->Health, pTechno, true);

	if (pType->PassDetonateLocal)
	{
		CoordStruct detonateCoords = pBullet->Location;
		detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);
		pBullet->SetLocation(detonateCoords);
	}

	const auto targetSnapDistance = pType->TargetSnapDistance.Get();

	if (pType->PassThrough || targetSnapDistance <= 0)
		return;

	const auto pTarget = flag_cast_to <ObjectClass*>(pBullet->Target);
	const auto coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (coords.DistanceFrom(pBullet->Location) <= targetSnapDistance)
	{
		const auto pExt = BulletExtContainer::Instance.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}
}

void StraightTrajectoryVarianC::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(this->AttachedTo->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectoryVarianC::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void StraightTrajectoryVarianC::PrepareForOpenFire()
{
	const auto pType = this->GetTrajectoryType();
	BulletClass* pBullet = this->AttachedTo;
	double rotateAngle = 0.0;
	const auto pTarget = pBullet->Target;
	auto theTargetCoords = pBullet->TargetCoords;
	auto theSourceCoords = pBullet->SourceCoords;

	// TODO If I could calculate this before firing, perhaps it can solve the problem of one frame delay and not so correct turret orientation.
	if (pType->LeadTimeCalculate && pTarget)
	{
		theTargetCoords = pTarget->GetCoords();
		theSourceCoords = pBullet->Location;

		if (theTargetCoords != this->LastTargetCoord)
		{
			const auto extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
			const auto targetSourceCoord = theSourceCoords - theTargetCoords;
			const auto lastSourceCoord = theSourceCoords - this->LastTargetCoord;

			const auto theDistanceSquared = targetSourceCoord.pow();
			const auto targetSpeedSquared = extraOffsetCoord.pow();
			const auto targetSpeed = std::sqrt(targetSpeedSquared);

			const auto crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).pow();
			const auto verticalDistanceSquared = crossFactor / targetSpeedSquared;

			const auto horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
			const auto horizonDistance = std::sqrt(horizonDistanceSquared);

			const auto straightSpeedSquared = this->TrajectorySpeed * this->TrajectorySpeed;
			const auto baseFactor = straightSpeedSquared - targetSpeedSquared;
			const auto squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

			if (squareFactor > 1e-10)
			{
				const auto minusFactor = -(horizonDistance * targetSpeed);
				int travelTime = 0;

				if (Math::abs(baseFactor) < 1e-10)
				{
					travelTime = Math::abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
				}
				else
				{
					const auto travelTimeM = static_cast<int>((minusFactor - std::sqrt(squareFactor)) / baseFactor);
					const auto travelTimeP = static_cast<int>((minusFactor + std::sqrt(squareFactor)) / baseFactor);

					if (travelTimeM > 0 && travelTimeP > 0)
						travelTime = travelTimeM < travelTimeP ? travelTimeM : travelTimeP;
					else if (travelTimeM > 0)
						travelTime = travelTimeM;
					else if (travelTimeP > 0)
						travelTime = travelTimeP;

					if (targetSourceCoord.pow() < lastSourceCoord.pow())
						travelTime += 1;
					else
						travelTime += 2;
				}

				theTargetCoords += extraOffsetCoord * travelTime;
			}
		}
	}

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) // For disperse.
	{
		const auto theOwnerCoords = pBullet->Owner->GetCoords();
		rotateAngle = Math::atan2((double)(theTargetCoords.Y - theOwnerCoords.Y), (double)(theTargetCoords.X - theOwnerCoords.X));
	}
	else
	{
		rotateAngle = Math::atan2((double)(theTargetCoords.Y - theSourceCoords.Y), (double)(theTargetCoords.X - theSourceCoords.X));
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		theTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(rotateAngle) + this->OffsetCoord.Y * Math::sin(rotateAngle));
		theTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(rotateAngle) - this->OffsetCoord.Y * Math::cos(rotateAngle));
		theTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		this->SetInaccurate();
		const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
		const auto offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const auto offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatterMin.Get(Leptons(0)));
		const auto offsetMax = pTypeExt->BallisticScatterMax.isset() ? (int)(pTypeExt->BallisticScatterMax.Get()) : RulesClass::Instance()->BallisticScatter;
		const auto offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	if (pType->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->RemainingDistance += static_cast<int>(this->DetonationDistance + this->TrajectorySpeed);
		else if (this->DetonationDistance < 0)
			this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) - this->DetonationDistance + this->TrajectorySpeed);
		else
			this->RemainingDistance = INT_MAX;
	}
	else
	{
		this->RemainingDistance += static_cast<int>(theSourceCoords.DistanceFrom(theTargetCoords) + this->TrajectorySpeed);
	}

	pBullet->TargetCoords = theTargetCoords;
	pBullet->Velocity.X = static_cast<double>(theTargetCoords.X - theSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(theTargetCoords.Y - theSourceCoords.Y);

	if (pType->ConfineAtHeight > 0 && pType->PassDetonateLocal)
		pBullet->Velocity.Z = 0;
	else
		pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ());

	if (!this->UseDisperseBurst && Math::abs(pType->RotateCoord.Get()) > 1e-10 && this->CountOfBurst > 1)
	{
		const auto axis = pType->AxisOfRotation.Get();

		VelocityClass rotationAxis
		{
			axis.X * Math::cos(rotateAngle) + axis.Y * Math::sin(rotateAngle),
			axis.X * Math::sin(rotateAngle) - axis.Y * Math::cos(rotateAngle),
			static_cast<double>(axis.Z)
		};

		const auto rotationAxisLengthSquared = rotationAxis.pow();

		if (Math::abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / std::sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (this->CurrentBurst % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const auto cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}

	if (this->CalculateBulletVelocity())
		this->RemainingDistance = 0;
}

int StraightTrajectoryVarianC::GetVelocityZ()
{
	const auto pType = this->GetTrajectoryType();
	BulletClass* pBullet = this->AttachedTo;
	auto sourceCellZ = pBullet->SourceCoords.Z;
	auto targetCellZ = pBullet->TargetCoords.Z;
	auto bulletVelocityZ = static_cast<int>(targetCellZ - sourceCellZ);

	if (!pType->PassThrough)
		return bulletVelocityZ;

	if (const auto pTechno = pBullet->Owner)
	{
		const auto pCell = pTechno->GetCell();
		sourceCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			sourceCellZ += Unsorted::BridgeHeight;
	}

	if (const auto pTarget = flag_cast_to<ObjectClass*>(pBullet->Target))
	{
		const auto pCell = pTarget->GetCell();
		targetCellZ = pCell->Level * Unsorted::LevelHeight;

		if (pCell->ContainsBridge())
			targetCellZ += Unsorted::BridgeHeight;
	}

	if (sourceCellZ == targetCellZ || Math::abs(bulletVelocityZ) <= 32)
	{
		if (!this->DetonationDistance)
			return 0;

		const CoordStruct sourceCoords { pBullet->SourceCoords.X, pBullet->SourceCoords.Y, 0 };
		const CoordStruct targetCoords { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, 0 };
		const auto distanceOfTwo = sourceCoords.DistanceFrom(targetCoords);
		const auto theDistance = (this->DetonationDistance < 0) ? (distanceOfTwo - this->DetonationDistance) : this->DetonationDistance;

		if (Math::abs(theDistance) > 1e-10)
			bulletVelocityZ = static_cast<int>(bulletVelocityZ * (distanceOfTwo / theDistance));
		else
			return 0;
	}

	return bulletVelocityZ;
}

bool StraightTrajectoryVarianC::CalculateBulletVelocity()
{
	BulletClass* pBullet = this->AttachedTo;
	const auto velocityLength = pBullet->Velocity.Length();

	if (velocityLength > 1e-10)
		pBullet->Velocity *= this->Type->Trajectory_Speed / velocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectoryVarianC::BulletPrepareCheck()
{
	BulletClass* pBullet = this->AttachedTo;

	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Technos will update its location after firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya
	if (this->WaitOneFrame == 2)
	{
		if (const auto pTarget = pBullet->Target)
		{
			this->LastTargetCoord = pTarget->GetCoords();
			this->WaitOneFrame = 1;
			return true;
		}
	}

	this->WaitOneFrame = 0;
	this->PrepareForOpenFire();

	return false;
}

bool StraightTrajectoryVarianC::BulletDetonatePreCheck()
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	if (this->ExtraCheck)
		return true;

	this->RemainingDistance -= static_cast<int>(this->TrajectorySpeed);

	if (this->RemainingDistance < 0)
		return true;

	if (!pType->PassThrough && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (pType->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
		return true;

	if (const auto pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

// If there is an obstacle on the route, the bullet should need to reduce its speed so it will not penetrate the obstacle.
void StraightTrajectoryVarianC::BulletDetonateVelocityCheck(HouseClass* pOwner)
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	bool velocityCheck = false;
	double locationDistance = this->RemainingDistance;

	if (locationDistance < this->TrajectorySpeed)
		velocityCheck = true;

	const bool checkThrough = (!pType->ThroughBuilding || !pType->ThroughVehicles);
	const bool checkSubject = (pType->SubjectToGround || pBullet->Type->SubjectToWalls);

	if (this->TrajectorySpeed < 256.0) // Low speed with checkSubject was already done well.
	{
		if (checkThrough)
		{
			if (const auto pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
			{
				if (this->CheckThroughAndSubjectInCell(pCell, pOwner))
				{
					locationDistance = 0.0;
					velocityCheck = true;
				}
			}
		}
	}
	else if (checkThrough || checkSubject)
	{
		const auto theSourceCoords = pBullet->Location;
		const CoordStruct theTargetCoords
		{
			pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
			pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
			pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
		};

		const auto sourceCell = CellClass::Coord2Cell(theSourceCoords);
		const auto targetCell = CellClass::Coord2Cell(theTargetCoords);
		const auto cellDist = sourceCell - targetCell;
		const auto cellPace = CellStruct { static_cast<short>(Math::abs(cellDist.X)), static_cast<short>(Math::abs(cellDist.Y)) };

		auto largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const auto stepCoord = !largePace ? CoordStruct::Empty : (theTargetCoords - theSourceCoords) * (1.0 / largePace);
		auto curCoord = theSourceCoords;
		auto pCurCell = MapClass::Instance->GetCellAt(sourceCell);
		double cellDistance = locationDistance;

		for (size_t i = 0; i < largePace; ++i)
		{
			if (pType->SubjectToGround && (curCoord.Z + 15) < MapClass::Instance->GetCellFloorHeight(curCoord))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			if (pBullet->Type->SubjectToWalls && pCurCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->GetItem(pCurCell->OverlayTypeIndex)->Wall)
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			if (checkThrough && this->CheckThroughAndSubjectInCell(pCurCell, pOwner))
			{
				velocityCheck = true;
				cellDistance = curCoord.DistanceFrom(theSourceCoords);
				break;
			}

			curCoord += stepCoord;
			pCurCell = MapClass::Instance->GetCellAt(curCoord);
		}

		locationDistance = cellDistance;
	}

	if (velocityCheck)
	{
		this->RemainingDistance = 0;
		locationDistance += 32.0;

		if (locationDistance < this->TrajectorySpeed)
			pBullet->Velocity *= (locationDistance / this->TrajectorySpeed);
	}
}

// If the check result here is true, it only needs to be detonated in the next frame, without returning.
void StraightTrajectoryVarianC::BulletDetonateLastCheck(HouseClass* pOwner)
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	if (const auto pDetonateAt = this->ExtraCheck)
	{
		const auto position = pDetonateAt->GetCoords();
		const auto distance = position.DistanceFrom(pBullet->Location);
		const auto velocity = pBullet->Velocity.Length();

		pBullet->SetTarget(pDetonateAt);
		pBullet->TargetCoords = position;

		if (Math::abs(velocity) > 1e-10 && distance < velocity)
			pBullet->Velocity *= distance / velocity;

		if (this->ProximityImpact != 0)
		{
			const auto pWH = pType->ProximityWarhead;

			if (!pWH)
				return;

			auto damage = this->GetTheTrueDamage(this->ProximityDamage, pType->ProximityMedial ? nullptr : pDetonateAt, false);

			if (pType->ProximityDirect)
				pDetonateAt->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
			else
				WarheadTypeExtData::DetonateAt(pWH, pType->ProximityMedial ? nullptr : pDetonateAt , pType->ProximityMedial ? pBullet->Location : position, pBullet->Owner, damage , pOwner);

			this->CalculateNewDamage();
		}
	}
}

bool StraightTrajectoryVarianC::CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner)
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();
	auto pObject = pCell->GetContent();

	while (pObject)
	{
		const auto pTechno = flag_cast_to<TechnoClass*>(pObject);
		pObject = pObject->NextObject;

		if (!pTechno || (pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != flag_cast_to<TechnoClass*>(pBullet->Target)))
			continue;

		const auto technoType = pTechno->WhatAmI();

		if (technoType == AbstractType::Building)
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->InvisibleInGame)
				continue;

			if (pBuilding->IsStrange() ? !pType->ThroughVehicles : !pType->ThroughBuilding)
			{
				this->ExtraCheck = pTechno;
				return true;
			}
		}

		if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
		{
			this->ExtraCheck = pTechno;
			return true;
		}
	}

	return false;
}

void StraightTrajectoryVarianC::CalculateNewDamage()
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();
	const auto ratio = pType->CountAttenuation.Get();

	if (ratio != 1.0)
	{
		if (ratio)
		{
			if (pBullet->Health)
			{
				if (const auto newDamage = static_cast<int>(pBullet->Health * ratio))
					pBullet->Health = newDamage;
				else
					pBullet->Health = Math::signum(pBullet->Health);
			}

			if (this->ProximityDamage)
			{
				if (const auto newDamage = static_cast<int>(this->ProximityDamage * ratio))
					this->ProximityDamage = newDamage;
				else
					this->ProximityDamage = Math::signum(this->ProximityDamage);
			}

			if (this->PassDetonateDamage)
			{
				if (const auto newDamage = static_cast<int>(this->PassDetonateDamage * ratio))
					this->PassDetonateDamage = newDamage;
				else
					this->PassDetonateDamage = Math::signum(this->PassDetonateDamage);
			}
		}
		else
		{
			pBullet->Health = 0;
			this->ProximityDamage = 0;
			this->PassDetonateDamage = 0;
		}
	}
}

void StraightTrajectoryVarianC::PassWithDetonateAt(HouseClass* pOwner)
{
	if (this->PassDetonateTimer.Completed())
	{
		BulletClass* pBullet = this->AttachedTo;
		auto pType = this->GetTrajectoryType();
		const auto pWH = pType->PassDetonateWarhead;

		if (!pWH)
			return;

		this->PassDetonateTimer.Start(pType->PassDetonateDelay > 0 ? pType->PassDetonateDelay : 1);
		auto detonateCoords = pBullet->Location;

		if (pType->PassDetonateLocal)
			detonateCoords.Z = MapClass::Instance->GetCellFloorHeight(detonateCoords);

		const auto damage = this->GetTheTrueDamage(this->PassDetonateDamage, nullptr, false);
		WarheadTypeExtData::DetonateAt(pWH, detonateCoords, pBullet->Owner, damage,false, pOwner);
		this->CalculateNewDamage();
	}
}

// Select suitable targets and choose the closer targets then attack each target only once.
void StraightTrajectoryVarianC::PrepareForDetonateAt( HouseClass* pOwner)
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();
	const auto pWH = pType->ProximityWarhead;

	if (!pWH)
		return;

	// Step 1: Find valid targets on the ground within range.
	std::vector<CellClass*> recCellClass = this->GetCellsInProximityRadius();
	const size_t cellSize = recCellClass.size() * 2;
	size_t vectSize = cellSize;
	size_t thisSize = 0;

	const CoordStruct velocityCrd
	{
		static_cast<int>(pBullet->Velocity.X),
		static_cast<int>(pBullet->Velocity.Y),
		static_cast<int>(pBullet->Velocity.Z)
	};

	const auto velocitySq = velocityCrd.Length();
	std::vector<TechnoClass*> validTechnos;
	validTechnos.reserve(vectSize);
	const auto pTarget = pBullet->Target;

	for (const auto& pRecCell : recCellClass)
	{
		auto pObject = pRecCell->GetContent();

		while (pObject)
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(pObject);
			pObject = pObject->NextObject;

			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			const auto targetCrd = pTechno->GetCoords();
			const auto technoType = pTechno->WhatAmI();

			if (technoType == AbstractType::Building && static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame)
				continue;

			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			const auto distanceCrd = targetCrd - pBullet->SourceCoords;
			const auto terminalCrd = distanceCrd - velocityCrd;

			if (distanceCrd.Multiply(velocityCrd) < 0 || terminalCrd.Multiply(velocityCrd) > 0)
				continue;

			const auto distance = (velocitySq > 1e-10) ? std::sqrt(distanceCrd.CrossProduct(terminalCrd).pow() / velocitySq) : distanceCrd.Length();

			if (technoType != AbstractType::Building && distance > pType->ProximityRadius.Get())
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}

	// Step 2: Find valid targets in the air within range if necessary.
	if (pType->ProximityFlight)
	{
		const auto airTracker = &AircraftTrackerClass::Instance;
		auto pDest_fill = pBullet->Location + velocityCrd * 0.5;
		auto pFillCell = MapClass::Instance->GetCellAt(pDest_fill);
		airTracker->FillCurrentVector(pFillCell, static_cast<int>((pType->ProximityRadius.Get() + this->TrajectorySpeed / 2) / Unsorted::LeptonsPerCell));

		for (auto pTechno = airTracker->Get(); pTechno; pTechno = airTracker->Get())
		{
			if (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->Health <= 0 || pTechno->InLimbo || pTechno->IsSinking)
				continue;

			if (!pType->ProximityAllies && pOwner && pOwner->IsAlliedWith(pTechno->Owner) && pTechno != pTarget)
				continue;

			const auto distanceCrd = pTechno->GetCoords() - pBullet->Location;
			const auto terminalCrd = distanceCrd - velocityCrd;

			if (distanceCrd.Multiply(velocityCrd) < 0 || terminalCrd.Multiply(velocityCrd) > 0)
				continue;

			const auto distance = (velocitySq > 1e-10) ? std::sqrt(distanceCrd.CrossProduct(terminalCrd).pow() / velocitySq) : distanceCrd.Length();

			if (distance > pType->ProximityRadius.Get())
				continue;

			if (thisSize >= vectSize)
			{
				vectSize += cellSize;
				validTechnos.reserve(vectSize);
			}

			validTechnos.push_back(pTechno);
			thisSize += 1;
		}
	}

	// Step 3: Record each target without repetition.
	std::vector<int> casualtyChecked;
	casualtyChecked.reserve(std::max(validTechnos.size(), this->TheCasualty.size()));

	if (const auto pFirer = pBullet->Owner)
		this->TheCasualty[pFirer->UniqueID] = 20;

	for (const auto& [ID, remainTime] : this->TheCasualty)
	{
		if (remainTime > 0)
			this->TheCasualty[ID] = remainTime - 1;
		else
			casualtyChecked.push_back(ID);
	}

	for (const auto& ID : casualtyChecked)
		this->TheCasualty.erase(ID);

	std::vector<TechnoClass*> validTargets;

	for (const auto& pTechno : validTechnos)
	{
		if (!this->TheCasualty.contains(pTechno->UniqueID))
			validTargets.push_back(pTechno);

		this->TheCasualty[pTechno->UniqueID] = 20;
	}

	// Step 4: Detonate warheads in sequence based on distance.
	const auto targetsSize = validTargets.size();

	if (this->ProximityImpact > 0 && static_cast<int>(targetsSize) > this->ProximityImpact)
	{
		std::ranges::sort(validTargets, [pBullet](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
		{
			const auto distanceA = pTechnoA->GetCoords().DistanceFromSquared(pBullet->SourceCoords);
			const auto distanceB = pTechnoB->GetCoords().DistanceFromSquared(pBullet->SourceCoords);

			if (distanceA < distanceB)
				return true;

			if (distanceA > distanceB)
				return false;

			return pTechnoA->UniqueID < pTechnoB->UniqueID;
		});
	}

	for (const auto& pTechno : validTargets)
	{
		if (pTechno == this->ExtraCheck) // Not effective for the technos following it.
			break;

		if (this->ProximityImpact == 1)
		{
			this->ExtraCheck = pTechno;
			break;
		}

		const auto technoType = pTechno->WhatAmI();

		if (!pType->ThroughVehicles && (technoType == AbstractType::Unit || technoType == AbstractType::Aircraft))
			continue;

		if (technoType == AbstractType::Building && (static_cast<BuildingClass*>(pTechno)->IsStrange() ? !pType->ThroughVehicles : !pType->ThroughBuilding))
			continue;

		auto damage = this->GetTheTrueDamage(this->ProximityDamage, pType->ProximityMedial ? nullptr : pTechno, false);

		if (pType->ProximityDirect)
			pTechno->ReceiveDamage(&damage, 0, pWH, pBullet->Owner, false, false, pOwner);
		else
			WarheadTypeExtData::DetonateAt(pWH, pType->ProximityMedial ? nullptr : pTechno , pType->ProximityMedial ? pBullet->Location : pTechno->GetCoords(), pBullet->Owner, damage, pOwner);

		this->CalculateNewDamage();

		if (this->ProximityImpact > 0)
			--this->ProximityImpact;
	}
}

// A rectangular shape with a custom width from the current frame to the next frame in length.
std::vector<CellClass*> StraightTrajectoryVarianC::GetCellsInProximityRadius()
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	// Seems like the y-axis is reversed, but it's okay.
	const CoordStruct walkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
	const auto sideMult = pType->ProximityRadius.Get() / walkCoord.Length();

	const CoordStruct cor1Coord { static_cast<int>(walkCoord.Y * sideMult), static_cast<int>((-walkCoord.X) * sideMult), 0 };
	const CoordStruct cor4Coord { static_cast<int>((-walkCoord.Y) * sideMult), static_cast<int>(walkCoord.X * sideMult), 0 };
	const auto thisCell = CellClass::Coord2Cell(pBullet->Location);

	auto cor1Cell = CellClass::Coord2Cell((pBullet->Location + cor1Coord));
	auto cor4Cell = CellClass::Coord2Cell((pBullet->Location + cor4Coord));

	const auto off1Cell = cor1Cell - thisCell;
	const auto off4Cell = cor4Cell - thisCell;
	const auto nextCell = CellClass::Coord2Cell((pBullet->Location + walkCoord));

	auto cor2Cell = nextCell + off1Cell;
	auto cor3Cell = nextCell + off4Cell;

	// Arrange the vertices of the rectangle in order from bottom to top.
	int cornerIndex = 0;
	CellStruct corner[4] = { cor1Cell, cor2Cell, cor3Cell, cor4Cell };

	for (int i = 1; i < 4; ++i)
	{
		if (corner[cornerIndex].Y > corner[i].Y)
			cornerIndex = i;
	}

	cor1Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor2Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor3Cell = corner[cornerIndex];
	++cornerIndex %= 4;
	cor4Cell = corner[cornerIndex];

	std::vector<CellStruct> recCells = this->GetCellsInRectangle(cor1Cell, cor4Cell, cor2Cell, cor3Cell);
	std::vector<CellClass*> recCellClass;
	recCellClass.reserve(recCells.size());

	for (const auto& pCells : recCells)
	{
		if (CellClass* pRecCell = MapClass::Instance->TryGetCellAt(pCells))
			recCellClass.push_back(pRecCell);
	}

	return recCellClass;
}

// Record cells in the order of "draw left boundary, draw right boundary, fill middle, and move up one level".
std::vector<CellStruct> StraightTrajectoryVarianC::GetCellsInRectangle(CellStruct bottomStaCell, CellStruct leftMidCell, CellStruct rightMidCell, CellStruct topEndCell)
{
	std::vector<CellStruct> recCells;
	const auto cellNums = (Math::abs(topEndCell.Y - bottomStaCell.Y) + 1) * (Math::abs(rightMidCell.X - leftMidCell.X) + 1);
	recCells.reserve(cellNums);
	recCells.push_back(bottomStaCell);

	if (bottomStaCell == leftMidCell || bottomStaCell == rightMidCell)
	{
		auto middleCurCell = bottomStaCell;

		const auto middleTheDist = topEndCell - bottomStaCell;
		const CellStruct middleTheUnit { static_cast<short>(Math::signum(middleTheDist.X)), static_cast<short>(Math::signum(middleTheDist.Y)) };
		const CellStruct middleThePace { static_cast<short>(middleTheDist.X * middleTheUnit.X), static_cast<short>(middleTheDist.Y * middleTheUnit.Y) };
		auto mTheCurN = static_cast<float>((middleThePace.Y - middleThePace.X) / 2.0);

		while (middleCurCell != topEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= middleThePace.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
			}
			else if (mTheCurN < 0)
			{
				mTheCurN += middleThePace.Y;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
			else
			{
				mTheCurN += middleThePace.Y - middleThePace.X;
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
				middleCurCell.X -= middleTheUnit.X;
				middleCurCell.Y += middleTheUnit.Y;
				recCells.push_back(middleCurCell);
				middleCurCell.X += middleTheUnit.X;
				recCells.push_back(middleCurCell);
			}
		}
	}
	else
	{
		auto leftCurCell = bottomStaCell;
		auto rightCurCell = bottomStaCell;
		auto middleCurCell = bottomStaCell;

		bool leftNext = false;
		bool rightNext = false;
		bool leftSkip = false;
		bool rightSkip = false;
		bool leftContinue = false;
		bool rightContinue = false;

		const auto left1stDist = leftMidCell - bottomStaCell;
		const CellStruct left1stUnit { static_cast<short>(Math::signum(left1stDist.X)), static_cast<short>(Math::signum(left1stDist.Y)) };
		const CellStruct left1stPace { static_cast<short>(left1stDist.X * left1stUnit.X), static_cast<short>(left1stDist.Y * left1stUnit.Y) };
		auto left1stCurN = static_cast<float>((left1stPace.Y - left1stPace.X) / 2.0);

		const auto left2ndDist = topEndCell - leftMidCell;
		const CellStruct left2ndUnit { static_cast<short>(Math::signum(left2ndDist.X)), static_cast<short>(Math::signum(left2ndDist.Y)) };
		const CellStruct left2ndPace { static_cast<short>(left2ndDist.X * left2ndUnit.X), static_cast<short>(left2ndDist.Y * left2ndUnit.Y) };
		auto left2ndCurN = static_cast<float>((left2ndPace.Y - left2ndPace.X) / 2.0);

		const auto right1stDist = rightMidCell - bottomStaCell;
		const CellStruct right1stUnit { static_cast<short>(Math::signum(right1stDist.X)), static_cast<short>(Math::signum(right1stDist.Y)) };
		const CellStruct right1stPace { static_cast<short>(right1stDist.X * right1stUnit.X), static_cast<short>(right1stDist.Y * right1stUnit.Y) };
		auto right1stCurN = static_cast<float>((right1stPace.Y - right1stPace.X) / 2.0);

		const auto right2ndDist = topEndCell - rightMidCell;
		const CellStruct right2ndUnit { static_cast<short>(Math::signum(right2ndDist.X)), static_cast<short>(Math::signum(right2ndDist.Y)) };
		const CellStruct right2ndPace { static_cast<short>(right2ndDist.X * right2ndUnit.X), static_cast<short>(right2ndDist.Y * right2ndUnit.Y) };
		auto right2ndCurN = static_cast<float>((right2ndPace.Y - right2ndPace.X) / 2.0);

		while (leftCurCell != topEndCell || rightCurCell != topEndCell)
		{
			while (leftCurCell != topEndCell) // Left
			{
				if (!leftNext) // Bottom Left Side
				{
					if (left1stCurN > 0)
					{
						left1stCurN -= left1stPace.X;
						leftCurCell.Y += left1stUnit.Y;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
						}
						else
						{
							recCells.push_back(leftCurCell);
							break;
						}
					}
					else
					{
						left1stCurN += left1stPace.Y;
						leftCurCell.X += left1stUnit.X;

						if (leftCurCell == leftMidCell)
						{
							leftNext = true;
							leftSkip = true;
						}
					}
				}
				else // Top Left Side
				{
					if (left2ndCurN >= 0)
					{
						if (leftSkip)
						{
							leftSkip = false;
							left2ndCurN -= left2ndPace.X;
							leftCurCell.Y += left2ndUnit.Y;
						}
						else
						{
							leftContinue = true;
							break;
						}
					}
					else
					{
						left2ndCurN += left2ndPace.Y;
						leftCurCell.X += left2ndUnit.X;
					}
				}

				if (leftCurCell != rightCurCell) // Avoid double counting cells.
					recCells.push_back(leftCurCell);
			}

			while (rightCurCell != topEndCell) // Right
			{
				if (!rightNext) // Bottom Right Side
				{
					if (right1stCurN > 0)
					{
						right1stCurN -= right1stPace.X;
						rightCurCell.Y += right1stUnit.Y;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
						}
						else
						{
							recCells.push_back(rightCurCell);
							break;
						}
					}
					else
					{
						right1stCurN += right1stPace.Y;
						rightCurCell.X += right1stUnit.X;

						if (rightCurCell == rightMidCell)
						{
							rightNext = true;
							rightSkip = true;
						}
					}
				}
				else // Top Right Side
				{
					if (right2ndCurN >= 0)
					{
						if (rightSkip)
						{
							rightSkip = false;
							right2ndCurN -= right2ndPace.X;
							rightCurCell.Y += right2ndUnit.Y;
						}
						else
						{
							rightContinue = true;
							break;
						}
					}
					else
					{
						right2ndCurN += right2ndPace.Y;
						rightCurCell.X += right2ndUnit.X;
					}
				}

				if (rightCurCell != leftCurCell) // Avoid double counting cells.
					recCells.push_back(rightCurCell);
			}

			middleCurCell = leftCurCell;
			middleCurCell.X += 1;

			while (middleCurCell.X < rightCurCell.X) // Center
			{
				recCells.push_back(middleCurCell);
				middleCurCell.X += 1;
			}

			if (leftContinue) // Continue Top Left Side
			{
				leftContinue = false;
				left2ndCurN -= left2ndPace.X;
				leftCurCell.Y += left2ndUnit.Y;
				recCells.push_back(leftCurCell);
			}

			if (rightContinue) // Continue Top Right Side
			{
				rightContinue = false;
				right2ndCurN -= right2ndPace.X;
				rightCurCell.Y += right2ndUnit.Y;
				recCells.push_back(rightCurCell);
			}
		}
	}

	return recCells;
}

int StraightTrajectoryVarianC::GetTheTrueDamage(int damage, TechnoClass* pTechno, bool self)
{
	if (damage == 0)
		return 0;

	//BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	if (pType->EdgeAttenuation != 1.0)
	{
		const auto damageMultiplier = this->GetExtraDamageMultiplier(pTechno);
		const auto calculatedDamage = self ? damage * damageMultiplier : damage * this->FirepowerMult * damageMultiplier;
		const auto signal = Math::signum(calculatedDamage);
		damage = static_cast<int>(calculatedDamage);

		if (!damage && pType->EdgeAttenuation > 0.0)
			damage = signal;
	}

	return damage;
}

double StraightTrajectoryVarianC::GetExtraDamageMultiplier(TechnoClass* pTechno)
{
	double distance = 0.0;
	double damageMult = 1.0;

	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	if (pTechno)
		distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);
	else
		distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (this->AttenuationRange < static_cast<int>(distance))
		return pType->EdgeAttenuation;

	if (distance > 256.0)
		damageMult += (pType->EdgeAttenuation - 1.0) * ((distance - 256.0) / (static_cast<double>(this->AttenuationRange - 256)));

	return damageMult;
}

bool StraightTrajectoryVarianC::PassAndConfineAtHeight()
{
	BulletClass* pBullet = this->AttachedTo;
	auto pType = this->GetTrajectoryType();

	const CoordStruct futureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	if (const auto pCell = MapClass::Instance->TryGetCellAt(futureCoords))
	{
		auto checkDifference = MapClass::Instance->GetCellFloorHeight(futureCoords) - futureCoords.Z;

		if (pCell->ContainsBridge()) {

			const auto differenceOnBridge = checkDifference + Unsorted::BridgeHeight;
			if (Math::abs(differenceOnBridge) < Math::abs(checkDifference))
				checkDifference = differenceOnBridge;
		}

		if (Math::abs(checkDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			pBullet->Velocity.Z += static_cast<double>(checkDifference + pType->ConfineAtHeight);

			if (!pType->PassDetonateLocal && this->CalculateBulletVelocity())
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}