#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	enum { CannotFire = 0x6FCB7E };

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
			return CannotFire;
	}

	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* targetCell = nullptr;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir()) {
			if (const auto pCell = specific_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				targetCell = pObject->GetCell();
		}

		if (targetCell) {
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
				return CannotFire;
		}

		if (pTechno) {
			if (const auto pUnit = specific_cast<UnitClass*>(pTechno))
				if (pUnit->DeathFrameCounter > 0)
					return CannotFire;

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner)) {
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
	if (pThis->InOpenToppedTransport) {
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		if (auto const  pTransport = pThis->Transporter) {
			float nDamageMult = RulesGlobal->OpenToppedDamageMultiplier;

			if (auto const  pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())) {
				nDamageMult = pExt->OpenTopped_DamageMultiplier.Get(nDamageMult);
			}

			R->EAX(Game::F2I(nDamage* nDamageMult));
			return ApplyDamageMult;
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x7)
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(const WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	if (auto const pExt = WeaponTypeExt::ExtMap.Find(pWeaponType)) {
		if (pExt->AreaFire_Target == AreaFireTarget::Random) {
			auto const range = pWeaponType->Range / Unsorted::d_LeptonsPerCell;

			const std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
			size_t size = adjacentCells.size();

			for (int i = 0; i < (int)size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = pCell->MapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

				if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true)) {
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

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		if (pWeaponExt->FeedbackWeapon.isset()) {
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
			if (auto const pBulletExt = BulletExt::GetExtData(pBullet)) {
				pBulletExt->IsInterceptor = true;
			}

			if (auto const pBulletExt = BulletExt::GetExtData(pTargetObject))
				pBulletExt->InterceptedStatus = InterceptedStatus::Targeted;

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso) {
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

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		if (auto const  pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTransport->Deactivated && !pExt->OpenTopped_AllowFiringIfDeactivated)
				return DisallowFiring;
		}
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C0, TechnoClass_WeaponRange, 0x4)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = pWeapon->Range;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (!pTypeExt)
			return 0x0;

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
				else if (pPassenger->HasTurret())
					tWeaponIndex = pPassenger->CurrentWeaponNumber;

				if (WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType)
				{
					if (pTWeapon->Range < smallestRange)
						smallestRange = pTWeapon->Range;
				}

				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
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