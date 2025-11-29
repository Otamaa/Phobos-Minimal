#include "ParabolaTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Lib/gcem/gcem.hpp>

static COMPILETIMEEVAL std::array<const char* const, (size_t)ParabolaFireMode::count> ParabolaFireMode_ToStrings
{
	"Speed"  ,
	"Height"  ,
	"Angle" ,
	"SpeedAndHeight" ,
	"HeightAndAngle" ,
	"SpeedAndAngle" ,
};


namespace detail
{
	template <>
	OPTIONALINLINE bool read<ParabolaFireMode>(ParabolaFireMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			for (size_t i = 0; i < ParabolaFireMode_ToStrings.size(); ++i)
			{
				if (IS_SAME_STR_(ParabolaFireMode_ToStrings[i], parser.value()))
				{
					value = ParabolaFireMode(i);
					return true;
				}
			}


			Debug::INIParseFailed(pSection, pKey, parser.value(), "Parabola fire mode is invalid");
		}

		return false;
	}
}

bool ParabolaTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectoryType::Load(Stm, false)
		&&
	Stm
		.Process(this->TargetSnapDistance, false)
		.Process(this->OpenFireMode, false)
		.Process(this->ThrowHeight, false)
		.Process(this->LaunchAngle, false)
		.Process(this->LeadTimeCalculate, false)
		.Process(this->LeadTimeSimplify, false)
		.Process(this->LeadTimeMultiplier, false)
		.Process(this->DetonationAngle, false)
		.Process(this->DetonationHeight, false)
		.Process(this->BounceTimes, false)
		.Process(this->BounceOnWater, false)
		.Process(this->BounceDetonate, false)
		.Process(this->BounceAttenuation, false)
		.Process(this->BounceCoefficient, false)
		.Process(this->OffsetCoord, false)
		.Process(this->RotateCoord, false)
		.Process(this->MirrorCoord, false)
		.Process(this->UseDisperseBurst, false)
		.Process(this->AxisOfRotation, false)
		;

}

bool ParabolaTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectoryType::Save(Stm)
	&&
	Stm
		.Process(this->TargetSnapDistance)
		.Process(this->OpenFireMode)
		.Process(this->ThrowHeight)
		.Process(this->LaunchAngle)
		.Process(this->LeadTimeCalculate)
		.Process(this->LeadTimeSimplify)
		.Process(this->LeadTimeMultiplier)
		.Process(this->DetonationAngle)
		.Process(this->DetonationHeight)
		.Process(this->BounceTimes)
		.Process(this->BounceOnWater)
		.Process(this->BounceDetonate)
		.Process(this->BounceAttenuation)
		.Process(this->BounceCoefficient)
		.Process(this->OffsetCoord)
		.Process(this->RotateCoord)
		.Process(this->MirrorCoord)
		.Process(this->UseDisperseBurst)
		.Process(this->AxisOfRotation)
		;

}

bool ParabolaTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if(this->PhobosTrajectoryType::Read(pINI , pSection)){
		INI_EX exINI(pINI);
		this->DetonationDistance.Read(exINI, pSection, "Trajectory.Parabola.DetonationDistance");

		if (!this->DetonationDistance.isset())
			this->DetonationDistance = Leptons(102);

		this->TargetSnapDistance.Read(exINI, pSection, "Trajectory.Parabola.TargetSnapDistance");
		this->OpenFireMode.Read(exINI, pSection, "Trajectory.Parabola.OpenFireMode");
		this->ThrowHeight.Read(exINI, pSection, "Trajectory.Parabola.ThrowHeight");

		if (this->ThrowHeight < 0)
			this->ThrowHeight = 600;

		this->LaunchAngle.Read(exINI, pSection, "Trajectory.Parabola.LaunchAngle");
		this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeCalculate");
		this->LeadTimeSimplify.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeSimplify");
		this->LeadTimeMultiplier.Read(exINI, pSection, "Trajectory.Parabola.LeadTimeMultiplier");
		this->DetonationAngle.Read(exINI, pSection, "Trajectory.Parabola.DetonationAngle");
		this->DetonationHeight.Read(exINI, pSection, "Trajectory.Parabola.DetonationHeight");
		this->BounceTimes.Read(exINI, pSection, "Trajectory.Parabola.BounceTimes");
		this->BounceOnWater.Read(exINI, pSection, "Trajectory.Parabola.BounceOnWater");
		this->BounceDetonate.Read(exINI, pSection, "Trajectory.Parabola.BounceDetonate");
		this->BounceAttenuation.Read(exINI, pSection, "Trajectory.Parabola.BounceAttenuation");
		this->BounceCoefficient.Read(exINI, pSection, "Trajectory.Parabola.BounceCoefficient");
		this->OffsetCoord.Read(exINI, pSection, "Trajectory.Parabola.OffsetCoord");
		this->RotateCoord.Read(exINI, pSection, "Trajectory.Parabola.RotateCoord");
		this->MirrorCoord.Read(exINI, pSection, "Trajectory.Parabola.MirrorCoord");
		this->UseDisperseBurst.Read(exINI, pSection, "Trajectory.Parabola.UseDisperseBurst");
		this->AxisOfRotation.Read(exINI, pSection, "Trajectory.Parabola.AxisOfRotation");
		return true;
	}

	return false;
}

