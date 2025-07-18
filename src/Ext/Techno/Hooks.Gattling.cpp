#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Helpers/Macro.h>

namespace FiringAITemp
{
	bool canFire;
	int weaponIndex;
	bool isSecondary;
	WeaponTypeClass* WeaponType;
	FireError fireError;
}

ASMJIT_PATCH(0x520AD2, InfantryClass_FiringAI_NoTarget, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
		pThis->GattlingRateDown(1);

	FiringAITemp::canFire = false;
	return 0;
}

ASMJIT_PATCH(0x5206D2, InfantryClass_FiringAI_SetContext, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, WeaponIndex, EDI);
	enum { SkipGameCode = 0x5209A6 };

	auto const pWeapon = pThis->GetWeapon(WeaponIndex)->WeaponType;

	if (!pWeapon)
	{
		R->AL(false);
		return SkipGameCode;
	}

	const auto pTarget = pThis->Target;
	FiringAITemp::weaponIndex = WeaponIndex;
	FiringAITemp::isSecondary = TechnoTypeExtContainer::Instance.Find(pThis->Type)->IsSecondary(WeaponIndex);
	FiringAITemp::WeaponType = pWeapon;
	FiringAITemp::fireError = pThis->GetFireError(pTarget, WeaponIndex, true);
	FiringAITemp::canFire = true;

	return 0;
}

void NOINLINE CheckGattling(InfantryClass* pThis)
{
	if (FiringAITemp::canFire)
	{
		if (pThis->Type->IsGattling)
		{
			FireError fireError = FiringAITemp::fireError;

			switch (fireError)
			{
			case FireError::OK:
			case FireError::REARM:
			case FireError::FACING:
			case FireError::ROTATING: {
				if (pThis->IsDeployed())
					pThis->GattlingRateDown(1);
				else
					pThis->GattlingRateUp(1);
			}
				break;
			default:
				pThis->GattlingRateDown(1);
				break;
			}
		}

		FiringAITemp::canFire = false;
	}
}

ASMJIT_PATCH(0x520AD9, InfantryClass_FiringAI_IsGattling, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	CheckGattling(pThis);
	return 0;
}

ASMJIT_PATCH(0x5209EE, InfantryClass_UpdateFiring_BurstNoDelay, 0x5)
{
	enum { SkipVanillaFire = 0x520A57 };

	GET(InfantryClass* const, pThis, EBP);
	GET(const int, wpIdx, ESI);
	GET(AbstractClass* const, pTarget, EAX);

	if (const auto pWeapon = pThis->GetWeapon(wpIdx)->WeaponType)
	{
		if (pWeapon->Burst > 1)
		{
			const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

			if (pExt->Burst_NoDelay && (!pExt->DelayedFire_Duration.isset() || pExt->DelayedFire_OnlyOnInitialBurst))
			{
				if (pThis->Fire(pTarget, wpIdx))
				{
					if (!pThis->CurrentBurstIndex)
						return SkipVanillaFire;

					auto rof = pThis->DiskLaserTimer.TimeLeft;
					pThis->DiskLaserTimer.Start(0);

					for (auto i = pThis->CurrentBurstIndex; i < pWeapon->Burst && pThis->GetFireError(pTarget, wpIdx, true) == FireError::OK && pThis->Fire(pTarget, wpIdx); ++i)
					{
						rof = pThis->DiskLaserTimer.TimeLeft;
						pThis->DiskLaserTimer.Start(0);
					}

					pThis->DiskLaserTimer.Start(rof);
				}

				return SkipVanillaFire;
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x5209A7, InfantryClass_FiringAI_BurstDelays, 0x8)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	const auto pWeaponstruct = pThis->GetWeapon(FiringAITemp::weaponIndex);

	if (!pWeaponstruct) {
		return ReturnFromFunction;
	}

	const auto pWeapon = pWeaponstruct->WeaponType;

	if (!pWeapon) {
		return ReturnFromFunction;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int cumulativeDelay = 0;
	int projectedDelay = 0;
	//auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	// Calculate cumulative burst delay as well cumulative SellSounddelay after next shot (projected delay).
	if (pWeaponExt->Burst_FireWithinSequence)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			int burstDelay = WeaponTypeExtData::GetBurstDelay(pWeapon, i);
			int delay = burstDelay > -1 ? burstDelay : ScenarioClass::Instance->Random.RandomRanged(3, 5);

			// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
			if (i != 0)
				delay = MaxImpl(delay, 1);

			cumulativeDelay += delay;

			if (i == pThis->CurrentBurstIndex)
				projectedDelay = cumulativeDelay + delay;
		}
	}

	//IsFiring
	if (R->AL())
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		if (TechnoExtData::HandleDelayedFireWithPauseSequence(pThis, FiringAITemp::weaponIndex, firingFrame + cumulativeDelay))
			return ReturnFromFunction;

		if (pThis->Animation.Stage == firingFrame + cumulativeDelay)
		{
			if (pWeaponExt->Burst_FireWithinSequence)
			{
				int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

				// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
				if (firingFrame + projectedDelay > frameCount)
				{
					pExt->ForceFullRearmDelay = true;
				}
			}

			R->EAX(FiringAITemp::weaponIndex); // Reuse the weapon index to save some time.
			return Continue;
		}
	}

	return ReturnFromFunction;
}

// avoid repeatedly calling GetFireError().
ASMJIT_PATCH(0x5206E4, InfantryClass_FiringAI_SetFireError, 0x6)
{
	R->EAX(FiringAITemp::fireError);
	return R->Origin() == 0x5206E4 ? 0x5206F9 : 0x5209E4;
}ASMJIT_PATCH_AGAIN(0x5209D2, InfantryClass_FiringAI_SetFireError, 0x6)

// Do you think the infantry's way of determining that weapons are secondary is stupid?
ASMJIT_PATCH(0x520968, InfantryClass_UpdateFiring_IsSecondary, 0x6)
{
	enum { Secondary = 0x52096C, SkipGameCode = 0x5209A0 };

	return FiringAITemp::isSecondary ? Secondary : SkipGameCode;
}

// I think it's kind of stupid.
ASMJIT_PATCH(0x520888, InfantryClass_UpdateFiring_IsSecondary2, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return FiringAITemp::isSecondary ? Secondary : Primary;
}