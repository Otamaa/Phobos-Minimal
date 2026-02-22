#include "Body.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/Ares/Hooks/Header.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

#ifdef ATTACHMENT
ASMJIT_PATCH(0x6FC3F4, TechnoClass_CanFire_HandleAttachmentLogics, 0x6)
{
	enum { ReturnFireErrorIllegal = 0x6FC86A, ContinueCheck = 0x0 };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);
	GET(WeaponTypeClass*, pWeapon, EDI);

	//auto const& pExt = TechnoExtContainer::Instance.Find(pThis);
	//auto const& pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	bool illegalParentTargetWarhead = pWeapon->Warhead
		&& pWeapon->Warhead->IsLocomotor;

	if (illegalParentTargetWarhead && TechnoExtData::IsChildOf(pThis, pTarget))
		return ReturnFireErrorIllegal;

	return ContinueCheck;
}
#endif

// canEnter and ignoreForce should come before GetFireError().
DEFINE_JUMP(LJMP, 0x70054D, 0x70056C)

namespace WhatActionObjectTemp
{
	bool Skip = false;
}

ASMJIT_PATCH(0x700536, TechnoClass_WhatAction_Object_AllowAttack, 0x6)
{
	enum { CanAttack = 0x70055D, Continue = 0x700548 };

	GET_STACK(bool, canEnter, STACK_OFFSET(0x1C, 0x4));
	GET_STACK(bool, ignoreForce, STACK_OFFSET(0x1C, 0x8));
	GET(TechnoClass*, pThis, ESI);

	auto const pType = GET_TECHNOTYPE(pThis);

	if (TechnoTypeExtContainer::Instance.Find(pType)
		->NoManualFire)
		return 0x70056Cu;

	if (canEnter || ignoreForce)
		return CanAttack;

	GET(ObjectClass*, pObject, EDI);
	GET_STACK(int, WeaponIndex, STACK_OFFSET(0x1C, -0x8));

	WhatActionObjectTemp::Skip = true;
	R->EAX(pThis->GetFireError(pObject, WeaponIndex, true));
	WhatActionObjectTemp::Skip = false;

	return Continue;
}

ASMJIT_PATCH(0x6FC8F5, TechnoClass_CanFire_SkipROF, 0x6)
{
	return WhatActionObjectTemp::Skip ? 0x6FC981 : 0;
}

ASMJIT_PATCH(0x6FCAFA, TechnoClass_CanFire_Verses, 0x8)
{
	enum
	{
		FireIllegal = 0x6FCB7E,
		ContinueCheck = 0x6FCBCD,
		ContinueCheckB = 0x6FCCBD,
		ForceNewValue = 0x6FCBA6,
		TargetIsNotTechno = 0x6FCCBD
	};

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (!pTarget)
	{
		return TargetIsNotTechno;
	}

	if (pTarget->IsSinking)
		return FireIllegal;

	auto pWH = (FakeWarheadTypeClass*)pWeapon->Warhead;

	if (pWH->Parasite && pTarget->IsIronCurtained())
	{
		return FireIllegal;
	}

	if (pWH->MindControl)
	{
		if (auto pManager = (FakeCaptureManagerClass*)pThis->CaptureManager)
		{
			if (!pManager->__CanCapture(pTarget))
			{
				return FireIllegal;
			}
		}
	}

	Armor armor = TechnoExtData::GetTechnoArmor(pTarget, pWH);
	const auto vsData = pWH->GetVersesData(armor);

	// i think there is no way for the techno know if it attack using force fire or not
	if (vsData->Flags.ForceFire || vsData->Verses != 0.0)
	{
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

		if (pWHExt->FakeEngineer_CanCaptureBuildings || pWHExt->FakeEngineer_BombDisarm)
		{
			const int weaponRange = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
			const int currentRange = pThis->DistanceFrom(pTarget);

			if (pWHExt->FakeEngineer_BombDisarm
				&& pTarget->AttachedBomb
				&& BombExtContainer::Instance.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable)
			{

				if (currentRange <= weaponRange)
					R->EAX(FireError::OK);
				else
					R->EAX(FireError::RANGE); // Out of range

				return ForceNewValue;
			}

			if (pWHExt->FakeEngineer_CanCaptureBuildings)
			{
				const auto pBuilding = cast_to<BuildingClass*, false>(pTarget);
				const bool UneableToCapture = !pBuilding
					|| !pBuilding->IsAlive
					|| pBuilding->Health <= 0
					|| pThis->Owner->IsAlliedWith(pTarget)
					|| (!pBuilding->Type->Capturable && !pBuilding->Type->NeedsEngineer);

				if (!UneableToCapture)
				{
					if (currentRange <= weaponRange)
						R->EAX(FireError::OK);
					else
						R->EAX(FireError::RANGE); // Out of range

					return ForceNewValue;
				}
			}
		}

		if (pWH->BombDisarm &&
			(!pTarget->AttachedBomb ||
				!BombExtContainer::Instance.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable)
		)
		{
			return FireIllegal;
		}

		if (pWH->IvanBomb && pTarget->AttachedBomb)
			return FireIllegal;

		// Skips bridge-related coord checks to allow AA to target air units on bridges over water.
		if (pTarget->IsInAir())
		{
			return ContinueCheckB;
		}

		//elevation related checks
		return  ContinueCheck;
	}

	return FireIllegal;
}

