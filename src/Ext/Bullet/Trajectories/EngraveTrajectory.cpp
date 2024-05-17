#include "EngraveTrajectory.h"
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <LaserDrawClass.h>

bool EngraveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, RegisterForChange);

	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->IsSupported)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		;

	return true;
}

bool EngraveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->SourceCoord)
		.Process(this->TargetCoord)
		.Process(this->MirrorCoord)
		.Process(this->IsSupported)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->LaserInnerColor)
		.Process(this->LaserOuterColor)
		.Process(this->LaserOuterSpread)
		.Process(this->LaserThickness)
		.Process(this->LaserDuration)
		.Process(this->LaserDelay)
		.Process(this->DamageDelay)
		;

	return true;
}

bool EngraveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);
	this->SourceCoord.Read(exINI, pSection, "Trajectory.Engrave.SourceCoord");
	this->TargetCoord.Read(exINI, pSection, "Trajectory.Engrave.TargetCoord");
	this->MirrorCoord.Read(exINI, pSection, "Trajectory.Engrave.MirrorCoord");
	this->IsSupported.Read(exINI, pSection, "Trajectory.Engrave.IsSupported");
	this->IsHouseColor.Read(exINI, pSection, "Trajectory.Engrave.IsHouseColor");
	this->IsSingleColor.Read(exINI, pSection, "Trajectory.Engrave.IsSingleColor");
	this->LaserInnerColor.Read(exINI, pSection, "Trajectory.Engrave.LaserInnerColor");
	this->LaserOuterColor.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterColor");
	this->LaserOuterSpread.Read(exINI, pSection, "Trajectory.Engrave.LaserOuterSpread");
	this->LaserThickness.Read(exINI, pSection, "Trajectory.Engrave.LaserThickness");
	this->LaserDuration.Read(exINI, pSection, "Trajectory.Engrave.LaserDuration");
	this->LaserDelay.Read(exINI, pSection, "Trajectory.Engrave.LaserDelay");
	this->DamageDelay.Read(exINI, pSection, "Trajectory.Engrave.DamageDelay");

	this->LaserThickness = this->LaserThickness > 0 ? this->LaserThickness : 1;
	this->LaserDuration = this->LaserDuration > 0 ? this->LaserDuration : 1;
	this->LaserDelay = this->LaserDelay > 0 ? this->LaserDelay : 1;
	this->DamageDelay = this->DamageDelay > 0 ? this->DamageDelay : 1;

	return true;
}

bool EngraveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);
	this->Serialize(Stm);
	return true;
}

bool EngraveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);
	const_cast<EngraveTrajectory*>(this)->Serialize(Stm);
	return true;
}

void EngraveTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->SourceCoord = pType->SourceCoord;
	this->TargetCoord = pType->TargetCoord;

	this->LaserTimer = 0;
	this->DamageTimer = 0;
	this->SourceHeight = pBullet->SourceCoords.Z;
	this->SetItsLocation = false;
	this->TechnoInLimbo = pBullet->Owner ? pBullet->Owner->InLimbo : false;
	this->FirepowerMult = pBullet->Owner ? pBullet->Owner->FirepowerMultiplier : 1.0;
	this->FLHCoord = pBullet->SourceCoords;

	CoordStruct TheSourceCoords { pBullet->SourceCoords.X, pBullet->SourceCoords.Y, 0 };
	CoordStruct TheTargetCoords { pBullet->TargetCoords.X, pBullet->TargetCoords.Y, 0 };

	if (pBullet->Owner)
	{
		bool FLHFound = false;
		int WeaponIndex = 0;

		if (!this->TechnoInLimbo)
		{
			if (pBullet->WeaponType == TechnoExtData::GetCurrentWeapon(pBullet->Owner, WeaponIndex, false))
				this->FLHCoord = pBullet->Owner->GetWeapon(WeaponIndex)->FLH;
			else if (pBullet->WeaponType == TechnoExtData::GetCurrentWeapon(pBullet->Owner, WeaponIndex, true))
				this->FLHCoord = pBullet->Owner->GetWeapon(WeaponIndex)->FLH;

			auto&[FLHFound, FLH] = TechnoExtData::GetBurstFLH(pBullet->Owner, WeaponIndex);

			if (!FLHFound)
			{
				if (auto pInfantry = abstract_cast<InfantryClass*>(pBullet->Owner)){
					auto& [FLHFound_b, FLH_b] = TechnoExtData::GetInfantryFLH(pInfantry, WeaponIndex);
					FLHFound = FLHFound_b;
					FLH = FLH_b;
				}
			}

			if (FLHFound)
				this->FLHCoord = FLH;
		}
		else
		{
			if (TechnoClass* pTransporter = pBullet->Owner->Transporter)
			{
				FootClass* pCurrentPassenger = pTransporter->Passengers.GetFirstPassenger();
				FootClass* pBulletOwnerFoot = abstract_cast<FootClass*>(pBullet->Owner);

				while (pCurrentPassenger)
				{
					if (pBulletOwnerFoot != pCurrentPassenger)
					{
						WeaponIndex += 1;
						pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
					}
					else
					{
						break;
					}
				}

				if (const auto pTransporterTypeExt = TechnoTypeExtContainer::Instance.Find(pTransporter->GetTechnoType()))
				{
					if (WeaponIndex < static_cast<int>(pTransporterTypeExt->AlternateFLHs.size()))
						this->FLHCoord = pTransporterTypeExt->AlternateFLHs[WeaponIndex];
				}
			}
		}

		int BurstIndex = pBullet->Owner->CurrentBurstIndex;
		if (pType->MirrorCoord && BurstIndex % 2 == 1)
		{
			this->SourceCoord.Y = -(this->SourceCoord.Y);
			this->TargetCoord.Y = -(this->TargetCoord.Y);

			if (!this->TechnoInLimbo && !FLHFound)
				this->FLHCoord.Y = -(this->FLHCoord.Y);
		}

		TheSourceCoords = pBullet->Owner->GetCoords();
		TheSourceCoords.Z = 0;

		//Prevent incorrect explosion location when targeting buildings.
		pBullet->Target = MapClass::Instance->TryGetCellAt(TheSourceCoords);
	}

	double RotateAngle = Math::atan2(float(TheTargetCoords.Y - TheSourceCoords.Y), float(TheTargetCoords.X - TheSourceCoords.X));
	if (this->SourceCoord.X != 0 || this->SourceCoord.Y != 0)
	{
		TheSourceCoords = TheTargetCoords;
		TheSourceCoords.X += static_cast<int>(this->SourceCoord.X * Math::cos(RotateAngle) + this->SourceCoord.Y * Math::sin(RotateAngle));
		TheSourceCoords.Y += static_cast<int>(this->SourceCoord.X * Math::sin(RotateAngle) - this->SourceCoord.Y * Math::cos(RotateAngle));
	}

	TheSourceCoords.Z = GetFloorCoordHeight(TheSourceCoords);
	pBullet->SetLocation(TheSourceCoords);

	TheTargetCoords.X += static_cast<int>(this->TargetCoord.X * Math::cos(RotateAngle) + this->TargetCoord.Y * Math::sin(RotateAngle));
	TheTargetCoords.Y += static_cast<int>(this->TargetCoord.X * Math::sin(RotateAngle) - this->TargetCoord.Y * Math::cos(RotateAngle));

	pBullet->SourceCoords = TheSourceCoords;
	pBullet->TargetCoords = TheTargetCoords;

	pBullet->Velocity.X = TheTargetCoords.X - TheSourceCoords.X;
	pBullet->Velocity.Y = TheTargetCoords.Y - TheSourceCoords.Y;
	pBullet->Velocity.Z = 0;

	double StraightSpeed = this->GetTrajectorySpeed();
	StraightSpeed = StraightSpeed > 128.0 ? 128.0 : StraightSpeed;
	double CoordDistance = pBullet->Velocity.Length();

	if (CoordDistance > 0)
	{
		this->CheckTimesLimit = static_cast<int>(CoordDistance / StraightSpeed) + 1;
		pBullet->Velocity *= StraightSpeed / CoordDistance;
	}
	else
	{
		this->CheckTimesLimit = 0;
		pBullet->Velocity *= 0;
	}
}

