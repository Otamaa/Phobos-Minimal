#include "StraightTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>

bool StraightVariantCTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->StraightTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	this->PassDetonate.Read(exINI, pSection, "Trajectory.Straight.PassDetonate");
	this->PassDetonateDelay.Read(exINI, pSection, "Trajectory.Straight.PassDetonateDelay");
	this->PassDetonateTimer.Read(exINI, pSection, "Trajectory.Straight.PassDetonateTimer");
	this->PassDetonateLocal.Read(exINI, pSection, "Trajectory.Straight.PassDetonateLocal");
	this->LeadTimeCalculate.Read(exINI, pSection, "Trajectory.Straight.LeadTimeCalculate");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Straight.OffsetCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Straight.MirrorCoord");
	this->ProximityImpact.Read(exINI, pSection, "Trajectory.Straight.ProximityImpact");
	this->ProximityRadius.Read(exINI, pSection, "Trajectory.Straight.ProximityRadius");
	this->ProximityAllies.Read(exINI, pSection, "Trajectory.Straight.ProximityAllies");
	this->ThroughVehicles.Read(exINI, pSection, "Trajectory.Straight.ThroughVehicles");
	this->ThroughBuilding.Read(exINI, pSection, "Trajectory.Straight.ThroughBuilding");
	this->StraightWarhead.Read(exINI, pSection, "Trajectory.Straight.Warhead");
	this->StraightDamage.Read(exINI, pSection, "Trajectory.Straight.Damage");
	this->SubjectToGround.Read(exINI, pSection, "Trajectory.Straight.SubjectToGround");
	this->ConfineAtHeight.Read(exINI, pSection, "Trajectory.Straight.ConfineAtHeight");
	this->EdgeAttenuation.Read(exINI, pSection, "Trajectory.Straight.EdgeAttenuation");

	if (this->EdgeAttenuation <= 0.0)
		this->EdgeAttenuation = 1.0;

	if (this->PassDetonateDelay <= 0)
		this->PassDetonateDelay = 1;

	this->PassDetonateTimer = this->PassDetonateTimer < this->PassDetonateDelay ?
		this->PassDetonateTimer : this->PassDetonateDelay - 1;

	if (this->ProximityRadius <= 0.0)
		this->ProximityRadius = 0.0;

	return true;
}

bool StraightVariantCTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return StraightTrajectoryType::Load(Stm, RegisterForChange) &&
		Stm
		.Process(this->PassDetonate)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateTimer)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->MirrorCoord)
		.Process(this->ProximityImpact)
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->StraightWarhead)
		.Process(this->StraightDamage)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		;

}

bool StraightVariantCTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return StraightTrajectoryType::Save(Stm) &&
		Stm
		.Process(this->PassDetonate)
		.Process(this->PassDetonateDelay)
		.Process(this->PassDetonateTimer)
		.Process(this->PassDetonateLocal)
		.Process(this->LeadTimeCalculate)
		.Process(this->OffsetCoord)
		.Process(this->MirrorCoord)
		.Process(this->ProximityImpact)
		.Process(this->ProximityRadius)
		.Process(this->ProximityAllies)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->StraightWarhead)
		.Process(this->StraightDamage)
		.Process(this->SubjectToGround)
		.Process(this->ConfineAtHeight)
		.Process(this->EdgeAttenuation)
		;

}

bool StraightTrajectoryVarianC::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return StraightTrajectory::Load(Stm, RegisterForChange) &&
		Stm
		.Process(this->PassDetonateTimer)
		.Process(this->WaitOneFrame)
		.Process(this->ProximityImpact)
		.Process(this->CheckTimesLimit)
		.Process(this->OffsetCoord)
		.Process(this->LastTargetCoord)
		.Process(this->LastCasualty)
		.Process(this->LastCasualtyTimes)

		.Process(this->FirepowerMult)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->ExtraCheck1)
		.Process(this->ExtraCheck2)
		.Process(this->ExtraCheck3)
		;
}

bool StraightTrajectoryVarianC::Save(PhobosStreamWriter& Stm) const
{
	return StraightTrajectory::Save(Stm) &&
		Stm
		.Process(this->PassDetonateTimer)
		.Process(this->WaitOneFrame)
		.Process(this->ProximityImpact)
		.Process(this->CheckTimesLimit)
		.Process(this->OffsetCoord)
		.Process(this->LastTargetCoord)
		.Process(this->LastCasualty)
		.Process(this->LastCasualtyTimes)

		.Process(this->FirepowerMult)
		.Process(this->ThroughVehicles)
		.Process(this->ThroughBuilding)
		.Process(this->ExtraCheck1)
		.Process(this->ExtraCheck2)
		.Process(this->ExtraCheck3)
		;
}

