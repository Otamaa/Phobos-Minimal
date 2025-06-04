#include "BombardTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
		this->Serialize(Stm)
		;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
		const_cast<BombardTrajectoryType*>(this)->Serialize(Stm)
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
	this->FallScatter_Max.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Max");
	this->FallScatter_Min.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Min");
	this->FallScatter_Linear.Read(exINI, pSection, "Trajectory.Bombard.FallScatter.Linear");
	this->FallSpeed.Read(exINI, pSection, "Trajectory.Bombard.FallSpeed");

	if (Math::abs(this->FallSpeed.Get()) < 1e-10)
		this->FallSpeed = this->Trajectory_Speed;

	this->DetonationDistance.Read(exINI, pSection, "Trajectory.Bombard.DetonationDistance");
	this->DetonationHeight.Read(exINI, pSection, "Trajectory.Bombard.DetonationHeight");
	this->EarlyDetonation.Read(exINI, pSection, "Trajectory.Bombard.EarlyDetonation");
	this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Bombard.TargetSnapDistance");
	this->FreeFallOnTarget.Read(exINI, pSection, "Trajectory.Bombard.FreeFallOnTarget");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Bombard.LeadTimeCalculate");
	this->NoLaunch.Read(exINI, pSection, "Trajectory.Bombard.NoLaunch");
	this->TurningPointAnims.Read(exINI, pSection, "Trajectory.Bombard.TurningPointAnims");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Bombard.OffsetCoord");
	this->RotateCoord.Read(exINI, pSection, "Trajectory.Bombard.RotateCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Bombard.MirrorCoord");
	this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Bombard.UseDisperseBurst");
	this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Bombard.AxisOfRotation");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Bombard.SubjectToGround");

	return true;
}

#include <Ext/Anim/Body.h>

void BombardTrajectory::CreateRandomAnim(CoordStruct coords, TechnoClass* pTechno, HouseClass* pHouse, bool invoker, bool ownedObject)
{
	auto const pType = this->GetTrajectoryType();

	if (pType->TurningPointAnims.empty())
		return;

	auto const pAnimType = pType->TurningPointAnims[pType->TurningPointAnims.size() > 1 ?
		ScenarioClass::Instance->Random.RandomRanged(0, pType->TurningPointAnims.size() - 1) : 0];

	if (!pAnimType)
		return;

	auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);

	if (!pTechno)
		return;

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pHouse ? pHouse : pTechno->Owner, nullptr, pTechno, false, true);

	if (ownedObject)
		pAnim->SetOwnerObject(pTechno);

	if (invoker)
		FakeAnimClass::GetExtAttribute(pAnim)->Invoker = pTechno;

}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
		this->Serialize(Stm);
		;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) &&
		const_cast<BombardTrajectory*>(this)->Serialize(Stm);
}

void BombardTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->Height = pType->Height;
	this->FallPercent = pType->FallPercent - pType->FallPercentShift;
	this->OffsetCoord = pType->OffsetCoord.Get();
	this->UseDisperseBurst = pType->UseDisperseBurst;
	this->Height += pBullet->TargetCoords.Z;

	// use scaling since RandomRanged only support int
	this->FallPercent += ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(200 * pType->FallPercentShift)) / 100.0;
	this->InitialTargetCoord = pBullet->TargetCoords;
	this->LastTargetCoord = pBullet->TargetCoords;
	pBullet->Velocity = VelocityClass::Empty;

	if (WeaponTypeClass* const pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (TechnoClass* const pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->NoLaunch || !pType->LeadTimeCalculate || !flag_cast_to<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire();
	else
		this->WaitOneFrame = 2;
}

bool BombardTrajectory::OnAI()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (this->WaitOneFrame && this->BulletPrepareCheck())
		return false;

	if (this->BulletDetonatePreCheck())
		return true;

	// Extra check for trajectory falling
	auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;

	if (this->IsFalling && !pType->FreeFallOnTarget && this->BulletDetonateRemainCheck(pOwner))
		return true;

	this->BulletVelocityChange();

	return false;
}