bool ParabolaTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectory::Load(Stm, false)
	&&
	Stm
		.Process(this->BounceTimes)
		.Process(this->OffsetCoord)
		.Process(this->ShouldDetonate)
		.Process(this->ShouldBounce)
		.Process(this->NeedExtraCheck)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		.Process(this->LastVelocity)
		;
}

bool ParabolaTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectory::Save(Stm)
	&&
	Stm
		.Process(this->BounceTimes)
		.Process(this->OffsetCoord)
		.Process(this->ShouldDetonate)
		.Process(this->ShouldBounce)
		.Process(this->NeedExtraCheck)
		.Process(this->LastTargetCoord)
		.Process(this->CurrentBurst)
		.Process(this->CountOfBurst)
		.Process(this->WaitOneFrame)
		.Process(this->LastVelocity)
		;
}

void ParabolaTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();
	this->BounceTimes = pType->BounceTimes;
	this->ShouldDetonate = false;
	this->ShouldBounce = false;
	this->NeedExtraCheck = false;
	this->LastTargetCoord = pBullet->TargetCoords;
	this->CurrentBurst = 0;
	this->CountOfBurst = 0;
	this->LastVelocity = VelocityClass::Empty;
	pBullet->Velocity = VelocityClass::Empty;

	if (WeaponTypeClass* const pWeapon = pBullet->WeaponType)
		this->CountOfBurst = pWeapon->Burst;

	if (TechnoClass* const pOwner = pBullet->Owner)
	{
		this->CurrentBurst = pOwner->CurrentBurstIndex;

		if (pType->MirrorCoord && pOwner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	if (!pType->LeadTimeCalculate || !flag_cast_to<FootClass*>(pBullet->Target))
		this->PrepareForOpenFire();
	else
		this->WaitOneFrame.Start(1);
}

bool ParabolaTrajectory::OnAI()
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (this->WaitOneFrame.IsTicking() && this->BulletPrepareCheck())
		return false;

	if (this->BulletDetonatePreCheck())
		return true;

	CellClass* const pCell = MapClass::Instance->TryGetCellAt(pBullet->Location);

	if (!pCell)
		return true;

	const double gravity = BulletTypeExtData::GetAdjustedGravity(pBullet->Type);

	if (this->ShouldBounce && this->BounceTimes > 0)
		return (pCell->LandType == LandType::Water && !pType->BounceOnWater)
		|| this->CalculateBulletVelocityAfterBounce(pCell, gravity);

	return this->BulletDetonateLastCheck(gravity);
}

void ParabolaTrajectory::OnAIPreDetonate()
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (pType->TargetSnapDistance.Get() <= 0)
		return;

	const ObjectClass* const pTarget = flag_cast_to<ObjectClass*>(pBullet->Target);
	const CoordStruct coords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (coords.DistanceFrom(pBullet->Location) <= pType->TargetSnapDistance.Get())
	{
		BulletExtContainer::Instance.Find(pBullet)->SnappedToTarget = true;
		pBullet->SetLocation(coords);
	}
	else
	{
		const int cellHeight = MapClass::Instance->GetCellFloorHeight(pBullet->Location);

		if (pBullet->Location.Z < cellHeight)
			pBullet->SetLocation(CoordStruct { pBullet->Location.X, pBullet->Location.Y, cellHeight });
	}
}

void ParabolaTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	BulletClass* pBullet = this->AttachedTo;
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(pBullet->Type); // Seems like this is useless
}

TrajectoryCheckReturnType ParabolaTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType();
}

TrajectoryCheckReturnType ParabolaTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType();
}

#pragma region privateFuncs

