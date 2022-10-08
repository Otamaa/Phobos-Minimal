#include "PhobosTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>

#include <BulletClass.h>
#include <Helpers/Macro.h>

#include "BombardTrajectory.h"
#include "StraightTrajectory.h"
#include "ArtilleryTrajectory.h"
#include "BounceTrajectory.h"

bool PhobosTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI || !pINI->GetSection(pSection))
		return false;

	INI_EX exINI { pINI };
	this->DetonationDistance.Read(exINI, pSection, "Trajectory.DetonationDistance");

	return true;
}

bool PhobosTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return 	Stm
		.Process(this->DetonationDistance, false)
		.Success()
		;
}

bool PhobosTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return 	Stm
		.Process(this->DetonationDistance, false)
		.Success()
		;
}

void PhobosTrajectoryType::CreateType(PhobosTrajectoryType*& pType, CCINIClass* const pINI, const char* pSection, const char* pKey)
{
	PhobosTrajectoryType* pNewType = nullptr;
	bool bUpdateType = true;

	pINI->ReadString(pSection, pKey, "", Phobos::readBuffer);
	if (INIClass::IsBlank(Phobos::readBuffer))
		pNewType = nullptr;
	else if (_stricmp(Phobos::readBuffer, "Straight") == 0)
		pNewType = GameCreate<StraightTrajectoryType>();
	else if (_stricmp(Phobos::readBuffer, "Bombard") == 0)
		pNewType = GameCreate<BombardTrajectoryType>();
	else if (_stricmp(Phobos::readBuffer, "Artillery") == 0)
		pNewType = GameCreate<ArtilleryTrajectoryType>();
	else if (_stricmp(Phobos::readBuffer, "Bounce") == 0)
		pNewType = GameCreate<BounceTrajectoryType>();
	else
		bUpdateType = false;

	if (pNewType)
	{
		pNewType->Read(pINI, pSection);

		if (pNewType->Flag == TrajectoryFlag::Bounce) {
			const auto pBounceType = reinterpret_cast<BounceTrajectoryType*>(pNewType);
			if(!(pBounceType->BounceAmount > 0)) {
				GameDelete<true,false>(pNewType);
				pNewType = nullptr;
			}
		}
	}

	if (bUpdateType) {
		GameDelete<true>(pType); // GameDelete already has if(pType) check here.
		pType = (pNewType);
	}
}

PhobosTrajectoryType* PhobosTrajectoryType::LoadFromStream(PhobosStreamReader& Stm)
{
	PhobosTrajectoryType* pType = nullptr;
	bool bExist = false;
	Stm.Load(bExist);

	if (bExist)
	{
		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);
		switch (nFlag)
		{
		case TrajectoryFlag::Straight:
			pType = GameCreate<StraightTrajectoryType>();
			break;

		case TrajectoryFlag::Bombard:
			pType = GameCreate<BombardTrajectoryType>();
			break;

		case TrajectoryFlag::Artillery:
			pType = GameCreate<ArtilleryTrajectoryType>();
			break;

		case TrajectoryFlag::Bounce:
			pType = GameCreate<BounceTrajectoryType>();
			break;

		default:
			return nullptr;
		}
		pType->Load(Stm, false);
	}

	return pType;
}

void PhobosTrajectoryType::WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType)
{
	const bool Exist = pType;
	Stm.Save(Exist);
	if (Exist) {
		Stm.Process(pType->Flag,false);
		pType->Save(Stm);
	}
}

PhobosTrajectoryType* PhobosTrajectoryType::ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectoryType* pType)
{
	UNREFERENCED_PARAMETER(pType);
	return LoadFromStream(Stm);
}

PhobosTrajectoryType* PhobosTrajectoryType::ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType)
{
	WriteToStream(Stm, pType);
	return pType;
}

double PhobosTrajectory::GetTrajectorySpeed(BulletClass* pBullet) const
{
	double nResult = 100.0;
	auto nWeaponnResult = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType)->Trajectory_Speed.Get();

	return BulletExt::ExtMap.Find(pBullet)->TypeExt->Trajectory_Speed.Get(nWeaponnResult == 0.0 ? nResult : nWeaponnResult);
}

double PhobosTrajectory::GetTrajectorySpeed(BulletExt::ExtData* pBulletExt) const
{
	double nResult = 100.0;
	auto nWeaponnResult = WeaponTypeExt::ExtMap.Find(pBulletExt->Get()->WeaponType)->Trajectory_Speed.Get();

	return pBulletExt->TypeExt->Trajectory_Speed.Get(nWeaponnResult == 0.0 ? nResult : nWeaponnResult);
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

PhobosTrajectory* PhobosTrajectory::CreateInstance(PhobosTrajectoryType* pType, BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity)
{
	PhobosTrajectory* pRet = nullptr;

	switch (pType->Flag)
	{
	case TrajectoryFlag::Straight:
		pRet = GameCreate<StraightTrajectory>(pType);
		break;

	case TrajectoryFlag::Bombard:
		pRet = GameCreate<BombardTrajectory>(pType);
		break;

	case TrajectoryFlag::Artillery:
		pRet = GameCreate<ArtilleryTrajectory>(pType);
		break;

	case TrajectoryFlag::Bounce:
		pRet = GameCreate<BounceTrajectory>(pType);
		break;

	default:
		break;
	}

	if (pRet) {
		pRet->OnUnlimbo(pBullet,pCoord, pVelocity);
	}

	return pRet;
}

PhobosTrajectory* PhobosTrajectory::LoadFromStream(PhobosStreamReader& Stm)
{
	PhobosTrajectory* pTraj = nullptr;
	bool bExist = false;
	Stm.Load(bExist);

	if (bExist)
	{
		TrajectoryFlag nFlag = TrajectoryFlag::Invalid;
		Stm.Process(nFlag, false);
		switch (nFlag)
		{
		case TrajectoryFlag::Straight:
			pTraj = GameCreate<StraightTrajectory>();
			break;

		case TrajectoryFlag::Bombard:
			pTraj = GameCreate<BombardTrajectory>();
			break;

		case TrajectoryFlag::Artillery:
			pTraj = GameCreate<ArtilleryTrajectory>();
			break;

		case TrajectoryFlag::Bounce:
			pTraj = GameCreate<BounceTrajectory>();
			break;

		default:
			return nullptr;
		}
		pTraj->Load(Stm, false);
	}
	return pTraj;
}

void PhobosTrajectory::WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj)
{
	const bool Exist = pTraj;
	Stm.Save(Exist);
	if (Exist)
	{
		Stm.Process(pTraj->Flag);
		pTraj->Save(Stm);
	}
}

PhobosTrajectory* PhobosTrajectory::ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectory* pTraj)
{
	UNREFERENCED_PARAMETER(pTraj);
	return LoadFromStream(Stm);
}

PhobosTrajectory* PhobosTrajectory::ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj)
{
	WriteToStream(Stm, pTraj);
	return pTraj;
}