void BombardTrajectory::OnAIPreDetonate()
{
	auto const pBullet = this->AttachedTo;
	//auto const pType = this->GetTrajectoryType();

	auto pTarget = flag_cast_to<ObjectClass*>(pBullet->Target);
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
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(this->AttachedTo->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

void BombardTrajectory::PrepareForOpenFire()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	this->CalculateTargetCoords();

	if (!pType->NoLaunch)
	{
		const CoordStruct middleLocation = this->CalculateMiddleCoords();

		pBullet->Velocity.X = static_cast<double>(middleLocation.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(middleLocation.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(middleLocation.Z - pBullet->SourceCoords.Z);
		pBullet->Velocity *= pType->Trajectory_Speed / pBullet->Velocity.Length();

		this->CalculateDisperseBurst();
	}
	else
	{
		this->IsFalling = true;
		CoordStruct middleLocation = CoordStruct::Empty;

		if (!pType->FreeFallOnTarget)
		{
			middleLocation = this->CalculateMiddleCoords();

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - middleLocation.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - middleLocation.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - middleLocation.Z);
			pBullet->Velocity *= pType->FallSpeed / pBullet->Velocity.Length();

			this->CalculateDisperseBurst();
			this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation) + pType->FallSpeed);
		}
		else
		{
			middleLocation = CoordStruct { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, static_cast<int>(this->Height) };
		}

		auto const pExt = BulletExtContainer::Instance.Find(pBullet);

		for (auto& trail : pExt->LaserTrails)
			trail.LastLocation = middleLocation;

		this->RefreshBulletLineTrail();

		pBullet->SetLocation(middleLocation);
		HouseClass* const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;
		this->CreateRandomAnim(middleLocation, pBullet->Owner, pOwner, true);
	}
}

CoordStruct BombardTrajectory::CalculateMiddleCoords()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	const double length = ScenarioClass::Instance->Random.RandomRanged(pType->FallScatter_Min.Get(), pType->FallScatter_Max.Get());
	const double vectorX = (pBullet->TargetCoords.X - pBullet->SourceCoords.X) * this->FallPercent;
	const double vectorY = (pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * this->FallPercent;
	double scatterX = 0.0;
	double scatterY = 0.0;

	if (!pType->FallScatter_Linear)
	{
		const double angel = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
		scatterX = length * Math::cos(angel);
		scatterY = length * Math::sin(angel);
	}
	else
	{
		const double vectorModule = std::sqrt(vectorX * vectorX + vectorY * vectorY);
		scatterX = vectorY / vectorModule * length;
		scatterY = -(vectorX / vectorModule * length);

		if (ScenarioClass::Instance->Random.RandomRanged(0, 1))
		{
			scatterX = -scatterX;
			scatterY = -scatterY;
		}
	}

	return CoordStruct
	{
		pBullet->SourceCoords.X + static_cast<int>(vectorX + scatterX),
		pBullet->SourceCoords.Y + static_cast<int>(vectorY + scatterY),
		static_cast<int>(this->Height)
	};
}

void BombardTrajectory::CalculateTargetCoords()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	CoordStruct theTargetCoords = pBullet->TargetCoords;
	CoordStruct theSourceCoords = pBullet->SourceCoords;

	if (pType->NoLaunch)
		theTargetCoords += this->CalculateBulletLeadTime();

	pBullet->TargetCoords = theTargetCoords;

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const CoordStruct theOwnerCoords = pBullet->Owner->GetCoords();
		this->RotateAngle = Math::atan2(double(theTargetCoords.Y - theOwnerCoords.Y), double(theTargetCoords.X - theOwnerCoords.X));
	}
	else
	{
		this->RotateAngle = Math::atan2(double(theTargetCoords.Y - theSourceCoords.Y), double(theTargetCoords.X - theSourceCoords.X));
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		pBullet->TargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(this->RotateAngle) + this->OffsetCoord.Y * Math::sin(this->RotateAngle));
		pBullet->TargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(this->RotateAngle) - this->OffsetCoord.Y * Math::cos(this->RotateAngle));
		pBullet->TargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
		const double offsetMult = 0.0004 * pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		const int offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatterMin.Get(Leptons(0)));
		const int offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		pBullet->TargetCoords = MapClass::GetRandomCoordsNear(pBullet->TargetCoords, offsetDistance, false);
	}
}

