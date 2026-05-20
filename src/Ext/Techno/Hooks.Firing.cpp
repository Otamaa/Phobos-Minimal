#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>

#include <Locomotor/Cast.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <TerrainClass.h>

ASMJIT_PATCH(0x6FDE05, TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0;
} ASMJIT_PATCH_AGAIN(0x6FF933, TechnoClass_FireAt_End, 0x5);

ASMJIT_PATCH(0x6FDD50, TechnoClass_FireAt_PreFire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	//GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(const int, nWeapon, 0x8);
	//GET(AbstractClass*, pTarget, EDI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->CurrentWeaponIdx = nWeapon;

	return 0x0;
}

ASMJIT_PATCH(0x6FF905, TechnoClass_FireAt_FireOnce_A, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pInf = cast_to<InfantryClass*, false>(pThis))
	{
		GET(WeaponTypeClass*, pWeapon, EBX);

		if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->FireOnce_ResetSequence)
			InfantryExtContainer::Instance.Find(pInf)->SkipTargetChangeResetSequence = true;
	}

	return 0;
}

ASMJIT_PATCH(0x6FE337, TechnoClass_FireAt_DamageMult, 0x6)
{
	GET(int, damage, EDI);
	GET(TechnoClass*, pThis, ESI);

	int _damage = (int)TechnoExtData::GetDamageMult(pThis, (double)damage);
	R->Stack(0x28, GET_TECHNOTYPE(pThis));
	R->EDI(_damage);
	R->EAX(_damage);
	return 0x6FE3DF;
}

ASMJIT_PATCH(0x6FED2F, TechnoClass_FireAt_VerticalInitialFacing, 0x6)
{
	enum { Continue = 0x6FED39, SkipGameCode = 0x6FED8F };

	GET(FakeBulletTypeClass*, pBulletType, EAX);

	if (pBulletType->_GetExtData()->VerticalInitialFacing.Get(pBulletType->Voxel || pBulletType->Vertical))
		return Continue;

	return SkipGameCode;
}

ASMJIT_PATCH(0x6FF0DD, TechnoClass_FireAt_TurretRecoil, 0x6)
{
	enum { SkipGameCode = 0x6FF15B };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFSET(0xB0, -0x70));

	if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->TurretRecoil_Suppress)
	{
		GET(TechnoClass* const, pThis, ESI);
		TechnoExtContainer::Instance.Find(pThis)->RecordRecoilData();
	}

	return SkipGameCode;
}

static WeaponStruct* __fastcall TechnoClass_FireAt_GetWeapon_(TechnoClass* pTech, void*, int idx)
{
	return pTech->GetWeapon(TechnoExtContainer::Instance.Find(pTech)->CurrentWeaponIdx);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x6FDD69, TechnoClass_FireAt_GetWeapon_);

ASMJIT_PATCH(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	switch (TechnoExtData::ApplyAreaFire(pThis, pCell, pWeaponType))
	{
	case AreaFireReturnFlag::Continue:
	{
		R->EAX(pCell);
		return Continue;
	}
	case AreaFireReturnFlag::DoNotFire:
	{
		return DoNotFire;
	}
	case AreaFireReturnFlag::SkipSetTarget:
	{
		R->EAX(pThis);
		return SkipSetTarget;
	}
	default:
		return Continue;
		break;
	}
}