void ParabolaTrajectory::PrepareForOpenFire()
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();
	const AbstractClass* const pTarget = pBullet->Target;
	bool leadTimeCalculate = pType->LeadTimeCalculate && pTarget;
	CoordStruct theTargetCoords = leadTimeCalculate ? pTarget->GetCoords() : pBullet->TargetCoords;
	CoordStruct theSourceCoords = leadTimeCalculate ? pBullet->Location : pBullet->SourceCoords;
	leadTimeCalculate &= theTargetCoords != this->LastTargetCoord;
	double rotateAngle = 0.0;

	if (!pType->LeadTimeCalculate && theTargetCoords == theSourceCoords && pBullet->Owner) //For disperse.
	{
		const CoordStruct theOwnerCoords = pBullet->Owner->GetCoords();
		rotateAngle = std::atan2(double(theTargetCoords.Y - theOwnerCoords.Y),
								double(theTargetCoords.X - theOwnerCoords.X));
	}
	else
	{
		rotateAngle = std::atan2(double(theTargetCoords.Y - theSourceCoords.Y),
								  double(theTargetCoords.X - theSourceCoords.X));
	}

	if (this->OffsetCoord != CoordStruct::Empty)
	{
		theTargetCoords.X += static_cast<int>(this->OffsetCoord.X * std::cos(rotateAngle)
						  + this->OffsetCoord.Y * std::sin(rotateAngle));
		theTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * std::sin(rotateAngle)
						  - this->OffsetCoord.Y * std::cos(rotateAngle));
		theTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
		const double offsetMult = 0.0004 * theSourceCoords.DistanceFrom(theTargetCoords);
		const int offsetMin = static_cast<int>(offsetMult * pTypeExt->BallisticScatterMin.Get(Leptons(0)));
		const int offsetMax = static_cast<int>(offsetMult * pTypeExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter)));
		const int offsetDistance = ScenarioClass::Instance->Random.RandomRanged(offsetMin, offsetMax);
		theTargetCoords = MapClass::GetRandomCoordsNear(theTargetCoords, offsetDistance, false);
	}

	pBullet->TargetCoords = theTargetCoords;
	const double gravity = BulletTypeExtData::GetAdjustedGravity(pBullet->Type);

	if (gravity <= 1e-10)
	{
		pBullet->Velocity = VelocityClass::Empty;
		this->ShouldDetonate = true;
		return;
	}

	if (leadTimeCalculate)
		this->CalculateBulletVelocityLeadTime(&theSourceCoords, gravity);
	else
		this->CalculateBulletVelocityRightNow(&theSourceCoords, gravity);

	if (!pType->UseDisperseBurst && Math::abs(pType->RotateCoord.Get()) > 1e-10 && this->CountOfBurst > 1)
	{
		VelocityClass rotationAxis
		{
			pType->AxisOfRotation->X * std::cos(rotateAngle) + pType->AxisOfRotation->Y * std::sin(rotateAngle),
			pType->AxisOfRotation->X * std::sin(rotateAngle) - pType->AxisOfRotation->Y * std::cos(rotateAngle),
			static_cast<double>(pType->AxisOfRotation->Z)
		};

		const double rotationAxisLengthSquared = rotationAxis.pow();

		if (Math::abs(rotationAxisLengthSquared) > 1e-10)
		{
			double extraRotate = 0.0;
			rotationAxis *= 1 / std::sqrt(rotationAxisLengthSquared);

			if (pType->MirrorCoord)
			{
				if (pBullet->Owner && pBullet->Owner->CurrentBurstIndex % 2 == 1)
					rotationAxis *= -1;

				extraRotate = Math::Pi *
					(pType->RotateCoord.Get() *
					(double(this->CurrentBurst / 2) / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}
			else
			{
				extraRotate = Math::Pi * (pType->RotateCoord.Get()
							* (this->CurrentBurst / (this->CountOfBurst - 1.0) - 0.5)) / 180;
			}

			const double cosRotate = std::cos(extraRotate);
			pBullet->Velocity =
				(pBullet->Velocity * cosRotate) + (rotationAxis *
				((1 - cosRotate) * (pBullet->Velocity * rotationAxis))) +
				(rotationAxis.CrossProduct(pBullet->Velocity) * std::sin(extraRotate));
		}
	}
}

bool ParabolaTrajectory::BulletPrepareCheck()
{
	if (this->WaitOneFrame.HasTimeLeft())
		return true;

	this->PrepareForOpenFire();
	this->WaitOneFrame.Stop();

	return false;
}

void ParabolaTrajectory::CalculateBulletVelocityLeadTime(CoordStruct* pSourceCoords, double gravity)
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (pType->LeadTimeSimplify) // Only simple guess, not exact solution
	{
		int leadTime = 0;

		// Step 1: Guess the time of encounter between the projectile and the target based on known conditions
		// Directly assume that the distance between the position where the projectile hits the target and the starting point is 4 grids
		switch (pType->OpenFireMode)
		{
		case ParabolaFireMode::Height:
		case ParabolaFireMode::HeightAndAngle:
		{
			// Assuming equal height
			leadTime = static_cast<int>(std::sqrt((pType->ThrowHeight << 1) / gravity)
					* 1.25);
			break;
		}
		case ParabolaFireMode::Angle:
		{
			double radian = pType->LaunchAngle * Math::Pi / 180.0;
			radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;
			const double factor = std::cos(radian);

			// Check if the angle is appropriate
			if (Math::abs(factor) < 1e-10)
				break;

			const double mult = std::sin(2 * radian);

			// Check if the angle is appropriate again
			if (Math::abs(mult) < 1e-10)
				break;

			const double velocity = std::sqrt((Unsorted::LeptonsPerCell << 2) * gravity / mult);

			// Assuming equal height
			leadTime = static_cast<int>((Unsorted::LeptonsPerCell << 2) / (velocity * factor));
			break;
		}
		default:
		{
			// Assuming equal height
			leadTime = static_cast<int>((Unsorted::LeptonsPerCell << 2) / this->GetTrajectorySpeed());
			break;
		}
		}

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (pBullet->Target->GetCoords() - this->LastTargetCoord) * (pType->LeadTimeMultiplier * leadTime);

		// Step 3: Calculate the parabolic starting point vector
		this->CalculateBulletVelocityRightNow(pSourceCoords, gravity);
		return;
	}

	CoordStruct targetCoords = pBullet->Target->GetCoords();
	CoordStruct offsetCoords = pBullet->TargetCoords - targetCoords;

	// A coefficient that should not exist here normally, but even so, there are still errors
	const double speedFixMult = pType->LeadTimeMultiplier * 0.75;

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedHeightMeetTime(pSourceCoords, &targetCoords, &offsetCoords, gravity);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 4: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X / meetTime;
		pBullet->Velocity.Y = destinationCoords.Y / meetTime;

		// Step 5: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 6: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 7: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 2: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedAngleMeetTime(pSourceCoords, &targetCoords, &offsetCoords, radian, gravity);

		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 5: Calculate each horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X / meetTime;
		pBullet->Velocity.Y = destinationCoords.Y / meetTime;

		// Step 6: Calculate whole horizontal component of the projectile velocity
		const double horizontalDistance = Point2D { destinationCoords.X, destinationCoords.Y }.Length();
		const double horizontalVelocity = horizontalDistance / meetTime;

		// Step 7: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = horizontalVelocity * std::tan(radian) + gravity / 2;

		// Step 8: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 2: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, horizontalSpeed);

		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 5: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D { destinationCoords.X, destinationCoords.Y }.Length();
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 6: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 7: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 8: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 9: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Using Newton Iteration Method to determine the time of encounter between the projectile and the target
		const double meetTime = this->SearchFixedHeightMeetTime(pSourceCoords, &targetCoords, &offsetCoords, gravity);

		// Step 2: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 3: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 4: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = sourceHeight + destinationCoords.Z;
		const int maxHeight = destinationCoords.Z > 0 ? pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 5: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight)) + gravity / 2;

		// Step 6: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;

		// Step 7: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D { destinationCoords.X, destinationCoords.Y }.Length();
		const double mult = (pBullet->Velocity.Z / std::tan(radian)) / horizontalDistance;

		// Step 8: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 9: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 2: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, horizontalSpeed);

		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 5: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D { destinationCoords.X, destinationCoords.Y }.Length();
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 6: Calculate each horizontal component of the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;

		// Step 7: Calculate whole horizontal component of the projectile velocity
		const double horizontalVelocity = horizontalDistance * mult;

		// Step 8: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 9: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = horizontalVelocity * std::tan(radian) + gravity / 2;

		// Step 10: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 2: Calculate the time when the projectile meets the target directly using horizontal velocity
		const double meetTime = this->SolveFixedSpeedMeetTime(pSourceCoords, &targetCoords, &offsetCoords, horizontalSpeed);

		// Step 3: Substitute the time into the calculation of the attack coordinates
		pBullet->TargetCoords += (targetCoords - this->LastTargetCoord) * (speedFixMult * meetTime);
		const CoordStruct destinationCoords = pBullet->TargetCoords - *pSourceCoords;

		// Step 4: Check if it is an unsolvable solution
		if (meetTime <= 1e-10 || destinationCoords.Length() <= 1e-10)
			break;

		// Step 5: Calculate the ratio of horizontal velocity to horizontal distance
		const double horizontalDistance = Point2D { destinationCoords.X, destinationCoords.Y }.Length();
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 6: Calculate the projectile velocity
		pBullet->Velocity.X = destinationCoords.X * mult;
		pBullet->Velocity.Y = destinationCoords.Y * mult;
		pBullet->Velocity.Z = destinationCoords.Z * mult + (gravity * horizontalDistance) / (2 * horizontalSpeed) + gravity / 2;

		// Step 7: Record whether it requires additional checks during the flight
		this->CheckIfNeedExtraCheck();
		return;
	}
	}

	// Reset target position
	pBullet->TargetCoords = targetCoords + offsetCoords;

	// Substitute into the no lead time algorithm
	this->CalculateBulletVelocityRightNow(pSourceCoords, gravity);
}

