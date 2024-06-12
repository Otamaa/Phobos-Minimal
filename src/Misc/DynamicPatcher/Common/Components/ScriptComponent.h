#pragma once

#include <YRPP.h>
#include <GeneralDefinitions.h>
#include <SpecificStructures.h>

#include "Component.h"
#include "GameObject.h"
#include "Scriptable.h"

#include <Utilities/Container.h>

/// @brief 所有的脚本都位于GameObject下
class ScriptComponent : public Component
{
public:
	ScriptComponent() : Component() {
		this->c_Type |= ComponentType::Script;
	}

	virtual void Clean() override
	{
		Component::Clean();
	}

	virtual GameObject* GetGameObject() = 0;
	__declspec(property(get = GetGameObject)) GameObject* _gameObject;
};

#define SCRIPT_COMPONENT(SCRIPT_TYPE, TBASE, TEXT, P_NAME , C_TYPE) \
	SCRIPT_TYPE() : ScriptComponent() {	this->c_Type |= ComponentType::##C_TYPE;} \
	\
	virtual GameObject* GetGameObject() override \
	{ \
	return ((TEXT::ExtData*)_extData)->_GameObject; \
	}\
	\
	TBASE* GetOwner() \
	{ \
		return ((TEXT::ExtData*)_extData)->OwnerObject(); \
	} \
	__declspec(property(get = GetOwner)) TBASE* P_NAME; \

#pragma once

#include <Misc/DynamicPatcher/Extension/AnimExt.h>
#include <Misc/DynamicPatcher/Extension/BulletExt.h>
#include <Misc/DynamicPatcher/Extension/EBoltExt.h>
#include <Misc/DynamicPatcher/Extension/TechnoExt.h>
#include <Misc/DynamicPatcher/Extension/SuperWeaponExt.h>

#include <Misc/DynamicPatcher/Ext/Helper/Status.h>

#include <Misc/DynamicPatcher/Ext/BulletType/Trajectory/TrajectoryData.h>
#include <Misc/DynamicPatcher/Extension/AnimExt.h>

enum class AnimScripts {
	unk ,
	AnimStand,
	AnimStatus,
};

class AnimScript : public ScriptComponent, public IAnimScript
{
public:
	SCRIPT_COMPONENT(AnimScript, AnimClass, AnimExt, pAnim , Anim);

	virtual void Clean() override { ScriptComponent::Clean(); }
	virtual AnimScripts GetCurrentScriptType() = 0;
};

enum class ObjectScripts
{
	unk,
	AttachEffect,
	AttachFire,
};

class ObjectScript : public ScriptComponent, public ITechnoScript, public IBulletScript
{
public:
	ObjectScript() : ScriptComponent() {
		this->c_Type |= ComponentType::Object;
	 }

	virtual ObjectScripts GetCurrentScriptType() = 0;

	virtual GameObject* GetGameObject() override
	{
		switch (_extData->Type)
		{
		case ExtType::Techno:
			return ((TechnoExt::ExtData*)_extData)->_GameObject;
		case ExtType::Bullet:
			return ((BulletExt::ExtData*)_extData)->_GameObject;
		default:
			return nullptr;
		}
	}

	TechnoClass* GetTechno()
	{
		switch (_extData->Type)
		{
		case ExtType::Techno:
			return ((TechnoExt::ExtData*)_extData)->OwnerObject();
		default:
			return nullptr;
		}
	}

	__declspec(property(get = GetTechno)) TechnoClass* pTechno;

	BulletClass* GetBullet()
	{
		switch (_extData->Type)
		{
		case ExtType::Bullet:
			return ((BulletExt::ExtData*)_extData)->OwnerObject();
		default:
			return nullptr;
		}
	}

	__declspec(property(get = GetBullet)) BulletClass* pBullet;

	ObjectClass* GetOwner()
	{
		ObjectClass* pObject = pTechno;
		if (!pTechno)
		{
			pObject = pBullet;
		}
		if (!pObject)
		{
			Debug::Log("Warning: ObjectScript \"%s\" got a unknown ExtData!\n", Name.c_str());
		}
		return pObject;
	}

	__declspec(property(get = GetOwner)) ObjectClass* pObject;

	AbstractType GetAbsType()
	{
		if (_absType == AbstractType::None)
		{
			_absType = pObject->WhatAmI();
		}
		return _absType;
	}

	bool IsBullet()
	{
		return pBullet != nullptr;
	}

	bool IsBuilding()
	{
		return pTechno && GetAbsType() == AbstractType::Building;
	}

	bool IsInfantry()
	{
		return pTechno && GetAbsType() == AbstractType::Infantry;
	}

	bool IsUnit()
	{
		return pTechno && GetAbsType() == AbstractType::Unit;
	}

	bool IsAircraft()
	{
		return pTechno && GetAbsType() == AbstractType::Aircraft;
	}

	bool IsFoot()
	{
		return !IsBullet() && !IsBuilding();
	}

	bool InBuilding()
	{
		if (IsBuilding())
		{
			BuildingClass* pBuilding = static_cast<BuildingClass*>(pTechno);
			return pBuilding->BState == BStateType::Construction && pBuilding->CurrentMission != Mission::Selling;
		}
		return false;
	}

	bool InSelling()
	{
		if (IsBuilding())
		{
			BuildingClass* pBuilding = static_cast<BuildingClass*>(pTechno);
			return pBuilding->BState == BStateType::Construction && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus > 0;
		}
		return false;
	}

	virtual void Clean() override
	{
		ScriptComponent::Clean();

		_absType = AbstractType::None;
	}
protected:
	AbstractType _absType = AbstractType::None;
};

enum class TechnoScripts
{
	unk ,
	AircraftAttitude,
	AircraftDive,
	AircraftGuard,
	AircraftPut,
	AutoFireAreaWeapon,
	BaseNormal,
	CrawlingFLH,
	DamageText,
	DecoyMissile,
	HealthText,
	JumpjetCarryall,
	JumpjetFacing,
	MissileHoming,
	Spawn,
	SupportSpawns,
	TechnoStatus,
	TechnoTrail,
	TurretAngle,

};

class TechnoScript : public ScriptComponent, public ITechnoScript
{
public:
	SCRIPT_COMPONENT(TechnoScript, TechnoClass, TechnoExt, pTechno, Techno);

	AbstractType GetAbsType()
	{
		if (_absType == AbstractType::None)
		{
			_absType = pTechno->WhatAmI();
		}
		return _absType;
	}

	bool IsBuilding()
	{
		return GetAbsType() == AbstractType::Building;
	}
	bool IsInfantry()
	{
		return GetAbsType() == AbstractType::Infantry;
	}
	bool IsUnit()
	{
		return GetAbsType() == AbstractType::Unit;
	}
	bool IsAircraft()
	{
		return GetAbsType() == AbstractType::Aircraft;
	}

	LocoType GetThisLocoType()
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

	bool IsFly()
	{
		return GetThisLocoType() == LocoType::Fly;
	}
	bool IsJumpjet()
	{
		return GetThisLocoType() == LocoType::Jumpjet;
	}
	bool IsShip()
	{
		return GetThisLocoType() == LocoType::Ship;
	}

	bool IsRocket()
	{
		return IsAircraft() && GetThisLocoType() == LocoType::Rocket;
	}

	virtual void Clean() override
	{
		ScriptComponent::Clean();

		_absType = AbstractType::None;
		_locoType = LocoType::None;
	}

	virtual TechnoScripts GetCurrentScriptType() = 0;
protected:
	// 单位类型
	AbstractType _absType = AbstractType::None;
	LocoType _locoType = LocoType::None;
};

static FORCEINLINE constexpr bool IsITechnoScript(Component* C){
	return C->c_Type & ComponentType::Object || C->c_Type & ComponentType::Techno;
}

enum class BulletScripts
{
	unk,
	Bounce,
	BulletStatus,
	BulletTrail,
	ArcingTrajectory,
	MissileTrajectory,
	StraightTrajectory,

};
class BulletScript : public ScriptComponent, public IBulletScript
{
public:
	SCRIPT_COMPONENT(BulletScript, BulletClass, BulletExt, pBullet, Bullet);

	BulletType GetBulletType()
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

	bool IsArcing()
	{
		return GetBulletType() == BulletType::ARCING;
	}
	bool IsMissile()
	{
		return GetBulletType() == BulletType::MISSILE;
	}
	bool IsRocket()
	{
		return GetBulletType() == BulletType::ROCKET;
	}
	bool IsBomb()
	{
		return GetBulletType() == BulletType::BOMB;
	}

	virtual void Clean() override
	{
		ScriptComponent::Clean();

		_bulletType = BulletType::UNKNOWN;
		_trajectoryData = nullptr;
	}

	virtual BulletScripts GetCurrentScriptType() = 0;
protected:
	// 抛射体类型
	BulletType _bulletType = BulletType::UNKNOWN;
	// 弹道配置
	TrajectoryData* _trajectoryData = nullptr;
	TrajectoryData* GetTrajectoryData()
	{
		if (!_trajectoryData)
		{
			_trajectoryData = INI::GetConfig<TrajectoryData>(INI::Rules, pBullet->GetType()->ID)->Data;
		}
		return _trajectoryData;
	}
	__declspec(property(get = GetTrajectoryData)) TrajectoryData* trajectoryData;
};

enum class SuperWeaponScripts
{
	unk
};
class SuperWeaponScript : public ScriptComponent, public ISuperScript
{
public:
	SCRIPT_COMPONENT(SuperWeaponScript, SuperClass, SuperWeaponExt, pSuper , Super);

	virtual void Clean() override { ScriptComponent::Clean(); }
	virtual SuperWeaponScripts GetCurrentScriptType() = 0;
};

enum class EBoltScripts
{
	unk , EBoltStatus
};
class EBoltScript : public ScriptComponent
{
public:
	SCRIPT_COMPONENT(EBoltScript, EBolt, EBoltExt, pBolt , EBolt);

	virtual void Clean() override { ScriptComponent::Clean(); }
	virtual EBoltScripts GetCurrentScriptType() = 0;
};

#define OBJECT_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, ObjectScript) \

#define TECHNO_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, TechnoScript) \
	virtual TechnoScripts GetCurrentScriptType() override { return TechnoScripts::##CLASS_NAME##; }\

#define BULLET_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, BulletScript) \
	virtual BulletScripts GetCurrentScriptType() override { return BulletScripts::##CLASS_NAME##; }\

#define SUPER_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, SuperWeaponScript) \
	virtual SuperWeaponScripts GetCurrentScriptType() override { return SuperWeaponScripts::##CLASS_NAME##; }\

#define EBOLT_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, EBoltScript) \
	virtual EBoltScripts GetCurrentScriptType() override { return EBoltScripts::##CLASS_NAME##; }\

#define ANIM_SCRIPT(CLASS_NAME) \
	DECLARE_COMPONENT(CLASS_NAME, AnimScript) \
	virtual AnimScripts GetCurrentScriptType() override { return AnimScripts::##CLASS_NAME##; }\

