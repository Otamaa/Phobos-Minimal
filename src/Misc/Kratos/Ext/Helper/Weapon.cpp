#include "Weapon.h"
#include "CastEx.h"
#include "DrawEx.h"
#include "FLH.h"
#include "MathEx.h"
#include "Scripts.h"
#include "Status.h"

#include <CellClass.h>
#include <ObjectClass.h>
#include <SpawnManagerClass.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>

#include <Misc/Kratos/Extension/TechnoExt.h>
#include <Misc/Kratos/Extension/BulletExt.h>
#include <Misc/Kratos/Extension/TechnoTypeExt.h>
#include <Misc/Kratos/Extension/WeaponTypeExt.h>

#include <Misc/Kratos/Ext/ObjectType/AttachEffect.h>
#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>
#include <Misc/Kratos/Ext/WeaponType/TargetLaserData.h>

#include <Utilities/EnumFunctions.h>

// ----------------
// 高级弹道学
// ----------------
#pragma region Advanced Ballistics
VelocityClass GetBulletVelocity(CoordStruct source, CoordStruct target)
{
	CoordStruct flh{ 1, 0, 0 };
	DirStruct dir = Point2Dir(source, target);
	CoordStruct v = GetFLHAbsoluteOffset(flh, dir);
	auto vv = ToVelocity(v);
	return  { vv.X, vv.Y , vv.Z };
}

VelocityClass RecalculateBulletVelocity(BulletClass* pBullet, CoordStruct source, CoordStruct target)
{
	CoordStruct vector = target - source;
	auto vv = ToVelocity(vector);
	double dist = target.DistanceFrom(source);
	vv *= isnan(dist) || dist <= 0 ? 0 : (pBullet->Speed / dist);
	pBullet->Velocity = { vv.X, vv.Y , vv.Z };
	pBullet->SourceCoords = source;
	pBullet->TargetCoords = target;
	return pBullet->Velocity;
}

VelocityClass RecalculateBulletVelocity(BulletClass* pBullet, CoordStruct target)
{
	return RecalculateBulletVelocity(pBullet, pBullet->GetCoords(), target);
}

VelocityClass RecalculateBulletVelocity(BulletClass* pBullet)
{
	return RecalculateBulletVelocity(pBullet, pBullet->GetCoords(), pBullet->GetTargetCoords());
}

CoordStruct GetInaccurateOffset(float scatterMin, float scatterMax)
{
	// 不精确, 需要修改目标坐标
	int min = (int)(scatterMin * 256);
	int max = scatterMax > 0 ? (int)(scatterMax * 256) : RulesClass::Instance->BallisticScatter;
	if (min > max)
	{
		int temp = min;
		min = max;
		max = temp;
	}
	// 随机偏移
	return GetRandomOffset(min, max);
}

VelocityClass GetBulletArcingVelocity(CoordStruct sourcePos, CoordStruct targetPos,
	double speed, double gravity, bool lobber,
	int zOffset, double& straightDistance, double& realSpeed)
{
	// 重算抛物线弹道
	if (gravity == 0)
	{
		gravity = RulesClass::Instance->Gravity;
	}
	CoordStruct tempSourcePos = sourcePos;
	CoordStruct tempTargetPos = targetPos;
	int zDiff = tempTargetPos.Z - tempSourcePos.Z + zOffset; // 修正高度差
	tempTargetPos.Z = 0;
	tempSourcePos.Z = 0;
	straightDistance = tempTargetPos.DistanceFrom(tempSourcePos);
	realSpeed = speed;
	if (straightDistance == 0 || std::isnan(straightDistance))
	{
		// 直上直下
		return VelocityClass{ 0.0, 0.0, gravity };
	}
	if (realSpeed == 0)
	{
		// realSpeed = WeaponTypeClass.GetSpeed((int)straightDistance, gravity);
		realSpeed = Math::sqrt(straightDistance * gravity * 1.2);
	}
	// 高抛弹道
	if (lobber)
	{
		realSpeed = (int)(realSpeed * 0.5);
	}
	double vZ = (zDiff * realSpeed) / straightDistance + 0.5 * gravity * straightDistance / realSpeed;
	VelocityClass v { tempTargetPos.X - tempSourcePos.X, tempTargetPos.Y - tempSourcePos.Y, 0.0 };
	v *= realSpeed / straightDistance;
	v.Z = vZ;
	return v;
}