void ParabolaTrajectory::CalculateBulletVelocityRightNow(CoordStruct* pSourceCoords, double gravity)
{
	BulletClass* pBullet = this->AttachedTo;

	// Calculate horizontal distance
	const CoordStruct distanceCoords = pBullet->TargetCoords - *pSourceCoords;
	const double distance = distanceCoords.Length();
	const double horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Length();

	if (distance <= 1e-10)
	{
		pBullet->Velocity = VelocityClass::Empty;
		this->ShouldDetonate = true;
		return;
	}

	auto const pType = this->GetTrajectoryType();

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ? pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Calculate the total time it takes for the projectile to meet the target using the heights of the ascending and descending phases
		const double meetTime = std::sqrt(2 * (maxHeight - sourceHeight) / gravity)
						+ std::sqrt(2 * (maxHeight - targetHeight) / gravity);

		// Step 4: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X / meetTime;
		pBullet->Velocity.Y = distanceCoords.Y / meetTime;
		break;
	}
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	{
		// Step 1: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;

		// Step 2: Using Newton Iteration Method to determine the projectile velocity
		double velocity = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? 100.0 : this->SearchVelocity(horizontalDistance, distanceCoords.Z, radian, gravity);

		// Step 3: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = velocity * std::sin(radian);

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = velocity * std::cos(radian) / horizontalDistance;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndHeight: // Fixed horizontal speed and fixed max height
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ? pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		// Step 1: Determine the maximum height that the projectile should reach
		const int sourceHeight = pSourceCoords->Z, targetHeight = pBullet->TargetCoords.Z;
		const int maxHeight = distanceCoords.Z > 0 ?
				pType->ThrowHeight + targetHeight : pType->ThrowHeight + sourceHeight;

		// Step 2: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = std::sqrt(2 * gravity * (maxHeight - sourceHeight));

		// Step 3: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= 1e-10) ? (Math::HalfPi / 3) : radian;

		// Step 4: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = (pBullet->Velocity.Z / std::tan(radian)) / horizontalDistance;

		// Step 5: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		break;
	}
	case ParabolaFireMode::SpeedAndAngle: // Fixed horizontal speed and fixed fire angle
	{
		// Step 1: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 2: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 3: Calculate the horizontal component of the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;

		// Step 4: Read the appropriate fire angle
		double radian = pType->LaunchAngle * Math::Pi / 180.0;
		radian = (radian >= Math::HalfPi || radian <= -Math::HalfPi) ? (Math::HalfPi / 3) : radian;

		// Step 5: Calculate the vertical component of the projectile velocity
		pBullet->Velocity.Z = horizontalSpeed * std::tan(radian);
		break;
	}
	default: // Fixed horizontal speed and aim at the target
	{
		// Step 1: Read the appropriate horizontal speed
		const double horizontalSpeed = this->GetTrajectorySpeed();

		// Step 2: Calculate the ratio of horizontal velocity to horizontal distance
		const double mult = horizontalDistance > 1e-10 ? horizontalSpeed / horizontalDistance : 1.0;

		// Step 3: Calculate the projectile velocity
		pBullet->Velocity.X = distanceCoords.X * mult;
		pBullet->Velocity.Y = distanceCoords.Y * mult;
		pBullet->Velocity.Z = distanceCoords.Z * mult + (gravity * horizontalDistance) / (2 * horizontalSpeed);
		break;
	}
	}

	// Record whether it requires additional checks during the flight
	this->CheckIfNeedExtraCheck();

	// Offset the gravity effect of the first time update
	pBullet->Velocity.Z += gravity / 2;
}