ASMJIT_PATCH(0x6FC0D3, TechnoClass_CanFire_DisableWeapons, 8)
{
	enum
	{
		FireRange = 0x6FC0DF,  //keep targeting ?
		FireIllegal = 0x6FCCDF, //cannot fire at all , ignore target
		ContinueCheck = 0x0  //weeee
	};

	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFSET(0x20, 0x8));

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	// Force weapon check
	const int newIndex = pTypeExt->SelectPhobosWeapon(pThis, pTarget);

	if (newIndex >= 0)
	{
		weaponIndex = newIndex;
	}

	pExt->CanFireWeaponType = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pExt->CanFireWeaponType)
	{
		if (pExt->DisableWeaponTimer.InProgress())
			return FireRange;

		if (pExt->AE.flags.DisableWeapons)
			return FireRange;


		if (auto const pTechnoT = flag_cast_to<TechnoClass*, false>(pTarget))
		{
			if (!TechnoExtData::CanTargetICUnit(pThis, (FakeWeaponTypeClass*)pExt->CanFireWeaponType, pTechnoT))
				return FireIllegal;
		}

		return ContinueCheck;
	}


	return FireIllegal;
}

//weapons can take more than one round of ammo
ASMJIT_PATCH(0x6FCA0D, TechnoClass_CanFire_Ammo, 6)
{
	enum { FireErrAmmo = 0x6FCA17u, Continue = 0x6FCA5Eu, FireErrorCloaked = 0x6FCA4Fu };
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto nAmmo = pThis->Ammo;
	if (nAmmo < 0)
		return Continue;

	if (!(nAmmo >= WeaponTypeExtContainer::Instance.Find(pWeapon)->Ammo))
		return FireErrAmmo;

	if (!pWeapon->DecloakToFire)
		return Continue;

	if (pWeapon->DecloakToFire)
	{
		if (pThis->CloakState == CloakState::Uncloaked)
			return Continue;

		return pThis->WhatAmI() != AircraftClass::AbsID || pThis->CloakState == CloakState::Cloaked ? FireErrorCloaked : Continue;
	}

	return Continue;
	/*const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const bool IsDisabled = pTypeExt->NoAmmoWeapon == -1;

	if (!IsDisabled)
	{
		if ((nAmmo - WeaponTypeExtContainer::Instance.Find(pWeapon)->Ammo) >= 0)
			return Continue;
	}

	return (!IsDisabled || !nAmmo) ? FireErrAmmo : Continue;*/
}

ASMJIT_PATCH(0x6FCD1D, TechnoClass_CanFire_OpenTopCloakFix, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(bool, checkIfTargetInRange, STACK_OFFSET(0x20, 0xC));

	if (checkIfTargetInRange && pThis->InOpenToppedTransport && pThis->Transporter)
		pThis->Transporter->Uncloak(true);

	return 0;
}

ASMJIT_PATCH(0x6FC3AE, TechnoClass_CanFire_TankInBunker_LocomotorWarhead, 0x6)
{
	enum { Illegal = 0x6FC86A };

	GET(WeaponTypeClass*, pWeapon, EDI);

	return pWeapon->Warhead && pWeapon->Warhead->IsLocomotor ? Illegal : 0;
}