VelocityClass GetBulletArcingVelocity(CoordStruct sourcePos, CoordStruct& targetPos,
	double speed, double gravity, bool lobber, bool inaccurate, float scatterMin, float scatterMax,
	int zOffset, double& straightDistance, double& realSpeed, CellClass*& pTargetCell)
{
	// 不精确
	if (inaccurate)
	{
		targetPos += GetInaccurateOffset(scatterMin, scatterMax);
	}
	// 不潜地
	if ((pTargetCell = MapClass::Instance->TryGetCellAt(targetPos)) != nullptr)
	{
		targetPos.Z = pTargetCell->GetCoordsWithBridge().Z;
	}
	return GetBulletArcingVelocity(sourcePos, targetPos, speed, gravity, lobber, zOffset, straightDistance, realSpeed);
}
#pragma endregion

// ----------------
// 自定义武器发射
// ----------------
#pragma region Custom fire weapon

RadialFire::RadialFire(DirStruct dir, int burst, int splitAngle)
{
	this->burst = burst;
	InitData(dir, splitAngle);
}

void RadialFire::InitData(DirStruct dir, int splitAngle)
{
	dirRad = dir.GetRadian();
	splitRad = Math::deg2rad(splitAngle * 0.5); // Deg2Rad是逆时针
	delta = splitAngle / (burst + 1);
	deltaZ = 1.0f / (burst / 2.0f + 1);
}

VelocityClass RadialFire::GetBulletVelocity(int index, bool radialZ)
{
	int z = 0;
	float temp = burst / 2.0f;
	if (radialZ)
	{
		if (index - temp < 0)
		{
			z = index;
		}
		else
		{
			z = Math::abs(index - burst + 1);
		}
	}
	int angle = delta * (index + 1);
	double radians = Math::deg2rad(angle); // 逆时针
	Matrix3D mtx;
	mtx.MakeIdentity();
	mtx.RotateZ(static_cast<float>(dirRad)); // 转到单位朝向
	mtx.RotateZ(static_cast<float>(splitRad)); // 逆时针转到发射角
	mtx.RotateZ(-static_cast<float>(radians)); // 顺时针发射
	mtx.Translate(256, 0, 0);
	Vector3D<float> offset;
	Matrix3D::MatrixMultiply(&offset, &mtx, &Vector3D<float>::Empty);
	return { offset.X, -offset.Y, deltaZ * z };
}

TechnoClass* WhoIsShooter(TechnoClass* pAttacker)
{
	TechnoClass* pTransporter = pAttacker->Transporter;
	if (pTransporter)
	{
		// I'm a passengers
		pAttacker = WhoIsShooter(pTransporter);
	}
	return pAttacker;
}


bool InRange(ObjectClass* pObject, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int minRange, int maxRange)
{
	CoordStruct location = pObject->GetCoords();
	switch (pObject->WhatAmI())
	{
	case AbstractType::Building:
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Aircraft:

		// 需要检查CellRangeFinding和InAir
		if (pWeapon->CellRangefinding)
		{
			if (CellClass* pCell = MapClass::Instance->TryGetCellAt(location))
			{
				location = pCell->GetCoordsWithBridge();
			}
		}
		if (pObject->IsInAir())
		{
			location.Z = pTarget->GetCoords().Z;
		}

		return flag_cast_to<TechnoClass*, true>(pObject)->InRange(location, pTarget, pWeapon);
	default:
		CoordStruct targetPos = pTarget->GetCoords();
		double distance = targetPos.DistanceFrom(location);
		if (isnan(distance))
		{
			distance = 0;
		}
		return distance <= pWeapon->Range && distance >= minRange;
	}
}


