#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/TechnoType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

static bool CeaseFire(TechnoClass* pThis)
{
	bool bCeaseFire = false;
	PassengersFunctional::CanFire(pThis, bCeaseFire);
	return bCeaseFire;
}
#endif

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	enum { CannotFire = 0x6FCB7E };

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto const pObjectT = generic_cast<ObjectClass*>(pTarget);

	if (pObjectT && pWeaponExt->Targeting_Health_Percent.isset())
	{

		auto const pHP = pObjectT->GetHealthPercentage_();

		if (!pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP <= pWeaponExt->Targeting_Health_Percent.Get())
			return CannotFire;
		else if (pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP >= pWeaponExt->Targeting_Health_Percent.Get())
			return CannotFire;
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	if (CeaseFire(pThis))
		return CannotFire;
#endif
	auto pTechnoExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	{
		auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
			return CannotFire;
	}

	//this code is used to remove Techno as auto target consideration , so interceptor can find target faster
	if (pTechnoExt->Interceptor.Get() &&
		pTechnoExt->Interceptor_OnlyTargetBullet.Get() &&
		!specific_cast<BulletClass*>(pTarget))
	{
		return CannotFire;
	}

	{
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		if (pTechno)
		{
			const auto pExt = TechnoExt::ExtMap.Find(pTechno);

			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon))
				{
					return CannotFire;
				}
			}
		}

		CellClass* targetCell = nullptr;

		// Ignore target cell for airborne technos.
		if (pTarget)
		{
			if (const auto pCell = specific_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget)){
				// Ignore target cell for technos that are in air.
				if ((pTechno && !pTechno->IsInAir()) || pObject != pTechno)
					targetCell = pObject->GetCell();
			}
		}

		if (targetCell)
		{
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
				return CannotFire;

			//if (const auto pOverlayType = OverlayTypeClass::Array->GetItemOrDefault(targetCell->OverlayTypeIndex))
			//{
			//	if (!pWeapon->IsWallDestroyer(pOverlayType) ||
			//		!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, HouseClass::Array->GetItemOrDefault(targetCell->WallOwnerIndex))
			//		)
			//	{
			//		return CannotFire;
			//	}
			//}
		}

		if (pTechno)
		{

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}

			if (pTechno->AbstractFlags & AbstractFlags::Foot)
			{
				const auto pFoot = static_cast<FootClass*>(pTechno);

				if (pFoot->WhatAmI() == AbstractType::Unit)
				{
					auto const pUnit = static_cast<UnitClass*>(pTechno);
#ifdef Ares_3_0_p1
					auto const bDriverKilled = (*(bool*)((char*)pUnit->align_154 + 0x9C));

					if (pUnit->DeathFrameCounter > 0 || bDriverKilled && pWeaponExt->Abductor.Get())
#else
					if (pUnit->DeathFrameCounter > 0)
#endif
						return CannotFire;
				}

				if (TechnoExt::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
					return CannotFire;
			}
		}
	}

	return 0;
}

// Weapon Firing
DEFINE_HOOK(0x6FE43B, TechnoClass_FireAt_OpenToppedDmgMult, 0x6) //7
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		if (auto const  pTransport = pThis->Transporter)
		{
			float nDamageMult = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
			R->EAX(Game::F2I(nDamage * nDamageMult));
			return ApplyDamageMult;
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	{
		auto const pExt = WeaponTypeExt::ExtMap.Find(pWeaponType);
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / Unsorted::d_LeptonsPerCell;

			std::vector<CellStruct> adjacentCells;
			GeneralUtils::AdjacentCellsInRange(adjacentCells, static_cast<size_t>(range + 0.99));
			size_t size = adjacentCells.size();

			for (int i = 0; i < (int)size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = pCell->MapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

				if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true))
				{
					R->EAX(tgtCell);
					return Continue;
				}
			}

			return DoNotFire;
		}
		else if (pExt->AreaFire_Target == AreaFireTarget::Self)
		{
			if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false))
				return DoNotFire;

			R->EAX(pThis);
			return SkipSetTarget;
		}

		if (!EnumFunctions::AreCellAndObjectsEligible(pCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false))
			return DoNotFire;
	}

	return Continue;
}

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x6)//8
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->FeedbackWeapon.isset())
	{
		if (auto fbWeapon = pWeaponExt->FeedbackWeapon.Get())
		{
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

		if (pSourceTypeExt->Interceptor.Get())
		{
			BulletExt::ExtMap.Find(pBullet)->IsInterceptor = true;
			BulletExt::ExtMap.Find(pTargetObject)->InterceptedStatus = InterceptedStatus::Targeted;

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			{
				WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead)->InterceptBullets(pSource, pWeaponType, pTargetObject->Location);
			}
		}
	}

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (pWeaponExt->ShakeLocal.Get() && !pSource->IsOnMyView())
		return 0x0;

	if (pWeaponExt->Xhi || pWeaponExt->Xlo)
		GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

	if (pWeaponExt->Yhi || pWeaponExt->Ylo)
		GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));


	return 0;
}

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		auto const  pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
		if (pTransport->Deactivated && !pExt->OpenTopped_AllowFiringIfDeactivated)
			return DisallowFiring;
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = pWeapon->Range;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pThis->GetTechnoType()->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else //if (pPassenger->HasTurret())
					//tWeaponIndex = pPassenger->CurrentWeaponNumber;
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				const auto pTWeapon = pPassenger->GetWeapon(tWeaponIndex);

				if (pTWeapon->WeaponType && pTWeapon->WeaponType->FireInTransport)
				{
					if (pTWeapon->WeaponType->Range < smallestRange)
					{
						smallestRange = pTWeapon->WeaponType->Range;
					}
				}

				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	if (result == 0 && pThis->GetTechnoType()->OpenTopped && pThis->WhatAmI() == AbstractType::Aircraft)
	{
		result = pThis->GetTechnoType()->GuardRange;
		if (result == 0)
			Debug::Log("Warning ! , range of Aircraft[%s] return 0 result will cause Aircraft to stuck ! \n", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x20, 0x8));
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = specific_cast<CellClass*>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_none &&
			((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach)) && !pCell->ContainsBridge())
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = specific_cast<TerrainClass*>(pTarget))
	{
		pCell = pTerrain->GetCell();

		if (pType->LandTargeting == LandTargetingType::Land_not_okay && pCell->LandType != LandType::Water && pCell->LandType != LandType::Beach)
			return DisallowFiring;
		//else if (pType->LandTargeting == LandTargetingType::Land_secondary && nWeaponIdx == 0)
		//	return DisallowFiring;
		else if (pType->NavalTargeting == NavalTargetingType::Naval_none && ((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pCell->ContainsBridge()))
			return DisallowFiring;
	}

	return 0;
}

DEFINE_HOOK(0x6FC815, TechnoClass_CanFire_CellTargeting, 0x6)
{
	enum
	{
		LandTargetingCheck = 0x6FC857,
		SkipLandTargetingCheck = 0x6FC879
	};


	GET(AbstractClass*, pTarget, EBX);
	GET(TechnoClass*, pThis, ESI);

	CellClass* pCell = specific_cast<CellClass*>(pTarget);
	if (!pCell)
		return SkipLandTargetingCheck;

	if(pCell->ContainsBridge())
		return LandTargetingCheck;
	
	if (pCell->LandType == LandType::Water && pThis->GetTechnoType()->NavalTargeting == NavalTargetingType::Naval_none)
		return LandTargetingCheck;

	return pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water ?
		LandTargetingCheck : SkipLandTargetingCheck;
}