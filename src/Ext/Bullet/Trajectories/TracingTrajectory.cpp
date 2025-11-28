#include "TracingTrajectory.h"
// #include "DisperseTrajectory.h" // TODO If merge #1295
// #include "StraightTrajectory.h" // TODO If merge #1294
// #include "EngraveTrajectory.h" // TODO If merge #1293

#include <AnimClass.h>
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <ParticleSystemClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.h>

template<typename T>
bool TracingTrajectoryType::Serialize(T& Stm)
{
	return Stm
		.Process(this->TraceMode)
		.Process(this->TheDuration)
		.Process(this->TolerantTime)
		.Process(this->ROT)
		.Process(this->BulletSpin)
		.Process(this->PeacefullyVanish)
		.Process(this->TraceTheTarget)
		.Process(this->CreateAtTarget)
		.Process(this->CreateCoord)
		.Process(this->OffsetCoord)
		.Process(this->WeaponCoord)
		.Process(this->Weapons)
		.Process(this->WeaponCount)
		.Process(this->WeaponDelay)
		.Process(this->WeaponInitialDelay)
		.Process(this->WeaponCycle)
		.Process(this->WeaponCheck)
		.Process(this->Synchronize)
		.Process(this->SuicideAboveRange)
		.Process(this->SuicideIfNoWeapon)
		.Success()
		;
}

bool TracingTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->PhobosTrajectoryType::Load(Stm, false) && this->Serialize(Stm);
}

bool TracingTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return this->PhobosTrajectoryType::Save(Stm) &&
	const_cast<TracingTrajectoryType*>(this)->Serialize(Stm);
}

bool TracingTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI(pINI);

	pINI->ReadString(pSection, "Trajectory.Tracing.TraceMode", Phobos::readDefval, Phobos::readBuffer);

	if (IS_SAME_STR_(Phobos::readBuffer, "global"))
		this->TraceMode = TraceTargetMode::Global;
	else if (IS_SAME_STR_(Phobos::readBuffer, "body"))
		this->TraceMode = TraceTargetMode::Body;
	else if (IS_SAME_STR_(Phobos::readBuffer, "turret"))
		this->TraceMode = TraceTargetMode::Turret;
	else if (IS_SAME_STR_(Phobos::readBuffer, "rotatecw"))
		this->TraceMode = TraceTargetMode::RotateCW;
	else if (IS_SAME_STR_(Phobos::readBuffer, "rotateccw"))
		this->TraceMode = TraceTargetMode::RotateCCW;
	else
		this->TraceMode = TraceTargetMode::Connection;

	this->TheDuration.Read(exINI, pSection, "Trajectory.Tracing.TheDuration");
	this->TolerantTime.Read(exINI, pSection, "Trajectory.Tracing.TolerantTime");
	this->ROT.Read(exINI, pSection, "Trajectory.Tracing.ROT");
	this->BulletSpin.Read(exINI, pSection, "Trajectory.Tracing.BulletSpin");
	this->PeacefullyVanish.Read(exINI, pSection, "Trajectory.Tracing.PeacefullyVanish");
	this->TraceTheTarget.Read(exINI, pSection, "Trajectory.Tracing.TraceTheTarget");
	this->CreateAtTarget.Read(exINI, pSection, "Trajectory.Tracing.CreateAtTarget");
	this->CreateCoord.Read(exINI, pSection, "Trajectory.Tracing.CreateCoord");
	this->OffsetCoord.Read(exINI, pSection, "Trajectory.Tracing.OffsetCoord");
	this->WeaponCoord.Read(exINI, pSection, "Trajectory.Tracing.WeaponCoord");
	this->Weapons.Read(exINI, pSection, "Trajectory.Tracing.Weapons");
	this->WeaponCount.Read(exINI, pSection, "Trajectory.Tracing.WeaponCount");
	this->WeaponDelay.Read(exINI, pSection, "Trajectory.Tracing.WeaponDelay");
	this->WeaponInitialDelay.Read(exINI, pSection, "Trajectory.Tracing.WeaponInitialDelay");
	this->WeaponInitialDelay = MaxImpl(0, this->WeaponInitialDelay);
	this->WeaponCycle.Read(exINI, pSection, "Trajectory.Tracing.WeaponCycle");
	this->WeaponCheck.Read(exINI, pSection, "Trajectory.Tracing.WeaponCheck");
	this->Synchronize.Read(exINI, pSection, "Trajectory.Tracing.Synchronize");
	this->SuicideAboveRange.Read(exINI, pSection, "Trajectory.Tracing.SuicideAboveRange");
	this->SuicideIfNoWeapon.Read(exINI, pSection, "Trajectory.Tracing.SuicideIfNoWeapon");
	return true;
}