void FireWeaponTo(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, HouseClass* pAttacingHouse,
	WeaponTypeClass* pWeapon, CoordStruct flh, bool isOnTurret,
	FireBulletToTarget callback, CoordStruct bulletSourcePos,
	bool radialFire, int splitAngle, bool radialZ)
{
	if (!pTarget)
	{
		return;
	}
	CoordStruct targetPos = pTarget->GetCoords();
	// radial fire
	int burst = pWeapon->Burst;
	DirStruct dir;
	if (pShooter)
	{
		dir = pShooter->F_GetRealfacing().Desired();
		if (pShooter->HasTurret())
		{
			dir = pShooter->F_TurretFacing().Desired();
		}
	}
	RadialFire radial{ dir, burst, splitAngle };
	int flipY = -1;
	for (int i = 0; i < burst; i++)
	{
		VelocityClass bulletVelocity;
		if (radialFire)
		{
			flipY = (i < burst / 2.0) ? -1 : 1;
			bulletVelocity = radial.GetBulletVelocity(i, radialZ);
		}
		else
		{
			flipY *= -1;
		}
		CoordStruct sourcePos = bulletSourcePos;
		if (sourcePos.IsEmpty())
		{
			// get flh
			sourcePos = GetFLHAbsoluteCoords(pShooter, flh, isOnTurret, flipY, CoordStruct::Empty);
		}
		if (bulletVelocity.IsEmpty())
		{
			bulletVelocity = GetBulletVelocity(sourcePos, targetPos);
		}
		CoordStruct tempFLH = flh;
		tempFLH.Y *= flipY;
		BulletClass* pBullet = FireBulletTo(pShooter, pAttacker, pTarget, pAttacingHouse, pWeapon, sourcePos, targetPos, bulletVelocity, tempFLH, isOnTurret);
		// callback
		if (callback && pBullet)
		{
			callback(i, burst, pBullet, pTarget);
		}
	}
}

BulletClass* FireBulletTo(ObjectClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, HouseClass* pAttacingHouse,
	WeaponTypeClass* pWeapon, CoordStruct sourcePos, CoordStruct targetPos, VelocityClass velocity, CoordStruct flh, bool isOnTurret)
{
	TechnoClass* pTargetTechno = nullptr;
	if (!pTarget || (CastToTechno(pTarget, pTargetTechno) && IsDeadOrInvisible(pTargetTechno)))
	{
		return nullptr;
	}
	// Get fireMulti
	double fireMulti = 1;
	if (!IsDead(pAttacker))
	{
		// check spawner
		SpawnManagerClass* pSpawns = pAttacker->SpawnManager;
		if (pWeapon->Spawner && pSpawns)
		{
			pSpawns->SetTarget(pTarget);
			return nullptr;
		}
		// check Abilities FIREPOWER
		fireMulti = GetDamageMulti(pAttacker);
	}
	// Fire Weapon
	BulletClass* pBullet = FireBullet(pAttacker, pTarget, pAttacingHouse, pWeapon, fireMulti, sourcePos, targetPos, velocity);
	// Draw bullet effect
	DrawBulletEffect(pWeapon, sourcePos, targetPos, pAttacker, pTarget, pAttacingHouse, flh, isOnTurret);
	// Draw particle system
	AttachedParticleSystem(pWeapon, sourcePos, targetPos, pAttacker, pTarget, pAttacingHouse);
	// Play report sound
	PlayReportSound(pWeapon, sourcePos);
	if (pShooter)
	{
		// Draw weapon anim
		DrawWeaponAnim(pShooter, pAttacker, pAttacingHouse, pWeapon, sourcePos, targetPos);
		// feedbackAE
		TechnoClass* pShooterT = nullptr;
		BulletClass* pShooterB = nullptr;
		AttachEffect* pAEM = nullptr;
		if ((CastToTechno(pShooter, pShooterT) && TryGetAEManager<TechnoExt>(pShooterT, pAEM))
			|| (CastToBullet(pShooter, pShooterB) && TryGetAEManager<BulletExt>(pShooterB, pAEM))
		)
		{
			pAEM->FeedbackAttach(pWeapon);
		}
		if (pShooterT)
		{
			TechnoStatus* status = nullptr;
			if (TryGetStatus<TechnoExt>(pShooterT, status))
			{
				status->RockerPitch(pWeapon);
				// Draw target laser
				TargetLaserData* data = INI::GetConfig<TargetLaserData>(INI::Rules, pWeapon->ID)->Data;
				if (data->Enable)
				{
					if (!data->BreakTargetLaser)
					{
						status->StartTargetLaser(pTarget, pWeapon, *data, flh, isOnTurret);
					}
					else
					{
						status->CloseTargetLaser(pTarget);
					}
				}
			}
		}
	}
	return pBullet;
}