void StraightTrajectoryVarianC::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->DetonationDistance = pType->DetonationDistance;
	this->PassDetonateTimer = pType->PassDetonateTimer;
	this->OffsetCoord = pType->OffsetCoord;
	this->ProximityImpact = pType->ProximityImpact;
	this->ThroughVehicles = pType->ThroughVehicles;
	this->ThroughBuilding = pType->ThroughBuilding;

	this->LastCasualty.reserve(2);
	this->LastCasualtyTimes.reserve(2);
	this->FirepowerMult = 1.0;
	this->LastTargetCoord = pBullet->TargetCoords;

	if (!pType->LeadTimeCalculate || (pBullet->Target && pBullet->Target->WhatAmI() == AbstractType::Building))
		PrepareForOpenFire();
	else
		this->WaitOneFrame = 2;//OnAI() not always check after OnUnlimbo() immediately.


}

bool StraightTrajectoryVarianC::OnAI()
{
	auto const pType = this->GetTrajectoryType();

	if (this->WaitOneFrame > 0)
	{
		this->WaitOneFrame -= 1;
		if (this->WaitOneFrame == 0 || !this->AttachedTo->Target || (this->AttachedTo->Target && this->AttachedTo->Target->GetCoords() != this->LastTargetCoord))
		{
			PrepareForOpenFire();
			this->WaitOneFrame = 0;
		}
	}
	else
	{
		double StraightSpeed = this->GetTrajectorySpeed();
		auto const pOwner = this->AttachedTo->Owner ? this->AttachedTo->Owner->Owner : BulletExtContainer::Instance.Find(this->AttachedTo)->Owner;

		if (BulletDetonatePreCheck(pOwner))
			return true;

		if (pType->PassDetonate && pType->StraightWarhead)
			PassWithDetonateAt(pOwner);

		if (this->ProximityImpact != 0)
			PrepareForDetonateAt(pOwner, this->AttachedTo->Location);

		if (StraightSpeed > 256.0)
			BulletDetonateLastCheck(StraightSpeed);
		else
		{
			if (pType->ConfineAtHeight > 0)
				if (PassAndConfineAtHeight(StraightSpeed))
					return true;
		}
	}

	return false;
}

void StraightTrajectoryVarianC::OnAIPreDetonate()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;

	if (pType->EdgeAttenuation != 1.0 || pType->ProximityAllies != 0)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pBullet->Target);
		int Damage = static_cast<int>(pBullet->WeaponType->Damage * this->FirepowerMult * GetExtraDamageMultiplier(pTechno, pOwner, true));
		pBullet->Construct(pBullet->Type, pBullet->Target, pBullet->Owner, Damage, pBullet->WH, pBullet->Speed, pBullet->Bright);
	}

	if (pType->PassThrough || pType->TargetSnapDistance.Value <= 0)
		return;

	auto pTarget = abstract_cast<ObjectClass*>(pBullet->Target);
	auto pCoords = pTarget ? pTarget->GetCoords() : pBullet->Data.Location;

	if (pCoords.DistanceFrom(pBullet->Location) <= pType->TargetSnapDistance.Value)
	{
		auto const pExt = BulletExtContainer::Instance.Find(pBullet);
		pExt->SnappedToTarget = true;
		pBullet->SetLocation(pCoords);
	}
}