template<typename T>
bool TracingTrajectory::Serialize(T& Stm)
{
	return Stm
		.Process(this->Type)
		.Process(this->WeaponIndex)
		.Process(this->WeaponCount)
		.Process(this->WeaponCycle)
		.Process(this->ExistTimer)
		.Process(this->WeaponTimer)
		.Process(this->TolerantTimer)
		.Process(this->TechnoInTransport)
		.Process(this->NotMainWeapon)
		.Process(this->FLHCoord)
		.Process(this->BuildingCoord)
		.Process(this->FirepowerMult)
		.Success()
		;
}

bool TracingTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) && this->Serialize(Stm);
}

bool TracingTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) && const_cast<TracingTrajectory*>(this)->Serialize(Stm);
}

void TracingTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	this->WeaponCycle = pType->WeaponCycle;

	if (!pType->Weapons.empty() && !pType->WeaponCount.empty() && this->WeaponCycle)
	{
		this->WeaponCount = pType->WeaponCount[0];
		this->WeaponTimer.Start(pType->WeaponInitialDelay);
	}

	this->FLHCoord = pBullet->SourceCoords;

	if (const auto pFirer = pBullet->Owner)
	{
		this->TechnoInTransport = static_cast<bool>(pFirer->Transporter);
		this->FirepowerMult = pFirer->FirepowerMultiplier;

		this->FirepowerMult *= TechnoExtContainer::Instance.Find(pFirer)->AE.FirepowerMultiplier;

		this->GetTechnoFLHCoord(pFirer);
	}
	else
	{
		this->TechnoInTransport = false;
		this->NotMainWeapon = true;
	}

	this->SetSourceLocation();
	this->InitializeDuration(pType->TheDuration);
}

bool TracingTrajectory::OnAI()
{
	auto pBullet = this->AttachedTo;
	const auto pTechno = pBullet->Owner;

	if (!this->NotMainWeapon && this->InvalidFireCondition(pTechno))
		return true;

	if (this->BulletDetonatePreCheck())
		return true;

	if (this->WeaponTimer.Completed() && this->CheckFireFacing() && this->PrepareTracingWeapon())
		return true;

	if (this->ExistTimer.Completed())
		return true;

	this->ChangeFacing();

	return false;
}

void TracingTrajectory::OnAIPreDetonate()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	if (pType->PeacefullyVanish)
	{
		pBullet->Health = 0;
		pBullet->Limbo();
		pBullet->UnInit();
	}
}

void TracingTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	*pSpeed = this->ChangeVelocity(); // This is for location calculation

	if (pType->ROT < 0 && !pType->BulletSpin) // This is for image drawing
		pBullet->Velocity = *pSpeed;
}

TrajectoryCheckReturnType TracingTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

TrajectoryCheckReturnType TracingTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck;
}

