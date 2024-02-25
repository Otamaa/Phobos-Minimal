#include "PhobosTrajectory.h"

#include <Ext/WeaponType/Body.h> //for weaponTypeExt::ExtData
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <BulletClass.h>
#include <Helpers/Macro.h>

#include "BombardTrajectory.h"
#include "StraightTrajectory.h"
#include "ArtilleryTrajectory.h"
#include "BounceTrajectory.h"
#include "MeteorTrajectory.h"
#include "SpiralTrajectory.h"
#include "VerticalTrajectory.h"
#include "WaveTrajectory.h"
#include "ArcingTrajectory.h"

bool PhobosTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI { pINI };
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");

	return true;
}

bool PhobosTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->DetonationDistance)
		.Success()
		;
}

bool PhobosTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return 	Stm
		.Process(this->DetonationDistance)
		.Success()
		;
}

bool PhobosTrajectoryType::UpdateType(std::unique_ptr<PhobosTrajectoryType>& pType, TrajectoryFlag nFlag)
{
	switch (nFlag)
	{
	case TrajectoryFlag::Straight:
	{
		pType = (std::make_unique<StraightTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Bombard:
	{
		pType = (std::make_unique<BombardTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Artillery:
	{
		pType = (std::make_unique<ArtilleryTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Bounce:
	{
		pType = (std::make_unique<BounceTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Vertical:
	{
		pType = (std::make_unique<VerticalTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Meteor:
	{
		pType = (std::make_unique<MeteorTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Spiral:
	{
		pType = (std::make_unique<SpiralTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Wave:
	{
		pType = (std::make_unique<WaveTrajectoryType>());
	}
	break;
	case TrajectoryFlag::Arcing:
		pType = (std::make_unique<ArcingTrajectoryType>());
		break;
	default:
		pType.release();
		return false;
	}

	return true;
}

void PhobosTrajectoryType::CreateType(std::unique_ptr<PhobosTrajectoryType>& pType, CCINIClass* const pINI, const char* pSection, const char* pKey)
{
	if (!pINI->GetKey(pSection, pKey))
		return;

	static constexpr std::array<const char*, (size_t)TrajectoryFlag::Count> TrajectoryTypeToSrings
	{ {
		{"Straight"} ,
		{"Bombard"} ,
		{"Artillery"} ,
		{"Bounce"} ,
		{"Vertical"} ,
		{"Meteor"} ,
		{"Spiral"} ,
		{"Wave"} ,
		{"Arcing" },
	 }
	};

	TrajectoryFlag nFlag = TrajectoryFlag::Invalid;

	if(pINI->ReadString(pSection, pKey, "", Phobos::readBuffer)  > 0) {
		if (!GameStrings::IsBlank(Phobos::readBuffer)) {
			for (size_t i = 0; i < TrajectoryTypeToSrings.size(); ++i) {
				if (IS_SAME_STR_(Phobos::readBuffer, TrajectoryTypeToSrings[i])) {
					nFlag = static_cast<TrajectoryFlag>(i);
					break;
				}
			}
		}
	}

	if (pType && pType->Flag == nFlag)
		return;
	else
	{
		if (PhobosTrajectoryType::UpdateType(pType,nFlag))
		{
			if (!pType->Read(pINI, pSection))
				Debug::Log("Failed When Reading Projectile[%s] With TrajectoryType %s ! \n", pSection, Phobos::readBuffer);

			if (pType->Flag == TrajectoryFlag::Bounce)
			{
				Debug::Log("BounceTrajectory is Unfinished ! \n");
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
	Stm.Load(bExist);

	if (bExist)
	{
		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);

		//Read the pointer value again so it can be registered
		PhobosTrajectoryType* pOld = nullptr;
		if (!Stm.Load(pOld) || !pOld)
			return;

		//create the new type
		if(PhobosTrajectoryType::UpdateType(pType, nFlag)) {
			// register the change if succeeded
			PhobosSwizzle::Instance.RegisterChange(pOld, pType.get());
			pType->Load(Stm, false);
		}
	}
}

void PhobosTrajectoryType::ProcessFromStream(PhobosStreamWriter& Stm, std::unique_ptr<PhobosTrajectoryType>& pType)
{
	const bool Exist = pType.get();
	Stm.Save(Exist);
	if (Exist)
	{
		Stm.Process(pType->Flag, false);
		//write the pointer value
		if(Savegame::WritePhobosStream(Stm, pType.get()))
			pType->Save(Stm);
	}
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

	return 100.0;
}

bool PhobosTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->AttachedTo)
		.Process(this->Type)
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
	case TrajectoryFlag::Straight:
		pTraj = std::make_unique<StraightTrajectory>(pBullet , pType);
		break;

	case TrajectoryFlag::Bombard:
		pTraj = std::make_unique<BombardTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Artillery:
		pTraj = std::make_unique<ArtilleryTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Bounce:
		pTraj = std::make_unique<BounceTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Meteor:
		pTraj = std::make_unique<MeteorTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Spiral:
		pTraj = std::make_unique<SpiralTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Vertical:
		pTraj = std::make_unique<VerticalTrajectory>(pBullet, pType);
		break;

	case TrajectoryFlag::Wave:
		pTraj = std::make_unique<WaveTrajectory>(pBullet, pType);
		break;
	case TrajectoryFlag::Arcing:
		pTraj = std::make_unique<ArcingTrajectory>(pBullet, pType);
		break;
	default:
		pTraj.release();
		return false;
		break;
	}

	return true;
}

void PhobosTrajectory::CreateInstance(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (!pBulletTypeExt->TrajectoryType)
		return;

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);

	if (PhobosTrajectory::UpdateType(pBullet , pBulletExt->Trajectory ,pBulletTypeExt->TrajectoryType.get())) {
		pBulletExt->Trajectory->OnUnlimbo(pCoord, pVelocity);
	}

}

void PhobosTrajectory::ProcessFromStream(PhobosStreamReader& Stm, std::unique_ptr<PhobosTrajectory>& pTraj)
{
	bool bExist = false;
	Stm.Load(bExist);

	if (bExist)
	{
		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);
		switch (nFlag)
		{
		case TrajectoryFlag::Straight:
			pTraj = std::make_unique<StraightTrajectory>();
			break;

		case TrajectoryFlag::Bombard:
			pTraj = std::make_unique<BombardTrajectory>();
			break;

		case TrajectoryFlag::Artillery:
			pTraj = std::make_unique<ArtilleryTrajectory>();
			break;

		case TrajectoryFlag::Bounce:
			pTraj = std::make_unique<BounceTrajectory>();
			break;

		case TrajectoryFlag::Meteor:
			pTraj = std::make_unique<MeteorTrajectory>();
			break;

		case TrajectoryFlag::Spiral:
			pTraj = std::make_unique<SpiralTrajectory>();
			break;

		case TrajectoryFlag::Vertical:
			pTraj = std::make_unique<VerticalTrajectory>();
			break;

		case TrajectoryFlag::Wave:
			pTraj = std::make_unique<WaveTrajectory>();
			break;

		case TrajectoryFlag::Arcing:
			pTraj = std::make_unique<ArcingTrajectory>();
			break;

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
	Stm.Save(Exist);
	if (Exist)
	{
		Stm.Process(pTraj->Flag);
		pTraj->Save(Stm);
	}
}

void PhobosTrajectory::SetInaccurate() const
{
	this->AttachedTo->TargetCoords += BulletTypeExtData::CalculateInaccurate(this->AttachedTo->Type);
}