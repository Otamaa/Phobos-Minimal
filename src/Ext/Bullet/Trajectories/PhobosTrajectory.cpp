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

	static std::array<const char*, (size_t)TrajectoryFlag::Count> TrajectoryTypeToSrings
	{ {
		{"Straight"} ,
		{"Bombard"} ,
		{"Artillery"} ,
		{"Bounce"} ,
		{"Vertical"} ,
		{"Meteor"} ,
		{"Spiral"} ,
		{"Wave"} ,
	 }
	};

	pINI->ReadString(pSection, pKey, "", Phobos::readBuffer);

	TrajectoryFlag nFlag = TrajectoryFlag::Invalid;

	if (!INIClass::IsBlank(Phobos::readBuffer)) {
		for (size_t i = 0; i < TrajectoryTypeToSrings.size(); ++i) {
			if (IS_SAME_STR_(Phobos::readBuffer, TrajectoryTypeToSrings[i])) {
				nFlag = static_cast<TrajectoryFlag>(i);
				break;
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

double PhobosTrajectory::GetTrajectorySpeed(BulletClass* pBullet) const
{
	double nResult = 100.0;

	if (!pBullet->WeaponType)
		return nResult;

	auto const nWeaponnResult = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType)->Trajectory_Speed.Get();
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

	return pTypeExt->Trajectory_Speed.Get(nWeaponnResult == 0.0 ? nResult : nWeaponnResult);
}

bool PhobosTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->Type, true)
		.Process(this->DetonationDistance, false)
		.Success()
		;
}

bool PhobosTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return 	Stm
		.Process(this->Type, true)
		.Process(this->DetonationDistance, false)
		.Success()
		;
}

bool PhobosTrajectory::UpdateType(std::unique_ptr<PhobosTrajectory>& pTraj, PhobosTrajectoryType* pType, TrajectoryFlag flag)
{
	switch (flag)
	{
	case TrajectoryFlag::Straight:
		pTraj = std::make_unique<StraightTrajectory>(pType);
		break;

	case TrajectoryFlag::Bombard:
		pTraj = std::make_unique<BombardTrajectory>(pType);
		break;

	case TrajectoryFlag::Artillery:
		pTraj = std::make_unique<ArtilleryTrajectory>(pType);
		break;

	case TrajectoryFlag::Bounce:
		pTraj = std::make_unique<BounceTrajectory>(pType);
		break;

	case TrajectoryFlag::Meteor:
		pTraj = std::make_unique<MeteorTrajectory>(pType);
		break;

	case TrajectoryFlag::Spiral:
		pTraj = std::make_unique<SpiralTrajectory>(pType);
		break;

	case TrajectoryFlag::Vertical:
		pTraj = std::make_unique<VerticalTrajectory>(pType);
		break;

	case TrajectoryFlag::Wave:
		pTraj = std::make_unique<WaveTrajectory>(pType);
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

	auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (!pBulletTypeExt->TrajectoryType)
		return;

	auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);

	if (PhobosTrajectory::UpdateType(pBulletExt->Trajectory, pBulletTypeExt->TrajectoryType.get() , pBulletTypeExt->TrajectoryType->Flag)) {
		pBulletExt->Trajectory->OnUnlimbo(pBullet, pCoord, pVelocity);
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

void PhobosTrajectory::SetInaccurate(BulletClass* pBullet)
{
	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

		const int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		const int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? (int)(pTypeExt->BallisticScatter_Max.Get()) : ballisticScatter;
		const int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? (int)(pTypeExt->BallisticScatter_Min.Get()) : (scatterMax / 2);

		const double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		const double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};

		pBullet->TargetCoords += offset;
	}
}