ASMJIT_PATCH(0x6FDDC0, TechnoClass_FireAt_Early, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AE.flags.HasOnFireDiscardables)
	{
		for (auto& attachEffect : pExt->PhobosAE)
		{
			if (!attachEffect || attachEffect->ShouldBeDiscarded)
				continue;

			attachEffect->DiscardOnFire();
		}
	}

	// if (pThis->Passengers.FirstPassenger)
	// {
	// 	// TODO : implement this for UnitClass
	// 	pThis->DropOffParadropCargo();
	// }

	if (pWeapon)
	{
		auto pWeaponExt = pWeapon->_GetExtData();

		auto& timer = pExt->DelayedFireTimer;
		if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
			pExt->ResetDelayedFireTimer();

		if (pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
		{
			auto const rtti = pThis->WhatAmI();

			if (pWeaponExt->DelayedFire_PauseFiringSequence && (rtti == AbstractType::Infantry
				|| (rtti == AbstractType::Unit && !pThis->HasTurret() && !GET_TECHNOTYPE(pThis)->Voxel)))
			{
				return 0;
			}

			if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
			{
				if (timer.InProgress())
					return 0x6FDE03;

				if (!timer.HasStarted())
				{
					pExt->DelayedFireWeaponIndex = weaponIndex;
					timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
					auto pAnimType = pWeaponExt->DelayedFire_Animation;

					if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
						pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation.Get();

					auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

					if (pWeaponExt->DelayedFire_AnimOffset.isset())
						firingCoords = pWeaponExt->DelayedFire_AnimOffset;

					pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

					if (pWeaponExt->DelayedFire_InitialBurstSymmetrical)
						pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
							pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, { firingCoords.X, -firingCoords.Y, firingCoords.Z });

					return 0x6FDE03;
				}
				else
				{
					pExt->ResetDelayedFireTimer();
				}
			}
		}

		const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget);

		if (pTargetTechno)
		{
			auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			if (pWeaponExt->NoRepeatFire > 0)
			{
				pTargetExt->LastBeLockedFrame = Unsorted::CurrentFrame;
			}

			if (pWeaponExt->AttachEffect_Enable)
			{
				auto const info = &pWeaponExt->AttachEffects;
				PhobosAttachEffectClass::Attach(pTargetTechno, pThis->Owner, pThis, pWeapon->Warhead, info);
				PhobosAttachEffectClass::Detach(pTargetTechno, info);
				PhobosAttachEffectClass::DetachByGroups(pTargetTechno, info);
			}
		}

		if (pWeapon->Suicide && pThis->IsAlive)
		{
			int scdamage = pThis->Health;
			pThis->ReceiveDamage(&scdamage, 0, RulesClass::Instance->C4Warhead, nullptr, false, true, nullptr);
			return 0x6FDE03;
		}

		if (pWeaponExt->OnlyAttacker.Get() && pTarget == pThis->Target && pTargetTechno)
		{
			const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			pTargetExt->AddFirer(pWeapon, pThis);
		}
	}

	return 0x6FDE0E;
}

ASMJIT_PATCH(0x6FDD7D, TechnoClass_FireAt_UpdateWeaponType, 0x5)
{

	enum { CanNotFire = 0x6FDE03 };

	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(TechnoClass*, pThis, ESI);

	const auto pWH = pWeapon->Warhead;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWeapon->LimboLaunch)
	{
		if (!pWH->Parasite && pWHExt->UnlimboDetonate)
		{
			if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
			{
				if (pFoot->Locomotor->Is_Really_Moving_Now())
					return CanNotFire;
			}
		}
	}

	{
		if (pThis->CurrentBurstIndex && pWeapon != pExt->LastWeaponType && pTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst))
		{
			if (pExt->LastWeaponType && pExt->LastWeaponType->Burst)
			{

				const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pExt->LastWeaponType->Burst;
				const auto rof = static_cast<int>(ratio * pExt->LastWeaponType->ROF * pExt->AE.ROFMultiplier) - (Unsorted::CurrentFrame.get() - pThis->LastFireBulletFrame);

				if (rof > 0)
				{
					pThis->ROF = rof;
					pThis->RearmTimer.Start(rof);
					pThis->CurrentBurstIndex = 0;
					pExt->LastWeaponType = pWeapon;

					return CanNotFire;
				}
			}

			pThis->CurrentBurstIndex = 0;

		}

		pExt->LastWeaponType = pWeapon;
	}

	return 0;
}

namespace UnlimboDetonateFireTemp
{
	BulletClass* Bullet;
	bool InSelected;
	bool InLimbo;
}

ASMJIT_PATCH(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);


	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto pBulletExt = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	const auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	UnlimboDetonateFireTemp::Bullet = ret;
	UnlimboDetonateFireTemp::InSelected = pThis->IsSelected;
	UnlimboDetonateFireTemp::InLimbo = pThis->InLimbo;
	R->EAX(ret);
	return 0x6FE562;
}

ASMJIT_PATCH(0x6FF7FF, TechnoClass_FireAt_UnlimboDetonate, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WarheadTypeClass* const, pWH, EAX);

	const auto pBullet = UnlimboDetonateFireTemp::Bullet;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pThis->IsAlive && pThis->Health > 0 && pBullet
		&& !UnlimboDetonateFireTemp::InLimbo && !pWH->Parasite && pWHExt->UnlimboDetonate)
	{
		if (pWHExt->UnlimboDetonate_KeepSelected)
		{
			TechnoExtContainer::Instance.Find(pThis)->IsSelected = UnlimboDetonateFireTemp::InSelected;
			ScenarioExtData::Instance()->LimboLaunchers.emplace(pThis);
		}

		pBullet->Owner = pThis;
	}

	return 0;
}