void StraightTrajectoryVarianC::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(this->AttachedTo->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType StraightTrajectoryVarianC::OnAITechnoCheck(TechnoClass* pTechno)
{
	if (pTechno)
	{
		auto const pType = this->GetTrajectoryType();
		auto pBullet = this->AttachedTo;
		auto const pOwner = pBullet->Owner ? pBullet->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;

		if (pType->ProximityAllies == 0 && pOwner->IsAlliedWith(pTechno->Owner))
			return TrajectoryCheckReturnType::SkipGameCheck;

		if (!pType->ThroughBuilding && pTechno->WhatAmI() == AbstractType::Building)
			this->ExtraCheck1 = pTechno;

		else if (!pType->ThroughVehicles && (pTechno->WhatAmI() == AbstractType::Unit || pTechno->WhatAmI() == AbstractType::Aircraft))
			this->ExtraCheck1 = pTechno;
	}

	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}

void StraightTrajectoryVarianC::PrepareForOpenFire()
{
	auto const pType = this->GetTrajectoryType();
	double StraightSpeed = this->GetTrajectorySpeed();
	auto pBullet = this->AttachedTo;

	if (StraightSpeed > 256.0) {
		this->ThroughVehicles = true;
		this->ThroughBuilding = true;
	}

	if (pBullet->Owner)
	{
		this->LastCasualty.push_back(pBullet->Owner);
		this->LastCasualtyTimes.push_back(20);
		this->FirepowerMult = pBullet->Owner->FirepowerMultiplier;

		if (pType->MirrorCoord && pBullet->Owner->CurrentBurstIndex % 2 == 1)
			this->OffsetCoord.Y = -(this->OffsetCoord.Y);
	}

	ObjectClass* pTarget = abstract_cast<ObjectClass*>(pBullet->Target);

	if (!pType->PassThrough)
	{
		if (TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			this->LastCasualty.push_back(pTargetTechno);
			this->LastCasualtyTimes.push_back(-1);

			if (this->LastCasualty[0] > this->LastCasualty[1])
			{
				TechnoClass* ChangeCasualty = this->LastCasualty[0];
				this->LastCasualty[0] = this->LastCasualty[1];
				this->LastCasualty[1] = ChangeCasualty;
				this->LastCasualtyTimes[0] = -1;
				this->LastCasualtyTimes[1] = 20;
			}
		}
	}

	CoordStruct TheTargetCoords = pBullet->TargetCoords;
	CoordStruct TheSourceCoords = pBullet->SourceCoords;

	if (pType->LeadTimeCalculate && pTarget)
	{
		TheTargetCoords = pTarget->GetCoords();
		TheSourceCoords = pBullet->Location;

		if (TheTargetCoords != this->LastTargetCoord)
		{
			CoordStruct ExtraOffsetCoord = TheTargetCoords - this->LastTargetCoord;
			CoordStruct TargetSourceCoord = TheSourceCoords - TheTargetCoords;
			CoordStruct LastSourceCoord = TheSourceCoords - this->LastTargetCoord;

			double TheDistanceSquared = TheSourceCoords.DistanceFromSquared(TheTargetCoords);

			double TargetSpeedSquared = ExtraOffsetCoord.pow();
			double TargetSpeed = std::sqrt(TargetSpeedSquared);

			double CrossFactor = LastSourceCoord.CrossProduct(TargetSourceCoord).pow();
			double VerticalDistanceSquared = CrossFactor / TargetSpeedSquared;

			double HorizonDistanceSquared = TheDistanceSquared - VerticalDistanceSquared;
			double HorizonDistance = std::sqrt(HorizonDistanceSquared);

			double StraightSpeedSquared = StraightSpeed * StraightSpeed;
			double BaseFactor = StraightSpeedSquared - TargetSpeedSquared;

			double SquareFactor = BaseFactor * VerticalDistanceSquared + StraightSpeedSquared * HorizonDistanceSquared;
			if (SquareFactor > 0)
			{
				double MinusFactor = -(HorizonDistance * TargetSpeed);
				int TravelTime = 0;

				if (BaseFactor == 0)
				{
					if (HorizonDistance != 0)
						TravelTime = static_cast<int>(TheDistanceSquared / (2 * HorizonDistance * TargetSpeed)) + 1;
				}
				else
				{
					int TravelTimeM = static_cast<int>((MinusFactor - std::sqrt(SquareFactor)) / BaseFactor);
					int TravelTimeP = static_cast<int>((MinusFactor + std::sqrt(SquareFactor)) / BaseFactor);

					if (TravelTimeM > 0 && TravelTimeP > 0)
						TravelTime = TravelTimeM < TravelTimeP ? TravelTimeM : TravelTimeP;
					else if (TravelTimeM > 0)
						TravelTime = TravelTimeM;
					else if (TravelTimeP > 0)
						TravelTime = TravelTimeP;

					if (TargetSourceCoord.pow() < LastSourceCoord.pow())
						TravelTime += 1;
					else
						TravelTime += 2;
				}

				TheTargetCoords.X += ExtraOffsetCoord.X * TravelTime;
				TheTargetCoords.Y += ExtraOffsetCoord.Y * TravelTime;
				TheTargetCoords.Z += ExtraOffsetCoord.Z * TravelTime;
			}
		}
	}

	if (this->OffsetCoord.X != 0 || this->OffsetCoord.Y != 0 || this->OffsetCoord.Z != 0)
	{
		double RotateAngle = Math::atan2(float(TheTargetCoords.Y - TheSourceCoords.Y), float(TheTargetCoords.X - TheSourceCoords.X));
		TheTargetCoords.X += static_cast<int>(this->OffsetCoord.X * Math::cos(RotateAngle) + this->OffsetCoord.Y * Math::sin(RotateAngle));
		TheTargetCoords.Y += static_cast<int>(this->OffsetCoord.X * Math::sin(RotateAngle) - this->OffsetCoord.Y * Math::cos(RotateAngle));
		TheTargetCoords.Z += this->OffsetCoord.Z;
	}

	if (pBullet->Type->Inaccurate)
	{
		const auto pTypeExt = BulletTypeExtContainer::Instance.Find(this->AttachedTo->Type);
		const int ScatterMax = pTypeExt->BallisticScatterMax.isset() ? (int)(pTypeExt->BallisticScatterMax.Get()) : RulesClass::Instance()->BallisticScatter;
		const int ScatterMin = pTypeExt->BallisticScatterMin.isset() ? (int)(pTypeExt->BallisticScatterMin.Get()) : (ScatterMax / 2);
		double OffsetMult = 0.0004 * TheSourceCoords.DistanceFrom(TheTargetCoords);

		int OffsetMin = static_cast<int>(OffsetMult * ScatterMin);
		int OffsetMax = static_cast<int>(OffsetMult * ScatterMax);

		int OffsetDistance = ScenarioClass::Instance->Random.RandomRanged(OffsetMin, OffsetMax);
		TheTargetCoords = MapClass::GetRandomCoordsNear(TheTargetCoords, OffsetDistance, false);
	}

	double DistanceOfTwo = TheSourceCoords.DistanceFrom(TheTargetCoords);

	if (pType->PassThrough)
	{
		if (this->DetonationDistance > 0)
			this->CheckTimesLimit = static_cast<int>(this->DetonationDistance / StraightSpeed) + 1;
		else if (this->DetonationDistance < 0)
			this->CheckTimesLimit = static_cast<int>((DistanceOfTwo - this->DetonationDistance) / StraightSpeed) + 1;
		else
			this->CheckTimesLimit = INT_MAX;
	}
	else
		this->CheckTimesLimit = static_cast<int>(DistanceOfTwo / StraightSpeed) + 1;

	pBullet->TargetCoords = TheTargetCoords;

	pBullet->Velocity.X = static_cast<double>(TheTargetCoords.X - TheSourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(TheTargetCoords.Y - TheSourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(this->GetVelocityZ());

	if (CalculateBulletVelocity(StraightSpeed))
		this->CheckTimesLimit = 0;
}

int StraightTrajectoryVarianC::GetVelocityZ() const
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	int velocity = static_cast<int>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

	if (!pType->PassThrough)
		return velocity;

	if (pBullet->Owner && abs(pBullet->Owner->GetCoords().Z - pBullet->TargetCoords.Z) <= 32)
	{
		double DistanceOfTwo = pBullet->SourceCoords.DistanceFrom(pBullet->TargetCoords);
		double TheDistance = this->DetonationDistance;

		if (this->DetonationDistance == 0)
			return 0;

		if (this->DetonationDistance < 0)
			TheDistance = DistanceOfTwo - this->DetonationDistance;

		if (TheDistance != 0)
			velocity = static_cast<int>(velocity * (DistanceOfTwo / TheDistance));
		else
			return 0;
	}

	return velocity;
}

bool StraightTrajectoryVarianC::CalculateBulletVelocity(double StraightSpeed)
{
	double VelocityLength = this->AttachedTo->Velocity.Length();

	if (VelocityLength > 0)
		this->AttachedTo->Velocity *= StraightSpeed / VelocityLength;
	else
		return true;

	return false;
}

bool StraightTrajectoryVarianC::BulletDetonatePreCheck(HouseClass* pOwner)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	if (this->ExtraCheck1)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck1->GetCoords();

		if (this->ExtraCheck1->WhatAmI() != AbstractType::Building)
			pBullet->SetLocation(ExtraCheckCoord);

		pBullet->Target = this->ExtraCheck1;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	if (this->ExtraCheck2)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck2->GetCoords();

		pBullet->SetLocation(ExtraCheckCoord);
		pBullet->Target = this->ExtraCheck2;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	if (this->ExtraCheck3)
	{
		CoordStruct ExtraCheckCoord = this->ExtraCheck3->GetCoords();

		pBullet->SetLocation(ExtraCheckCoord);
		pBullet->Target = this->ExtraCheck3;
		pBullet->TargetCoords = ExtraCheckCoord;

		return true;
	}

	this->CheckTimesLimit -= 1;
	if (this->CheckTimesLimit < 0)
		return true;

	if (!pType->PassThrough && this->DetonationDistance > 0
		&& pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (pType->SubjectToGround && MapClass::Instance->GetCellFloorHeight(pBullet->Location) >= pBullet->Location.Z)
		return true;

	if (CellClass* pCell = MapClass::Instance->TryGetCellAt(pBullet->Location))
		return false;
	else
		return true;
}

void StraightTrajectoryVarianC::BulletDetonateLastCheck(double StraightSpeed)
{
	auto const pType = this->GetTrajectoryType();
	if (StraightSpeed < 512.0)
	{
		auto pBullet = this->AttachedTo;
		int VelocityCheck = 0;

		CoordStruct MiddleCoords
		{
			pBullet->Location.X + static_cast<int>(pBullet->Velocity.X / 2),
			pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y / 2),
			pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z / 2)
		};

		double LocationDistance = pBullet->Location.DistanceFrom(pBullet->TargetCoords);

		if (!pType->PassThrough)
		{
			if (LocationDistance < StraightSpeed)
			{
				VelocityCheck = 2;
			}
		}

		if (VelocityCheck != 2)
		{
			if (pType->SubjectToGround && MapClass::Instance->GetCellFloorHeight(MiddleCoords) >= MiddleCoords.Z)
				VelocityCheck = 1;
		}

		if (VelocityCheck == 1)
		{
			this->CheckTimesLimit = 0;
			pBullet->Velocity *= 0.5;
		}

		else if (VelocityCheck == 2)
		{
			this->CheckTimesLimit = 0;
			pBullet->Velocity *= LocationDistance / StraightSpeed;
		}
	}
}

