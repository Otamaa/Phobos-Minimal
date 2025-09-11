#include "PhobosTrajectory.h"

#include <Ext/WeaponType/Body.h> //for weaponTypeExt::ExtData
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <BulletClass.h>
#include <Helpers/Macro.h>

#include "BombardTrajectory.h"
#include "StraightTrajectory.h"
#include "StraightTrajectoryVarianC.h"
#include "ArtilleryTrajectory.h"
#include "BounceTrajectory.h"
#include "MeteorTrajectory.h"
#include "SpiralTrajectory.h"
#include "VerticalTrajectory.h"
#include "WaveTrajectory.h"
#include "ArcingTrajectory.h"
#include "EngraveTrajectory.h"
#include "DisperseTrajectory.h"
#include "TracingTrajectory.h"

#define Make_traj(enum_type , type) \
case TrajectoryFlag::##enum_type##: \
	pTraj = std::make_unique<##type##>(pBullet, pType); break;

#define Make_DefaultTraj(enum_type , type) \
case TrajectoryFlag::##enum_type##: \
	pTraj = std::make_unique<##type##>(); break;

#define Make_DefaultTrajType(enum_type) \
case TrajectoryFlag::##enum_type##: \
	pType = std::make_unique<##enum_type##TrajectoryType>(); break;

bool PhobosTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI { pINI };
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");

	this->Trajectory_Speed.Read(exINI, pSection, "Trajectory.Speed");
	this->Trajectory_Speed = MaxImpl(0.001, this->Trajectory_Speed);

	return true;
}

bool PhobosTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->DetonationDistance)
		.Process(this->Trajectory_Speed)
		.Success()
		;
}

bool PhobosTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return 	Stm
		.Process(this->DetonationDistance)
		.Process(this->Trajectory_Speed)
		.Success()
		;
}

bool PhobosTrajectoryType::UpdateType(std::unique_ptr<PhobosTrajectoryType>& pType, TrajectoryFlag nFlag)
{
	switch (nFlag)
	{
	Make_DefaultTrajType(Straight)
	Make_DefaultTrajType(StraightVariantB)
	Make_DefaultTrajType(StraightVariantC)
	Make_DefaultTrajType(Disperse)
	Make_DefaultTrajType(Engrave)
	Make_DefaultTrajType(Bombard)
	Make_DefaultTrajType(Artillery)
	Make_DefaultTrajType(Bounce)
	Make_DefaultTrajType(Vertical)
	Make_DefaultTrajType(Meteor)
	Make_DefaultTrajType(Spiral)
	Make_DefaultTrajType(Wave)
	Make_DefaultTrajType(Arcing)
	Make_DefaultTrajType(Tracing)
	default:
		pType.release();
		return false;
	}

	return true;
}

bool PhobosTrajectory::CanSnap(std::unique_ptr<PhobosTrajectory>& traj)
{
	COMPILETIMEEVAL TrajectoryFlag flags[] = {
		TrajectoryFlag::Straight,
		TrajectoryFlag::StraightVariantB,
		TrajectoryFlag::Bombard,
		TrajectoryFlag::StraightVariantC,
		TrajectoryFlag::Disperse,
		TrajectoryFlag::Engrave,
		TrajectoryFlag::Parabola,
		TrajectoryFlag::Tracing
	};

	for (auto flag : flags) {
		if (traj->Flag == flag) {
			return true;
		}
	}

	return false;
}

bool PhobosTrajectory::BlockDrawTrail(std::unique_ptr<PhobosTrajectory>& traj)
{
	if (!traj)
		return false;

	COMPILETIMEEVAL TrajectoryFlag flags[] = {
		TrajectoryFlag::StraightVariantC,
		TrajectoryFlag::Disperse,
		TrajectoryFlag::Engrave
	};

	for (auto flag : flags) {
		if (traj->Flag == flag) {
			return true;
		}
	}

	return false;
}

bool PhobosTrajectory::IgnoreAircraftROT0(std::unique_ptr<PhobosTrajectory>& traj)
{
	if (!traj)
		return false;

	COMPILETIMEEVAL TrajectoryFlag flags[] = {
		TrajectoryFlag::StraightVariantC,
		TrajectoryFlag::Parabola,
		TrajectoryFlag::Disperse,
		TrajectoryFlag::Tracing,
	};

	for (auto flag : flags) {
		if (traj->Flag == flag) {
			return true;
		}
	}

	return false;
}