ASMJIT_PATCH(0x6FE3E3, TechnoClass_FireAt_OccupyDamageBonus, 0xA) //B
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_STACK(int, nDamage, 0x2C);
	GET_BASE(int, weapon_idx, 0xC);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	auto pExtType = GET_TECHNOTYPEEXT(pThis);

	if (pThis->CanOccupyFire())
	{
		if (auto const Building = cast_to<BuildingClass*, false>(pThis))
		{
			nDamage = int(nDamage * BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier));
		}
		else
		{
			nDamage = int(nDamage * RulesClass::Instance->OccupyDamageMultiplier);
		}
	}

	if (pThis->InOpenToppedTransport)
	{
		nDamage = int(nDamage * pExtType->OpenTransport_DamageMultiplier);

		if (auto const  pTransport = pThis->Transporter)
		{
			float nDamageMult = GET_TECHNOTYPEEXT(pTransport)->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
			nDamage = int(nDamage * nDamageMult);
		}
		else
		{
			nDamage = int(nDamage * RulesClass::Instance->OpenToppedDamageMultiplier);
		}
	}

	if (!pWeapon->DiskLaser)
	{
		R->EDI(nDamage);
		R->Stack(0x2C, nDamage);
		return 0x6FE4F6; // continue check
	}

	auto pDiskLaser = GameCreate<DiskLaserClass>();

	++pThis->CurrentBurstIndex;
	int rearm = pThis->GetROF(weapon_idx);
	TechnoExtData::SetChargeTurretDelay(pThis, rearm, pWeapon);
	pThis->RearmTimer.Start(rearm);
	pThis->CurrentBurstIndex %= pWeapon->Burst;
	((FakeDiskLaserClass*)pDiskLaser)->__Fire(pThis, pTarget, pWeapon, nDamage);
	//pDiskLaser->Fire(pThis, pTarget, pWeapon, nDamage);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0x6FE4E7; //end of func
}

ASMJIT_PATCH(0x6FDFA8, TechnoClass_FireAt_SprayOffsets, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	LEA_STACK(CoordStruct*, pCoord, 0xB0 - 0x28);

	auto pType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->SprayAttack)
	{
		if (pThis->CurrentBurstIndex)
		{
			pThis->SprayOffsetIndex = (pExt->SprayOffsets.size() / pWeapon->Burst + pThis->SprayOffsetIndex) % pExt->SprayOffsets.size();
		}
		else
		{
			pThis->SprayOffsetIndex = ScenarioClass::Instance->Random.RandomRanged(0, pExt->SprayOffsets.size() - 1);
		}

		auto& Coord = pExt->SprayOffsets[pThis->SprayOffsetIndex];
		pCoord->X = (pThis->Location.X + Coord->X);//X
		pCoord->Y = (pThis->Location.Y + Coord->Y);//Y
		R->EAX(pThis->Location.Z + Coord->Z); //Z
		return 0x6FE218;
	}

	return 0x6FE140;
}