void StraightTrajectoryVarianC::PassWithDetonateAt(HouseClass* pOwner)
{
	auto const pType = this->GetTrajectoryType();

	if (this->PassDetonateTimer == 0)
	{
		auto pBullet = this->AttachedTo;
		CoordStruct DetonateCoords = this->AttachedTo->Location;
		int Damage = pType->StraightDamage;

		if (pType->PassDetonateLocal)
			DetonateCoords.Z = MapClass::Instance->GetCellFloorHeight(DetonateCoords);

		if (pType->EdgeAttenuation != 1.0 || pType->ProximityAllies != 0)
			Damage = static_cast<int>(Damage * GetExtraDamageMultiplier(nullptr, nullptr, false));

		WarheadTypeExtData::DetonateAt(pType->StraightWarhead, DetonateCoords, pBullet->Owner, Damage, pOwner);
	}

	this->PassDetonateTimer += 1;
	if (this->PassDetonateTimer > 0)
		this->PassDetonateTimer %= pType->PassDetonateDelay;
}

void StraightTrajectoryVarianC::PrepareForDetonateAt(HouseClass* pOwner, CoordStruct Coord)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	std::vector<CellClass*> RecCellClass = GetCellsInProximityRadius();
	size_t Capacity = RecCellClass.size() * 8;//Looking for better methods
	size_t ThisSize = 0;
	Capacity = Capacity > 2000 ? 2000 : Capacity;
	std::vector<TechnoClass*> ValidTechnos;
	ValidTechnos.reserve(Capacity);
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

	for (auto const pRecCell : RecCellClass)
	{
		if (ObjectClass* pObject = pRecCell->FirstObject)
		{
			if (ThisSize >= Capacity)
				break;

			while (pObject)
			{
				auto const pTechno = abstract_cast<TechnoClass*>(pObject);
				pObject = pObject->NextObject;

				if (pTechno)
				{
					auto const TechnoType = pTechno->WhatAmI();

					if (TechnoType == AbstractType::Building)
					{
						auto const pBuilding = static_cast<BuildingClass*>(pTechno);
						if (pBuilding->Type->InvisibleInGame)
							continue;
					}

					CoordStruct TechnoCrd = pTechno->GetCoords();
					CoordStruct BulletCrd = pBullet->Location;
					CoordStruct FutureCrd { BulletCrd.X + static_cast<int>(pBullet->Velocity.X), BulletCrd.Y + static_cast<int>(pBullet->Velocity.Y), 0 };
					double Distance = abs(TechnoCrd.X - BulletCrd.X);
					if (FutureCrd.X != BulletCrd.X)
					{
						double Factor = double(FutureCrd.Y - BulletCrd.Y) / double(FutureCrd.X - BulletCrd.X);
						Distance = abs(Factor * TechnoCrd.X - TechnoCrd.Y + BulletCrd.Y - Factor * BulletCrd.X) / std::sqrt(Factor * Factor + 1);
					}

					if (Distance > pType->ProximityRadius * 256.0)
						continue;

					if (pTargetTechno && pTechno == pTargetTechno)
					{
						if (!pType->PassThrough)
							continue;

						else if (this->DetonationDistance < 0 && pType->ProximityAllies == 0 && pOwner->IsAlliedWith(pTargetTechno->Owner))
							this->ExtraCheck3 = pTargetTechno;
					}

					if (!pType->ThroughBuilding && TechnoType == AbstractType::Building)
						continue;

					if (!pType->ThroughVehicles && (TechnoType == AbstractType::Unit || TechnoType == AbstractType::Aircraft))
						continue;

					if (pType->ProximityAllies == 0 && pOwner->IsAlliedWith(pTechno->Owner))
						continue;

					if (pTechno == pBullet->Owner)
						continue;

					ValidTechnos.push_back(pTechno);
					ThisSize += 1;

					if (ThisSize >= Capacity)
						break;
				}
			}
		}
	}

	this->ExtraCheck2 = CompareThenDetonateAt(ValidTechnos, pOwner, pBullet);

}