ASMJIT_PATCH(0x6FC617, TechnoClass_CanFire_AirCarrierSkipCheckNearBridge, 0x8)
{
	enum { ContinueCheck = 0x6FC61F, TemporaryCannotFire = 0x6FCD0E };

	GET(TechnoClass* const, pThis, ESI);
	GET(const bool, nearBridge, EAX);

	return (nearBridge && !pThis->IsInAir()) ? TemporaryCannotFire : ContinueCheck;
}

ASMJIT_PATCH(0x6FC7EB, TechnoClass_CanFire_InterceptBullet, 0x7)
{
	enum { IgnoreAG = 0x6FC815, ContinueCheck = 0x6FC7F2 };

	GET(AbstractClass*, pTarget, EBX);

	if (pTarget->WhatAmI() == AbstractType::Bullet)
		return IgnoreAG;

	R->AL(pTarget->IsInAir());
	return ContinueCheck;
}

ASMJIT_PATCH(0x6FC749, TechnoClass_CanFire_AntiUnderground, 0x5)
{
	enum { Illegal = 0x6FC86A, GoOtherChecks = 0x6FC762 };

	GET(Layer, layer, EAX);
	//GET(TechnoClass*, pThis, EBX);
	GET(WeaponTypeClass*, pWeapon, EDI);

	auto const pProj = pWeapon->Projectile;
	auto const pProjExt = BulletTypeExtContainer::Instance.Find(pProj);

	if (layer == Layer::Underground && !pProjExt->AU)
		return Illegal;

	if ((layer == Layer::Air || layer == Layer::Top) && !pProj->AA)
		return Illegal;

	return GoOtherChecks;
}

ASMJIT_PATCH(0x6FC5C7, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { Illegal = 0x6FC86A, OutOfRange = 0x6FC0DF, Continue = 0x6FC5D5 };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTransport, EAX);
	//GET_STACK(int, weaponIndex, STACK_OFFSET(0x20, 0x8));

	auto const pTypeExt = GET_TECHNOTYPEEXT(pTransport);

	if (pTransport->Transporter || (pTransport->Deactivated && !pTypeExt->OpenTopped_AllowFiringIfDeactivated))
		return Illegal;

	TechnoExtData* pThisExt = TechnoExtContainer::Instance.Find(pThis);

	if (pTypeExt->OpenTopped_CheckTransportDisableWeapons
		&& TechnoExtContainer::Instance.Find(pTransport)->AE.flags.DisableWeapons
		&& pThisExt->CanFireWeaponType
		) return OutOfRange;

	return Continue;
}

ASMJIT_PATCH(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x20, 0x8));
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = GET_TECHNOTYPE(pThis);
	auto pCell = cast_to<CellClass*, false>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_none &&
			((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach)) && !pCell->ContainsBridge())
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = cast_to<TerrainClass*, false>(pTarget))
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

ASMJIT_PATCH(0x6FC815, TechnoClass_CanFire_CellTargeting, 0x7)
{
	enum
	{
		LandTargetingCheck = 0x6FC857,
		SkipLandTargetingCheck = 0x6FC879
	};

	GET(AbstractClass*, pTarget, EBX);
	GET(TechnoClass*, pThis, ESI);

	CellClass* pCell = cast_to<CellClass*, false>(pTarget);
	if (!pCell)
		return SkipLandTargetingCheck;

	if (pCell->ContainsBridge())
		return LandTargetingCheck;

	if (pCell->LandType == LandType::Water && GET_TECHNOTYPE(pThis)->NavalTargeting == NavalTargetingType::Naval_none)
		return LandTargetingCheck;

	return pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water ?
		LandTargetingCheck : SkipLandTargetingCheck;
}