void TracingTrajectory::GetTechnoFLHCoord(TechnoClass* pTechno)
{
#ifdef _WTF_IS_THIS_SHIT
	const auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	if (!pExt || !pExt->LastWeaponType || pExt->LastWeaponType->Projectile != pBullet->Type)
	{
		this->NotMainWeapon = true;
		return;
	}
	else if (pTechno->WhatAmI() == AbstractType::Building)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		Matrix3D mtx;
		mtx.MakeIdentity();

		if (pTechno->HasTurret())
		{
			TechnoTypeExt::ApplyTurretOffset(pBuilding->Type, &mtx);
			mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
		}

		mtx.Translate(static_cast<float>(pExt->LastWeaponFLH.X), static_cast<float>(pExt->LastWeaponFLH.Y), static_cast<float>(pExt->LastWeaponFLH.Z));
		const auto result = mtx.GetTranslation();
		this->BuildingCoord = pBullet->SourceCoords - pBuilding->GetCoords() - CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) };
	}

	this->FLHCoord = pExt->LastWeaponFLH;
#endif
}

void TracingTrajectory::SetSourceLocation()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	CoordStruct theOffset = pType->CreateCoord;

	if (pType->CreateCoord->X != 0 || pType->CreateCoord->Y != 0)
	{
		const auto& theSource = pBullet->SourceCoords;
		const auto& theTarget = pBullet->TargetCoords;
		const auto rotateAngle = std::atan2(double(theTarget.Y - theSource.Y), double(theTarget.X - theSource.X));

		theOffset.X = static_cast<int>(pType->CreateCoord->X * std::cos(rotateAngle) + pType->CreateCoord->Y * std::sin(rotateAngle));
		theOffset.Y = static_cast<int>(pType->CreateCoord->X * std::sin(rotateAngle) - pType->CreateCoord->Y * std::cos(rotateAngle));
	}

	if (pType->CreateAtTarget)
	{
		if (const auto pTarget = pBullet->Target)
			pBullet->SetLocation(pTarget->GetCoords() + theOffset);
		else
			pBullet->SetLocation(pBullet->TargetCoords + theOffset);
	}
	else
	{
		pBullet->SetLocation(pBullet->SourceCoords + theOffset);
	}

	VelocityClass facing
	{
		static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X),
		static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y),
		0
	};

	pBullet->Velocity = facing * (1 / facing.Length());
}

inline void TracingTrajectory::InitializeDuration(int duration)
{
	auto pBullet = this->AttachedTo;
	if (duration <= 0)
	{
		if (const auto pWeapon = pBullet->WeaponType)
			duration = (pWeapon->ROF > 10) ? pWeapon->ROF - 10 : 1;
		else
			duration = 120;
	}

	this->ExistTimer.Start(duration);
}

inline bool TracingTrajectory::InvalidFireCondition(TechnoClass* pTechno) const
{
	return (!pTechno
		|| !pTechno->IsAlive
		|| (pTechno->InLimbo && !pTechno->Transporter)
		|| pTechno->IsSinking
		|| pTechno->Health <= 0
		|| this->TechnoInTransport != static_cast<bool>(pTechno->Transporter)
		|| pTechno->Deactivated
		|| pTechno->TemporalTargetingMe
		|| pTechno->BeingWarpedOut
		|| pTechno->IsUnderEMP());
}

bool TracingTrajectory::BulletDetonatePreCheck()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	if (!pBullet->Target && !pType->TolerantTime)
		return true;

	const auto pTechno = pBullet->Owner;

	if (!pType->TraceTheTarget && !pTechno)
		return true;

	if (pType->Synchronize)
	{
		if (pTechno)
		{
			if (pBullet->Target != pTechno->Target && !pType->TolerantTime)
				return true;

			pBullet->Target = pTechno->Target;
		}
	}

	if (const auto pTarget = pBullet->Target)
	{
		pBullet->TargetCoords = pTarget->GetCoords();
		this->TolerantTimer.Stop();
	}
	else if (pType->TolerantTime > 0)
	{
		if (this->TolerantTimer.Completed())
			return true;
		else if (!this->TolerantTimer.IsTicking())
			this->TolerantTimer.Start(pType->TolerantTime);
	}

	if (pType->SuicideAboveRange)
	{
		if (const auto pWeapon = pBullet->WeaponType)
		{
			auto source = (pTechno && !this->NotMainWeapon) ? pTechno->GetCoords() : pBullet->SourceCoords;
			auto target = pBullet->TargetCoords;

			if (this->NotMainWeapon || (pTechno && pTechno->IsInAir()))
			{
				source.Z = 0;
				target.Z = 0;
			}

			if (static_cast<int>(source.DistanceFrom(target)) >= (pWeapon->Range + 256))
				return true;
		}
	}

	return false;
}