void ParabolaTrajectory::CheckIfNeedExtraCheck()
{
	auto const pType = this->GetTrajectoryType();

	switch (pType->OpenFireMode)
	{
	case ParabolaFireMode::Height: // Fixed max height and aim at the target
	case ParabolaFireMode::Angle: // Fixed fire angle and aim at the target
	case ParabolaFireMode::HeightAndAngle: // Fixed max height and fixed fire angle
	{
		this->NeedExtraCheck = Vector2D<double>
		{ this->AttachedTo->Velocity.X, this->AttachedTo->Velocity.Y }.pow() > 65536.0;
		break;
	}
	default: // Fixed horizontal speed and blabla
	{
		this->NeedExtraCheck = this->GetTrajectorySpeed() > 256.0;
		break;
	}
	}
}

double ParabolaTrajectory::SearchVelocity(double horizontalDistance, int distanceCoordsZ, double radian, double gravity)
{
	// Estimate initial velocity
	const double mult = std::sin(2 * radian);
	double velocity = Math::abs(mult) > 1e-10 ? std::sqrt(horizontalDistance * gravity / mult) : 0.0;
	velocity += distanceCoordsZ / gravity;
	velocity = velocity > 10.0 ? velocity : 10.0;

	// Step size
	const double delta = 1e-6;

	// Newton Iteration Method
	for (int i = 0; i < 10; ++i)
	{
		// Substitute into the estimate speed
		const double differential = this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, velocity, radian, gravity);
		const double dDifferential = (this->CheckVelocityEquation(horizontalDistance, distanceCoordsZ, (velocity + delta), radian, gravity) - differential) / delta;

		// Check unacceptable divisor
		if (Math::abs(dDifferential) < 1e-10)
			return velocity;

		// Calculate the speed of the next iteration
		const double difference = differential / dDifferential;
		const double velocityNew = velocity - difference;

		// Check tolerable error
		if (Math::abs(difference) < 8.0)
			return velocityNew;

		// Update the speed
		velocity = velocityNew;
	}

	// Unsolvable
	return 10.0;
}

double ParabolaTrajectory::CheckVelocityEquation(double horizontalDistance, int distanceCoordsZ, double velocity, double radian, double gravity)
{
	// Calculate each component of the projectile velocity
	const double horizontalVelocity = velocity * std::cos(radian);
	const double verticalVelocity = velocity * std::sin(radian);

	// Calculate the time of the rising phase
	const double upTime = verticalVelocity / gravity;

	// Calculate the maximum height that the projectile can reach
	const double maxHeight = 0.5 * verticalVelocity * upTime;

	// Calculate the time of the descent phase
	const double downTime = std::sqrt(2 * (maxHeight - distanceCoordsZ) / gravity);

	// Calculate the total time required for horizontal movement
	const double wholeTime = horizontalDistance / horizontalVelocity;

	// Calculate the difference between the total vertical motion time and the total horizontal motion time
	return wholeTime - (upTime + downTime);
}