bool DisguiseAllowed(const TechnoTypeExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

// Pre-Firing Checks
ASMJIT_PATCH(0x6FC31C, TechnoClass_CanFire_PreFiringChecks, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));

	enum { FireIllegal = 0x6FCB7E, Continue = 0x0, FireCant = 0x6FCD29 };

	const auto pThisExt = TechnoExtContainer::Instance.Find(pThis);

	FakeWeaponTypeClass* pWeapon = (FakeWeaponTypeClass*)pThisExt->CanFireWeaponType;

	auto const pObjectT = flag_cast_to<ObjectClass*, false>(pTarget);
	auto const pTechnoT = flag_cast_to<TechnoClass*, false>(pTarget);
	auto const pFootT = flag_cast_to<FootClass*, false>(pTarget);
	auto const pWeaponExt = pWeapon->_GetExtData();

	R->EDI(pThisExt->CanFireWeaponType);

	if (pWeaponExt->NoRepeatFire > 0)
	{
		if (pTechnoT)
		{
			const auto pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTechnoT);

			if ((Unsorted::CurrentFrame - pTargetTechnoExt->LastBeLockedFrame) < pWeaponExt->NoRepeatFire)
				return FireIllegal;
		}
	}

	if (auto pTerrain = cast_to<TerrainClass*, false>(pTarget))
		if (pTerrain->Type->Immune)
			return FireIllegal;

	if (pWeapon->Warhead->MakesDisguise && pObjectT)
	{
		if (!DisguiseAllowed(GET_TECHNOTYPEEXT(pThis), pObjectT->GetDisguise(true)))
			return FireIllegal;
	}

	// Ares TechnoClass_GetFireError_OpenToppedGunnerTemporal
	// gunners and opentopped together do not support temporals, because the gunner
	// takes away the TemporalImUsing from the infantry, and thus it is missing
	// when the infantry fires out of the opentopped vehicle
	if (pWeapon->Warhead->Temporal && pThis->Transporter)
	{
		auto const pType = GET_TECHNOTYPE(pThis->Transporter);
		if (pType->Gunner && pType->OpenTopped)
		{
			if (!pThis->TemporalImUsing)
				return FireCant;
		}
	}

	//if (!TechnoExtData::FireOnceAllowFiring(pThis, pWeapon, pTarget))
	//	return FireCant;

	if (!pWeaponExt->SkipWeaponPicking && !TechnoExtData::ObjectHealthAllowFiring(pObjectT, pWeapon))
		return FireIllegal;

	if (PassengersFunctional::CanFire(pThis))
		return FireIllegal;

	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeapon->Warhead))
		return FireIllegal;

	if (!TechnoExtData::InterceptorAllowFiring(pThis, pObjectT))
		return FireIllegal;

	auto const& [pTargetTechno, targetCell] = TechnoExtData::GetTargets(pObjectT, pTarget);

	// AAOnly doesn't need to be checked if LandTargeting=1.
	if (GET_TECHNOTYPE(pThis)->LandTargeting != LandTargetingType::Land_not_okay && pWeapon->Projectile->AA && pTarget && !pTarget->IsInAir())
	{
		if (BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AAOnly)
			return FireIllegal;
	}

	if (!TechnoExtData::CheckCellAllowFiring(pThis, targetCell, pWeapon))
		return FireIllegal;

	if (pTargetTechno)
	{
		const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);

		if (pWeaponExt->OnlyAttacker.Get() && !pTargetExt->ContainFirer(pWeapon, pThis))
			return FireIllegal;

		if (pThis->Berzerk && !EnumFunctions::CanTargetHouse(RulesExtData::Instance()->BerzerkTargeting, pThis->Owner, pTargetTechno->Owner))
			return FireIllegal;

		if (!TechnoExtData::TechnoTargetAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		//if (!TechnoExtData::TargetTechnoShieldAllowFiring(pTargetTechno, pWeapon))
		//	return FireIllegal;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		if (pWeapon->Warhead->Airstrike)
		{
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets, false))
				return FireIllegal;

			if (!GET_TECHNOTYPEEXT(pTargetTechno)->AllowAirstrike.Get(pFootT || cast_to<BuildingClass*, false>(pTargetTechno)->Type->CanC4))
				return FireIllegal;
		}
	}

	return Continue;
}