CoordStruct BombardTrajectory::CalculateBulletLeadTime()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();
	CoordStruct coords = CoordStruct::Empty;

	if (pType->LeadTimeCalculate)
	{
		if (const AbstractClass* const pTarget = pBullet->Target)
		{
			const CoordStruct theTargetCoords = pTarget->GetCoords();
			const CoordStruct theSourceCoords = pBullet->Location;

			if (theTargetCoords != this->LastTargetCoord)
			{
				int travelTime = 0;
				const CoordStruct extraOffsetCoord = theTargetCoords - this->LastTargetCoord;
				const CoordStruct targetSourceCoord = theSourceCoords - theTargetCoords;
				const CoordStruct lastSourceCoord = theSourceCoords - this->LastTargetCoord;

				if (pType->FreeFallOnTarget)
				{
					travelTime += static_cast<int>(std::sqrt(2 * (this->Height - theTargetCoords.Z) / BulletTypeExtData::GetAdjustedGravity(pBullet->Type)));
					coords += extraOffsetCoord * (travelTime + 1);
				}
				else
				{
					const double theDistanceSquared = targetSourceCoord.pow();
					const double targetSpeedSquared = extraOffsetCoord.pow();
					const double targetSpeed = std::sqrt(targetSpeedSquared);

					const double crossFactor = lastSourceCoord.CrossProduct(targetSourceCoord).pow();
					const double verticalDistanceSquared = crossFactor / targetSpeedSquared;

					const double horizonDistanceSquared = theDistanceSquared - verticalDistanceSquared;
					const double horizonDistance = std::sqrt(horizonDistanceSquared);

					const double straightSpeed = pType->FreeFallOnTarget ? pType->Trajectory_Speed : pType->FallSpeed;
					const double straightSpeedSquared = straightSpeed * straightSpeed;

					const double baseFactor = straightSpeedSquared - targetSpeedSquared;
					const double squareFactor = baseFactor * verticalDistanceSquared + straightSpeedSquared * horizonDistanceSquared;

					if (squareFactor > 1e-10)
					{
						const double minusFactor = -(horizonDistance * targetSpeed);

						if (Math::abs(baseFactor) < 1e-10)
						{
							travelTime = Math::abs(horizonDistance) > 1e-10 ? (static_cast<int>(theDistanceSquared / (2 * horizonDistance * targetSpeed)) + 1) : 0;
						}
						else
						{
							const int travelTimeM = static_cast<int>((minusFactor - std::sqrt(squareFactor)) / baseFactor);
							const int travelTimeP = static_cast<int>((minusFactor + std::sqrt(squareFactor)) / baseFactor);

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

						coords += extraOffsetCoord * travelTime;
					}
				}
			}
		}
	}

	return coords;
}