void PhobosTrajectoryType::CreateType(std::unique_ptr<PhobosTrajectoryType>& pType, CCINIClass* const pINI, const char* pSection, const char* pKey)
{
	if (!pINI->GetKey(pSection, pKey))
		return;

	TrajectoryFlag nFlag = TrajectoryFlag::Invalid;

	if(pINI->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer)  > 0) {
		if (!GameStrings::IsBlank(Phobos::readBuffer)) {
			for (size_t i = 0; i < TrajectoryTypeToSrings.size(); ++i) {
				if (IS_SAME_STR_(Phobos::readBuffer, TrajectoryTypeToSrings[i].second.data())) {
					nFlag = TrajectoryTypeToSrings[i].first;
					break;
				}
			}
		}
	}

	if (pType && pType->Flag == nFlag || nFlag == TrajectoryFlag::Invalid)
		return;
	else
	{
		if (PhobosTrajectoryType::UpdateType(pType,nFlag))
		{
			if (!pType->Read(pINI, pSection))
				Debug::LogInfo("Failed When Reading Projectile[{}] With TrajectoryType {} ! ", pSection, Phobos::readBuffer);

			if (pType->Flag == TrajectoryFlag::Bounce)
			{
				Debug::LogInfo("BounceTrajectory is Unfinished ! ");
				//const auto pBounceType = reinterpret_cast<BounceTrajectoryType*>(pType.get());
				//if (!(pBounceType->BounceAmount > 0))
				//{
					pType.release();
				//}
			}
		}
	}

}

void PhobosTrajectoryType::ProcessFromStream(PhobosStreamReader& Stm, std::unique_ptr<PhobosTrajectoryType>& pType)
{
	bool bExist = false;
	Stm.Process(bExist);

	if (bExist) {

		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);

		//Read the pointer value again so it can be registered
		PhobosTrajectoryType* pOld = nullptr;
		if (!Stm.Load(pOld) || !pOld)
			return;

		//create the new type
		if(PhobosTrajectoryType::UpdateType(pType, nFlag)) {
			// register the change if succeeded
			PHOBOS_SWIZZLE_REGISTER_POINTER((long)pOld, pType.get(), pType->Name())
			pType->Load(Stm, true);
		}
	}
}

void PhobosTrajectoryType::ProcessFromStream(PhobosStreamWriter& Stm, std::unique_ptr<PhobosTrajectoryType>& pType)
{
	const bool Exist = pType.get();
	Stm.Process(Exist);

	if (Exist) {
		Stm.Process(pType->Flag, false);
		//write the pointer value
		Stm.Save(pType.get());
		pType->Save(Stm);
	}
}

bool PhobosTrajectoryType::TrajectoryValidation(BulletTypeClass* pAttached) {

	bool ret = false;

	// Trajectory validation combined with other projectile behaviour.
	if (auto& pTraj = BulletTypeExtContainer::Instance.Find(pAttached)->TrajectoryType)
	{
		if (pTraj->Flag == TrajectoryFlag::Invalid)
			return ret;

		const char* pSection = pAttached->ID;
		auto&pTrjType = PhobosTrajectoryType::TrajectoryTypeToSrings[(int)pTraj->Flag].second;

		if (pAttached->Arcing) {
			Debug::LogInfo("Bullet[{}] has Trajectory[{}] set together with Arcing. Arcing has been set to false.", pSection , pTrjType);
			pAttached->Arcing = false;
			ret = true;
		}

		if (pAttached->Inviso) {
			Debug::LogInfo("Bullet[{}] has Trajectory[{}] set together with Inviso. Inviso has been set to false.", pSection, pTrjType);
			pAttached->Inviso = false;
			ret = true;
		}

		if (pAttached->ROT) {
			Debug::LogInfo("Bullet[{}] has Trajectory[{}] set together with ROT value other than 0. ROT has been set to 0.", pSection, pTrjType);
			pAttached->ROT = 0;
			ret = true;
		}

		if (pAttached->Vertical) {
			Debug::LogInfo("Bullet[{}] has Trajectory[{}] set together with Vertical. Vertical has been set to false.", pSection, pTrjType);
			pAttached->Vertical = false;
			ret = true;
		}
	}

	return ret;
}

double PhobosTrajectory::GetTrajectorySpeed() const
{
	auto const pBullet = this->AttachedTo;

	if (pBullet->WeaponType) {
		const auto result = WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType)->Trajectory_Speed.Get(BulletTypeExtContainer::Instance.Find(pBullet->Type)->Trajectory_Speed.Get(pBullet->Speed));

		if (result != 0.0)
			return result;
	}
	else
	{
		const auto result = BulletTypeExtContainer::Instance.Find(pBullet->Type)->Trajectory_Speed.Get(pBullet->Speed);

		if (result != 0.0)
			return result;
	}

	return this->GetTrajectoryType()->Trajectory_Speed;
}

bool PhobosTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->AttachedTo, true)
		.Process(this->Type, true)
		.Process(this->DetonationDistance)
		.Success()
		;
}

bool PhobosTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return 	Stm
		.Process(this->AttachedTo)
		.Process(this->Type)
		.Process(this->DetonationDistance)
		.Success()
		;
}

bool PhobosTrajectory::UpdateType(BulletClass* pBullet, std::unique_ptr<PhobosTrajectory>& pTraj, PhobosTrajectoryType* pType)
{
	switch (pType->Flag)
	{
	Make_traj(Straight , StraightTrajectory)
	Make_traj(StraightVariantB , StraightTrajectoryVarianB)
	Make_traj(StraightVariantC, StraightTrajectoryVarianC)
	Make_traj(Disperse, DisperseTrajectory)
	Make_traj(Engrave, EngraveTrajectory)
	Make_traj(Bombard, BombardTrajectory)
	Make_traj(Artillery, ArtilleryTrajectory)
	Make_traj(Bounce, BounceTrajectory)
	Make_traj(Meteor, MeteorTrajectory)
	Make_traj(Spiral, SpiralTrajectory)
	Make_traj(Vertical, VerticalTrajectory)
	Make_traj(Wave, WaveTrajectory)
	Make_traj(Arcing, ArcingTrajectory)
	Make_traj(Tracing, TracingTrajectory)
	default:
		pTraj.release();
		return false;
		break;
	}

	return true;
}

