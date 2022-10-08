#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
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
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* targetCell = nullptr;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = specific_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				targetCell = pObject->GetCell();
		}

		if (targetCell)
		{
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
				return CannotFire;

			if (const auto pOverlayType = OverlayTypeClass::Array->GetItemOrDefault(targetCell->OverlayTypeIndex)) {
				if (pWeapon->IsWallDestroyer(pOverlayType) &&
					!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, HouseClass::Array->GetItemOrDefault(targetCell->WallOwnerIndex))
					) {
					return CannotFire;
				}
			}
		}

		if (pTechno)
		{

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}

			//if(pTargetTechnoExt->IsDummy.Get())
			//	return CannotFire;

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

				auto pTargetTechnoExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

				if (pTargetTechnoExt->Get()->Locomotor == LocomotionClass::CLSIDs::Teleport &&
					pFoot->IsWarpingIn() && pTargetTechnoExt->ChronoDelay_Immune.Get())
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
	if (pThis->InOpenToppedTransport) {
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		if (auto const  pTransport = pThis->Transporter) {
			float nDamageMult = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())->OpenTopped_DamageMultiplier
				.Get(RulesGlobal->OpenToppedDamageMultiplier);
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
	GET_STACK(const WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	{
		auto const pExt = WeaponTypeExt::ExtMap.Find(pWeaponType);
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / Unsorted::d_LeptonsPerCell;

			const std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
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

	{
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (pWeaponExt->FeedbackWeapon.isset()) {
			if(auto fbWeapon = pWeaponExt->FeedbackWeapon.Get()) {

				if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
					return 0;

				WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis);
			}
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
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso) {
				WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead)->InterceptBullets(pSource, pWeaponType, pTargetObject->Location);
			}
		}
	}

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (pWeaponExt->ShakeLocal.Get() && !pSource->IsOnMyView())
		return 0x0;

	if (pWeaponExt->Xhi || pWeaponExt->Xlo)
		GeneralUtils::CalculateShakeVal(Map.ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

	if (pWeaponExt->Yhi || pWeaponExt->Ylo)
		GeneralUtils::CalculateShakeVal(Map.ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));


	return 0;
}

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter) {
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

//DEFINE_HOOK(0x6F858F, TechnoClass_CanAutoTarget_BuildingOut1, 0x6)
//{
//	GET(TechnoClass*, pThis, EDI);
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F85F8, NextIf = 0x6F860C, FarIf = 0x6F866D };
//
//	if (FootClass* pFoot = abstract_cast<FootClass*>(pThis))
//	{
//		if (pFoot->Team != nullptr
//			|| !pFoot->Owner->IsControlledByHuman()
//			|| pTarget->IsStrange()
//			|| pTarget->WhatAmI() != AbstractType::Building
//			|| pTarget->GetTurretWeapon() && pTarget->GetTurretWeapon()->WeaponType != nullptr && pTarget->GetThreatValue())
//		{
//			//game code
//			if (!pThis->IsEngineer())
//				return FarIf;
//
//			return NextIf;
//		}
//		else
//		{
//			if (!pThis->IsEngineer())
//			{
//				//dehardcode
//				if (pTarget->WhatAmI() == AbstractType::Building && pTarget->GetThreatValue())
//				{
//					return FarIf;
//				}
//
//				return CannotTarget;
//			}
//
//			return NextIf;
//		}
//	}
//
//	return NextIf;
//}

//
//DEFINE_HOOK(0x6F889B, TechnoClass_CanAutoTarget_BuildingOut2, 0xA)
//{
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F894F, GameCode = 0x6F88BF };
//
//	if (pTarget->WhatAmI() != AbstractType::Building)
//		return CannotTarget;
//
//	WeaponStruct* pWeapon = pTarget->GetTurretWeapon();
//
//	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
//	{
//		if (pTarget->GetThreatValue())
//			return GameCode;
//
//		return CannotTarget;
//	}
//
//	return GameCode;
//}