void BombardTrajectory::CalculateDisperseBurst()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (!this->UseDisperseBurst && Math::abs(pType->RotateCoord.Get()) > 1e-10 && this->CountOfBurst > 1)
	{
		const CoordStruct axis = pType->AxisOfRotation;

		VelocityClass rotationAxis
		{
			axis.X * Math::cos(this->RotateAngle) + axis.Y * Math::sin(this->RotateAngle),
			axis.X * Math::sin(this->RotateAngle) - axis.Y * Math::cos(this->RotateAngle),
			static_cast<double>(axis.Z)
		};

		const double rotationAxisLengthSquared = rotationAxis.pow();

		if (Math::abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / std::sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (this->CurrentBurst % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi * (pType->RotateCoord * ((this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / (this->IsFalling ? 90 : 180);
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord * (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / (this->IsFalling ? 90 : 180);
			}

			const double cosRotate = Math::cos(extraRotate);
			pBullet->Velocity = (pBullet->Velocity * cosRotate) + (rotationAxis * ((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) + (rotationAxis.CrossProduct(pBullet->Velocity) * Math::sin(extraRotate));
		}
	}
}

bool BombardTrajectory::BulletPrepareCheck()
{
	// The time between bullets' Unlimbo() and Update() is completely uncertain.
	// Technos will update its location after firing, which may result in inaccurate
	// target position recorded by the LastTargetCoord in Unlimbo(). Therefore, it's
	// necessary to record the position during the first Update(). - CrimRecya

	auto const pBullet = this->AttachedTo;

	if (this->WaitOneFrame == 2)
	{
		if (const AbstractClass* const pTarget = pBullet->Target)
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

bool BombardTrajectory::BulletDetonatePreCheck()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	// Close enough
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < pType->DetonationDistance.Get())
		return true;

	// Height
	if (pType->DetonationHeight >= 0)
	{
		if (pType->EarlyDetonation && (pBullet->Location.Z - pBullet->SourceCoords.Z) > pType->DetonationHeight)
			return true;
		else if (this->IsFalling && (pBullet->Location.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)
			return true;
	}

	// Ground, must be checked when free fall
	if (pType->SubjectToGround || (this->IsFalling && pType->FreeFallOnTarget))
	{
		if (MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= (pBullet->Location.Z + 15))
			return true;
	}

	return false;
}

bool BombardTrajectory::BulletDetonateRemainCheck(HouseClass* pOwner)
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	this->RemainingDistance -= static_cast<int>(pType->FallSpeed);

	if (this->RemainingDistance < 0)
		return true;

	if (this->RemainingDistance < pType->FallSpeed)
	{
		pBullet->Velocity *= this->RemainingDistance / pType->FallSpeed;
		this->RemainingDistance = 0;
	}

	return false;
}

void BombardTrajectory::BulletVelocityChange()
{
	auto const pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (!this->IsFalling)
	{
		if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
		{
			if (this->ToFalling)
			{
				this->IsFalling = true;
				const AbstractClass* const pTarget = pBullet->Target;
				CoordStruct middleLocation = CoordStruct::Empty;

				if (!pType->FreeFallOnTarget)
				{
					middleLocation = CoordStruct
					{
						static_cast<int>(pBullet->Location.X + pBullet->Velocity.X),
						static_cast<int>(pBullet->Location.Y + pBullet->Velocity.Y),
						static_cast<int>(pBullet->Location.Z + pBullet->Velocity.Z)
					};

					if (pType->LeadTimeCalculate && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime();

					pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - middleLocation.X);
					pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - middleLocation.Y);
					pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - middleLocation.Z);
					pBullet->Velocity *= pType->FallSpeed / pBullet->Velocity.Length();

					this->CalculateDisperseBurst();
					this->RemainingDistance += static_cast<int>(pBullet->TargetCoords.DistanceFrom(middleLocation) + pType->FallSpeed);
				}
				else
				{
					if (pType->LeadTimeCalculate && pTarget)
						pBullet->TargetCoords += pTarget->GetCoords() - this->InitialTargetCoord + this->CalculateBulletLeadTime();

					middleLocation = pBullet->TargetCoords;
					middleLocation.Z = pBullet->Location.Z;

					pBullet->Velocity = VelocityClass::Empty;
				}

				auto const pExt = BulletExtContainer::Instance.Find(pBullet);

				for (auto& trail : pExt->LaserTrails)
					trail.LastLocation = middleLocation;

				this->RefreshBulletLineTrail();
				pBullet->SetLocation(middleLocation);
				TechnoClass* const pTechno = pBullet->Owner;
				this->CreateRandomAnim(middleLocation, pTechno, pTechno ? pTechno->Owner : pExt->Owner, true);
			}
			else
			{
				this->ToFalling = true;
				const AbstractClass* const pTarget = pBullet->Target;

				if (pType->LeadTimeCalculate && pTarget)
					this->LastTargetCoord = pTarget->GetCoords();

				pBullet->Velocity *= Math::abs((this->Height - pBullet->Location.Z) / pBullet->Velocity.Z);
			}
		}
	}
	else if (pType->FreeFallOnTarget)
	{
		pBullet->Velocity.Z -= BulletTypeExtData::GetAdjustedGravity(pBullet->Type);
	}
}

#include <LineTrail.h>

void BombardTrajectory::RefreshBulletLineTrail()
{
	auto pBullet = this->AttachedTo;

	// remove previous line trail
	GameDelete<true, true>(std::exchange(pBullet->LineTrailer, nullptr));

	BulletTypeClass* const pType = pBullet->Type;

	// create new one if new type require it
	if (pType->UseLineTrail)
	{
		pBullet->LineTrailer = GameCreate<LineTrail>();

		if (RulesClass::Instance->LineTrailColorOverride != ColorStruct::Empty) {
			pBullet->LineTrailer->Color = RulesClass::Instance->LineTrailColorOverride;
		}
		else
		{
			pBullet->LineTrailer->Color = pType->LineTrailColor;
		}

		pBullet->LineTrailer->SetDecrement(pType->LineTrailColorDecrement);
		pBullet->LineTrailer->Owner = pBullet;
	}
}