std::vector<CellClass*> StraightTrajectoryVarianC::GetCellsInProximityRadius()
{
	std::vector<CellClass*> RecCellClass;
	auto pBullet = this->AttachedTo;
	if (pBullet->Velocity.X != 0 || pBullet->Velocity.Y != 0)
	{
		auto const pType = this->GetTrajectoryType();
		CoordStruct WalkCoord { static_cast<int>(pBullet->Velocity.X), static_cast<int>(pBullet->Velocity.Y), 0 };
		double SideMult = (pType->ProximityRadius * 256.0) / WalkCoord.Length();

		CoordStruct Cor1Coord { static_cast<int>(WalkCoord.Y * SideMult), static_cast<int>((-WalkCoord.X) * SideMult), 0 };
		CoordStruct Cor4Coord { static_cast<int>((-WalkCoord.Y) * SideMult), static_cast<int>(WalkCoord.X * SideMult), 0 };

		CellStruct ThisCell = CellClass::Coord2Cell(pBullet->Location);
		CellStruct Cor1Cell = CellClass::Coord2Cell((pBullet->Location + Cor1Coord));
		CellStruct Cor4Cell = CellClass::Coord2Cell((pBullet->Location + Cor4Coord));

		CellStruct Off1Cell = Cor1Cell - ThisCell;
		CellStruct Off4Cell = Cor4Cell - ThisCell;

		CellStruct NextCell = CellClass::Coord2Cell((pBullet->Location + WalkCoord));
		CellStruct Cor2Cell = NextCell + Off1Cell;
		CellStruct Cor3Cell = NextCell + Off4Cell;

		std::vector<CellStruct> RecCells;

		if (Cor1Cell.X > Cor2Cell.X)//Left
		{
			if (Cor1Cell.Y >= Cor2Cell.Y)//↙ and ←
				RecCells = GetCellsInRectangle(Cor3Cell, Cor2Cell, Cor4Cell, Cor1Cell);
			else//↖
				RecCells = GetCellsInRectangle(Cor4Cell, Cor3Cell, Cor1Cell, Cor2Cell);
		}

		else if (Cor1Cell.X == Cor2Cell.X)//Mid
		{
			if (Cor1Cell.Y >= Cor2Cell.Y)//↓ and Center
				RecCells = GetCellsInRectangle(Cor2Cell, Cor1Cell, Cor3Cell, Cor4Cell);
			else//↑
				RecCells = GetCellsInRectangle(Cor4Cell, Cor3Cell, Cor1Cell, Cor2Cell);
		}

		else//Right
		{
			if (Cor1Cell.Y >= Cor2Cell.Y)//↘ and →
				RecCells = GetCellsInRectangle(Cor2Cell, Cor1Cell, Cor3Cell, Cor4Cell);
			else//↗
				RecCells = GetCellsInRectangle(Cor1Cell, Cor4Cell, Cor2Cell, Cor3Cell);
		}

		RecCellClass.reserve(RecCells.size());
		for (auto const& pCells : RecCells)
		{
			if (CellClass* pRecCell = MapClass::Instance->TryGetCellAt(pCells))
				RecCellClass.push_back(pRecCell);
		}
	}

	return RecCellClass;
}

