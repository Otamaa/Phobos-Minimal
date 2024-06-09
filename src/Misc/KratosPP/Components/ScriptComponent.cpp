#include "ScriptComponent.h"

#include <Misc/KratosPP/Ext/Bullet/Body.h>
#include <Misc/KratosPP/Ext/Techno/Body.h>

GameObject* ObjectScript::GetGameObject()
{
	if (TechnoExtData* technoExtData = dynamic_cast<TechnoExtData*>(_extData))
	{
		return technoExtData->_GameObject;
	}
	else if (BulletExtData* bulletExtData = dynamic_cast<BulletExtData*>(_extData))
	{
		return bulletExtData->_GameObject;
	}
	return nullptr;
}

TechnoClass* ObjectScript::GetTechno()
{
	if (TechnoExtData* technoExtData = dynamic_cast<TechnoExtData*>(_extData))
	{
		return technoExtData->OwnerObject();
	}
	return nullptr;
}

BulletClass* ObjectScript::GetBullet()
{
	if (BulletExtData* bulletExtData = dynamic_cast<BulletExtData*>(_extData))
	{
		return bulletExtData->OwnerObject();
	}
	return nullptr;
}

LocoType ObjectScript::GetThisLocoType()
{
	if (!IsBuilding())
	{
		if (_locoType == LocoType::None)
		{
			_locoType = GetLocoType(pTechno);
		}
	}
	return _locoType;
}

BulletType BulletScript::GetBulletType()
{
	if (_bulletType == BulletType::UNKNOWN)
	{
		_bulletType = WhatAmI(pBullet);
		if (_bulletType != BulletType::ROCKET && trajectoryData->IsStraight())
		{
			_bulletType = BulletType::ROCKET;
		}
	}
	return _bulletType;
}

TrajectoryData* BulletScript::GetTrajectoryData()
{
	if (!_trajectoryData)
	{
		_trajectoryData = INI::GetConfig<TrajectoryData>(INI::Rules, pBullet->GetType()->ID)->Data;
	}
	return _trajectoryData.get();
}