ASMJIT_PATCH(0x6FC3FE, TechnoClass_CanFire_Immunities, 0x6)
{
	enum { FireIllegal = 0x6FC86A, ContinueCheck = 0x6FC425 };

	//GET(TechnoClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWarhead, EAX);
	GET(TechnoClass*, pTarget, EBP);

	if (pTarget)
	{
		//const auto nRank = pTarget->Veterancy.GetRemainingLevel();

		//const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		//if(pWHExt->ImmunityType.isset() &&
		//	 TechnoExtData::HasImmunity(nRank, pTarget , pWHExt->ImmunityType.Get()))
		//	return FireIllegal;

		if (pWarhead->Psychedelic && TechnoExtData::IsPsionicsImmune(pTarget))
			return FireIllegal;
	}

	return ContinueCheck;
}

ASMJIT_PATCH(0x6FC3A1, TechnoClass_CanFire_InBunkerRangeCheck, 0x5)
{
	enum { ContinueChecks = 0x6FC3C5, CannotFire = 0x6FC86A };

	GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	if (pTarget->WhatAmI() == AbstractType::Unit && WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) < 384.0)
		return CannotFire;

	return ContinueChecks;
}

#pragma region Unit
ASMJIT_PATCH(0x51CAD1, InfantryClass_CanFire_Sync, 0x6)
{
	GET(FakeInfantryClass*, pInf, EBX);
	R->ESI(pInf->_GetExtData()->CanFireWeaponType);
	return 0x51CAE2;
}

ASMJIT_PATCH(0x7410EC, UnitClass_CanFire_Sync, 0x5)
{
	GET(FakeUnitClass*, pUnit, ESI);
	R->EBX(pUnit->_GetExtData()->CanFireWeaponType);
	return 0x7410F9;
}

ASMJIT_PATCH(0x741050, UnitClass_CanFire_DeployToFire, 0x6)
{
	enum { NoNeedToCheck = 0x74132B, SkipGameCode = 0x7410B7, MustDeploy = 0x7410A8 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->DeployToFire
		&& pThis->CanDeployNow()
		&& !TechnoExtData::CanDeployIntoBuilding(pThis, true)
		)
	{
		return MustDeploy;
	}

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt->NoTurret_TrackTarget.Get(RulesExtData::Instance()->NoTurret_TrackTarget))
	{
		return NoNeedToCheck;
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x7410D6, UnitClass_CanFire_Tethered, 0x7)
{
	GET(TechnoClass*, pLink, EAX);
	return !pLink ? 0x7410DD : 0x0;
}

ASMJIT_PATCH(0x741206, UnitClass_CanFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto Type = pThis->Type;

	if (!Type->TurretCount || Type->IsGattling)
	{
		return 0x741229;
	}

	const auto W = pThis->GetWeapon(pThis->SelectWeapon(nullptr));
	return (W->WeaponType && W->WeaponType->Warhead->Temporal)
		? 0x741210u
		: 0x741229u
		;
}


ASMJIT_PATCH(0x51C913, InfantryClass_CanFire_Heal, 7)
{
	enum { retFireIllegal = 0x51C939, retContinue = 0x51C947 };
	GET(InfantryClass*, pThis, EBX);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x20, 0x8));

	const auto pThatTechno = flag_cast_to<TechnoClass*>(pTarget);

	if (!pThatTechno || pThatTechno->IsIronCurtained())
	{
		return retFireIllegal;
	}

	return  TechnoExt_ExtData::FiringAllowed(pThis, pThatTechno, pThis->GetWeapon(nWeaponIdx)->WeaponType) ?
		retContinue : retFireIllegal;

}

ASMJIT_PATCH(0x741113, UnitClass_CanFire_Heal, 0xA)
{
	enum { retFireIllegal = 0x74113A, retContinue = 0x741149 };
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pThatTechno, EDI);
	GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x1C, 0x8));

	return !pThatTechno->IsIronCurtained() && TechnoExt_ExtData::FiringAllowed(pThis, pThatTechno, pThis->GetWeapon(nWeaponIdx)->WeaponType) ?
		retContinue : retFireIllegal;
}

ASMJIT_PATCH(0x741288, UnitClass_CanFire_DeployFire_DoNotErrorFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pThis->Type->DeployFire
		&& !pThis->Type->IsSimpleDeployer
		&& !pThis->Deployed
		&& pThis->CurrentMission == Mission::Unload
	)
	{
		return 0x741327; //fireOK
	}

	return 0x0;
}

#pragma endregion

#pragma region Building
#pragma endregion

#pragma region Infantry
#pragma endregion