std::vector<CellStruct> StraightTrajectoryVarianC::GetCellsInRectangle(CellStruct bStaCell, CellStruct lMidCell, CellStruct rMidCell, CellStruct tEndCell)
{
	std::vector<CellStruct> RecCells;
	int CellNums = (abs(tEndCell.Y - bStaCell.Y) + 1) * (abs(rMidCell.X - lMidCell.X) + 1);
	RecCells.reserve(CellNums);
	RecCells.push_back(bStaCell);

	if (bStaCell == lMidCell || bStaCell == rMidCell)
	{
		CellStruct mCurCell = bStaCell;
		CellStruct mTheDist = tEndCell - bStaCell;
		CellStruct mTheUnit = { static_cast<short>(Math::signum(mTheDist.X)), static_cast<short>(Math::signum(mTheDist.Y)) };
		mTheDist = { static_cast<short>(mTheDist.X * mTheUnit.X), static_cast<short>(mTheDist.Y * mTheUnit.Y) };
		float mTheCurN = static_cast<float>((mTheDist.Y - mTheDist.X) / 2.0);

		while (mCurCell != tEndCell)
		{
			if (mTheCurN > 0)
			{
				mTheCurN -= mTheDist.X;
				mCurCell.Y += mTheUnit.Y;
				RecCells.push_back(mCurCell);
			}

			else if (mTheCurN < 0)
			{
				mTheCurN += mTheDist.Y;
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
			}

			else
			{
				mTheCurN += mTheDist.Y - mTheDist.X;
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
				mCurCell.X -= mTheUnit.X;
				mCurCell.Y += mTheUnit.Y;
				RecCells.push_back(mCurCell);
				mCurCell.X += mTheUnit.X;
				RecCells.push_back(mCurCell);
			}
		}
	}

	else
	{
		CellStruct lCurCell = bStaCell, rCurCell = bStaCell, mCurCell = bStaCell;
		bool lNext = false, rNext = false, lSkip = false, rSkip = false, lContinue = false, rContinue = false;

		CellStruct l1stDist = lMidCell - bStaCell;
		CellStruct l1stUnit = { static_cast<short>(Math::signum(l1stDist.X)), static_cast<short>(Math::signum(l1stDist.Y)) };
		l1stDist = { static_cast<short>(l1stDist.X * l1stUnit.X), static_cast<short>(l1stDist.Y * l1stUnit.Y) };
		float l1stCurN = static_cast<float>((l1stDist.Y - l1stDist.X) / 2.0);

		CellStruct l2ndDist = tEndCell - lMidCell;
		CellStruct l2ndUnit = { static_cast<short>(Math::signum(l2ndDist.X)), static_cast<short>(Math::signum(l2ndDist.Y)) };
		l2ndDist = { static_cast<short>(l2ndDist.X * l2ndUnit.X), static_cast<short>(l2ndDist.Y * l2ndUnit.Y) };
		float l2ndCurN = static_cast<float>((l2ndDist.Y - l2ndDist.X) / 2.0);

		CellStruct r1stDist = rMidCell - bStaCell;
		CellStruct r1stUnit = { static_cast<short>(Math::signum(r1stDist.X)), static_cast<short>(Math::signum(r1stDist.Y)) };
		r1stDist = { static_cast<short>(r1stDist.X * r1stUnit.X), static_cast<short>(r1stDist.Y * r1stUnit.Y) };
		float r1stCurN = static_cast<float>((r1stDist.Y - r1stDist.X) / 2.0);

		CellStruct r2ndDist = tEndCell - rMidCell;
		CellStruct r2ndUnit = { static_cast<short>(Math::signum(r2ndDist.X)), static_cast<short>(Math::signum(r2ndDist.Y)) };
		r2ndDist = { static_cast<short>(r2ndDist.X * r2ndUnit.X), static_cast<short>(r2ndDist.Y * r2ndUnit.Y) };
		float r2ndCurN = static_cast<float>((r2ndDist.Y - r2ndDist.X) / 2.0);

		while (lCurCell != tEndCell || rCurCell != tEndCell)
		{
			while (lCurCell != tEndCell)//Left
			{
				if (!lNext)//Bottom Left Side
				{
					if (l1stCurN > 0)
					{
						l1stCurN -= l1stDist.X;
						lCurCell.Y += l1stUnit.Y;

						if (lCurCell == lMidCell)
							lNext = true;
						else
						{
							RecCells.push_back(lCurCell);
							break;
						}
					}

					else
					{
						l1stCurN += l1stDist.Y;
						lCurCell.X += l1stUnit.X;

						if (lCurCell == lMidCell)
						{
							lNext = true;
							lSkip = true;
						}
					}
				}

				else//Top Left Side
				{
					if (l2ndCurN >= 0)
					{
						if (lSkip)
						{
							lSkip = false;
							l2ndCurN -= l2ndDist.X;
							lCurCell.Y += l2ndUnit.Y;
						}

						else
						{
							lContinue = true;
							break;
						}
					}

					else
					{
						l2ndCurN += l2ndDist.Y;
						lCurCell.X += l2ndUnit.X;
					}
				}

				RecCells.push_back(lCurCell);
			}

			while (rCurCell != tEndCell)//Right
			{
				if (!rNext)//Bottom Right Side
				{
					if (r1stCurN > 0)
					{
						r1stCurN -= r1stDist.X;
						rCurCell.Y += r1stUnit.Y;

						if (rCurCell == rMidCell)
							rNext = true;
						else
						{
							RecCells.push_back(rCurCell);
							break;
						}
					}

					else
					{
						r1stCurN += r1stDist.Y;
						rCurCell.X += r1stUnit.X;

						if (rCurCell == rMidCell)
						{
							rNext = true;
							rSkip = true;
						}
					}
				}

				else//Top Right Side
				{
					if (r2ndCurN >= 0)
					{
						if (rSkip)
						{
							rSkip = false;
							r2ndCurN -= r2ndDist.X;
							rCurCell.Y += r2ndUnit.Y;
						}
						else
						{
							rContinue = true;
							break;
						}
					}

					else
					{
						r2ndCurN += r2ndDist.Y;
						rCurCell.X += r2ndUnit.X;
					}
				}

				if (rCurCell != lCurCell)
					RecCells.push_back(rCurCell);
			}

			mCurCell = lCurCell;
			mCurCell.X += 1;
			while (mCurCell.X < rCurCell.X)//Center
			{
				RecCells.push_back(mCurCell);
				mCurCell.X += 1;
			}

			if (lContinue)//Continue Top Left Side
			{
				lContinue = false;
				l2ndCurN -= l2ndDist.X;
				lCurCell.Y += l2ndUnit.Y;
				RecCells.push_back(lCurCell);
			}

			if (rContinue)//Continue Top Right Side
			{
				rContinue = false;
				r2ndCurN -= r2ndDist.X;
				rCurCell.Y += r2ndUnit.Y;
				RecCells.push_back(rCurCell);
			}
		}
	}

	return RecCells;
}