ASMJIT_PATCH(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

ASMJIT_PATCH(0x6FF031, TechnoClass_FireAt_ReverseVelocityWhileGravityIsZero, 0xA)
{
	GET(BulletClass*, pBullet, EBX);
	//GET(TechnoClass*, pThis, ESI);

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (pBulletExt->Trajectory &&
		pBulletExt->Trajectory->Flag != TrajectoryFlag::Invalid)
		return 0x0;

	if (pBullet->Type->Arcing && pBulletTypeExt->GetAdjustedGravity() == 0.0)
	{
		pBullet->Velocity *= -1;
		if (pBulletTypeExt->Gravity_HeightFix)
		{
			const auto speed = pBullet->Velocity.Length();

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

			const auto magnitude = pBullet->Velocity.Length();
			pBullet->Velocity *= speed / magnitude;
		}
	}

	return 0;
}

namespace FireAtTemp
{
	BulletClass* FireBullet = nullptr;
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
	AbstractClass* pWaveOwnerTarget = nullptr;
}

ASMJIT_PATCH(0x6FF08B, TechnoClass_FireAt_RecordBullet, 0x6)
{
	GET(BulletClass*, pBullet, EBX);
	FireAtTemp::FireBullet = pBullet;
	return 0;
}

ASMJIT_PATCH(0x6FF15F, TechnoClass_FireAt_Additionals_Start, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);

	REF_STACK(CoordStruct, crdSrc, 0xB0 - 0x6C);
	REF_STACK(CoordStruct, crdTgt, 0xB0 - 0x28);
	REF_STACK(CoordStruct, railgunCrrd_1, 0xB0 - 0x1C);

	GET_BASE(AbstractClass*, pOriginalTarget, 0x8);

	GET_BASE(int, weaponIdx, 0xC);

	auto coords = pOriginalTarget->GetCenterCoords();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pBuilding = cast_to<BuildingClass*, false>(pOriginalTarget))
		coords = pBuilding->GetTargetCoords();

	// This is set to a temp variable as well, as accessing it everywhere needed from TechnoExt would be more complicated.
	FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(crdSrc, coords, pWeapon->Projectile, pThis->Owner);
	pExt->FiringObstacleCell = FireAtTemp::pObstacleCell;

	R->Stack(0x10, &crdSrc);

	if (pWeapon->UseFireParticles && !pThis->Sys.Fire && pWeapon->AttachedParticleSystem)
	{
		pThis->Sys.Fire = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->UseSparkParticles && !pThis->Sys.Spark && pWeapon->AttachedParticleSystem)
	{
		pThis->Sys.Spark = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->AttachedParticleSystem && (pWeapon->_GetExtData()->IsDetachedRailgun || (pWeapon->IsRailgun && !pThis->Sys.Railgun)))
	{
		auto coord = pThis->DealthParticleDamage(&railgunCrrd_1, &crdSrc, pOriginalTarget, pWeapon);
		auto pRailgun = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, &crdSrc,
			nullptr, pThis, coord, nullptr);

		if (!pWeapon->_GetExtData()->IsDetachedRailgun)
			pThis->Sys.Railgun = pRailgun;
	}

	++pThis->CurrentBurstIndex;
	int ROF = pThis->GetROF(weaponIdx);

	if (pThis->Berzerk)
	{
		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
		const double multiplier = pTypeExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
		ROF = static_cast<int>(ROF * multiplier);
	}

	TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);

	pThis->RearmTimer.Start(ROF);

	// Issue #46: Laser is mirrored relative to FireFLH
	// Author: Starkku
	--pThis->CurrentBurstIndex;

	AnimTypeClass* pFiringAnim = nullptr;
	auto pWeaponExt = pWeapon->_GetExtData();

	if (pThis->CanOccupyFire())
	{
		if (pWeaponExt->OccupantAnim_UseMultiple.Get() && !pWeaponExt->OccupantAnims.empty())
		{
			if (pWeaponExt->OccupantAnims.size() == 1)
			{
				pFiringAnim = pWeaponExt->OccupantAnims[0];
			}
			else
			{
				pFiringAnim = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
			}

		}
		else
		{
			pFiringAnim = pWeapon->OccupantAnim;
		}

	}
	else
	{

		if (pWeapon->Anim.Count > 0)
		{
			int nIdx = -1;

			if (pWeapon->Anim.Count == 1)
				nIdx = 0;
			else
			{

				DirStruct facing {};
				pThis->GetRealFacing(&facing);

				if (pWeapon->Anim.Count == 8)
				{
					nIdx = (facing.GetFacing<8>() + 8 / 8) % 8;
				}
				else if (pWeapon->Anim.Count == 16)
				{
					nIdx = (facing.GetFacing<16>() + 16 / 8) % 16;
				}
				else if (pWeapon->Anim.Count == 32)
				{
					nIdx = (facing.GetFacing<32>() + 32 / 8) % 32;
				}
				else if (pWeapon->Anim.Count == 64)
				{
					nIdx = (facing.GetFacing<64>() + 64 / 8) % 64;
				}
				else
				{
					//only execute if the anim count is more than 1
					const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

					// 2^highest is the frame count, 3 means 8 frames
					if (highest >= 3)
					{
						nIdx = facing.GetValue(highest, 1u << (highest - 3));
					}

					nIdx %= pWeapon->Anim.Count;
				}
			}

			pFiringAnim = pWeapon->Anim.Items[nIdx];
		}
	}

	if (pWeapon->Report.Count > 0 && !GET_TECHNOTYPE(pThis)->IsGattling)
	{
		if (pWeapon->Report.Count == 1)
		{
			VocClass::SafeImmedietelyPlayAt(pWeapon->Report[0], &crdSrc, nullptr);
		}
		else
		{
			auto v116 = pThis->weapon_sound_randomnumber_3C8 % pWeapon->Report.Count;
			VocClass::SafeImmedietelyPlayAt(pWeapon->Report[v116], &crdSrc, nullptr);
		}
	}

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? crdSrc : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		{
			auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord);
			AnimExtData::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false, false);

			if (pThis->WhatAmI() != BuildingClass::AbsID)
			{
				pFeedBackAnim->SetOwnerObject(pThis);
			}
			else
			{
				if (pThis->GetOccupantCount() > 0)
				{
					pFeedBackAnim->ZAdjust = -200;
				}
				else
				{
					auto rend = pThis->GetRenderCoords();
					pFeedBackAnim->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
				}
			}
		}
	}

	if (!pThis->IsAlive)
	{
		return 0x6FF92F;
	}

	if (pFiringAnim)
	{
		auto pFiring = GameCreate<AnimClass>(pFiringAnim, crdSrc, 0, 1, AnimFlag::AnimFlag_600, 0, 0);
		AnimExtData::SetAnimOwnerHouseKind(pFiring, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false, false);
		auto pAnimExt = AnimExtContainer::Instance.Find(pFiring);

		if (pWeapon->_GetExtData()->Anim_Update.Get(RulesExtData::Instance()->FiringAnim_Update))
		{
			pAnimExt->FromWeapon = pWeapon;
			pAnimExt->FromWeaponIdx = weaponIdx;
			pAnimExt->FromBurstIdx = pThis->CurrentBurstIndex;
		}
		// if (pThis->WhatAmI() != BuildingClass::AbsID)
		// {
		// 	pFiring->SetOwnerObject(pThis);
		// } else
		{
			if (pThis->GetOccupantCount() > 0)
			{
				pFiring->ZAdjust = -200;
			}
			else
			{
				auto rend = pThis->GetRenderCoords();
				pFiring->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
			}
		}
	}

	if (!pThis->IsAlive)
	{
		return 0x6FF92F;
	}

