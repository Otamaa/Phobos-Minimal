#include "Body.h"
#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x70DE40, BuildingClass_sub_70DE40_GattlingRateDownDelay, 0xA)
{
	enum { Return = 0x70DE62 };

	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(int, rateDown, STACK_OFFSET(0x0, 0x4));

	auto newValue = pThis->GattlingValue;
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if (pTypeExt->RateDown_Reset && (!pThis->Target || pExt->LastTargetID != pThis->Target->UniqueID))
	{
		pExt->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
		pThis->GattlingValue = 0;
		pThis->CurrentGattlingStage = 0;
		return Return;
	}

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

	newValue -= (rateDown * remain);
	pThis->GattlingValue = (newValue <= 0) ? 0 : newValue;
	return Return;
}

DEFINE_HOOK(0x70DE70, TechnoClass_sub_70DE70_GattlingRateDownReset, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	{
		if (pTypeExt->RateDown_Reset && (!pThis->Target || pExt->LastTargetID != pThis->Target->UniqueID))
		{
			pExt->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
		}

		pExt->AccumulatedGattlingValue = 0;
		pExt->ShouldUpdateGattlingValue = false;
	}

	return 0;
}

DEFINE_HOOK(0x70E01E, TechnoClass_sub_70E000_GattlingRateDownDelay, 0x6)
{
	enum { SkipGameCode = 0x70E04D };

	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(int, rateMult, STACK_OFFSET(0x10, 0x4));

	auto newValue = pThis->GattlingValue;
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if (pTypeExt->RateDown_Reset && (!pThis->Target || pExt->LastTargetID != pThis->Target->UniqueID))
	{
		pExt->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
		pThis->GattlingValue = 0;
		pThis->CurrentGattlingStage = 0;
		return SkipGameCode;
	}

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

	newValue -= (rateDown * remain);
	pThis->GattlingValue = (newValue <= 0) ? 0 : newValue;
	return SkipGameCode;
}