double ParabolaTrajectory::SolveFixedSpeedMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double horizontalSpeed)
{
	// Project all conditions onto a horizontal plane
	const Point2D targetSpeedCrd { pTargetCrd->X - this->LastTargetCoord.X, pTargetCrd->Y - this->LastTargetCoord.Y };
	const Point2D destinationCrd { pTargetCrd->X + pOffsetCrd->X - pSourceCrd->X, pTargetCrd->Y + pOffsetCrd->Y - pSourceCrd->Y };

	// Establishing a quadratic equation using time as a variable:
	// (destinationCrd + targetSpeedCrd * time).Magnitude() = horizontalSpeed * time

	// Solve this quadratic equation
	const double divisor = (targetSpeedCrd.pow() - horizontalSpeed * horizontalSpeed) * 2;
	const double factor = 2 * (targetSpeedCrd * destinationCrd).pow();
	const double delta = factor * factor - 2 * divisor * destinationCrd.pow();

	if (delta >= 1e-10)
	{
		const double timeP = (-factor + std::sqrt(delta)) / divisor;
		const double timeM = (-factor - std::sqrt(delta)) / divisor;

		if (timeM > 1e-10)
			return timeM;

		return timeP;
	}

	return -1.0;
}

double ParabolaTrajectory::SearchFixedHeightMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double gravity)
{
	auto const pType = this->GetTrajectoryType();
	// Similar to method SearchVelocity, no further elaboration will be provided
	const double delta = 1e-5;
	double meetTime = (pType->ThrowHeight << 2) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const double differential = this->CheckFixedHeightEquation(pSourceCrd, pTargetCrd, pOffsetCrd, meetTime, gravity);
		const double dDifferential = (this->CheckFixedHeightEquation(pSourceCrd, pTargetCrd, pOffsetCrd, (meetTime + delta), gravity) - differential) / delta;

		if (Math::abs(dDifferential) < 1e-10)
			return meetTime;

		const double difference = differential / dDifferential;
		const double meetTimeNew = meetTime - difference;

		if (Math::abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedHeightEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double gravity)
{
	auto const pType = this->GetTrajectoryType();
	// Calculate how high the target will reach during this period of time
	const int meetHeight = static_cast<int>((pTargetCrd->Z - this->LastTargetCoord.Z) * meetTime) + pTargetCrd->Z + pOffsetCrd->Z;

	// Calculate how high the projectile can fly during this period of time
	const int maxHeight = meetHeight > pSourceCrd->Z ?
		pType->ThrowHeight + meetHeight : pType->ThrowHeight + pSourceCrd->Z;

	// Calculate the difference between these two times
	return std::sqrt((maxHeight - pSourceCrd->Z) * 2 / gravity) + std::sqrt((maxHeight - meetHeight) * 2 / gravity) - meetTime;
}

double ParabolaTrajectory::SearchFixedAngleMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double radian, double gravity)
{
	// Similar to method SearchVelocity, no further elaboration will be provided
	const double delta = 1e-5;
	double meetTime = 512 * std::sin(radian) / gravity;

	for (int i = 0; i < 10; ++i)
	{
		const double differential = this->CheckFixedAngleEquation(pSourceCrd, pTargetCrd, pOffsetCrd, meetTime, radian, gravity);
		const double dDifferential = (this->CheckFixedAngleEquation(pSourceCrd, pTargetCrd, pOffsetCrd, (meetTime + delta), radian, gravity) - differential) / delta;

		if (Math::abs(dDifferential) < 1e-10)
			return meetTime;

		const double difference = differential / dDifferential;
		const double meetTimeNew = meetTime - difference;

		if (Math::abs(difference) < 1.0)
			return meetTimeNew;

		meetTime = meetTimeNew;
	}

	return -1.0;
}

double ParabolaTrajectory::CheckFixedAngleEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double radian, double gravity)
{
	// Using the estimated time to obtain the predicted location of the target
	const CoordStruct distanceCoords = (*pTargetCrd - this->LastTargetCoord) * meetTime + *pTargetCrd + *pOffsetCrd - *pSourceCrd;

	// Calculate the horizontal distance between the target and the calculation
	const double horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Length();

	// Calculate the horizontal velocity
	const double horizontalVelocity = horizontalDistance / meetTime;

	// Calculate the vertical velocity
	const double verticalVelocity = horizontalVelocity * std::tan(radian);

	// Calculate the time of the rising phase
	const double upTime = verticalVelocity / gravity;

	// Calculate the maximum height that the projectile can reach
	const double maxHeight = 0.5 * verticalVelocity * upTime;

	// Calculate the time of the descent phase
	const double downTime = std::sqrt(2 * (maxHeight - distanceCoords.Z) / gravity);

	// Calculate the difference between the actual flight time of the projectile obtained and the initially estimated time
	return upTime + downTime - meetTime;
}

bool ParabolaTrajectory::CalculateBulletVelocityAfterBounce(CellClass* pCell, double gravity)
{
	BulletClass* pBullet = this->AttachedTo;

	--this->BounceTimes;
	this->ShouldBounce = false;
	auto const pType = this->GetTrajectoryType();

	const VelocityClass groundNormalVector = this->GetGroundNormalVector(pCell);
	pBullet->Velocity = (this->LastVelocity - groundNormalVector *
			(this->LastVelocity * groundNormalVector) * 2) * pType->BounceCoefficient;
	pBullet->Velocity.Z -= gravity;

	if (pType->BounceDetonate)
	{
		TechnoClass* const pFirer = pBullet->Owner;
		HouseClass* const pOwner = pFirer ? pFirer->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;
		WarheadTypeExtData::DetonateAt(pBullet->WH, pBullet->Location, pFirer, pBullet->Health, pOwner);
	}

	if (const int damage = pBullet->Health)
	{
		if (const int newDamage = static_cast<int>(damage * pType->BounceAttenuation))
			pBullet->Health = newDamage;
		else
			pBullet->Health = damage > 0 ? 1 : -1;
	}

	return false;
}