#ifndef PERFORMANCE_HEAVY
	//TargetSet
	FireAtTemp::originalTargetCoords = crdTgt;
	FireAtTemp::pOriginalTarget = pOriginalTarget;

	if (FireAtTemp::pObstacleCell)
	{
		crdTgt = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
		R->Base(8, FireAtTemp::pObstacleCell); // Replace original target so it gets used by Ares sonic wave stuff etc. as well.
		pOriginalTarget = FireAtTemp::pObstacleCell;
	}
#endif

	//FeedbackWeapon
	if (auto fbWeapon = pWeaponExt->FeedbackWeapon.Get())
	{
		if (!pThis->InOpenToppedTransport || fbWeapon->FireInTransport)
		{
			WeaponTypeExtData::DetonateAt1(fbWeapon, pThis, pThis, true, nullptr);
			//pThis techno was die after after getting affect of FeedbackWeapon
			//if the function not bail out , it will crash the game because the vtable is already invalid
			if (!pThis->IsAlive)
			{
				return 0x6FF92F;
			}
		}
	}

	if (pExt->AE.flags.HasFeedbackWeapon)
	{
		for (auto const& pAE : pExt->PhobosAE)
		{

			if (!pAE || !pAE->IsActive())
				continue;

			if (auto const pWeaponFeedback = pAE->GetType()->FeedbackWeapon)
			{
				if (pThis->InOpenToppedTransport && !pWeaponFeedback->FireInTransport)
					return 0;

				WeaponTypeExtData::DetonateAt1(pWeaponFeedback, pThis, pThis, true, nullptr);
			}
		}

		//pThis techno was die after after getting affect of FeedbackWeapon
		//if the function not bail out , it will crash the game because the vtable is already invalid
		if (!pThis->IsAlive)
		{
			return 0x6FF92F;
		}
	}

	// if(pWeapon->IsSonic){
	// 	pThis->Wave = WaveExtData::Create(crdSrc, crdTgt, pThis, WaveType::Sonic, pOriginalTarget, pWeapon);
	// }

	return 0x6FF48A;
}

