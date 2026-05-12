#include "Body.h"

#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <CaptureManagerClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

// Replaces TechnoClass::CanPassiveAquireTargets (0x7091D0 - 0x709282).
// Determines whether this techno is eligible to auto-acquire weapon targets.
// Integrates: TechnoClass_CanPassiveAquire_KillDriver (0x7091D6),
//             TechnoClass_CanPassiveAquire_AI         (0x7091FC).
bool __fastcall FakeTechnoClass::_CanPassiveAquire(TechnoClass* pThis)
{
	// Cannot passive acquire while a temporal weapon is actively warping a target out.
	if (pThis->IsWarpingSomethingOut())
		return false;

	// (Phobos) Units whose driver has been killed must not auto-attack.
	if (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled)
		return false;

	// Enslaved units (e.g. Slave Miner drones) are controlled by their master;
	// they should not independently passive acquire targets.
	if (pThis->SlaveOwner)
		return false;

	// Evaluate the type's CanPassiveAquire flag, with Phobos AI and Naval overrides.
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pOwner = pThis->Owner;

	bool canPassiveAquire;
	if (pTypeExt->PassiveAcquire_AI.isset()
		&& pOwner
		&& !pOwner->Type->MultiplayPassive
		&& !pOwner->IsControlledByHuman())
	{
		// AI-controlled house with an explicit AI override: use it.
		canPassiveAquire = pTypeExt->PassiveAcquire_AI.Get();
	}
	else
	{
		// Human or no AI override: respect the Naval-specific override if applicable,
		// otherwise fall back to the vanilla CanPassiveAquire flag.
		canPassiveAquire = (pType->Naval && pTypeExt->CanPassiveAquire_Naval.isset())
			? pTypeExt->CanPassiveAquire_Naval.Get()
			: pType->CanPassiveAquire;
	}

	if (!canPassiveAquire)
		return false;

	// Garrisonable buildings can only passive acquire through their occupants;
	// an empty garrisonable building has nothing to fire.
	if (pThis->WhatAmI() == AbstractType::Building)
	{
		auto const pBuildingType = static_cast<BuildingClass*>(pThis)->Type;
		if (pBuildingType->CanBeOccupied && !pThis->GetOccupantCount())
			return false;
	}

	// Determine whether this unit is a mind controller that still has capacity.
	auto const pCaptureManager = pThis->CaptureManager;
	bool const isMindControllerWithCapacity = pCaptureManager && !pCaptureManager->CannotControlAnyMore();

	// Engineers (renovators) under direct player control should not auto-attack;
	// the player directs them explicitly.
	if (pThis->IsEngineer() && pOwner->IsControlledByHuman())
		return false;

	// A mind controller that still has room for more units defers to the
	// mind-control targeting system rather than using its weapon passively.
	if (isMindControllerWithCapacity)
		return false;

	// The unit must actually have a weapon to be able to acquire a target.
	if (!pThis->IsArmed())
		return false;

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7091D0, FakeTechnoClass::_CanPassiveAquire)
