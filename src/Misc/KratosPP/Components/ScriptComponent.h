#pragma once
#include "Component.h"
#include "GameObject.h"

#include <Misc/KratosPP/Interfaces/Scriptables.h>
#include <Misc/KratosPP/Utils/Enums.h>
#include <Misc/KratosPP/Utils/TExtension.h>

class TrajectoryData;


class ScriptComponent : public Component
{
public:
	ScriptComponent() : Component() { }

	virtual GameObject* GetGameObject() = 0;
	__declspec(property(get = GetGameObject)) GameObject* _gameObject;
};

#define SCRIPT_COMPONENT(SCRIPT_TYPE, TBASE, TEXT, P_NAME) \
	SCRIPT_TYPE() : ScriptComponent() {} \
	\
	virtual GameObject* GetGameObject() override \
	{ \
	return ((TEXT*)_extData)->_GameObject; \
	}\
	\
	TBASE* GetOwner() \
	{ \
		return ((TEXT*)_extData)->OwnerObject(); \
	} \
	__declspec(property(get = GetOwner)) TBASE* P_NAME; \

class ObjectScript : public ScriptComponent, public ITechnoScript, public IBulletScript
{
public:
	ObjectScript() : ScriptComponent() { }

	virtual GameObject* GetGameObject() override;

	TechnoClass* GetTechno();

	__declspec(property(get = GetTechno)) TechnoClass* pTechno;

	BulletClass* GetBullet();

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
			BuildingClass* pBuilding = dynamic_cast<BuildingClass*>(pTechno);
			return pBuilding->BState == BStateType::Construction && pBuilding->CurrentMission != Mission::Selling;
		}
		return false;
	}

	bool InSelling()
	{
		if (IsBuilding())
		{
			BuildingClass* pBuilding = dynamic_cast<BuildingClass*>(pTechno);
			return pBuilding->BState == BStateType::Construction && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus > 0;
		}
		return false;
	}

protected:
	AbstractType _absType = AbstractType::None;
};

class TechnoExtData;
class TechnoScript : public ScriptComponent, public ITechnoScript
{
public:
	SCRIPT_COMPONENT(TechnoScript, TechnoClass, TechnoExtData, pTechno);

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

	LocoType GetThisLocoType();

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

protected:
	// 单位类型
	AbstractType _absType = AbstractType::None;
	LocoType _locoType = LocoType::None;
};

class BulletScript : public ScriptComponent, public IBulletScript
{
public:
	SCRIPT_COMPONENT(BulletScript, BulletClass, BulletExtData, pBullet);

	BulletType GetBulletType();

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

protected:
	// 抛射体类型
	BulletType _bulletType = BulletType::UNKNOWN;
	// 弹道配置
	std::unique_ptr<TrajectoryData> _trajectoryData;
	TrajectoryData* GetTrajectoryData();

	__declspec(property(get = GetTrajectoryData)) TrajectoryData* trajectoryDa;
};

class AnimScript : public ScriptComponent, public IAnimScript
{
public:
	SCRIPT_COMPONENT(AnimScript, AnimClass, AnimExtData, pAnim);
};

class SuperWeaponScript : public ScriptComponent, public ISuperScript
{
public:
	SCRIPT_COMPONENT(SuperWeaponScript, SuperClass, SuperWeaponExtData, pSuper);
};

class EBoltScript : public ScriptComponent
{
public:
	SCRIPT_COMPONENT(EBoltScript, EBolt, EBoltExtData, pBolt);
};

#define DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, ...) \
	CLASS_NAME() : __VA_ARGS__() \
	{ \
		this->Name = ScriptName; \
	} \
	\
	inline static std::string ScriptName = #CLASS_NAME; \
	static Component* Create() \
	{ \
		return static_cast<Component*>(new CLASS_NAME()); \
	} \
	\
	inline static int g_temp_##CLASS_NAME = \
	ComponentFactory::GetInstance().Register(#CLASS_NAME, CLASS_NAME::Create); \

#define OBJECT_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, ObjectScript) \

#define TECHNO_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, TechnoScript) \

#define BULLET_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, BulletScript) \

#define ANIM_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, AnimScript) \

#define SUPER_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, SuperWeaponScript) \

#define EBOLT_SCRIPT(CLASS_NAME) \
	DECLARE_DYNAMIC_SCRIPT(CLASS_NAME, EBoltScript) \
