#include "SupportSpawns.h"

#include <Misc/Kratos/Extension/WeaponTypeExt.h>

#include <Misc/Kratos/Ext/Helper/FLH.h>
#include <Misc/Kratos/Ext/Helper/Weapon.h>
#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/ObjectType/AttachEffect.h>

SupportSpawnsData* SupportSpawns::GetSupportSpawnsData()
{
	if (!_data)
	{
		TechnoClass* pSpawnOwner = pTechno->SpawnOwner;
		if (pSpawnOwner)
		{
			_data = INI::GetConfig<SupportSpawnsData>(INI::Rules, pSpawnOwner->GetTechnoType()->ID)->Data;
		}
	}
	return _data;
}

SupportSpawnsFLHData* SupportSpawns::GetSupportSpawnsFLHData()
{
	if (!_flhData)
	{
		TechnoClass* pSpawnOwner = pTechno->SpawnOwner;
		if (pSpawnOwner)
		{
			const char* section = pSpawnOwner->GetTechnoType()->ID;
			std::string image = INI::GetSection(INI::Rules, section)->Get("Image", std::string{ section });
			_flhData = INI::GetConfig<SupportSpawnsFLHData>(INI::Art, image.c_str())->Data;
		}
	}
	return _flhData;
}

void SupportSpawns::Setup()
{
	_data = nullptr;
	_flhData = nullptr;
	_alwaysFire = false;
	if (!IsAircraft() || !pTechno->GetTechnoType()->Spawned)
	{
		Disable();
	}
}

AttachEffect* SupportSpawns::AEManager()
{
	AttachEffect* aeManager = nullptr;
	if (_parent)
	{
		aeManager = _parent->GetComponent<AttachEffect>();
	}
	return aeManager;
}

void SupportSpawns::FireSupportWeaponToSpawn(bool checkROF)
{
	SupportSpawnsData* data = GetSupportSpawnsData();
	std::vector<std::string> weapons = data->Weapons;
	CoordStruct flh = CoordStruct::Empty;
	CoordStruct hitFLH = CoordStruct::Empty;
	SupportSpawnsFLHData* flhData = GetSupportSpawnsFLHData();
	if (flhData)
	{
		flh = flhData->SupportWeaponFLH;
		hitFLH = flhData->SupportWeaponHitFLH;
	}
	if (pTechno->Veterancy.IsElite())
	{
		weapons = data->EliteWeapons;
		if (flhData)
		{
			flh = flhData->EliteSupportWeaponFLH;
			hitFLH = flhData->EliteSupportWeaponHitFLH;
		}
	}
	TechnoClass* pAttacker = pTechno->SpawnOwner;
	HouseClass* pAttackingHouse = pAttacker->Owner;
	TechnoClass* pTarget = pTechno;
	TechnoClass* pShooter = WhoIsShooter(pAttacker);
	if (!weapons.empty())
	{
		if (data->SwitchFLH)
		{
			flh.Y = flh.Y * _flipY;
			_flipY *= -1;
		}
		bool isOnTurret = data->IsOnTurret;
		bool turnTurret = data->TurnTurret;

		for (std::string weaponId : weapons)
		{
			if (IsNotNone(weaponId))
			{
				if (WeaponTypeClass* pWeapon = WeaponTypeClass::Find(weaponId.c_str()))
				{
					// Check CanFire
					if (WeaponTypeExt::TypeData* weaponData = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeapon))
					{
						if (!weaponData->CanFireToTarget(pTarget, pShooter, pAttacker, pAttackingHouse, pWeapon))
						{
							continue;
						}
					}
					// Check ROF
					if (checkROF)
					{
						auto it = _rof.find(weaponId);
						if (it != _rof.end())
						{
							if (it->second.InProgress())
							{
								continue;
							}
						}
					}
					// Ready to fire
					CoordStruct sourcePos = GetFLHAbsoluteCoords(pShooter, flh, isOnTurret);
					CoordStruct targetPos = GetFLHAbsoluteCoords(pTarget, hitFLH, true);
					VelocityClass v = GetBulletVelocity(sourcePos, targetPos);
					// Fire weapon
					FireBulletTo(pShooter, pAttacker, pTarget, pAttackingHouse, pWeapon, sourcePos, targetPos, v, flh, isOnTurret);
					if (turnTurret && pShooter->HasTurret())
					{
						// 强制扭头
						DirStruct facing = Point2Dir(sourcePos, targetPos);
						pShooter->F_TurretFacing().Set_Current(facing);
					}
					if (checkROF)
					{
						int rof = pWeapon->ROF;
						auto it = _rof.find(weaponId);
						if (it != _rof.end())
						{
							it->second.Start(rof);
						}
						else
						{
							CDTimerClass timer{ rof };
							_rof[weaponId] = timer;
						}
					}
				}
			}
		}
	}
	if (data->Attach)
	{
		// 给自己附加AE
		AttachEffect* aeManager = AEManager();
		if (aeManager)
		{
			aeManager->Attach(data->AttachEffects, data->AttachChances, false, pAttacker, pAttackingHouse);
		}
	}
}

void SupportSpawns::Awake()
{
	Setup();
}

void SupportSpawns::ExtChanged()
{
	Setup();
}

void SupportSpawns::OnPut(CoordStruct* pCoord, DirType dirType)
{
	Setup();
	// SpawnOwner在awake拿不到，在put里检查
	TechnoClass* pSpawnOwner = pTechno->SpawnOwner;
	if (!pSpawnOwner || !GetSupportSpawnsData()->Enable)
	{
		Disable();
		return;
	}
	_alwaysFire = GetSupportSpawnsData()->AlwaysFire;
}

void SupportSpawns::OnRemove()
{
	_alwaysFire = false;
}

void SupportSpawns::OnUpdate()
{
	if (_alwaysFire && !IsDeadOrInvisible(pTechno))
	{
		TechnoClass* pSpawnOwner = pTechno->SpawnOwner;
		if (!IsDeadOrInvisible(pSpawnOwner))
		{
			FireSupportWeaponToSpawn(true);
		}
	}
}

void SupportSpawns::OnFire(AbstractClass* pTarget, int weaponIdx)
{
	if (!_alwaysFire)
	{
		TechnoClass* pSpawnOwner = pTechno->SpawnOwner;
		if (!IsDeadOrInvisible(pSpawnOwner))
		{
			FireSupportWeaponToSpawn(false);
		}
	}
}