DEFINE_HOOK(0x6FDD93, TechnoClass_FireAt_DelayedFire, 0x6) // or 0x6FDD99  , 0x6
{
	GET(WeaponTypeClass*, pWeaponType, EBX);
	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(FootClass*, pTarget, 0xD4);//8);

	enum { skipDelayedFire = 0, skipFireAt = 0x6FDE03 };

	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pWeaponTypeExt->DelayedFire_Anim_LoopCount <= 0 || !pWeaponTypeExt->DelayedFire_Anim.isset())
		return skipDelayedFire;

	if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0 && pExt->DelayedFire_DurationTimer < 0)
		pExt->DelayedFire_DurationTimer = pWeaponTypeExt->DelayedFire_DurationTimer.Get();

	AnimTypeClass* pDelayedFireAnimType = pWeaponTypeExt->DelayedFire_Anim.isset() ? pWeaponTypeExt->DelayedFire_Anim.Get() : nullptr;
	bool hasValidDelayedFireAnimType = pDelayedFireAnimType ? true : false;

	if (!hasValidDelayedFireAnimType)
	{
		pExt->DelayedFire_Anim = nullptr;
		pExt->DelayedFire_Anim_LoopCount = 0;
		pExt->DelayedFire_DurationTimer = -1;

		return skipDelayedFire;
	}

	bool isDelayedFireAnimPlaying = pExt->DelayedFire_Anim ? true : false;
	bool hasDeployAnimFinished = (isDelayedFireAnimPlaying && (pExt->DelayedFire_Anim->Animation.Value >= pDelayedFireAnimType->End + pDelayedFireAnimType->Start - 1)) ? true : false;

	if (!isDelayedFireAnimPlaying)
	{
		// Create the DelayedFire animation & stop the Fire process
		TechnoTypeClass* pThisType = pThis->GetTechnoType();
		int weaponIndex = pThis->CurrentWeaponNumber;
		CoordStruct animLocation = pThis->Location;

		if (pWeaponTypeExt->DelayedFire_Anim_UseFLH)
			animLocation = TechnoExt::GetFLHAbsoluteCoords(pThis, pThisType->GetWeapon(weaponIndex).FLH, pThis->HasTurret());//pThisType->Weapon[weaponIndex].FLH;

		if (auto pAnim = GameCreate<AnimClass>(pDelayedFireAnimType, animLocation))//pThis->Location))//animLocation))
		{
			pExt->DelayedFire_Anim = pAnim;
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);
			pExt->DelayedFire_Anim->SetOwnerObject(pThis);
			pExt->DelayedFire_Anim_LoopCount++;
		}
		else
		{
			Debug::Log("ERROR! DelayedFire animation [%s] -> %s can't be created.\n", pThis->GetTechnoType()->ID, pDelayedFireAnimType->ID);
			pExt->DelayedFire_Anim = nullptr;
			pExt->DelayedFire_Anim_LoopCount = 0;
			pExt->DelayedFire_DurationTimer = -1;

			return skipDelayedFire;
		}
	}
	else
	{
		if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0)
		{
			pExt->DelayedFire_DurationTimer--;

			if (pExt->DelayedFire_DurationTimer <= 0)
			{
				pExt->DelayedFire_Anim = nullptr;
				pExt->DelayedFire_Anim_LoopCount = 0;
				pExt->DelayedFire_DurationTimer = -1;

				return skipDelayedFire;
			}
		}

		if (hasDeployAnimFinished)
		{
			// DelayedFire animation finished but it can repeat more times, if set
			pExt->DelayedFire_Anim = nullptr;

			if (pExt->DelayedFire_Anim_LoopCount >= pWeaponTypeExt->DelayedFire_Anim_LoopCount && pWeaponTypeExt->DelayedFire_Anim_LoopCount > 0)
			{
				pExt->DelayedFire_Anim_LoopCount = 0;
				pExt->DelayedFire_DurationTimer = -1;

				return skipDelayedFire;
			}
		}
	}


	return skipFireAt;
}