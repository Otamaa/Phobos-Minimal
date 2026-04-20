#include "Body.h"

#include <Ext/Aircraft/Body.h>
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

#include <SpawnManagerClass.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

bool DisguiseAllowed(const TechnoTypeExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

FireError __fastcall FakeTechnoClass::__CanFireMod(
TechnoClass* pThis,
AbstractClass* pTarget,
int nWeaponIdx,
bool bCheckRange,
bool bSkipROF)
{
	// ====================================================================
// [1] Basic null / enslaved
// ====================================================================
	if (!pTarget || pThis->SlaveOwner)
		return FireError::ILLEGAL;

	// ====================================================================
	// [HOOK 0x6FC0D3] DisableWeapons — weapon selection & disable checks
	// Runs before original IsBeingWarpedOut check.
	// ====================================================================
	if (!pThis->IsArmed())
		return FireError::ILLEGAL;

	auto const pObjectT = flag_cast_to<ObjectClass*, false>(pTarget);
	auto const pTechnoT = flag_cast_to<TechnoClass*, false>(pTarget);
	auto const pFootT = flag_cast_to<FootClass*, false>(pTarget);
	auto const pBuildingT = cast_to<BuildingClass*, false>(pTarget);
	auto const pCellTarget = cast_to<CellClass*, false>(pTarget);

	auto const pThisExt = TechnoExtContainer::Instance.Find(pThis);
	auto const pThisFoot = flag_cast_to<FootClass*, false>(pThis);
	auto const pThisType = GET_TECHNOTYPE(pThis);
	auto const pThisTypeExt = GET_TECHNOTYPEEXT(pThis);

	auto const pCoordsCellT = MapClass::Instance->GetCellAt(pTarget->GetCoords());

	// Phobos weapon selection override
	const int nSelectedIdx = pThisTypeExt->SelectPhobosWeapon(pThis, pTarget);
	if (nSelectedIdx >= 0)
		nWeaponIdx = nSelectedIdx;

	// Cache selected weapon for extension checks downstream
	//const int weaponCount = pThisType->WeaponCount > 0 ? pThisType->WeaponCount : 2;

	pThisExt->CanFireWeaponType = pThis->GetWeapon(nWeaponIdx)->WeaponType;

	if (!pThisExt->CanFireWeaponType)
		return FireError::ILLEGAL;

	if (pThisExt->DisableWeaponTimer.InProgress())
		return FireError::REARM;

	if (pThisExt->AE.flags.DisableWeapons)
		return FireError::REARM;

	if (pTechnoT)
	{
		if (!TechnoExtData::CanTargetICUnit(
			pThis, (FakeWeaponTypeClass*)pThisExt->CanFireWeaponType, pTechnoT))
		{
			return FireError::ILLEGAL;
		}
	}
	// [END HOOK 0x6FC0D3]

	// ====================================================================
	// [2] Warping / status
	// ====================================================================
	if (pThis->IsWarpingIn())         // vt+0x1D8
		return FireError::REARM;

	if (pThis->LocomotorTarget == pTarget)          // +0x2AC LocomotorTarget
		return FireError::REARM;

	if (pThis->IsBeingWarpedOut()                // vt+0x1D4
		|| pThis->Deactivated             // +0x1C8
		|| pThis->IsSinking               // +0x3CD
		|| pTarget == pThis->DrainTarget  // +0x1CC
		|| pTarget == pThis->Transporter) // +0x11C
	{
		return FireError::ILLEGAL;
	}

	// ====================================================================
	// [3] Temporal re-arm
	// ====================================================================
	if (pThis->IsWarpingSomethingOut())              // vt+0x1DC
	{
		if (auto const pTemporal = pThis->TemporalImUsing) // +0x274
		{
			if (pTarget == pTemporal->Target)
				return FireError::REARM;
		}
	}

	// ====================================================================
	// [5] FootClass Locomotor attack check
	// ====================================================================
	if (pThisFoot && pThisFoot->IsAttackedByLocomotor)
	{
		return FireError::ILLEGAL;
	}

	// ====================================================================
	// [6] Target-is-Techno early checks
	// ====================================================================
	if (pTechnoT)
	{
		if (pTechnoT->InLimbo)
			return FireError::ILLEGAL;

		if (pThis->Berzerk // +0x298
			&& pTechnoT->GetTechnoType()->BerserkFriendly)
		{
			return FireError::ILLEGAL;
		}

		if (pThis->GetTechnoType()->Natural
			&& pTechnoT->GetTechnoType()->Unnatural)
		{
			return FireError::ILLEGAL;
		}

		if (pTechnoT->IsImmobilized) // +0x27C
			return FireError::ILLEGAL;
		
		//DEFINE_JUMP(LJMP, 0x6FC22A, 0x6FC24D) // Skip IronCurtain check
		// AI won't fire at Iron Curtained targets
		//if (!pThis->Owner->IsHumanPlayer
		//	&& pTechnoT->IsIronCurtained())
		//{
		//	return FireError::ILLEGAL;
		//}

		// Visibility: VISUAL_HIDDEN and not sensed
		if (pTechnoT->VisualCharacter(true, pThis->Owner) == VisualType::Hidden
			&& !pCoordsCellT->Sensors_InclHouse(pThis->Owner->ArrayIndex)
			&& (pThis->CombatDamage(-1) > 0
				|| !pTechnoT->Owner->IsAlliedWith(pThis->Owner)))
		{
			return FireError::CANT;
		}

		// Tethered BalloonHover not in air
		if (pTechnoT->IsTethered // +0x418
			&& pTechnoT->GetTechnoType()->BalloonHover
			&& !pTechnoT->IsInAir())
		{
			return FireError::ILLEGAL;
		}
	}

	// ====================================================================
	// [7] Falling / dying
	// ====================================================================
	if (pThis->IsFallingDown)
		return FireError::ILLEGAL;

	if (pThis->IsUnderEMP())
	{
		if (pThis->WhatAmI() != AbstractType::Unit)
			return FireError::CANT;

		auto pThisUnit = static_cast<UnitClass*>(pThis);

		if (!pThisUnit->Type->LargeVisceroid && !pThisUnit->Type->SmallVisceroid)
		{
			return FireError::CANT;
		}
	}

	// ====================================================================
	// [8] Get weapon + extension checks
	// [HOOK 0x6FC31C] PreFiringChecks — uses CanFireWeaponType from [HOOK 0x6FC0D3]
	// ====================================================================
	auto const pWeapon = static_cast<FakeWeaponTypeClass*>(pThisExt->CanFireWeaponType);
	if (!pWeapon)
		return FireError::CANT;

	auto const pWeaponExt = pWeapon->_GetExtData();

	if (pWeapon->IonSensitive && pThis->Owner->IonSensitivesShouldBeOffline())
		return FireError::CANT;

	// --- Phobos / Ares pre-fire extension checks ---

	// NoRepeatFire
	if (pWeaponExt->NoRepeatFire > 0 && pTechnoT)
	{
		auto const pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTechnoT);
		if ((Unsorted::CurrentFrame.get() - pTargetTechnoExt->LastBeLockedFrame)
			< pWeaponExt->NoRepeatFire)
		{
			return FireError::ILLEGAL;
		}
	}

	// Immune terrain
	if (auto const pTerrain = cast_to<TerrainClass*, false>(pTarget))
	{
		if (pTerrain->Type->Immune)
			return FireError::ILLEGAL;
	}

	// MakesDisguise
	if (pWeapon->Warhead->MakesDisguise && pObjectT)
	{
		if (!DisguiseAllowed(GET_TECHNOTYPEEXT(pThis), pObjectT->GetDisguise(true)))
			return FireError::ILLEGAL;
	}

	// Ares: Gunner + OpenTopped + Temporal incompatibility
	if (pWeapon->Warhead->Temporal && pThis->Transporter)
	{
		auto const pTransType = GET_TECHNOTYPE(pThis->Transporter);
		if (pTransType->Gunner && pTransType->OpenTopped)
		{
			if (!pThis->TemporalImUsing)
				return FireError::CANT;
		}
	}

	// ObjectHealth allow firing
	if (!pWeaponExt->SkipWeaponPicking
		&& !TechnoExtData::ObjectHealthAllowFiring(pObjectT, pWeapon))
	{
		return FireError::ILLEGAL;
	}

	// Funds
	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeapon->Warhead))
		return FireError::ILLEGAL;

	// Interceptor
	if (!TechnoExtData::InterceptorAllowFiring(pThis, pObjectT))
		return FireError::ILLEGAL;

	// AAOnly
	auto const& [pResolvedTechno, pResolvedCell] =
		TechnoExtData::GetTargets(pObjectT, pTarget);

	if (GET_TECHNOTYPE(pThis)->LandTargeting != LandTargetingType::Land_not_okay
		&& pWeapon->Projectile->AA
		&& pTarget
		&& !pTarget->IsInAir())
	{
		if (BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AAOnly)
			return FireError::ILLEGAL;
	}

	// Cell allow firing
	if (!TechnoExtData::CheckCellAllowFiring(pThis, pResolvedCell, pWeapon))
		return FireError::ILLEGAL;

	// Target-techno extension checks
	if (pResolvedTechno)
	{
		auto const pTargetExt = TechnoExtContainer::Instance.Find(pResolvedTechno);

		if (pWeaponExt->OnlyAttacker.Get()
			&& !pTargetExt->ContainFirer(pWeapon, pThis))
		{
			return FireError::ILLEGAL;
		}

		if (pThis->Berzerk
			&& !EnumFunctions::CanTargetHouse(
				RulesExtData::Instance()->BerzerkTargeting,
				pThis->Owner, pResolvedTechno->Owner))
		{
			return FireError::ILLEGAL;
		}

		if (!TechnoExtData::TechnoTargetAllowFiring(pThis, pResolvedTechno, pWeapon))
			return FireError::ILLEGAL;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pResolvedTechno, pWeapon))
			return FireError::ILLEGAL;

		if (pWeapon->Warhead->Airstrike)
		{
			auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

			if (!EnumFunctions::IsTechnoEligible(
				pResolvedTechno, pWHExt->AirstrikeTargets, false))
			{
				return FireError::ILLEGAL;
			}

			if (!GET_TECHNOTYPEEXT(pResolvedTechno)->AllowAirstrike.Get(
				pFootT || cast_to<BuildingClass*, false>(pResolvedTechno)->Type->CanC4))
			{
				return FireError::ILLEGAL;
			}
		}
	}
	// [END HOOK 0x6FC31C]

	// ====================================================================
	// [9] Drain weapon
	// ====================================================================
	if (pWeapon->DrainWeapon)
	{
		if (pTechnoT)
		{
			if (pTechnoT->DrainingMe)
				return FireError::ILLEGAL;

			if (!pTechnoT->GetTechnoType()->Drainable)
				return FireError::ILLEGAL;
		}
		// When !pTargetTechno the original gotos past the bunker check,
		// but the bunker check gates on pTargetTechno anyway — no-op skip.
	}

	// ====================================================================
	// [10] Bunker + short range
	// [HOOK 0x6FC3A1] InBunkerRangeCheck — uses GetRangeWithModifiers
	// [HOOK 0x6FC3AE] TankInBunker_LocomotorWarhead — was DEAD CODE, now integrated
	// ====================================================================
	if (pTechnoT
		&& pTechnoT->BunkerLinkedItem // +0x2E4
		&& pTechnoT->WhatAmI() == AbstractType::Unit)
	{
		// [0x6FC3AE] Locomotor warhead cannot affect bunkered units
		if (pWeapon->Warhead && pWeapon->Warhead->IsLocomotor)
			return FireError::ILLEGAL;

		// [0x6FC3A1] Range check with modifiers instead of raw < 384
		if (WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) < 384)
			return FireError::ILLEGAL;
	}

	// ====================================================================
	// [11] Self deploying check (Unit)
	// ====================================================================
	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		if (static_cast<UnitClass*>(pThis)->Deploying || static_cast<UnitClass*>(pThis)->Undeploying)
			return FireError::ILLEGAL;
	}

	// ====================================================================
	// [12] Warhead: Psychedelic
	// [HOOK 0x6FC3FE] Immunities — IsPsionicsImmune replaces type check
	// ====================================================================
	auto const pWarhead = static_cast<FakeWarheadTypeClass*>(pWeapon->Warhead);

	if (pWarhead && pWarhead->Psychedelic && pTechnoT)
	{
		// [0x6FC3FE] Extended psionic immunity
		if (TechnoExtData::IsPsionicsImmune(pTechnoT))
			return FireError::ILLEGAL;

		if (pTechnoT->BunkerLinkedItem)
			return FireError::ILLEGAL;
	}

	// ====================================================================
	// [13] Warhead: IsLocomotor
	// ====================================================================
	if (pWarhead && pWarhead->IsLocomotor)
	{
		// Deploying unit
		if (pTarget->WhatAmI() == AbstractType::Unit)
		{
			if (static_cast<UnitClass*>(pTarget)->Deploying || static_cast<UnitClass*>(pTarget)->Undeploying)
				return FireError::ILLEGAL;
		}

		// JumpJet currently moving
		if (pFootT && pFootT->GetTechnoType()->JumpJet)
		{
			auto const pFoot = static_cast<FootClass*>(pTarget);
			auto const pLoco = pFoot->Locomotor.GetInterfacePtr();
			if (!pLoco)
				_com_issue_error(E_POINTER);

			// vt+0x80 on ILocomotion — Is_Moving_Now
			if (pLoco->Is_Moving_Now())
				return FireError::ILLEGAL;
		}

		// SimpleDeployer in MISSION_UNLOAD
		if (pTarget->WhatAmI() == AbstractType::Unit)
		{
			auto const pUnit = static_cast<UnitClass*>(pTarget);
			if (pUnit->Type->IsSimpleDeployer
				&& pUnit->CurrentMission == Mission::Unload)
			{
				return FireError::ILLEGAL;
			}
		}

		// Organic targets
		if (pTechnoT && pTechnoT->GetTechnoType()->Organic)
			return FireError::ILLEGAL;
	}

	// ====================================================================
	// [14] Open-topped transport
	// [HOOK 0x6FC5C7] OpenTopped — extended transport checks
	// ====================================================================
	if (pThis->InOpenToppedTransport) // +0x82
	{
		if (!pWeapon->FireInTransport)
			return FireError::ILLEGAL;

		auto const pTransport = pThis->Transporter;
		if (pTransport)
		{
			if (pTransport->IsUnderEMP())
				return FireError::ILLEGAL;

			// [HOOK 0x6FC5C7]
			auto const pTransTypeExt = GET_TECHNOTYPEEXT(pTransport);

			if (pTransport->Transporter)
				return FireError::ILLEGAL;

			if (pTransport->Deactivated
				&& !pTransTypeExt->OpenTopped_AllowFiringIfDeactivated)
			{
				return FireError::ILLEGAL;
			}

			if (pTransTypeExt->OpenTopped_CheckTransportDisableWeapons
				&& TechnoExtContainer::Instance.Find(pTransport)->AE.flags.DisableWeapons
				&& pThisExt->CanFireWeaponType)
			{
				return FireError::REARM;
			}
		}
	}

	// ====================================================================
	// [15] Warp-shielded target + non-Temporal weapon
	// ====================================================================
	if (pTechnoT
		&& pWarhead
		&& pTechnoT->IsBeingWarpedOut()
		&& !pWarhead->Temporal)
	{
		return FireError::ILLEGAL;
	}

	// ====================================================================
	// [16] Spawner
	// [HOOK 0x6FC617] AirCarrierSkipCheckNearBridge
	// ====================================================================
	if (pWeapon->Spawner)
	{
		// [0x6FC617] Bridge check — skip if spawner is airborne
		if (pThis->IsOnBridge() && !pThis->IsInAir())
			return FireError::CANT;

		if (pThis->IsParalyzed())
			return FireError::CANT;

		if (!pThis->SpawnManager->CountAliveSpawns()) // +0x2D0
			return FireError::REARM;
	}

	// ====================================================================
	// [17] Temporal vs Spawned Aircraft
	// ====================================================================
	if (pWarhead && pWarhead->Temporal
		&& pTechnoT
		&& pTechnoT->WhatAmI() == AbstractType::Aircraft
		&& pTechnoT->GetTechnoType()->Spawned)
	{
		return FireError::ILLEGAL;
	}

	// [HOOK 0x6FC689] LandNavalTarget Pre
	if (pCellTarget)
	{
		const auto landType = pCellTarget->LandType;

		if (pThisType->NavalTargeting == NavalTargetingType::Naval_none
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			return FireError::ILLEGAL;
		}
	}
	else if (auto const pTerrainTarget = cast_to<TerrainClass*, false>(pTarget))
	{
		const auto landType = pTerrainTarget->GetCell()->LandType;

		if (pThisType->LandTargeting == LandTargetingType::Land_not_okay
			&& landType != LandType::Water && landType != LandType::Beach)
		{
			return FireError::ILLEGAL;
		}
		else if (pThisType->NavalTargeting == NavalTargetingType::Naval_none
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			return FireError::ILLEGAL;
		}
	}

	{
		const int nOtherSlot = (nWeaponIdx == 0) ? 1 : 0;
		auto const pOtherWeapon = pThis->GetWeapon(nOtherSlot)->WeaponType;

		if (pOtherWeapon)
		{
			if ((pOtherWeapon->UseFireParticles && pThis->Sys.Fire)
				|| (pOtherWeapon->IsRailgun && pThis->Sys.Railgun)
				|| (pOtherWeapon->UseSparkParticles && pThis->Sys.Spark)
				|| (pOtherWeapon->IsSonic && pThis->Wave))
			{
				return FireError::CANT;
			}
		}
	}

	// ====================================================================
	// [19] Anti-air / anti-ground
	// [HOOK 0x6FC7EB] InterceptBullet — bullet targets bypass AG check
	// [HOOK 0x6FC749] AntiUnderground — underground layer + AU
	// ====================================================================
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjExt = BulletTypeExtContainer::Instance.Find(pProjectile);
	bool const bTargetInAir = pTarget->IsInAir();
	bool const bTargetIsBullet = (pTarget->WhatAmI() == AbstractType::Bullet);

	// In-air target without AA
	if (bTargetInAir && !pProjectile->AA)
		return (pTarget != pThis->Target) ? FireError::ILLEGAL : FireError::REARM;

	// [HOOK 0x6FC749] Layer-based checks for foot-class targets
	if (pFootT)
	{
		switch (pFootT->InWhichLayer())
		{
		case Layer::Air:
		case Layer::Top:
			if (!pWeapon->Projectile->AA)
				return FireError::ILLEGAL;
			break;
		case Layer::Underground:
			if (!pProjExt->AU)
				return FireError::ILLEGAL;
			break;
		default:
			break;
		}
	}

	// ====================================================================
	// [20] Naval / ground / land targeting
	// [HOOK 0x6FC815] CellTargeting
	// ====================================================================

	if (pTechnoT)
	{
		// Techno target — water/bridge/air checks
		auto const pTechnoCell = pTechnoT->GetCell();
		bool bIsOnWater = (pTechnoCell->LandType == LandType::Water
			|| pTechnoCell->LandType == LandType::Beach);

		if (pTechnoT->IsInAir())
			bIsOnWater = false;

		if (pTechnoT->IsOnBridge())
		{
			bIsOnWater = false;
		}
		else if (bIsOnWater)
		{
			if (pThis->SelectNavalTargeting(pTarget) == NavalTargetingType::NotAvaible)
				return FireError::ILLEGAL;
		}

		if (pTechnoT->IsOnFloor() && !bIsOnWater)
		{
			if (pThisType->LandTargeting == LandTargetingType::Land_not_okay)
				return FireError::ILLEGAL;
		}
	}
	else
	{
		// Non-techno target

		// [HOOK 0x6FC7EB] Bullets bypass AG check
		if (!bTargetIsBullet && !bTargetInAir && !pProjectile->AG)
			return FireError::ILLEGAL;

		// [HOOK 0x6FC815] Bridge-aware cell targeting
		if(pCellTarget && (pCellTarget->ContainsBridge()
			|| pCellTarget->LandType == LandType::Water && pThisType->NavalTargeting == NavalTargetingType::Naval_none
			|| pCellTarget->LandType == LandType::Beach || pCellTarget->LandType == LandType::Water))
		{
			if (pThisType->LandTargeting == LandTargetingType::Land_not_okay)
				return FireError::ILLEGAL;
		}
	}

	// ====================================================================
	// [21] MagBeam
	// ====================================================================
	if (pWeapon->IsMagBeam)
	{
		auto const pCurrentTarget = pThis->Target; // +0x2AC

		if (pCurrentTarget
			&& pCurrentTarget != pTarget
			&& pThis->Wave
			&& pTarget->WhatAmI() != AbstractType::Building)
		{
			return FireError::REARM;
		}

		if (pCurrentTarget
			&& pCurrentTarget == pTarget
			&& pTarget->WhatAmI() != AbstractType::Building)
		{
			return FireError::REARM;
		}
	}

	// ====================================================================
	// [22] Burst sync + Arm timer
	// bSkipROF parameter replaces WhatActionObjectTemp::Skip global
	// ====================================================================
	if (!bSkipROF)
	{
		bool bTimerReady = false;

		// Unit burst synchronization (primary weapon only)
		if (nWeaponIdx == 0 && pThis->WhatAmI() == AbstractType::Unit)
		{
			int nBurstIndex = pThis->CurrentBurstIndex % pWeapon->Burst;
			if (nBurstIndex < 2)
			{
				auto const pUnitType = static_cast<UnitClass*>(pThis)->Type;

				int nBurstDelay = pUnitType->FiringSyncFrames[nBurstIndex];

				if (nBurstDelay != -1)
				{
					int nTimerValue = static_cast<UnitClass*>(pThis)->CurrentFiringFrame;

					if (nTimerValue != -1)
					{
						if (nTimerValue == nBurstDelay)
							bTimerReady = true;
						else
							return FireError::REARM;
					}
				}
			}
		}

		// Normal arm timer
		if (!bTimerReady)
		{
			int nRemaining = pThis->ROFTimer.TimeLeft; // +0x2F4

			if (pThis->ROFTimer.StartTime != -1) // +0x2EC
			{
				int nElapsed = Unsorted::CurrentFrame.get() - pThis->ROFTimer.StartTime;
				if (nElapsed >= nRemaining)
					bTimerReady = true;
				else
					nRemaining -= nElapsed;
			}

			if (!bTimerReady && nRemaining > 0)
				return FireError::REARM;
		}
	}

	// ====================================================================
	// [23] Post-timer particle / wave re-checks (current weapon)
	// ====================================================================
	if (pWeapon->UseFireParticles && pThis->Sys.Fire)
		return FireError::REARM;

	if (pWeapon->IsRailgun && pThis->Sys.Railgun)
		return FireError::REARM;

	if (pWeapon->UseSparkParticles && pThis->Sys.Spark)
		return FireError::REARM;

	if (pWeapon->IsSonic && pThis->Wave)
		return FireError::REARM;

	// ====================================================================
	// [24] Ammo
	// [HOOK 0x6FCA0D] Ammo — weapon-specific ammo cost
	// ====================================================================
	{
		const int nAmmo = pThis->Ammo; // +0x2FC
		if (nAmmo >= 0) // negative = infinite ammo
		{
			if (nAmmo < WeaponTypeExtContainer::Instance.Find(pWeapon)->Ammo)
				return FireError::AMMO;
		}
	}

	// ====================================================================
	// [25] Decloak
	// [HOOK 0x6FCA0D] — OpenTopped transport decloak override
	// ====================================================================
	if (pWeapon->DecloakToFire && pThis->CloakState != CloakState::Uncloaked)
	{
		bool bTransportHandlesDecloak = false;

		if (pThis->InOpenToppedTransport && pThis->Transporter)
		{
			auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(
				pThis->Transporter->GetTechnoType());

			bTransportHandlesDecloak = pTransTypeExt->OpenTopped_DecloakToFire.Get(
				RulesExtData::Instance()->OpenTopped_DecloakToFire);
		}

		if (!bTransportHandlesDecloak)
		{
			if (pThis->WhatAmI() != AbstractType::Aircraft
				|| pThis->CloakState == CloakState::Cloaked)
			{
				return FireError::CLOAKED;
			}
		}
	}

	// ====================================================================
	// [26] HunterSeeker
	// ====================================================================
	if (pThis->GetTechnoType()->HunterSeeker)
		return FireError::RANGE;

	// ====================================================================
	// [27] Parasite pre-checks
	// ====================================================================
	if (pWarhead->Parasite)
	{
		if (pThis->AbstractFlags & AbstractFlags::Foot)
		{
			if (!((FootClass*)pThis)->ParasiteImUsing->CanInfect(pFootT))
				return FireError::ILLEGAL;
		}

		if (pFootT && (Unsorted::CurrentFrame()
			< pFootT->LastBeParasitedStartFrame))
		{
			return FireError::ILLEGAL;
		}
	}

	// ====================================================================
	// [28] Target-is-Techno: warhead compatibility
	// [HOOK 0x6FCAFA] Verses — full replacement with custom armor, FakeEngineer, etc.
	// ====================================================================
	if (pTechnoT)
	{
		if (pTechnoT->IsSinking)
			return FireError::ILLEGAL;

		if (pWarhead->Parasite && pTechnoT->IsIronCurtained())
			return FireError::ILLEGAL;

		// Mind Control
		if (pWarhead->MindControl)
		{
			if (auto const pManager = (FakeCaptureManagerClass*)pThis->CaptureManager)
			{
				if (!pManager->__CanCapture(pTechnoT))
					return FireError::ILLEGAL;
			}
		}

		// Custom armor + verses with ForceFire
		Armor nArmor = TechnoExtData::GetTechnoArmor(pTechnoT, pWarhead);
		const auto pVsData = pWarhead->GetVersesData(nArmor);

		if (!pVsData->Flags.ForceFire && pVsData->Verses == 0.0)
			return FireError::ILLEGAL;

		// FakeEngineer
		auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

		if (pWHExt->FakeEngineer_CanCaptureBuildings || pWHExt->FakeEngineer_BombDisarm)
		{
			const int nWeaponRange = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
			const int nCurrentRange = pThis->DistanceFrom(pTechnoT);

			if (pWHExt->FakeEngineer_BombDisarm
				&& pTechnoT->AttachedBomb
				&& BombExtContainer::Instance.Find(pTechnoT->AttachedBomb)
					->Weapon->Ivan_Detachable)
			{
				return (nCurrentRange <= nWeaponRange)
					? FireError::OK : FireError::RANGE;
			}

			if (pWHExt->FakeEngineer_CanCaptureBuildings)
			{
				bool bCanCapture = pBuildingT
					&& pBuildingT->IsAlive
					&& pBuildingT->Health > 0
					&& !pThis->Owner->IsAlliedWith(pBuildingT)
					&& (pBuildingT->Type->Capturable || pBuildingT->Type->NeedsEngineer);

				if (bCanCapture)
				{
					return (nCurrentRange <= nWeaponRange)
						? FireError::OK : FireError::RANGE;
				}
			}
		}

		// BombDisarm — must have a detachable bomb
		if (pWarhead->BombDisarm)
		{
			if (!pTechnoT->AttachedBomb
				|| !BombExtContainer::Instance.Find(pTechnoT->AttachedBomb)
					->Weapon->Ivan_Detachable)
			{
				return FireError::ILLEGAL;
			}
		}

		// IvanBomb — can't stack
		if (pWarhead->IvanBomb && pTechnoT->AttachedBomb)
			return FireError::ILLEGAL;

		// Bridge / elevation — skip entirely if target is airborne
		if (!pTechnoT->IsInAir())
		{
			if (pThis->OnBridge != pTechnoT->OnBridge)
			{
				// Both on bridged cells
				auto const pThisCell = pThis->GetCell();
				if (pThisCell
					&& (pThisCell->Flags & CellFlags::Bridge)
					&& !pThis->IsInAir())
				{
					auto const pTgtCell = pTechnoT->GetCell();
					if (pTgtCell && (pTgtCell->Flags & CellFlags::Bridge))
						return FireError::ILLEGAL;
				}

				// Parasite elevation
				if (pWarhead->Parasite)
				{
					int nZDiff = Math::abs(
						pThis->GetCoords().Z - pTechnoT->GetCoords().Z);
					if (nZDiff > 2 * Unsorted::BridgeHeight)
						return FireError::ILLEGAL;
				}
			}
		}
	}
	// [END HOOK 0x6FCAFA]

	// ====================================================================
	// [29] Organic + Paralyzed
	// ====================================================================
	if (pThis->GetTechnoType()->Organic && pThis->IsParalyzed())
		return FireError::ILLEGAL;

	// ====================================================================
	// [30] Range check
	// ====================================================================
	if (bCheckRange && !pThis->IsCloseEnough(pTarget, nWeaponIdx))
		return FireError::RANGE;

	// ====================================================================
	// [HOOK 0x6FCD1D] OpenTopCloakFix
	// ====================================================================
	if (bCheckRange
		&& pThis->InOpenToppedTransport
		&& pThis->Transporter)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(
			pThis->Transporter->GetTechnoType());

		if (pTransTypeExt->OpenTopped_DecloakToFire.Get(
			RulesExtData::Instance()->OpenTopped_DecloakToFire))
		{
			pThis->Transporter->Uncloak(true);
		}
	}

	return FireError::OK;
}