ASMJIT_PATCH(0x6FF5F5, TechnoClass_FireAt_Additionals_End, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass*, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));
	GET_BASE(int, weaponIndex, 0xC);
	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct, pTargetCoords, 0x88);

	auto const pData = WeaponTypeExtContainer::Instance.Find(pWeaponType);
	//TechnoClass_Fire_OtherWaves
	if (pData->IsWave() && !pThis->Wave)
	{
		WaveType nType = WaveType::Sonic;
		if (pWeaponType->IsMagBeam)
			nType = WaveType::Magnetron;
		else
			nType = pData->Wave_IsBigLaser
			? WaveType::BigLaser : WaveType::Laser;

		pThis->Wave = WaveExtData::Create(crdSrc, pTargetCoords, pThis, nType, pTarget, pWeaponType);
	}

	//remove ammo rounds depending on weapon
	TechnoExtData::DecreaseAmmo(pThis, pWeaponType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

#ifndef PERFORMANCE_HEAVY
	// Restore original target & coords
	pTargetCoords = FireAtTemp::originalTargetCoords;
	R->Base(8, FireAtTemp::pOriginalTarget);
	pTarget = FireAtTemp::pOriginalTarget;
	R->EDI(FireAtTemp::pOriginalTarget);

	// Reset temp values
	FireAtTemp::originalTargetCoords = CoordStruct::Empty;
	FireAtTemp::FireBullet = nullptr;
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;
#endif

	//TechnoClass_FireAt_ToggleLaserWeaponIndex
	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeaponType->IsLaser)
	{
		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	//TechnoClass_FireAt_BurstOffsetFix_2
	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pWeaponType->Burst;

	if (pExt->ForceFullRearmDelay)
	{
		pExt->ForceFullRearmDelay = false;
		pThis->CurrentBurstIndex = 0;
	}

	if (auto const pTargetObject = cast_to<BulletClass* const, false>(pTarget))
	{
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor())
		{
			auto pBulletTargetExt = BulletExtContainer::Instance.Find(pTargetObject);
			auto pBulletTypeTargetExt = BulletTypeExtContainer::Instance.Find(pTargetObject->Type);

			if (!pBulletTypeTargetExt->Armor.isset())
				pBulletTargetExt->InterceptedStatus |= InterceptedStatus::Locked;

			const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

			pBulletExt->InterceptorTechnoType = GET_TECHNOTYPE(pThis);
			pBulletExt->InterceptedStatus |= InterceptedStatus::Targeted;

			if (!pTypeExt->Interceptor_ApplyFirepowerMult)
				pBullet->Health = pWeaponType->Damage;
		}
	}

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);

	if (!Phobos::Config::HideShakeEffects)
	{
		if (!pWeaponExt->ShakeLocal.Get() || pThis->IsOnMyView())
		{
			if (pWeaponExt->Xhi || pWeaponExt->Xlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

			if (pWeaponExt->Yhi || pWeaponExt->Ylo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));
		}
	}
	return 0x6FF660;
}

ASMJIT_PATCH(0x6FF48D, TechnoClass_FireAt_IsLaser, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EDI);
	GET(WeaponTypeClass* const, pFiringWeaponType, EBX);
	GET_BASE(int, idxWeapon, 0xC);// don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	auto pType = GET_TECHNOTYPE(pThis);
	if (pType->TargetLaser && pThis->Owner->ControlledByCurrentPlayer())
	{

		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

		if (pTypeExt->TargetLaser_WeaponIdx.empty()
			|| pTypeExt->TargetLaser_WeaponIdx.Contains(idxWeapon))
		{
			pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
		}
	}

	if (pFiringWeaponType->IsLaser)
	{
		auto const pData = WeaponTypeExtContainer::Instance.Find(pFiringWeaponType);
		int const Thickness = pData->Laser_Thickness;

		if (auto const pBld = cast_to<BuildingClass*, false>(pThis))
		{	//ToggleLaserWeaponIndex

			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = idxWeapon;
			else
				pExt->CurrentLaserWeaponIndex.clear();

			auto const pTWeapon = pBld->GetPrimaryWeapon()->WeaponType;

			if (auto const pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty))
			{

				//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
				pLaser->Thickness = Thickness == -1 ? 3 : Thickness;
				auto const pBldTypeData = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				if (pBldTypeData->PrismForwarding.CanAttack())
				{
					//is a prism tower

					if (pBld->SupportingPrisms > 0)
					{ //Ares sets this to the longest backward chain
						//is being supported... so increase beam intensity
						if (pBldTypeData->PrismForwarding.Intensity < 0)
						{
							pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
						}
						else if (pBldTypeData->PrismForwarding.Intensity > 0)
						{
							pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
						}

						// always supporting
						pLaser->IsSupported = true;
					}
				}
			}
		}
		else
		{
			if (auto const pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, CoordStruct::Empty))
			{
				if (Thickness == -1)
				{
					pLaser->Thickness = 2;
				}
				else
				{
					pLaser->Thickness = Thickness;

					// required for larger Thickness to work right
					pLaser->IsSupported = (Thickness > 3);
				}
			}
		}

		// skip all default handling
		return 0x6FF656;
	}

	//other affects
	return 0x6FF57D;
}

