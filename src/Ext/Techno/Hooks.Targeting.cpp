#include <Ext/TechnoType/Body.h>

#include "Body.h"
#include <InfantryClass.h>

ASMJIT_PATCH(0x6F7CE2, TechnoClass_CanAutoTargetObject_DisallowMoving, 0x6)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(AbstractClass* const, pTarget, ESI);
	GET(const int, WeaponIndex, EBX);

	if (const auto pUnit = cast_to<UnitClass*,false>(pThis))
	{
		if (TechnoExtData::CannotMove(pUnit))
		{
			R->EAX(pUnit->GetFireError(pTarget, WeaponIndex, true));
			return 0x6F7CEE;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x7088E3, TechnoClass_ShouldRetaliate_DisallowMoving, 0x6)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(AbstractClass* const, pTarget, EBP);
	GET(const int, WeaponIndex, EBX);

	if (const auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (TechnoExtData::CannotMove(pUnit))
		{
			R->Stack(STACK_OFFSET(0x18, 0x4), WeaponIndex);
			R->EAX(pUnit->GetFireError(pTarget, WeaponIndex, true));
			return 0x7088F3;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x6F7E24, TechnoClass_EvaluateObject_MapZone, 0x6)
{
	enum { AllowedObject = 0x6F7EA2, DisallowedObject = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(ZoneType, zone, STACK_OFFSET(0x3C ,0x18));

	auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!TechnoExtData::AllowedTargetByZone(pThis, pTarget, pThisTypeExt->TargetZoneScanType, pWeapon, zone))
		return DisallowedObject;

	return AllowedObject;
}

#pragma region PassiveAcquireMode

ASMJIT_PATCH(0x6F8E1F, TechnoClass_SelectAutoTarget_CeasefireMode, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(TechnoClass*, pThis, ESI);

	if(TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDriverKilled)
		return 0x6F8E38;

	R->CL(pType->NoAutoFire || (TechnoExtContainer::Instance.Find(pThis)->GetPassiveAcquireMode()) == PassiveAcquireMode::Ceasefire);
	return R->Origin() + 0x6;
}

ASMJIT_PATCH(0x7087DD, TechnoClass_CanRetaliateToAttacker_CeasefireMode, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(TechnoClass*, pThis, ESI);

	if(TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDriverKilled) return 0x73761Fu;

	R->CL(pType->CanRetaliate && (TechnoExtContainer::Instance.Find(pThis)->GetPassiveAcquireMode() != PassiveAcquireMode::Ceasefire));
	return R->Origin() + 0x6;
}

#pragma endregion