FireError __fastcall FakeTechnoClass::__CanFireOriginal(TechnoClass* pThis, discard_t, AbstractClass* pTarget, int nWeaponIdx, bool bCheckRange)
{
	return FakeTechnoClass::__CanFireMod(pThis, pTarget, nWeaponIdx, bCheckRange);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6FC0B0, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D20, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E9054, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(CALL, 0x74103E, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(CALL, 0x51C9A7, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(CALL, 0x447FD4, FakeTechnoClass::__CanFireOriginal)
DEFINE_FUNCTION_JUMP(CALL, 0x41A9F6, FakeTechnoClass::__CanFireOriginal)

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

	R->EAX(FakeTechnoClass::__CanFireMod(pThis, pObject, WeaponIndex, true, true));
	return Continue;
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

	return  TechnoExtData::FiringAllowed(pThis, pThatTechno, pThis->GetWeapon(nWeaponIdx)->WeaponType) ?
		retContinue : retFireIllegal;

}

ASMJIT_PATCH(0x741113, UnitClass_CanFire_Heal, 0xA)
{
	enum { retFireIllegal = 0x74113A, retContinue = 0x741149 };
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pThatTechno, EDI);
	GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x1C, 0x8));

	return !pThatTechno->IsIronCurtained() && TechnoExtData::FiringAllowed(pThis, pThatTechno, pThis->GetWeapon(nWeaponIdx)->WeaponType) ?
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