ASMJIT_PATCH(0x6FF008, TechnoClass_FireAt_FSW, 8)
{
	REF_STACK(CoordStruct const, src, 0x44);
	REF_STACK(CoordStruct const, tgt, 0x88);

	const DWORD origin = R->Origin();
	auto const Bullet = origin == 0x6FF860
		? R->EDI<FakeBulletClass*>()
		: R->EBX<FakeBulletClass*>()
		;

	if (origin != 0x6FF860)
	{

		GET_STACK(CoordStruct, crdOffset, STACK_OFFSET(0xB0, -0x1C));
		GET_STACK(CoordStruct, fireCoords, STACK_OFFSET(0xB0, -0x6C));

		const auto crdTgt = crdOffset + fireCoords;
		if (Bullet->Type->Arcing && !Bullet->_GetTypeExtData()->Arcing_AllowElevationInaccuracy)
		{
			REF_STACK(VelocityClass, velocity, STACK_OFFSET(0xB0, -0x60));
			REF_STACK(CoordStruct, crdSrc, STACK_OFFSET(0xB0, -0x6C));

			Bullet->_GetExtData()->ApplyArcingFix(crdSrc, crdTgt, velocity);
		}
	}

	if (!HouseExtContainer::Instance.IsAnyFirestormActive || !Bullet->Type->IgnoresFirestorm)
	{
		return 0;
	}

	auto const crd = MapClass::Instance->FindFirstFirestorm(src, tgt, Bullet->Owner->Owner);

	if (crd.IsValid())
	{
		Bullet->Target = MapClass::Instance->GetCellAt(crd)->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x6FF860, TechnoClass_FireAt_FSW, 8)

ASMJIT_PATCH(0x6FF923, TechnoClass_FireAt_FireOnce_B, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	pThis->SetTarget(nullptr);
	if (auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (pUnit->Type->DeployFire
			&& !pUnit->Type->IsSimpleDeployer
			&& !pUnit->Deployed
			&& pThis->CurrentMission == Mission::Unload
		)
		{
			TechnoExtContainer::Instance.Find(pUnit)->DeployFireTimer.Start(pWeapon->ROF);
		}
	}

	return 0x6FF92F;
}

ASMJIT_PATCH(0x6FE31C, TechnoClass_FireAt_AllowDamage, 8)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	// whether conventional damage should be used
	const bool applyDamage =
		WeaponTypeExtContainer::Instance.Find(pWeapon)->ApplyDamage.Get(!pWeapon->IsSonic && !pWeapon->UseFireParticles);

	if (!applyDamage)
	{
		// clear damage
		R->EDI(0);
		return 0x6FE3DFu;
	}

	return 0x6FE32Fu;
}

ASMJIT_PATCH(0x6FE709, TechnoClass_FireAt_BallisticScatter1, 6)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for FlakScatter && !Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(0));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE71C;
}

ASMJIT_PATCH(0x6FE7FE, TechnoClass_FireAt_BallisticScatter2, 5)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for !FlakScatter || Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(RulesClass::Instance->BallisticScatter / 2));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE821;
}

//=======================================================================================================================
// An example for quick tilting test
ASMJIT_PATCH(0x7413DD, UnitClass_FireAt_RecoilForce, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	if (!pThis->IsVoxel())
		return 0;

	GET(BulletClass* const, pTraj, EDI);

	const auto& force = WeaponTypeExtContainer::Instance.Find(pTraj->WeaponType)->RecoilForce;

	if (!force.isset() || Math::abs(force.Get()) < 0.005)
		return 0x0;

	double force_result = force / MaxImpl(pThis->Type->Weight, 1.);

	if (Math::abs(force.Get()) < 0.002)
		return 0;

	const double theta = pThis->GetRealFacing().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>();

	pThis->RockingForwardsPerFrame = (float)(-force_result * Math::cos(theta));
	pThis->RockingSidewaysPerFrame = (float)(force_result * Math::sin(theta) * Math::pow(pThis->Type->VoxelScaleX / pThis->Type->VoxelScaleY, 2.0));

	return 0;
}

