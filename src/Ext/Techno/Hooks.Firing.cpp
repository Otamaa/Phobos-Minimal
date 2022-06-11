#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Weapon Selection
DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { ReturnGameCode = 0x6F3341, ReturnHandled = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (!pThis)
		Debug::FatalErrorAndExit(__FUNCTION__" Has Missing TechnoPointer ! \n");

	if (pTarget && pTarget->WhatAmI() == AbstractType::Bullet)
	{
		if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pTypeExt->Interceptor.Get())
			{
				R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
				return ReturnHandled;
			}
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());

	return ReturnGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
		{
			if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true))
				return Secondary;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x8)
{
	enum {
		ReturnHandled = 0x6F37AF
	};

	GET(TechnoClass*, pTechno, ECX);

	if (pTechno && pTechno->Target)
	{
		auto pTechnoType = pTechno->GetTechnoType();
		if (!pTechnoType)
			return 0;

		auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target);
		if (!pTarget)
			return 0;

		auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType))
		{
			if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0
				&& pTargetType->Cloakable && pTargetType->Naval
				&& pTarget->CloakState == CloakState::Uncloaked)
			{
				R->EAX(pTechnoTypeExt->ForceWeapon_Naval_Decloaked.Get());
				return ReturnHandled;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (const auto pSecondary = pThis->GetWeapon(1))
		{
			if (const auto pSecondaryExt = WeaponTypeExt::ExtMap.Find(pSecondary->WeaponType))
			{
				if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondaryExt->CanTarget, true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondaryExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pSecondaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
				{
					return Primary;
				}

				if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
				{
					if (pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
						return Primary;

					if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pPrimaryExt->CanTarget, true)) ||
						(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pPrimaryExt->CanTarget) ||
							!EnumFunctions::CanTargetHouse(pPrimaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
					{
						return Secondary;
					}
				}
			}
		}

		if (!pTargetTechno)
			return Primary;

		if (const auto pTargetExt = TechnoExt::GetExtData(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive())
				{
					if (pThis->GetWeapon(1) && !(pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1)))
					{
						if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
							return Secondary;
						else
							return FurtherCheck;
					}

					return Primary;
				}
			}
		}
	}

	return OriginalCheck;
}

DEFINE_HOOK(0x52190D, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x7)
{
	GET(InfantryClass*, pThis, ESI);
	return pThis->Type->DeployFireWeapon == -1 ? 0x52194E : 0x0;
}

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	enum { CannotFire = 0x6FCB7E };

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		const int nMoney = pWHExt->TransactMoney;
		if (!pThis->Owner->CanTransactMoney(nMoney))
			return CannotFire;
	}
	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* targetCell = nullptr;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				targetCell = pObject->GetCell();
		}

		if (targetCell)
		{
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
				return CannotFire;
		}

		if (pTechno)
		{
			if (auto pUnit = specific_cast<UnitClass*>(pTechno))
				if (pUnit->DeathFrameCounter > 0)
					return CannotFire;

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}
		}
	}

	return 0;
}

// Weapon Firing
DEFINE_HOOK(0x6FE43B, TechnoClass_FireAt_OpenToppedDmgMult, 0x7)
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		if (auto pTransport = pThis->Transporter)
		{
			if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
			{
				float nDamageMult = pExt->OpenTopped_DamageMultiplier.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
				R->EAX(Game::F2I(nDamage * nDamageMult));
				return ApplyDamageMult;
			}
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x7)
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	if (auto pExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / Unsorted::d_LeptonsPerCell;

			std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
			size_t size = adjacentCells.size();

			for (unsigned int i = 0; i < size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = pCell->MapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

				if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget, pExt->CanTargetHouses, pThis->Owner, true))
				{
					R->EAX(tgtCell);
					return Continue;
				}
			}

			return DoNotFire;
		}
		else if (pExt->AreaFire_Target == AreaFireTarget::Self)
		{
			if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
				return DoNotFire;

			R->EAX(pThis);
			return SkipSetTarget;
		}

		if (!EnumFunctions::AreCellAndObjectsEligible(pCell, pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
			return DoNotFire;
	}

	return Continue;
}

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (pWeaponExt->FeedbackWeapon.isset())
		{
			auto fbWeapon = pWeaponExt->FeedbackWeapon.Get();

			if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
				return 0;

			WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_Interceptor, 0x6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));

	if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
	{
		auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());

		if (pSourceTypeExt && pSourceTypeExt->Interceptor.Get())
		{
			if (auto const pBulletExt = BulletExt::GetExtData(pBullet))
			{
				pBulletExt->IsInterceptor = true;
			}

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			{
				if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead))
					pWHExt->InterceptBullets(pSource, pWeaponType, pTargetObject->Location);
			}
		}
	}

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pWeaponExt->ShakeLocal.Get() && !pSource->IsOnMyView())
			return 0x0;

		if (pWeaponExt->Xhi || pWeaponExt->Xlo)
			Map.ScreenShakeX = abs(ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

		if (pWeaponExt->Yhi || pWeaponExt->Ylo)
			Map.ScreenShakeY = abs(ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));
	}

	return 0;
}

DEFINE_HOOK(0x6F7481, TechnoClass_Targeting_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EDX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6F74A4;
}

DEFINE_HOOK(0x6FDAA6, TechnoClass_FireAngle_6FDA00_ApplyGravity, 0x5)
{
	GET(WeaponTypeClass* const, pWeaponType, EDI);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6FDACE;
}

DEFINE_HOOK(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

DEFINE_HOOK(0x6FF031, TechnoClass_FireAt_ReverseVelocityWhileGravityIsZero, 0xA)
{
	GET(BulletClass*, pBullet, EBX);

	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (auto pTraj = pData->TrajectoryType)
		if (pTraj->Flag != TrajectoryFlag::Invalid)
			return 0x0;

	if (pBullet->Type->Arcing && BulletTypeExt::GetAdjustedGravity(pBullet->Type) == 0.0)
	{
		pBullet->Velocity *= -1;
		if (pData->Gravity_HeightFix)
		{
			auto speed = pBullet->Velocity.Magnitude();

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

			auto magnitude = pBullet->Velocity.Magnitude();
			pBullet->Velocity *= speed / magnitude;
		}
	}

	return 0;
}

DEFINE_HOOK(0x415F5C, AircraftClass_FireAt_SpeedModifiers, 0xA)
{
	GET(AircraftClass*, pThis, EDI);

	if (pThis->Type->Locomotor == LocomotionClass::CLSIDs::Fly)
	{
		if (const auto pLocomotor = static_cast<FlyLocomotionClass*>(pThis->Locomotor.get()))
		{
			double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
				TechnoExt::GetCurrentSpeedMultiplier(pThis);

			R->EAX(Game::F2I(currentSpeed));
		}
	}

	return 0;
}