void TracingTrajectory::ChangeFacing()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	constexpr double ratio = Math::TwoPi / 256;

	if (!pType->BulletSpin)
	{
		if (pType->ROT < 0)
			return;

		VelocityClass desiredFacing
		{
			static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X),
			static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y),
			0
		};

		if (!pType->ROT)
		{
			pBullet->Velocity = desiredFacing * (1 / desiredFacing.Length());
			return;
		}

		const auto current = std::atan2(pBullet->Velocity.Y, pBullet->Velocity.X);
		const auto desired = std::atan2(desiredFacing.Y, desiredFacing.X);
		const auto rotate = pType->ROT * ratio;

		const auto differenceP = desired - current;
		const bool dir1 = differenceP > 0;
		const auto delta = dir1 ? differenceP : -differenceP;

		const bool dir2 = delta > Math::Pi;
		const auto differenceR = dir2 ? (Math::TwoPi - delta) : delta;
		const bool dirR = dir1 ^ dir2;

		if (differenceR <= rotate)
		{
			pBullet->Velocity = desiredFacing * (1 / desiredFacing.Length());
			return;
		}

		const auto facing = current + (dirR ? rotate : -rotate);
		pBullet->Velocity.X = std::cos(facing);
		pBullet->Velocity.Y = std::sin(facing);
		pBullet->Velocity.Z = 0;
	}
	else
	{
		const auto radian = std::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) + (pType->ROT * ratio);
		pBullet->Velocity.X = std::cos(radian);
		pBullet->Velocity.Y = std::sin(radian);
		pBullet->Velocity.Z = 0;
	}
}

bool TracingTrajectory::CheckFireFacing()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	if (!pType->WeaponCheck || !pType->Synchronize || pType->TraceTheTarget || pType->ROT < 0 || pType->BulletSpin)
		return true;

	const auto& theBullet = pBullet->Location;
	const auto& theTarget = pBullet->TargetCoords;
	const auto targetDir = DirStruct { std::atan2(double(theTarget.Y - theBullet.Y), double(theTarget.X - theBullet.X)) };
	const auto bulletDir = DirStruct { std::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) };

	return Math::abs(static_cast<short>(static_cast<short>(targetDir.Raw) - static_cast<short>(bulletDir.Raw))) <= (2048 + (pType->ROT << 8));
}