TechnoClass* StraightTrajectoryVarianC::CompareThenDetonateAt(std::vector<TechnoClass*>& Technos,
	HouseClass* pOwner, BulletClass* pBullet)
{
	auto const pType = this->GetTrajectoryType();

	short i = 0, iMax = static_cast<short>(this->LastCasualty.size());
	short j = 0, jMax = static_cast<short>(Technos.size());
	short Capacity = iMax + jMax;
	Capacity = Capacity > 10000 ? 10000 : Capacity;

	std::vector<TechnoClass*> Casualty;
	Casualty.reserve(Capacity);
	std::vector<short> CasualtyTimes;
	CasualtyTimes.reserve(Capacity);

	TechnoClass* pThis = nullptr;
	short pThisTimes = 2;
	TechnoClass* pLast = nullptr;
	bool Check = false;
	TechnoClass* Detonate = nullptr;
	std::sort(&Technos[0], &Technos[jMax]);

	for (short k = 0; k < Capacity; k++)//Merge
	{
		if (i < iMax && j < jMax)
		{
			if (this->LastCasualty[i] < Technos[j])
			{
				Check = false;
				pThis = this->LastCasualty[i];
				pThisTimes = this->LastCasualtyTimes[i];
				if (pThisTimes > 0)
					pThisTimes -= 1;
				i += 1;
			}
			else if (this->LastCasualty[i] > Technos[j])
			{
				Check = true;
				pThis = Technos[j];
				pThisTimes = 20;
				j += 1;
			}
			else
			{
				Check = false;
				pThis = this->LastCasualty[i];
				pThisTimes = this->LastCasualtyTimes[i];
				if (pThisTimes > 0)
					pThisTimes = 20;
				else
					Check = true;
				i += 1;
				j += 1;
			}
		}
		else if (i < iMax)
		{
			Check = false;
			pThis = this->LastCasualty[i];
			pThisTimes = this->LastCasualtyTimes[i];
			if (pThisTimes > 0)
				pThisTimes -= 1;
			i += 1;
		}
		else if (j < jMax)
		{
			Check = true;
			pThis = Technos[j];
			pThisTimes = 20;
			j += 1;
		}
		else
			break;

		if (pThis && pThis != pLast)
		{
			if (pThisTimes > 0)
			{
				Casualty.push_back(pThis);
				CasualtyTimes.push_back(pThisTimes);

				if (Check)
				{
					if (this->ProximityImpact != 1 && pType->StraightWarhead)
					{
						int Damage = pType->StraightDamage;
						if (pType->EdgeAttenuation != 1.0 || pType->ProximityAllies != 0)
							Damage = static_cast<int>(Damage * GetExtraDamageMultiplier(pThis, pOwner, false));

						WarheadTypeExtData::DetonateAt(pType->StraightWarhead, pThis->GetCoords(), pBullet->Owner, Damage, pOwner);
						if (this->ProximityImpact > 0)
							this->ProximityImpact -= 1;
					}
					else
					{
						Detonate = pThis;
						break;
					}
				}

				if (Casualty.size() >= 10000)
					break;
			}

			pLast = pThis;
		}
	}

	this->LastCasualty = Casualty;
	this->LastCasualtyTimes = CasualtyTimes;

	return Detonate;
}

