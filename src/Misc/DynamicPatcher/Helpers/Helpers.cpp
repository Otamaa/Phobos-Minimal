#ifdef COMPILE_PORTED_DP_FEATURES
#include "Helpers.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>

CoordStruct Helpers_DP::GetFLHAbsoluteCoords(TechnoClass* pTechno, CoordStruct& flh, bool isOnTurret, int flipY, bool nextFrame)
{
	CoordStruct turretOffset {};
	auto const pType = pTechno->GetTechnoType();

	if (isOnTurret)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType))
		{
			turretOffset = pTypeExt->TurretOffset;
		}
		else
		{
			turretOffset.X = pType->TurretOffset;
		}
	}
	return GetFLHAbsoluteCoords(pTechno, flh, isOnTurret, flipY, turretOffset, nextFrame);
}

TechnoClass* Helpers_DP::CreateAndPutTechno(TechnoTypeClass* pType, HouseClass* pHouse, CoordStruct& location, CellClass* pCell , bool bPathfinding)
{
	if (pType)
	{
		auto const pTechno = generic_cast<TechnoClass*>(pType->CreateObject(pHouse));
		bool UnlimboSuccess = false;

		if (!pCell && location != CoordStruct::Empty)
			pCell = MapClass::Instance->GetCellAt(location);

		if (pCell)
		{
			const auto occFlags = pCell->OccupationFlags;

			if(!bPathfinding) {
				pTechno->OnBridge = pCell->ContainsBridge();
				++Unsorted::IKnowWhatImDoing;
				UnlimboSuccess = pTechno->Unlimbo(pCell->GetCoordsWithBridge(), static_cast<DirType>(DirTypes::DIR_E));
				--Unsorted::IKnowWhatImDoing;
			} else {

				if (pType->WhatAmI() == AbstractType::BuildingType) {
					if (!pCell->CanThisExistHere(pType->SpeedType, static_cast<BuildingTypeClass*>(pType), pHouse)) {
						location = MapClass::Instance->GetRandomCoordsNear(location, 0, false);
						pCell = MapClass::Instance->GetCellAt(location);
					}
				}
				
				pTechno->OnBridge = pCell->ContainsBridge();
				UnlimboSuccess = pTechno->Unlimbo(pCell->GetCoordsWithBridge(), static_cast<DirType>(DirTypes::DIR_E));
			}

			if (UnlimboSuccess)
			{
				pCell->OccupationFlags = occFlags;
				pTechno->SetLocation(location);
				return pTechno;
			}
			else
			{
				Debug::Log("Gifbox - Failed To Place Techno [%s] ! \n", pType->ID);

				if (pTechno)
				{
					TechnoExt::HandleRemove(pTechno);
				}
			}
		}
	}

	return nullptr;
}

void Helpers_DP::FireWeaponTo(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, FireBulletToTarget callback,const CoordStruct& bulletSourcePos, bool radialFire, int splitAngle)
{
	if (!pWeapon)
		return;

	if (!pTarget)
		return;

	CoordStruct targetPos {};
	if (auto const pFoot = generic_cast<FootClass*>(pTarget))
		targetPos = CellClass::Cell2Coord(pFoot->GetDestinationMapCoords());
	else
		targetPos = pTarget->GetCoords();

	if(auto const pCell  = MapClass::Instance->GetCellAt(targetPos))
		targetPos.Z = pCell->GetFloorHeight(Point2D::Empty);

	// radial fire
	int burst = pWeapon->Burst;
	RadialFireHelper radialFireHelper { pShooter, burst, splitAngle };
	int flipY = -1;

	for (int i = 0; i < burst; i++)
	{
		VelocityClass bulletVelocity { };
		if (radialFire) {
			flipY = (i < burst / 2.0f) ? -1 : 1;
			bulletVelocity = radialFireHelper.GetBulletVelocity(i);
		} else {
			flipY *= -1;
		}

		CoordStruct sourcePos = bulletSourcePos;

		if (!bulletSourcePos)
		{
			CoordStruct nFLh_ = flh;
			sourcePos = GetFLHAbsoluteCoords(pShooter, nFLh_, true, flipY);
		}

		if (!bulletVelocity)
		{
			bulletVelocity = GetBulletVelocity(sourcePos, targetPos);
		}

		auto pBullet = FireBulletTo(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);

		if (callback && pBullet) {
			callback(i, burst, pBullet, pTarget);
		}

	}
}

BulletClass* Helpers_DP::FireBulletTo(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity)
{
	if (!pWeapon)
		return nullptr;

	if (!pTarget || !pAttacker->IsAlive)
		return nullptr;

	// Fire weapon
	auto const pBullet = FireBullet(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);
	// Draw bullet effect
	DrawBulletEffect(pWeapon, sourcePos, targetPos, pAttacker, pTarget);
	// Draw particle system
	AttachedParticleSystem(pWeapon, sourcePos, pTarget, pAttacker, targetPos);
	// Play report sound
	PlayReportSound(pWeapon, sourcePos);
	// Draw weapon anim
	DrawWeaponAnim(pWeapon, sourcePos, targetPos, pAttacker, pTarget);
	return pBullet;
}

BulletClass* Helpers_DP::FireBullet(TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, VelocityClass& bulletVelocity)
{
	if (!pWeapon)
		return nullptr;

	double fireMult = 1;

	if (pAttacker && pAttacker->IsAlive && !pAttacker->IsCrashing && !pAttacker->IsSinking)
	{
		// check spawner
		auto pSpawn = pAttacker->SpawnManager;
		if (pWeapon->Spawner && pSpawn)
		{
			pSpawn->SetTarget(pTarget);
			return nullptr;
		}

		// check Abilities FIREPOWER
		fireMult = GetDamageMult(pAttacker);
	}

	int damage = static_cast<int>(pWeapon->Damage * fireMult);
	auto pWH = pWeapon->Warhead;
	int speed = pWeapon->GetWeaponSpeed(sourcePos, targetPos);
	bool bright = pWeapon->Bright || pWH->Bright;

	auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);
	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pAttacker, damage, pWH, speed, pExt->GetProjectileRange(), bright);

	if (!pBullet)
		return nullptr;

	pBullet->SetWeaponType(pWeapon);
	pBullet->MoveTo(sourcePos, bulletVelocity);

	if (pWeapon->Projectile->Inviso && !pWeapon->Projectile->Airburst)
	{
		pBullet->Detonate(targetPos);
		pBullet->UnInit();

		return nullptr;
	}

	return pBullet;
}

void Helpers_DP::DrawWeaponAnim(WeaponTypeClass* pWeapon, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pOwner, AbstractClass* pTarget)
{ 
	if (!pWeapon)
		return;

	// Anim
	if (pWeapon->Anim.Count > 0)
	{
		int facing = pWeapon->Anim.Count;
		int index = 0;
		if (facing % 8 == 0)
		{
			auto nDir = Point2Dir(sourcePos, targetPos);
			index = Dir2FacingIndex(nDir, facing);
			index = (int)(facing / 8) + index;
			if (index >= facing)
			{
				index = 0;
			}
		}

		if (auto pAnimType = pWeapon->Anim.GetItem(index))
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, sourcePos))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner ? pOwner->GetOwningHouse() : nullptr, pTarget ? pTarget->GetOwningHouse() : nullptr, pOwner, false);
			}
		}
	}
}

#endif