VelocityClass TracingTrajectory::ChangeVelocity()
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	const auto pFirer = pBullet->Owner;

	const auto destination = (pType->TraceTheTarget || !pFirer) ? pBullet->TargetCoords : (pFirer->Transporter ? pFirer->Transporter->GetCoords() : pFirer->GetCoords());
	CoordStruct theOffset = pType->OffsetCoord;

	if (pType->OffsetCoord->X != 0 || pType->OffsetCoord->Y != 0)
	{
		switch (pType->TraceMode)
		{
		case TraceTargetMode::Global:
		{
			break;
		}
		case TraceTargetMode::Body:
		{
			if (const auto pTechno = flag_cast_to<TechnoClass*>(pType->TraceTheTarget ? pBullet->Target : pBullet->Owner))
			{
				const auto rotateAngle = -(pTechno->PrimaryFacing.Current().GetRadian<32>());

				theOffset.X = static_cast<int>(pType->OffsetCoord->X * std::cos(rotateAngle) + pType->OffsetCoord->Y * std::sin(rotateAngle));
				theOffset.Y = static_cast<int>(pType->OffsetCoord->X * std::sin(rotateAngle) - pType->OffsetCoord->Y * std::cos(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::Turret:
		{
			if (const auto pTechno = flag_cast_to<TechnoClass*>(pType->TraceTheTarget ? pBullet->Target : pBullet->Owner))
			{
				const auto rotateAngle = (pTechno->HasTurret() ? -(pTechno->TurretFacing().GetRadian<32>()) : -(pTechno->PrimaryFacing.Current().GetRadian<32>()));

				theOffset.X = static_cast<int>(pType->OffsetCoord->X * std::cos(rotateAngle) + pType->OffsetCoord->Y * std::sin(rotateAngle));
				theOffset.Y = static_cast<int>(pType->OffsetCoord->X * std::sin(rotateAngle) - pType->OffsetCoord->Y * std::cos(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::RotateCW:
		{
			const auto distanceCoords = pBullet->Location - destination;
			const auto radius = Point2D { pType->OffsetCoord->X,pType->OffsetCoord->Y }.Length();

			if ((radius * 1.2) > Point2D { distanceCoords.X,distanceCoords.Y }.Length())
			{
				auto rotateAngle = std::atan2((double)distanceCoords.Y, (double)distanceCoords.X);

				if (Math::abs(radius) > 1e-10)
					rotateAngle += float(pType->Trajectory_Speed / radius);

				theOffset.X = static_cast<int>(radius * std::cos(rotateAngle));
				theOffset.Y = static_cast<int>(radius * std::sin(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		case TraceTargetMode::RotateCCW:
		{
			const auto distanceCoords = pBullet->Location - destination;
			const auto radius = Point2D { pType->OffsetCoord->X,pType->OffsetCoord->Y }.Length();

			if ((radius * 1.2) > Point2D { distanceCoords.X,distanceCoords.Y }.Length())
			{
				auto rotateAngle = std::atan2((double)distanceCoords.Y, (double)distanceCoords.X);

				if (Math::abs(radius) > 1e-10)
					rotateAngle -= float(pType->Trajectory_Speed / radius);

				theOffset.X = static_cast<int>(radius * std::cos(rotateAngle));
				theOffset.Y = static_cast<int>(radius * std::sin(rotateAngle));
			}
			else
			{
				theOffset.X = 0;
				theOffset.Y = 0;
			}

			break;
		}
		default:
		{
			const auto& theSource = pBullet->SourceCoords;
			const auto& theTarget = pBullet->TargetCoords;

			const auto rotateAngle = std::atan2(double(theTarget.Y - theSource.Y), double(theTarget.X - theSource.X));

			theOffset.X = static_cast<int>(pType->OffsetCoord->X * std::cos(rotateAngle) + pType->OffsetCoord->Y * std::sin(rotateAngle));
			theOffset.Y = static_cast<int>(pType->OffsetCoord->X * std::sin(rotateAngle) - pType->OffsetCoord->Y * std::cos(rotateAngle));
			break;
		}
		}
	}

	const auto distanceCoords = ((destination + theOffset) - pBullet->Location);
	const auto distance = distanceCoords.Length();

	VelocityClass velocity
	{
		static_cast<double>(distanceCoords.X),
		static_cast<double>(distanceCoords.Y),
		static_cast<double>(distanceCoords.Z)
	};

	if (distance > pType->Trajectory_Speed)
		velocity *= (pType->Trajectory_Speed / distance);

	return velocity;
}

AbstractClass* TracingTrajectory::GetBulletTarget(TechnoClass* pTechno, HouseClass* pOwner, WeaponTypeClass* pWeapon)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;
	if (pType->TraceTheTarget)
		return pBullet;

	if (pType->Synchronize)
		return pBullet->Target;

	const auto vec = Helpers::Alex::getCellSpreadItems(pBullet->Location, (pWeapon->Range / 256.0), pWeapon->Projectile->AA);

	for (const auto& pOpt : vec)
	{
		if (!pOpt->IsAlive || !pOpt->IsOnMap || pOpt->InLimbo || pOpt->IsSinking || pOpt->Health <= 0)
			continue;

		const auto pOptType = pOpt->GetTechnoType();

		if (!pOptType->LegalTarget || pOpt == pTechno)
			continue;

		const auto absType = pOpt->WhatAmI();

		if (pOwner->IsAlliedWith(pOpt->Owner))
			continue;

		const auto pCell = pOpt->GetCell();

		if (absType == AbstractType::Infantry && pOpt->IsDisguisedAs(pOwner) && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
			continue;

		if (absType == AbstractType::Unit && pOpt->IsDisguised() && !pCell->DisguiseSensors_InclHouse(pOwner->ArrayIndex))
			continue;

		if (pOpt->CloakState == CloakState::Cloaked && !pCell->Sensors_InclHouse(pOwner->ArrayIndex))
			continue;

		if (FakeWarheadTypeClass::ModifyDamage(100, pWeapon->Warhead, pOptType->Armor, 0) == 0)
			continue;

		auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
		if ((!EnumFunctions::IsTechnoEligible(pOpt, pExt->CanTarget) || !pExt->HasRequiredAttachedEffects(pOpt, pTechno)))
			continue;

		return pOpt;
	}

	return pBullet->Target;
}

#include <Ext/TechnoType/Body.h>

CoordStruct TracingTrajectory::GetWeaponFireCoord(TechnoClass* pTechno)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	if (pType->TraceTheTarget)
	{
		const auto pTransporter = pTechno->Transporter;

		if (!this->NotMainWeapon && pTechno && (pTransporter || !pTechno->InLimbo))
		{
			if (pTechno->WhatAmI() != AbstractType::Building)
			{
				if (!pTransporter)
					return TechnoExtData::GetFLHAbsoluteCoords(pTechno, this->FLHCoord, pTechno->HasTurret());

				return TechnoExtData::GetFLHAbsoluteCoords(pTransporter, this->FLHCoord, pTransporter->HasTurret());
			}

			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			Matrix3D mtx;
			mtx.MakeIdentity();

			if (pTechno->HasTurret())
			{
				TechnoTypeExtContainer::Instance.Find(pBuilding->Type)->ApplyTurretOffset(&mtx , 1.0);
				mtx.RotateZ(static_cast<float>(pTechno->TurretFacing().GetRadian<32>()));
			}

			mtx.Translate(static_cast<float>(this->FLHCoord.X), static_cast<float>(this->FLHCoord.Y), static_cast<float>(this->FLHCoord.Z));
			const auto result = mtx.GetTranslation();

			return (pBuilding->GetCoords() + this->BuildingCoord + CoordStruct { static_cast<int>(result.X), -static_cast<int>(result.Y), static_cast<int>(result.Z) });
		}

		return pBullet->SourceCoords;
	}

	const auto& weaponCoord = pType->WeaponCoord.Get();

	if (weaponCoord == CoordStruct::Empty)
		return pBullet->Location;

	const auto rotateRadian = std::atan2(double(pBullet->TargetCoords.Y - pBullet->Location.Y), double(pBullet->TargetCoords.X - pBullet->Location.X));
	CoordStruct fireOffsetCoord
	{
		static_cast<int>(weaponCoord.X * std::cos(rotateRadian) + weaponCoord.Y * std::sin(rotateRadian)),
		static_cast<int>(weaponCoord.X * std::sin(rotateRadian) - weaponCoord.Y * std::cos(rotateRadian)),
		weaponCoord.Z
	};

	return pBullet->Location + fireOffsetCoord;
}

bool TracingTrajectory::PrepareTracingWeapon()
{
	auto const pType = this->GetTrajectoryType();
	//auto pBullet = this->AttachedTo;

	if (const auto pWeapon = pType->Weapons[this->WeaponIndex])
		this->CreateTracingBullets(pWeapon);

	if (this->WeaponCount < 0 || --this->WeaponCount > 0)
		return false;

	if (++this->WeaponIndex < static_cast<int>(pType->Weapons.size()))
	{
		const int validCounts = pType->WeaponCount.size();
		this->WeaponCount = pType->WeaponCount[(this->WeaponIndex < validCounts) ? this->WeaponIndex : (validCounts - 1)];

		return false;
	}

	this->WeaponIndex = 0;
	this->WeaponCount = pType->WeaponCount[0];

	if (this->WeaponCycle < 0 || --this->WeaponCycle > 0)
		return false;

	this->WeaponTimer.Stop();

	return pType->SuicideIfNoWeapon;
}

void TracingTrajectory::CreateTracingBullets(WeaponTypeClass* pWeapon)
{
	auto const pType = this->GetTrajectoryType();
	auto pBullet = this->AttachedTo;

	const auto pTechno = pBullet->Owner;
	auto pOwner = pTechno ? pTechno->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner;

	if (!pOwner || pOwner->Defeated)
	{
		if (const auto pNeutral = HouseClass::FindNeutral())
			pOwner = pNeutral;
		else
			return;
	}

	const auto pTarget = this->GetBulletTarget(pTechno, pOwner, pWeapon);

	if (!pTarget)
		return;

	if (const int validDelays = pType->WeaponDelay.size())
	{
		const auto delay = pType->WeaponDelay[(this->WeaponIndex < validDelays) ? this->WeaponIndex : (validDelays - 1)];
		this->WeaponTimer.Start((delay > 0) ? delay : 1);
	}

	if (!this->WeaponCount)
		return;

#ifdef FUCK_THIS
	const auto fireCoord = this->GetWeaponFireCoord(pBullet, pTechno);
	const auto targetCoords = pTarget->GetCoords();
	const auto finalDamage = static_cast<int>(pWeapon->Damage * this->FirepowerMult);


	if (const auto pCreateBullet = pWeapon->Projectile->CreateBullet(pTarget, pTechno, finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
	{
		BulletExt::SimulatedFiringUnlimbo(pCreateBullet, pOwner, pWeapon, fireCoord, false);
		const auto pBulletExt = BulletExt::ExtMap.Find(pCreateBullet);

		if (const auto pTraj = pBulletExt->Trajectory.get())
		{
			const auto flag = pTraj->Flag();

			if (flag == TrajectoryFlag::Tracing)
			{
				const auto pTrajectory = static_cast<TracingTrajectory*>(pTraj);
				pTrajectory->FirepowerMult = this->FirepowerMult;
				pTrajectory->NotMainWeapon = true;
			}
			/*			else if (flag == TrajectoryFlag::Disperse) // TODO If merge #1295
						{
							const auto pTrajectory = static_cast<DisperseTrajectory*>(pTraj);
							pTrajectory->FirepowerMult = this->FirepowerMult;
						}*/
						/*			else if (flag == TrajectoryFlag::Straight) // TODO If merge #1294
									{
										const auto pTrajectory = static_cast<StraightTrajectory*>(pTraj);
										pTrajectory->FirepowerMult = this->FirepowerMult;
									}*/
									/*			else if (flag == TrajectoryFlag::Engrave) // TODO If merge #1293
												{
													const auto pTrajectory = static_cast<EngraveTrajectory*>(pTraj);
													pTrajectory->NotMainWeapon = true;
												}*/
		}

		const auto pAttach = pType->TraceTheTarget ? (!this->NotMainWeapon ? static_cast<ObjectClass*>(pTechno) : nullptr) : pBullet;
		BulletExt::SimulatedFiringEffects(pCreateBullet, pOwner, pAttach, true, true);
	}
#endif

}