BulletClass* FireBullet(TechnoClass* pAttacker, AbstractClass* pTarget, HouseClass* pAttacingHouse,
	WeaponTypeClass* pWeapon, double fireMulti,
	CoordStruct sourcePos, CoordStruct targetPos, VelocityClass velocity)
{
	BulletTypeClass* pBulletType = pWeapon->Projectile;
	if (pBulletType)
	{
		int damage = static_cast<int>(pWeapon->Damage * fireMulti);
		WarheadTypeClass* pWH = pWeapon->Warhead;
		int speed = pWeapon->GetWeaponSpeed(sourcePos, targetPos);
		bool bright = pWeapon->Bright; // 原游戏中弹头上的bright是无效的
		BulletClass* pBullet = pBulletType->CreateBullet(pTarget, pAttacker, damage, pWH, speed, bright);
		pBullet->WeaponType = pWeapon;
		// 记录所属
		SetSourceHouse(pBullet, pAttacingHouse);
		if (velocity.IsEmpty())
		{
			velocity = GetBulletVelocity(sourcePos, targetPos);
		}
		pBullet->MoveTo(sourcePos, velocity);
		if (!targetPos.IsEmpty())
		{
			pBullet->TargetCoords = targetPos;
		}
		return pBullet;
	}
	return nullptr;
}

