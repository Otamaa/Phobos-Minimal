#include "Body.h"
#include <Ext/TechnoType/Body.h>

ASMJIT_PATCH(0x70DE40, BuildingClass_sub_70DE40_GattlingRateDownDelay, 0xA)
{
	enum { Return = 0x70DE62 };

	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(int, rateDown, STACK_OFFSET(0x0, 0x4));

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if (pTypeExt->RateDown_Delay < 0)
		return Return;

	++pExt->AccumulatedGattlingValue;
	auto remain = pExt->AccumulatedGattlingValue;

	if (!pExt->ShouldUpdateGattlingValue)
		remain -= pTypeExt->RateDown_Delay;

	if (remain <= 0)
		return Return;

	// Time's up
	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = true;

	if (pThis->Ammo <= pTypeExt->RateDown_Ammo)
		rateDown = pTypeExt->RateDown_Cover;

	if (!rateDown)
	{
		pThis->GattlingValue = 0;
		return Return;
	}

	pThis->GattlingValue -= (rateDown * remain);
	pThis->GattlingValue = (pThis->GattlingValue <= 0) ? 0 : pThis->GattlingValue;
	return Return;
}

ASMJIT_PATCH(0x70DE70, TechnoClass_sub_70DE70_GattlingRateDownReset, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = false;

	return 0;

}

ASMJIT_PATCH(0x70E01E, TechnoClass_sub_70E000_GattlingRateDownDelay, 0x6)
{
	enum { SkipGameCode = 0x70E04D };

	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(int, rateMult, STACK_OFFSET(0x10, 0x4));

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if (pTypeExt->RateDown_Delay < 0)
		return SkipGameCode;

	pExt->AccumulatedGattlingValue += rateMult;
	auto remain = pExt->AccumulatedGattlingValue;

	if (!pExt->ShouldUpdateGattlingValue)
		remain -= pTypeExt->RateDown_Delay;

	if (remain <= 0 && rateMult)
		return SkipGameCode;

	// Time's up
	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = true;

	if (!rateMult)
	{
		pThis->GattlingValue = 0;
		return SkipGameCode;
	}

	const auto rateDown = (pThis->Ammo <= pTypeExt->RateDown_Ammo) ? pTypeExt->RateDown_Cover.Get() : pTypeExt->AttachedToObject->RateDown;

	if (!rateDown)
	{
		pThis->GattlingValue = 0;
		return SkipGameCode;
	}

	pThis->GattlingValue -= (rateDown * remain);
	pThis->GattlingValue = (pThis->GattlingValue <= 0) ? 0 : pThis->GattlingValue;
	return SkipGameCode;
}
#include <Helpers/Macro.h>
#include <InfantryClass.h>

ASMJIT_PATCH(0x5206E4, InfantryClass_UpdateFiring_IsGattling, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, nWeaponIndex, EDI);

	if (pThis->Type->IsGattling)
	{
		auto fireError = pThis->GetFireError(pThis->Target, nWeaponIndex, true);

		if (fireError == FireError::OK ||
						fireError == FireError::REARM ||
						fireError == FireError::FACING ||
						fireError == FireError::ROTATING)
			pThis->GattlingRateUp(1);
		else
			pThis->GattlingRateDown(1);

		R->EAX(fireError);
		return 0x5206F9;
	}

	return 0;
}

ASMJIT_PATCH(0x5209D2, InfantryClass_UpdateFiring_IsGattling2, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, nWeaponIndex, ESI);

	if (pThis->Type->IsGattling)
	{
		auto fireError = pThis->GetFireError(pThis->Target, nWeaponIndex, true);

		if (fireError == FireError::OK ||
						fireError == FireError::REARM ||
						fireError == FireError::FACING ||
						fireError == FireError::ROTATING)
			pThis->GattlingRateUp(1);
		else
			pThis->GattlingRateDown(1);

		R->EAX(fireError);
		return 0x5209E4;
	}

	return 0;
}

ASMJIT_PATCH(0x520AD2, InfantryClass_UpdateFiring_IsGattling3, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
		pThis->GattlingRateDown(1);

	return 0;
}ASMJIT_PATCH_AGAIN(0x520A03, InfantryClass_UpdateFiring_IsGattling3, 0x7)


// Do you think the infantry's way of determining that weapons are secondary is stupid ?
// I think it's kind of stupid.
ASMJIT_PATCH(0x520888, InfantryClass_UpdateFiring_IsSecondary, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, weaponIdx, EDI);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->IsSecondary(weaponIdx) ? Secondary : Primary;
}