bool EngraveTrajectory::OnAI()
{
	auto pBullet = this->AttachedTo;

	if (!pBullet->Owner)
		return true;

	auto const pType = this->GetTrajectoryType();

	if (this->TechnoInLimbo != pBullet->Owner->InLimbo)
		return true;

	this->CheckTimesLimit -= 1;
	if (this->CheckTimesLimit < 0)
	{
		return true;
	}
	else //SetLocation() seems to work wrong if I put this part into OnAIVelocity().
	{
		CoordStruct BulletCoords = pBullet->Location;

		if (this->SetItsLocation)
		{
			BulletCoords.Z = GetFloorCoordHeight(BulletCoords);
			pBullet->SetLocation(BulletCoords);
		}

		CoordStruct FutureCoords
		{
			BulletCoords.X + static_cast<int>(pBullet->Velocity.X),
			BulletCoords.Y + static_cast<int>(pBullet->Velocity.Y),
			BulletCoords.Z + static_cast<int>(pBullet->Velocity.Z)
		};

		int CheckDifference = GetFloorCoordHeight(FutureCoords) - FutureCoords.Z;

		if (abs(CheckDifference) >= 384)
		{
			if (CheckDifference > 0)
			{
				BulletCoords.Z += CheckDifference;
				pBullet->SetLocation(BulletCoords);
				this->SetItsLocation = false;
			}
			else
			{
				this->SetItsLocation = true;
			}
		}
		else
		{
			pBullet->Velocity.Z += CheckDifference;
			this->SetItsLocation = false;
		}
	}

	TechnoClass* pTechno = pBullet->Owner;
	auto const pOwner = pBullet->Owner->Owner;

	if (this->LaserTimer == 0)
	{
		LaserDrawClass* pLaser;
		CoordStruct FireCoord = pTechno->GetCoords();

		if (pTechno->WhatAmI() != AbstractType::Building)
		{
			if (this->TechnoInLimbo)
			{
				if (TechnoClass* pTransporter = pBullet->Owner->Transporter)
					FireCoord = TechnoExtData::GetFLHAbsoluteCoords(pTransporter, this->FLHCoord, pTransporter->HasTurret());
				else
					return true;
			}
			else
			{
				FireCoord = TechnoExtData::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());
			}
		}
		else //Not accurate, just got the similar FLH.
		{
			float RotateAngle = 0.0;

			if (pTechno->HasTurret())
				RotateAngle = static_cast<float>(-(pTechno->TurretFacing().GetRadian<32>()));
			else
				RotateAngle = static_cast<float>(-(pTechno->PrimaryFacing.Current().GetRadian<32>()));

			FireCoord.X += static_cast<int>(this->FLHCoord.X * Math::cos(RotateAngle) + this->FLHCoord.Y * Math::sin(RotateAngle));
			FireCoord.Y += static_cast<int>(this->FLHCoord.X * Math::sin(RotateAngle) - this->FLHCoord.Y * Math::cos(RotateAngle));

			if (const auto pBuildingType = static_cast<BuildingTypeClass*>(pTechno->GetTechnoType()))
				FireCoord.Z += this->FLHCoord.Z + 30 * (pBuildingType->GetFoundationWidth() + pBuildingType->GetFoundationHeight(false) + 2);
		}

		if (pType->IsHouseColor)
		{
			pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, pOwner->LaserColor,
				ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pType->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else if (pType->IsSingleColor)
		{
			pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, pType->LaserInnerColor,
				ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 }, pType->LaserDuration);
			pLaser->IsHouseColor = true;
		}
		else
		{
			pLaser = GameCreate<LaserDrawClass>(FireCoord, pBullet->Location, pType->LaserInnerColor,
				pType->LaserOuterColor, pType->LaserOuterSpread, pType->LaserDuration);
			pLaser->IsHouseColor = false;
		}

		pLaser->Thickness = pType->LaserThickness;
		pLaser->IsSupported = pType->IsSupported;
	}

	if (this->DamageTimer == 0)
	{
		int LaserDamage = static_cast<int>(pBullet->WeaponType->Damage * this->FirepowerMult);
		WarheadTypeExtData::DetonateAt(pBullet->WH, pBullet->Location, pTechno, LaserDamage, pOwner);
	}

	this->LaserTimer += 1;
	this->LaserTimer %= pType->LaserDelay;
	this->DamageTimer += 1;
	this->DamageTimer %= pType->DamageDelay;

	return false;
}

void EngraveTrajectory::OnAIPreDetonate() { } //do nothing

void EngraveTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	pSpeed->Z += BulletTypeExtData::GetAdjustedGravity(this->AttachedTo->Type);
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType EngraveTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

int EngraveTrajectory::GetFloorCoordHeight(CoordStruct Coord) const
{
	int Difference = 0;

	if (auto const pCell = MapClass::Instance->GetCellAt(Coord))
	{
		Difference = MapClass::Instance->GetCellFloorHeight(Coord) - this->SourceHeight;

		CoordStruct CellCoords = pCell->GetCoordsWithBridge();
		int OnBridge = CellCoords.Z - this->SourceHeight;

		if (OnBridge < 0 && abs(OnBridge - Difference) > 384)
			Difference = OnBridge;
	}

	return (this->SourceHeight + Difference);
}