ASMJIT_PATCH(0x51B20E, InfantryClass_AssignTarget_FireOnce, 0x6)
{
	enum { SkipGameCode = 0x51B255 };

	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);

	auto const pExt = InfantryExtContainer::Instance.Find(pThis);

	if (!pTarget && pExt->SkipTargetChangeResetSequence)
	{
		pThis->IsFiring = false;
		pExt->SkipTargetChangeResetSequence = false;
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x772AA2, WeaponTypeClass_AllowedThreats_AAOnly, 0x5)
{
	GET(BulletTypeClass* const, pType, ECX);

	if (BulletTypeExtContainer::Instance.Find(pType)->AAOnly) {
		R->EAX(4);
		return 0x772AB3;
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
ASMJIT_PATCH(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = GET_TECHNOTYPE(pThis);

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);

		if (pThisType->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = GET_TECHNOTYPE(pPassenger)->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else //if (pPassenger->HasTurret())
					//tWeaponIndex = pPassenger->CurrentWeaponNumber;
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				const auto pTWeapon = pPassenger->GetWeapon(tWeaponIndex);

				if (pTWeapon->WeaponType && pTWeapon->WeaponType->FireInTransport)
				{
					int TWeaponRange = WeaponTypeExtData::GetRangeWithModifiers(pTWeapon->WeaponType, pPassenger);

					if (TWeaponRange < smallestRange) {
						smallestRange = TWeaponRange;
					}
				}

				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	if (result == 0 && pThis->WhatAmI() == AircraftClass::AbsID && pThisType->OpenTopped)
	{
		result = pThisType->GuardRange;
		if (result == 0)
			Debug::LogInfo("Warning ! , range of Aircraft[{}] return 0 result will cause Aircraft to stuck ! ", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

#pragma region WallWeaponStuff

ASMJIT_PATCH(0x51C1F1, InfantryClass_CanEnterCell_WallWeapon, 0x5)
{
	enum { SkipGameCode = 0x51C1FE };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x73F495, UnitClass_CanEnterCell_WallWeapon, 0x6)
{
	enum { SkipGameCode = 0x73F4A1 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x70095A, TechnoClass_WhatAction_WallWeapon, 0x6) {
	GET_STACK(OverlayTypeClass*, pOverlayTypeClass, STACK_OFFSET(0x2C, -0x18));
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));
	return 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F8C10, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9572, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F971F, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9904, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9AB8, FakeTechnoClass::_EvaluateJustCell);
#pragma endregion

ASMJIT_PATCH(0x6FCDD2, TechnoClass_AssignTarget_Changed, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pNewTarget, EDI);

	if (!pNewTarget)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		pExt->ResetDelayedFireTimer();
	}

	return 0;
}

//===========================================================================================================================
// https://github.com/Phobos-developers/Phobos/pull/825
// Todo :  Otamaa : massive FPS drops !
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Adjust target coordinates for laser drawing.
ASMJIT_PATCH(0x6FD38D, TechnoClass_DrawSth_Coords, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	const auto pBullet = FireAtTemp::FireBullet;

	if (pBullet && pBullet->WeaponType)
	{
		// The weapon may not have been set up
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType);
		// pBullet->Data.Location (0x4E1130) -> pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords
		CoordStruct _targetCoords= pBullet->GetDestinationCoords();
		if (pWeaponExt && pWeaponExt->VisualScatter)
		{
			const auto pRulesExt = RulesExtData::Instance();
			auto min = pWeaponExt->VisualScatter_Min.Get(pRulesExt->VisualScatter_Min);
			auto max = pWeaponExt->VisualScatter_Max.Get(pRulesExt->VisualScatter_Max);
			const auto radius = ScenarioClass::Instance->Random.RandomRanged(min, max);
			*pTargetCoords = MapClass::GetRandomCoordsNear(_targetCoords, radius, false);
		}
		else
		{
			*pTargetCoords = _targetCoords;
		}
	} else if (FireAtTemp::pObstacleCell)
		*pTargetCoords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();

	R->EAX(pTargetCoords);
	return 0;
}ASMJIT_PATCH_AGAIN(0x6FD70D, TechnoClass_DrawSth_Coords, 0x6) // CreateRBeam
ASMJIT_PATCH_AGAIN(0x6FD514, TechnoClass_DrawSth_Coords, 0x7) // CreateEBolt

// Cut railgun logic off at obstacle coordinates.
ASMJIT_PATCH(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { Continue = 0x70CA79, Stop = 0x70CAD8 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	if (MapClass::Instance->GetCellAt(coords) == FireAtTemp::pObstacleCell)
		return Stop;

	return Continue;
}

// WaveClass requires the firer's target and wave's target to match so it needs bit of extra handling here for obstacle cell targets.
ASMJIT_PATCH(0x762AFF, WaveClass_WaveAI_TargetSet, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (pThis->Target && pThis->Owner)
	{
		auto const pObstacleCell = TechnoExtContainer::Instance.Find(pThis->Owner)->FiringObstacleCell;

		if (pObstacleCell == pThis->Target && pThis->Owner->Target)
		{
			FireAtTemp::pWaveOwnerTarget = pThis->Owner->Target;
			pThis->Owner->Target = pThis->Target;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x762D57, WaveClass_Wave_AI_TargetUnset, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (FireAtTemp::pWaveOwnerTarget)
	{
		if (pThis->Owner->Target)
			pThis->Owner->Target = FireAtTemp::pWaveOwnerTarget;

		FireAtTemp::pWaveOwnerTarget = nullptr;
	}

	return 0;
}
//===========================================================================================================================