bool PhobosTrajectory::CreateInstance(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (!pBulletTypeExt->TrajectoryType)
		return false;

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);

	if (PhobosTrajectory::UpdateType(pBullet , pBulletExt->Trajectory ,pBulletTypeExt->TrajectoryType.get())) {
		pBulletExt->Trajectory->OnUnlimbo(pCoord, pVelocity);
	}

	return true;
}

void PhobosTrajectory::ProcessFromStream(PhobosStreamReader& Stm, std::unique_ptr<PhobosTrajectory>& pTraj)
{
	bool bExist = false;
	Stm.Process(bExist);

	if (bExist)
	{
		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);
		switch (nFlag)
		{
		Make_DefaultTraj(Straight , StraightTrajectory)
		Make_DefaultTraj(StraightVariantB, StraightTrajectoryVarianB)
		Make_DefaultTraj(StraightVariantC, StraightTrajectoryVarianC)
		Make_DefaultTraj(Disperse, DisperseTrajectory)
		Make_DefaultTraj(Engrave, EngraveTrajectory)
		Make_DefaultTraj(Bombard, BombardTrajectory)
		Make_DefaultTraj(Artillery, ArtilleryTrajectory)
		Make_DefaultTraj(Bounce, BounceTrajectory)
		Make_DefaultTraj(Meteor, MeteorTrajectory)
		Make_DefaultTraj(Spiral, SpiralTrajectory)
		Make_DefaultTraj(Vertical, VerticalTrajectory)
		Make_DefaultTraj(Wave, WaveTrajectory)
		Make_DefaultTraj(Arcing, ArcingTrajectory)
		Make_DefaultTraj(Tracing, TracingTrajectory)
		default:
			pTraj.release();
			return;
		}

		pTraj->Load(Stm, false);
	}
}

void PhobosTrajectory::ProcessFromStream(PhobosStreamWriter& Stm, std::unique_ptr<PhobosTrajectory>& pTraj)
{
	const bool Exist = pTraj.get();
	Stm.Process(Exist);
	if (Exist)
	{
		Stm.Process(pTraj->Flag);
		pTraj->Save(Stm);
	}
}

DWORD PhobosTrajectory::OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords)
{
	enum { SkipCheck = 0x4678F8, ContinueAfterCheck = 0x467879, Detonate = 0x467E53 };

	if (auto& pTraj = BulletExtContainer::Instance.Find(pBullet)->Trajectory)
	{
#ifndef Debug_Traj
		switch (pTraj->OnAITargetCoordCheck(coords))
#else
		const auto result = pTraj->OnAITargetCoordCheck(coords);
		Debug::LogInfo(__FUNCTION__" Bullet[{} - {}] with Trajectory[{}] result [{}]", pBullet->get_ID(), pBullet, TrajectoryTypeToSrings[(int)pTraj->Flag], EnumFunctions::TrajectoryCheckReturnType_to_strings[(int)result]);
		switch(result)
#endif
		{
		case TrajectoryCheckReturnType::SkipGameCheck:
			return SkipCheck;
		case TrajectoryCheckReturnType::SatisfyGameCheck:
			return ContinueAfterCheck;
		case TrajectoryCheckReturnType::Detonate:
			return Detonate;
		default:
			break;
		}
	}

	return 0;

}

DWORD PhobosTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	enum { SkipCheck = 0x467A2B, ContinueAfterCheck = 0x4679EB, Detonate = 0x467E53 };

	if (auto& pTraj = BulletExtContainer::Instance.Find(pBullet)->Trajectory)
	{
#ifndef Debug_Traj
		switch (pTraj->OnAITechnoCheck(pTechno))
#else
		const auto result = pTraj->OnAITechnoCheck(pTechno);
		Debug::LogInfo(__FUNCTION__" Bullet[{} - {}] with Trajectory[{}] result [{}]", pBullet->get_ID(), pBullet, TrajectoryTypeToSrings[(int)pTraj->Flag], EnumFunctions::TrajectoryCheckReturnType_to_strings[(int)result]);
		switch (result)
#endif
		{
		case TrajectoryCheckReturnType::SkipGameCheck:
			return SkipCheck;
		case TrajectoryCheckReturnType::SatisfyGameCheck:
			return ContinueAfterCheck;
		case TrajectoryCheckReturnType::Detonate:
			return Detonate;
		default:
			break;
		}
	}

	return 0;
}

void PhobosTrajectory::SetInaccurate() const
{
	this->AttachedTo->TargetCoords += BulletTypeExtData::CalculateInaccurate(this->AttachedTo->Type);
}

#undef Make_traj
#undef Make_DefaultTraj
#undef Make_DefaultTrajType