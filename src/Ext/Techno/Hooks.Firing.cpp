#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>

#include <Misc/AresData.h>

#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

bool DisguiseAllowed(const TechnoTypeExt::ExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

//unnessesary i think , already put check at `CanFire` lets hope AI not freaking out about it !
//DEFINE_HOOK(0x70EF00, TechnoClass_CanFireAt_Disguise, 0x7)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(AbstractClass*, pTarget, EDI);
//
//	if (const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pTarget))->WeaponType)
//	{
//		if (pTarget->AbstractFlags & AbstractFlags::Object) {
//			auto const pObjectT = static_cast<ObjectClass*>(pTarget);
//			ObjectTypeClass* pObjectDisguise = pObjectT->GetDisguise(true);
//
//			if (pWeapon->Warhead->MakesDisguise &&
//				!DisguiseAllowed(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()),
//					pObjectDisguise))
//			{
//				R->EAX(false);
//				return 0x70EFC1;
//			}
//
//			if (pObjectDisguise) {
//				const auto nVtable = GetVtableAddr(pObjectDisguise);
//				if (nVtable == TerrainTypeClass::vtable)
//				{
//					if ((RulesClass::Instance->TreeTargeting || pWeapon->TerrainFire))
//					{
//						R->EAX(true);
//						return 0x70EFC1;
//					}
//				}
//				else if (nVtable == OverlayTypeClass::vtable && static_cast<OverlayTypeClass*>(pObjectDisguise)->IsARock)
//				{
//					if(pWeapon->TerrainFire)
//					{
//						R->EAX(true);
//						return 0x70EFC1;
//					}
//				}
//			}
//
//			if (Is_Terrain(pTarget) && (RulesClass::Instance->TreeTargeting || pWeapon->TerrainFire))
//			{
//				R->EAX(true);
//				return 0x70EFC1;
//			}
//		}
//		else if (pWeapon->TerrainFire)
//		{
//			if (Is_Cell(pTarget))
//			{
//				auto const pCellT = static_cast<CellClass*>(pTarget);
//				const auto nIdx = pCellT->OverlayTypeIndex;
//
//				if (nIdx != -1 && OverlayTypeClass::Array->GetItem(nIdx)->IsARock)
//				{
//					R->EAX(true);
//					return 0x70EFC1;
//				}
//			}
//		}
//	}
//
//
//	R->EAX(false);
//	return 0x70EFC1;
//}

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire_PreFiringChecks, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));

	enum { FireIllegal = 0x6FCB7E, Continue = 0x0 , FireCant = 0x6FCD29 };

	auto const pObjectT = generic_cast<ObjectClass*>(pTarget);

	if (pWeapon->Warhead->MakesDisguise && pObjectT) {
		if (!DisguiseAllowed(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()), pObjectT->GetDisguise(true)))
			return FireIllegal;
	}

	// Ares TechnoClass_GetFireError_OpenToppedGunnerTemporal
	// gunners and opentopped together do not support temporals, because the gunner
	// takes away the TemporalImUsing from the infantry, and thus it is missing
	// when the infantry fires out of the opentopped vehicle
	if (pWeapon->Warhead->Temporal && pThis->Transporter) {
		auto const pType = pThis->Transporter->GetTechnoType();
		if (pType->Gunner && pType->OpenTopped) {
			if(!pThis->TemporalImUsing)
				return FireCant;
		}
	}

	//if (!TechnoExt::FireOnceAllowFiring(pThis, pWeapon, pTarget))
	//	return FireCant;

	if (!TechnoExt::ObjectHealthAllowFiring(pObjectT, pWeapon))
		return FireIllegal;

	if (PassengersFunctional::CanFire(pThis))
		return FireIllegal;

	if (!TechnoExt::CheckFundsAllowFiring(pThis, pWeapon->Warhead))
		return FireIllegal;

	if (!TechnoExt::InterceptorAllowFiring(pThis, pObjectT))
		return FireIllegal;

	auto const& [pTargetTechno, targetCell] = TechnoExt::GetTargets(pObjectT, pTarget);

	// AAOnly doesn't need to be checked if LandTargeting=1.
	if ((!pTargetTechno
		|| pTargetTechno->GetTechnoType()->LandTargeting != LandTargetingType::Land_not_okay)
		&& pWeapon->Projectile->AA
		&& pTarget && !pTarget->IsInAir()
		) {
		if (BulletTypeExt::ExtMap.Find(pWeapon->Projectile)->AAOnly)
			return FireIllegal;
	}

	if (!TechnoExt::CheckCellAllowFiring(targetCell, pWeapon))
		return FireIllegal;

	if (pTargetTechno)
	{

		if (!TechnoExt::TechnoTargetAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		//if (!TechnoExt::TargetTechnoShieldAllowFiring(pTargetTechno, pWeapon))
		//	return FireIllegal;

		if (!TechnoExt::TargetFootAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;
	}

	return Continue;
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
			R->EAX(int(nDamage * nDamageMult));
			return ApplyDamageMult;
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	switch (TechnoExt::ApplyAreaFire(pThis, pCell, pWeaponType))
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

			WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis , true , nullptr);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_Middle, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));
	GET_BASE(int, weaponIndex, 0xC);

	//TechnoClass_FireAt_ToggleLaserWeaponIndex
	if (Is_Building(pThis) && pWeaponType->IsLaser)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	//TechnoClass_FireAt_BurstOffsetFix_2
	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pWeaponType->Burst;

	if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
	{
		if (TechnoExt::ExtMap.Find(pThis)->IsInterceptor())
		{
			BulletExt::ExtMap.Find(pBullet)->IsInterceptor = true;
			BulletExt::ExtMap.Find(pTargetObject)->InterceptedStatus = InterceptedStatus::Targeted;

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			{
				WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead)->InterceptBullets(pThis, pWeaponType, pTargetObject->Location);
			}
		}
	}

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (pWeaponExt->ShakeLocal.Get() && !pThis->IsOnMyView())
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

		//if(IS_SAME_STR_("LYNXBUFF" , pThis->get_ID()))
		// if(pTransport->IsBeingWarpedOut()){
		 //	return DisallowFiring;
		//}
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = pThis->GetTechnoType();

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = pWeapon->Range;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);

		if (pThisType->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
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

	if (result == 0 && Is_Aircraft(pThis) && pThisType->OpenTopped)
	{
		result = pThisType->GuardRange;
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

	if (pCell->ContainsBridge())
		return LandTargetingCheck;

	if (pCell->LandType == LandType::Water && pThis->GetTechnoType()->NavalTargeting == NavalTargetingType::Naval_none)
		return LandTargetingCheck;

	return pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water ?
		LandTargetingCheck : SkipLandTargetingCheck;
}

//DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_DropPassenger, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(AbstractClass*, pTarget, EDI);
//	GET(WeaponTypeClass*, pWeapon, EBX);
//
//	if (pThis->Passengers.FirstPassenger)
//	{
//		// TODO : implement this for UnitClass
//		pThis->DropOffParadropCargo();
//	}
//
//	return 0x0;
//}