double StraightTrajectoryVarianC::GetExtraDamageMultiplier(TechnoClass* pTechno, HouseClass* pOwner, bool Self) const
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	double Distance = 0;
	double DamageMult = 1.0;
	double AlliesMultiplier = 1.0;
	double MaxDistance = static_cast<double>(pBullet->WeaponType->Range);

	if (pTechno)
	{
		Distance = pTechno->GetCoords().DistanceFrom(pBullet->SourceCoords);
		if (pOwner && pOwner->IsAlliedWith(pTechno->Owner))
			AlliesMultiplier = pType->ProximityAllies;
	}
	else
		Distance = pBullet->Location.DistanceFrom(pBullet->SourceCoords);

	if (MaxDistance < Distance)
		return pType->EdgeAttenuation;

	if (Distance > 256.0)
		DamageMult += (pType->EdgeAttenuation - 1.0) * ((Distance - 256.0) / (MaxDistance - 256.0));

	if (!Self)
		DamageMult *= AlliesMultiplier;

	return DamageMult;
}

bool StraightTrajectoryVarianC::PassAndConfineAtHeight(double StraightSpeed)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	CoordStruct FutureCoords
	{
		pBullet->Location.X + static_cast<int>(pBullet->Velocity.X),
		pBullet->Location.Y + static_cast<int>(pBullet->Velocity.Y),
		pBullet->Location.Z + static_cast<int>(pBullet->Velocity.Z)
	};

	if (auto const pCell = MapClass::Instance->GetCellAt(FutureCoords))
	{
		int CheckDifference = MapClass::Instance->GetCellFloorHeight(FutureCoords) - FutureCoords.Z;
		CoordStruct CellCoords = pCell->GetCoordsWithBridge();
		int DifferenceOnBridge = CellCoords.Z - FutureCoords.Z;

		if (abs(DifferenceOnBridge) < abs(CheckDifference))
			CheckDifference = DifferenceOnBridge;

		if (abs(CheckDifference) < 384 || !pBullet->Type->SubjectToCliffs)
		{
			pBullet->Velocity.Z += static_cast<double>(CheckDifference + pType->ConfineAtHeight);
			if (CalculateBulletVelocity(StraightSpeed))
				return true;
		}
		else
			return true;
	}

	return false;
}