VelocityClass ParabolaTrajectory::GetGroundNormalVector(CellClass* pCell)
{
	BulletClass* pBullet = this->AttachedTo;

	if (const unsigned char index = pCell->SlopeIndex)
	{
		Vector2D<double> factor { 0.0, 0.0 };
		COMPILETIMEEVAL auto _base_1 = (Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell);
		COMPILETIMEEVAL auto _base_2 = (Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell);

		//0.3763770469559380854890894443664 ->
		COMPILETIMEEVAL auto _1_val = Unsorted::LevelHeight / gcem::sqrt(_base_1);
		// 0.9264665771223091335116047861327 ->
		COMPILETIMEEVAL auto _2_val = Unsorted::LeptonsPerCell / gcem::sqrt(_base_1);
		// 0.3522530794922131411764879370407 ->
		COMPILETIMEEVAL auto _3_val = Unsorted::LevelHeight / gcem::sqrt(2 * _base_1);
		// 0.8670845033654477321267395373309 ->
		COMPILETIMEEVAL auto _4_val = Unsorted::LeptonsPerCell / gcem::sqrt(2 * _base_1);
		// 0.5333964609104418418483761938761 ->
		COMPILETIMEEVAL auto _5_val = Unsorted::CellHeight / gcem::sqrt(2 * _base_2);
		// 0.6564879518897745745826168540013 ->
		COMPILETIMEEVAL auto _6_val = Unsorted::LeptonsPerCell / gcem::sqrt(2 * _base_2);

		if (index <= 4)
			factor = Vector2D<double> { _1_val, _2_val };
		else if (index <= 12)
			factor = Vector2D<double> { _3_val, _4_val };
		else
			factor = Vector2D<double> { _5_val, _6_val };

		switch (index)
		{
		case 1:
			return { -factor.X, 0.0, factor.Y };
		case 2:
			return { 0.0, -factor.X, factor.Y };
		case 3:
			return { factor.X, 0.0, factor.Y };
		case 4:
			return { 0.0, factor.X, factor.Y };
		case 5:
		case 9:
		case 13:
			return { -factor.X, -factor.X, factor.Y };
		case 6:
		case 10:
		case 14:
			return { factor.X, -factor.X, factor.Y };
		case 7:
		case 11:
		case 15:
			return { factor.X, factor.X, factor.Y };
		case 8:
		case 12:
		case 16:
			return { -factor.X, factor.X, factor.Y };
		default:
			return { 0.0, 0.0, 1.0 };
		}
	}

	// 362.1 -> Unsorted::LeptonsPerCell * sqrt(2)
	const double horizontalVelocity = Vector2D<double> { pBullet->Velocity.X, pBullet->Velocity.Y }.Length();
	const VelocityClass velocity = horizontalVelocity > 362.1 ? pBullet->Velocity * (362.1 / horizontalVelocity) : pBullet->Velocity;
	const CoordStruct velocityCoords { static_cast<int>(velocity.X), static_cast<int>(velocity.Y), static_cast<int>(velocity.Z) };

	const int cellHeight = pCell->GetCoords().Z;
	const int bulletHeight = pBullet->Location.Z;
	const int lastCellHeight = MapClass::Instance->GetCellFloorHeight(pBullet->Location - velocityCoords);

	if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
	{
		CellStruct cell = pCell->MapCoords;
		const short reverseSgnX = pBullet->Velocity.X > -(1e-10) ? -1 : 1;
		const short reverseSgnY = pBullet->Velocity.Y > -(1e-10) ? -1 : 1;
		int index = 0;

		if (this->CheckBulletHitCliff(cell.X + reverseSgnX, cell.Y, bulletHeight, lastCellHeight))
		{
			if (!this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
			{
				if (!this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y, bulletHeight, lastCellHeight))
					return { 0.0, static_cast<double>(reverseSgnY), 0.0 };

				index = 2;
			}
		}
		else
		{
			if (this->CheckBulletHitCliff(cell.X + reverseSgnX, cell.Y - reverseSgnY, bulletHeight, lastCellHeight))
			{
				if (this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 1;
				else if (!this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 2;
			}
			else
			{
				if (this->CheckBulletHitCliff(cell.X, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					return { static_cast<double>(reverseSgnX), 0.0, 0.0 };
				else if (this->CheckBulletHitCliff(cell.X - reverseSgnX, cell.Y + reverseSgnY, bulletHeight, lastCellHeight))
					index = 1;
			}
		}

		// 0.4472135954999579392818347337463 ->
		COMPILETIMEEVAL auto val_7 = (1 / gcem::sqrt(5));
		// 0.8944271909999158785636694674925 ->
		COMPILETIMEEVAL auto val_8 = (2 / gcem::sqrt(5));

		if (index == 1)
			return { val_8 * reverseSgnX, val_7 * reverseSgnY, 0.0 };
		else if (index == 2)
			return { val_7 * reverseSgnX, val_8 * reverseSgnY, 0.0 };

		// 0.7071067811865475244008443621049 ->
		COMPILETIMEEVAL auto val_9 = (1 / gcem::sqrt(2));
		return { val_9 * reverseSgnX, val_9 * reverseSgnY, 0.0 };
	}

	return { 0.0, 0.0, 1.0 };
}

bool ParabolaTrajectory::CheckBulletHitCliff(short X, short Y, int bulletHeight, int lastCellHeight)
{
	if (CellClass* const pCell = MapClass::Instance->TryGetCellAt(CellStruct { X, Y }))
	{
		const int cellHeight = pCell->GetCoords().Z;

		if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
			return true;
	}

	return false;
}

bool ParabolaTrajectory::BulletDetonatePreCheck()
{
	BulletClass* pBullet = this->AttachedTo;
	auto const pType = this->GetTrajectoryType();

	if (this->ShouldDetonate)
		return true;

	if (pType->DetonationHeight >= 0 && pBullet->Velocity.Z < 1e-10 && (pBullet->Location.Z - pBullet->SourceCoords.Z) < pType->DetonationHeight)
		return true;

	if (Math::abs(pType->DetonationAngle.Get()) < 1e-10)
	{
		if (pBullet->Velocity.Z < 1e-10)
			return true;
	}
	else if (Math::abs(pType->DetonationAngle.Get()) < 90.0)
	{
		const double horizontalVelocity = Vector2D<double> { pBullet->Velocity.X, pBullet->Velocity.Y }.Length();

		if (horizontalVelocity > 1e-10)
		{
			if ((pBullet->Velocity.Z / horizontalVelocity) < std::tan(pType->DetonationAngle.Get() * Math::Pi / 180))
				return true;
		}
		else if (pType->DetonationAngle.Get() > 1e-10 || pBullet->Velocity.Z < 1e-10)
		{
			return true;
		}
	}

	if (this->Type->DetonationDistance.Value > 0 && pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->Type->DetonationDistance.Value)
		return true;

	return false;
}

bool ParabolaTrajectory::BulletDetonateLastCheck(double gravity)
{
	BulletClass* pBullet = this->AttachedTo;
	pBullet->Velocity.Z -= gravity;

	const CoordStruct velocityCoords { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), static_cast<int>(pBullet->Velocity.Z) };
	const CoordStruct futureCoords = pBullet->Location + velocityCoords;

	if (this->NeedExtraCheck)
	{
		const CellStruct sourceCell = CellClass::Coord2Cell(pBullet->Location);
		const CellStruct targetCell = CellClass::Coord2Cell(futureCoords);
		const CellStruct cellDist = sourceCell - targetCell;
		const CellStruct cellPace = CellStruct { static_cast<short>(Math::abs(cellDist.X)), static_cast<short>(Math::abs(cellDist.Y)) };

		const size_t largePace = static_cast<size_t>(std::max(cellPace.X, cellPace.Y));
		const CoordStruct stepCoord = largePace ? velocityCoords * (1.0 / largePace) : CoordStruct::Empty;
		CoordStruct curCoord = pBullet->Location;
		CellClass* pCurCell = MapClass::Instance->GetCellAt(sourceCell);

		for (size_t i = 0; i < largePace; ++i)
		{
			const int cellHeight = MapClass::Instance->GetCellFloorHeight(curCoord);

			if (curCoord.Z < cellHeight)
			{
				this->LastVelocity = pBullet->Velocity;
				const double heightMult = Math::abs((pBullet->Location.Z - cellHeight) / pBullet->Velocity.Z);
				const double speedMult = static_cast<double>(i) / largePace;
				this->BulletDetonateEffectuate((heightMult < speedMult ? heightMult : speedMult));
				break;
			}

			if (pBullet->Type->SubjectToWalls && pCurCell->OverlayTypeIndex != -1 && OverlayTypeClass::Array->operator[](pCurCell->OverlayTypeIndex)->Wall)
			{
				pBullet->Velocity *= static_cast<double>(i) / largePace;
				this->ShouldDetonate = true;
				return false;
			}

			curCoord += stepCoord;
			pCurCell = MapClass::Instance->GetCellAt(curCoord);
		}
	}
	else
	{
		const int cellHeight = MapClass::Instance->GetCellFloorHeight(futureCoords);

		if (cellHeight < futureCoords.Z)
			return false;

		this->LastVelocity = pBullet->Velocity;
		this->BulletDetonateEffectuate(Math::abs((pBullet->Location.Z - cellHeight) / pBullet->Velocity.Z));
	}

	return false;
}

void ParabolaTrajectory::BulletDetonateEffectuate(double velocityMult)
{
	BulletClass* pBullet = this->AttachedTo;

	if (velocityMult < 1.0)
		pBullet->Velocity *= velocityMult;

	if (this->BounceTimes > 0)
		this->ShouldBounce = true;
	else
		this->ShouldDetonate = true;
}

#pragma endregion