void DrawBulletEffect(WeaponTypeClass* pWeapon, CoordStruct sourcePos, CoordStruct targetPos,
	TechnoClass* pAttacker, AbstractClass* pTarget, HouseClass* pAttacingHouse, CoordStruct flh, bool isOnTurret)
{
	WeaponTypeExt::TypeData* weaponData = nullptr;
	// IsLaser
	if (pWeapon->IsLaser)
	{
		LaserType laser;
		ColorStruct houseColor;
		if (pWeapon->IsHouseColor)
		{
			laser.IsHouseColor = true;
			if (pAttacker)
			{
				houseColor = pAttacker->Owner->LaserColor;
			}
			else if (pAttacingHouse)
			{
				houseColor = pAttacingHouse->LaserColor;
			}
		}
		laser.InnerColor = pWeapon->LaserInnerColor;
		laser.OuterColor = pWeapon->LaserOuterColor;
		laser.OuterSpread = pWeapon->LaserOuterSpread;
		laser.IsHouseColor = pWeapon->IsHouseColor;
		laser.Duration = pWeapon->LaserDuration;
		// get thickness and fade
		weaponData = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeapon);
		if (weaponData->LaserThickness > 0)
		{
			laser.Thickness = weaponData->LaserThickness;
		}
		laser.Fade = weaponData->LaserFade;
		laser.IsSupported = weaponData->IsSupported || laser.Thickness > 3;
		// 单一颜色
		laser.IsSingleColor = weaponData->IsSingleColor;
		// 随机颜色
		laser.RandomColor = weaponData->LaserRandomColor;
		// 视觉散布
		laser.VisualScatter = weaponData->VisualScatter;
		laser.VisualScatterMin = weaponData->VisualScatterMin;
		laser.VisualScatterMax = weaponData->VisualScatterMax;
		// draw the laser
		DrawLaser(laser, sourcePos, targetPos, houseColor);
	}
	// IsRadBeam
	if (pWeapon->IsRadBeam)
	{
		RadBeamType beamType = RadBeamType::RadBeam;
		if (pWeapon->Warhead && pWeapon->Warhead->Temporal)
		{
			beamType = RadBeamType::Temporal;
		}
		BeamType type{ beamType };
		if (!weaponData)
		{
			weaponData = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeapon);
		}
		type.VisualScatter = weaponData->VisualScatter;
		type.VisualScatterMin = weaponData->VisualScatterMin;
		type.VisualScatterMax = weaponData->VisualScatterMax;
		DrawBeam(sourcePos, targetPos, type);
	}
	// IsElectricBolt
	if (pWeapon->IsElectricBolt)
	{
		if (pAttacker && pTarget)
		{
			DrawBolt(pAttacker, pTarget, pWeapon, sourcePos, flh, isOnTurret);
		}
		else
		{
			DrawBolt(sourcePos, targetPos, pWeapon->IsAlternateColor);
		}
	}
}

void AttachedParticleSystem(WeaponTypeClass* pWeapon, CoordStruct sourcePos, CoordStruct targetPos,
	TechnoClass* pAttacker, AbstractClass* pTarget, HouseClass* pAttacingHouse)
{
	if (ParticleSystemTypeClass* psType = pWeapon->AttachedParticleSystem)
	{
		DrawParticle(psType, sourcePos, targetPos, pAttacker, pTarget);
	}
}

void PlayReportSound(WeaponTypeClass* pWeapon, CoordStruct sourcePos)
{
	if (pWeapon->Report.Count > 0)
	{
		int index = Random::RandomRanged(0, pWeapon->Report.Count - 1);
		VocClass::SafeImmedietelyPlayAt(pWeapon->Report.Items[index], sourcePos);
	}
}

void DrawWeaponAnim(ObjectClass* pShooter, TechnoClass* pAttacker, HouseClass* pAttackingHouse, WeaponTypeClass* pWeapon, CoordStruct sourcePos, CoordStruct targetPos)
{
	// Anim
	if (pWeapon->Anim.Count > 0)
	{
		int facing = pWeapon->Anim.Count;
		int index = 0;
		if (facing % 8 == 0)
		{
			CoordStruct tempSourcePos = sourcePos;
			tempSourcePos.Z = 0;
			CoordStruct tempTargetPos = targetPos;
			tempTargetPos.Z = 0;
			DirStruct dir = Point2Dir(tempSourcePos, tempTargetPos);
			index = Dir2FrameIndex(dir, facing);
		}
		if (AnimTypeClass* pAnimType = pWeapon->Anim.get_or_default(index))
		{
			AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, sourcePos);
			SetAnimOwner(pAnim, pAttackingHouse);
			if (!pAnimType->Bouncer && !pAnimType->IsMeteor)
			{
				pAnim->SetOwnerObject(pShooter);
			}
		}
	}
}
#pragma endregion

// ----------------
// 武器选择
// ----------------
#pragma region Weapon Selection

// Checks if the techno can fire a no-ammo weapon with the given index.
bool CanFireNoAmmoWeapon(TechnoClass* pTechno, int weaponIndex)
{
	TechnoTypeClass* pType = pTechno->GetTechnoType();

	if (pType->Ammo > 0)
	{
		TechnoTypeExt::TypeData* typeData = GetTypeData<TechnoTypeExt, TechnoTypeExt::TypeData>(pType);

		return pTechno->Ammo <= typeData->NoAmmoAmount && (typeData->NoAmmoWeapon == weaponIndex || typeData->NoAmmoWeapon == -1);
	}

	return false;
}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int PickWeaponIndex(TechnoClass* pTechno, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	WeaponStruct* pWeaponOne = pTechno->GetWeapon(weaponIndexOne);
	WeaponStruct* pWeaponTwo = pTechno->GetWeapon(weaponIndexTwo);
	if  (!pWeaponOne && !pWeaponTwo)
	{
		// 两个武器都没有
		return -1;
	}
	else if (!pWeaponTwo)
	{
		// 只有第一个武器
		return weaponIndexOne;
	}
	else if (!pWeaponOne)
	{
		// 只有第二个武器
		return weaponIndexTwo;
	}

	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (pTarget && (!pTargetTechno || !pTargetTechno->IsInAir()))
	{
		if (ObjectClass* pObject = flag_cast_to<ObjectClass*, true>(pTarget))
			pTargetCell = pObject->GetCell();
		else if (CellClass* pCell = cast_to<CellClass*, true>(pTarget))
			pTargetCell = pCell;
	}

	// 检查第二个武器能不能射
	WeaponTypeClass* pWeaponTypeTwo = pWeaponTwo->WeaponType;
	WeaponTypeExt::TypeData* dataTwo = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeaponTypeTwo);
	if (!dataTwo->SkipWeaponPicking)
	{
		// 第二武器不能攻击格子
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, dataTwo->CanTarget, true, true))
		{
			return weaponIndexOne;
		}
		// 第二武器不能攻击目标
		if (pTargetTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, dataTwo->CanTarget)
				|| !EnumFunctions::CanTargetHouse(dataTwo->CanTargetHouses, pTechno->Owner, pTargetTechno->Owner)
				// Phobos对低血量目标和PhobosAE的筛选
				|| !IsHealthInThreshold(pTargetTechno, dataTwo->CanTargetMinHealth, dataTwo->CanTargetMaxHealth)
				)
			{
				return weaponIndexOne;
			}
		}
	}

	// 第二武器对空检查
	bool toAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTypeTwo->Projectile->AA;
	if (!allowFallback
		&& (!allowAAFallback || !toAA)
		&& !CanFireNoAmmoWeapon(pTechno, 1))
	{
		return weaponIndexOne;
	}

	// 检查第一个武器能不能射
	WeaponTypeClass* pWeaponTypeOne = pWeaponOne->WeaponType;
	WeaponTypeExt::TypeData* dataOne = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeaponTypeOne);
	if (!dataOne->SkipWeaponPicking)
	{
		// 第一个武器不能攻击格子
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, dataOne->CanTarget, true, true))
		{
			return weaponIndexTwo;
		}
		// 第一个武器不能攻击目标
		if (pTargetTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, dataOne->CanTarget)
				|| !EnumFunctions::CanTargetHouse(dataOne->CanTargetHouses, pTechno->Owner, pTargetTechno->Owner)
				// Phobos对低血量目标和PhobosAE的筛选
				|| !IsHealthInThreshold(pTargetTechno, dataOne->CanTargetMinHealth, dataOne->CanTargetMaxHealth)
				)
			{
				return weaponIndexTwo;
			}
		}
	}

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && pTargetCell)
	{
		TechnoTypeClass* pType = pTechno->GetTechnoType();

		if (pType->NavalTargeting == NavalTargetingType::Naval_primary
			|| pType->LandTargeting == LandTargetingType::Land_secondary)
		{
			LandType landType = pTargetCell->LandType;

			if (landType != LandType::Water && landType != LandType::Beach)
			{
				return weaponIndexTwo;
			}
		}
	}

	return -1;
}
#pragma endregion
